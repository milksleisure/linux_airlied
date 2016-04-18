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
#include "command_table.h"
#include "cgs_linux.h"

#include "dce80/command_table_helper_dce80.h"
#include "dce110/command_table_helper_dce110.h"

#define EXEC_BIOS_CMD_TABLE(command, params)\
	(cgs_atom_exec_cmd_table(bp->cgs, \
		GetIndexIntoMasterTable(COMMAND, command), \
		&params) == 0)

#define BIOS_CMD_TABLE_REVISION(command, frev, crev)\
	cgs_atom_get_cmd_table_revs(bp->cgs, \
		GetIndexIntoMasterTable(COMMAND, command), &frev, &crev)

#define BIOS_CMD_TABLE_PARA_REVISION(command)\
	bios_cmd_table_para_revision(bp->cgs, \
		GetIndexIntoMasterTable(COMMAND, command))

static uint32_t bios_cmd_table_para_revision(void *cgs_device,
					     uint32_t index)
{
	uint8_t frev, crev;

	if (cgs_atom_get_cmd_table_revs(cgs_device,
					index,
					&frev, &crev) != 0)
		return 0;
	return crev;
}

static bool cmd_table_helper_controller_id_to_atom(enum controller_id id,
						   uint8_t *atom_id)
{
	if (atom_id == NULL) {
		return false;
	}

	switch (id) {
	case CONTROLLER_ID_D0:
		*atom_id = ATOM_CRTC1;
		return true;
	case CONTROLLER_ID_D1:
		*atom_id = ATOM_CRTC2;
		return true;
	case CONTROLLER_ID_D2:
		*atom_id = ATOM_CRTC3;
		return true;
	case CONTROLLER_ID_D3:
		*atom_id = ATOM_CRTC4;
		return true;
	case CONTROLLER_ID_D4:
		*atom_id = ATOM_CRTC5;
		return true;
	case CONTROLLER_ID_D5:
		*atom_id = ATOM_CRTC6;
		return true;
	case CONTROLLER_ID_UNDERLAY0:
		*atom_id = ATOM_UNDERLAY_PIPE0;
		return true;
	case CONTROLLER_ID_UNDEFINED:
		*atom_id = ATOM_CRTC_INVALID;
		return true;
	default:
		/* Wrong controller id */
		return false;
	}
}

static uint8_t cmd_table_helper_disp_power_gating_action_to_atom(
        enum bp_pipe_control_action action)
{
        uint8_t atom_pipe_action = 0;

        switch (action) {
        case ASIC_PIPE_DISABLE:
                atom_pipe_action = ATOM_DISABLE;
                break;
        case ASIC_PIPE_ENABLE:
                atom_pipe_action = ATOM_ENABLE;
                break;
        case ASIC_PIPE_INIT:
                atom_pipe_action = ATOM_INIT;
                break;
        default:
                break;
        }

        return atom_pipe_action;
}

static uint8_t cmd_table_helper_encoder_id_to_atom(
	enum encoder_id id)
{
	switch (id) {
	case ENCODER_ID_INTERNAL_LVDS:
		return ENCODER_OBJECT_ID_INTERNAL_LVDS;
	case ENCODER_ID_INTERNAL_TMDS1:
		return ENCODER_OBJECT_ID_INTERNAL_TMDS1;
	case ENCODER_ID_INTERNAL_TMDS2:
		return ENCODER_OBJECT_ID_INTERNAL_TMDS2;
	case ENCODER_ID_INTERNAL_DAC1:
		return ENCODER_OBJECT_ID_INTERNAL_DAC1;
	case ENCODER_ID_INTERNAL_DAC2:
		return ENCODER_OBJECT_ID_INTERNAL_DAC2;
	case ENCODER_ID_INTERNAL_LVTM1:
		return ENCODER_OBJECT_ID_INTERNAL_LVTM1;
	case ENCODER_ID_INTERNAL_HDMI:
		return ENCODER_OBJECT_ID_HDMI_INTERNAL;
	case ENCODER_ID_EXTERNAL_TRAVIS:
		return ENCODER_OBJECT_ID_TRAVIS;
	case ENCODER_ID_EXTERNAL_NUTMEG:
		return ENCODER_OBJECT_ID_NUTMEG;
	case ENCODER_ID_INTERNAL_KLDSCP_TMDS1:
		return ENCODER_OBJECT_ID_INTERNAL_KLDSCP_TMDS1;
	case ENCODER_ID_INTERNAL_KLDSCP_DAC1:
		return ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC1;
	case ENCODER_ID_INTERNAL_KLDSCP_DAC2:
		return ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC2;
	case ENCODER_ID_EXTERNAL_MVPU_FPGA:
		return ENCODER_OBJECT_ID_MVPU_FPGA;
	case ENCODER_ID_INTERNAL_DDI:
		return ENCODER_OBJECT_ID_INTERNAL_DDI;
	case ENCODER_ID_INTERNAL_UNIPHY:
		return ENCODER_OBJECT_ID_INTERNAL_UNIPHY;
	case ENCODER_ID_INTERNAL_KLDSCP_LVTMA:
		return ENCODER_OBJECT_ID_INTERNAL_KLDSCP_LVTMA;
	case ENCODER_ID_INTERNAL_UNIPHY1:
		return ENCODER_OBJECT_ID_INTERNAL_UNIPHY1;
	case ENCODER_ID_INTERNAL_UNIPHY2:
		return ENCODER_OBJECT_ID_INTERNAL_UNIPHY2;
	case ENCODER_ID_INTERNAL_UNIPHY3:
		return ENCODER_OBJECT_ID_INTERNAL_UNIPHY3;
	case ENCODER_ID_INTERNAL_WIRELESS:
		return ENCODER_OBJECT_ID_INTERNAL_VCE;
	case ENCODER_ID_UNKNOWN:
		return ENCODER_OBJECT_ID_NONE;
	default:
		/* Invalid encoder id */
		return ENCODER_OBJECT_ID_NONE;
	}
}

