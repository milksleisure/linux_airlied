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
	init_enable_crtc(bp);
	init_dac_load_detection(bp);
	init_blank_crtc(bp);
	init_enable_disp_power_gating(bp);
	init_adjust_display_pll(bp);
	init_enable_spread_spectrum_on_ppll(bp);
}
