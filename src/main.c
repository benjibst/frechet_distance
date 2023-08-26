#include "frechet_dist.h"
#include "visualizer.h"
#include <stdio.h>

int main(void) {
	InitGUI();
	VisualizeSegments((FD_segment){{100,100},{300,100}},(FD_segment){{200,200},{200,300}});
}
