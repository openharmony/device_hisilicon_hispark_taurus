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

#ifndef __TDE_OSICTL_H__
#define __TDE_OSICTL_H__

#include "drv_tde_type.h"
#include "tde_adp.h"
#include "tde_osictl_k.h"
#include "drv_tde_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

hi_s32 tde_osi_open(hi_void);

hi_s32 tde_osi_close(hi_void);

/*
  * Function:      tde_osi_begin_job
  * Description:   get TDE task handle
  * Input:         handle:get task handle pointer
  * Return:        success / error code
  */
hi_s32 tde_osi_begin_job(hi_s32 *handle, hi_void *private_data);

hi_s32 tde_osi_begin_job_ex(hi_s32 *handle);

/*
 * Function:      tde_osi_end_job
 * Description:   submit TDE task
 * Input:         handle: task handle
 *                is_block: if block
 *                time_out: timeout value(unit by 10ms)
 *                is_sync: if synchronization or not of sumbitting operate
 *                func_compl_cb: callback function of completing operate
 * Return:        success / error code
 */
hi_s32 tde_osi_end_job(drv_tde_end_job_cmd *end_job, tde_func_cb func_compl_cb, hi_void *func_para);

/*
 * Function:      tde_osi_cancel_job
 * Description:   delete TDE task created, only effectivly callede before endjob
 *                release software resource of list of its task
 * Input:         handle: task handle
 * Return:        success / error code
 */
hi_s32 tde_osi_cancel_job(hi_s32 handle);

/*
 * Function:      tde_osi_wait_for_done
 * Description:   wait for completing of submiting TDE operate
 * Input:         handle: task handle
 * Return:        success / error code
 */
hi_s32 tde_osi_wait_for_done(hi_s32 handle, hi_u32 time_out);

hi_s32 tde_osi_wait_all_done(hi_bool is_sync);

hi_s32 tde_osi_quick_copy(hi_s32 handle, hi_tde_single_src *single_src);

/*
 * Function:      tde_osi_quick_fill
 * Description:   quickly fill fixed value to target bitmap
 * Input:         handle: task handle
 *                fill_data: fill data, pixel format is the same as target bitmap format
 * Return:        success / error code
 */
hi_s32 tde_osi_quick_fill(hi_s32 handle, hi_tde_surface *dst_surface, hi_tde_rect *dst_rect, hi_u32 fill_data);

/*
 * Function:      tde_osi_quick_resize
 * Description:   zoom the size of source bitmap to the size of target bitmap,
                  source and target are the same bitmap
 * Input:         handle: task handle
 *                src_surface: source bitmap info struct
 *                src_rect: source bitmap operate rect
 *                dst_surface: target bitmap info struct
 *                dst_rect: target bitmap operate rect
 * Return:        success / error code
 */
hi_s32 tde_osi_quick_resize(hi_s32 handle, hi_tde_surface *src_surface, hi_tde_rect *src_rect,
                            hi_tde_surface *dst_surface, hi_tde_rect *dst_rect);

/*
 * Function:      tde_osi_quick_flicker
 * Description:   deflicker source bitmap,output to target bitmap,
                  source and target can be the same bitmap
 * Input:         handle: task handle
 *                src_surface: source bitmap info struct
 *                src_rect: source bitmap operate rect
 *                dst_surface: target bitmap info struct
 *                dst_rect: target bitmap operate rect
 * Return:        success / error code
 */
hi_s32 tde_osi_quick_flicker(hi_s32 handle, hi_tde_surface *src_surface, hi_tde_rect *src_rect,
                             hi_tde_surface *dst_surface, hi_tde_rect *dst_rect);

/*
 * Function:      tde_osi_blit
 * Description:   operate pstBackGround with pstForeGround, which result output to pDst, operate setting is in pOpt
 * Input:         handle:task handle
 *                opt: operate parameter setting struct
 * Return:        success / error code
 */
