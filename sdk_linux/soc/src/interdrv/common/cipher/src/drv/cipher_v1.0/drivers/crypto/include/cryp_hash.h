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

#ifndef __CRYP_HASH_H_
#define __CRYP_HASH_H_

#include "drv_cipher_kapi.h"
#include "drv_hash.h"

/* Initialize crypto of hash. */
hi_s32 cryp_hash_init(hi_void);

/* Deinitialize crypto of hash. */
hi_void cryp_hash_deinit(hi_void);

/*
 * \brief          Create hash handle
 *
 * \param mode     Hash mode
 * \return         ctx if successful, or NULL
 */
typedef hi_void *(*func_hash_create)(hash_mode mode);

/*
 * \brief          Clear hash context
 *
 * \param ctx      symc handle to be destroy
 */
typedef hi_s32 (*func_hash_destroy)(hi_void *ctx);

/*
 * \brief          Hash message chunk calculation
 *
 * Note: the message must be write to the buffer
 * which get from cryp_hash_get_cpu_addr, and the length of message chunk
 * can't large than the length which get from cryp_hash_get_cpu_addr.
 *
 * \param ctx      hash handle to be destroy
 * \param chunk    hash message to update
 * \param length   length of hash message
 * \param src      source of hash message
 */
typedef hi_s32 (*func_hash_update)(hi_void *ctx, const hi_u8 *chunk, hi_u32 chunk_len, hash_chunk_src src);

/*
 * \brief          HASH final digest
 *
 * \param ctx      Hash handle
 * \param hash     HASH checksum result
 * \param hashlen  Length of HASH checksum result
 */
typedef hi_s32 (*func_hash_finish)(hi_void *ctx,  hi_void *hash, hi_u32 hash_buf_len, hi_u32 *hashlen);

/* struct of Hash function template. */
typedef struct {
    hi_u32 valid;                  /* valid or not */
    hi_u32 mode;                   /* Mode of Hash */
    hi_u32 block_size;             /* block size */
    hi_u32 size;                   /* hash output size */
    func_hash_create  create;      /* Create function */
    func_hash_destroy destroy;     /* destroy function */
    func_hash_update  update;      /* update function */
    func_hash_finish  finish;      /* finish function */
} hash_func;

/*
 * \brief  Clone the function from template of hash engine.
 * \param[out]  func The struct of function.
 * \param[in]  mode The work mode.
 * \retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
 */
hash_func *cryp_get_hash(hash_mode mode);
#endif
