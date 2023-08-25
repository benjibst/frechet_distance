#include "frechet_dist.h"

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