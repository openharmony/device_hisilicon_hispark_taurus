/*
 * Copyright (C) 2021 HiSilicon (Shanghai) Technologies CO., LIMITED.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <linux/platform_device.h>
#include "hiirq.h"
#include "hi_osal.h"
#include "securec.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SINGLE_REG_SIZE   4
#define MAX_NODE_LIMIT    64
#define HIIRQ_PAGE_SHIFT  12

static int node_cnt = 0;
static osal_spinlock_t g_irq_spin_lock = { 0 };
#define irq_spin_lock(flags) osal_spin_lock_irqsave(&g_irq_spin_lock, &(flags))
#define irq_spin_unlock(flags) osal_spin_unlock_irqrestore(&g_irq_spin_lock, &(flags))

#define HIIRQ_PFX "hiirq: "

#ifdef CONFIG_HI_LOG_TRACE_SUPPORT
#define hiirq_trace osal_printk
#else
#define hiirq_trace(str, fmt...)
#endif

typedef struct {
    volatile unsigned int reg_num;
    volatile unsigned int *raw_int_reg[HI_MAX_STATE_NUM];
    volatile unsigned int *mask_int_reg[HI_MAX_STATE_NUM];
    volatile unsigned int *clr_int_reg[HI_MAX_STATE_NUM];
} hiirq_normal_reg_map;

typedef struct {
    volatile unsigned int *cap_int_reg;
    volatile unsigned int *pt_int_reg[VI_MAX_DEV_NUM];
    volatile unsigned int *ch_int_reg[VI_MAX_PHY_PIPE_NUM];
    volatile unsigned int *isp_fe_int_reg[VI_MAX_ISP_FE_NUM];
} hiirq_vicap_reg_map;

typedef struct {
    volatile unsigned int *aio_int_status_reg;
    volatile unsigned int *aio_int_raw_reg;
    volatile unsigned int *tx_clr_reg[AO_DEV_MAX_NUM];
    volatile unsigned int *tx_raw_reg[AO_DEV_MAX_NUM];
    volatile unsigned int *tx_stat_reg[AO_DEV_MAX_NUM];
    volatile unsigned int *rx_clr_reg[AI_DEV_MAX_NUM];
    volatile unsigned int *rx_raw_reg[AI_DEV_MAX_NUM];
    volatile unsigned int *rx_stat_reg[AI_DEV_MAX_NUM];
} hiirq_aio_reg_map;

typedef struct {
    hi_int_type type;
    union {
        hiirq_normal_reg_map normal;
        hiirq_vicap_reg_map vicap;
        hiirq_aio_reg_map aio;
    };
} hiirq_reg_map_info;


typedef struct irq_strct {
    int                  irq_cnt;
    hiirq_irq_attr       irq_attr;
    osal_wait_t          irq_wait;
    hiirq_reg_map_info   map_info;
    hi_int_state_info    int_info;
    struct irq_strct     *next;
} hiirq_irq_list;

static hiirq_irq_list *head = NULL;

static hiirq_irq_list *add_list(hiirq_irq_attr *irqattr)
{
    hiirq_irq_list *tmp;

    tmp = (hiirq_irq_list *)osal_kmalloc(sizeof(hiirq_irq_list), osal_gfp_kernel);
    if (tmp == NULL) {
        return NULL;
    }
    (hi_void)memset_s(tmp, sizeof(hiirq_irq_list), 0, sizeof(hiirq_irq_list));
    (hi_void)memcpy_s(&(tmp->irq_attr), sizeof(hiirq_irq_attr), irqattr, sizeof(hiirq_irq_attr));
    tmp->irq_attr.enable_flag = HI_FALSE;
    tmp->irq_cnt = 0;
    osal_wait_init(&(tmp->irq_wait));
    tmp->next = head;
    head = tmp;
    return tmp;
}

static void free_list(void)
{
    hiirq_irq_list *tmp = NULL;

    while (head != NULL) {
        tmp = head->next;
        osal_kfree(head);
        head = tmp;
    }
}

static hiirq_irq_list *get_list_node(int irq, void *dev_id)
{
    hiirq_irq_list *tmp = head;

    node_cnt = 0;
    while (tmp != NULL) {
        if (tmp->irq_attr.irq_num == irq && ((tmp->irq_attr.dev == dev_id) || (tmp->irq_attr.dev == NULL))) {
            if (tmp->irq_attr.dev == NULL) {
                tmp->irq_attr.dev = dev_id;
            }
            return tmp;
        }
        node_cnt++;
        tmp = tmp->next;
    }
    return NULL;
}

static void unmap_int_reg_for_aio(hiirq_aio_reg_map *map)
{
    int i;

    if (map->aio_int_status_reg != HI_NULL) {
        osal_iounmap((void *)map->aio_int_status_reg, SINGLE_REG_SIZE);
        map->aio_int_status_reg = HI_NULL;
    }
    if (map->aio_int_raw_reg != HI_NULL) {
        osal_iounmap((void *)map->aio_int_raw_reg, SINGLE_REG_SIZE);
        map->aio_int_raw_reg = HI_NULL;
    }

    for (i = 0; i < AO_DEV_MAX_NUM; i++) {
        if (map->tx_clr_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->tx_clr_reg[i], SINGLE_REG_SIZE);
            map->tx_clr_reg[i] = HI_NULL;
        }
        if (map->tx_raw_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->tx_raw_reg[i], SINGLE_REG_SIZE);
            map->tx_raw_reg[i] = HI_NULL;
        }
        if (map->tx_stat_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->tx_stat_reg[i], SINGLE_REG_SIZE);
            map->tx_stat_reg[i] = HI_NULL;
        }
    }

    for (i = 0; i < AI_DEV_MAX_NUM; i++) {
        if (map->rx_clr_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->rx_clr_reg[i], SINGLE_REG_SIZE);
            map->rx_clr_reg[i] = HI_NULL;
        }
        if (map->rx_raw_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->rx_raw_reg[i], SINGLE_REG_SIZE);
            map->rx_raw_reg[i] = HI_NULL;
        }
        if (map->rx_stat_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->rx_stat_reg[i], SINGLE_REG_SIZE);
            map->rx_stat_reg[i] = HI_NULL;
        }
    }

    return;
}

static void unmap_int_reg_for_vicap(hiirq_vicap_reg_map *map)
{
    int i;

    if (map->cap_int_reg != HI_NULL) {
        osal_iounmap((void *)map->cap_int_reg, SINGLE_REG_SIZE);
        map->cap_int_reg = HI_NULL;
    }

    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (map->pt_int_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->pt_int_reg[i], SINGLE_REG_SIZE);
            map->pt_int_reg[i] = HI_NULL;
        }
    }

    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++) {
        if (map->ch_int_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->ch_int_reg[i], SINGLE_REG_SIZE);
            map->ch_int_reg[i] = HI_NULL;
        }
    }

    for (i = 0; i < VI_MAX_ISP_FE_NUM; i++) {
        if (map->isp_fe_int_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->isp_fe_int_reg[i], SINGLE_REG_SIZE);
            map->isp_fe_int_reg[i] = HI_NULL;
        }
    }

    return;
}

static void unmap_int_reg_for_normal(hiirq_normal_reg_map *map)
{
    int i;

    for (i = 0; i < map->reg_num; i++) {
        if (map->raw_int_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->raw_int_reg[i], SINGLE_REG_SIZE);
            map->raw_int_reg[i] = HI_NULL;
        }
        if (map->mask_int_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->mask_int_reg[i], SINGLE_REG_SIZE);
            map->mask_int_reg[i] = HI_NULL;
        }
        if (map->clr_int_reg[i] != HI_NULL) {
            osal_iounmap((void *)map->clr_int_reg[i], SINGLE_REG_SIZE);
            map->clr_int_reg[i] = HI_NULL;
        }
    }
    map->reg_num = 0;

    return;
}

static void unmap_int_reg(hiirq_reg_map_info *map_info)
{
    if (map_info->type == HI_INT_VICAP) {
        unmap_int_reg_for_vicap(&map_info->vicap);
    } else if (map_info->type == HI_INT_AIO) {
        unmap_int_reg_for_aio(&map_info->aio);
    } else {
        unmap_int_reg_for_normal(&map_info->normal);
    }
    return;
}

static void del_list(void)
{
    hiirq_irq_list *tmp = head;
    hiirq_irq_list *tmp2 = NULL;

    while (tmp != NULL) {
        tmp2 = tmp->next;
        osal_wait_destroy(&(tmp->irq_wait));
        unmap_int_reg(&(tmp->map_info));
        osal_kfree(tmp);
        tmp = tmp2;
    }
    head = NULL;
}

static void read_int_status_for_aio(hiirq_aio_reg_map *map, hi_aio_int_state *state)
{
    int i;

    state->aio_int_status_state = *map->aio_int_status_reg;
    state->aio_int_raw_state = *map->aio_int_raw_reg;

    for (i = 0; i < AO_DEV_MAX_NUM; i++) {
        state->tx_raw_state[i] = *map->tx_raw_reg[i];
        state->tx_stat_state[i] = *map->tx_stat_reg[i];
    }
    for (i = 0; i < AI_DEV_MAX_NUM; i++) {
        state->rx_raw_state[i] = *map->rx_raw_reg[i];
        state->rx_stat_state[i] = *map->rx_stat_reg[i];
    }
    return;
}

static void read_int_status_for_vicap(hiirq_vicap_reg_map *map, hi_vicap_int_state *state)
{
    int i;

    state->cap_state = *map->cap_int_reg;
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        state->pt_state[i] = *map->pt_int_reg[i];
    }
    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++) {
        state->ch_state[i] = *map->ch_int_reg[i];
    }
    for (i = 0; i < VI_MAX_ISP_FE_NUM; i++) {
        state->isp_fe_state[i] = *map->isp_fe_int_reg[i];
    }
    return;
}

static void read_int_status_for_normal(hiirq_normal_reg_map *map, hi_normal_int_state *state)
{
    int i;

    state->state_num = map->reg_num;
    for (i = 0; i < map->reg_num; i++) {
        state->raw_state[i] = *(map->raw_int_reg[i]);
        state->mask_state[i] = *(map->mask_int_reg[i]);
    }

    return;
}

static void read_int_status(hiirq_irq_list *irq_node, hi_int_state_info *int_info)
{
    int_info->type = irq_node->map_info.type;
    if (irq_node->map_info.type == HI_INT_VICAP) {
        read_int_status_for_vicap(&irq_node->map_info.vicap, &int_info->vicap);
    } else if (irq_node->map_info.type == HI_INT_AIO) {
        read_int_status_for_aio(&irq_node->map_info.aio, &int_info->aio);
    } else {
        read_int_status_for_normal(&irq_node->map_info.normal, &int_info->normal);
    }
    return;
}

static void clear_int_for_aio(hiirq_aio_reg_map *map, hi_aio_int_state *state)
{
    int i;

    for (i = 0; i < AI_DEV_MAX_NUM; i++) {
        *map->rx_clr_reg[i] = state->rx_raw_state[i];
    }

    for (i = 0; i < AO_DEV_MAX_NUM; i++) {
        *map->tx_clr_reg[i] = state->tx_raw_state[i];
    }

    return;
}

static void clear_int_for_vicap(hiirq_vicap_reg_map *map, hi_vicap_int_state *state)
{
    int i;

    *map->cap_int_reg = state->cap_state;
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        *map->pt_int_reg[i] = state->pt_state[i];
    }
    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++) {
        *map->ch_int_reg[i] = state->ch_state[i];
    }
    for (i = 0; i < VI_MAX_ISP_FE_NUM; i++) {
        *map->isp_fe_int_reg[i] = state->isp_fe_state[i];
    }
    return;
}

static void clear_int_for_scd(hiirq_normal_reg_map *map)
{
    int i;
    for (i = 0; i < map->reg_num; i++) {
        *map->clr_int_reg[i] = 1;
    }
    return;
}

static void clear_int_for_vdh(hiirq_normal_reg_map *map, hi_normal_int_state *state)
{
    int i;
    for (i = 0; i < map->reg_num; i++) {
        *map->clr_int_reg[i] = state->mask_state[i];
    }
    return;
}

static void clear_int_for_nnie(hiirq_normal_reg_map *map, hi_normal_int_state *state)
{
    int i;
    for (i = 0; i < map->reg_num; i++) {
        *map->clr_int_reg[i] = state->mask_state[i];
    }
    return;
}

static void clear_int_for_normal(hiirq_normal_reg_map *map, hi_normal_int_state *state)
{
    int i;
    for (i = 0; i < map->reg_num; i++) {
        *map->clr_int_reg[i] = state->raw_state[i];
    }
    return;
}

static void clear_int(hiirq_irq_list *irq_node, hi_int_state_info *int_info)
{
    if (irq_node->map_info.type == HI_INT_VICAP) {
        clear_int_for_vicap(&irq_node->map_info.vicap, &int_info->vicap);
    } else if (irq_node->map_info.type == HI_INT_AIO) {
        clear_int_for_aio(&irq_node->map_info.aio, &int_info->aio);
    } else if (irq_node->map_info.type == HI_INT_SCD) {
        clear_int_for_scd(&irq_node->map_info.normal);
    } else if (irq_node->map_info.type == HI_INT_VDH) {
        clear_int_for_vdh(&irq_node->map_info.normal, &int_info->normal);
    } else if (irq_node->map_info.type == HI_INT_NNIE) {
        clear_int_for_nnie(&irq_node->map_info.normal, &int_info->normal);
    } else {
        clear_int_for_normal(&irq_node->map_info.normal, &int_info->normal);
    }
    return;
}

static void save_int_status_for_aio(hi_aio_int_state *save, hi_aio_int_state *state)
{
    int i;

    save->aio_int_status_state |= state->aio_int_status_state;
    save->aio_int_raw_state |= state->aio_int_raw_state;
    for (i = 0; i < AO_DEV_MAX_NUM; i++) {
        save->tx_raw_state[i] |= state->tx_raw_state[i];
        save->tx_stat_state[i] |= state->tx_stat_state[i];
    }
    for (i = 0; i < AI_DEV_MAX_NUM; i++) {
        save->rx_raw_state[i] |= state->rx_raw_state[i];
        save->rx_stat_state[i] |= state->rx_stat_state[i];
    }
    return;
}

static void save_int_status_for_vicap(hi_vicap_int_state *save, hi_vicap_int_state *state)
{
    int i;

    save->cap_state |= state->cap_state;
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        save->pt_state[i] |= state->pt_state[i];
    }
    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++) {
        save->ch_state[i] |= state->ch_state[i];
    }
    for (i = 0; i < VI_MAX_ISP_FE_NUM; i++) {
        save->isp_fe_state[i] |= state->isp_fe_state[i];
    }
    return;
}

static void save_int_status_for_normal(hi_normal_int_state *save, hi_normal_int_state *state)
{
    int i;

    save->state_num = state->state_num;
    for (i = 0; i < state->state_num; i++) {
        save->raw_state[i] |= state->raw_state[i];
        save->mask_state[i] |= state->mask_state[i];
    }
    return;
}

static void save_int_status(hiirq_irq_list *irq_node, hi_int_state_info *int_info)
{
    irq_node->int_info.type = int_info->type;
    if (int_info->type == HI_INT_VICAP) {
        save_int_status_for_vicap(&irq_node->int_info.vicap, &int_info->vicap);
    } else if (int_info->type == HI_INT_AIO) {
        save_int_status_for_aio(&irq_node->int_info.aio, &int_info->aio);
    } else {
        save_int_status_for_normal(&irq_node->int_info.normal, &int_info->normal);
    }
}

static int hiirq_interrupt(int irq, void *dev_id)
{
    hiirq_irq_list *irq_node = NULL;
    hi_int_state_info int_info = { 0 };
    unsigned long flags;

    irq_node = get_list_node(irq, dev_id);
    if (irq_node == NULL || irq_node->irq_attr.enable_flag != HI_TRUE) {
        return OSAL_IRQ_NONE;
    } else {
        irq_spin_lock(flags);
        irq_node->irq_cnt++;
        read_int_status(irq_node, &int_info);
        osal_isb();
        clear_int(irq_node, &int_info);
        save_int_status(irq_node, &int_info);
        irq_spin_unlock(flags);

        osal_wakeup(&(irq_node->irq_wait));
    }
    return OSAL_IRQ_HANDLED;
}

static int hiirq_wait_condition_callback(const void *param)
{
    hiirq_irq_list *irq = (hiirq_irq_list *)param;
    return (irq->irq_cnt != 0);
}

hi_s32 hiirq_request_or_free_irq(hi_uintptr_t arg, hi_void *private_data)
{
    hiirq_irq_attr *p = (hiirq_irq_attr *)arg;
    hiirq_irq_list *irq_node = NULL;
    int ret;

    if (p == NULL) {
        hiirq_trace("[%s,line:%d]hiirq_set_irq_param arg is null\n", HIIRQ_PFX, __LINE__);
        return HI_FAILURE;
    }

    irq_node = get_list_node(p->irq_num, p->dev);
    if (irq_node == NULL) {
        hiirq_trace("[%s,line:%d] not find irq_node.\n", HIIRQ_PFX, __LINE__);
        return HI_FAILURE;
    }

    if (p->enable_flag == irq_node->irq_attr.enable_flag) {
        return HI_FAILURE;
    }

    if (p->enable_flag) { // add irq
        (hi_void)memcpy_s(irq_node->irq_attr.irq_name, MAX_IRQ_NAME_LEN, p->irq_name, MAX_IRQ_NAME_LEN);
        ret = osal_request_irq(p->irq_num, hiirq_interrupt, NULL, irq_node->irq_attr.irq_name, p->dev);
        if (ret != 0) {
            hiirq_trace("[%s,line:%d]hiirq: failed to register (%s),irq_num:%d\n", HIIRQ_PFX, __LINE__, p->irq_name,
                p->irq_num);
            ret = HI_FAILURE;
            return ret;
        }
        irq_node->irq_cnt = 0;
    }
    if (!p->enable_flag) { // free irq
        hiirq_trace("hiirq:disable irq_num:%d\n", p->irq_num);
        irq_node->irq_cnt = -1;
        osal_wakeup(&(irq_node->irq_wait));
        osal_free_irq(p->irq_num, p->dev);
    }
    irq_node->irq_attr.enable_flag = p->enable_flag;
    irq_node->irq_attr.irq_mod     = p->irq_mod;
    irq_node->irq_attr.wait_mode   = p->wait_mode;
    irq_node->irq_attr.dev         = p->dev;
    return HI_SUCCESS;
}

hi_s32 hiirq_wait_irq(hi_uintptr_t arg, hi_void *private_data)
{
    hiirq_irq_attr *para = (hiirq_irq_attr *)arg;
    hiirq_irq_list *irq_node = NULL;
    hi_irq_arg user_dev = { 0 };
    unsigned long flags;
    int ret;

    if (para == NULL) {
        hiirq_trace("[%s,line:%d] error invalid arg\n", HIIRQ_PFX, __LINE__);
        return HI_FAILURE;
    }
    osal_copy_from_user(&user_dev, para->dev, sizeof(hi_irq_arg));
    irq_node = get_list_node(para->irq_num, para->dev);
    if (irq_node == NULL || irq_node->irq_attr.enable_flag == HI_FALSE) {
        hiirq_trace("[%s,line:%d]irq_num:%d not enable\n", HIIRQ_PFX, __LINE__, para->irq_num);
        return HI_ERR_IRQ_UNEXIST;
    }
retry:
    if (irq_node->irq_cnt == 0) {
        ret = osal_wait_event_interruptible(&(irq_node->irq_wait), hiirq_wait_condition_callback, irq_node);
        if (ret != 0) {
            hiirq_trace("[%s,line:%d]osal_wait_event_interruptible return error:%d, irq: %d\n", HIIRQ_PFX, __LINE__,
                ret, para->irq_num);
            return ret;
        }
        if (irq_node->irq_cnt < 0) {
            return HI_FAILURE;
        }
        if (irq_node->irq_attr.wait_mode == IRQ_WAIT_FOREVER) { // wait forever
            goto retry;
        }
    } else {
        if (irq_node->irq_cnt < 0) {
            return HI_FAILURE;
        }
    }
    irq_spin_lock(flags);
    if (irq_node->irq_attr.irq_mod == IRQ_TRIG_ONECE) { // clear irqcnt for trigger mode 0
        irq_node->irq_cnt = 0;
    } else {
        irq_node->irq_cnt--;
    }
    (hi_void)memcpy_s(&user_dev.int_info, sizeof(hi_int_state_info), &irq_node->int_info, sizeof(hi_int_state_info));
    (hi_void)memset_s(&irq_node->int_info, sizeof(hi_int_state_info), 0, sizeof(hi_int_state_info));
    irq_spin_unlock(flags);

    osal_copy_to_user(para->dev, &user_dev, sizeof(hi_irq_arg));

    return HI_SUCCESS;
}

static struct osal_dev *g_hiirq_dev = NULL;
static struct platform_device *g_hiirq_pdev = NULL;
hi_s32 hiirq_get_irq_num(hi_uintptr_t arg, hi_void *private_data)
{
    hiirq_irq_attr *para = (hiirq_irq_attr *)arg;

    if (para == NULL) {
        hiirq_trace("[%s,line:%d] error invalid arg\n", HIIRQ_PFX, __LINE__);
        return HI_FAILURE;
    }
    if (para->irq_name[0] != 0) {
        return platform_get_irq_byname(g_hiirq_pdev, para->irq_name);
    }
    return platform_get_irq(g_hiirq_pdev, para->irq_num);
}

static hi_s32 hiirq_map_irq_reg_for_aio(hiirq_aio_reg_map *map, hi_aio_int_reg *reg)
{
    int i;

    map->aio_int_status_reg = osal_ioremap(reg->aio_int_status_reg, SINGLE_REG_SIZE);
    map->aio_int_raw_reg = osal_ioremap(reg->aio_int_raw_reg, SINGLE_REG_SIZE);
    if ((map->aio_int_status_reg == NULL) || (map->aio_int_raw_reg == NULL)) {
        hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
        goto fail;
    }

    for (i = 0; i < AO_DEV_MAX_NUM; i++) {
        map->tx_clr_reg[i] = osal_ioremap(reg->tx_clr_reg[i], SINGLE_REG_SIZE);
        map->tx_raw_reg[i] = osal_ioremap(reg->tx_raw_reg[i], SINGLE_REG_SIZE);
        map->tx_stat_reg[i] = osal_ioremap(reg->tx_stat_reg[i], SINGLE_REG_SIZE);
        if ((map->tx_clr_reg[i] == NULL) || (map->tx_raw_reg[i] == NULL) || (map->tx_stat_reg[i] == NULL)) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }
    }
    for (i = 0; i < AI_DEV_MAX_NUM; i++) {
        map->rx_clr_reg[i] = osal_ioremap(reg->rx_clr_reg[i], SINGLE_REG_SIZE);
        map->rx_raw_reg[i] = osal_ioremap(reg->rx_raw_reg[i], SINGLE_REG_SIZE);
        map->rx_stat_reg[i] = osal_ioremap(reg->rx_stat_reg[i], SINGLE_REG_SIZE);
        if ((map->rx_clr_reg[i] == NULL) || (map->rx_raw_reg[i] == NULL) || (map->rx_stat_reg[i] == NULL)) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }
    }
    return HI_SUCCESS;

fail:
    unmap_int_reg_for_aio(map);
    return HI_FAILURE;
}


static hi_s32 hiirq_map_irq_reg_for_vicap(hiirq_vicap_reg_map *map, hi_vicap_int_reg *reg)
{
    int i;

    map->cap_int_reg = osal_ioremap(reg->cap_reg, SINGLE_REG_SIZE);
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        map->pt_int_reg[i] = osal_ioremap(reg->pt_reg[i], SINGLE_REG_SIZE);
        if (map->pt_int_reg[i] == NULL) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }
    }
    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++) {
        map->ch_int_reg[i] = osal_ioremap(reg->ch_reg[i], SINGLE_REG_SIZE);
        if (map->ch_int_reg[i] == NULL) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }
    }
    for (i = 0; i < VI_MAX_ISP_FE_NUM; i++) {
        map->isp_fe_int_reg[i] = osal_ioremap(reg->isp_fe_reg[i], SINGLE_REG_SIZE);
        if (map->isp_fe_int_reg[i] == NULL) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }
    }
    return HI_SUCCESS;

fail:
    unmap_int_reg_for_vicap(map);
    return HI_FAILURE;
}

static hi_s32 hiirq_map_irq_reg_for_normal(hiirq_normal_reg_map *map, hi_normal_int_reg *reg)
{
    int i;

    if (reg->reg_num > HI_MAX_STATE_NUM) {
        hiirq_trace("[%s,line:%d] error invalid reg_num:%d\n", HIIRQ_PFX, __LINE__, reg->reg_num);
        return HI_FAILURE;
    }

    for (i = 0; i < reg->reg_num; i++) {
        map->raw_int_reg[i] = osal_ioremap(reg->raw_int_reg[i], SINGLE_REG_SIZE);
        if (map->raw_int_reg[i] == NULL) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }

        map->mask_int_reg[i] = osal_ioremap(reg->mask_int_reg[i], SINGLE_REG_SIZE);
        if (map->mask_int_reg[i] == NULL) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }

        map->clr_int_reg[i] = osal_ioremap(reg->clr_int_reg[i], SINGLE_REG_SIZE);
        if (map->clr_int_reg[i] == NULL) {
            hiirq_trace("[%s,line:%d] osal_ioremap failed\n", HIIRQ_PFX, __LINE__);
            goto fail;
        }
    }

    map->reg_num = reg->reg_num;
    return HI_SUCCESS;

fail:
    unmap_int_reg_for_normal(map);
    return HI_FAILURE;
}

hi_s32 hiirq_set_irq_reg(hi_uintptr_t arg, hi_void *private_data)
{
    hiirq_set_irq_reg_info *para = (hiirq_set_irq_reg_info *)arg;
    hiirq_irq_list *irq_node = NULL;
    hiirq_irq_attr irqattr = { 0 };
    int ret;

    if (para == NULL) {
        hiirq_trace("[%s,line:%d] error invalid arg\n", HIIRQ_PFX, __LINE__);
        return HI_FAILURE;
    }

    irq_node = get_list_node(para->irq_num, 0);
    if (irq_node == NULL) {
        if (node_cnt >= MAX_NODE_LIMIT) {
            hiirq_trace("hiirq: node_cnt out of limit!\n");
            return HI_FAILURE;
        }
        irqattr.irq_num = para->irq_num;
        irq_node = add_list(&irqattr);
        if (irq_node == NULL) {
            hiirq_trace("hiirq: add irq node failed!\n");
            return HI_FAILURE;
        }
    }

    irq_node->map_info.type = para->reg_info.type;
    if (para->reg_info.type == HI_INT_VICAP) {
        ret = hiirq_map_irq_reg_for_vicap(&irq_node->map_info.vicap, &para->reg_info.vicap);
    } else if (para->reg_info.type == HI_INT_AIO) {
        ret = hiirq_map_irq_reg_for_aio(&irq_node->map_info.aio, &para->reg_info.aio);
    } else {
        ret = hiirq_map_irq_reg_for_normal(&irq_node->map_info.normal, &para->reg_info.normal);
    }

    return ret;
}

static long hiirq_ioctl(unsigned int cmd, unsigned long para, void *private_data)
{
    hi_uintptr_t arg = (hi_uintptr_t)para;
    switch (cmd) {
        case IRQ_REQUEST_OR_FREE_IRQ_CTRL:
            return hiirq_request_or_free_irq(arg, private_data);

        case IRQ_WAIT_IRQ_CTRL:
            return hiirq_wait_irq(arg, private_data);

        case IRQ_IOC_GET_IRQ_NUM_CTRL:
            return hiirq_get_irq_num(arg, private_data);

        case IRQ_IOC_SET_IRQ_REG_CTRL:
            return hiirq_set_irq_reg(arg, private_data);

        default:
            hiirq_trace("[%s,line:%d]Error: Inappropriate ioctl for device. cmd=%u\n", HIIRQ_PFX, __LINE__, cmd);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static hi_s32 hiirq_mmap(osal_vm_t *vm, hi_ulong start, hi_ulong end, hi_ulong vm_pgoff, hi_void *private_data)
{
    hi_s32 size;
    hi_u64 phy_addr;

    if (start > end) {
        return HI_FAILURE;
    }

    size = (end - start) / 2;

    /* it's necessary for the variable "size" to align 4k(page_size). */
