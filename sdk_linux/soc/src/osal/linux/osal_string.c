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

#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/version.h>

int osal_strcmp(const char *cs, const char *ct)
{
    return strcmp(cs, ct);
}
EXPORT_SYMBOL(osal_strcmp);
int osal_strncmp(const char *cs, const char *ct, int count)
{
    return strncmp(cs, ct, count);
}
EXPORT_SYMBOL(osal_strncmp);
int osal_strnicmp(const char *s1, const char *s2, int len)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    return strnicmp(s1, s2, len);
#else
    osal_unused(s1);
    osal_unused(s2);
    osal_unused(len);
#endif
    return 0;
}
EXPORT_SYMBOL(osal_strnicmp);
int osal_strcasecmp(const char *s1, const char *s2)
{
    return strcasecmp(s1, s2);
}
EXPORT_SYMBOL(osal_strcasecmp);
int osal_strncasecmp(const char *s1, const char *s2, int n)
{
    return strncasecmp(s1, s2, n);
}
EXPORT_SYMBOL(osal_strncasecmp);
char *osal_strchr(const char *s, int c)
{
    return strchr(s, c);
}
EXPORT_SYMBOL(osal_strchr);
char *osal_strnchr(const char *s, int count, int c)
{
    return strnchr(s, count, c);
}
EXPORT_SYMBOL(osal_strnchr);
char *osal_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}
EXPORT_SYMBOL(osal_strrchr);
char *osal_strstr(const char *s1, const char *s2)
{
    return strstr(s1, s2);
}
EXPORT_SYMBOL(osal_strstr);
char *osal_strnstr(const char *s1, const char *s2, int len)
{
    return strnstr(s1, s2, len);
}
EXPORT_SYMBOL(osal_strnstr);
int osal_strlen(const char *s)
{
    return strlen(s);
}
EXPORT_SYMBOL(osal_strlen);
int osal_strnlen(const char *s, int count)
{
    return strnlen(s, count);
}
EXPORT_SYMBOL(osal_strnlen);
char *osal_strpbrk(const char *cs, const char *ct)
{
    return strpbrk(cs, ct);
}
EXPORT_SYMBOL(osal_strpbrk);
char *osal_strsep(char **s, const char *ct)
{
    return strsep(s, ct);
}
EXPORT_SYMBOL(osal_strsep);
int osal_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}
EXPORT_SYMBOL(osal_strspn);
int osal_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}
EXPORT_SYMBOL(osal_strcspn);
void *osal_memscan(void *addr, int c, int size)
{
    return memscan(addr, c, size);
}
EXPORT_SYMBOL(osal_memscan);
int osal_memcmp(const void *cs, const void *ct, int count)
{
    return memcmp(cs, ct, count);
}
EXPORT_SYMBOL(osal_memcmp);
void *osal_memchr(const void *s, int c, int n)
{
    return memchr(s, c, n);
}
EXPORT_SYMBOL(osal_memchr);
void *osal_memchr_inv(const void *start, int c, int bytes)
{
    return memchr_inv(start, c, bytes);
}
EXPORT_SYMBOL(osal_memchr_inv);
unsigned long long osal_strtoull(const char *cp, char **endp, unsigned int base)
{
    return simple_strtoull(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoull);
unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base)
{
    return simple_strtoul(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoul);
long osal_strtol(const char *cp, char **endp, unsigned int base)
{
    return simple_strtol(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtol);
long long osal_strtoll(const char *cp, char **endp, unsigned int base)
{
    return simple_strtoll(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoll);

