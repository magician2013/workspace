/**
 * @file fft.cc
 * @brief FFT by DIT
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 * FFT by DIT
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <cstdio>
#include <cstdlib>
#include "mathtool.hh"
#include "fft.hh"
#include "mem_util.hh"
#include "debug.hh"

/** Fast Fourier Transform by DIT
 *
 *  @param size FFT size. When size < 0, apply INVERSE FFT.
 *  @param *in_re Real part of input
 *  @param *in_im Imaginary part of input
 *  @param *out_re Real part of output
 *  @param *out_im 
 *
 *  @retval FFT_NO_ERROR(0) No error
 *  @retval FFT_ERR_NO_MEMORY(1) Not enough memory
 *  @retval FFT_ERR_NO_POWEROF2(2) Input length is not power of two
 */ 
int fft( int size , FLOAT *in_re , FLOAT *in_im , FLOAT *out_re, FLOAT *out_im )
{
  int i,j,k,n1,n2,N,M,p,S; // worker
  FLOAT t1,t2,*ct,*st;    // worker , cosine and sine table
  
  // check 'N'  
  N = abs(size);
  for( M = 0 , n1 = N ; n1 > 1 ; n1 /= 2 )
    {
      if( n1 % 2 == 1 ) {
        PRINT_ERR("fft","FFT_ERR_NO_POWEROF2","Size is not power of 2");
        return FFT_ERR_NO_POWEROF2;
      }
      M++;
    }
  // make cosine and sine table
  try
    {
      ct = st = NULL;
      ct = new FLOAT[N/2];
      st = new FLOAT[N/2];
    }
  catch( std::bad_alloc xa )
    {
      SAFE_DELETE_ARRAY(ct);
      SAFE_DELETE_ARRAY(st);
      return FFT_ERR_NO_MEMORY;
    }
  for( i = 0 ; i < N/2 ; i++ )
    {
      t1 = -2.0 * PI * i / N;
      ct[i] = cos( t1 );
      st[i] = sin( t1 );
    }
  
  // scrambler
  j = 0;
  n2 = N/2;
  out_re[0] = in_re[0];
  out_im[0] = in_im[0];
  out_re[N-1] = in_re[N-1];
  out_im[N-1] = in_im[N-1];
  for( i = 1 ; i < N - 1 ; i++ )
    {
      n1 = n2;
      while( j >= n1 )
        {
          j -= n1;
          n1 /= 2;
        }
      j += n1;
      if( i < j )
        {
          out_re[i] = in_re[j];
          out_im[i] = in_im[j];
          out_re[j] = in_re[i];
          out_im[j] = in_im[i];
        }
      else if( i == j )
        {
          out_re[i] = in_re[i];
          out_im[i] = in_im[i];
        }
    }

  // fft by dit
  n1 = 0; 
  n2 = 1;
  S = ( size >= 0 ? 1 : (-1) );
  for( i = 0 ; i < M ; i++ )
    {
      n1 =  n2;
      n2 += n2;    
      for( j = 0 ; j < n1 ; j++ )
        {
          for( k = j ; k < N ; k += n2 )
            {
							//Note: order of calc p to avoid overflow
              p = ( j * (N/2/n1) );
              t1 = ct[p]*out_re[k+n1] - S*st[p]*out_im[k+n1];
              t2 = S *st[p]*out_re[k+n1] + ct[p]*out_im[k+n1];
              out_re[k+n1] = out_re[k] - t1;
              out_im[k+n1] = out_im[k] - t2;
              out_re[k] += t1;
              out_im[k] += t2;
            }
        }
    }
  if( size < 0 )
    for(i=0;i<N;i++)
      {
        out_re[i]/=(FLOAT)N;
        out_im[i]/=(FLOAT)N;
      }
  SAFE_DELETE_ARRAY(ct);
  SAFE_DELETE_ARRAY(st);

  return 0;
}

/** Fast Fourier Transform by DIT
 *
 *  @param size 入力配列の大きさ．負の大きさにすると逆変換を行う．
 *  @param *in  入力
 *  @param *out 出力の虚部
 *
 *  @retval FFT_NO_ERROR(0) 成功
 *  @retval FFT_ERR_NO_MEMORY(1) メモリ不足
 *  @retval FFT_ERR_NO_POWEROF2(2) 入力が 2 の n 乗でない
 */ 