#define PAGE_SIZE_MASK 0xfffff000
    if ((hi_u32)size & (~PAGE_SIZE_MASK)) {
        return HI_FAILURE;
    }
#undef PAGE_SIZE_MASK

    phy_addr = (hi_u64)(vm_pgoff << HIIRQ_PAGE_SHIFT);
    if (cmpi_check_mmz_phy_addr(phy_addr, size) != HI_SUCCESS) {
        hiirq_trace("addr: %#llx, size: %d, invalid phyaddr!\n", phy_addr, size);
        return HI_FAILURE;
    }

    osal_pgprot_noncached(vm);

    if (osal_remap_pfn_range(vm, start, vm_pgoff, size)) {
        return HI_FAILURE;
    }
    if (osal_remap_pfn_range(vm, start + size, vm_pgoff, size)) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static int hiirq_open(void *private_data)
{
    hiirq_trace("Enter hiirq_open\n");
    return HI_SUCCESS;
}

static int hiirq_release(void *private_data)
{
    hiirq_trace("Enter hiirq_release.\n");
    free_list();
    return HI_SUCCESS;
}

static struct osal_fileops g_hiirq_fops = {
    open :           hiirq_open,
    unlocked_ioctl : hiirq_ioctl,
    mmap :           hiirq_mmap,
    release :        hiirq_release,
};

