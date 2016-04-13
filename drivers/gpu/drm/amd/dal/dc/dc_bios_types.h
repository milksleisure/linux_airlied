/*
 * Copyright 2016 Advanced Micro Devices, Inc.
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

#ifndef DC_BIOS_TYPES_H
#define DC_BIOS_TYPES_H

/******************************************************************************
 * Interface file for VBIOS implementations.
 *
 * The default implementation is inside DC.
 * Display Manager (which instantiates DC) has the option to supply it's own
 * (external to DC) implementation of VBIOS, which will be called by DC, using
 * this interface.
 * (The intended use is Diagnostics, but other uses may appear.)
 *****************************************************************************/

#include "include/bios_parser_types.h"

uint8_t dc_bios_get_connectors_number(struct dc_bios *bios);

void dc_bios_power_up(struct dc_bios *bios);

struct graphics_object_id dc_bios_get_encoder_id(struct dc_bios *bios,
						 uint32_t i);
struct graphics_object_id dc_bios_get_connector_id(struct dc_bios *bios,
						   uint8_t connector_index);
enum bp_result dc_bios_get_src_obj(struct dc_bios *bios,
				    struct graphics_object_id object_id,
				    uint32_t index,
				    struct graphics_object_id *src_object_id);
enum bp_result dc_bios_get_i2c_info(struct dc_bios *dcb,
				    struct graphics_object_id id,
				    struct graphics_object_i2c_info *info);

enum bp_result dc_bios_get_hpd_info(struct dc_bios *bios,
				    struct graphics_object_id id,
				    struct graphics_object_hpd_info *info);
enum bp_result dc_bios_get_device_tag(struct dc_bios *bios,
				      struct graphics_object_id connector_object_id,
				      uint32_t device_tag_index,
				      struct connector_device_tag_info *info);

enum bp_result dc_bios_get_firmware_info(struct dc_bios *bios,
					 struct firmware_info *info);

enum bp_result dc_bios_get_spread_spectrum_info(struct dc_bios *bios,
						enum ss_signal_type signal,
						uint32_t index,
						struct spread_spectrum_info *ss_info);
uint32_t dc_bios_get_ss_entry_number(struct dc_bios *bios,
				     enum ss_signal_type signal);
enum bp_result dc_bios_get_embedded_panel_info(struct dc_bios *bios,
					       struct embedded_panel_info *info);

enum bp_result dc_bios_get_gpio_pin_info(struct dc_bios *bios,
					 uint32_t gpio_id,
					 struct gpio_pin_info *info);

enum bp_result dc_bios_get_encoder_cap_info(struct dc_bios *bios,
					    struct graphics_object_id object_id,
					    struct bp_encoder_cap_info *info);

bool dc_bios_is_device_id_supported(struct dc_bios *bios,
				    struct device_id id);
bool dc_bios_is_accelerated_mode(struct dc_bios *bios);

/* COMMANDS */
enum bp_result dc_bios_encoder_control(struct dc_bios *bios,
				       struct bp_encoder_control *cntl);
enum bp_result dc_bios_transmitter_control(struct dc_bios *bios,
					   struct bp_transmitter_control *cntl);
enum bp_result dc_bios_enable_crtc(struct dc_bios *bios,
				   enum controller_id id,
				   bool enable);
enum bp_result dc_bios_adjust_pixel_clock(struct dc_bios *bios,
					  struct bp_adjust_pixel_clock_parameters *bp_params);
enum bp_result dc_bios_set_pixel_clock(struct dc_bios *bios,
				       struct bp_pixel_clock_parameters *bp_params);
enum bp_result dc_bios_set_dce_clock(struct dc_bios *bios,
				     struct bp_set_dce_clock_parameters *bp_params);
enum bp_result dc_bios_enable_spread_spectrum_on_ppll(struct dc_bios *bios,
						      struct bp_spread_spectrum_parameters *bp_params,
						      bool enable);
enum bp_result dc_bios_program_crtc_timing(struct dc_bios *bios,
					   struct bp_hw_crtc_timing_parameters *bp_params);
enum bp_result dc_bios_crtc_source_select(struct dc_bios *bios,
					  struct bp_crtc_source_select *bp_params);
enum bp_result dc_bios_program_display_engine_pll(struct dc_bios *bios,
						  struct bp_pixel_clock_parameters *bp_params);

enum bp_result dc_bios_enable_disp_power_gating(struct dc_bios *bios,
						enum controller_id controller_id,
						enum bp_pipe_control_action action);

typedef bool (*bp_i2c_read_fn)(void *data,
			       struct graphics_object_i2c_info *i2c_info,
			       uint8_t *buffer,
			       uint32_t length);


void dc_bios_post_init(struct dc_bios *bios,
		       void *i2c_ptr,
		       bp_i2c_read_fn read_fn);

struct integrated_info *dc_bios_create_integrated_info(struct dc_bios *bios);

void dc_bios_destroy_integrated_info(struct dc_bios *dcb,
				     struct integrated_info **info);

struct dc_bios;

#endif /* DC_BIOS_TYPES_H */
