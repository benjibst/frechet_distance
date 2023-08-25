#ifndef __FRECHET_DIST_H__
#define __FRECHET_DIST_H__

#include "frechet_dist_types.h"
#include "geometry.h"

#ifndef __WIN32__
#define Min(x,y) ((x)<(y)?(x):(y))
#define Max(x,y) ((x)>(y)?(x):(y))
#endif

//computes a free space cell for 2 line segments with a given epsilon
void GetFreespace(FD_segment P, FD_segment Q, FD_float eps,
						 FD_freespace *const fsp);

//sets freespace entry or exit interval for one side and returns if entry or exit from that side is possible
bool GetFreeSpaceOneSide(FD_point p, FD_segment seg, FD_float eps,
						 FD_float* fsp_entry_range_begin,FD_float* fsp_entry_range_end);
#endif // __FRECHET_DIST_H__