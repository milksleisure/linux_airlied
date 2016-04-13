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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#ifndef __DAL_GRPH_OBJECT_CTRL_DEFS_H__
#define __DAL_GRPH_OBJECT_CTRL_DEFS_H__

#include "grph_object_defs.h"

/*
 * #####################################################
 * #####################################################
 *
 * These defines shared between asic_control/bios_parser and other
 * DAL components
 *
 * #####################################################
 * #####################################################
 */

/* Bitvector and bitfields of possible optimizations
 #IMPORTANT# Keep bitfields match bitvector! */
enum optimization_feature {
	/* Don't do HW programming on panels that were enabled by VBIOS */
	OF_SKIP_HW_PROGRAMMING_ON_ENABLED_EMBEDDED_DISPLAY = 0x1,
	OF_SKIP_RESET_OF_ALL_HW_ON_S3RESUME = 0x2,
	OF_SKIP_HW_RESET_OF_EMBEDDED_DISPLAY_ON_S3RESUME = 0x4,
	OF_SKIP_POWER_UP_VBIOS_ENABLED_ENCODER = 0x8,
	/* Do not turn off VCC while powering down on boot or resume */
	OF_KEEP_VCC_DURING_POWER_DOWN_ON_BOOT_OR_RESUME = 0x10,
	/* Do not turn off VCC while performing SetMode */
	OF_KEEP_VCC_DURING_SET_MODE = 0x20,
	OF_DO_NOT_WAIT_FOR_HPD_LOW = 0x40,
	OF_SKIP_POWER_DOWN_INACTIVE_ENCODER = 0x80
};

/*
 * DFS-bypass flag
 */
/* Copy of SYS_INFO_GPUCAPS__ENABEL_DFS_BYPASS from atombios.h */
enum {
	DFS_BYPASS_ENABLE = 0x10
};


#endif
