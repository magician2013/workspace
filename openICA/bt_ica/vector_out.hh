/**
 * @file vector_out.hh
 * @brief output vectors data as text
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __VECTOR_OUT_
#define __VECTOR_OUT_

#include <iostream>
#include <vector>
#include "mathtool.hh"

extern int vector_out( char   *inData , char *inFilename , int inLength );
extern int vector_out( short  *inData , char *inFilename , int inLength );
extern int vector_out( int    *inData , char *inFilename , int inLength );
extern int vector_out( float  *inData , char *inFilename , int inLength );
extern int vector_out( double *inData , char *inFilename , int inLength );
extern int vector_out( C_VECTOR &inData , char *inFileName , int inLength );
extern int vector_out( D_VECTOR &inData , char *inFileName , int inLength );

#endif