static uint32_t cmd_table_helper_encoder_mode_bp_to_atom(
	enum signal_type s,
	bool enable_dp_audio)
{
	switch (s) {
	case SIGNAL_TYPE_DVI_SINGLE_LINK:
	case SIGNAL_TYPE_DVI_DUAL_LINK:
		return ATOM_ENCODER_MODE_DVI;
	case SIGNAL_TYPE_HDMI_TYPE_A:
		return ATOM_ENCODER_MODE_HDMI;
	case SIGNAL_TYPE_LVDS:
		return ATOM_ENCODER_MODE_LVDS;
	case SIGNAL_TYPE_EDP:
	case SIGNAL_TYPE_DISPLAY_PORT_MST:
	case SIGNAL_TYPE_DISPLAY_PORT:
		if (enable_dp_audio)
			return ATOM_ENCODER_MODE_DP_AUDIO;
		else
			return ATOM_ENCODER_MODE_DP;
	case SIGNAL_TYPE_RGB:
		return ATOM_ENCODER_MODE_CRT;
	default:
		return ATOM_ENCODER_MODE_CRT;
	}
}

/*
 *
 * SET PIXEL CLOCK
 *
 */


static enum bp_result set_pixel_clock_v3(
	struct bios_parser *bp,
	struct bp_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;
	PIXEL_CLOCK_PARAMETERS_V3 *params;
	SET_PIXEL_CLOCK_PS_ALLOCATION allocation;

	memset(&allocation, 0, sizeof(allocation));

	if (CLOCK_SOURCE_ID_PLL1 == bp_params->pll_id)
		allocation.sPCLKInput.ucPpll = ATOM_PPLL1;
	else if (CLOCK_SOURCE_ID_PLL2 == bp_params->pll_id)
		allocation.sPCLKInput.ucPpll = ATOM_PPLL2;
	else
		return BP_RESULT_BADINPUT;

	allocation.sPCLKInput.usRefDiv =
			cpu_to_le16((uint16_t)bp_params->reference_divider);
	allocation.sPCLKInput.usFbDiv =
			cpu_to_le16((uint16_t)bp_params->feedback_divider);
	allocation.sPCLKInput.ucFracFbDiv =
			(uint8_t)bp_params->fractional_feedback_divider;
	allocation.sPCLKInput.ucPostDiv =
			(uint8_t)bp_params->pixel_clock_post_divider;

	/* We need to convert from KHz units into 10KHz units */
	allocation.sPCLKInput.usPixelClock =
			cpu_to_le16((uint16_t)(bp_params->target_pixel_clock / 10));

	params = (PIXEL_CLOCK_PARAMETERS_V3 *)&allocation.sPCLKInput;
	params->ucTransmitterId =
		cmd_table_helper_encoder_id_to_atom(
			display_graphics_object_id_get_encoder_id(
				bp_params->encoder_object_id));
	params->ucEncoderMode =
		cmd_table_helper_encoder_mode_bp_to_atom(
			bp_params->signal_type, false);

	if (bp_params->flags.FORCE_PROGRAMMING_OF_PLL)
		params->ucMiscInfo |= PIXEL_CLOCK_MISC_FORCE_PROG_PPLL;

	if (bp_params->flags.USE_E_CLOCK_AS_SOURCE_FOR_D_CLOCK)
		params->ucMiscInfo |= PIXEL_CLOCK_MISC_USE_ENGINE_FOR_DISPCLK;

	if (CONTROLLER_ID_D1 != bp_params->controller_id)
		params->ucMiscInfo |= PIXEL_CLOCK_MISC_CRTC_SEL_CRTC2;

	if (EXEC_BIOS_CMD_TABLE(SetPixelClock, allocation))
		result = BP_RESULT_OK;

	return result;
}

#ifndef SET_PIXEL_CLOCK_PS_ALLOCATION_V5
/* video bios did not define this: */
typedef struct _SET_PIXEL_CLOCK_PS_ALLOCATION_V5 {
	PIXEL_CLOCK_PARAMETERS_V5 sPCLKInput;
	/* Caller doesn't need to init this portion */
	ENABLE_SPREAD_SPECTRUM_ON_PPLL sReserved;
} SET_PIXEL_CLOCK_PS_ALLOCATION_V5;
#endif

#ifndef SET_PIXEL_CLOCK_PS_ALLOCATION_V6
/* video bios did not define this: */
typedef struct _SET_PIXEL_CLOCK_PS_ALLOCATION_V6 {
	PIXEL_CLOCK_PARAMETERS_V6 sPCLKInput;
	/* Caller doesn't need to init this portion */
	ENABLE_SPREAD_SPECTRUM_ON_PPLL sReserved;
} SET_PIXEL_CLOCK_PS_ALLOCATION_V6;
#endif

