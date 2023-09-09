#include "visualizer.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

const int WINDOW_X = 800;
const int WINDOW_Y = 500;
const float LINE_THICK = 6;
const char* const WINDOW_NAME = "Frechet distance";

void InitGUI()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WINDOW_X, WINDOW_Y, WINDOW_NAME);
	SetTargetFPS(144);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
}

/////////////////////////////////////////gui data
Rectangle curves_panel_rect;
Rectangle fsp_panel_rect;
Rectangle fsp_label_rect;
Rectangle eps_valuebox_rect;
Rectangle fsp_grid_rect;
struct
{
	int width, height;
} window_sz;
Vector2 mouse;
///////////////////////////////////////////////////

static inline bool IsMouseInRectangle(Rectangle rect)
{
	return mouse.x >= rect.x && mouse.x <= rect.x + rect.width && mouse.y >= rect.y &&
		mouse.y <= rect.y + rect.height;
}

static void CalcGui()
{
	mouse = GetMousePosition();
	window_sz.width = GetScreenWidth();
	window_sz.height = GetScreenHeight();
	curves_panel_rect = (Rectangle){ 0.0f, 0.0f, window_sz.width / 2.0f, window_sz.height };
	fsp_panel_rect =
		(Rectangle){ window_sz.width / 2.0f, 0.0f, window_sz.width / 2.0f, window_sz.height };
	eps_valuebox_rect = (Rectangle){ fsp_panel_rect.x + fsp_panel_rect.width - 50,fsp_panel_rect.y,50,RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT };
}


static void DrawFreeSpaceCell(const FD_freespace_cell* const fsp, Rectangle base)
{
	FD_point base_vert[4] = {
		{base.x,base.y + base.height},				//bottom left
		{base.x,base.y},							//top left
		{base.x + base.width,base.y},				//top right
		{base.x + base.width,base.y + base.height}	//bottom right
	};
	FD_segment base_edges[4] = {
		{base_vert + 0,base_vert + 1}, //left
		{base_vert + 1,base_vert + 2}, //top
		{base_vert + 3,base_vert + 2}, //right
		{base_vert + 0,base_vert + 3}  //bottom
	};
	struct
	{
		FD_float fsp_poly_vert_paramspace;
		uint32_t fsp_rect_edge;
	} fsp_poly_vertices[8];
	FD_point fsp_poly_vert[8];
	uint32_t index = 0;

	DrawRectangleLinesEx(base, 1, SKYBLUE);

	// for all 4 edges of the free space diagram
	for (uint32_t i = 0; i < 4; i++)
	{ // if entry or exit from that edge is possible
		if (fsp->pass & (FD_fsp_entry_exit_bits)(1 << i))
		{ // add the respective points to the polygon vertices and increase index for next points
			fsp_poly_vertices[index].fsp_poly_vert_paramspace = fsp->fsp_vertices[2 * i];
			fsp_poly_vertices[index++].fsp_rect_edge = i;
			fsp_poly_vertices[index].fsp_poly_vert_paramspace = fsp->fsp_vertices[2 * i + 1];
			fsp_poly_vertices[index++].fsp_rect_edge = i;
		}
	}
	if (!index)
		return;
	// now the number of verticess is stored in index and we convert
	for (uint32_t i = 0; i < index; i++) {
		fsp_poly_vert[i] = ParameterToPoint(base_edges[fsp_poly_vertices[i].fsp_rect_edge], fsp_poly_vertices[i].fsp_poly_vert_paramspace);
	}
	for (uint32_t i = 0; i < index - 1; i++)
		DrawLine(fsp_poly_vert[i].x, fsp_poly_vert[i].y, fsp_poly_vert[i + 1].x, fsp_poly_vert[i + 1].y, DARKGREEN);
	// if the polygon is at least a triangle, connect the last point to the first point
	if (index > 2)
		DrawLine(fsp_poly_vert[index - 1].x, fsp_poly_vert[index - 1].y, fsp_poly_vert[0].x, fsp_poly_vert[0].y, DARKGREEN);
}

static void DrawFreeSpaceGrid(FD_freespace_cell_grid grid)
{
	if (!grid.n_segments_P || !grid.n_segments_Q)
		return;
	int max_side_len_rect = Min(fsp_panel_rect.width, (fsp_panel_rect.height - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT));
	float cellratio = (float)grid.n_segments_P / grid.n_segments_Q;
	const float width_ratio = 0.9f;
	if (grid.n_segments_P > grid.n_segments_Q)
	{
		fsp_grid_rect.width = width_ratio * max_side_len_rect;
		fsp_grid_rect.height = fsp_grid_rect.width / cellratio;
	}
	else
	{
		fsp_grid_rect.height = width_ratio * max_side_len_rect;
		fsp_grid_rect.width = fsp_grid_rect.height * cellratio;
	}
	fsp_grid_rect.x = fsp_panel_rect.x + (fsp_panel_rect.width - fsp_grid_rect.width) / 2;
	fsp_grid_rect.y = fsp_panel_rect.y + (fsp_panel_rect.height - fsp_grid_rect.height + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT) / 2;
	int fsp_width = fsp_grid_rect.width / grid.n_segments_P;
	int fsp_height = fsp_grid_rect.height / grid.n_segments_Q;
	for (size_t i = 0; i < grid.n_segments_P; i++)
	{
		int fsp_x = fsp_grid_rect.x + i * fsp_width;
		for (size_t j = 0; j < grid.n_segments_Q; j++)
		{
			int fsp_y = fsp_grid_rect.y + (grid.n_segments_Q - j - 1) * fsp_height;
			Rectangle fsp_cell_base = { fsp_x,fsp_y,fsp_width,fsp_height };
			DrawFreeSpaceCell(grid.cells + (i * grid.n_segments_Q + j), fsp_cell_base);
		}
	}
}

