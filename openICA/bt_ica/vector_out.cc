/**
 * @file vector_out.cc
 * @brief output vector data
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <iostream>
#include <vector>
#include <cstdio>
#include "mathtool.hh"

/** output char-vector
 * @param inData target vector
 * @param inFileName output filenmae 
 * @param inLength length of target vector
 *
 * @retval (0) : No error occurred.
 * @retval (1) : Cannot open output file
 */
int vector_out( char *inData , char *inFileName , int inLength )
{
	FILE  *fp;
	int i;

	if( ( fp = fopen( inFileName , "wb" ) ) == NULL ) {
		return 1;
	}

	for ( i = 0 ; i < inLength ; i++ ) {
		fprintf( fp , "%d %d\n",i,inData[i]);
	}

	fclose(fp);

	return 0;
}
/** output short-vector
 * @param inData target vector
 * @param inFileName output filenmae 
 * @param inLength length of target vector
 *
 * @retval (0) : No error occurred.
 * @retval (1) : Cannot open output file
 */
int vector_out( short *inData , char *inFileName , int inLength )
{
	FILE  *fp;
	int i;

	if( ( fp = fopen( inFileName , "wb" ) ) == NULL ) {
		return 1;
	}

	for ( i = 0 ; i < inLength ; i++ ) {
		fprintf( fp , "%d %d\n",i,inData[i]);
	}

	fclose(fp);

	return 0;
}
/** output int-vector
 * @param inData target vector
 * @param inFileName output filenmae 
 * @param inLength length of target vector
 *
 * @retval (0) : No error occurred.
 * @retval (1) : Cannot open output file
 */
int vector_out( int *inData , char *inFileName , int inLength )
{
	FILE  *fp;
	int i;

	if( ( fp = fopen( inFileName , "wb" ) ) == NULL ) {
		return 1;
	}

	for ( i = 0 ; i < inLength ; i++ ) {
		fprintf( fp , "%d %d\n",i,inData[i]);
	}

	fclose(fp);

	return 0;
}
/** output float-vector
 * @param inData target vector
 * @param inFileName output filenmae 
 * @param inLength length of target vector
 *
 * @retval (0) : No error occurred.
 * @retval (1) : Cannot open output file
 */
int vector_out( float *inData , char *inFileName , int inLength )
{
	FILE  *fp;
	int i;

	if( ( fp = fopen( inFileName , "wb" ) ) == NULL ) {
		return 1;
	}

	for ( i = 0 ; i < inLength ; i++ ) {
		fprintf( fp , "%d %g\n",i,inData[i]);
	}

	fclose(fp);

	return 0;
}
/** output double-vector
 * @param inData target vector
 * @param inFileName output filenmae 
 * @param inLength length of target vector
 *
 * @retval (0) : No error occurred.
 * @retval (1) : Cannot open output file
 */
int vector_out( double *inData , char *inFileName , int inLength )
{
	FILE  *fp;
	int i;

	if( ( fp = fopen( inFileName , "wb" ) ) == NULL ) {
		return 1;
	}

	for ( i = 0 ; i < inLength ; i++ ) {
		fprintf( fp , "%d %g\n",i,inData[i]);
	}

	fclose(fp);

	return 0;
}
/** output C_VECTOR-vector
 * @param inData target vector
 * @param inFileName output filenmae 
 * @param inLength length of target vector
 *
 * @retval (0) : No error occurred.
 * @retval (1) : Cannot open output file
 */
int vector_out( C_VECTOR &inData, char *inFileName , int inLength )
{
	FILE  *fp;
	int i;

	if( ( fp = fopen( inFileName , "wb" ) ) == NULL ) {
		return 1;
	}

	for ( i = 0 ; i < inLength ; i++ ) {
		fprintf( fp , "%d %g  %g\n",i,inData[i].re,inData[i].im);
	}

	fclose(fp);

	return 0;
}
/** output D_VECTOR-vector
 * @param inData target vector
 * @param inFileName output filenmae 
 * @param inLength length of target vector
 *
 * @retval (0) : No error occurred.
 * @retval (1) : Cannot open output file
 */
int vector_out( D_VECTOR &inData, char *inFileName , int inLength )
{
	FILE  *fp;
	int i;

	if( ( fp = fopen( inFileName , "wb" ) ) == NULL ) {
		return 1;
	}

	for ( i = 0 ; i < inLength ; i++ ) {
		fprintf( fp , "%d %g\n",i,inData[i]);
	}

	fclose(fp);

	return 0;
}
