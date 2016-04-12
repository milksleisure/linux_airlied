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

#ifndef __DISPLAY_GPIO_TYPES_H__
#define __DISPLAY_GPIO_TYPES_H__

/* this file is a counterpart to DAL's gpio_types.h
 * it contains the types that we need for the non-DAL code.
 */

struct gpio_pin_info {
	uint32_t offset;
	uint32_t offset_y;
	uint32_t offset_en;
	uint32_t offset_mask;

	uint32_t mask;
	uint32_t mask_y;
	uint32_t mask_en;
	uint32_t mask_mask;
};

enum gpio_pin_output_state {
	GPIO_PIN_OUTPUT_STATE_ACTIVE_LOW,
	GPIO_PIN_OUTPUT_STATE_ACTIVE_HIGH,
	GPIO_PIN_OUTPUT_STATE_DEFAULT = GPIO_PIN_OUTPUT_STATE_ACTIVE_LOW
};

#endif

