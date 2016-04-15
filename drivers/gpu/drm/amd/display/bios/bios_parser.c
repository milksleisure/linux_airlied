/*
 * Copyright 2012-15 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#include "atom.h"
#include "bios_parser_interface.h"
#include "bios_parser.h"

#define BIOS_IMAGE_SIZE_OFFSET 2
#define BIOS_IMAGE_SIZE_UNIT 512

struct display_bios *display_bios_parser_create(struct bp_init_data *init,
						enum dce_version version)
{
	struct bios_parser *bp = NULL;

	bp = kzalloc(sizeof(*bp), GFP_KERNEL);
	if (!bp)
		return NULL;

	bp->cgs = init->cgs;
	bp->bios = init->bios;
	bp->bios_size = bp->bios[BIOS_IMAGE_SIZE_OFFSET] * BIOS_IMAGE_SIZE_UNIT;

	display_bios_parser_init_cmd_tbl(bp);
	return (struct display_bios *)bp;
}

void display_bios_parser_destroy(struct display_bios *dcb)
{
	struct bios_parser *bp = dcb_to_bp(dcb);

	kfree(bp);
}

enum bp_result display_bios_blank_crtc(struct display_bios *dcb,
				       struct bp_blank_crtc_parameters *bp_params,
				       bool blank)
{
	struct bios_parser *bp = dcb_to_bp(dcb);
	if (!bp->cmd_tbl.blank_crtc)
		return BP_RESULT_FAILURE;
	return bp->cmd_tbl.blank_crtc(bp, bp_params, blank);
}

enum bp_result display_bios_enable_crtc(struct display_bios *dcb,
					enum controller_id id,
					bool enable)
{
	struct bios_parser *bp = dcb_to_bp(dcb);

	if (!bp->cmd_tbl.enable_crtc)
		return BP_RESULT_FAILURE;

	return bp->cmd_tbl.enable_crtc(bp, id, enable);
}

enum bp_result display_bios_enable_disp_power_gating(
	struct display_bios *dcb,
	enum controller_id controller_id,
	enum bp_pipe_control_action action)
{
	struct bios_parser *bp = dcb_to_bp(dcb);

	if (!bp->cmd_tbl.enable_disp_power_gating)
		return BP_RESULT_FAILURE;

	return bp->cmd_tbl.enable_disp_power_gating(bp, controller_id,
						    action);
}

enum bp_result display_bios_adjust_pixel_clock(struct display_bios *dcb,
					       struct bp_adjust_pixel_clock_parameters *bp_params)
{
	struct bios_parser *bp = dcb_to_bp(dcb);

	if (!bp->cmd_tbl.adjust_display_pll)
		return BP_RESULT_FAILURE;

	return bp->cmd_tbl.adjust_display_pll(bp, bp_params);
}

enum bp_result display_bios_enable_spread_spectrum_on_ppll(struct display_bios *dcb,
							   struct bp_spread_spectrum_parameters *bp_params,
							   bool enable)
{
	struct bios_parser *bp = dcb_to_bp(dcb);

	if (!bp->cmd_tbl.enable_spread_spectrum_on_ppll)
		return BP_RESULT_FAILURE;

	return bp->cmd_tbl.enable_spread_spectrum_on_ppll(
			bp, bp_params, enable);
}

enum signal_type display_bios_dac_load_detect(struct display_bios *dcb,
					      struct graphics_object_id encoder,
					      struct graphics_object_id connector,
					      enum signal_type display_signal)
{
	struct bios_parser *bp = dcb_to_bp(dcb);

	if (!bp->cmd_tbl.dac_load_detection)
		return SIGNAL_TYPE_NONE;

	return bp->cmd_tbl.dac_load_detection(bp, encoder, connector,
					      display_signal);
}
