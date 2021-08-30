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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include "securec.h"
#include "hi_osal.h"

#define OSAL_PROC_DEBUG 0

static struct osal_list_head g_list;
static osal_proc_entry_t *g_proc_entry = NULL;

static int osal_seq_show(struct seq_file *s, void *p)
{
    osal_proc_entry_t *oldsentry = s->private;
    osal_proc_entry_t sentry;

    osal_unused(p);
    if (oldsentry == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    (void)memset_s(&sentry, sizeof(osal_proc_entry_t), 0, sizeof(osal_proc_entry_t));
    /* only these two parameters are used */
    sentry.seqfile = s;
    sentry.private = oldsentry->private;
    oldsentry->read(&sentry);
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
static ssize_t osal_procwrite(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    osal_proc_entry_t *sentry = ((struct seq_file *)(file->private_data))->private;
    return sentry->write(sentry, (char *)buf, count, (long long *)ppos);
}
#else
static ssize_t osal_procwrite(struct file *file, const char __user *buf,
                              size_t count, loff_t *ppos)
{
    osal_proc_entry_t *item = PDE_DATA(file_inode(file));

    if ((item != NULL) && (item->write != NULL)) {
        return item->write(item, buf, count, (long long *)ppos);
    }

    return -ENOSYS;
}
#endif

static int osal_procopen(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    osal_proc_entry_t *sentry = PDE(inode)->data;
#else
    osal_proc_entry_t *sentry = PDE_DATA(inode);
#endif
    if ((sentry != NULL) && (sentry->open != NULL)) {
        sentry->open(sentry);
    }
    return single_open(file, osal_seq_show, sentry);
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,6,0)
static struct proc_ops g_osal_proc_ops = {
    .proc_open = osal_procopen,
    .proc_read = seq_read,
    .proc_write = osal_procwrite,
    .proc_lseek = seq_lseek,
    .proc_release = single_release
};
#else
static struct file_operations g_osal_proc_ops = {
    .owner = THIS_MODULE,
    .open = osal_procopen,
    .read = seq_read,
    .write = osal_procwrite,
    .llseek = seq_lseek,
    .release = single_release
};
#endif

osal_proc_entry_t *osal_create_proc(const char *name, osal_proc_entry_t *parent)
{
    struct proc_dir_entry *entry = NULL;
    osal_proc_entry_t *sentry = NULL;
    void *parent_proc_entry = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    if (parent == NULL) {
        osal_trace("%s - parent is NULL!\n", __FUNCTION__);
    } else {
        parent_proc_entry = parent->proc_dir_entry;
        osal_trace("%s - parent is not NULL! parent=%pK\n", __FUNCTION__, parent->proc_dir_entry);
    }
    entry = create_proc_entry(name, 0, parent_proc_entry);
    if (entry == NULL) {
        osal_trace("%s - create_proc_entry failed!\n", __FUNCTION__);
        return NULL;
    }
    sentry = kmalloc(sizeof(struct osal_proc_dir_entry), GFP_KERNEL);
    if (sentry == NULL) {
        remove_proc_entry(name, parent_proc_entry);
        osal_trace("%s - kmalloc failed!\n", __FUNCTION__);
        return NULL;
    }

    (void)memset_s(sentry, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));

    if (strncpy_s(sentry->name, OSAL_MAX_PROC_NAME_LEN, name, sizeof(sentry->name) - 1) != EOK) {
        kfree(sentry);
        remove_proc_entry(name, parent_proc_entry);
        osal_trace("%s - strncpy_s failed!\n", __FUNCTION__);
        return NULL;
    }
    sentry->proc_dir_entry = entry;
    sentry->open = NULL;
    entry->proc_fops = &g_osal_proc_ops;
    entry->data = sentry;
#else
    sentry = kmalloc(sizeof(struct osal_proc_dir_entry), GFP_KERNEL);
    if (sentry == NULL) {
        osal_trace("%s - kmalloc failed!\n", __FUNCTION__);
        return NULL;
    }

    (void)memset_s(sentry, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));

    if (strncpy_s(sentry->name, OSAL_MAX_PROC_NAME_LEN, name, sizeof(sentry->name) - 1) != EOK) {
        osal_trace("%s - strncpy_s failed!\n", __FUNCTION__);
        kfree(sentry);
        return NULL;
    }
    if (parent != NULL) {
        parent_proc_entry = parent->proc_dir_entry;
    }
    entry = proc_create_data(name, 0, parent_proc_entry, &g_osal_proc_ops, sentry);
    if (entry == NULL) {
        osal_trace("%s - create_proc_entry failed!\n", __FUNCTION__);
        kfree(sentry);
        return NULL;
    }
    sentry->proc_dir_entry = entry;
    sentry->open = NULL;
#endif
    osal_list_add_tail(&(sentry->node), &g_list);
    return sentry;
}