static enum bp_result set_pixel_clock_v5(
	struct bios_parser *bp,
	struct bp_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;
	SET_PIXEL_CLOCK_PS_ALLOCATION_V5 clk;
	uint8_t controller_id;
	uint32_t pll_id;

	memset(&clk, 0, sizeof(clk));

	if (bp->cmd_helper->clock_source_id_to_atom(bp_params->pll_id, &pll_id)
	    && cmd_table_helper_controller_id_to_atom(
		    bp_params->controller_id, &controller_id)) {
		clk.sPCLKInput.ucCRTC = controller_id;
		clk.sPCLKInput.ucPpll = (uint8_t)pll_id;
		clk.sPCLKInput.ucRefDiv =
				(uint8_t)(bp_params->reference_divider);
		clk.sPCLKInput.usFbDiv =
				cpu_to_le16((uint16_t)(bp_params->feedback_divider));
		clk.sPCLKInput.ulFbDivDecFrac =
				cpu_to_le32(bp_params->fractional_feedback_divider);
		clk.sPCLKInput.ucPostDiv =
				(uint8_t)(bp_params->pixel_clock_post_divider);
		clk.sPCLKInput.ucTransmitterID =
			cmd_table_helper_encoder_id_to_atom(
						display_graphics_object_id_get_encoder_id(
								bp_params->encoder_object_id));
		clk.sPCLKInput.ucEncoderMode =
			(uint8_t)cmd_table_helper_encoder_mode_bp_to_atom(
						bp_params->signal_type, false);

		/* We need to convert from KHz units into 10KHz units */
		clk.sPCLKInput.usPixelClock =
				cpu_to_le16((uint16_t)(bp_params->target_pixel_clock / 10));

		if (bp_params->flags.FORCE_PROGRAMMING_OF_PLL)
			clk.sPCLKInput.ucMiscInfo |=
					PIXEL_CLOCK_MISC_FORCE_PROG_PPLL;

		if (bp_params->flags.SET_EXTERNAL_REF_DIV_SRC)
			clk.sPCLKInput.ucMiscInfo |=
					PIXEL_CLOCK_MISC_REF_DIV_SRC;

		/* clkV5.ucMiscInfo bit[3:2]= HDMI panel bit depth: =0: 24bpp
		 * =1:30bpp, =2:32bpp
		 * driver choose program it itself, i.e. here we program it
		 * to 888 by default.
		 */

		if (EXEC_BIOS_CMD_TABLE(SetPixelClock, clk))
			result = BP_RESULT_OK;
	}

	return result;
}

static enum bp_result set_pixel_clock_v6(
	struct bios_parser *bp,
	struct bp_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;
	SET_PIXEL_CLOCK_PS_ALLOCATION_V6 clk;
	uint8_t controller_id;
	uint32_t pll_id;

	memset(&clk, 0, sizeof(clk));

	if (bp->cmd_helper->clock_source_id_to_atom(bp_params->pll_id, &pll_id)
	    && cmd_table_helper_controller_id_to_atom(
		    bp_params->controller_id, &controller_id)) {
		/* Note: VBIOS still wants to use ucCRTC name which is now
		 * 1 byte in ULONG
		 *typedef struct _CRTC_PIXEL_CLOCK_FREQ
		 *{
		 * target the pixel clock to drive the CRTC timing.
		 * ULONG ulPixelClock:24;
		 * 0 means disable PPLL/DCPLL. Expanded to 24 bits comparing to
		 * previous version.
		 * ATOM_CRTC1~6, indicate the CRTC controller to
		 * ULONG ucCRTC:8;
		 * drive the pixel clock. not used for DCPLL case.
		 *}CRTC_PIXEL_CLOCK_FREQ;
		 *union
		 *{
		 * pixel clock and CRTC id frequency
		 * CRTC_PIXEL_CLOCK_FREQ ulCrtcPclkFreq;
		 * ULONG ulDispEngClkFreq; dispclk frequency
		 *};
		 */
		clk.sPCLKInput.ulCrtcPclkFreq.ucCRTC = controller_id;
		clk.sPCLKInput.ucPpll = (uint8_t) pll_id;
		clk.sPCLKInput.ucRefDiv =
				(uint8_t) bp_params->reference_divider;
		clk.sPCLKInput.usFbDiv =
				cpu_to_le16((uint16_t) bp_params->feedback_divider);
		clk.sPCLKInput.ulFbDivDecFrac =
				cpu_to_le32(bp_params->fractional_feedback_divider);
		clk.sPCLKInput.ucPostDiv =
				(uint8_t) bp_params->pixel_clock_post_divider;
		clk.sPCLKInput.ucTransmitterID =
				cmd_table_helper_encoder_id_to_atom(
						display_graphics_object_id_get_encoder_id(
								bp_params->encoder_object_id));
		clk.sPCLKInput.ucEncoderMode =
				(uint8_t) cmd_table_helper_encoder_mode_bp_to_atom(
						bp_params->signal_type, false);

		/* We need to convert from KHz units into 10KHz units */
		clk.sPCLKInput.ulCrtcPclkFreq.ulPixelClock =
				cpu_to_le32(bp_params->target_pixel_clock / 10);

		if (bp_params->flags.FORCE_PROGRAMMING_OF_PLL) {
			clk.sPCLKInput.ucMiscInfo |=
					PIXEL_CLOCK_V6_MISC_FORCE_PROG_PPLL;
		}

		if (bp_params->flags.SET_EXTERNAL_REF_DIV_SRC) {
			clk.sPCLKInput.ucMiscInfo |=
					PIXEL_CLOCK_V6_MISC_REF_DIV_SRC;
		}

		/* clkV6.ucMiscInfo bit[3:2]= HDMI panel bit depth: =0:
		 * 24bpp =1:30bpp, =2:32bpp
		 * driver choose program it itself, i.e. here we pass required
		 * target rate that includes deep color.
		 */

		if (EXEC_BIOS_CMD_TABLE(SetPixelClock, clk))
			result = BP_RESULT_OK;
	}

	return result;
}

