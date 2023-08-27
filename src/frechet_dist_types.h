#ifndef __FRECHET_DIST_TYPES_H__
#define __FRECHET_DIST_TYPES_H__

#include "geometry.h"
#include <stdint.h>

typedef enum
{
	ENTRY_LEFT = 1 << 0,
	EXIT_TOP = 1 << 1,
	EXIT_RIGHT = 1 << 2,
	ENTRY_BOTTOM = 1 << 3,
} FD_fsp_entry_exit_bits;

typedef struct
{
	// start at 0 = a_ij, then go clockwise. 7 = c_ij
	FD_float fsp_vertices[8];
	FD_fsp_entry_exit_bits pass;
} FD_freespace;

#endif // __FRECHET_DIST_TYPES_H__