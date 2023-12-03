#include <stdlib.h>
#include "visualizer.h"
#include "raylib.h"
#include "raygui.h"
#include "frechet_dist.h"

#define RAYGUI_IMPLEMENTATION

#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION

static const int WINDOW_X = 800;
static const int WINDOW_Y = 500;
static const char *const WINDOW_NAME = "Frechet distance";

void InitGUI(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_X, WINDOW_Y, WINDOW_NAME);
    SetTargetFPS(144);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
}

/////////////////////////////////////////gui data
static Rectangle curves_panel_rect;
static Rectangle fsp_panel_rect;
static Rectangle eps_valuebox_rect;
static Rectangle fsp_grid_rect;
static struct {
    int width, height;
} window_sz;
static Vector2 mouse;
///////////////////////////////////////////////////

static inline bool IsMouseInRectangle(Rectangle rect) {
    return mouse.x >= rect.x && mouse.x <= rect.x + rect.width && mouse.y >= rect.y &&
           mouse.y <= rect.y + rect.height;
}

static void CalcGui(void) {
    mouse = GetMousePosition();
    window_sz.width = GetScreenWidth();
    window_sz.height = GetScreenHeight();
    curves_panel_rect = (Rectangle) {0.0f, 0.0f, window_sz.width / 2.0f, window_sz.height};
    fsp_panel_rect =
            (Rectangle) {window_sz.width / 2.0f, 0.0f, window_sz.width / 2.0f, window_sz.height};
    eps_valuebox_rect = (Rectangle) {fsp_panel_rect.x + fsp_panel_rect.width - 50, fsp_panel_rect.y, 50,
                                     RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT};
}

static void
DrawFreeSpaceCellEdges(uint32_t P_seg_idx, uint32_t Q_seg_idx, FD_freespace_edge_data edge_data, Rectangle base) {
    FD_point base_vert[4] = {
            {base.x,              base.y + base.height},                // bottom left
            {base.x,              base.y},                            // top left
            {base.x + base.width, base.y},                // top right
            {base.x + base.width, base.y + base.height} // bottom right
    };
    FD_segment base_edges[4] = {
            {base_vert + 0, base_vert + 1}, // left
            {base_vert + 1, base_vert + 2}, // top
            {base_vert + 3, base_vert + 2}, // right
            {base_vert + 0, base_vert + 3}    // bottom
    };
    // look at function GetFreespaceEdgeData to see how the edges are stored in memory
    // first all the vertical edges then the horizontal edges
    size_t offset = (edge_data.n_points_Q - 1) * edge_data.n_points_P;
    size_t offsets[4] = {
            P_seg_idx * (edge_data.n_points_Q - 1) + Q_seg_idx,
            offset + (Q_seg_idx + 1) * (edge_data.n_points_P - 1) + P_seg_idx,
            (P_seg_idx + 1) * (edge_data.n_points_Q - 1) + Q_seg_idx,
            offset + Q_seg_idx * (edge_data.n_points_P - 1) + P_seg_idx};
    struct {
        FD_float fsp_poly_vert_paramspace;
        uint32_t fsp_rect_edge;
    } fsp_poly_vertices[8];
    FD_point fsp_poly_vert[8];
    uint32_t index = 0;

    DrawRectangleLinesEx(base, 1, SKYBLUE);
    // for all 4 edges of the free space diagram
    for (uint32_t i = 0; i < 4; i++) { // if entry or exit from that edge is possible
        if (edge_data.reachable[offsets[i]]) {                // add the respective points to the polygon vertices and increase index for next points
            if (i >= 2) // if its the right or bottom edge, flip entry and exit so we still go counterclockwise
            {
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edges[offsets[i]].exit;
                fsp_poly_vertices[index++].fsp_rect_edge = i;
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edges[offsets[i]].entry;
                fsp_poly_vertices[index++].fsp_rect_edge = i;
            } else {
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edges[offsets[i]].entry;
                fsp_poly_vertices[index++].fsp_rect_edge = i;
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edges[offsets[i]].exit;
                fsp_poly_vertices[index++].fsp_rect_edge = i;
            }
        }
    }
    if (!index)
        return;
    // now the number of verticess is stored in index and we convert
    for (uint32_t i = 0; i < index; i++) {
        fsp_poly_vert[i] = ParameterToPoint(base_edges[fsp_poly_vertices[i].fsp_rect_edge],
                                            fsp_poly_vertices[i].fsp_poly_vert_paramspace);
    }
    for (uint32_t i = 0; i < index - 1; i++)
        DrawLine(fsp_poly_vert[i].x, fsp_poly_vert[i].y, fsp_poly_vert[i + 1].x, fsp_poly_vert[i + 1].y, RED);
    // if the polygon is at least a triangle, connect the last point to the first point
    if (index > 2)
        DrawLine(fsp_poly_vert[index - 1].x, fsp_poly_vert[index - 1].y, fsp_poly_vert[0].x, fsp_poly_vert[0].y, RED);
}

