#include <stdlib.h>
#include "frechet_dist.h"
#include "visualizer.h"
#include "raylib.h"

#include "raygui.h"

#define RAYGUI_IMPLEMENTATION

#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION

static const int WINDOW_X = 800;
static const int WINDOW_Y = 500;
static const char *const WINDOW_NAME = "Frechet distance";

static void InitGUI(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_X, WINDOW_Y, WINDOW_NAME);
    SetTargetFPS(60);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
}

static void DeInitGUI(void) {
    CloseWindow();
}

/////////////////////////////////////////gui data
static Rectangle curves_panel_rect;
static Rectangle fsp_panel_rect;
static Rectangle eps_valuebox_rect;
static Rectangle compute_button_rect;
static Rectangle fsp_grid_rect;
static struct {
    int width, height;
} window_sz;
static Vector2 mouse;
///////////////////////////////////////////////////

static inline bool IsMouseInRect(Rectangle rect) {
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
    eps_valuebox_rect = (Rectangle) {fsp_panel_rect.x + fsp_panel_rect.width - 100, fsp_panel_rect.y, 100,
                                     RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT};
    compute_button_rect = (Rectangle) {fsp_panel_rect.x + fsp_panel_rect.width - 200, fsp_panel_rect.y, 100,
                                       RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT};
}

