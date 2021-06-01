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

#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include <linux/module.h>
#include "securec.h"
#include "hi_osal.h"

OSAL_LIST_HEAD(nb_list);

struct nb_node {
    struct osal_notifier_block *osal_nb;
    struct notifier_block *nb;
    struct osal_list_head node;
};

static struct osal_notifier_block *osal_find_ob(struct notifier_block *nb)
{
    struct osal_list_head *this = NULL;
    if (osal_list_empty(&nb_list)) {
        osal_trace("find nb failed! nb_list is empty!\n");
        return NULL;
    }
    osal_list_for_each(this, &nb_list) {
        struct nb_node *ns = osal_list_entry(this, struct nb_node, node);
        if (ns->nb == nb) {
            return ns->osal_nb;
        }
    }
    osal_trace("find ob failed!\n");
    return NULL;
}

static int osal_del_nb(struct osal_notifier_block *nb)
{
    struct osal_list_head *this = NULL;
    struct osal_list_head *next = NULL;
    if (osal_list_empty(&nb_list)) {
        osal_trace("find nb failed! nb_list is empty!\n");
        return -1;
    }
    osal_list_for_each_safe(this, next, &nb_list) {
        struct nb_node *ns = osal_list_entry(this, struct nb_node, node);
        if (ns->osal_nb == nb) {
            osal_list_del(this);
            kfree(ns);
            return 0;
        }
    }
    osal_trace("del nb failed!\n");
    return -1;
}

static int osal_notifier(struct notifier_block *nb, unsigned long action, void *data)
{
    struct osal_notifier_block *ob = osal_find_ob(nb);
    if ((ob != NULL) && (ob->notifier_call != NULL)) {
        return ob->notifier_call(ob, action, data);
    }
    return 0;
}

int osal_register_reboot_notifier(struct osal_notifier_block *ob)
{
    struct notifier_block *nb = NULL;
    struct nb_node *node = NULL;

    nb = kmalloc(sizeof(struct notifier_block), GFP_KERNEL);
    if (nb == NULL) {
        osal_trace("osal_register_reboot_notifier malloc nb failed!\n");
        return -1;
    }
    (void)memset_s(nb, sizeof(struct notifier_block), 0, sizeof(struct notifier_block));

    node = kmalloc(sizeof(struct nb_node), GFP_KERNEL);
    if (node == NULL) {
        osal_trace("osal_register_reboot_notifier kmalloc nb_node failed!\n");
        kfree(nb);
        return -1;
    }
    (void)memset_s(node, sizeof(struct nb_node), 0, sizeof(struct nb_node));

    nb->notifier_call = osal_notifier;
    register_reboot_notifier(nb);
    ob->notifier_block = nb;
    node->osal_nb = ob;
    node->nb = nb;
    osal_list_add(&(node->node), &nb_list);
    return 0;
}
EXPORT_SYMBOL(osal_register_reboot_notifier);

int osal_unregister_reboot_notifier(struct osal_notifier_block *nb)
{
    if (nb != NULL) {
        osal_del_nb(nb);
        unregister_reboot_notifier((struct notifier_block *)nb->notifier_block);
        kfree((struct notifier_block *)nb->notifier_block);
    }
    return 0;
}
EXPORT_SYMBOL(osal_unregister_reboot_notifier);