static void DrawPQPoints(FD_curve P, FD_curve Q,FD_float eps)
{
	float pos_x_in_fsp_rect = mouse.x - fsp_grid_rect.x;
	float pos_y_in_fsp_rect = fsp_grid_rect.height + fsp_grid_rect.y - mouse.y; //in this case .y is the top side so add height
	float fsp_cell_width = fsp_grid_rect.width / P.n_segments;
	float fsp_cell_height = fsp_grid_rect.height / Q.n_segments;
	uint32_t P_index = pos_x_in_fsp_rect / fsp_cell_width;
	uint32_t Q_index = pos_y_in_fsp_rect / fsp_cell_height;
	float cell_x = fsp_grid_rect.x + P_index * fsp_cell_width;
	float cell_y = fsp_grid_rect.y + fsp_grid_rect.height - Q_index * fsp_cell_height;
	float pos_x_in_cell = (mouse.x - cell_x) / fsp_cell_width;
	float pos_y_in_cell = (cell_y - mouse.y) / fsp_cell_height; //in this case it is the bottom side so no need to add height
	FD_point point_P = ParameterToPoint((FD_segment) {P.points + P_index, P.points + P_index + 1}, pos_x_in_cell);
	FD_point point_Q = ParameterToPoint((FD_segment) { Q.points + Q_index, Q.points + Q_index + 1 }, pos_y_in_cell);
	DrawCircle(point_P.x, point_P.y, 5, DARKGREEN);
	DrawCircle(point_Q.x, point_Q.y, 5, DARKGREEN);
	Color c = Hypot(point_P.x - point_Q.x, point_P.y - point_Q.y) > eps ? RED : GREEN;
	DrawCircleLines(point_P.x, point_P.y, eps, c);
}

void RunVisualizer()
{
	int eps = 0;
	bool eps_editing = false, recalc_grid = false;
	FD_curve P = AllocateCurve(10), Q = AllocateCurve(10);
	FD_freespace_cell_grid grid = { 0 };
	FD_point pointP = { 0 }, pointQ = { 0 };
	Color P_color = BEIGE;
	Color Q_color = PURPLE;
	Color P_color_editing = { P_color.r, P_color.g, P_color.b, P_color.a / 2 };
	Color Q_color_editing = { Q_color.r, Q_color.g, Q_color.b, Q_color.a / 2 };
	FD_freespace_cell fsp = { 0 };
	while (!WindowShouldClose())
	{
		CalcGui();
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseInRectangle(curves_panel_rect))
		{
			AddPointToCurve(&P, (FD_point) { mouse.x, mouse.y });
			recalc_grid = true;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && IsMouseInRectangle(curves_panel_rect))
		{
			AddPointToCurve(&Q, (FD_point) { mouse.x, mouse.y });
			recalc_grid = true;
		}
		if (IsKeyPressed(KEY_X)) //reset curves
		{
			Q.n_points = 0;
			P.n_points = 0;
			Q.n_segments = 0;
			P.n_segments = 0;
			recalc_grid = true;
		}
		BeginDrawing();
		GuiPanel(fsp_panel_rect, "Free Space");
		GuiPanel(curves_panel_rect, "Segments");
		if (GuiValueBox(eps_valuebox_rect, NULL, &eps, 0, 10000, eps_editing))
		{
			if (eps_editing)
				recalc_grid = true;
			eps_editing = !eps_editing;
		}
		for (long long i = 0; i < ((long long)P.n_points) - 1; i++)
		{
			DrawLineEx((Vector2) { P.points[i].x, P.points[i].y }, (Vector2) { P.points[i + 1].x, P.points[i + 1].y }, 2, BEIGE);
		}
		for (long long i = 0; i < ((long long)Q.n_points) - 1; i++)
		{
			DrawLineEx((Vector2) { Q.points[i].x, Q.points[i].y }, (Vector2) { Q.points[i + 1].x, Q.points[i + 1].y }, 2, PURPLE);
		}
		if (P.n_segments >= 1 && Q.n_segments >= 1)
		{
			if (recalc_grid)
			{
				GetFreespaceCellGrid(P, Q, eps, &grid);
				recalc_grid = false;
			}
			if (IsMouseInRectangle(fsp_grid_rect))
				DrawPQPoints(P, Q, eps);
			DrawFreeSpaceGrid(grid);
		}
		EndDrawing();
	}
}