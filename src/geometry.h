#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <stddef.h>
#include <math.h>
#include <stdbool.h>

#if SINGLE
typedef float FD_float;
#define Abs(x) fabsf(x)
#define Hypot(x, y) hypotf(x, y)
#define Sqrt(x) sqrtf(x)
#else
typedef double FD_float;
#define Abs(x) fabs(x)
#define Hypot(x, y) hypot(x, y)
#define Sqrt(x) sqrt(x)
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

typedef enum {
	PASSANT = 1,
	TANGENT,
	SECANT,
} FD_line_circle_intersection;

FD_float OrthogonalDistance(FD_point p, FD_segment seg);
FD_point PointAlongSegment(FD_segment seg, FD_float p); //norm goes from 0 to 1
FD_line_circle_intersection CircleLineIntersection(FD_point center,FD_float eps,FD_segment seg,FD_point* p1,FD_point* p2);
bool IsPointInSegment(FD_point p,FD_segment seg);

#endif // __GEOMETRY_H__