#ifndef __FRECHET_DIST_H__
#define __FRECHET_DIST_H__

#include "geometry.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef __WIN32__
#define Min(x,y) ((x)<(y)?(x):(y))
#define Max(x,y) ((x)>(y)?(x):(y))
#endif

typedef enum fsp_entry_exit_bits
{
	ENTRY_LEFT = 1 << 0,
	EXIT_TOP = 1 << 1,
	EXIT_RIGHT = 1 << 2,
	ENTRY_BOTTOM = 1 << 3,
} FD_fsp_entry_exit_bits;

typedef struct freespace_cell
{
	// start at 0 = a_ij, then go clockwise. 7 = c_ij
	FD_float fsp_vertices[8];
	FD_fsp_entry_exit_bits pass;
} FD_freespace_cell;

typedef struct freespace_cell_grid
{
	FD_freespace_cell* cells;
	uint32_t n_segments_P;
	uint32_t n_segments_Q;
	size_t cap;
} FD_freespace_cell_grid;

void FreeSpaceCellGridMaybeAlloc(FD_freespace_cell_grid* grid, uint32_t n_P, uint32_t n_Q);

void GetFreespaceCellGrid(FD_curve P, FD_curve Q, FD_float eps, FD_freespace_cell_grid* grid);

//computes a free space cell for 2 line segments with a given epsilon
void GetFreespaceCell(FD_segment P, FD_segment Q, FD_float eps,
	FD_freespace_cell* const fsp);

//sets freespace entry or exit interval for one side and returns if entry or exit from that side is possible
bool GetFreeSpaceCellOneEdge(const FD_point* const p, const FD_segment seg, FD_float eps,
	FD_float* fsp_entry_range_begin, FD_float* fsp_entry_range_end);
#endif // __FRECHET_DIST_H__