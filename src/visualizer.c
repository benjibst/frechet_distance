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
	SetTargetFPS(60);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
}

/////////////////////////////////////////gui data
Rectangle curves_panel_rect;
Rectangle fsp_panel_rect;
Rectangle fsp_rect;
FD_segment fsp_rect_edges[4];
Rectangle fsp_label_rect;
Rectangle eps_valuebox_rect;
struct
{
	int width, height;
} window;
Vector2 mouse;
Vector2 mouse_fsp;
char freespace_label_text[14];
///////////////////////////////////////////////////

static inline bool IsMouseInRectangle(Rectangle rect)
{
	return mouse.x >= rect.x && mouse.x <= rect.x + rect.width && mouse.y >= rect.y &&
		mouse.y <= rect.y + rect.height;
}

static void CalcGui()
{
	mouse = GetMousePosition();
	window.width = GetScreenWidth();
	window.height = GetScreenHeight();
	curves_panel_rect = (Rectangle){ 0, 0, window.width / 2, window.height };
	fsp_panel_rect =
		(Rectangle){ window.width / 2, 0, window.width / 2, window.height };
	int fsp_side =
		Min(fsp_panel_rect.width, fsp_panel_rect.height) * 0.8f;
	fsp_rect = (Rectangle){
		fsp_panel_rect.x + ((fsp_panel_rect.width - fsp_side) / 2),
		(window.height - fsp_side) / 2, fsp_side, fsp_side };
	fsp_rect_edges[0] = (FD_segment){ {fsp_rect.x + LINE_THICK / 2, fsp_rect.y + fsp_rect.height - LINE_THICK / 2}, {fsp_rect.x + LINE_THICK / 2, fsp_rect.y + LINE_THICK / 2} };
	fsp_rect_edges[1] = (FD_segment){ {fsp_rect.x + LINE_THICK / 2, fsp_rect.y + LINE_THICK / 2}, {fsp_rect.x + fsp_rect.width - LINE_THICK / 2, fsp_rect.y + LINE_THICK / 2} };
	fsp_rect_edges[2] = (FD_segment){ {fsp_rect.x + fsp_rect.width - LINE_THICK / 2, fsp_rect.y + fsp_rect.height - LINE_THICK / 2}, {fsp_rect.x + fsp_rect.width - LINE_THICK / 2, fsp_rect.y + LINE_THICK / 2} };
	fsp_rect_edges[3] = (FD_segment){ {fsp_rect.x + LINE_THICK / 2, fsp_rect.y + fsp_rect.height - LINE_THICK / 2}, {fsp_rect.x + fsp_rect.width - LINE_THICK / 2, fsp_rect.y + fsp_rect.height - LINE_THICK / 2} };
	if (IsMouseInRectangle(fsp_rect))
	{
		mouse_fsp.x =
			(mouse.x - fsp_rect.x) / fsp_rect.width;
		mouse_fsp.y =
			1.0f - (mouse.y - fsp_rect.y) / fsp_rect.height;
		sprintf_s(freespace_label_text, sizeof(freespace_label_text), "[%.2f | %.2f]", mouse_fsp.x, mouse_fsp.y);
	}
	else
		freespace_label_text[0] = 0;
	fsp_label_rect = (Rectangle){ fsp_rect.x, fsp_rect.y - 30, 100, 20 };
	eps_valuebox_rect = (Rectangle){
		fsp_rect.x, fsp_rect.y + fsp_rect.height + 10, 80, 20 };
}

static void DrawFreeSpace(const FD_freespace* const fsp)
{
	struct
	{
		FD_float fsp_poly_vert_paramspace;
		int fsp_rect_edge;
	} fsp_poly_vertices[8];
	FD_point fsp_poly_vert[9];
	unsigned index = 0;
	// for all 4 edges of the free space diagram
	for (size_t i = 0; i < 4; i++)
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
	// now the number of vertices is stored in index and we convert
	for (unsigned i = 0; i < index; i++) {
		fsp_poly_vert[i] = ParameterToPoint(fsp_rect_edges[fsp_poly_vertices[i].fsp_rect_edge], fsp_poly_vertices[i].fsp_poly_vert_paramspace);
		DrawCircle(fsp_poly_vert[i].x, fsp_poly_vert[i].y, LINE_THICK, RED);
	}
	for (unsigned i = 0; i < index - 1; i++)
		DrawLineEx((Vector2) { fsp_poly_vert[i].x, fsp_poly_vert[i].y }, (Vector2) { fsp_poly_vert[i + 1].x, fsp_poly_vert[i + 1].y }, LINE_THICK, DARKGREEN);
	// if the polygon is at least a triangle, connect the last point to the first point
	if (index > 2)
		DrawLineEx((Vector2) { fsp_poly_vert[index - 1].x, fsp_poly_vert[index - 1].y }, (Vector2) { fsp_poly_vert[0].x, fsp_poly_vert[0].y }, LINE_THICK, DARKGREEN);
}

