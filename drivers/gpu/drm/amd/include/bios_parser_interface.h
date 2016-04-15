#ifndef __DISPLAY_BIOS_PARSER_INTERFACE_H__
#define __DISPLAY_BIOS_PARSER_INTERFACE_H__

#include "display_types.h"
#include "display_grph_object_id.h"

struct bp_init_data {
	struct cgs_device *cgs;
	uint8_t *bios;
};

struct display_bios;

struct display_bios *display_bios_parser_create(struct bp_init_data *init,
						enum dce_version dce_version);

void display_bios_parser_destroy(struct display_bios *dcb);

enum bp_result {
	BP_RESULT_OK = 0, /* There was no error */
	BP_RESULT_BADINPUT, /*Bad input parameter */
	BP_RESULT_BADBIOSTABLE, /* Bad BIOS table */
	BP_RESULT_UNSUPPORTED, /* BIOS Table is not supported */
	BP_RESULT_NORECORD, /* Record can't be found */
	BP_RESULT_FAILURE
};

enum bp_result display_bios_enable_crtc(struct display_bios *bios,
					enum controller_id id,
					bool enable);

#endif