static enum bp_result set_pixel_clock_v7(
	struct bios_parser *bp,
	struct bp_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;
#ifdef LATEST_ATOM_BIOS_SUPPORT
	PIXEL_CLOCK_PARAMETERS_V7 clk;
	uint8_t controller_id;
	uint32_t pll_id;

	memset(&clk, 0, sizeof(clk));

	if (bp->cmd_helper->clock_source_id_to_atom(bp_params->pll_id, &pll_id)
			&& bp->cmd_helper->controller_id_to_atom(bp_params->controller_id, &controller_id)) {
		/* Note: VBIOS still wants to use ucCRTC name which is now
		 * 1 byte in ULONG
		 *typedef struct _CRTC_PIXEL_CLOCK_FREQ
		 *{
		 * target the pixel clock to drive the CRTC timing.
		 * ULONG ulPixelClock:24;
		 * 0 means disable PPLL/DCPLL. Expanded to 24 bits comparing to
		 * previous version.
		 * ATOM_CRTC1~6, indicate the CRTC controller to
		 * ULONG ucCRTC:8;
		 * drive the pixel clock. not used for DCPLL case.
		 *}CRTC_PIXEL_CLOCK_FREQ;
		 *union
		 *{
		 * pixel clock and CRTC id frequency
		 * CRTC_PIXEL_CLOCK_FREQ ulCrtcPclkFreq;
		 * ULONG ulDispEngClkFreq; dispclk frequency
		 *};
		 */
		clk.ucCRTC = controller_id;
		clk.ucPpll = (uint8_t) pll_id;
		clk.ucTransmitterID = cmd_table_helper_encoder_id_to_atom(display_graphics_object_id_get_encoder_id(bp_params->encoder_object_id));
		clk.ucEncoderMode = (uint8_t) cmd_table_helper_encoder_mode_bp_to_atom(bp_params->signal_type, false);

		/* We need to convert from KHz units into 10KHz units */
		clk.ulPixelClock = cpu_to_le32(bp_params->target_pixel_clock * 10);

		clk.ucDeepColorRatio = (uint8_t) bp->cmd_helper->transmitter_color_depth_to_atom(bp_params->color_depth);

		if (bp_params->flags.FORCE_PROGRAMMING_OF_PLL)
			clk.ucMiscInfo |= PIXEL_CLOCK_V7_MISC_FORCE_PROG_PPLL;

		if (bp_params->flags.SET_EXTERNAL_REF_DIV_SRC)
			clk.ucMiscInfo |= PIXEL_CLOCK_V7_MISC_REF_DIV_SRC;

		if (bp_params->flags.PROGRAM_PHY_PLL_ONLY)
			clk.ucMiscInfo |= PIXEL_CLOCK_V7_MISC_PROG_PHYPLL;

		if (bp_params->flags.SUPPORT_YUV_420)
			clk.ucMiscInfo |= PIXEL_CLOCK_V7_MISC_YUV420_MODE;

		if (bp_params->flags.SET_XTALIN_REF_SRC)
			clk.ucMiscInfo |= PIXEL_CLOCK_V7_MISC_REF_DIV_SRC_XTALIN;

		if (bp_params->flags.SET_GENLOCK_REF_DIV_SRC)
			clk.ucMiscInfo |= PIXEL_CLOCK_V7_MISC_REF_DIV_SRC_GENLK;

		if (bp_params->signal_type == SIGNAL_TYPE_DVI_DUAL_LINK)
			clk.ucMiscInfo |= PIXEL_CLOCK_V7_MISC_DVI_DUALLINK_EN;

		if (EXEC_BIOS_CMD_TABLE(SetPixelClock, clk))
			result = BP_RESULT_OK;
	}
#endif
	return result;
}

static void init_set_pixel_clock(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(SetPixelClock)) {
	case 3:
		bp->cmd_tbl.set_pixel_clock = set_pixel_clock_v3;
		break;
	case 5:
		bp->cmd_tbl.set_pixel_clock = set_pixel_clock_v5;
		break;
	case 6:
		bp->cmd_tbl.set_pixel_clock = set_pixel_clock_v6;
		break;
	case 7:
		bp->cmd_tbl.set_pixel_clock = set_pixel_clock_v7;
		break;
	default:
		bp->cmd_tbl.set_pixel_clock = NULL;
		break;
	}
}

/*
 * DAC LOAD DETECTION
 */
static enum signal_type dac_load_detection_v3(
	struct bios_parser *bp,
	struct graphics_object_id encoder,
	struct graphics_object_id connector,
	enum signal_type display_signal)
{
	DAC_LOAD_DETECTION_PS_ALLOCATION params;
	enum signal_type signal = SIGNAL_TYPE_NONE;

