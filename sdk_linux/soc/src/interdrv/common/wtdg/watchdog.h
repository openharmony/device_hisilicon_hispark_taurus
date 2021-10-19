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

#ifndef _LINUX_WATCHDOG_H
#define _LINUX_WATCHDOG_H

#include <linux/types.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#define hi_wtdg_unused(x) (void)(x)

int watchdog_init(void);
void watchdog_exit(void);

#define    WATCHDOG_IOCTL_BASE    'W'

struct watchdog_info {
    unsigned int options;             /* Options the card/driver supports */
    unsigned int firmware_version;    /* Firmware version of the card */
    unsigned char identity[32];       /* 32 bit Identity of the board */
};

#define WDIOC_GETSUPPORT     _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define WDIOC_GETSTATUS      _IOR(WATCHDOG_IOCTL_BASE, 1, int)
#define WDIOC_GETBOOTSTATUS  _IOR(WATCHDOG_IOCTL_BASE, 2, int)

#define WDIOC_SETOPTIONS     _IOWR(WATCHDOG_IOCTL_BASE, 4, int)
#define WDIOC_KEEPALIVE      _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define WDIOC_SETTIMEOUT     _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT     _IOR(WATCHDOG_IOCTL_BASE, 7, int)

#define WDIOF_UNKNOWN        (-1)    /* Unknown flag error */
#define WDIOS_UNKNOWN        (-1)    /* Unknown status error */

#define WDIOF_OVERHEAT       0x0001    /* Reset due to CPU overheat */
#define WDIOF_FANFAULT       0x0002    /* Fan failed */
#define WDIOF_EXTERN1        0x0004    /* External relay 1 */
#define WDIOF_EXTERN2        0x0008    /* External relay 2 */
#define WDIOF_POWERUNDER     0x0010    /* Power bad/power fault */
#define WDIOF_CARDRESET      0x0020    /* Card previously reset the CPU */
#define WDIOF_POWEROVER      0x0040    /* Power over voltage */
#define WDIOF_SETTIMEOUT     0x0080    /* Set timeout (in seconds) */
#define WDIOF_MAGICCLOSE     0x0100    /* Supports magic close char */
#define WDIOF_PRETIMEOUT     0x0200    /* Pretimeout (in seconds), get/set */
#define WDIOF_KEEPALIVEPING  0x8000    /* Keep alive ping reply */

#define WDIOS_DISABLECARD    0x0001    /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD     0x0002    /* Turn on the watchdog timer */
#define WDIOS_TEMPPANIC      0x0004    /* Kernel panic on temperature trip */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
