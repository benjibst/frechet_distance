#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <math.h>
#include <stdint.h>

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

typedef struct point {
	FD_float x, y;
} FD_point;

typedef struct curve {
	FD_point* points;
	uint32_t n_points;
	uint32_t n_segments;
	uint32_t capacity;
} FD_curve;

typedef struct segment {
	FD_point* p1, * p2;
} FD_segment;

typedef enum line_circle_intersection_type {
	PASSANT = 1,
	TANGENT,
	SECANT,
} FD_line_circle_intersection_type;

FD_float OrthogonalDistance(const FD_point* const p, FD_segment seg);
FD_point ParameterToPoint(FD_segment seg, FD_float p); //norm goes from 0 to 1
FD_line_circle_intersection_type CircleLineIntersection(const FD_point* const center, FD_float eps, FD_segment seg, FD_point* p1, FD_point* p2);
FD_float PointToParameter(FD_point p, FD_segment);
FD_curve AllocateCurve(uint32_t cap);
void AddPointToCurve(FD_curve* c, FD_point p);
void FreeCurve(FD_curve* c);

#endif // __GEOMETRY_H__