void VisualizeSegments()
{
	int eps = 0;
	bool eps_editing = false;
	bool P_initialized = false, Q_initialized = false;
	bool P_editing = false, Q_editing = false;
	FD_segment P = { 0 }, Q = { 0 };
	FD_point pointP = { 0 }, pointQ = { 0 };
	Color P_color = BEIGE;
	Color Q_color = PURPLE;
	Color P_color_editing = { P_color.r, P_color.g, P_color.b, P_color.a / 2 };
	Color Q_color_editing = { Q_color.r, Q_color.g, Q_color.b, Q_color.a / 2 };
	FD_freespace fsp = { 0 };
	while (!WindowShouldClose())
	{
		CalcGui();
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseInRectangle(curves_panel_rect) && !Q_editing)
		{
			if (!P_editing)
				P.p1 = (FD_point){ mouse.x, mouse.y };
			else
			{
				P.p2 = (FD_point){ mouse.x, mouse.y };
				if (Q_initialized)
					GetFreespace(P, Q, eps, &fsp);
			}
			P_initialized = P_editing;
			P_editing = !P_editing;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && IsMouseInRectangle(curves_panel_rect) && !P_editing)
		{
			if (!Q_editing)
				Q.p1 = (FD_point){ mouse.x, mouse.y };
			else
			{
				Q.p2 = (FD_point){ mouse.x, mouse.y };
				if (P_initialized)
					GetFreespace(P, Q, eps, &fsp);
			}
			Q_initialized = Q_editing;
			Q_editing = !Q_editing;
		}
		if (P_initialized && Q_initialized)
		{
			pointP = ParameterToPoint(P, mouse_fsp.x);
			pointQ = ParameterToPoint(Q, mouse_fsp.y);
		}
		BeginDrawing();
		GuiPanel(fsp_panel_rect, "Free Space");
		GuiPanel(curves_panel_rect, "Segments");
		GuiLabel(fsp_label_rect, freespace_label_text);
		if (GuiValueBox(eps_valuebox_rect, NULL, &eps, 0, 100000, eps_editing))
		{
			if (eps_editing && P_initialized && Q_initialized)
				GetFreespace(P, Q, eps, &fsp);
			eps_editing = !eps_editing;
		}
		DrawRectangleLinesEx(fsp_rect, LINE_THICK, SKYBLUE);
		if (P_initialized)
			DrawLineEx((Vector2) { P.p1.x, P.p1.y }, (Vector2) { P.p2.x, P.p2.y }, LINE_THICK, P_color);
		else if (P_editing)
			DrawLineEx((Vector2) { P.p1.x, P.p1.y }, (Vector2) { mouse.x, mouse.y }, LINE_THICK, P_color_editing);
		if (Q_initialized)
			DrawLineEx((Vector2) { Q.p1.x, Q.p1.y }, (Vector2) { Q.p2.x, Q.p2.y }, LINE_THICK, Q_color);
		else if (Q_editing)
			DrawLineEx((Vector2) { Q.p1.x, Q.p1.y }, (Vector2) { mouse.x, mouse.y }, LINE_THICK, Q_color_editing);
		if (P_initialized && Q_initialized)
		{
			DrawFreeSpace(&fsp);
			DrawCircle(pointP.x, pointP.y, 4, DARKGRAY);
			DrawCircle(pointQ.x, pointQ.y, 4, DARKGREEN);
			Color c = Hypot(pointP.x - pointQ.x, pointP.y - pointQ.y) > eps ? RED : GREEN;
			DrawCircleLines(pointP.x, pointP.y, eps, c);
		}
		EndDrawing();
	}
}
