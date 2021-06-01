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
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include "securec.h"
#include "base.h"

static void himedia_bus_release(struct device *dev)
{
    osal_unused(dev);
    return;
}

struct device g_himedia_bus = {
    .init_name = "himedia",
    .release = himedia_bus_release
};


/* bus match & uevent */
static int himedia_match(struct device *dev, struct device_driver *drv)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    return (strncmp(pdev->devfs_name, drv->name, sizeof(pdev->devfs_name)) == 0);
}

static int himedia_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    add_uevent_var(env, "MODALIAS=himedia:%s", pdev->devfs_name);
    return 0;
}

/* pm methods */
static int himedia_pm_prepare(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_prepare == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_prepare(pdev);
}

static void himedia_pm_complete(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_complete == NULL)) {
        return;
    }

    pdrv->ops->pm_complete(pdev);
}

static int himedia_pm_suspend(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_suspend == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_suspend(pdev);
}

static int himedia_pm_resume(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_resume == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_resume(pdev);
}

static int himedia_pm_freeze(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_freeze == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_freeze(pdev);
}

static int himedia_pm_thaw(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_thaw == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_thaw(pdev);
}

static int himedia_pm_poweroff(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_poweroff == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_poweroff(pdev);
}

static int himedia_pm_restore(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_restore == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_restore(pdev);
}
static int himedia_pm_suspend_noirq(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_suspend_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_suspend_noirq(pdev);
}

static int himedia_pm_resume_noirq(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_resume_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_resume_noirq(pdev);
}

static int himedia_pm_freeze_noirq(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_freeze_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_freeze_noirq(pdev);
}

static int himedia_pm_thaw_noirq(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_thaw_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_thaw_noirq(pdev);
}

static int himedia_pm_poweroff_noirq(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_poweroff_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_poweroff_noirq(pdev);
}

static int himedia_pm_restore_noirq(struct device *dev)
{
    struct himedia_device *pdev = to_himedia_device(dev);
    struct himedia_driver *pdrv = to_himedia_driver(dev->driver);

    if ((pdrv->ops == NULL) || (pdrv->ops->pm_restore_noirq == NULL)) {
        return 0;
    }

    return pdrv->ops->pm_restore_noirq(pdev);
}

static struct dev_pm_ops g_himedia_bus_pm_ops = {
    .prepare = himedia_pm_prepare,
    .complete = himedia_pm_complete,

    /* with irq */
    .suspend = himedia_pm_suspend,
    .resume = himedia_pm_resume,

    .freeze = himedia_pm_freeze,
    .thaw = himedia_pm_thaw,
    .poweroff = himedia_pm_poweroff,
    .restore = himedia_pm_restore,

    /* with noirq */
    .suspend_noirq = himedia_pm_suspend_noirq,
    .resume_noirq = himedia_pm_resume_noirq,
    .freeze_noirq = himedia_pm_freeze_noirq,
    .thaw_noirq = himedia_pm_thaw_noirq,
    .poweroff_noirq = himedia_pm_poweroff_noirq,
    .restore_noirq = himedia_pm_restore_noirq,
};

struct bus_type g_himedia_bus_type = {
    .name = "himedia",
    .match = himedia_match,
    .uevent = himedia_uevent,
    .pm = &g_himedia_bus_pm_ops,
};

int himedia_bus_init(void)
{
    int ret;
    ret = device_register(&g_himedia_bus);
    if (ret) {
        return ret;
    }

    ret = bus_register(&g_himedia_bus_type);
    if (ret) {
        goto error;
    }

    return 0;
error:
    device_unregister(&g_himedia_bus);
    return ret;
}

void himedia_bus_exit(void)
{
    bus_unregister(&g_himedia_bus_type);
    device_unregister(&g_himedia_bus);
}

static void himedia_device_release(struct device *dev)
{
    osal_unused(dev);
    return;
}

int himedia_device_register(struct himedia_device *pdev)
{
    dev_set_name(&pdev->device, "%s", pdev->devfs_name);

    pdev->device.devt = MKDEV(HIMEDIA_DEVICE_MAJOR, pdev->minor);
    pdev->device.release = himedia_device_release;
    pdev->device.bus = &g_himedia_bus_type;

    return device_register(&pdev->device);
}

void himedia_device_unregister(struct himedia_device *pdev)
{
    device_unregister(&pdev->device);
}

struct himedia_driver *himedia_driver_register(const char *name,
                                               struct module *owner, struct himedia_ops *ops)
{
    int ret;
    struct himedia_driver *pdrv = NULL;

    if ((name == NULL) || (owner == NULL)) {
        return ERR_PTR(-EINVAL);
    }

    pdrv = kzalloc(sizeof(struct himedia_driver) + strnlen(name, HIMIDIA_MAX_DEV_NAME_LEN), GFP_KERNEL);
    if (pdrv == NULL) {
        return ERR_PTR(-ENOMEM);
    }

    /* init driver object */
    if (strncpy_s(pdrv->name, HIMIDIA_MAX_DEV_NAME_LEN, name, strnlen(name, HIMIDIA_MAX_DEV_NAME_LEN - 1)) != EOK) {
        kfree(pdrv);
        pdrv = NULL;
        return ERR_PTR(-EINVAL);
    }
    pdrv->name[HIMIDIA_MAX_DEV_NAME_LEN - 1] = '\0';

    pdrv->ops = ops;

    pdrv->driver.name = pdrv->name;
    pdrv->driver.owner = owner;
    pdrv->driver.bus = &g_himedia_bus_type;

    ret = driver_register(&pdrv->driver);
    if (ret) {
        kfree(pdrv);
        pdrv = NULL;
        return ERR_PTR(ret);
    }

    return pdrv;
}

void himedia_driver_unregister(struct himedia_driver *pdrv)
{
    if (pdrv != NULL) {
        driver_unregister(&pdrv->driver);
        kfree(pdrv);
        pdrv = NULL;
    }
}
