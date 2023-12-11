#include "geometry.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

double OrthogonalDistance(const Point2d *const p, Segment seg)
{
    double x2mx1 = seg.B->x - seg.A->x;
    double y2my1 = seg.B->y - seg.A->y;
    double y1my0 = seg.A->y - p->y;
    double x1mx0 = seg.A->x - p->x;
    return fabs(x2mx1 * y1my0 - x1mx0 * y2my1) / hypot(x2mx1, y2my1);
}

double Distance(const Point2d *const p1, const Point2d *const p2)
{
    return hypot(p1->x - p2->x, p1->y - p2->y);
}

static inline int Sign(double f)
{
    return (f >= 0) - (f < 0);
}

LineCircleIntersectType
CircleLineIntersection(const Point2d *const center, double eps, Segment seg, Point2d *p1, Point2d *p2)
{
    double orthdist = OrthogonalDistance(center, seg);
    if (orthdist > eps)
        return PASSANT;
    double dx = seg.B->x - seg.A->x;
    double dy = seg.B->y - seg.A->y;
    double drsquared = dx * dx + dy * dy;
    //shift points because the calculation assumes the circle is centered at [0|0]
    double D = (seg.A->x - center->x) * (seg.B->y - center->y) - (seg.B->x - center->x) * (seg.A->y - center->y);
    if (orthdist == eps) {
        p1->x = (D * dy) / drsquared + center->x;
        p1->y = (-D * dx) / drsquared + center->y;
        return TANGENT;
    }
    double sqrtdisc = sqrt(eps * eps * drsquared - D * D);
    double Ddy = D * dy;
    double addx = Sign(dy) * dx * sqrtdisc;
    p1->x = (Ddy + addx) / drsquared + center->x;
    p2->x = (Ddy - addx) / drsquared + center->x;
    double mDdx = -D * dx;
    double addy = fabs(dy) * sqrtdisc;
    p1->y = (mDdx + addy) / drsquared + center->y;
    p2->y = (mDdx - addy) / drsquared + center->y;
    return SECANT;
}

// assumes that the point p lies along the line seg and
// returns how far along the line the point is.
// if p is equal to seg.p2 returns 1, if p is equal to seg.p1 returns 0
double PointToParameter(Point2d p, Segment seg)
{
    if (p.x == seg.A->x && p.y == seg.A->y)
        return 0;
    if (p.x == seg.B->x && p.y == seg.B->y)
        return 1;
    double seglen = hypot(seg.B->x - seg.A->x, seg.B->y - seg.A->y);
    double p_to_p1_len = hypot(p.x - seg.A->x, p.y - seg.A->y);
    double lenratio = p_to_p1_len / seglen;
    //if the point lies "behind" the starting point
    if ((p.x - seg.A->x) * (seg.B->x - seg.A->x) < 0 || (p.y - seg.A->y) * (seg.B->y - seg.A->y) < 0)
        lenratio = -lenratio;
    return lenratio;
}

Curve AllocateCurve(uint32_t cap)
{
    Curve c;
    c.points = malloc(cap * sizeof(Point2d));
    if (!c.points) {
        fprintf(stderr, "Failed allocating curve");
        exit(1);
    }
    c.n_points = 0;
    c.xy_min = (Point2d) {DBL_MAX, DBL_MAX}; //2 corners of bounding rect
    c.xy_max = (Point2d) {-DBL_MAX, -DBL_MAX};
    c.capacity = cap;
    c.n_segments = 0;
    return c;
}

void AddPointToCurve(Curve *c, Point2d p)
{
    if (c->n_points && (c->points[c->n_points - 1].x == p.x) && (c->points[c->n_points - 1].y == p.y))
        return; //dont add repeated points
    if (c->capacity == c->n_points) {
        void *points_new = realloc(c->points, sizeof(Point2d) * 2 * c->capacity);
        if (!points_new) {
            fprintf(stderr, "Failed reallocating curve");
            exit(1);
        }
        c->capacity *= 2;
        c->points = points_new;
    }
    if (p.x < c->xy_min.x)
        c->xy_min.x = p.x;
    if (p.y < c->xy_min.y)
        c->xy_min.y = p.y;
    if (p.x > c->xy_max.x)
        c->xy_max.x = p.x;
    if (p.y > c->xy_max.y)
        c->xy_max.y = p.y;
    c->points[c->n_points] = p;
    c->n_points++;
    if (c->n_points >= 2)
        c->n_segments++;
}

void FreeCurve(Curve *c)
{
    if (c->points)
        free(c->points);
    memset(c, 0, sizeof(Curve));
}

// if param is 0 returns seg.p1-> if param is 1 returns seg.p2->
Point2d ParameterToPoint(Segment seg, double param)
{
    return (Point2d) {seg.A->x * (1.0 - param) + seg.B->x * param, seg.A->y * (1.0 - param) + seg.B->y * param};
}