int hiirq_init(struct platform_device *pdev)
{
    osal_spin_lock_init(&g_irq_spin_lock);
    g_hiirq_dev = osal_createdev(HIIRQ_DEVICE_NAME);
    if (g_hiirq_dev == NULL) {
        hiirq_trace("[%s,line:%d]Error: can't create dev\n", HIIRQ_PFX, __LINE__);
        return HI_FAILURE;
    }

    g_hiirq_dev->minor = 255; /* define a macro to substitute 255 */
    g_hiirq_dev->fops = &g_hiirq_fops;
    if (osal_registerdevice(g_hiirq_dev) != 0) {
        hiirq_trace("[%s,line:%d]Error: can't register\n", HIIRQ_PFX, __LINE__);
        osal_destroydev(g_hiirq_dev);
        return HI_FAILURE;
    }
    g_hiirq_pdev = pdev;

    hiirq_trace("hi_irq init ok. ver=%s, %s.\n", __DATE__, __TIME__);
    return HI_SUCCESS;
}

void hiirq_exit(void)
{
    del_list();
    osal_deregisterdevice(g_hiirq_dev);
    osal_destroydev(g_hiirq_dev);
    g_hiirq_pdev = NULL;
    osal_spin_lock_destroy(&g_irq_spin_lock);

    hiirq_trace("hi_irq exit ok.\n");
}
