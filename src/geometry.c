#include "geometry.h"
#include <math.h>

FD_float OrthogonalDistance(FD_point p, FD_segment seg) {
	FD_float x2mx1 = seg.p2.x - seg.p1.x;
	FD_float y2my1 = seg.p2.y - seg.p1.y;
	FD_float y1my0 = seg.p1.y - p.y;
	FD_float x1mx0 = seg.p1.x - p.x;
	return Abs(x2mx1 * y1my0 - x1mx0 * y2my1) / Hypot(x2mx1, y2my1);
}

static inline int Sign(FD_float f) { return (f >= 0) - (f < 0); }

// https://mathworld.wolfram.com/Circle-LineIntersection.html
FD_line_circle_intersection CircleLineIntersection(FD_point center, FD_float eps,
												   FD_segment seg, FD_point *p1,
												   FD_point *p2) {
	FD_float orthdist = OrthogonalDistance(center, seg);
	if (orthdist > eps)
		return PASSANT;
	seg.p1.x -= center.x; // shift the points because calculation assumes the circle is
						  // centered at [0 | 0]
	seg.p2.x -= center.x;
	seg.p1.y -= center.y;
	seg.p2.y -= center.y;
	FD_float dx = seg.p2.x - seg.p1.x;
	FD_float dy = seg.p2.y - seg.p1.y;
	FD_float drsquared = dx * dx + dy * dy;
	FD_float D = seg.p1.x * seg.p2.y - seg.p2.x * seg.p1.y;
	FD_float sqrtdisc = Sqrt(eps * eps * drsquared - D * D);
	if (orthdist == eps) {
		p1->x = (D * dy) / drsquared + center.x;
		p1->y = (-D * dx) / drsquared + center.y;
		return TANGENT;
	}
	FD_float Ddy = D * dy;
	FD_float addx = Sign(dy) * dx * sqrtdisc;
	p1->x = (Ddy + addx) / drsquared + center.x;
	p2->x = (Ddy - addx) / drsquared + center.x;
	FD_float mDdx = -D * dx;
	FD_float addy = Abs(dy) * sqrtdisc;
	p1->y = (mDdx + addy) / drsquared + center.y;
	p2->y = (mDdx - addy) / drsquared + center.y;
	return SECANT;
}

bool IsPointInSegment(FD_point p, FD_segment seg) {
	FD_float dxseg = seg.p2.x - seg.p1.x;
	FD_float dyseg = seg.p2.y - seg.p1.y;
	FD_float dxp = p.x - seg.p1.x;
	FD_float dyp = p.y - seg.p1.y;
	FD_float dxratio = dxp / dxseg;
	FD_float dyratio = dyp / dyseg;
	// tolerance determined empirically such that points calculated by circle line
	// intersection will return true
	return (Abs(dxratio - dyratio) < 0.000001 && dxratio <= 1 && dxratio >= 0 &&
			dyratio >= 0 && dyratio <= 1);
}

FD_point PointAlongSegment(FD_segment seg, FD_float p) {
	return (FD_point){seg.p1.x * (1.0 - p) + seg.p2.x * p,
					  seg.p1.y * (1.0 - p) + seg.p2.y * p};
}