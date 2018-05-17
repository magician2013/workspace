/**
 * @file svdcmp.cc
 * @brief singular valued decompotision and psudo inversion based on Neumerical Recipes in C
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svdcmp.hh"
#ifndef NR_END_SVD
#define NR_END_SVD 1
#endif

#ifndef SIGN_SVD
#define SIGN_SVD(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#endif

#ifndef FMAX_SVD
#define FMAX_SVD(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef IMIN_SVD
#define IMIN_SVD(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef SQR_SVD
#define SQR_SVD(a) ( (a) == 0.0 ? 0.0 : (a)*(a))
#endif

#ifndef FREE_ARG_SVD
#define FREE_ARG_SVD char*
#endif

#ifndef __MIN
#define __MIN(a,b) ( a < b ? a : b )
#endif



/** Pseudo inversion of complex-valued matrix based on SVD.
 *
 *  @param A input matrix
 *  @param B output matrix
 *  @param tol tolerance of SVD
 *
 *  @retval rank of B
 */
int pinv(C2_VECTOR &A, C2_VECTOR &B, FLOAT tol )
{
	int m = A.size();
	int n = A[0].size();
	int i,j,r;

	/** Allocate memories for SVD **/
	D_VECTOR  lambda( 2*n + 1 );
	D2_VECTOR U( 2*n + 1, D_VECTOR(2*n + 1) );
	D2_VECTOR V( 2*n + 1, D_VECTOR(2*n + 1) );

	/** Allocate memory for pinv **/
	D2_VECTOR w1( 2*n, D_VECTOR(2*n) );
	D2_VECTOR w2( 2*n, D_VECTOR(2*n) );
	D2_VECTOR w3( 2*n, D_VECTOR(2*n) );
	D2_VECTOR mlambda( 2*n,  D_VECTOR(2*n) );

	/** Copy from specified matrix A **/
	for( i = 1 ; i < m + 1 ; i++ ) {
		for( j = 1 ; j < n + 1; j++ ) {
			U[i][j]     =  A[i-1][j-1].re;
			U[i][j+n]   = -A[i-1][j-1].im;
			U[i+m][j]   =  A[i-1][j-1].im;
			U[i+m][j+n] =  A[i-1][j-1].re;
		}
	}

	/** Singular value decomposition **/
	svdcmp( U, 2*m, 2*n, lambda, V );

	/** ╢коб **/
	for( i = 1, r = 0; i < __MIN(2*n+1,2*m+1) ; i++ ){
		if( lambda[i] > tol ){
			mlambda[i-1][i-1] = 1.0 / lambda[i];
			r++;
		} else {
			lambda[i] = mlambda[i-1][i-1] = 0.0;
		}
	}

	/** pinv **/
	for( i = 1 ; i < 2*n + 1 ; i++ ) {
		for( j = 1 ; j < 2*n + 1; j++ ) {
			w2[i-1][j-1] = V[i][j];
		}
	}
	matrix_mul( w1, w2, mlambda );

	/** generate transpose of U **/
	for( i = 1 ; i < 2*m + 1 ; i++ ) {
		for( j = 1 ; j < 2*m + 1; j++ ) {
			w2[i-1][j-1] = U[j][i];
		}
	}
	
	/** resize output matrix **/
	matrix_mul( w3, w1, w2);

	/** resize output matrix **/
	B[0].resize(m);
	B.resize(n);
	
	/** Reconstruct complex valued matrix **/
	for( i = 0 ; i < m; i++ ) {
		for( j = 0 ; j < n; j++ ) {
			B[i][j].re = w3[i][j];
			B[i][j].im = w3[i+m][j];
		}
	}
	return r;
}

/** Pseudo inversion of real-valued matrix based on SVD.
 *
 *  @param A input matrix
 *  @param B output matrix
 *  @param tol tolerance of SVD
 *
 *  @retval rank of B
 */