	memset(&params, 0, sizeof(params));

	/* load detection is cupported for CRT, TV and CV */
	switch (display_signal) {
	case SIGNAL_TYPE_RGB:
		switch (display_graphics_object_id_get_encoder_id(encoder)) {
		case ENCODER_ID_INTERNAL_DAC1:
		case ENCODER_ID_INTERNAL_KLDSCP_DAC1:
			params.sDacload.usDeviceID =
				cpu_to_le16(ATOM_DEVICE_CRT1_SUPPORT);
			break;
		case ENCODER_ID_INTERNAL_DAC2:
		case ENCODER_ID_INTERNAL_KLDSCP_DAC2:
			params.sDacload.usDeviceID =
				cpu_to_le16(ATOM_DEVICE_CRT2_SUPPORT);
			break;
		default:
			break;
		}
		break;
		default:
			return signal;
	}

	/* set the encoder to detect on */
	switch (display_graphics_object_id_get_encoder_id(encoder)) {
	case ENCODER_ID_INTERNAL_DAC1:
	case ENCODER_ID_INTERNAL_KLDSCP_DAC1:
		params.sDacload.ucDacType = ATOM_DAC_A;
		break;
	case ENCODER_ID_INTERNAL_DAC2:
	case ENCODER_ID_INTERNAL_KLDSCP_DAC2:
		params.sDacload.ucDacType = ATOM_DAC_B;
		break;
	default:
		return signal;
	}

	if (!EXEC_BIOS_CMD_TABLE(DAC_LoadDetection, params))
		return signal;
#if 0 //TODO
#if defined(CONFIG_DRM_AMD_DAL_VBIOS_PRESENT)
	signal = bp->bios_helper->detect_sink(
			bp->ctx,
			encoder,
			connector,
			display_signal);
#else
	BREAK_TO_DEBUGGER(); /* VBios is needed */
#endif
#endif
	return signal;
}

static void init_dac_load_detection(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(DAC_LoadDetection)) {
	case 3:
		bp->cmd_tbl.dac_load_detection = dac_load_detection_v3;
		break;
	default:
		bp->cmd_tbl.dac_load_detection = NULL;
		break;
	}
}

/*
 * ENABLE CRTC
 */
static enum bp_result enable_crtc_v1(
	struct bios_parser *bp,
	enum controller_id controller_id,
	bool enable)
{
	bool result = BP_RESULT_FAILURE;
	ENABLE_CRTC_PARAMETERS params = {0};
	uint8_t id;

	if (cmd_table_helper_controller_id_to_atom(controller_id, &id))
		params.ucCRTC = id;
	else
		return BP_RESULT_BADINPUT;

	if (enable)
		params.ucEnable = ATOM_ENABLE;
	else
		params.ucEnable = ATOM_DISABLE;

	if (EXEC_BIOS_CMD_TABLE(EnableCRTC, params))
		result = BP_RESULT_OK;

	return result;
}

static void init_enable_crtc(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(EnableCRTC)) {
	case 1:
		bp->cmd_tbl.enable_crtc = enable_crtc_v1;
		break;
	default:
		bp->cmd_tbl.enable_crtc = NULL;
		break;
	}
}

/*
 * BLANK CRTC
 */
static enum bp_result blank_crtc_v1(struct bios_parser *bp,
				    struct bp_blank_crtc_parameters *bp_params,
				    bool blank)
{
	enum bp_result result = BP_RESULT_FAILURE;
	BLANK_CRTC_PARAMETERS params = {0};
	uint8_t atom_controller_id;

	if (cmd_table_helper_controller_id_to_atom(bp_params->controller_id,
						   &atom_controller_id)) {
		params.ucCRTC = (uint8_t)atom_controller_id;

		if (blank)
			params.ucBlanking = ATOM_BLANKING;
		else
			params.ucBlanking = ATOM_BLANKING_OFF;
		params.usBlackColorRCr =
				cpu_to_le16((uint16_t)bp_params->black_color_rcr);
		params.usBlackColorGY =
				cpu_to_le16((uint16_t)bp_params->black_color_gy);
		params.usBlackColorBCb =
				cpu_to_le16((uint16_t)bp_params->black_color_bcb);

		if (EXEC_BIOS_CMD_TABLE(BlankCRTC, params))
			result = BP_RESULT_OK;
	} else
		/* Not support more than two CRTC as current ASIC, update this
		 * if needed.
		 */
		result = BP_RESULT_BADINPUT;

	return result;
}

static void init_blank_crtc(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(BlankCRTC)) {
	case 1:
		bp->cmd_tbl.blank_crtc = blank_crtc_v1;
		break;
	default:
		bp->cmd_tbl.blank_crtc = NULL;
		break;
	}
}

/*
 *                  DISPLAY PLL
 */
