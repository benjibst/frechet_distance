#include <stdio.h>
#include "visualizer.h"
#include "frechet_dist.h"

int main(void)
{
    FD_point center = {1,1};
    FD_segment seg = {{-1030,-2001},{1.000005,2.0001}};
    FD_point p1;
    FD_point p2;
    CircleLineIntersection(center,1,seg,&p1,&p2);
    printf("%d %d\n",IsPointInSegment(p1,seg),IsPointInSegment(p2,seg));
    /* InitGUI();
    VisualizeSegments((FD_segment){{100,100},{300,400}},(FD_segment){{100,200},{400,300}}); */
}
