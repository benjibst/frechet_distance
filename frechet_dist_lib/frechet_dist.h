#ifndef FRECHET_DIST_H_
#define FRECHET_DIST_H_

#include "geometry.h"
#include <stdbool.h>
#include <stddef.h>

#define FRECHET_DIST_APPROX_STEPS 20 //magic number determined to be precise enough for 5 significant digits

#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

typedef struct {
    float entry, exit;
} FreeSpaceEdge;

typedef struct {
    uint32_t x, y;
} FreespaceCellPos;

typedef struct {
    // Edges are stored like this in memory
    // 10---11---     Vertical edges are Q line segments
    // 1|   3|  5|    Horizontal edges are P line segments
    //  8--- 9---
    // 0|   2|  4|
    //  6--- 7---
    //store edges and reachable state for every edge separately to save memory because of alignment
    //stores the beginning of the Q and P edges (they are after each other in memory)
    FreeSpaceEdge *edgesQ;
    FreeSpaceEdge *edgesP; //stores the beginning of the P edges
    //stores the beginning of the Q and P edges (they are after each other in memory)
    uint8_t *reachableQ;
    uint8_t *reachableP;
    uint32_t n_points_Q;
    uint32_t n_points_P;
    size_t cap; //the number of edges in the free space diagram
} FreeSpaceEdgeData;

//Calculate the free space diagram
void GetFreespaceEdgeData(Curve P, Curve Q, double eps, FreeSpaceEdgeData *edges);

void FreeEdgeData(FreeSpaceEdgeData *data);

//checks if a forward walking path from beginning to end is possible in the given free space diagram
bool FrechetDistLeqEps(FreeSpaceEdgeData *edges);

//Computes the Frechet distance for curves P and Q and stores the edge data of the
//free space grid into data
double ComputeFrechetDistance(Curve P, Curve Q, FreeSpaceEdgeData *data);

#endif // FRECHET_DIST_H_
