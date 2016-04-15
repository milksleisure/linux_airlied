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

#ifndef __DISPLAY_COMMAND_TABLE_H__
#define __DISPLAY_COMMAND_TABLE_H__

struct bios_parser;

struct cmd_tbl {
	enum bp_result (*enable_crtc)(struct bios_parser *bp,
				      enum controller_id controller_id,
				      bool enable);
	enum bp_result (*blank_crtc)(struct bios_parser *bp,
				     struct bp_blank_crtc_parameters *bp_params,
				     bool blank);
	enum bp_result (*enable_disp_power_gating)(struct bios_parser *bp,
						   enum controller_id crtc_id,
						   enum bp_pipe_control_action action);
	enum bp_result (*adjust_display_pll)(struct bios_parser *bp,
					     struct bp_adjust_pixel_clock_parameters *bp_params);
	enum bp_result (*enable_spread_spectrum_on_ppll)(struct bios_parser *bp,
							 struct bp_spread_spectrum_parameters *bp_params,
							 bool enable);
	enum signal_type (*dac_load_detection)(struct bios_parser *bp,
					       struct graphics_object_id encoder,
					       struct graphics_object_id connector,
					       enum signal_type display_signal);

};

void display_bios_parser_init_cmd_tbl(struct bios_parser *bp);

#endif
