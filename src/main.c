#include <stdio.h>
#include "visualizer.h"

int main(void)
{
    InitGUI();
    VisualizeSegments((FD_segment){{100,100},{300,400}},(FD_segment){{100,200},{400,300}});
}
