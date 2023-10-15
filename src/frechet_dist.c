#include "frechet_dist.h"
#include <stdio.h>
#include <stdlib.h>

void FreeSpaceEdgesMaybeAlloc(FD_freespace_edge_data *edge_data, uint32_t n_points_P, uint32_t n_points_Q)
{
	size_t n_edges = (n_points_P - 1) * n_points_Q + (n_points_Q - 1) * n_points_P;
	if (n_edges > edge_data->cap)
	{
		edge_data->cap = 2 * n_edges;
		edge_data->edges = malloc(edge_data->cap * sizeof(FD_freespace_edge));
		edge_data->reachable = malloc(edge_data->cap * sizeof(uint8_t));
		if (!edge_data->edges || !edge_data->reachable)
		{
			fprintf(stderr, "Failed reallocating freespace grid");
			abort();
		}
	}
	edge_data->n_points_P = n_points_P;
	edge_data->n_points_Q = n_points_Q;
}

void GetFreespaceEdgeData(FD_curve P, FD_curve Q, FD_float eps, FD_freespace_edge_data *edge_data)
{
	// Edges are stored like this in memory
	// 10---11---
	// 1|   3|  5|
	//  8--- 9---
	// 0|   2|  4|
	//  6--- 7---
	uint32_t index = 0;
	FreeSpaceEdgesMaybeAlloc(edge_data, P.n_points, Q.n_points);
	for (uint32_t i = 0; i < P.n_points; i++) // for every point of P and every segment of Q
	{
		for (uint32_t j = 0; j < Q.n_points - 1; j++)
		{
			FD_segment seg = {Q.points + j, Q.points + j + 1};
			edge_data->reachable[index] = GetFreeSpaceCellOneEdge(P.points + i, seg, eps, &(edge_data->edges[index].entry), &(edge_data->edges[index].exit));
			index++;
		}
	}
	for (uint32_t i = 0; i < Q.n_points; i++)
	{
		for (uint32_t j = 0; j < P.n_points-1; j++)
		{
			FD_segment seg = {P.points + j, P.points + j + 1};
			edge_data->reachable[index] = GetFreeSpaceCellOneEdge(Q.points + i, seg, eps, &(edge_data->edges[index].entry), &(edge_data->edges[index].exit));
 			index++;
		}
	}
}

bool GetFreeSpaceCellOneEdge(FD_point* p, FD_segment seg, FD_float eps,
							 FD_float *fsp_entry_range_begin, FD_float *fsp_entry_range_end)
{
	FD_point p1 = {0}, p2 = {0};
	switch (CircleLineIntersection(p, eps, seg, &p1, &p2))
	{
	case PASSANT:
		return false;
	case TANGENT:
	{
		FD_float scal = PointToParameter(p1, seg);
		if (scal >= 0 && scal <= 1)
		{
			*fsp_entry_range_begin = scal;
			*fsp_entry_range_end = scal;
			return true;
		}
		return false;
	}
	case SECANT:
	{
		FD_float scal1 = PointToParameter(p1, seg);
		FD_float scal2 = PointToParameter(p2, seg);
		if (scal1 > scal2) // swap if order is wrong
		{
			FD_float tmp = scal1;
			scal1 = scal2;
			scal2 = tmp;
		}
		if (scal2 < 0 || scal1 > 1) // both points are outside the valid 0..1 range
		{
			return false;
		}
		*fsp_entry_range_begin = scal1 > 0 ? scal1 : 0;
		*fsp_entry_range_end = scal2 < 1 ? scal2 : 1;
		return true;
	}
	}
	return false;
}