void osal_remove_proc(const char *name, osal_proc_entry_t *parent)
{
    struct osal_proc_dir_entry *sproc = NULL;

    if (name == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }
    if (parent != NULL) {
        remove_proc_entry(name, parent->proc_dir_entry);
    } else {
        remove_proc_entry(name, NULL);
    }
    osal_list_for_each_entry(sproc, &g_list, node) {
        if (osal_strncmp(sproc->name, name, sizeof(sproc->name)) == 0) {
            osal_list_del(&(sproc->node));
            kfree(sproc);
            sproc = NULL;
            break;
        }
    }
}

osal_proc_entry_t *osal_create_proc_entry(const char *name, osal_proc_entry_t *parent)
{
    parent = g_proc_entry;

    return osal_create_proc(name, parent);
}
EXPORT_SYMBOL(osal_create_proc_entry);

void osal_remove_proc_entry(const char *name, osal_proc_entry_t *parent)
{
    parent = g_proc_entry;
    osal_remove_proc(name, parent);
    return;
}
EXPORT_SYMBOL(osal_remove_proc_entry);

osal_proc_entry_t *osal_proc_mkdir(const char *name, osal_proc_entry_t *parent)
{
    struct proc_dir_entry *proc = NULL;
    struct osal_proc_dir_entry *sproc = NULL;
    void *parent_proc_entry = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    if (parent != NULL) {
        parent_proc_entry = parent->proc_dir_entry;
        proc = proc_mkdir(name, parent_proc_entry);
        osal_trace("%s - parent is not NULL!\n", __FUNCTION__);
    } else {
        proc = proc_mkdir(name, parent_proc_entry);
        osal_trace("%s - parent is NULL! proc=%pK \n", __FUNCTION__, proc);
    }
    if (proc == NULL) {
        osal_trace("%s - proc_mkdir failed!\n", __FUNCTION__);
        return NULL;
    }
    sproc = kmalloc(sizeof(struct osal_proc_dir_entry), GFP_KERNEL);
    if (sproc == NULL) {
        remove_proc_entry(name, parent_proc_entry);
        osal_trace("%s - kmalloc failed!\n", __FUNCTION__);
        return NULL;
    }

    (void)memset_s(sproc, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));

    if (strncpy_s(sproc->name, OSAL_MAX_PROC_NAME_LEN, name, sizeof(sproc->name) - 1) != EOK) {
        remove_proc_entry(name, parent_proc_entry);
        kfree(sproc);
        osal_trace("%s - strncpy_s failed!\n", __FUNCTION__);
        return NULL;
    }
    sproc->proc_dir_entry = proc;
    proc->data = sproc;
#else
    sproc = kmalloc(sizeof(struct osal_proc_dir_entry), GFP_KERNEL);
    if (sproc == NULL) {
        osal_trace("%s - kmalloc failed!\n", __FUNCTION__);
        return NULL;
    }

    (void)memset_s(sproc, sizeof(struct osal_proc_dir_entry), 0, sizeof(struct osal_proc_dir_entry));

    if (strncpy_s(sproc->name, OSAL_MAX_PROC_NAME_LEN, name, sizeof(sproc->name) - 1) != EOK) {
        kfree(sproc);
        osal_trace("%s - strncpy_s failed!\n", __FUNCTION__);
        return NULL;
    }
    if (parent != NULL) {
        parent_proc_entry = parent->proc_dir_entry;
    }
    proc = proc_mkdir_data(name, 0, parent_proc_entry, sproc);
    if (proc == NULL) {
        kfree(sproc);
        osal_trace("%s - proc_mkdir failed!\n", __FUNCTION__);
        return NULL;
    }
    sproc->proc_dir_entry = proc;
#endif
    osal_list_add_tail(&(sproc->node), &g_list);
    return sproc;
}
EXPORT_SYMBOL(osal_proc_mkdir);

void osal_remove_proc_root(const char *name, osal_proc_entry_t *parent)
{
    struct osal_proc_dir_entry *sproc = NULL;

    if (name == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }
    if (parent != NULL) {
        remove_proc_entry(name, parent->proc_dir_entry);
    } else {
        remove_proc_entry(name, NULL);
    }
    osal_list_for_each_entry(sproc, &g_list, node) {
        if (osal_strncmp(sproc->name, name, sizeof(sproc->name)) == 0) {
            osal_list_del(&(sproc->node));
            break;
        }
    }
    if (sproc != NULL) {
        kfree(sproc);
    }
}

void osal_seq_printf(osal_proc_entry_t *entry, const char *fmt, ...)
{
    struct seq_file *s = (struct seq_file *)(entry->seqfile);
    va_list args;

    va_start(args, fmt);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)
    (void)seq_vprintf(s, fmt, args);
#else
    seq_vprintf(s, fmt, args);
#endif
    va_end(args);
}
EXPORT_SYMBOL(osal_seq_printf);

void osal_proc_init(void)
{
    OSAL_INIT_LIST_HEAD(&g_list);
    g_proc_entry = osal_proc_mkdir("umap", NULL);
    if (g_proc_entry == NULL) {
        osal_trace("test init, proc mkdir error!\n");
    }
}
void osal_proc_exit(void)
{
    osal_remove_proc_root("umap", NULL);
}
