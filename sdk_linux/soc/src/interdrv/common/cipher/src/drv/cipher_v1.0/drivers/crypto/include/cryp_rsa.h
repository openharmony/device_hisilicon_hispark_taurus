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

#ifndef __CRYP_RSA_H_
#define __CRYP_RSA_H_

#include "drv_cipher_kapi.h"
#include "drv_osal_lib.h"
#include "drv_srsa.h"

#define CRYP_TRNG_TIMEOUT  (-1)

typedef enum {
    RSA_BLOCK_TYPE_0 = 0X0,
    RSA_BLOCK_TYPE_1,
    RSA_BLOCK_TYPE_2,
} rsa_pkcs1_padding_type;

typedef struct {
    hi_u32 mode;
    hi_u32 klen;
    hi_u8 *in;
    hi_u32 in_len;
    hi_u8 *out;
    hi_u32 *out_len;
    hi_u8 bt;
} rsa_padding_pack;

/*
 * brief          Initialize crypto of rsa *
 */
hi_s32 cryp_rsa_init(hi_void);

/*
 * brief          Deinitialize crypto of rsa *
 */
hi_void cryp_rsa_deinit(hi_void);

/*
 * brief RSA encryption a plaintext with a RSA key.
 * param[in] key:         rsa key.
 * param[in] rsa_crypt:   rsa encrypt data.
 * retval ::HI_SUCCESS  Call this API successful.
 * retval ::HI_FAILURE  Call this API fails.
 * see \n
 * N/A
 */
hi_s32 cryp_rsa_encrypt(const cryp_rsa_key *key, cryp_rsa_crypt_data *rsa_crypt);

/*
 * brief RSA decryption a plaintext with a RSA key.
 * param[in] key:         rsa key.
 * param[in] rsa_crypt:   rsa decrypt data.
 * retval ::HI_SUCCESS  Call this API successful.
 * retval ::HI_FAILURE  Call this API fails.
 * see \n
 * N/A
 */
hi_s32 cryp_rsa_decrypt(const cryp_rsa_key *key, cryp_rsa_crypt_data *rsa_crypt);

/*
 * brief RSA sign a hash value with a RSA private key.
 * param[in] key:         rsa key.
 * param[in] rsa_sign:    rsa sign data.
 * retval ::HI_SUCCESS  Call this API successful.
 * retval ::HI_FAILURE  Call this API fails.
 * see \n
 * N/A
 */
hi_s32 cryp_rsa_sign_hash(const cryp_rsa_key *key, cryp_rsa_sign_data *rsa_sign);

/*
 * brief RSA verify a hash value with a RSA public key.
 * param[in] key:         rsa key.
 * param[in] rsa_verify:  rsa sign data.
 * retval ::HI_SUCCESS  Call this API successful.
 * retval ::HI_FAILURE  Call this API fails.
 * see \n
 * N/A
 */
hi_s32 cryp_rsa_verify_hash(const cryp_rsa_key *key, const cryp_rsa_sign_data *rsa_verify);

hi_s32 cryp_rsa_bn_exp_mod(cryp_rsa_exp_mod *exp_mod);

/*
 * brief Generate random.
 * N/A
 */
int mbedtls_get_random(hi_void *param, hi_u8 *rand, size_t size);
#endif
