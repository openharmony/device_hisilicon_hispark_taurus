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

#include "hi_cipher_compat.h"
#include "hi_drv_compat.h"
#include "cryp_symc.h"
#include "securec.h"

/* max package numher of symc multi encrypt */
#define SYMC_MULTI_MAX_PKG      0x1000

#define RSA_PUBLIC_BUFFER_NUM   0x03
#define RSA_PRIVATE_BUFFER_NUM  0x07

#define MAX_CENC_SUB_SAMPLE     100

typedef hi_s32 (*hi_drv_func)(hi_void *param);

typedef struct {
    const char *name;
    hi_drv_func func;
    hi_u32 cmd;
} crypto_dispatch_func;

typedef struct {
    hi_u8 *buf;
    hi_u32 buf_size;
    hi_u32 offset;
} kapi_rsa_buf;

/* ****************************** API Code **************************** */
static hi_s32 dispatch_symc_create_handle(hi_void *argp)
{
    hi_s32 ret;
    symc_create_t *symc_create = argp;

    hi_log_func_enter();

    /* allocate a aes channel */
    ret = kapi_symc_create(&symc_create->id);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_create, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_symc_destroy_handle(hi_void *argp)
{
    hi_s32 ret;
    symc_destroy_t *destroy = argp;

    hi_log_func_enter();

    ret = kapi_symc_destroy(destroy->id);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_destroy, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_symc_cfg(hi_void *argp)
{
    hi_s32 ret;
    symc_cfg_t *cfg = argp;

    hi_log_func_enter();

    ret = kapi_symc_cfg(cfg);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_cfg, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_symc_encrypt(hi_void *argp)
{
    hi_s32 ret;
    symc_encrypt_t *encrypt = argp;

    hi_log_func_enter();

    if ((encrypt->operation == SYMC_OPERATION_ENCRYPT) || (encrypt->operation == SYMC_OPERATION_DECRYPT)) {
        ret = cipher_check_mmz_phy_addr((hi_phys_addr_t)addr_u64(encrypt->in), encrypt->len);
        if (ret != HI_SUCCESS) {
            hi_log_print_func_err(cipher_check_mmz_phy_addr, ret);
            return ret;
        }

        ret = cipher_check_mmz_phy_addr((hi_phys_addr_t)addr_u64(encrypt->out), encrypt->len);
        if (ret != HI_SUCCESS) {
            hi_log_print_func_err(cipher_check_mmz_phy_addr, ret);
            return ret;
        }

        ret = kapi_symc_crypto(encrypt);
        if (ret != HI_SUCCESS) {
            hi_log_print_func_err(kapi_symc_crypto, ret);
            return ret;
        }
    } else if ((encrypt->operation == SYMC_OPERATION_ENCRYPT_VIA) ||
        (encrypt->operation == SYMC_OPERATION_DECRYPT_VIA)) {
        ret = kapi_symc_crypto_via(encrypt, HI_TRUE);
        if (ret != HI_SUCCESS) {
            hi_log_print_func_err(kapi_symc_crypto_via, ret);
            return ret;
        }
    } else {
        hi_log_error("encrypt operation(0x%x) is unsupported.\n", encrypt->operation);
        return HI_ERR_CIPHER_UNSUPPORTED;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_symc_encrypt_multi(hi_void *argp)
{
    hi_s32 ret;
    symc_encrypt_multi_t *encrypt_multi = argp;

    hi_log_func_enter();

    hi_log_debug("operation %u\n", encrypt_multi->operation);
    ret = kapi_symc_crypto_multi(encrypt_multi->id,
        addr_via(encrypt_multi->pack), encrypt_multi->pack_num, encrypt_multi->operation, HI_TRUE);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_crypto_multi, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_symc_get_tag(hi_void *argp)
{
    hi_s32 ret;
    aead_tag_t *aead_tag = argp;

    hi_log_func_enter();

    ret = kapi_aead_get_tag(aead_tag->id, aead_tag->tag, &aead_tag->taglen);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_aead_get_tag, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_symc_get_cfg(hi_void *argp)
{
    hi_s32 ret;
    symc_get_cfg_t *get_cfg = argp;

    hi_log_func_enter();

    ret = kapi_symc_get_cfg(get_cfg->id, &get_cfg->ctrl);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_get_cfg, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_klad_key(hi_void *argp)
{
    hi_s32 ret;
    klad_key_t *klad = argp;

    hi_log_func_enter();

    ret = kapi_symc_klad_encrypt_key(klad->keysel,
        klad->target, klad->clear, klad->encrypt, sizeof(klad->clear));
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_klad_encrypt_key, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_hash_start(hi_void *argp)
{
    hi_s32 ret;
    hash_start_t *start = argp;
    hi_u8 *key = HI_NULL;

    hi_log_func_enter();
    hi_log_chk_param_return(start->type >= HI_CIPHER_HASH_TYPE_BUTT);

    if (start->type == HI_CIPHER_HASH_TYPE_SM3) {
        hi_log_error("Sm3 is unsupported.\n");
        return HI_ERR_CIPHER_UNSUPPORTED;
    }

    if (start->type == HI_CIPHER_HASH_TYPE_HMAC_SHA1 ||
        start->type == HI_CIPHER_HASH_TYPE_HMAC_SHA224 ||
        start->type == HI_CIPHER_HASH_TYPE_HMAC_SHA256 ||
        start->type == HI_CIPHER_HASH_TYPE_HMAC_SHA384 ||
        start->type == HI_CIPHER_HASH_TYPE_HMAC_SHA512) {
        hi_log_chk_param_return(start->keylen > MAX_MALLOC_BUF_SIZE);
        hi_log_chk_param_return(addr_via(start->key) == HI_NULL);

        key = (hi_u8 *)crypto_calloc(1, start->keylen);
        if (key == HI_NULL) {
            hi_log_print_err_code(HI_ERR_CIPHER_FAILED_MEM);
            hi_log_print_func_err(crypto_calloc, HI_ERR_CIPHER_FAILED_MEM);
            return HI_ERR_CIPHER_FAILED_MEM;
        }

        crypto_chk_err_goto(crypto_copy_from_user(key, start->keylen, addr_via(start->key), start->keylen));
    }

    crypto_chk_err_goto(kapi_hash_start(&start->id, start->type, key, start->keylen));

exit__:
    if (key != HI_NULL) {
        (hi_void)memset_s(key, start->keylen, 0, start->keylen);
        crypto_free(key);
        key = HI_NULL;
    }

    hi_log_func_exit();

    return ret;
}

static hi_s32 dispatch_hash_update(hi_void *argp)
{
    hi_s32 ret;
    hash_update_t *update = argp;

    hi_log_func_enter();

    hi_log_chk_param_return(addr_via(update->input) == HI_NULL);

    update->src = HASH_CHUNCK_SRC_USER;
    ret = kapi_hash_update(update->id, addr_via(update->input), update->length, update->src);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_hash_update, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_hash_finish(hi_void *argp)
{
    hi_s32 ret;
    hash_finish_t *finish = argp;

    hi_log_func_enter();

    ret = kapi_hash_finish(finish->id, (hi_u8 *)finish->hash, sizeof(finish->hash), &finish->hashlen);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_hash_finish, ret);
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 rsa_buf_chk_info_param(const rsa_info_t *rsa_info)
{
    hi_log_chk_param_return(rsa_info == HI_NULL);
    hi_log_chk_param_return(addr_via(rsa_info->in) == HI_NULL);
    hi_log_chk_param_return(addr_via(rsa_info->out) == HI_NULL);

    if (rsa_info->public == HI_FALSE) {
        hi_log_chk_param_return((addr_via(rsa_info->d) == HI_NULL) &&
            ((addr_via(rsa_info->p) == HI_NULL) ||
             (addr_via(rsa_info->q) == HI_NULL)  ||
             (addr_via(rsa_info->dp) == HI_NULL) ||
             (addr_via(rsa_info->dq) == HI_NULL) ||
             (addr_via(rsa_info->qp) == HI_NULL)));
    }

    hi_log_chk_param_return(rsa_info->inlen > rsa_info->klen);
    hi_log_chk_param_return(rsa_info->outlen > rsa_info->klen);
    hi_log_chk_param_return(rsa_info->klen < RSA_KEY_BITWIDTH_1024);
    hi_log_chk_param_return(rsa_info->klen > RSA_KEY_BITWIDTH_4096);

    return HI_SUCCESS;
}

static hi_s32 rsa_pub_alloc(cryp_rsa_key *key, const rsa_info_t *rsa_info, hi_u8 **in, hi_u8 **out)
{
    hi_s32 ret;
    hi_u32 size;
    hi_u8 *buf = HI_NULL;
    hi_u32 klen = rsa_info->klen;

    hi_log_chk_param_return(key->ca_type != HI_CIPHER_KEY_SRC_USER);

    /* buffer size of key, input and output */
    size = rsa_info->klen * RSA_PUBLIC_BUFFER_NUM;
    buf = crypto_calloc(1, size);
    if (buf == HI_NULL) {
        hi_log_print_func_err(crypto_calloc, HI_ERR_CIPHER_FAILED_MEM);
        return HI_ERR_CIPHER_FAILED_MEM;
    }

    key->n = buf;
    buf += klen;
    *in  = buf;
    buf += klen;
    *out = buf;
    buf += klen;
    key->bufsize = size;

    crypto_chk_err_goto(crypto_copy_from_user(key->n, klen, addr_via(rsa_info->n), klen));
    crypto_chk_err_goto(crypto_copy_from_user(*in, klen, addr_via(rsa_info->in), klen));
    key->e = rsa_info->e;

    return HI_SUCCESS;

exit__:
    if (key->n != HI_NULL) {
        (hi_void)memset_s(key->n, key->bufsize, 0, key->bufsize);
        crypto_free(key->n);
        key->n = HI_NULL;
    }

    hi_log_error("error, copy rsa key from user failed\n");
    hi_log_print_err_code(HI_ERR_CIPHER_FAILED_MEM);
    return HI_ERR_CIPHER_FAILED_MEM;
}

static hi_void rsa_private_set_key_param(cryp_rsa_key *key, kapi_rsa_buf *rsa_buf, hi_u32 klen, hi_u32 e)
{
    hi_u8 *ptr = rsa_buf->buf;

    key->n  = ptr;
    ptr += klen;
    rsa_buf->offset += klen;

    key->d  = ptr;
    ptr += klen;
    rsa_buf->offset += klen;

    key->p  = ptr;
    ptr += klen / MUL_VAL_2;
    rsa_buf->offset += klen / MUL_VAL_2;

    key->q  = ptr;
    ptr += klen / MUL_VAL_2;
    rsa_buf->offset += klen / MUL_VAL_2;

    key->dp = ptr;
    ptr += klen / MUL_VAL_2;
    rsa_buf->offset += klen / MUL_VAL_2;

    key->dq = ptr;
    ptr += klen / MUL_VAL_2;
    rsa_buf->offset += klen / MUL_VAL_2;

    key->qp = ptr;
    ptr += klen / MUL_VAL_2;
    rsa_buf->offset += klen / MUL_VAL_2;

    key->e = e;
    key->bufsize = rsa_buf->buf_size;
}

static hi_s32 rsa_private_get_cfg(cryp_rsa_key *key,
    const rsa_info_t *rsa_info, hi_u8 **in, hi_u8 **out, hi_u8 *buf)
{
    hi_s32 ret;
    hi_u8 *ptr = buf;
    hi_u32 klen = rsa_info->klen;

    if (addr_via(rsa_info->n) != HI_NULL) {
        /* invalid user n, n is even number. */
        crypto_chk_err_goto(crypto_copy_from_user(key->n, klen, addr_via(rsa_info->n), klen));
        if ((key->ca_type == HI_CIPHER_KEY_SRC_USER) && ((key->n[klen - BOUND_VAL_1] & CRYPTO_NUM_1) == 0)) {
            hi_log_error("invalid n, n is even number.\n");
            goto exit__;
        }
    }

    if (addr_via(rsa_info->d) != HI_NULL) {
        crypto_chk_err_goto(crypto_copy_from_user(key->d, klen, addr_via(rsa_info->d), klen));
    } else {
        crypto_chk_err_goto(crypto_copy_from_user(
            key->p, klen / MUL_VAL_2, addr_via(rsa_info->p), klen / MUL_VAL_2));
        crypto_chk_err_goto(crypto_copy_from_user(
            key->q, klen / MUL_VAL_2, addr_via(rsa_info->q), klen / MUL_VAL_2));
        crypto_chk_err_goto(crypto_copy_from_user(
            key->dp, klen / MUL_VAL_2, addr_via(rsa_info->dp), klen / MUL_VAL_2));
        crypto_chk_err_goto(crypto_copy_from_user(
            key->dq, klen / MUL_VAL_2, addr_via(rsa_info->dq), klen / MUL_VAL_2));
        crypto_chk_err_goto(crypto_copy_from_user(
            key->qp, klen / MUL_VAL_2, addr_via(rsa_info->qp), klen / MUL_VAL_2));
        key->d = HI_NULL;
    }

    *in  = ptr;
    ptr += klen;
    *out = ptr;
    ptr += klen;

    if (addr_via(rsa_info->in) != HI_NULL) {
        crypto_chk_err_goto(crypto_copy_from_user(*in, klen, addr_via(rsa_info->in), rsa_info->inlen));
    }

    return HI_SUCCESS;
exit__:
    return ret;
}

static hi_s32 rsa_private_alloc(cryp_rsa_key *key, const rsa_info_t *rsa_info, hi_u8 **in, hi_u8 **out)
{
    hi_s32 ret;
    hi_u32 size;
    hi_u8 *ptr = HI_NULL;
    hi_u8 *buf = HI_NULL;
    hi_u32 klen = rsa_info->klen;
    kapi_rsa_buf rsa_buf;

    /* n + d or n + p + q + dP + dQ + qp
     * the length of n/d is klen,
     * the length of p/q/dP/dQ/qp is klen/2,
     * the length of input is klen
     * the length of output is klen
     */
    hi_log_chk_param_return(key->ca_type > HI_CIPHER_KEY_SRC_BUTT);
    (hi_void)memset_s(&rsa_buf, sizeof(rsa_buf), 0, sizeof(rsa_buf));

    size = klen * RSA_PRIVATE_BUFFER_NUM;
    buf = crypto_calloc(MUL_VAL_1, size);
    if (buf == HI_NULL) {
        hi_log_print_func_err(crypto_calloc, HI_ERR_CIPHER_FAILED_MEM);
        return HI_ERR_CIPHER_FAILED_MEM;
    }
    ptr = buf;

    rsa_buf.buf = buf;
    rsa_buf.buf_size = size;
    rsa_private_set_key_param(key, &rsa_buf, klen, rsa_info->e);

    ptr += rsa_buf.offset; /* set buf for in and out */
    crypto_chk_err_goto(rsa_private_get_cfg(key, rsa_info, in, out, ptr));
    return HI_SUCCESS;

exit__:
    if (buf != HI_NULL) {
        (hi_void)memset_s(buf, size, 0, size);
        crypto_free(buf);
        buf = HI_NULL;
    }

    hi_log_error("error, copy rsa key from user failed\n");
    hi_log_print_err_code(HI_ERR_CIPHER_FAILED_MEM);
    return HI_ERR_CIPHER_FAILED_MEM;
}

static hi_s32 rsa_alloc_buffer(cryp_rsa_key *key, const rsa_info_t *rsa_info, hi_u8 **in, hi_u8 **out)
{
    hi_s32 ret;
    hi_u32 klen;

    hi_log_func_enter();

    ret = rsa_buf_chk_info_param(rsa_info);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(rsa_buf_chk_info_param, ret);
        return ret;
    }

    (hi_void)memset_s(key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    klen = rsa_info->klen;
    key->klen = klen;
    key->public = rsa_info->public;
    key->ca_type = rsa_info->ca_type;

    if (rsa_info->public) {
        ret = rsa_pub_alloc(key, rsa_info, in, out);
        if (ret != HI_SUCCESS) {
            hi_log_print_func_err(rsa_pub_alloc, ret);
            return ret;
        }
    } else {
        ret = rsa_private_alloc(key, rsa_info, in, out);
        if (ret != HI_SUCCESS) {
            hi_log_print_func_err(rsa_private_alloc, ret);
            return ret;
        }
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_void rsa_free_buffer(cryp_rsa_key *key)
{
    hi_log_func_enter();

    if (key->n != HI_NULL) {
        (hi_void)memset_s(key->n, key->bufsize, 0, key->bufsize);
        crypto_free(key->n);
        key->n = HI_NULL;
    }

    hi_log_func_exit();
    return;
}

static hi_s32 dispatch_rsa_encrypt(hi_void *argp)
{
    hi_s32 ret;
    rsa_info_t *rsa_info = (rsa_info_t *)argp;
    cryp_rsa_key key;
    cryp_rsa_crypt_data rsa;

    hi_log_func_enter();

    (hi_void)memset_s(&rsa, sizeof(rsa), 0, sizeof(rsa));
    (hi_void)memset_s(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &rsa.in, &rsa.out);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(rsa_alloc_buffer, ret);
        return ret;
    }
    rsa.in_len  = rsa_info->inlen;
    rsa.out_len = rsa_info->outlen;
    rsa.scheme  = rsa_info->scheme;

    ret = kapi_rsa_encrypt(&key, &rsa);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(kapi_rsa_encrypt, ret);
        return ret;
    }

    ret = crypto_copy_to_user(addr_via(rsa_info->out), rsa_info->outlen, rsa.out, rsa.out_len);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(crypto_copy_to_user, ret);
        return ret;
    }

    rsa_info->outlen = rsa.out_len;
    rsa_free_buffer(&key);
    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_rsa_decrypt(hi_void *argp)
{
    hi_s32 ret;
    rsa_info_t *rsa_info = (rsa_info_t *)argp;
    cryp_rsa_key key;
    cryp_rsa_crypt_data rsa;

    hi_log_func_enter();

    (hi_void)memset_s(&rsa, sizeof(rsa), 0, sizeof(rsa));
    (hi_void)memset_s(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &rsa.in, &rsa.out);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(rsa_alloc_buffer, ret);
        return ret;
    }
    rsa.in_len  = rsa_info->inlen;
    rsa.out_len = rsa_info->outlen;
    rsa.scheme  = rsa_info->scheme;

    ret = kapi_rsa_decrypt(&key, &rsa);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(kapi_rsa_decrypt, ret);
        return ret;
    }

    ret = crypto_copy_to_user(addr_via(rsa_info->out), rsa_info->outlen, rsa.out, rsa.out_len);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(crypto_copy_to_user, ret);
        return ret;
    }

    rsa_info->outlen = rsa.out_len;
    rsa_free_buffer(&key);
    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_rsa_sign_hash(hi_void *argp)
{
    hi_s32 ret;
    rsa_info_t *rsa_info = (rsa_info_t *)argp;
    cryp_rsa_key key;
    cryp_rsa_sign_data rsa;

    hi_log_func_enter();

    (hi_void)memset_s(&rsa, sizeof(rsa), 0, sizeof(rsa));
    (hi_void)memset_s(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &rsa.in, &rsa.out);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(rsa_alloc_buffer, ret);
        return ret;
    }
    rsa.in_len  = rsa_info->inlen;
    rsa.out_len = rsa_info->outlen;
    rsa.scheme  = rsa_info->scheme;

    ret = kapi_rsa_sign_hash(&key, &rsa);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(kapi_rsa_sign_hash, ret);
        return ret;
    }

    ret = crypto_copy_to_user(addr_via(rsa_info->out), rsa_info->outlen, rsa.out, rsa.out_len);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(crypto_copy_to_user, ret);
        return ret;
    }

    rsa_info->outlen = rsa.out_len;
    rsa_free_buffer(&key);
    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 dispatch_rsa_verify_hash(hi_void *argp)
{
    hi_s32 ret;
    rsa_info_t *rsa_info = (rsa_info_t *)argp;
    cryp_rsa_key key;
    cryp_rsa_sign_data rsa;

    hi_log_func_enter();

    (hi_void)memset_s(&rsa, sizeof(rsa), 0, sizeof(rsa));
    (hi_void)memset_s(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &rsa.in, &rsa.out);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(rsa_alloc_buffer, ret);
        return ret;
    }
    rsa.in_len  = rsa_info->inlen;
    rsa.out_len = rsa_info->outlen;
    rsa.scheme  = rsa_info->scheme;

    /* copy hash value from user */
    ret = crypto_copy_from_user(rsa.out, rsa.out_len, addr_via(rsa_info->out), rsa_info->outlen);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(crypto_copy_from_user, ret);
        return ret;
    }

    ret = kapi_rsa_verify_hash(&key, &rsa);
    if (ret != HI_SUCCESS) {
        rsa_free_buffer(&key);
        hi_log_print_func_err(kapi_rsa_verify_hash, ret);
        return ret;
    }

    rsa_free_buffer(&key);
    hi_log_func_exit();
    return HI_SUCCESS;
}

static hi_s32 rsa_exp_mod_alloc(cryp_rsa_exp_mod *cryp_mod, const rsa_bn_exp_mod_t *exp_mod)
{
    hi_s32 ret;
    hi_u8 *buf;
    cryp_mod->length = exp_mod->length;

    buf = crypto_calloc(MUL_VAL_4, cryp_mod->length); /* n, k, in, out, 4 groups */
    if (buf == HI_NULL) {
        hi_log_print_func_err(crypto_calloc, HI_ERR_CIPHER_FAILED_MEM);
        return HI_ERR_CIPHER_FAILED_MEM;
    }

    cryp_mod->n = buf;
    cryp_mod->k = buf + MUL_VAL_1 * cryp_mod->length;
    cryp_mod->in = buf + MUL_VAL_2 * cryp_mod->length;
    cryp_mod->out = buf + MUL_VAL_3 * cryp_mod->length;

    crypto_chk_err_goto(crypto_copy_from_user(cryp_mod->n, cryp_mod->length,
        addr_via_const(exp_mod->n), exp_mod->length));

    crypto_chk_err_goto(crypto_copy_from_user(cryp_mod->k, cryp_mod->length,
        addr_via_const(exp_mod->k), exp_mod->length));

    crypto_chk_err_goto(crypto_copy_from_user(cryp_mod->in, cryp_mod->length,
        addr_via_const(exp_mod->in), exp_mod->length));

    return HI_SUCCESS;
exit__:

    crypto_free(buf);
    return ret;
}

static hi_void rsa_exp_mod_free(cryp_rsa_exp_mod *cryp_mod)
{
    if (cryp_mod != HI_NULL) {
        crypto_free(cryp_mod->n);
        cryp_mod->n = HI_NULL;
        cryp_mod->k = HI_NULL;
        cryp_mod->in = HI_NULL;
        cryp_mod->out = HI_NULL;
    }
}

static hi_s32 dispatch_rsa_bn_exp_mod(hi_void *argp)
{
    hi_s32 ret;
    rsa_bn_exp_mod_t *exp_mod = (rsa_bn_exp_mod_t *)argp;
    cryp_rsa_exp_mod cryp_mod;

    (hi_void)memset_s(&cryp_mod, sizeof(cryp_rsa_exp_mod), 0, sizeof(cryp_rsa_exp_mod));

    hi_log_chk_param_return(exp_mod == HI_NULL);
    hi_log_chk_param_return(addr_via_const(exp_mod->n) == HI_NULL);
    hi_log_chk_param_return(addr_via_const(exp_mod->k) == HI_NULL);
    hi_log_chk_param_return(addr_via_const(exp_mod->in) == HI_NULL);
    hi_log_chk_param_return(addr_via(exp_mod->out) == HI_NULL);
    hi_log_chk_param_return(exp_mod->length < RSA_KEY_BITWIDTH_1024);
    hi_log_chk_param_return(exp_mod->length > RSA_KEY_BITWIDTH_4096);

    ret = rsa_exp_mod_alloc(&cryp_mod, exp_mod);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(rsa_exp_mod_alloc, ret);
        return ret;
    }

    ret = kapi_rsa_bn_exp_mod(&cryp_mod);
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_rsa_bn_exp_mod, ret);
        goto exit__;
    }

    crypto_chk_err_goto(crypto_copy_to_user(addr_via(exp_mod->out), cryp_mod.length,
        cryp_mod.out, cryp_mod.length));

exit__:
    rsa_exp_mod_free(&cryp_mod);
    return ret;
}

static hi_s32 dispatch_trng_get_random(hi_void *argp)
{
    trng_t *trng = (trng_t *)argp;
    hi_s32 ret;

    hi_log_func_enter();

    ret = kapi_trng_get_random(&trng->randnum, trng->timeout);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

static crypto_dispatch_func g_dispatch_func[CRYPTO_CMD_COUNT] = {
    {"CreateHandle",  dispatch_symc_create_handle,  CRYPTO_CMD_SYMC_CREATEHANDLE},
    {"DestroyHandle", dispatch_symc_destroy_handle, CRYPTO_CMD_SYMC_DESTROYHANDLE},
    {"ConfigChn",     dispatch_symc_cfg,            CRYPTO_CMD_SYMC_CONFIGHANDLE},
    {"Encrypt",       dispatch_symc_encrypt,        CRYPTO_CMD_SYMC_ENCRYPT},
    {"EncryptMulti",  dispatch_symc_encrypt_multi,  CRYPTO_CMD_SYMC_ENCRYPTMULTI},
    {"GetTag",        dispatch_symc_get_tag,        CRYPTO_CMD_SYMC_GETTAG},
    {"HashStart",     dispatch_hash_start,          CRYPTO_CMD_HASH_START},
    {"HashUpdate",    dispatch_hash_update,         CRYPTO_CMD_HASH_UPDATE},
    {"HashFinish",    dispatch_hash_finish,         CRYPTO_CMD_HASH_FINISH},
    {"RsaEncrypt",    dispatch_rsa_encrypt,         CRYPTO_CMD_RSA_ENC},
    {"RsaDecrypt",    dispatch_rsa_decrypt,         CRYPTO_CMD_RSA_DEC},
    {"RsaSign",       dispatch_rsa_sign_hash,       CRYPTO_CMD_RSA_SIGN},
    {"RsaVerify",     dispatch_rsa_verify_hash,     CRYPTO_CMD_RSA_VERIFY},
    {"TRNG",          dispatch_trng_get_random,     CRYPTO_CMD_TRNG},
    {"GetSymcConfig", dispatch_symc_get_cfg,        CRYPTO_CMD_SYMC_GET_CONFIG},
    {"KladKey",       dispatch_klad_key,            CRYPTO_CMD_KLAD_KEY},
    {"KladKey",       dispatch_rsa_bn_exp_mod,      CRYPTO_CMD_BN_EXP_MOD},
};

hi_s32 crypto_ioctl(hi_u32 cmd, hi_void *argp)
{
    hi_u32 nr;
    hi_s32 ret;

    hi_log_func_enter();

    nr = crypto_ioc_nr(cmd);
    hi_log_chk_param_return(argp == HI_NULL);
    hi_log_chk_param_return(nr >= CRYPTO_CMD_COUNT);
    hi_log_chk_param_return(cmd != g_dispatch_func[nr].cmd);

    hi_log_debug("cmd 0x%x, nr %u, size %u, local cmd 0x%x\n",
                 cmd, nr, crypto_ioc_size(cmd), g_dispatch_func[nr].cmd);

    hi_log_info("Link Func NR %u, Name:  %s\n", nr, g_dispatch_func[nr].name);
    ret = g_dispatch_func[nr].func(argp);
    if (ret != HI_SUCCESS) {
        /* TRNG may be empty in FIFO, don't report error, try to read it again */
        if (cmd != CRYPTO_CMD_TRNG) {
            hi_log_error("error, call dispatch_fun fun failed!\n");
            hi_log_print_func_err(crypto_dispatch_func, ret);
        }
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

hi_s32 crypto_entry(hi_void)
{
    hi_s32 ret;

    hi_log_func_enter();

    crypto_mem_init();

    ret = module_addr_map();
    if (ret != HI_SUCCESS) {
        hi_log_error("module addr map failed\n");
        hi_log_print_func_err(module_addr_map, ret);
        return ret;
    }

    ret = kapi_symc_init();
    if (ret != HI_SUCCESS) {
        hi_log_error("kapi symc init failed\n");
        hi_log_print_func_err(kapi_symc_init, ret);
        goto error;
    }

    ret = kapi_hash_init();
    if (ret != HI_SUCCESS) {
        hi_log_error("kapi hash init failed\n");
        hi_log_print_func_err(kapi_hash_init, ret);
        goto error1;
    }

    ret = kapi_rsa_init();
    if (ret != HI_SUCCESS) {
        hi_log_error("kapi rsa init failed\n");
        hi_log_print_func_err(kapi_rsa_init, ret);
        goto error2;
    }

    ret = hi_drv_compat_init();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(hi_drv_compat_init, ret);
        goto error3;
    }

    hi_log_func_exit();
    return HI_SUCCESS;

error3:
    kapi_rsa_deinit();
error2:
    kapi_hash_deinit();
error1:
    kapi_symc_deinit();
error:
    module_addr_unmap();

    return ret;
}

hi_s32 crypto_exit(hi_void)
{
    hi_s32 ret;

    hi_log_func_enter();

    ret = kapi_symc_deinit();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_deinit, ret);
        return ret;
    }

    ret = kapi_hash_deinit();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_hash_deinit, ret);
        return ret;
    }

    ret = kapi_rsa_deinit();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_rsa_deinit, ret);
        return ret;
    }

    ret = hi_drv_compat_deinit();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(hi_drv_compat_deinit, ret);
        return ret;
    }

    ret = module_addr_unmap();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(module_addr_unmap, ret);
        return ret;
    }

    crypto_mem_deinit();

    hi_log_func_exit();
    return HI_SUCCESS;
}


hi_s32 crypto_release(hi_void)
{
    hi_s32 ret;

    ret = kapi_symc_release();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_symc_release, ret);
        return ret;
    }

    ret = kapi_hash_release();
    if (ret != HI_SUCCESS) {
        hi_log_print_func_err(kapi_hash_release, ret);
        return ret;
    }

    return HI_SUCCESS;
}
