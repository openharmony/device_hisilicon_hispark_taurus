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

#ifndef __DRV_HASH_H__
#define __DRV_HASH_H__

#include "drv_osal_lib.h"

/* SHA1, SHA224, SHA256  hash block size */
#define HASH_BLOCK_SIZE_64         64
/* SHA384, SHA512  hash block size. */
#define HASH_BLOCK_SIZE_128        128

/* hash capacity, 0-nonsupport, 1-support */
typedef struct {
    hi_u32 sha1        : 1;    /* Support SHA1  */
    hi_u32 sha224      : 1;    /* Support SHA224  */
    hi_u32 sha256      : 1;    /* Support SHA256  */
    hi_u32 sha384      : 1;    /* Support SHA384  */
    hi_u32 sha512      : 1;    /* Support SHA512  */
    hi_u32 sm3         : 1;    /* Support SM3  */
} hash_capacity;

/* hash mode */
typedef enum {
    HASH_MODE_SHA1,      /* SHA1  */
    HASH_MODE_SHA224,    /* SHA2 224  */
    HASH_MODE_SHA256,    /* SHA2 256  */
    HASH_MODE_SHA384,    /* SHA2 384  */
    HASH_MODE_SHA512,    /* SHA2 512  */
    HASH_MODE_SM3,       /* SM3  */
    HASH_MODE_COUNT,
} hash_mode;

/* ****************************** API Declaration **************************** */
/*
 * \brief  Initialize the hash module.
 * \retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
 */
hi_s32 drv_hash_init(void);

/*
 * \brief  Deinitialize the hash module.
 * \retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
 */
hi_s32 drv_hash_deinit(void);

/*
 * \brief  suspend the hash module.
 * \retval     NA.
 */
void drv_hash_suspend(void);

/*
 * \brief  resume the hash module.
 * \retval     NA.
 */
void drv_hash_resume(void);

/*
 * \brief  set work params.
 * \param[in]  chn_num The channel number.
 * \param[in]  mode The hash mode.
 * \param[in] state The hash initial result, length is HASH_RESULT_MAX_SIZE_IN_WORD.
 * \retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
 */
hi_s32 drv_hash_cfg(hi_u32 chn_num, hash_mode mode, const hi_u32 *state);

/*
 * \brief  start hash calculation.
 * \param[in]  chn_num The channel number.
 * \param[in]  buf_phy The MMZ/SMMU address of in buffer.
 * \param[in]  buf_size The MMZ/SMMU size of in buffer.
 * \retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
 */
hi_s32 drv_hash_start(hi_u32 chn_num, const crypto_mem *mem, hi_u32 buf_size);

/*
 * \brief  wait running finished.
 * \param[in]  chn_num The channel number.
 * \param[out] state The hash result.
 * \param[in]  hashLen The length of hash result.
 * \retval     On received interception, HI_TRUE is returned  otherwise HI_FALSE is returned.
 */
hi_s32 drv_hash_wait_done(hi_u32 chn_num, hi_u32 *state);

/*
 * \brief  reset hash after hash finished.
 * \param[in]  chn_num The channel number.
 * \retval     NA.
 */
void drv_hash_reset(hi_u32 chn_num);

/*
 * \brief  get the hash capacity.
 * \param[out] capacity The hash capacity.
 * \retval     NA.
 */
void drv_hash_get_capacity(hash_capacity *capacity);
#endif