static enum bp_result program_clock_v5(
	struct bios_parser *bp,
	struct bp_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;

	SET_PIXEL_CLOCK_PS_ALLOCATION_V5 params;
	uint32_t atom_pll_id;

	memset(&params, 0, sizeof(params));
	if (!bp->cmd_helper->clock_source_id_to_atom(
			bp_params->pll_id, &atom_pll_id)) {
		return BP_RESULT_BADINPUT;
	}

	/* We need to convert from KHz units into 10KHz units */
	params.sPCLKInput.ucPpll = (uint8_t) atom_pll_id;
	params.sPCLKInput.usPixelClock =
			cpu_to_le16((uint16_t) (bp_params->target_pixel_clock / 10));
	params.sPCLKInput.ucCRTC = (uint8_t) ATOM_CRTC_INVALID;

	if (bp_params->flags.SET_EXTERNAL_REF_DIV_SRC)
		params.sPCLKInput.ucMiscInfo |= PIXEL_CLOCK_MISC_REF_DIV_SRC;

	if (EXEC_BIOS_CMD_TABLE(SetPixelClock, params))
		result = BP_RESULT_OK;

	return result;
}

static enum bp_result program_clock_v6(
	struct bios_parser *bp,
	struct bp_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;

	SET_PIXEL_CLOCK_PS_ALLOCATION_V6 params;
	uint32_t atom_pll_id;

	memset(&params, 0, sizeof(params));

	if (!bp->cmd_helper->clock_source_id_to_atom(
			bp_params->pll_id, &atom_pll_id)) {
		return BP_RESULT_BADINPUT;
	}

	/* We need to convert from KHz units into 10KHz units */
	params.sPCLKInput.ucPpll = (uint8_t)atom_pll_id;
	params.sPCLKInput.ulDispEngClkFreq =
			cpu_to_le32(bp_params->target_pixel_clock / 10);

	if (bp_params->flags.SET_EXTERNAL_REF_DIV_SRC)
		params.sPCLKInput.ucMiscInfo |= PIXEL_CLOCK_MISC_REF_DIV_SRC;

	if (EXEC_BIOS_CMD_TABLE(SetPixelClock, params)) {
		/* True display clock is returned by VBIOS if DFS bypass
		 * is enabled. */
		bp_params->dfs_bypass_display_clock =
				(uint32_t)(le32_to_cpu(params.sPCLKInput.ulDispEngClkFreq) * 10);
		result = BP_RESULT_OK;
	}

	return result;
}

static void init_program_clock(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(SetPixelClock)) {
	case 5:
		bp->cmd_tbl.program_clock = program_clock_v5;
		break;
	case 6:
		bp->cmd_tbl.program_clock = program_clock_v6;
		break;
	default:
		bp->cmd_tbl.program_clock = NULL;
		break;
	}
}


/*
 * ENABLE DISPLAY POWER GATING
 */
static enum bp_result enable_disp_power_gating_v2_1(
	struct bios_parser *bp,
	enum controller_id crtc_id,
	enum bp_pipe_control_action action)
{
	enum bp_result result = BP_RESULT_FAILURE;

	ENABLE_DISP_POWER_GATING_PARAMETERS_V2_1 params = {0};
	uint8_t atom_crtc_id;

	if (cmd_table_helper_controller_id_to_atom(crtc_id, &atom_crtc_id))
		params.ucDispPipeId = atom_crtc_id;
	else
		return BP_RESULT_BADINPUT;

	params.ucEnable = cmd_table_helper_disp_power_gating_action_to_atom(action);

	if (EXEC_BIOS_CMD_TABLE(EnableDispPowerGating, params))
		result = BP_RESULT_OK;

	return result;
}

static void init_enable_disp_power_gating(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(EnableDispPowerGating)) {
	case 1:
		bp->cmd_tbl.enable_disp_power_gating =
			enable_disp_power_gating_v2_1;
		break;
	default:
		bp->cmd_tbl.enable_disp_power_gating = NULL;
		break;
	}
}

/*
 * ADJUST DISPLAY PLL
 */
static enum bp_result adjust_display_pll_v2(
	struct bios_parser *bp,
	struct bp_adjust_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;
	ADJUST_DISPLAY_PLL_PS_ALLOCATION params = { 0 };

	/* We need to convert from KHz units into 10KHz units and then convert
	 * output pixel clock back 10KHz-->KHz */
	uint32_t pixel_clock_10KHz_in = bp_params->pixel_clock / 10;

	params.usPixelClock = cpu_to_le16((uint16_t)(pixel_clock_10KHz_in));
	params.ucTransmitterID =
			cmd_table_helper_encoder_id_to_atom(
					display_graphics_object_id_get_encoder_id(
							bp_params->encoder_object_id));
	params.ucEncodeMode =
		(uint8_t)cmd_table_helper_encoder_mode_bp_to_atom(
					bp_params->signal_type, false);
	return result;
}

static enum bp_result adjust_display_pll_v3(
	struct bios_parser *bp,
	struct bp_adjust_pixel_clock_parameters *bp_params)
{
	enum bp_result result = BP_RESULT_FAILURE;
	ADJUST_DISPLAY_PLL_PS_ALLOCATION_V3 params;
	uint32_t pixel_clk_10_kHz_in = bp_params->pixel_clock / 10;

	memset(&params, 0, sizeof(params));

	/* We need to convert from KHz units into 10KHz units and then convert
	 * output pixel clock back 10KHz-->KHz */
	params.sInput.usPixelClock = cpu_to_le16((uint16_t)pixel_clk_10_kHz_in);
	params.sInput.ucTransmitterID =
		cmd_table_helper_encoder_id_to_atom(
			display_graphics_object_id_get_encoder_id(
				bp_params->encoder_object_id));
	params.sInput.ucEncodeMode =
		(uint8_t)cmd_table_helper_encoder_mode_bp_to_atom(
			bp_params->signal_type, false);

