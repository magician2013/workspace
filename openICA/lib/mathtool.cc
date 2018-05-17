/**
 * @file mathtool.cc
 * @brief functions for complex value
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <iostream>

#include "mathtool.hh"

/** '1' in complex value ( Re = 1 , Im = 0 )
 *
 */
COMPLEX c_one( )
{
	COMPLEX z;
	z.re = 1.0;
	z.im = 0.0;

	return z;
}
/** set a complex value with specified real part, and imag. part.
 *  @param re real part
 *  @param im imaginary part
 *
 *  @retval specified complex value
 */
COMPLEX c_val( FLOAT re , FLOAT im )
{
	COMPLEX z;
	z.re = re;
	z.im = im;
	return z;
}

/** add in complex value (returns a + b )
 * @param a complex value 1
 * @param b complex value 2
 *
 * @retval returns a + b in complex value.
 */
COMPLEX c_add( COMPLEX a , COMPLEX b )
{
	COMPLEX z;
	z.re = a.re + b.re;
	z.im = a.im + b.im;
	return z;
}
/** sub in complex value ( returns a - b )
 * @param a complex value 1
 * @param b complex value 2
 *
 * @retval returns a - b in complex value.
 */
COMPLEX c_sub( COMPLEX a , COMPLEX b )
{
	COMPLEX z;
	z.re = a.re - b.re;
	z.im = a.im - b.im;
	return z;
}
/** mul in complex value ( returns a *b )
 * @param a complex value 1
 * @param b complex value 2
 *
 * @retval returns a * b in complex value.
 */
COMPLEX c_mul( COMPLEX a , COMPLEX b )
{
	COMPLEX z;
	z.re = a.re*b.re - a.im*b.im;
	z.im = a.im*b.re + a.re*b.im;
	return z;
}
/** div in complex value ( returns a/b )
 * @param a complex value 1
 * @param b complex value 2
 *
 * @retval returns a / b in complex value.
 */
COMPLEX c_div( COMPLEX a , COMPLEX b )
{
	COMPLEX z;
	FLOAT w;
	
	w = b.re*b.re+b.im*b.im;
	z.re = (a.re*b.re + a.im*b.im)/w;
	z.im = (a.im*b.re - a.re*b.im)/w;
	return z;
}
/** returns conjugation of complex value 'a'
 * @param a complex value
 *
 * @retval returns conj(a)
 */
COMPLEX c_conj( COMPLEX a )
{
	a.im = -a.im;
	return a;
}
/** returns zero in complex value
 */
COMPLEX c_zero( )
{
	COMPLEX z;
	z.re = z.im = 0.0;
	return z;
}
/** returns oppsite sign in complex value
 * @param a complex value
 *
 */
COMPLEX c_neg(COMPLEX a )
{
	COMPLEX z;
	z.re = -a.re;
	z.im = -a.im;
	return z;
}
/**returns absolute value of complex value
 * @param a complex value
 *
 */
FLOAT c_abs(COMPLEX a)
{
	return(sqrt(a.re*a.re+a.im*a.im));
}
/**returns power of complex value
 * @param a complex value
 *
 */
FLOAT c_power(COMPLEX a)
{
	return(a.re*a.re+a.im*a.im);
}
/**returns exponential in complex value
 * @param a complex value
 *
 * @retval exp(a) in complex value
 */
COMPLEX c_exp( FLOAT theta )
{
	COMPLEX z;
	z.re = cos(theta);
	z.im = sin(theta);
	return(z);
}
/** returns a * c, a: complex value, c: constant real value
 * @param a complex value
 * @param c 

 * @retval exp(a) in complex value
 */
COMPLEX c_mul_c( COMPLEX a , FLOAT c )
{
	COMPLEX z;
	z.re = c * a.re;
	z.im = c * a.im;
	return(z);
}
/** angle of a complex value
 *
 * @param x complex value
 * @retval arg(x)
 */
