#include "geometry.h"
#include <assert.h>

FD_float OrthogonalDistance(const FD_point* const p, FD_segment seg) {
	FD_float x2mx1 = seg.p2->x - seg.p1->x;
	FD_float y2my1 = seg.p2->y - seg.p1->y;
	FD_float y1my0 = seg.p1->y - p->y;
	FD_float x1mx0 = seg.p1->x - p->x;
	return Abs(x2mx1 * y1my0 - x1mx0 * y2my1) / Hypot(x2mx1, y2my1);
}

static inline int Sign(FD_float f) { return (f >= 0) - (f < 0); }

// https://mathworld.wolfram.com/Circle-LineIntersection.html
//if the circle and the line do not intersect return PASSANT
//if the line is tangent to the circle return TANGENT and store the tangential point in p1
//if the line crosses the circle return SECANT and store both intersection points in p1 and p2
FD_line_circle_intersection_type CircleLineIntersection(const FD_point* const center, FD_float eps,
	FD_segment seg, FD_point* p1,
	FD_point* p2) {
	FD_float orthdist = OrthogonalDistance(center, seg);
	if (orthdist > eps)
		return PASSANT;
	FD_float dx = seg.p2->x - seg.p1->x;
	FD_float dy = seg.p2->y - seg.p1->y;
	FD_float drsquared = dx * dx + dy * dy;
	//shift points because the calculation assumes the circle is centered at [0|0]
	FD_float D = (seg.p1->x - center->x) * (seg.p2->y - center->y) - (seg.p2->x - center->x) * (seg.p1->y - center->y); 
	if (orthdist == eps) {
		p1->x = (D * dy) / drsquared + center->x;
		p1->y = (-D * dx) / drsquared + center->y;
		return TANGENT;
	}
	FD_float sqrtdisc = Sqrt(eps * eps * drsquared - D * D);
	FD_float Ddy = D * dy;
	FD_float addx = Sign(dy) * dx * sqrtdisc;
	p1->x = (Ddy + addx) / drsquared + center->x;
	p2->x = (Ddy - addx) / drsquared + center->x;
	FD_float mDdx = -D * dx;
	FD_float addy = Abs(dy) * sqrtdisc;
	p1->y = (mDdx + addy) / drsquared + center->y;
	p2->y = (mDdx - addy) / drsquared + center->y;
	return SECANT;
}

// assumes that the point p lies along the line seg and
// returns how far along the line the point is.
// if p is equal to seg.p2 returns 1, if p is equal to seg.p1 returns 0
FD_float PointToParameter(FD_point p, FD_segment seg) {
	if (p.x == seg.p1->x && p.y == seg.p1->y)
		return 0;
	if (p.x == seg.p2->x && p.y == seg.p2->y)
		return 1;
	FD_float seglen = Hypot(seg.p2->x - seg.p1->x, seg.p2->y - seg.p1->y);
	FD_float p_to_p1_len = Hypot(p.x - seg.p1->x, p.y - seg.p1->y);
	FD_float lenratio = p_to_p1_len / seglen;
	//if the point lies "behind" the starting point
	if ((p.x - seg.p1->x) * (seg.p2->x - seg.p1->x) < 0 || (p.y - seg.p1->y) * (seg.p2->y - seg.p1->y) < 0)
		lenratio = -lenratio;
	return lenratio;
}

FD_curve AllocateCurve(uint32_t cap)
{
	FD_curve c;
	c.points = malloc(sizeof(FD_point) * cap);
	c.n_points = 0;
	c.capacity = cap;
	c.n_segments = 0;
	return c;
}

void AddPointToCurve(FD_curve* c, FD_point p)
{
	if (c->capacity == c->n_points)
	{
		void* points_new = realloc(c->points, sizeof(FD_point) * 2 * c->capacity);
		if (!points_new)
		{
			fprintf(stderr, "Failed reallocating curve");
			abort();
		}
		c->capacity *= 2;
		c->points = points_new;
	}
	c->points[c->n_points] = p;
	c->n_points++;
	if (c->n_points > 1)
		c->n_segments++;
}

void FreeCurve(FD_curve* c)
{
	if (c->points)
		free(c->points);
	c->points = NULL;
}

// if p is 0 returns seg.p1-> if p is 1 returns seg.p2->
FD_point ParameterToPoint(FD_segment seg, FD_float p) {
	return (FD_point) {
		seg.p1->x* (1.0 - p) + seg.p2->x * p,
			seg.p1->y* (1.0 - p) + seg.p2->y * p
	};
}