#include "frechet_dist.h"

void GetFreespace(FD_segment P, FD_segment Q, FD_float eps,
				  FD_freespace *const fsp)
{
	fsp->pass = 0;
	//0 and 1 correspond to a_ij and b_ij
	fsp->pass |= (GetFreeSpaceOneSide(P.p1, Q, eps, &(fsp->fsp_vertices[0]), &(fsp->fsp_vertices[1]))) * ENTRY_LEFT;
	fsp->pass |= (GetFreeSpaceOneSide(Q.p2, P, eps, &(fsp->fsp_vertices[2]), &(fsp->fsp_vertices[3]))) * EXIT_TOP;
	//4 is b_ij+1 5 is a_ij+1
	fsp->pass |= (GetFreeSpaceOneSide(P.p2, Q, eps, &(fsp->fsp_vertices[5]), &(fsp->fsp_vertices[4]))) * EXIT_RIGHT;
	fsp->pass |= (GetFreeSpaceOneSide(Q.p1, P, eps, &(fsp->fsp_vertices[7]), &(fsp->fsp_vertices[6]))) * ENTRY_BOTTOM;
}

bool GetFreeSpaceOneSide(FD_point p, FD_segment seg, FD_float eps,
						 FD_float *fsp_entry_range_begin, FD_float *fsp_entry_range_end)
{
	FD_point p1, p2;
	switch (CircleLineIntersection(p, eps, seg, &p1, &p2))
	{
	case PASSANT:
		return false;
	case TANGENT:
		FD_float scal = PointToParameter(p1, seg);
		if (scal >= 0 && scal <= 1)
		{
			*fsp_entry_range_begin = scal;
			*fsp_entry_range_end = scal;
			return true;
		}
		return false;
	case SECANT:
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
	return false;
}
