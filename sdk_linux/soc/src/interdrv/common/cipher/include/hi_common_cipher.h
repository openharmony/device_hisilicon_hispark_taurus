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

#ifndef __HI_COMMON_CIPHER_H__
#define __HI_COMMON_CIPHER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define  HI_ERR_CIPHER_NOT_INIT                             0x804D0001
#define  HI_ERR_CIPHER_INVALID_HANDLE                       0x804D0002
#define  HI_ERR_CIPHER_INVALID_POINT                        0x804D0003
#define  HI_ERR_CIPHER_INVALID_PARA                         0x804D0004
#define  HI_ERR_CIPHER_FAILED_INIT                          0x804D0005
#define  HI_ERR_CIPHER_FAILED_GETHANDLE                     0x804D0006
#define  HI_ERR_CIPHER_FAILED_RELEASEHANDLE                 0x804D0007
#define  HI_ERR_CIPHER_FAILED_CONFIGAES                     0x804D0008
#define  HI_ERR_CIPHER_FAILED_CONFIGDES                     0x804D0009
#define  HI_ERR_CIPHER_FAILED_ENCRYPT                       0x804D000A
#define  HI_ERR_CIPHER_FAILED_DECRYPT                       0x804D000B
#define  HI_ERR_CIPHER_BUSY                                 0x804D000C
#define  HI_ERR_CIPHER_NO_AVAILABLE_RNG                     0x804D000D
#define  HI_ERR_CIPHER_FAILED_MEM                           0x804D000E
#define  HI_ERR_CIPHER_UNAVAILABLE                          0x804D000F
#define  HI_ERR_CIPHER_OVERFLOW                             0x804D0010
#define  HI_ERR_CIPHER_HARD_STATUS                          0x804D0011
#define  HI_ERR_CIPHER_TIMEOUT                              0x804D0012
#define  HI_ERR_CIPHER_UNSUPPORTED                          0x804D0013
#define  HI_ERR_CIPHER_REGISTER_IRQ                         0x804D0014
#define  HI_ERR_CIPHER_ILLEGAL_UUID                         0x804D0015
#define  HI_ERR_CIPHER_ILLEGAL_KEY                          0x804D0016
#define  HI_ERR_CIPHER_INVALID_ADDR                         0x804D0017
#define  HI_ERR_CIPHER_INVALID_LENGTH                       0x804D0018
#define  HI_ERR_CIPHER_ILLEGAL_DATA                         0x804D0019
#define  HI_ERR_CIPHER_RSA_SIGN                             0x804D001A
#define  HI_ERR_CIPHER_RSA_VERIFY                           0x804D001B
#define  HI_ERR_CIPHER_MEMSET_S_FAILED                      0x804D001C
#define  HI_ERR_CIPHER_MEMCPY_S_FAILED                      0x804D001D
#define  HI_ERR_CIPHER_RSA_CRYPT_FAILED                     0x804D001E

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_COMMON_CIPHER_H__ */