int fft( int size , std::vector<COMPLEX>&in, std::vector<COMPLEX>&out )
{
  int i,j,k,n1,n2,N,M,p,S; // worker
  FLOAT t1,t2,*ct,*st;    // worker , cosine and sine table
  
  // check 'N'  
  N = abs(size);
  for( M = 0 , n1 = N ; n1 > 1 ; n1 /= 2 )
    {
      if( n1 % 2 == 1 ){
        PRINT_ERR("fft","FFT_ERR_NO_POWEROF2","Size is not power of 2");
        return FFT_ERR_NO_POWEROF2;
      }
      M++;
    }
  // make cosine and sine table
  try
    {
      ct = st = NULL;
      ct = new FLOAT[N/2];
      st = new FLOAT[N/2];
    }
  catch( std::bad_alloc xa )
    {
      PRINT_ERR("fft","FFT_ERR_NO_MEMORY","Not enough memory");
      SAFE_DELETE_ARRAY(ct);
      SAFE_DELETE_ARRAY(st);
      return FFT_ERR_NO_MEMORY;
    }
  for( i = 0 ; i < N/2 ; i++ )
    {
      t1 = -2.0 * PI * i / N;
      ct[i] = cos( t1 );
      st[i] = sin( t1 );
    }
  
  // scrambler
  j = 0;
  n2 = N/2;
  out[0] = in[0];
  out[N-1] = in[N-1];
  for( i = 1 ; i < N - 1 ; i++ )
    {
      n1 = n2;
      while( j >= n1 )
        {
          j -= n1;
          n1 /= 2;
        }
      j += n1;
      if( i < j )
        {
          out[i] = in[j];
          out[j] = in[i];
        }
      else if( i == j )
        {
          out[i] = in[i];
        }
    }

  // fft by dit
  n1 = 0; 
  n2 = 1;
  S = ( size >= 0 ? 1 : (-1) );
  for( i = 0 ; i < M ; i++ )
    {
      n1 =  n2;
      n2 += n2;    
      for( j = 0 ; j < n1 ; j++ )
        {
          for( k = j ; k < N ; k += n2 )
            {
							//Note: order of calc p to avoid overflow
              p = ( j * N/2/n1 );
              t1 = ct[p]*out[k+n1].re - S*st[p]*out[k+n1].im;
              t2 = S *st[p]*out[k+n1].re + ct[p]*out[k+n1].im;
              out[k+n1].re = out[k].re - t1;
              out[k+n1].im = out[k].im - t2;
              out[k].re += t1;
              out[k].im += t2;
            }
        }
    }
  if( size < 0 )
    for(i=0;i<N;i++)
      {
        out[i].re/=(FLOAT)N;
        out[i].im/=(FLOAT)N;
      }
  SAFE_DELETE_ARRAY(ct);
  SAFE_DELETE_ARRAY(st);

  return 0;
}

/** Fast Fourier Transform by DIT
 *
 *  @param size 入力配列の大きさ．負の大きさにすると逆変換を行う．
 *  @param *in  入力
 *  @param *out 出力の虚部
 *  @param factor 何番目の要素が実際に FFT をかける配列か [ 0 -- 1 ]
 *
 *  @retval FFT_NO_ERROR(0) 成功
 *  @retval FFT_ERR_NO_MEMORY(1) メモリ不足
 *  @retval FFT_ERR_NO_POWEROF2(2) 入力が 2 の n 乗でない
 */ 
int fft2( int size , C2_VECTOR &in, C2_VECTOR &out , int factor )
{
  C_VECTOR tmp1,tmp2;
  int i,j,in_len;
  // 単純要素毎に場合わけしたほうが早い
  if( factor == 0 ) {
    in_len = in.size();
    tmp1.resize( in_len );
    tmp2.resize( in_len );
    for( j = 0 ; j < (int)in[0].size() ; j++ ) {
      // copy
      for(i=0;i<in_len;i++)
        tmp1[i] = in[i][j];
      // FFT
      fft( size , tmp1 , tmp2 );
      // copy
      for(i=0;i<in_len;i++)
        out[i][j] = tmp2[i];
    }
  } else {
    // 単純に FFT
    for( j = 0 ; j < (int)in.size() ; j++ ) {
      fft( size , in[j] , out[j] );
    }
  }
  return 0;
}
/** Fast Fourier Transform by DIT
 *
 *  @param size 入力配列の大きさ．負の大きさにすると逆変換を行う．
 *  @param *in  入力
 *  @param *out 出力の虚部
 *  @param factor 何番目の要素が実際に FFT をかける配列か [ 0 -- 2 ]
 *
 *  @retval FFT_NO_ERROR(0) 成功
 *  @retval FFT_ERR_NO_MEMORY(1) メモリ不足
 *  @retval FFT_ERR_NO_POWEROF2(2) 入力が 2 の n 乗でない
 */ 
int fft3( int size , C3_VECTOR &in, C3_VECTOR &out , int factor )
{
  C_VECTOR tmp1,tmp2;
  int i,j,k,in_len;
  // 単純要素毎に場合わけしたほうが早い
  if( factor == 0 ) {
    in_len = in.size();
    tmp1.resize( in_len );
    tmp2.resize( in_len );
    for( j = 0 ; j < (int)in[0].size() ; j++ ) {
      for( k = 0 ; k < (int)in[0][0].size() ; k++ ) {
        // copy
        for(i=0;i<in_len;i++)
          tmp1[i] = in[i][j][k];
        // FFT
        fft( size , tmp1 , tmp2 );
        // copy
        for(i=0;i<in_len;i++)
          out[i][j][k] = tmp2[i];
      }
    }
  } else if( factor == 1 ) {
    in_len = in[0].size();
    tmp1.resize( in_len );
    tmp2.resize( in_len );
    for( i = 0 ; i < (int)in[0].size() ; i++ ) {
      for( k = 0 ; k < (int)in[0][0].size() ; k++ ) {
        // copy
        for(j=0;j<in_len;j++)
          tmp1[j] = in[i][j][k];
        // FFT
        fft( size , tmp1 , tmp2 );
        // copy
        for(j=0;j<in_len;j++)
          out[i][j][k] = tmp2[j];
      }
    }
  } else {
    // 単純に FFT
    for( i = 0 ; i < (int)in.size() ; i++ ){
      for( j = 0 ; j < (int)in[0].size() ; j++ ) {
        fft( size , in[i][j] , out[i][j] );
      }
    }
  }
  return 0;
}
// end of fft.cc
