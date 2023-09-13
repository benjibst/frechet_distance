#ifndef __FRECHET_DIST_H__
#define __FRECHET_DIST_H__

#include "geometry.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef __WIN32__
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))
#endif

typedef enum fsp_entry_exit_bits
{
	ENTRY_LEFT = 1 << 0,
	EXIT_TOP = 1 << 1,
	EXIT_RIGHT = 1 << 2,
	ENTRY_BOTTOM = 1 << 3,
} FD_fsp_entry_exit_bits;

typedef struct freespace_edge
{
	FD_float entry, exit;
} FD_freespace_edge;

typedef struct freespace_edges
{
	FD_freespace_edge *edges; // stores the edges of the free space diagram
	uint32_t n_points_P;
	uint32_t n_points_Q;
	size_t cap;
	uint8_t *reachable; // for every edge stores if entry or exit from that edge is possible
} FD_freespace_edge_data;

void FreeSpaceEdgesMaybeAlloc(FD_freespace_edge_data *edges, uint32_t n_points_P, uint32_t n_points_Q);

void GetFreespaceEdgeData(FD_curve P, FD_curve Q, FD_float eps, FD_freespace_edge_data *edges);

// sets freespace entry or exit interval for one side and returns if entry or exit from that side is possible
bool GetFreeSpaceCellOneEdge(const FD_point *const p, const FD_segment seg, FD_float eps,
							 FD_float *fsp_entry_range_begin, FD_float *fsp_entry_range_end);
#endif // __FRECHET_DIST_H__