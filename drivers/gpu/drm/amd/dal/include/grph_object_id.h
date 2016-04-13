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

#ifndef __DAL_GRPH_OBJECT_ID_H__
#define __DAL_GRPH_OBJECT_ID_H__

#include "display_grph_object_id.h"
/* some simple functions for convenient graphics_object_id handle */

static inline struct graphics_object_id dal_graphics_object_id_init(
	uint32_t id,
	enum object_enum_id enum_id,
	enum object_type type)
{
	struct graphics_object_id result = {
		id, enum_id, type, 0
	};

	return result;
}

bool dal_graphics_object_id_is_valid(
	struct graphics_object_id id);
bool dal_graphics_object_id_is_equal(
	struct graphics_object_id id1,
	struct graphics_object_id id2);
uint32_t dal_graphics_object_id_to_uint(
	struct graphics_object_id id);

enum controller_id dal_graphics_object_id_get_controller_id(
	struct graphics_object_id id);
enum clock_source_id dal_graphics_object_id_get_clock_source_id(
	struct graphics_object_id id);
enum encoder_id dal_graphics_object_id_get_encoder_id(
	struct graphics_object_id id);
enum connector_id dal_graphics_object_id_get_connector_id(
	struct graphics_object_id id);
enum audio_id dal_graphics_object_id_get_audio_id(
	struct graphics_object_id id);
enum engine_id dal_graphics_object_id_get_engine_id(
	struct graphics_object_id id);
#endif
