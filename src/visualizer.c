#include "visualizer.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

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
FD_segment fsp_rect_edge_left, fsp_rect_edge_bottom, fsp_rect_edge_top, fsp_rect_edge_right;
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
	curves_panel_rect = (Rectangle){0, 0, window.width / 2, window.height};
	fsp_panel_rect =
		(Rectangle){window.width / 2, 0, window.width / 2, window.height};
	int fsp_side =
		Min(fsp_panel_rect.width, fsp_panel_rect.height) * 0.8f;
	fsp_rect = (Rectangle){
		fsp_panel_rect.x + ((fsp_panel_rect.width - fsp_side) / 2),
		(window.height - fsp_side) / 2, fsp_side, fsp_side};
	fsp_rect_edge_left = (FD_segment){{fsp_rect.x, fsp_rect.y + fsp_rect.height}, {fsp_rect.x, fsp_rect.y}};
	fsp_rect_edge_right = (FD_segment){{fsp_rect.x + fsp_rect.width, fsp_rect.y + fsp_rect.height}, {fsp_rect.x + fsp_rect.width, fsp_rect.y}};
	fsp_rect_edge_bottom = (FD_segment){{fsp_rect.x, fsp_rect.y + fsp_rect.height}, {fsp_rect.x + fsp_rect.width, fsp_rect.y + fsp_rect.height}};
	fsp_rect_edge_top = (FD_segment){{fsp_rect.x, fsp_rect.y}, {fsp_rect.x + fsp_rect.width, fsp_rect.y}};
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
	fsp_label_rect = (Rectangle){fsp_rect.x, fsp_rect.y - 30, 100, 20};
	eps_valuebox_rect = (Rectangle){
		fsp_rect.x, fsp_rect.y + fsp_rect.height + 10, 80, 20};
}

static void DrawParameterSpaceRangeOnSeg(FD_segment freespace_rect_edge, FD_float range_begin, FD_float range_end)
{
	if (range_begin == range_end) // if the 2 points are the same draw a dot instead of a line(0 length line will be invisible)
	{
		FD_point p1 = ParameterSpaceToPoint(freespace_rect_edge, range_begin);
		DrawRectangle(p1.x - 3, p1.y - 3, 3, 3, DARKGREEN);
	}
	else
	{
		FD_point p1 = ParameterSpaceToPoint(freespace_rect_edge, range_begin);
		FD_point p2 = ParameterSpaceToPoint(freespace_rect_edge, range_end);
		DrawLineEx((Vector2){p1.x, p1.y}, (Vector2){p2.x, p2.y}, 3, DARKGREEN);
	}
}

static void DrawFreeSpace(const FD_freespace *const fsp)
{
	if (fsp->pass & ENTRY_LEFT)
		DrawParameterSpaceRangeOnSeg(fsp_rect_edge_left, fsp->a_ij, fsp->b_ij);
	if (fsp->pass & ENTRY_BOTTOM)
		DrawParameterSpaceRangeOnSeg(fsp_rect_edge_bottom, fsp->c_ij, fsp->d_ij);
	if (fsp->pass & EXIT_TOP)
		DrawParameterSpaceRangeOnSeg(fsp_rect_edge_top, fsp->c_ijp1, fsp->d_ijp1);
	if (fsp->pass & EXIT_RIGHT)
		DrawParameterSpaceRangeOnSeg(fsp_rect_edge_right, fsp->a_ip1j, fsp->b_ip1j);
}

void VisualizeSegments()
{
	int eps = 0;
	bool eps_editing = false;
	bool P_initialized = false, Q_initialized = false;
	bool P_editing = false, Q_editing = false;
	FD_segment P, Q;
	FD_point pointP, pointQ;
	FD_freespace fsp = {0};
	while (!WindowShouldClose())
	{
		CalcGui();
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseInRectangle(curves_panel_rect) && !Q_editing)
		{
			if (!P_editing)
				P.p1 = (FD_point){mouse.x, mouse.y};
			else
			{
				P.p2 = (FD_point){mouse.x, mouse.y};
				if (Q_initialized)
					GetFreespace(P, Q, eps, &fsp);
			}
			P_initialized = P_editing;
			P_editing = !P_editing;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && IsMouseInRectangle(curves_panel_rect) && !P_editing)
		{
			if (!Q_editing)
				Q.p1 = (FD_point){mouse.x, mouse.y};
			else
			{
				Q.p2 = (FD_point){mouse.x, mouse.y};
				if (P_initialized)
					GetFreespace(P, Q, eps, &fsp);
			}
			Q_initialized = Q_editing;
			Q_editing = !Q_editing;
		}
		if (P_initialized && Q_initialized)
		{
			pointP = ParameterSpaceToPoint(P, mouse_fsp.x);
			pointQ = ParameterSpaceToPoint(Q, mouse_fsp.y);
		}
		BeginDrawing();
		GuiPanel(fsp_panel_rect, "Free Space");
		GuiPanel(curves_panel_rect, "Curves");
		GuiLabel(fsp_label_rect, freespace_label_text);
		if (GuiValueBox(eps_valuebox_rect, NULL, &eps, 0, 100000, eps_editing))
		{
			if (eps_editing && P_initialized && Q_initialized)
				GetFreespace(P, Q, eps, &fsp);
			eps_editing = !eps_editing;
		}
		DrawRectangleLinesEx(fsp_rect, 3, SKYBLUE);
		if (P_initialized)
			DrawLineEx((Vector2){P.p1.x, P.p1.y}, (Vector2){P.p2.x, P.p2.y}, 3, BEIGE);
		else if (P_editing)
			DrawLineEx((Vector2){P.p1.x, P.p1.y}, (Vector2){mouse.x, mouse.y}, 3, BEIGE);
		if (Q_initialized)
			DrawLineEx((Vector2){Q.p1.x, Q.p1.y}, (Vector2){Q.p2.x, Q.p2.y}, 3, PURPLE);
		else if (Q_editing)
			DrawLineEx((Vector2){Q.p1.x, Q.p1.y}, (Vector2){mouse.x, mouse.y}, 3, PURPLE);
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
