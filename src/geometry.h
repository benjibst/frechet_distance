#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <stddef.h>
#include <math.h>

#if SINGLE
typedef float FD_float;
#define FD_abs(x) fabsf(x)
#define FD_hyp(x, y) hypotf(x, y)
#define FD_sqrt(x) sqrtf(x)
#else
typedef double FD_float;
#define FD_abs(x) fabs(x)
#define FD_hyp(x, y) hypot(x, y)
#define FD_sqrt(x) sqrt(x)
#endif

typedef struct {
	FD_float x, y;
} FD_point;

typedef struct {
	FD_point *points;
	size_t n;
} FD_curve;

typedef struct {
	FD_point p1, p2;
} FD_segment;

FD_float OrthogonalDistance(FD_point p, FD_segment seg);
FD_point PointAlongSegment(FD_segment seg, FD_float p); //norm goes from 0 to 1

#endif // __GEOMETRY_H__