hi_s32 tde_osi_blit(hi_s32 handle, hi_tde_double_src *double_src, hi_tde_opt *opt);

/*
 * Function:      tde_osi_mb_blit
 * Description:   MB blit interface
 * Input:         handle: task handle
 *                mb_opt:  operate parameter setting struct
 * Return:        >0: return task id of current operate ; <0: fail
 */
hi_s32 tde_osi_mb_blit(hi_s32 handle, hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt);

/*
 * Function:      tde_osi_bitmap_mask_rop
 * Description:   Ropmask source with mask bitmap at first, and then ropmask source1 with middle bitmap
 *                output result into target bitmap
 * Input:         handle: task handle
 * Return:        success / error code
 */
#if (TDE_CAPABILITY & MASKROP)
hi_s32 tde_osi_bitmap_mask_rop(hi_s32 handle, hi_tde_triple_src *triple_src,
                               hi_tde_rop_mode rop_color, hi_tde_rop_mode rop_alpha);
#endif
/*
 * Function:      tde_osi_bitmap_mask_blend
 * Description:   blendmask source2 with mask bitmap at first, and then blend source1 with middle bitmap
 * 			      output result to target bitmap
 * Input:         handle: task handle
 * Return:        success / error code
 */
#if (TDE_CAPABILITY & MASKBLEND)
hi_s32 tde_osi_bitmap_mask_blend(hi_s32 handle, hi_tde_triple_src *triple_src,
                                 hi_u8 alpha, hi_tde_alpha_blending blend_mode);
#endif

/*
 * Function:      tde_osi_solid_draw
 * Description:   output result of operate single color value with source bitmap,operate setting is in pOpt
 * Input:         handle: task handle
 *                single_src: source bitmap information struct
 *                fill_color: color info
 *                opt: operate parameter setting struct
 * Return:        success / error code
 */
hi_s32 tde_osi_solid_draw(hi_s32 handle, hi_tde_single_src *single_src, hi_tde_fill_color *fill_color,
                          hi_tde_opt *opt);

hi_s32 tde_osi_set_deflicker_level(hi_tde_deflicker_level deflicker_level);

hi_s32 tde_osi_get_deflicker_level(hi_tde_deflicker_level *deflicker_level);

hi_s32 tde_osi_set_alpha_threshold_value(hi_u8 threshold_value);

hi_s32 tde_osi_get_alpha_threshold_value(hi_u8 *threshold_value);

hi_s32 tde_osi_set_alpha_threshold_state(hi_bool alpha_threshold_en);

hi_s32 tde_osi_get_alpha_threshold_state(hi_bool *alpha_threshold_en);
/*
 * Function:      tde_osi_pattern_fill
 * Description:   pattern fill
 * Output:        handle:task handle
                  opt:pattern fill operate option
 * Return:        success/error code
 * Others:        pattern width is 256 at most, height is not litmited
 */
hi_s32 tde_osi_pattern_fill(hi_s32 handle, hi_tde_double_src *double_src, hi_tde_pattern_fill_opt *opt);

hi_s32 tde_osi_enable_region_deflicker(hi_bool region_deflicker);

hi_s32 tde_cal_scale_rect(const hi_tde_rect *src_rect, const hi_tde_rect *dst_rect, hi_tde_rect *rect_in_src,
                          hi_tde_rect *rect_in_dst);
#ifdef CONFIG_TDE_BLIT_EX
hi_s32 tde_osi_single_blit_ex(hi_s32 handle, hi_tde_surface *src_surface, hi_tde_rect *src_rect,
    hi_tde_surface *dst_surface, hi_tde_rect *dst_rect, hi_tde_opt *opt, hi_bool mmz_for_src, hi_bool mmz_for_dst);

#endif

hi_s32 tde_osi_quick_rotate(hi_s32 handle, hi_tde_single_src *single_src, hi_tde_rotate_angle rotate_angle);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TDE_OSICTL_H_ */