static void
DrawFreeSpaceCellEdges(uint32_t P_seg_idx, uint32_t Q_seg_idx, FreeSpaceEdgeData edge_data, Rectangle base) {
    Point2d base_vert[4] = {
            {base.x,              base.y + base.height},// bottom left
            {base.x,              base.y},// top left
            {base.x + base.width, base.y},// top right
            {base.x + base.width, base.y + base.height} // bottom right
    };
    Segment base_edges[4] = {
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
        double fsp_poly_vert_paramspace;
        uint32_t fsp_rect_edge;
    } fsp_poly_vertices[8];
    Point2d fsp_poly_vert[8];
    uint32_t index = 0;

    DrawRectangleLinesEx(base, 1, SKYBLUE);
    // for all 4 edges of the free space diagram
    for (uint32_t i = 0; i < 4; i++) { // if entry or exit from that edge is possible
        if (edge_data.reachableQ[offsets[i]]) { // add the respective points to the polygon vertices and increase index for next points
            if (i >= 2) // if its the right or bottom edge, flip entry and exit so we still go counterclockwise
            {
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edgesQ[offsets[i]].exit;
                fsp_poly_vertices[index++].fsp_rect_edge = i;
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edgesQ[offsets[i]].entry;
                fsp_poly_vertices[index++].fsp_rect_edge = i;
            } else {
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edgesQ[offsets[i]].entry;
                fsp_poly_vertices[index++].fsp_rect_edge = i;
                fsp_poly_vertices[index].fsp_poly_vert_paramspace = edge_data.edgesQ[offsets[i]].exit;
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

static void DrawFreeSpaceEdges(FreeSpaceEdgeData edge_data) {
    float max_side_len_rect = Min(fsp_panel_rect.width, (fsp_panel_rect.height - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT));
    float cellratio = (float) (edge_data.n_points_P - 1) / (edge_data.n_points_Q - 1);
    const float width_ratio = 0.98f;
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
    float fsp_width = fsp_grid_rect.width / (edge_data.n_points_P - 1);
    float fsp_height = fsp_grid_rect.height / (edge_data.n_points_Q - 1);
    for (uint32_t i = 0; i < edge_data.n_points_P - 1; i++) {
        float fsp_x = fsp_grid_rect.x + i * fsp_width;
        for (uint32_t j = 0; j < edge_data.n_points_Q - 1; j++) {
            float fsp_y = fsp_grid_rect.y + (edge_data.n_points_Q - j - 2) * fsp_height;
            Rectangle fsp_cell_base = {fsp_x, fsp_y, fsp_width, fsp_height};
            DrawFreeSpaceCellEdges(i, j, edge_data, fsp_cell_base);
        }
    }
}

static void DrawPQPoints(Curve P, Curve Q, double eps) {
    float pos_x_in_fsp_rect = mouse.x - fsp_grid_rect.x;
    float pos_y_in_fsp_rect = fsp_grid_rect.height + fsp_grid_rect.y - mouse.y;
    float fsp_cell_width = fsp_grid_rect.width / P.n_segments;
    float fsp_cell_height = fsp_grid_rect.height / Q.n_segments;
    uint32_t P_index = pos_x_in_fsp_rect / fsp_cell_width;
    uint32_t Q_index = pos_y_in_fsp_rect / fsp_cell_height;
    float cell_x = fsp_grid_rect.x + P_index * fsp_cell_width;
    float cell_y = fsp_grid_rect.y + fsp_grid_rect.height - Q_index * fsp_cell_height;
    float pos_x_in_cell = (mouse.x - cell_x) / fsp_cell_width;
    float pos_y_in_cell = (cell_y - mouse.y) / fsp_cell_height;
    Point2d point_P = ParameterToPoint((Segment) {P.points + P_index, P.points + P_index + 1}, pos_x_in_cell);
    Point2d point_Q = ParameterToPoint((Segment) {Q.points + Q_index, Q.points + Q_index + 1}, pos_y_in_cell);
    DrawCircle(point_P.x, point_P.y, 5, DARKGREEN);
    DrawCircle(point_Q.x, point_Q.y, 5, DARKGREEN);
    Color c = hypot(point_P.x - point_Q.x, point_P.y - point_Q.y) > eps ? RED : GREEN;
    DrawCircleLines(point_P.x, point_P.y, eps, c);
}

void RunVisualizer(void) {
    InitGUI();
    float eps_f = 0;
    double eps_d = 0;
    char valuebox_buf[RAYGUI_VALUEBOX_MAX_CHARS + 1] = "\0";
    bool recalc_grid = false, frechet_dist_leq_eps = false;
    double frechet_distance;
    bool compute_frechet_distance = false;
    Curve P = AllocateCurve(10), Q = AllocateCurve(10);
    FreeSpaceEdgeData edge_data = {0};
    while (!WindowShouldClose()) {
        CalcGui();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseInRect(curves_panel_rect)) {
            Point2d p_new = {mouse.x, mouse.y};
            AddPointToCurve(&P, p_new);
            compute_frechet_distance = false;
            recalc_grid = true;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && IsMouseInRect(curves_panel_rect)) {
            Point2d p_new = {mouse.x, mouse.y};
            AddPointToCurve(&Q, p_new);
            compute_frechet_distance = false;
            recalc_grid = true;
        }
        if (IsKeyPressed(KEY_X)) // reset curves
        {
            Q.n_points = 0;
            P.n_points = 0;
            Q.n_segments = 0;
            P.n_segments = 0;
            compute_frechet_distance = false;
            recalc_grid = true;
        }
        BeginDrawing();
        GuiPanel(fsp_panel_rect, "Free Space diagram");
        GuiPanel(curves_panel_rect, "Curves");
        if (GuiValueBoxF(eps_valuebox_rect, NULL, valuebox_buf, &eps_f, true)) {
            compute_frechet_distance = false;
            eps_d = (double) eps_f;
            recalc_grid = true;
        }
        if (GuiButton(compute_button_rect, "Compute")) {
            compute_frechet_distance = true;
            recalc_grid = false;
        }
        for (int i = 0; i < ((int) P.n_points) - 1; i++) {
            DrawLineEx((Vector2) {P.points[i].x, P.points[i].y}, (Vector2) {P.points[i + 1].x, P.points[i + 1].y},
                       2,
                       BEIGE);
        }
        for (int i = 0; i < ((int) Q.n_points) - 1; i++) {
            DrawLineEx((Vector2) {Q.points[i].x, Q.points[i].y}, (Vector2) {Q.points[i + 1].x, Q.points[i + 1].y}, 2,
                       PURPLE);
        }
        if (P.n_segments >= 1 && Q.n_segments >= 1) {
            if (recalc_grid) {
                GetFreespaceEdgeData(P, Q, eps_d, &edge_data);
                frechet_dist_leq_eps = FrechetDistLeqEps(&edge_data);
                recalc_grid = false;
            } else if (compute_frechet_distance) {
                frechet_distance = ComputeFrechetDistance(P, Q, &edge_data);
                sprintf(valuebox_buf, "%f", frechet_distance);
                printf("The frechet distance is ~%f\n", frechet_distance);
                frechet_dist_leq_eps = true;
                eps_d = frechet_distance;
                compute_frechet_distance = false;
            }
            if (IsMouseInRect(fsp_grid_rect))
                DrawPQPoints(P, Q, eps_d);
            DrawFreeSpaceEdges(edge_data);
            if (frechet_dist_leq_eps) {
                DrawRectangleLinesEx(fsp_grid_rect, 3, GREEN);
            }
        }
        EndDrawing();
    }
    printf("Cleaning up");
    DeInitGUI();
    free(edge_data.edgesQ);
    free(edge_data.reachableQ);
    FreeCurve(&P);
    FreeCurve(&Q);
}
