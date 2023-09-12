#include "frechet_dist.h"
#include <assert.h>
#include <time.h>

void FreeSpaceCellGridMaybeAlloc(FD_freespace_cell_grid* grid, uint32_t n_P, uint32_t n_Q)
{
	if (n_P * n_Q > grid->cap)
	{
		grid->cap = n_P * n_Q * 2;
		grid->cells = malloc(sizeof(FD_freespace_cell) * grid->cap);
		if (!grid->cells)
		{
			fprintf(stderr, "Failed reallocating freespace grid");
			abort();
		}
	}
	grid->n_segments_P = n_P;
	grid->n_segments_Q = n_Q;
	return;
}

void GetFreespaceCellGrid(FD_curve P, FD_curve Q, FD_float eps, FD_freespace_cell_grid* grid)
{
	time_t t1 = clock();
	FreeSpaceCellGridMaybeAlloc(grid, P.n_segments, Q.n_segments);
	for (size_t i = 0; i < P.n_segments; i++)
	{
		FD_segment seg_P = { P.points + i,P.points + i + 1 };
		for (size_t j = 0; j < Q.n_segments; j++)
		{
			FD_segment seg_Q = { Q.points + j,Q.points + j + 1 };
			GetFreespaceCell(seg_P, seg_Q, eps, grid->cells + i * Q.n_segments + j); //store cells [P|Q] [0|0][0|1]...[0|Q.n-1][1|0]
		}
	}
	time_t t2 = clock();
	printf("Computing fsp grid: %f\n",(float)(t2-t1)/CLOCKS_PER_SEC);
}

void GetFreespaceCell(FD_segment P, FD_segment Q, FD_float eps,
	FD_freespace_cell* const fsp)
{
	fsp->pass = 0;
	//0 and 1 correspond to a_ij and b_ij
	fsp->pass |= (GetFreeSpaceCellOneEdge(P.p1, Q, eps, &(fsp->fsp_vertices[0]), &(fsp->fsp_vertices[1]))) * ENTRY_LEFT;
	fsp->pass |= (GetFreeSpaceCellOneEdge(Q.p2, P, eps, &(fsp->fsp_vertices[2]), &(fsp->fsp_vertices[3]))) * EXIT_TOP;
	//4 is b_ij+1 5 is a_ij+1		  
	fsp->pass |= (GetFreeSpaceCellOneEdge(P.p2, Q, eps, &(fsp->fsp_vertices[5]), &(fsp->fsp_vertices[4]))) * EXIT_RIGHT;
	fsp->pass |= (GetFreeSpaceCellOneEdge(Q.p1, P, eps, &(fsp->fsp_vertices[7]), &(fsp->fsp_vertices[6]))) * ENTRY_BOTTOM;
}

bool GetFreeSpaceCellOneEdge(const FD_point* const p, const FD_segment seg, FD_float eps,
	FD_float* fsp_entry_range_begin, FD_float* fsp_entry_range_end)
{
	FD_point p1 = {0}, p2 = { 0 };
	switch (CircleLineIntersection(p, eps, seg, &p1, &p2))
	{
	case PASSANT:
		return false;
	case TANGENT: {
		FD_float scal = PointToParameter(p1, seg);
		if (scal >= 0 && scal <= 1)
		{
			*fsp_entry_range_begin = scal;
			*fsp_entry_range_end = scal;
			return true;
		}
		return false;
	}
	case SECANT: {
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
