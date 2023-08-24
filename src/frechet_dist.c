#include "frechet_dist_types.h"

FD_fsp_pass get_freespace(const FD_segment *const seg1, const FD_segment *const seg2,
				   FD_float eps, FD_freespace *const fsp) {
	unsigned char entrybits = 0x0F; //first set all bits
	if(OrthogonalDistance(seg2->p1,seg1)>eps)
	{
		fsp->a_ij = -1; fsp->b_ij = -1;
		//then turn off the respective ones
		entrybits &= ~((unsigned char)ENTRY_LEFT);
	}
	if(OrthogonalDistance(seg1->p1,seg2)>eps)
	{
		fsp->c_ij = -1; fsp->d_ij = -1;
		//return early to avoid computations
		if(!(entrybits&ENTRY_LEFT)) 
			return FSP_NO_ENTRY;
		entrybits &= ~((unsigned char)ENTRY_BOTTOM);
	}
	if(OrthogonalDistance(seg2->p2,seg1)>eps)
	{
		fsp->a_ip1j = -1; fsp->b_ip1j = -1;
		entrybits &= ~((unsigned char)EXIT_RIGHT);
	}
	if(OrthogonalDistance(seg1->p2,seg2)>eps)
	{
		fsp->c_ijp1 = -1; fsp->d_ijp1 = -1;
		if(!(entrybits&EXIT_RIGHT)) 
			return FSP_NO_EXIT;
		entrybits &= ~((unsigned char)EXIT_TOP);
	}
	return 0;
}