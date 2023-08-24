#include "visualizer.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

void InitGUI() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WINDOW_X, WINDOW_Y, WINDOW_NAME);
	SetTargetFPS(60);
}

/////////////////////////////////////////gui data
Rectangle curves_window_rect;
Rectangle freespace_window_rect;
Rectangle freespace_rect;
Rectangle freespace_label_rect;
Rectangle eps_spinner_rect;
struct {
	int width, height;
} window;
struct {
	int x, y;
} mouse;
struct {
	float x, y;
} mouse_free_space;
char freespace_label_text[12];
///////////////////////////////////////////////////

static inline bool MouseInFreeSpace() {
	return mouse.x >= freespace_rect.x &&
		   mouse.x <= freespace_rect.x + freespace_rect.width &&
		   mouse.y >= freespace_rect.y &&
		   mouse.y <= freespace_rect.y + freespace_rect.height;
}

static void CalcGui() {
	mouse.x = GetMouseX();
	mouse.y = GetMouseY();
	window.width = GetScreenWidth();
	window.height = GetScreenHeight();
	curves_window_rect = (Rectangle){0, 0, window.width / 2, window.height};
	freespace_window_rect =
		(Rectangle){window.width / 2, 0, window.width / 2, window.height};
	int freespace_side =
		min(freespace_window_rect.width, freespace_window_rect.height) * 0.8f;
	freespace_rect = (Rectangle){
		freespace_window_rect.x + ((freespace_window_rect.width - freespace_side) / 2),
		(window.height - freespace_side) / 2, freespace_side, freespace_side};
	if (MouseInFreeSpace()) {
		mouse_free_space.x =
			(float)(mouse.x - freespace_rect.x) / (float)freespace_rect.width;
		mouse_free_space.y =
			1.0f - (float)(mouse.y - freespace_rect.y) / (float)freespace_rect.height;
		sprintf(freespace_label_text, "[%.2f,%.2f]", mouse_free_space.x,
				mouse_free_space.y);
	} else
		freespace_label_text[0] = 0;
	freespace_label_rect = (Rectangle){freespace_rect.x, freespace_rect.y - 30, 100, 20};
	eps_spinner_rect = (Rectangle){freespace_rect.x,freespace_rect.y + freespace_rect.height+10,50,20};
}

void VisualizeSegments(FD_segment P, FD_segment Q) {
	int eps;
	while (!WindowShouldClose()) {
		CalcGui();
		BeginDrawing();
		GuiWindowBox(freespace_window_rect, "Free Space");
		GuiWindowBox(curves_window_rect, "Curves");
		GuiLabel(freespace_label_rect, freespace_label_text);
		int eps2 = GuiValueBox(eps_spinner_rect,"eps",&eps,0,1000000,false);
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
