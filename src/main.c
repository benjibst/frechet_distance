#include <stdio.h>
#include "visualizer.h"
#include "frechet_dist.h"

int main(void)
{
    FD_point center = {1,1};
    FD_segment line = {{1,0},{1,2}};
    FD_float radius = 1;
    FD_point p1;
    FD_point p2;
    FD_line_circle_intersection intersect = CircleLineIntersection(center,radius,line,&p1,&p2);
    switch (intersect)
    {
    case PASSANT:
        printf("Passant\n");
        break;
    case TANGENT:
        printf("Tangent in: [%.2f | %.2f]\n",p1.x,p1.y);
        break;
    case SECANT:
        printf("Secant in: [%.2f | %.2f] [%.2f | %.2f]",p1.x,p1.y,p2.x,p2.y);
    }
/*     InitGUI();
    VisualizeSegments((FD_segment){{100,100},{300,400}},(FD_segment){{100,200},{400,300}}); */
}