int pinv(D2_VECTOR &A, D2_VECTOR &B, FLOAT tol )
{
	int m = A.size();
	int n = A[0].size();
	int i,j,r;

	/** Allocate memories for SVD **/
	D2_VECTOR U( n + 1, D_VECTOR(n + 1) );
	D_VECTOR  lambda( n + 1 );
	D2_VECTOR V( n + 1, D_VECTOR(n + 1) );

	/** Allocate memory for pinv **/
	D2_VECTOR w1( n, D_VECTOR(n) );
	D2_VECTOR w2( n, D_VECTOR(n) );
	D2_VECTOR mlambda( n,  D_VECTOR(n) );

	/** Copy from specified matrix A **/
	for( i = 1 ; i < m + 1 ; i++ ) {
		for( j = 1 ; j < n + 1; j++ ) {
			U[i][j] = A[i-1][j-1];
		}
	}

	/** Singular value decomposition **/
	svdcmp( U, m, n, lambda, V );

	for( i = 1, r = 0; i < __MIN(n+1,m+1) ; i++ ){
		if( lambda[i] > tol ){
			mlambda[i-1][i-1] = 1.0 / lambda[i];
			r++;
		} else {
			lambda[i] = mlambda[i-1][i-1] = 0.0;
		}
	}

	/** pinv **/
	for( i = 1 ; i < n + 1 ; i++ ) {
		for( j = 1 ; j < n + 1; j++ ) {
			w2[i-1][j-1] = V[i][j];
		}
	}
	matrix_mul( w1, w2, mlambda );

	/** generate transpose of U **/
	for( i = 1 ; i < m + 1 ; i++ ) {
		for( j = 1 ; j < m + 1; j++ ) {
			w2[i-1][j-1] = U[j][i];
		}
	}
	
	/** resize output matrix **/
	B[0].resize(m);
	B.resize(n);

	matrix_mul( B, w1, w2);

	return r;
}
/*
   Singular value decomposition (SVD)
	 [ a, w, v ] = svd( a );

	 a = [ m x n ] sized matrix in real-valued
	 
	 Note:
     Practically, matrix [a] requires a[0...m][0...m].
	   Then, matrix [a] : [(m+1) x (m+1)] sized matrix.

	 Moreover, data should be put into a[1...m][1...n].

	 w[0...m] : output singular values. data is in w[1...m].
	 v[0...n][0...n] : data is in v[1...n][1...n] 

	 Note:
	  When the matrix[a] is singular matrix, m < n is required.
		If we want to using matrix [a] sized [ m > n ], 
		we must zero padding so that matrix [a] becomes non-singular matrix.
		(or using transpose of matrix [a] )
 */

void svdcmp(D2_VECTOR &a, int m, int n, D_VECTOR &w, D2_VECTOR &v)
{
	FLOAT pythag(FLOAT a, FLOAT b);
	FLOAT *vector_svd(long nl, long nh);
	void free_vector_svd(FLOAT *v, long nl, long nh);

	int flag,i,its,j,jj,k;
	int l = 0; int nm = 0;
	FLOAT anorm,c,f,g,h,s,scale,x,y,z,*rv1;

	rv1=vector_svd(1,n);
	g=scale=anorm=0.0;
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabs(a[k][i]);
			if (scale) {
				for (k=i;k<=m;k++) {
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
				}
				f=a[i][i];
				g = -SIGN_SVD(sqrt(s),f);
				h=f*g-s;
				a[i][i]=f-g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
					f=s/h;
					for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
				}
				for (k=i;k<=m;k++) a[k][i] *= scale;
			}
		}
		w[i]=scale *g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabs(a[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = -SIGN_SVD(sqrt(s),f);
				h=f*g-s;
				a[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
				for (j=l;j<=m;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
					for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
				}
				for (k=l;k<=n;k++) a[i][k] *= scale;
			}
		}
		anorm=FMAX_SVD(anorm,(fabs(w[i])+fabs(rv1[i])));
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++)
					v[j][i]=(a[i][j]/a[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=IMIN_SVD(m,n);i>=1;i--) {
		l=i+1;
		g=w[i];
		for (j=l;j<=n;j++) a[i][j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<=n;j++) {
				for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
				f=(s/a[i][i])*g;
				for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
			}
			for (j=i;j<=m;j++) a[j][i] *= g;
		} else for (j=i;j<=m;j++) a[j][i]=0.0;
		++a[i][i];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=30;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if ((FLOAT)(fabs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ((FLOAT)(fabs(w[nm])+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if ((FLOAT)(fabs(f)+anorm) == anorm) break;
					g=w[i];
					h=pythag(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=1;j<=m;j++) {
						y=a[j][nm];
						z=a[j][i];
						a[j][nm]=y*c+z*s;
						a[j][i]=z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=pythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN_SVD(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=pythag(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=pythag(f,h);
				w[j]=z;
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=1;jj<=m;jj++) {
					y=a[jj][j];
					z=a[jj][i];
					a[jj][j]=y*c+z*s;
					a[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	free_vector_svd(rv1,1,n);
}

FLOAT pythag(FLOAT a, FLOAT b)
{
	FLOAT absa,absb;
	absa=fabs(a);
	absb=fabs(b);
	if (absa > absb) return absa*sqrt(1.0+SQR_SVD(absb/absa));
	else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR_SVD(absa/absb)));
}

FLOAT *vector_svd(long nl, long nh)
/* allocate a FLOAT vector with subscript range v[nl..nh] */
{
	FLOAT *v;

	v=(FLOAT *)malloc((size_t) ((nh-nl+1+NR_END_SVD)*sizeof(FLOAT)));
	if (!v) fprintf(stderr,"allocation failure in vector()\n");
	return v-nl+NR_END_SVD;
}

void free_vector_svd(FLOAT *v, long nl, long nh)
/* free a FLOAT vector allocated with vector() */
{
	free((FREE_ARG_SVD) (v+nl-NR_END_SVD));
}

