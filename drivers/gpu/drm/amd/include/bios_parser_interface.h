#ifndef __DISPLAY_BIOS_PARSER_INTERFACE_H__
#define __DISPLAY_BIOS_PARSER_INTERFACE_H__

#include "display_types.h"
#include "display_grph_object_id.h"
#include "display_signal_types.h"
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

struct bp_blank_crtc_parameters {
        enum controller_id controller_id;
        uint32_t black_color_rcr;
        uint32_t black_color_gy;
        uint32_t black_color_bcb;
};

enum bp_pipe_control_action {
        ASIC_PIPE_DISABLE = 0,
        ASIC_PIPE_ENABLE,
        ASIC_PIPE_INIT
};

struct bp_adjust_pixel_clock_parameters {
	/* Input: Signal Type - to be converted to Encoder mode */
	enum signal_type signal_type;
	/* Input: Encoder object id */
	struct graphics_object_id encoder_object_id;
	/* Input: Pixel Clock (requested Pixel clock based on Video timing
	 * standard used) in KHz
	 */
	uint32_t pixel_clock;
	/* Output: Adjusted Pixel Clock (after VBIOS exec table) in KHz */
	uint32_t adjusted_pixel_clock;
	/* Output: If non-zero, this refDiv value should be used to calculate
	 * other ppll params */
	uint32_t reference_divider;
	/* Output: If non-zero, this postDiv value should be used to calculate
	 * other ppll params */
	uint32_t pixel_clock_post_divider;
	/* Input: Enable spread spectrum */
	bool ss_enable;
};

struct spread_spectrum_flags {
	/* 1 = Center Spread; 0 = down spread */
	uint32_t CENTER_SPREAD:1;
	/* 1 = external; 0 = internal */
	uint32_t EXTERNAL_SS:1;
	/* 1 = delta-sigma type parameter; 0 = ver1 */
	uint32_t DS_TYPE:1;
};

struct bp_spread_spectrum_parameters {
	enum clock_source_id pll_id;
	uint32_t percentage;
	uint32_t ds_frac_amount;

	union {
		struct {
			uint32_t step;
			uint32_t delay;
			uint32_t range; /* In Hz unit */
		} ver1;
		struct {
			uint32_t feedback_amount;
			uint32_t nfrac_amount;
			uint32_t ds_frac_size;
		} ds;
	};

	struct spread_spectrum_flags flags;
};

enum signal_type display_bios_dac_load_detect(struct display_bios *bios,
					      struct graphics_object_id encoder,
					      struct graphics_object_id connector,
					      enum signal_type display_signaxl);
enum bp_result display_bios_enable_crtc(struct display_bios *bios,
					enum controller_id id,
					bool enable);
enum bp_result display_bios_blank_crtc(struct display_bios *bios,
				       struct bp_blank_crtc_parameters *bp_params,
				       bool blank);
enum bp_result display_bios_enable_disp_power_gating(struct display_bios *bios,
						     enum controller_id controller_id,
						     enum bp_pipe_control_action action);
enum bp_result display_bios_adjust_pixel_clock(struct display_bios *bios,
					       struct bp_adjust_pixel_clock_parameters *bp_params);
enum bp_result display_bios_enable_spread_spectrum_on_ppll(struct display_bios *bios,
							   struct bp_spread_spectrum_parameters *bp_params,
							   bool enable);
#endif
