#include "visualizer.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

void InitGUI() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WINDOW_X, WINDOW_Y, WINDOW_NAME);
	SetTargetFPS(60);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
}

/////////////////////////////////////////gui data
Rectangle curves_panel_rect;
Rectangle freespace_panel_rect;
Rectangle freespace_rect;
Rectangle freespace_label_rect;
Rectangle eps_valuebox_rect;
struct {
	int width, height;
} window;
struct {
	int x, y;
} mouse;
struct {
	float x, y;
} mouse_free_space;
char freespace_label_text[14];
///////////////////////////////////////////////////

static inline bool MouseInRectangle(Rectangle rect) {
	return mouse.x >= rect.x && mouse.x <= rect.x + rect.width && mouse.y >= rect.y &&
		   mouse.y <= rect.y + rect.height;
}

static void CalcGui() {
	mouse.x = GetMouseX();
	mouse.y = GetMouseY();
	window.width = GetScreenWidth();
	window.height = GetScreenHeight();
	curves_panel_rect = (Rectangle){0, 0, window.width / 2, window.height};
	freespace_panel_rect =
		(Rectangle){window.width / 2, 0, window.width / 2, window.height};
	int freespace_side =
		Min(freespace_panel_rect.width, freespace_panel_rect.height) * 0.8f;
	freespace_rect = (Rectangle){
		freespace_panel_rect.x + ((freespace_panel_rect.width - freespace_side) / 2),
		(window.height - freespace_side) / 2, freespace_side, freespace_side};
	if (MouseInRectangle(freespace_rect)) {
		mouse_free_space.x =
			(float)(mouse.x - freespace_rect.x) / (float)freespace_rect.width;
		mouse_free_space.y =
			1.0f - (float)(mouse.y - freespace_rect.y) / (float)freespace_rect.height;
		if (snprintf(freespace_label_text, sizeof(freespace_label_text), "[%.2f | %.2f]",
					 mouse_free_space.x, mouse_free_space.y)) {
		}
	} else
		freespace_label_text[0] = 0;
	freespace_label_rect = (Rectangle){freespace_rect.x, freespace_rect.y - 30, 100, 20};
	eps_valuebox_rect = (Rectangle){
		freespace_rect.x, freespace_rect.y + freespace_rect.height + 10, 80, 20};
}

static void DrawFreeSpace(const FD_freespace *const fsp) {
	// if freespace is invalid fill with red
	if (fsp->pass == FSP_NO_ENTRY || fsp->pass == FSP_NO_EXIT)
		DrawRectangle(freespace_rect.x, freespace_rect.y, freespace_rect.width,
					  freespace_rect.height, RED);
	if (fsp->pass & ENTRY_LEFT) {
		FD_segment border_left = {
			{freespace_rect.x, freespace_rect.y + freespace_rect.height},
			{freespace_rect.x, freespace_rect.y}};
		FD_point entry_range_left_begin = ParameterSpaceToPoint(border_left, fsp->a_ij);
		FD_point entry_range_left_end = ParameterSpaceToPoint(border_left, fsp->b_ij);
		DrawLineEx((Vector2){entry_range_left_begin.x, entry_range_left_begin.y},
				   (Vector2){entry_range_left_end.x, entry_range_left_end.y},3,DARKGREEN);
	}
}

void VisualizeSegments(FD_segment P, FD_segment Q) {
	int eps = 0;
	bool eps_editing = false;
	FD_freespace fsp;
	while (!WindowShouldClose()) {
		CalcGui();
		FD_point pointP = ParameterSpaceToPoint(P, mouse_free_space.x);
		FD_point pointQ = ParameterSpaceToPoint(Q, mouse_free_space.y);
		BeginDrawing();
		GuiPanel(freespace_panel_rect, "Free Space");
		GuiPanel(curves_panel_rect, "Curves");
		GuiLabel(freespace_label_rect, freespace_label_text);
		if (GuiValueBox(eps_valuebox_rect, NULL, &eps, 0, 100000, eps_editing)) {
			if (eps_editing)
				GetFreespace(P, Q, eps, &fsp);
			eps_editing = !eps_editing;
		}
		DrawRectangleLinesEx(freespace_rect, 3, SKYBLUE);
		DrawFreeSpace(&fsp);
		DrawLineEx((Vector2){P.p1.x, P.p1.y}, (Vector2){P.p2.x, P.p2.y}, 3, BEIGE);
		DrawLineEx((Vector2){Q.p1.x, Q.p1.y}, (Vector2){Q.p2.x, Q.p2.y}, 3, PURPLE);
		DrawCircle(pointP.x, pointP.y, 4, GREEN);
		DrawCircle(pointQ.x, pointQ.y, 4, GREEN);
		DrawCircleLines(pointP.x, pointP.y, eps, GREEN);
		EndDrawing();
	}
}
