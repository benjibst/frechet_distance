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
struct
{
	int x, y;
} mouse;
struct
{
	float x, y;
} mouse_fsp;
char freespace_label_text[14];
///////////////////////////////////////////////////

static inline bool MouseInRectangle(Rectangle rect)
{
	return mouse.x >= rect.x && mouse.x <= rect.x + rect.width && mouse.y >= rect.y &&
		   mouse.y <= rect.y + rect.height;
}

static void CalcGui()
{
	mouse.x = GetMouseX();
	mouse.y = GetMouseY();
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
	if (MouseInRectangle(fsp_rect))
	{
		mouse_fsp.x =
			(float)(mouse.x - fsp_rect.x) / (float)fsp_rect.width;
		mouse_fsp.y =
			1.0f - (float)(mouse.y - fsp_rect.y) / (float)fsp_rect.height;
		if (snprintf(freespace_label_text, sizeof(freespace_label_text), "[%.2f | %.2f]",
					 mouse_fsp.x, mouse_fsp.y))
		{
		}
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
		DrawRectangle(p1.x-3, p1.y-3, 3, 3, DARKGREEN);
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

void VisualizeSegments(FD_segment P, FD_segment Q)
{
	int eps = 0;
	bool eps_editing = false;
	FD_freespace fsp = {0};
	while (!WindowShouldClose())
	{
		CalcGui();
		FD_point pointP = ParameterSpaceToPoint(P, mouse_fsp.x);
		FD_point pointQ = ParameterSpaceToPoint(Q, mouse_fsp.y);
		BeginDrawing();
		GuiPanel(fsp_panel_rect, "Free Space");
		GuiPanel(curves_panel_rect, "Curves");
		GuiLabel(fsp_label_rect, freespace_label_text);
		if (GuiValueBox(eps_valuebox_rect, NULL, &eps, 0, 100000, eps_editing))
		{
			if (eps_editing)
				GetFreespace(P, Q, eps, &fsp);
			eps_editing = !eps_editing;
		}
		DrawRectangleLinesEx(fsp_rect, 3, SKYBLUE);
		DrawFreeSpace(&fsp);
		DrawLineEx((Vector2){P.p1.x, P.p1.y}, (Vector2){P.p2.x, P.p2.y}, 3, BEIGE);
		DrawLineEx((Vector2){Q.p1.x, Q.p1.y}, (Vector2){Q.p2.x, Q.p2.y}, 3, PURPLE);
		DrawCircle(pointP.x, pointP.y, 4, DARKGRAY);
		DrawCircle(pointQ.x, pointQ.y, 4, DARKGREEN);
		Color c = Hypot(pointP.x-pointQ.x,pointP.y-pointQ.y)>eps?RED:GREEN;
		DrawCircleLines(pointP.x, pointP.y, eps, c);
		EndDrawing();
	}
}
