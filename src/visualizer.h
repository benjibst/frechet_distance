#ifndef __VISUALIZER_H__
#define __VISUALIZER_H__

#include "raylib.h"
#include "raygui.h"
#include "frechet_dist.h"
#include <stdio.h>

#define WINDOW_X 800
#define WINDOW_Y 500
#define WINDOW_NAME "Frechet distance"

void InitGUI();
void VisualizeSegments();

#endif // __VISUALIZER_H__