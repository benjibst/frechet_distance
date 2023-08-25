#ifndef __FRECHET_DIST_TYPES_H__
#define __FRECHET_DIST_TYPES_H__

#include "geometry.h"
#include <stdint.h>

typedef enum {
	ENTRY_LEFT = 1<<0,
	ENTRY_BOTTOM = 1<<1,
	EXIT_RIGHT = 1<<2,
	EXIT_TOP = 1<<3,
}FD_fsp_entry_exit_bits;

typedef enum {
	FSP_NO_ENTRY = ~(ENTRY_BOTTOM|ENTRY_LEFT),
	FSP_NO_EXIT = ~(EXIT_RIGHT|EXIT_TOP),
	FSP_ENTRY_BOTH_EXIT_RIGHT = (ENTRY_LEFT|ENTRY_BOTTOM|EXIT_RIGHT),
	FSP_ENTRY_BOTH_EXIT_TOP = (ENTRY_BOTTOM|ENTRY_LEFT|EXIT_TOP),
	FSP_ENTRY_LEFT_EXIT_BOTH = (ENTRY_LEFT|EXIT_RIGHT|EXIT_TOP),
	FSP_ENTRY_BOTTOM_EXIT_BOTH = (ENTRY_BOTTOM|EXIT_TOP|EXIT_RIGHT),
	FSP_ENTRY_BOTH_EXIT_BOTH = (ENTRY_BOTTOM|ENTRY_LEFT|EXIT_TOP|EXIT_RIGHT),
}FD_fsp_pass;

typedef struct {
	FD_float a_ij, b_ij, c_ij, d_ij, a_ip1j, b_ip1j, c_ijp1, d_ijp1;
	FD_fsp_pass pass;
} FD_freespace;

#endif // __FRECHET_DIST_TYPES_H__