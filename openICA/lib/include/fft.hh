/**
 * @file fft.hh
 * @brief definition for FFT
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __FFT_H_
#define __FFT_H_

#include <vector>
#include "types.hh"

/*! \def FFT_NO_ERROR
	  fft is successfully completed
 */
#define FFT_NO_ERROR          0
/*! \def FFT_ERR_NO_MEMORY
	  error depend on few memory
 */
#define FFT_ERR_NO_MEMORY     1
/*! \def FFT_ERR_NO_POWEROF2
	  input data length is not power of two.
 */
#define FFT_ERR_NO_POWEROF2   2

/*==================================================
 * Prototyping
 *--------------------------------------------------*/
extern int fft( int size , double *in_re , double *in_im , double *out_re, double *out_im );
extern int fft( int size , std::vector<COMPLEX>&in , std::vector<COMPLEX>&out);
extern int fft2( int size , C2_VECTOR &in , C2_VECTOR &out , int factor );
extern int fft3( int size , C3_VECTOR &in , C3_VECTOR &out , int factor );

#endif
