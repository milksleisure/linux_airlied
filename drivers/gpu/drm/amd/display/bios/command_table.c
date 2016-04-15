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


void display_bios_parser_init_cmd_tbl(struct bios_parser *bp)
{
	init_enable_crtc(bp);
	init_blank_crtc(bp);
	init_enable_disp_power_gating(bp);
}
