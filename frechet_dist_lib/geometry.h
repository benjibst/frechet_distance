#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <stdint.h>

typedef struct {
    double x, y;
} Point2d;

typedef struct {
    Point2d *points;
    Point2d xy_min;
    Point2d xy_max;
    uint32_t n_points;
    uint32_t n_segments;
    uint32_t capacity;
} Curve;

typedef struct {
    Point2d *A, *B;
} Segment;

typedef enum {
    PASSANT = 1, TANGENT, SECANT,
} LineCircleIntersectType;

double OrthogonalDistance(const Point2d *p, Segment seg);

//returns A+vec(AB)*param
Point2d ParameterToPoint(Segment seg, double param);

double Distance(const Point2d *p1, const Point2d *p2);

// https://mathworld.wolfram.com/Circle-LineIntersection.html
//if the circle and the line do not intersect return PASSANT
//if the line is tangent to the circle return TANGENT and store the tangential point in p1
//if the line crosses the circle return SECANT and store both intersection points in p1 and p2
LineCircleIntersectType
CircleLineIntersection(const Point2d *center, double eps, Segment seg, Point2d *p1, Point2d *p2);

//returns how far along the line the point p is.
//if p is A returns 0, if p is B returns 1
double PointToParameter(Point2d p, Segment);

//allocate a curve with capacity
Curve AllocateCurve(uint32_t cap);

void AddPointToCurve(Curve *c, Point2d p);

void FreeCurve(Curve *c);

#endif // GEOMETRY_H_