static void DrawFreeSpaceEdges(FD_freespace_edge_data edge_data) {
    int max_side_len_rect = Min(fsp_panel_rect.width, (fsp_panel_rect.height - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT));
    float cellratio = (float) (edge_data.n_points_P - 1) / (edge_data.n_points_Q - 1);
    const float width_ratio = 0.95f;
    if (edge_data.n_points_P > edge_data.n_points_Q) {
        fsp_grid_rect.width = width_ratio * max_side_len_rect;
        fsp_grid_rect.height = fsp_grid_rect.width / cellratio;
    } else {
        fsp_grid_rect.height = width_ratio * max_side_len_rect;
        fsp_grid_rect.width = fsp_grid_rect.height * cellratio;
    }
    fsp_grid_rect.x = fsp_panel_rect.x + (fsp_panel_rect.width - fsp_grid_rect.width) / 2;
    fsp_grid_rect.y =
            fsp_panel_rect.y + (fsp_panel_rect.height - fsp_grid_rect.height + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT) / 2;
    int fsp_width = fsp_grid_rect.width / (edge_data.n_points_P - 1);
    int fsp_height = fsp_grid_rect.height / (edge_data.n_points_Q - 1);
    for (uint32_t i = 0; i < edge_data.n_points_P - 1; i++) {
        int fsp_x = fsp_grid_rect.x + i * fsp_width;
        for (uint32_t j = 0; j < edge_data.n_points_Q - 1; j++) {
            int fsp_y = fsp_grid_rect.y + (edge_data.n_points_Q - j - 2) * fsp_height;
            Rectangle fsp_cell_base = {fsp_x, fsp_y, fsp_width, fsp_height};
            DrawFreeSpaceCellEdges(i, j, edge_data, fsp_cell_base);
        }
    }
}

static void DrawPQPoints(FD_curve P, FD_curve Q, FD_float eps) {
    float pos_x_in_fsp_rect = mouse.x - fsp_grid_rect.x;
    float pos_y_in_fsp_rect =
            fsp_grid_rect.height + fsp_grid_rect.y - mouse.y; // in this case .y is the top side so add height
    float fsp_cell_width = fsp_grid_rect.width / P.n_segments;
    float fsp_cell_height = fsp_grid_rect.height / Q.n_segments;
    uint32_t P_index = pos_x_in_fsp_rect / fsp_cell_width;
    uint32_t Q_index = pos_y_in_fsp_rect / fsp_cell_height;
    float cell_x = fsp_grid_rect.x + P_index * fsp_cell_width;
    float cell_y = fsp_grid_rect.y + fsp_grid_rect.height - Q_index * fsp_cell_height;
    float pos_x_in_cell = (mouse.x - cell_x) / fsp_cell_width;
    float pos_y_in_cell =
            (cell_y - mouse.y) / fsp_cell_height; // in this case it is the bottom side so no need to add height
    FD_point point_P = ParameterToPoint((FD_segment) {P.points + P_index, P.points + P_index + 1}, pos_x_in_cell);
    FD_point point_Q = ParameterToPoint((FD_segment) {Q.points + Q_index, Q.points + Q_index + 1}, pos_y_in_cell);
    DrawCircle(point_P.x, point_P.y, 5, DARKGREEN);
    DrawCircle(point_Q.x, point_Q.y, 5, DARKGREEN);
    Color c = Hypot(point_P.x - point_Q.x, point_P.y - point_Q.y) > eps ? RED : GREEN;
    DrawCircleLines(point_P.x, point_P.y, eps, c);
}

void RunVisualizer(void) {
    int eps = 0;
    bool eps_editing = false, recalc_grid = false, frechet_dist_leq_eps = false;
    FD_curve P = AllocateCurve(10), Q = AllocateCurve(10);
    FD_freespace_edge_data edge_data = {0};
    while (!WindowShouldClose()) {
        CalcGui();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseInRectangle(curves_panel_rect)) {
            FD_point p_new = {mouse.x, mouse.y};
            AddPointToCurve(&P, p_new);
            recalc_grid = true;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && IsMouseInRectangle(curves_panel_rect)) {
            FD_point p_new = {mouse.x, mouse.y};
            AddPointToCurve(&Q, p_new);
            recalc_grid = true;
        }
        if (IsKeyPressed(KEY_X)) // reset curves
        {
            Q.n_points = 0;
            P.n_points = 0;
            Q.n_segments = 0;
            P.n_segments = 0;
            recalc_grid = true;
        }
        BeginDrawing();
        GuiPanel(fsp_panel_rect, "Free Space diagram");
        GuiPanel(curves_panel_rect, "Curves");
        if (GuiValueBox(eps_valuebox_rect, NULL, &eps, 0, 10000, eps_editing)) {
            if (eps_editing)
                recalc_grid = true;
            eps_editing = !eps_editing;
        }
        for (long long i = 0; i < ((long long) P.n_points) - 1; i++) {
            DrawLineEx((Vector2) {P.points[i].x, P.points[i].y}, (Vector2) {P.points[i + 1].x, P.points[i + 1].y}, 2,
                       BEIGE);
        }
        for (long long i = 0; i < ((long long) Q.n_points) - 1; i++) {
            DrawLineEx((Vector2) {Q.points[i].x, Q.points[i].y}, (Vector2) {Q.points[i + 1].x, Q.points[i + 1].y}, 2,
                       PURPLE);
        }
        if (P.n_segments >= 1 && Q.n_segments >= 1) {
            if (recalc_grid) {
                GetFreespaceEdgeData(P, Q, eps, &edge_data);
                if (FrechetDistLeqEps(&edge_data))
                    frechet_dist_leq_eps = true;
                else frechet_dist_leq_eps = false;
                recalc_grid = false;
            }
            if (IsMouseInRectangle(fsp_grid_rect))
                DrawPQPoints(P, Q, eps);
            DrawFreeSpaceEdges(edge_data);
            if (frechet_dist_leq_eps) DrawRectangleLinesEx(fsp_grid_rect, 5, GREEN);
        }
        EndDrawing();
    }
    free(edge_data.edges);
    free(edge_data.reachable);
    free(P.points);
    free(Q.points);
}
