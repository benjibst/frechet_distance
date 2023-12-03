#include "frechet_dist.h"
#include <stdio.h>
#include <stdlib.h>

void FreeSpaceEdgesMaybeAlloc(FD_freespace_edge_data *edge_data, uint32_t np_P, uint32_t np_Q) {
    size_t n_edges = (np_P - 1) * np_Q + (np_Q - 1) * np_P;
    if (n_edges > edge_data->cap) {
        edge_data->cap = 2 * n_edges;
        edge_data->edges = malloc(edge_data->cap * sizeof(FD_freespace_edge));
        edge_data->edgesQ = edge_data->edges;
        edge_data->edgesP = edge_data->edges + (np_Q-1) * np_P;
        edge_data->reachable = malloc(edge_data->cap * sizeof(uint8_t));
        edge_data->reachableQ = edge_data->reachable;
        edge_data->reachableP = edge_data->reachable + (np_Q-1) * np_P;
        if (!edge_data->edges || !edge_data->reachable) {
            fprintf(stderr, "Failed reallocating freespace grid");
            abort();
        }
    }
    edge_data->n_points_P = np_P;
    edge_data->n_points_Q = np_Q;
}

void GetFreespaceEdgeData(FD_curve P, FD_curve Q, FD_float eps, FD_freespace_edge_data *edge_data) {
    // Edges are stored like this in memory
    // 10---11---
    // 1|   3|  5|
    //  8--- 9---
    // 0|   2|  4|
    //  6--- 7---
    uint32_t index = 0;
    FreeSpaceEdgesMaybeAlloc(edge_data, P.n_points, Q.n_points);
    for (uint32_t i = 0; i < P.n_points; i++) // for every point of P and every segment of Q
    {
        for (uint32_t j = 0; j < Q.n_points - 1; j++) {
            FD_segment seg = {Q.points + j, Q.points + j + 1};
            edge_data->reachable[index] = GetFreeSpaceCellOneEdge(P.points + i, seg, eps,
                                                                  &(edge_data->edges[index].entry),
                                                                  &(edge_data->edges[index].exit));
            index++;
        }
    }
    edge_data->edgesP = edge_data->edges+index;
    edge_data->reachableP = edge_data->reachable+index;
    for (uint32_t i = 0; i < Q.n_points; i++) {
        for (uint32_t j = 0; j < P.n_points - 1; j++) {
            FD_segment seg = {P.points + j, P.points + j + 1};
            edge_data->reachable[index] = GetFreeSpaceCellOneEdge(Q.points + i, seg, eps,
                                                                  &(edge_data->edges[index].entry),
                                                                  &(edge_data->edges[index].exit));
            index++;
        }
    }
}

bool GetFreeSpaceCellOneEdge(FD_point *p, FD_segment seg, FD_float eps,
                             FD_float *fsp_entry_range_begin, FD_float *fsp_entry_range_end) {
    FD_point p1 = {0}, p2 = {0};
    switch (CircleLineIntersection(p, eps, seg, &p1, &p2)) {
        case PASSANT:
            return false;
        case TANGENT: {
            FD_float scal = PointToParameter(p1, seg);
            if (scal >= 0 && scal <= 1) {
                *fsp_entry_range_begin = scal;
                *fsp_entry_range_end = scal;
                return true;
            }
            return false;
        }
        case SECANT: {
            FD_float scal1 = PointToParameter(p1, seg);
            FD_float scal2 = PointToParameter(p2, seg);
            if (scal1 > scal2) // swap if order is wrong
            {
                FD_float tmp = scal1;
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

//first check if entry and exit is possible before doing any actual work
bool GridEntryExitPossible(FD_freespace_edge_data *edges) {
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

bool TraverseGridUp(FD_freespace_edge_data *edges,FD_freespace_cell_pos prev_cell, FD_float prev_entry) {
    FD_freespace_cell_pos curr_cell = {prev_cell.x,prev_cell.y+1};
    //the bottom edge of the current cell is already at the top of the grid;
    if(curr_cell.y == edges->n_points_Q) return false;
    //return false if the bottom segment of the current cell is not reachable
    if(!edges->reachableP[curr_cell.y*(edges->n_points_P-1) + curr_cell.x]) return false;
    FD_freespace_edge curr_cell_bottom_edge = edges->edgesP[curr_cell.y*(edges->n_points_P-1) + curr_cell.x];
    if(prev_entry>curr_cell_bottom_edge.exit) return false;
    FD_float curr_entry;
    if(prev_entry<curr_cell_bottom_edge.entry)
        curr_entry = curr_cell_bottom_edge.entry;
    else curr_entry = prev_entry;
    printf("Cell: (%d,%d) PrevEntry: %.3f CurrEntry %.3f\n",curr_cell.x,curr_cell.y,prev_entry,curr_entry);
    return TraverseGridUp(edges,curr_cell,curr_entry);

}

bool TraverseGridRight(FD_freespace_edge_data *edges, FD_freespace_cell_pos prev_cell, FD_float prev_entry) {

}

bool FrechetDistLeqEps(FD_freespace_edge_data *edges) {
    if (!GridEntryExitPossible(edges)) return false;
    FD_freespace_cell_pos curr_cell = {0,0};
    if (TraverseGridUp(edges,curr_cell,0.0f)) return true;
    //if (TraverseGridRight(edges,curr_cell,0.0f)) return true;
    return false;
}