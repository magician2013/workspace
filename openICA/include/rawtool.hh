/**
 * @file rawtool.hh
 * @brief functions for treating raw files
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __RAWTOOL_H_
#define __RAWTOOL_H_

#include "config.hh"
#include "types.hh"

//! No error
#define RAWT_NOERROR 0
//! Cannot open file
#define RAWT_ERR_FILEOPEN 1

/** Reading <typename> raw file as FLOAT data with normalizing to [-1.0 1.0]
 * @param data read data
 * @param inFileName target filename
 * @param nBits quantized bits
 * @param isSwap set flag to 1, swap endian
 *
 * @retval RAWT_NOERROR(0) No error occurred
 * @retval RAWT_ERR_FILEOPEN(1) Cannot open specified file
 */
template<typename T>int rawread( D_VECTOR &data,char *inFileName , int isSwap )
{
	FILE *fp;
  long length,dlength;

	// file open
	if( ( fp = fopen( inFileName , "rb" ) ) == NULL )
		return RAWT_ERR_FILEOPEN;

	// get data length
	fseek(fp,1L,SEEK_END);
	length = ftell( fp );
  dlength = length/(sizeof(T));

  std::vector<T> rawdata(dlength);

	data.resize( dlength );

	// get data 
	rewind(fp); // move top of file
	fread( &rawdata[0] , sizeof(T) , dlength , fp );
  
  // normalize
  FLOAT c = (FLOAT)( ( 1 << (sizeof(T)*8) )/2  );
  if ( sizeof(T) == sizeof(FLOAT) )
    c = 1.0;
  for(int i=0;i<dlength;i++) {
    data[i] = (FLOAT)rawdata[i]/c;
  }
	fclose(fp);

	return RAWT_NOERROR;
}

/** Reading <typename> raw file only specified length as FLOAT data with normalizing to [-1.0 1.0] 
 * @param data read data
 * @param inFileName target filename
 * @param nBits quantized bits
 * @param in_length read only specified length
 * @param isSwap set flag to 1, swap endian
 *
 * @retval RAWT_NOERROR(0) No error occurred
 * @retval RAWT_ERR_FILEOPEN(1) Cannot open specified file
 */
template<typename T>int rawread( std::vector<FLOAT> &data,char *inFileName , int in_length, int isSwap )
{
	FILE *fp;
  long length,dlength;

	// file open
	if( ( fp = fopen( inFileName , "rb" ) ) == NULL )
		return RAWT_ERR_FILEOPEN;

	// get data length
	fseek(fp,1L,SEEK_END);
	length = ftell( fp );
  dlength = in_length;

  std::vector<T> rawdata(dlength);

	data.resize( dlength );

	// get data 
	rewind(fp); // move top of file
	fread( &rawdata[0] , sizeof(T) , dlength , fp );
  
  // normalize
  FLOAT c = (FLOAT)( ( 1 << (sizeof(T)*8) )/2  );
  if ( sizeof(T) == sizeof(FLOAT) )
    c = 1.0;
  for(int i=0;i<dlength;i++) {
    data[i] = (FLOAT)rawdata[i]/c;
  }
	fclose(fp);

	return RAWT_NOERROR;
}

/** write FLOAT raw data as <typename> raw file.
 * @param data target raw data
 * @param inFileName filename
 * @param isSwap when set to 1, endian swap is enabled.
 *
 * @retval RAWT_NOERROR No error
 * @retval RAWT_ERR_FILEOPEN Cannot open specified file
 */
template<typename T>int rawwrite( D_VECTOR &data, char *inFileName , int target_endian )
{
	FILE *fp;
  long length;
	T buffer;

	// file open
	if( ( fp = fopen( inFileName , "wb" ) ) == NULL )
		return RAWT_ERR_FILEOPEN;

	// get data length
	length = data.size();
  
  // normalize
  FLOAT c = (FLOAT)( ( 1 << (sizeof(T)*8) )/2  );
  if ( sizeof(T) == sizeof(FLOAT) )
    c = 1.0;
  for(int i=0;i<length;i++) {
		buffer = (T)(data[i]*c);
		fwrite(&buffer,sizeof(T),1,fp);
  }
	fclose(fp);
	return RAWT_NOERROR;
}

#endif /* end of rawtool.hh */
