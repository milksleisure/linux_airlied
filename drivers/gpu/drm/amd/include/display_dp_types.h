#ifndef __DISPLAY_DP_TYPES_H__
#define __DISPLAY_DP_TYPES_H__

enum dc_lane_count {
	LANE_COUNT_UNKNOWN = 0,
	LANE_COUNT_ONE = 1,
	LANE_COUNT_TWO = 2,
	LANE_COUNT_FOUR = 4,
	LANE_COUNT_EIGHT = 8,
	LANE_COUNT_DP_MAX = LANE_COUNT_FOUR
};

/* This is actually a reference clock (27MHz) multiplier
 * 162MBps bandwidth for 1.62GHz like rate,
 * 270MBps for 2.70GHz,
 * 324MBps for 3.24Ghz,
 * 540MBps for 5.40GHz
 * 810MBps for 8.10GHz
 */
enum dc_link_rate {
	LINK_RATE_UNKNOWN = 0,
	LINK_RATE_LOW = 0x06,
	LINK_RATE_HIGH = 0x0A,
	LINK_RATE_RBR2 = 0x0C,
	LINK_RATE_HIGH2 = 0x14,
	LINK_RATE_HIGH3 = 0x1E
};


#endif