	if (bp_params->ss_enable == true)
		params.sInput.ucDispPllConfig |= DISPPLL_CONFIG_SS_ENABLE;

	if (bp_params->signal_type == SIGNAL_TYPE_DVI_DUAL_LINK)
		params.sInput.ucDispPllConfig |= DISPPLL_CONFIG_DUAL_LINK;

	if (EXEC_BIOS_CMD_TABLE(AdjustDisplayPll, params)) {
		/* Convert output pixel clock back 10KHz-->KHz: multiply
		 * original pixel clock in KHz by ratio
		 * [output pxlClk/input pxlClk] */
		uint64_t pixel_clk_10_khz_out =
				(uint64_t)le32_to_cpu(params.sOutput.ulDispPllFreq);
		uint64_t pixel_clk = (uint64_t)bp_params->pixel_clock;

		if (pixel_clk_10_kHz_in != 0) {
			bp_params->adjusted_pixel_clock =
					div_u64(pixel_clk * pixel_clk_10_khz_out,
							pixel_clk_10_kHz_in);
		} else {
			bp_params->adjusted_pixel_clock = 0;
		}

		bp_params->reference_divider = params.sOutput.ucRefDiv;
		bp_params->pixel_clock_post_divider = params.sOutput.ucPostDiv;

		result = BP_RESULT_OK;
	}

	return result;
}

static void init_adjust_display_pll(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(AdjustDisplayPll)) {
	case 2:
		bp->cmd_tbl.adjust_display_pll = adjust_display_pll_v2;
		break;
	case 3:
		bp->cmd_tbl.adjust_display_pll = adjust_display_pll_v3;
		break;
	default:
		bp->cmd_tbl.adjust_display_pll = NULL;
		break;
	}
}

/*
 * ENABLE PIXEL CLOCK SS
 */

static enum bp_result enable_spread_spectrum_on_ppll_v1(
	struct bios_parser *bp,
	struct bp_spread_spectrum_parameters *bp_params,
	bool enable)
{
	enum bp_result result = BP_RESULT_FAILURE;
	ENABLE_SPREAD_SPECTRUM_ON_PPLL params;

	memset(&params, 0, sizeof(params));

	if ((enable == true) && (bp_params->percentage > 0))
		params.ucEnable = ATOM_ENABLE;
	else
		params.ucEnable = ATOM_DISABLE;

	params.usSpreadSpectrumPercentage =
			cpu_to_le16((uint16_t)bp_params->percentage);
	params.ucSpreadSpectrumStep =
			(uint8_t)bp_params->ver1.step;
	params.ucSpreadSpectrumDelay =
			(uint8_t)bp_params->ver1.delay;
	/* convert back to unit of 10KHz */
	params.ucSpreadSpectrumRange =
			(uint8_t)(bp_params->ver1.range / 10000);

	if (bp_params->flags.EXTERNAL_SS)
		params.ucSpreadSpectrumType |= ATOM_EXTERNAL_SS_MASK;

	if (bp_params->flags.CENTER_SPREAD)
		params.ucSpreadSpectrumType |= ATOM_SS_CENTRE_SPREAD_MODE;

	if (bp_params->pll_id == CLOCK_SOURCE_ID_PLL1)
		params.ucPpll = ATOM_PPLL1;
	else if (bp_params->pll_id == CLOCK_SOURCE_ID_PLL2)
		params.ucPpll = ATOM_PPLL2;

	if (EXEC_BIOS_CMD_TABLE(EnableSpreadSpectrumOnPPLL, params))
		result = BP_RESULT_OK;

	return result;
}

static enum bp_result enable_spread_spectrum_on_ppll_v2(
	struct bios_parser *bp,
	struct bp_spread_spectrum_parameters *bp_params,
	bool enable)
{
	enum bp_result result = BP_RESULT_FAILURE;
	ENABLE_SPREAD_SPECTRUM_ON_PPLL_V2 params;

	memset(&params, 0, sizeof(params));

	if (bp_params->pll_id == CLOCK_SOURCE_ID_PLL1)
		params.ucSpreadSpectrumType = ATOM_PPLL_SS_TYPE_V2_P1PLL;
	else if (bp_params->pll_id == CLOCK_SOURCE_ID_PLL2)
		params.ucSpreadSpectrumType = ATOM_PPLL_SS_TYPE_V2_P2PLL;

	if ((enable == true) && (bp_params->percentage > 0)) {
		params.ucEnable = ATOM_ENABLE;

		params.usSpreadSpectrumPercentage =
				cpu_to_le16((uint16_t)(bp_params->percentage));
		params.usSpreadSpectrumStep =
				cpu_to_le16((uint16_t)(bp_params->ds.ds_frac_size));

		if (bp_params->flags.EXTERNAL_SS)
			params.ucSpreadSpectrumType |=
					ATOM_PPLL_SS_TYPE_V2_EXT_SPREAD;

		if (bp_params->flags.CENTER_SPREAD)
			params.ucSpreadSpectrumType |=
					ATOM_PPLL_SS_TYPE_V2_CENTRE_SPREAD;

		/* Both amounts need to be left shifted first before bit
		 * comparison. Otherwise, the result will always be zero here
		 */
		params.usSpreadSpectrumAmount = cpu_to_le16((uint16_t)(
				((bp_params->ds.feedback_amount <<
						ATOM_PPLL_SS_AMOUNT_V2_FBDIV_SHIFT) &
						ATOM_PPLL_SS_AMOUNT_V2_FBDIV_MASK) |
						((bp_params->ds.nfrac_amount <<
								ATOM_PPLL_SS_AMOUNT_V2_NFRAC_SHIFT) &
								ATOM_PPLL_SS_AMOUNT_V2_NFRAC_MASK)));
	} else
		params.ucEnable = ATOM_DISABLE;

	if (EXEC_BIOS_CMD_TABLE(EnableSpreadSpectrumOnPPLL, params))
		result = BP_RESULT_OK;

	return result;
}

