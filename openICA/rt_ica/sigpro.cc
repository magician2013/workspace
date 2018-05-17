/**
 * @file sigpro.hh
 * @brief functions for signal processing
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */

/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/

#include "mathtool.hh"
#include "fft.hh"

/** Hanning window.
 * @param in_length length of window
 */
void hanning( D_VECTOR &hw, int in_length )
{
	if( in_length % 2 == 0 )
		in_length++;
	hw.resize(in_length);

	FLOAT A = 2.0 * PI / (FLOAT)( in_length - 1 );
	for( int i = 0 ; i < in_length ; i++ ){
		hw[ i ] = 0.50  - 0.50 * cos( A * (FLOAT)i );
	}
}

/** Hamming window.
 * @param in_length length of window
 */
void hamming( D_VECTOR &hw, int in_length )
{
	if( in_length % 2 == 0 )
		in_length++;
	hw.resize(in_length);

	FLOAT A = 2.0 * PI / (FLOAT)( in_length - 1 );
	for( int i = 0 ; i < in_length ; i++ ){
		hw[ i ] = 0.54  - 0.46 * cos( A * (FLOAT)i );
	}
}


/** swapping filter to be medium-position pulse in time domain.
 * @param filter target filter
 *
 */
void swap_filter( C_VECTOR &filter )
{
	COMPLEX tmp;
	int i,tapsize;

	tapsize = filter.size();

	for( i = 0 ; i < tapsize/2 ; i ++ ){
		tmp=filter[i];
		filter[i]=filter[tapsize/2+i];
		filter[tapsize/2+i]=tmp;
	}
}

/** swapping filter to be medium-position pulse in frequency domain.
 * @param filter target filter
 */
void swap_filter( C3_VECTOR &filter )
{
	C3_VECTOR tmp3d;
	int tapsize = filter.size();
	int m = filter[0].size();
	int n = filter[0][0].size();
	init_3d( tmp3d, tapsize, m,n );
	COMPLEX tmp;
	int i,j,k;

	fft3( -tapsize, filter, tmp3d, 0 );

	/** swap **/
	for( j = 0 ; j < m; j++ ){
		for( k = 0 ; k < n ; k++ ) {
			for( i = 0 ; i < tapsize/2 ; i ++ ){
				tmp=tmp3d[i][j][k];
				tmp3d[i][j][k]=tmp3d[tapsize/2+i][j][k];
				tmp3d[tapsize/2+i][j][k]=tmp;
			}
		}
	}
	fft3( tapsize, tmp3d, filter, 0 );
}

/** Apply Unmixing matrix by linear filtering.
 *
 * @param W Unmixing matrix, must be W[bins][output channel][input channel]
 * @param Xst separate target, must be Xst[input channel][frames][bins]
 * @param shift shift size
 * @param out output buffer of separate signal.
 *
 */
void linear_separate( C3_VECTOR &W, C3_VECTOR &Xst, int shift, D2_VECTOR &out )
{
	int ch = Xst.size();
	int frames = Xst[0].size();
	int tapsize = Xst[0][0].size();
	int i,j,k,l,frame;

	C_VECTOR f( tapsize * 2);
	C_VECTOR F( tapsize * 2);
	C_VECTOR x( tapsize * 2);
	C_VECTOR X( tapsize * 2);
	C_VECTOR O( tapsize * 2);
	C_VECTOR o( tapsize * 2);
	COMPLEX tmp;

	for( l = 0 ; l < ch; l++ )
		for( i = 0 ; i < (signed)out[0].size(); i++ )
			out[ l ][ i ] = 0.0;

	for( frame = 0 ; frame < frames; frame ++ ) {

		for( l = 0 ; l < ch ; l++ ) {
			for( k = 0 ; k < tapsize * 2 ; k++ )
				O [ k ] = c_zero();
			for( j = 0 ; j < ch; j++ ) {
				/** re-FFT of W **/
				for( k = 0 ; k < tapsize * 2; k++ )
					F[ k ] = f[ k ] = c_zero();
				for( k = 0 ; k < tapsize; k ++ )
					F[ k ] = W[ k ][ l ][ j ];
				fft( -tapsize, F, f );

				/** **/
				for( k = 0 ; k < tapsize/2 ; k++ ){
					tmp=f[k];
					f[k]=f[tapsize/2+k];
					f[tapsize/2+k]=tmp;
					}
				fft( tapsize*2, f, F );

				/** re-FFT of Xst **/
				for( k = 0 ; k < tapsize * 2; k++ )
					x[ k ] = c_zero();
				fft( -tapsize, Xst[j][frame], x );
				fft( tapsize*2, x, X );

				/** Convolution **/
				for( k = 0 ; k < tapsize * 2 ; k++ )
					O[ k ] = c_add( O[k], c_mul( F[k], X[k] ) );
			}
			/** reconstruct waveform and output **/
			fft( -tapsize*2, O, o );
			for( i = 0 ; i < tapsize * 2 ; i++ ) {
				out [l][ shift * frame + i ] += o[ i ].re;
			}
		}
	}
}

/** Generate highpass filter by windowing ideal highpass filter
 *
 * @param filter
 * @param filter_len length of filter
 * @param Fs Sampling frequency
 * @param cutFreq cut frequnecy
 *
 */
void makeHP( C_VECTOR &filter, int filter_len, FLOAT Fs, FLOAT cutFreq )
{
	int i;
	FLOAT freq;

	/** Windowing by hanning **/
	D_VECTOR hw;
	hanning( hw, filter_len );

	C_VECTOR out( filter_len );
	for( i = 0; i < filter_len/2 + 1 ; i++ ) {
		freq = Fs / (FLOAT)filter_len * (FLOAT) i;
		
		if( freq > cutFreq ) {
			filter[ i ].re = 1.0;
			filter[ i ].im = 0.0;
		} else {
			filter[ i ] = c_zero();
		}
	}
	for( i = filter_len/2+1 ; i < filter_len ; i++ ) {
		filter[i]=c_conj(filter[filter_len-i]);
	}

	fft( -filter_len, filter, out );
	
	swap_filter( out );

	for( i = 0 ; i < filter_len ; i++ ) {
		out[ i ] = c_mul_c( out[ i ], hw[ i ] );
	}

	fft( filter_len, out, filter );
}
