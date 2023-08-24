#include "visualizer.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

void InitGUI()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WINDOW_X, WINDOW_Y, WINDOW_NAME);
	SetTargetFPS(60);
	GuiSetStyle(DEFAULT,TEXT_SIZE,20);
}

/////////////////////////////////////////gui data
Rectangle curves_panel_rect;
Rectangle freespace_panel_rect;
Rectangle freespace_rect;
Rectangle freespace_label_rect;
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
} mouse_free_space;
char freespace_label_text[12];
///////////////////////////////////////////////////

static inline bool MouseInRectangle(Rectangle rect)
{
	return mouse.x >= rect.x &&
		   mouse.x <= rect.x + rect.width &&
		   mouse.y >= rect.y &&
		   mouse.y <= rect.y + rect.height;
}

static void CalcGui()
{
	mouse.x = GetMouseX();
	mouse.y = GetMouseY();
	window.width = GetScreenWidth();
	window.height = GetScreenHeight();
	curves_panel_rect = (Rectangle){0, 0, window.width / 2, window.height};
	freespace_panel_rect =
		(Rectangle){window.width / 2, 0, window.width / 2, window.height};
	int freespace_side =
		min(freespace_panel_rect.width, freespace_panel_rect.height) * 0.8f;
	freespace_rect = (Rectangle){
		freespace_panel_rect.x + ((freespace_panel_rect.width - freespace_side) / 2),
		(window.height - freespace_side) / 2, freespace_side, freespace_side};
	if (MouseInRectangle(freespace_rect))
	{
		mouse_free_space.x =
			(float)(mouse.x - freespace_rect.x) / (float)freespace_rect.width;
		mouse_free_space.y =
			1.0f - (float)(mouse.y - freespace_rect.y) / (float)freespace_rect.height;
		sprintf(freespace_label_text, "[%.2f | %.2f]", mouse_free_space.x,
				mouse_free_space.y);
	}
	else
		freespace_label_text[0] = 0;
	freespace_label_rect = (Rectangle){freespace_rect.x, freespace_rect.y - 30, 100, 20};
	eps_valuebox_rect = (Rectangle){freespace_rect.x, freespace_rect.y + freespace_rect.height + 10, 80, 20};
}

void VisualizeSegments(FD_segment P, FD_segment Q)
{
	int eps = 0;
	bool eps_editing = false;
	while (!WindowShouldClose())
	{
		CalcGui();
		BeginDrawing();
		GuiPanel(freespace_panel_rect, "Free Space");
		GuiPanel(curves_panel_rect, "Curves");
		GuiLabel(freespace_label_rect, freespace_label_text);
		if (GuiValueBox(eps_valuebox_rect, NULL, &eps, 0, 100000, eps_editing))
		{
			if (eps_editing)
				printf("Value: %d\n", eps);
			eps_editing = !eps_editing;
		}
		DrawRectangleLinesEx(freespace_rect, 3, SKYBLUE);
		DrawLineEx((Vector2){P.p1.x, P.p1.y}, (Vector2){P.p2.x, P.p2.y}, 3, BEIGE);
		DrawLineEx((Vector2){Q.p1.x, Q.p1.y}, (Vector2){Q.p2.x, Q.p2.y}, 3, PURPLE);
		FD_point pointP = PointAlongSegment(P, mouse_free_space.x);
		FD_point pointQ = PointAlongSegment(Q, mouse_free_space.y);
		DrawCircle(pointP.x, pointP.y, 4, GREEN);
		DrawCircle(pointQ.x, pointQ.y, 4, GREEN);
		EndDrawing();
	}
}
