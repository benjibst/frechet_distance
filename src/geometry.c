#include "geometry.h"
#include <math.h>

FD_float OrthogonalDistance(FD_point p, const FD_segment *const seg) {
	FD_float x2mx1 = seg->p2.x - seg->p1.x;
	FD_float y2my1 = seg->p2.y - seg->p1.y;
	FD_float y1my0 = seg->p1.y - p.y;
	FD_float x1mx0 = seg->p1.x - p.x;
	return FD_abs(x2mx1 * y1my0 - x1mx0 * y2my1) / FD_hyp(x2mx1, y2my1);
}

FD_point PointAlongSegment(FD_segment seg, FD_float p)
{
	return (FD_point){seg.p1.x*(1.0-p) + seg.p2.x*p,seg.p1.y*(1.0-p) + seg.p2.y*p};
}