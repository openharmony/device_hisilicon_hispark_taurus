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

#ifndef __DRV_CIPHER_DEFINE_H__
#define __DRV_CIPHER_DEFINE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* word index number. */
#define WORD_IDX_0                                 0
#define WORD_IDX_1                                 1
#define WORD_IDX_2                                 2
#define WORD_IDX_3                                 3
#define WORD_IDX_4                                 4
#define WORD_IDX_5                                 5
#define WORD_IDX_6                                 6
#define WORD_IDX_7                                 7

/* Boundary value 1. */
#define BOUND_VAL_1                                1

/* multiple value */
#define MUL_VAL_1                                  1
#define MUL_VAL_2                                  2
#define MUL_VAL_3                                  3
#define MUL_VAL_4                                  4

#define SHIFT_1BITS                                1
#define SHIFT_2BITS                                2
#define SHIFT_3BITS                                3
#define SHIFT_4BITS                                4
#define SHIFT_5BITS                                5
#define SHIFT_6BITS                                6
#define SHIFT_8BITS                                8
#define SHIFT_9BITS                                9
#define SHIFT_13BITS                               13
#define SHIFT_15BITS                               15
#define SHIFT_16BITS                               16
#define SHIFT_17BITS                               17
#define SHIFT_21BITS                               21
#define SHIFT_23BITS                               23
#define SHIFT_24BITS                               24
#define SHIFT_29BITS                               29
#define SHIFT_32BITS                               32

#define MAX_LOW_2BITS                              3
#define MAX_LOW_3BITS                              7
#define MAX_LOW_4BITS                              0xF
#define MAX_LOW_8BITS                              0xFF

#define WORD_WIDTH                                 4

/* width of word. */
#define WORD_BIT_WIDTH                             32
#define U32_MAX_SIZE                               0xFFFFFFFF

/* width of double word. */
#define DOUBLE_WORD_WIDTH                          8

#define BYTE_BITS                                  8
/* The offset in one byte. */
#define BYTE_2BIT                                  2
#define BYTE_4BIT                                  4
#define BYTE_6BIT                                  6

#define CRYPTO_NUM_1                               1
#define CRYPTO_NUM_2                               2
#define CRYPTO_NUM_3                               3

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DRV_CIPHER_DEFINE_H__ */

