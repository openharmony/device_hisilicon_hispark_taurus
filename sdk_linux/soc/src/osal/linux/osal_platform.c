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
#include <linux/platform_device.h>
#include <linux/of.h>
#include "securec.h"

int osal_platform_get_modparam_uint(void *pdev, const char *name, unsigned int *value)
{
    struct device_node *np = ((struct platform_device*)pdev)->dev.of_node;
    unsigned int temp_value;

    if (of_property_read_u32(np, name, &temp_value)) {
        return -1;
    }

    *value = temp_value;
    return 0;
}
EXPORT_SYMBOL(osal_platform_get_modparam_uint);

int osal_platform_get_modparam_int(void *pdev, const char *name, int *value)
{
    struct device_node *np = ((struct platform_device*)pdev)->dev.of_node;
    unsigned int temp_value;

    if (of_property_read_u32(np, name, &temp_value)) {
        return -1;
    }

    *value = (int)temp_value;
    return 0;
}
EXPORT_SYMBOL(osal_platform_get_modparam_int);

int osal_platform_get_modparam_uchar(void *pdev, const char *name, unsigned char *value)
{
    struct device_node *np = ((struct platform_device*)pdev)->dev.of_node;
    unsigned char temp_value;

    if (of_property_read_u8(np, name, &temp_value)) {
        return -1;
    }

    *value = temp_value;
    return 0;
}
EXPORT_SYMBOL(osal_platform_get_modparam_uchar);

int osal_platform_get_modparam_ushort(void *pdev, const char *name, unsigned short *value)
{
    struct device_node *np = ((struct platform_device*)pdev)->dev.of_node;
    unsigned short temp_value;

    if (of_property_read_u16(np, name, &temp_value)) {
        return -1;
    }

    *value = temp_value;
    return 0;
}
EXPORT_SYMBOL(osal_platform_get_modparam_ushort);

int osal_platform_get_modparam_string(void *pdev, const char *name, unsigned int size, char *value)
{
    struct device_node *np = ((struct platform_device*)pdev)->dev.of_node;
    const char *string = NULL;

    if (of_property_read_string(np, name, &string)) {
        return -1;
    }

    if (strncpy_s(value, size, string, size - 1) != EOK) {
        return -1;
    }
    return 0;
}
EXPORT_SYMBOL(osal_platform_get_modparam_string);

int osal_of_property_read_u32(const void *np, const char *propname, unsigned int *value)
{
    return of_property_read_u32((struct device_node*)np, propname, value);
}
EXPORT_SYMBOL(osal_of_property_read_u32);

int osal_platform_driver_register(void *drv)
{
    return __platform_driver_register((struct platform_driver *)drv, THIS_MODULE);
}
EXPORT_SYMBOL(osal_platform_driver_register);

void osal_platform_driver_unregister(void *drv)
{
    return platform_driver_unregister((struct platform_driver *)drv);
}
EXPORT_SYMBOL(osal_platform_driver_unregister);

void *osal_platform_get_resource_byname(void *dev, unsigned int type,
                                        const char *name)
{
    return (void *)platform_get_resource_byname((struct platform_device *)dev, type, name);
}
EXPORT_SYMBOL(osal_platform_get_resource_byname);

void *osal_platform_get_resource(void *dev, unsigned int type, unsigned int num)
{
    return (void *)platform_get_resource((struct platform_device *)dev, type, num);
}
EXPORT_SYMBOL(osal_platform_get_resource);

int osal_platform_get_irq(void *dev, unsigned int num)
{
    return platform_get_irq((struct platform_device *)dev, num);
}
EXPORT_SYMBOL(osal_platform_get_irq);

int osal_platform_get_irq_byname(void *dev, const char *name)
{
    return platform_get_irq_byname((struct platform_device *)dev, name);
}
EXPORT_SYMBOL(osal_platform_get_irq_byname);
