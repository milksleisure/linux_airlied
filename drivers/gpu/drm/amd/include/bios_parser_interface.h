#ifndef __DISPLAY_BIOS_PARSER_INTERFACE_H__
#define __DISPLAY_BIOS_PARSER_INTERFACE_H__

#include "display_types.h"

struct bp_init_data {
	struct cgs_device *cgs;
	uint8_t *bios;
};

struct display_bios;

struct display_bios *display_bios_parser_create(struct bp_init_data *init,
						enum dce_version dce_version);

void display_bios_parser_destroy(struct display_bios *dcb);

#endif
