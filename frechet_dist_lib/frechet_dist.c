#include "frechet_dist.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//if the size needed exceeds the current capacity reallocate the freespace edge data
static void FreeSpaceEdgesMaybeAlloc(FreeSpaceEdgeData *edge_data, uint32_t np_P, uint32_t np_Q) {
    size_t n_edges = (np_P - 1) * np_Q + (np_Q - 1) * np_P;
    if (n_edges > edge_data->cap) {
        edge_data->cap = 2 * n_edges;
        void *edges_tmp = realloc(edge_data->edgesQ, edge_data->cap * sizeof(FreeSpaceEdge));
        void *reachable_tmp = realloc(edge_data->reachableQ, edge_data->cap * sizeof(uint8_t));
        if (!edges_tmp || !reachable_tmp) {
            fprintf(stderr, "Failed reallocating freespace grid");
            abort();
        } else {
            edge_data->edgesQ = edges_tmp;
            edge_data->reachableQ = reachable_tmp;
        }
    }
    edge_data->edgesP = edge_data->edgesQ + (np_Q - 1) * np_P;
    edge_data->reachableP = edge_data->reachableQ + (np_Q - 1) * np_P;
    edge_data->n_points_P = np_P;
    edge_data->n_points_Q = np_Q;
}

static bool GetFreeSpaceCellOneEdge(Point2d *p, Segment seg, double eps,
                                    double *fsp_entry_range_begin, double *fsp_entry_range_end) {
    Point2d p1 = {0}, p2 = {0};
    switch (CircleLineIntersection(p, eps, seg, &p1, &p2)) {
        case PASSANT:
            return false;
        case TANGENT: {
            double scal = PointToParameter(p1, seg);
            if (scal >= 0 && scal <= 1) {
                *fsp_entry_range_begin = scal;
                *fsp_entry_range_end = scal;
                return true;
            }
            return false;
        }
        case SECANT: {
            double scal1 = PointToParameter(p1, seg);
            double scal2 = PointToParameter(p2, seg);
            if (scal1 > scal2) // swap if order is wrong
            {
                double tmp = scal1;
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
    }
    return false;
}

void GetFreespaceEdgeData(Curve P, Curve Q, double eps, FreeSpaceEdgeData *edge_data) {

    uint32_t index = 0;
    FreeSpaceEdgesMaybeAlloc(edge_data, P.n_points, Q.n_points);
    for (uint32_t i = 0; i < P.n_points; i++) // for every point of P and every segment of Q
    {
        for (uint32_t j = 0; j < Q.n_points - 1; j++) {
            Segment seg = {Q.points + j, Q.points + j + 1};
            edge_data->reachableQ[index] = GetFreeSpaceCellOneEdge(P.points + i, seg, eps,
                                                                   &(edge_data->edgesQ[index].entry),
                                                                   &(edge_data->edgesQ[index].exit));
            index++;
        }
    }
    edge_data->edgesP = edge_data->edgesQ + index;
    edge_data->reachableP = edge_data->reachableQ + index;
    for (uint32_t i = 0; i < Q.n_points; i++) {
        for (uint32_t j = 0; j < P.n_points - 1; j++) {
            Segment seg = {P.points + j, P.points + j + 1};
            edge_data->reachableQ[index] = GetFreeSpaceCellOneEdge(Q.points + i, seg, eps,
                                                                   &(edge_data->edgesQ[index].entry),
                                                                   &(edge_data->edgesQ[index].exit));
            index++;
        }
    }
}

void FreeEdgeData(FreeSpaceEdgeData *data) {
    if (data->reachableQ)
        free(data->reachableQ);
    if (data->edgesQ)
        free(data->edgesQ);
    memset(data, 0, sizeof(FreeSpaceEdgeData));
}

//first check if entry and exit is possible before doing any actual work
static bool GridEntryExitPossible(FreeSpaceEdgeData *edges) {
    if (!(edges->reachableP[0] && edges->reachableQ[0])) return false;
    if (!(edges->reachableP[(edges->n_points_P - 1) * edges->n_points_Q - 1] &&
          edges->reachableQ[edges->n_points_P * (edges->n_points_Q - 1) - 1])) //check if exit of P and Q are possible
        return false;
    if (edges->edgesP[0].entry == 0.0f && edges->edgesQ[0].entry == 0.0f &&
        edges->edgesP[(edges->n_points_P - 1) * edges->n_points_Q - 1].exit == 1.0f &&
        edges->edgesQ[edges->n_points_P * (edges->n_points_Q - 1) - 1].exit == 1.0f)
        return true;
    return false;
}

static bool TraverseGridRight(FreeSpaceEdgeData *edges, FreespaceCellPos prev_cell, double prev_entry);

static bool TraverseGridUp(FreeSpaceEdgeData *edges, FreespaceCellPos prev_cell, double prev_entry);

//recursively traverse the free space grid upwards or go right if upwards doesnt work
static bool TraverseGridUp(FreeSpaceEdgeData *edges, FreespaceCellPos prev_cell, double prev_entry) {
    FreespaceCellPos curr_cell = {prev_cell.x, prev_cell.y + 1};
    if (curr_cell.y == edges->n_points_Q) return false;
    //return false if the bottom segment of the current cell is not reachable
    if (!edges->reachableP[curr_cell.y * (edges->n_points_P - 1) + curr_cell.x]) return false;
    //if edge is reachable and were at the last cell return true (We have won)
    if (curr_cell.x == edges->n_points_P - 1 && curr_cell.y == edges->n_points_Q - 1) return true;
    FreeSpaceEdge curr_cell_bottom_edge = edges->edgesP[curr_cell.y * (edges->n_points_P - 1) + curr_cell.x];
    if (prev_entry > curr_cell_bottom_edge.exit) return false;
    double curr_entry;
    if (prev_entry < curr_cell_bottom_edge.entry)
        curr_entry = curr_cell_bottom_edge.entry;
    else curr_entry = prev_entry;
    if (TraverseGridUp(edges, curr_cell, curr_entry)) return true;
    return TraverseGridRight(edges, curr_cell, 0.0f);
}

//recursively traverse the free space grid to the right or go upwards if right doesnt work
static bool TraverseGridRight(FreeSpaceEdgeData *edges, FreespaceCellPos prev_cell, double prev_entry) {
    FreespaceCellPos curr_cell = {prev_cell.x + 1, prev_cell.y};
    if (curr_cell.x == edges->n_points_P) return false;
    //return false if the left segment of the current cell is not reachable
    if (!edges->reachableQ[curr_cell.x * (edges->n_points_Q - 1) + curr_cell.y]) return false;
    //if edge is reachable and were at the last cell return true (We have won)
    if (curr_cell.x == edges->n_points_P - 1 && curr_cell.y == edges->n_points_Q - 1) return true;
    FreeSpaceEdge curr_cell_left_edge = edges->edgesQ[curr_cell.x * (edges->n_points_Q - 1) + curr_cell.y];
    if (prev_entry > curr_cell_left_edge.exit) return false;
    double curr_entry;
    if (prev_entry < curr_cell_left_edge.entry)
        curr_entry = curr_cell_left_edge.entry;
    else curr_entry = prev_entry;
    if (TraverseGridRight(edges, curr_cell, curr_entry)) return true;
    return TraverseGridUp(edges, curr_cell, 0.0f);
}

bool FrechetDistLeqEps(FreeSpaceEdgeData *edges) {
    if (!GridEntryExitPossible(edges)) return false;
    FreespaceCellPos curr_cell = {0, 0};
    if (TraverseGridUp(edges, curr_cell, 0.0f)) return true;
    if (TraverseGridRight(edges, curr_cell, 0.0f)) return true;
    return false;
}

double ComputeFrechetDistance(Curve P, Curve Q, FreeSpaceEdgeData *data) {
    Point2d xy_max_pq = {Max(P.xy_max.x, Q.xy_max.x), Max(P.xy_max.y, Q.xy_max.y)};
    Point2d xy_min_pq = {Min(P.xy_min.x, Q.xy_min.x), Min(P.xy_min.y, Q.xy_min.y)};
    double dx = xy_max_pq.x - xy_min_pq.x;
    double dy = xy_max_pq.y - xy_min_pq.y;
    double frechet_dist_top = hypot(dx, dy);
    double frechet_dist_eps;
    double frechet_dist_bottom = 0;
    FreeSpaceEdgeData curr_eps_data = {0};
    for (int i = 0; i < FRECHET_DIST_APPROX_STEPS; ++i) {
        frechet_dist_eps = (frechet_dist_top + frechet_dist_bottom) / 2;
        GetFreespaceEdgeData(P, Q, frechet_dist_eps, &curr_eps_data);
        if (FrechetDistLeqEps(&curr_eps_data))
            frechet_dist_top = frechet_dist_eps;
        else
            frechet_dist_bottom = frechet_dist_eps;
    }
    GetFreespaceEdgeData(P, Q, frechet_dist_top, &curr_eps_data);
    *data = curr_eps_data;
    return frechet_dist_top;
}