FLOAT c_angle( COMPLEX x )
{
	return( atan2( x.im, x.re) );
}

/** inversion of 2x2 matrix
 * @param in_matrix   input matrix
 * @param out_matrix  output matrix
 *
 * @retval C_INV_NOERROR(0) no error
 * @retval C_INV_ZERODET(1) det(in_matrix) == 0
 */
int c_inv( C2_VECTOR &in_matrix , C2_VECTOR &out_matrix )
{
	COMPLEX AD_BC;
	AD_BC = c_sub(c_mul(in_matrix[0][0],in_matrix[1][1]),c_mul(in_matrix[0][1],in_matrix[1][0]));
	if( c_abs(AD_BC) < EPS ) {
		out_matrix[0][0] = c_one();
		out_matrix[1][0] = c_zero();
		out_matrix[0][1] = c_zero();
		out_matrix[1][1] = c_one();
		return C_INV_ZERODET;
	} else {
		out_matrix[0][0] = c_div(in_matrix[1][1],AD_BC);
		out_matrix[1][0] = c_neg(c_div(in_matrix[1][0],AD_BC));
		out_matrix[0][1] = c_neg(c_div(in_matrix[0][1],AD_BC));
		out_matrix[1][1] = c_div(in_matrix[0][0],AD_BC);
		return C_INV_NOERROR;
	}
}

/** check NaN of complex value z
 * @param z complex value
 *
 * @retval 0 z is not NaN
 * @retval 1 z is NaN
 */
int c_isnan( COMPLEX z )
{
	if( isnan(z.re) || isnan(z.im) )
		return 1;
	return 0;
}

/** check Inf of complex value z
 * @param z complex value
 *
 * @retval 0 z is not Inf
 * @retval 1 z is Inf
 */
int c_isinf( COMPLEX z )
{
	if( isinf(z.re) || isinf(z.im) )
		return 1;
	return 0;
}

/** real part of a complex value
 * @param z complex value
 */
FLOAT c_real( COMPLEX z )
{
	return z.re;
}
/** imaginary part of a complex value
 * @param z complex value
 */
FLOAT c_img( COMPLEX z )
{
	return z.im;
}

/** sign (swithcing) function in complex value
 *  
 * @param z complex value
 */
COMPLEX c_sign( COMPLEX z )
{
	COMPLEX a;
	if( fabs(z.re) < EPS )
		a.re =0.0;
	else if( z.re > 0 )
		a.re = 1.0;
	else
		a.re = -1.0;

	if( fabs(z.im) < EPS )
		a.im =0.0;
	else if( z.im > 0 )
		a.im = 1.0;
	else
		a.im = -1.0;
	
	return a;
}

/** matrix multiplier 
 *  
 * @param z output matrix in real value
 * @param x input matrix  in real value
 * @param y input matrix  in real value
 *
 */
void matrix_mul(D2_VECTOR &z, D2_VECTOR &x, D2_VECTOR &y )
{
  int i,j,k;

	int n = x.size();
	int m = x[0].size();
	int l = y[0].size();

  for(i=0; i<n; i++){
    for(j=0; j<l; j++){
      z[i][j] = 0.0;
      for(k=0; k<m; k++){
        z[i][j] += x[i][k] * y[k][j];
      }
    }
  }
}
/** matrix multiplier 
 *  
 * @param z output matrix in complex value
 * @param x input matrix  in complex value
 * @param y input matrix  in complex value
 *
 */
void matrix_mul(C2_VECTOR &z, C2_VECTOR &x, C2_VECTOR &y )
{
  int i,j,k;

	int n = x.size();
	int m = x[0].size();
	int l = y[0].size();

  for(i=0; i<n; i++){
    for(j=0; j<l; j++){
      z[i][j] = c_zero();
      for(k=0; k<m; k++){
        z[i][j] = c_add( z[i][j], c_mul( x[i][k], y[k][j] ) );
      }
    }
  }
}