static enum bp_result enable_spread_spectrum_on_ppll_v3(
	struct bios_parser *bp,
	struct bp_spread_spectrum_parameters *bp_params,
	bool enable)
{
	enum bp_result result = BP_RESULT_FAILURE;
	ENABLE_SPREAD_SPECTRUM_ON_PPLL_V3 params;

	memset(&params, 0, sizeof(params));

	switch (bp_params->pll_id) {
	case CLOCK_SOURCE_ID_PLL0:
		/* ATOM_PPLL_SS_TYPE_V3_P0PLL; this is pixel clock only,
		 * not for SI display clock.
		 */
		params.ucSpreadSpectrumType = ATOM_PPLL_SS_TYPE_V3_DCPLL;
		break;
	case CLOCK_SOURCE_ID_PLL1:
		params.ucSpreadSpectrumType = ATOM_PPLL_SS_TYPE_V3_P1PLL;
		break;

	case CLOCK_SOURCE_ID_PLL2:
		params.ucSpreadSpectrumType = ATOM_PPLL_SS_TYPE_V3_P2PLL;
		break;

	case CLOCK_SOURCE_ID_DCPLL:
		params.ucSpreadSpectrumType = ATOM_PPLL_SS_TYPE_V3_DCPLL;
		break;

	default:
		/* Unexpected PLL value!! */
		return result;
	}

	if (enable == true) {
		params.ucEnable = ATOM_ENABLE;

		params.usSpreadSpectrumAmountFrac =
				cpu_to_le16((uint16_t)(bp_params->ds_frac_amount));
		params.usSpreadSpectrumStep =
				cpu_to_le16((uint16_t)(bp_params->ds.ds_frac_size));

		if (bp_params->flags.EXTERNAL_SS)
			params.ucSpreadSpectrumType |=
					ATOM_PPLL_SS_TYPE_V3_EXT_SPREAD;
		if (bp_params->flags.CENTER_SPREAD)
			params.ucSpreadSpectrumType |=
					ATOM_PPLL_SS_TYPE_V3_CENTRE_SPREAD;

		/* Both amounts need to be left shifted first before bit
		 * comparison. Otherwise, the result will always be zero here
		 */
		params.usSpreadSpectrumAmount = cpu_to_le16((uint16_t)(
				((bp_params->ds.feedback_amount <<
						ATOM_PPLL_SS_AMOUNT_V3_FBDIV_SHIFT) &
						ATOM_PPLL_SS_AMOUNT_V3_FBDIV_MASK) |
						((bp_params->ds.nfrac_amount <<
								ATOM_PPLL_SS_AMOUNT_V3_NFRAC_SHIFT) &
								ATOM_PPLL_SS_AMOUNT_V3_NFRAC_MASK)));
	} else
		params.ucEnable = ATOM_DISABLE;

	if (EXEC_BIOS_CMD_TABLE(EnableSpreadSpectrumOnPPLL, params))
		result = BP_RESULT_OK;

	return result;
}

static void init_enable_spread_spectrum_on_ppll(struct bios_parser *bp)
{
	switch (BIOS_CMD_TABLE_PARA_REVISION(EnableSpreadSpectrumOnPPLL)) {
	case 1:
		bp->cmd_tbl.enable_spread_spectrum_on_ppll =
				enable_spread_spectrum_on_ppll_v1;
		break;
	case 2:
		bp->cmd_tbl.enable_spread_spectrum_on_ppll =
				enable_spread_spectrum_on_ppll_v2;
		break;
	case 3:
		bp->cmd_tbl.enable_spread_spectrum_on_ppll =
				enable_spread_spectrum_on_ppll_v3;
		break;
	default:
		bp->cmd_tbl.enable_spread_spectrum_on_ppll = NULL;
		break;
	}
}


void display_bios_parser_init_cmd_tbl(struct bios_parser *bp)
{
	init_set_pixel_clock(bp);
	init_enable_crtc(bp);
	init_dac_load_detection(bp);
	init_blank_crtc(bp);
	init_enable_disp_power_gating(bp);
	init_program_clock(bp);
	init_adjust_display_pll(bp);
	init_enable_spread_spectrum_on_ppll(bp);
}

void display_bios_parser_init_cmd_tbl_helper(const struct command_table_helper **h,
					     enum dce_version dce)
{
	switch (dce) {
	case DCE_VERSION_8_0:
		*h = display_cmd_tbl_helper_dce80_get_table();
		break;
	case DCE_VERSION_10_0:
	case DCE_VERSION_11_0:
		*h = display_cmd_tbl_helper_dce110_get_table();
	default:
		return;
	}
}
