#include "frechet_dist.h"
//https://mathworld.wolfram.com/Circle-LineIntersection.html

static inline int Sign(FD_float f)
{
	return (f>=0)-(f<0);
}

FD_line_circle_intersection CircleLineIntersection(FD_point center,FD_float eps,FD_segment seg,FD_point* p1,FD_point* p2)
{
	FD_float orthdist = OrthogonalDistance(center,seg)<eps;
	if(orthdist>eps)
		return PASSANT;
	seg.p1.x-=center.x; //shift the points because calculation assumes the circle is centered at [0 | 0]
	seg.p2.x-=center.x;
	seg.p1.y-=center.y;
	seg.p2.x-=center.y;
	FD_float dx = seg.p2.x - seg.p1.x;
	FD_float dy = seg.p2.y - seg.p1.y;
	FD_float drsquared = dx*dx+dy*dy;
	FD_float D = seg.p1.x*seg.p2.y-seg.p2.x*seg.p1.y;
	FD_float disc = eps*eps*drsquared-D*D;
	FD_float sqrtdisc = FD_sqrt(disc);
	if(orthdist == eps)
	{
		p1->x = (D*dy)/drsquared + center.x;
		p1->y = (-D*dx)/drsquared + center.y;
		return TANGENT;
	}
	FD_float Ddy = D*dy;
	FD_float addx = Sign(dy)*dx*sqrtdisc;
	p1->x = (Ddy+addx)/drsquared + center.x;
	p2->x = (Ddy-addx)/drsquared + center.x;
	FD_float mDdx = -D*dx;
	FD_float addy = FD_abs(dy)*sqrtdisc;
	p1->y = (mDdx+addy)/drsquared + center.y;
	p2->y = (mDdx-addy)/drsquared + center.y;
	return SECANT;
}

FD_fsp_pass GetFreespace(FD_segment P, FD_segment Q,
				   FD_float eps, FD_freespace *const fsp) {
	unsigned char entrybits = 0x0F; //first set all bits
	if(OrthogonalDistance(Q.p1,P)>eps)
	{
		fsp->a_ij = -1; fsp->b_ij = -1;
		//then turn off the respective ones
		entrybits &= ~((unsigned char)ENTRY_LEFT);
	}
	if(OrthogonalDistance(P.p1,Q)>eps)
	{
		fsp->c_ij = -1; fsp->d_ij = -1;
		//return early to avoid computations
		if(!(entrybits&ENTRY_LEFT)) 
			return FSP_NO_ENTRY;
		entrybits &= ~((unsigned char)ENTRY_BOTTOM);
	}
	if(OrthogonalDistance(Q.p2,P)>eps)
	{
		fsp->a_ip1j = -1; fsp->b_ip1j = -1;
		entrybits &= ~((unsigned char)EXIT_RIGHT);
	}
	if(OrthogonalDistance(P.p2,Q)>eps)
	{
		fsp->c_ijp1 = -1; fsp->d_ijp1 = -1;
		if(!(entrybits&EXIT_RIGHT)) 
			return FSP_NO_EXIT;
		entrybits &= ~((unsigned char)EXIT_TOP);
	}
	
	return 0;
}