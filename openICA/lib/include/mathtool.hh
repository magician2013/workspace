/**
 * @file mathtool.hh
 * @brief functions or macros for complex value
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __MATH_UTIL_H_
#define __MATH_UTIL_H_

#include <cmath>
#include <vector>

#ifdef __CYGWIN__
#include <ieeefp.h>
#endif

#include "types.hh"

/** \def PI
 */
#define PI 3.14159265258979

/** \def C_INV_NOERROR 
 *  \berief No errors in inversion of matrix
 */
#define C_INV_NOERROR 0

/** \def C_INV_ZERODET
 *  \brief det(W)=0 in inversion of matrix
 */
#define C_INV_ZERODET 1

/** \def EPS
 *  \brief epsilon for calc. ( double 10e-12 )
 */
#define EPS 1.0e-12


/***********************************************************************/
extern COMPLEX c_val( double, double );
extern COMPLEX c_add (COMPLEX a, COMPLEX b);
extern COMPLEX c_sub (COMPLEX a, COMPLEX b);
extern COMPLEX c_mul (COMPLEX a, COMPLEX b);
extern COMPLEX c_div (COMPLEX a, COMPLEX b);
extern COMPLEX c_neg(COMPLEX a );
extern COMPLEX c_conj (COMPLEX a);
extern COMPLEX c_zero ();
extern COMPLEX c_one ();
extern double c_abs (COMPLEX a);
extern double c_power (COMPLEX a);
extern COMPLEX c_exp (double theta);
extern COMPLEX c_mul_c (COMPLEX a, double c);
extern double c_real(COMPLEX z);
extern double c_img(COMPLEX z);
extern COMPLEX c_sign( COMPLEX z );
extern double c_angle( COMPLEX x );
extern int c_isnan( COMPLEX z );
extern int c_isinf( COMPLEX z );

extern void swap_filter( C_VECTOR &filter );
extern void swap_filter( C3_VECTOR &filter );

extern int c_inv( C2_VECTOR &in_matrix , C2_VECTOR &out_matrix );
extern void hanning( D_VECTOR &hw, int in_length );

/** matrix multiplier **/
extern void matrix_mul(D2_VECTOR &z, D2_VECTOR &x, D2_VECTOR &y );
extern void matrix_mul(C2_VECTOR &z, C2_VECTOR &x, C2_VECTOR &y );

#endif
