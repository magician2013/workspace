/**
 * @file svdcmp.hh
 * @brief singular valued decompotision and psudo inversion based on Neumerical Recipes in C
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __SVDCMP_H_
#define __SVDCMP_H_

#include "mathtool.hh"

extern int pinv( D2_VECTOR &A, D2_VECTOR &B, FLOAT tol );
extern int pinv (C2_VECTOR &A, C2_VECTOR &B, FLOAT tol );
extern void svdcmp(D2_VECTOR &a, int m, int n, D_VECTOR &w, D2_VECTOR &v);
extern FLOAT pythag(FLOAT a, FLOAT b);
extern FLOAT *vector_svd(long nl, long nh);
extern void free_vector_svd(FLOAT *v, long nl, long nh);

#endif
