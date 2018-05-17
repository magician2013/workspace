/**
 * @file fdica_batch.cc
 * @brief implementation of blind source separation (BSS) based on frequency-domain ICA (FDICA)
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "option.hh"
#include "defs.hh"
#include "mathtool.hh"
#include "sigpro.hh"
#include "fft.hh"
#include "infomax.hh"
#include "rawtool.hh"
#include "svdcmp.hh"

#include "vector_out.hh"

#define SOUND_VELOCITY   ((FLOAT)340.0)

void ProjectionBack_eachSource( C3_VECTOR &in_W, C3_VECTOR &out_WPB, int in_out_idx );
void SolvePerm_by_DOA( C3_VECTOR &W, D_VECTOR &micpos, int Fs );
void SolvePerm_by_Phase( C3_VECTOR &W );

void load_init_matrix( char *in_fname, C3_VECTOR &out_W );
void load_init_matrix_identity( C3_VECTOR &out_W );
void output_filter( char *out_fname, C3_VECTOR &out_W );


/************************************************************************/

/** BSS based on frequency-domain ICA (FDICA) (Now, 2 ch input, 2ch output only )
 *
 * @param param options
 *
 */
int FDICA ( OPTION *param )
{
	/** Workers **/
	int i,j,frame;

	/** load initial unmixing matrix **/
	C3_VECTOR W_ICA; /** unmixing matrices **/
	C3_VECTOR W_PB;  /** unmixing matrices **/
	init_3d( W_ICA, param->FFTsize, param->nOUTs, param->nMICs );
	init_3d( W_PB,  param->FFTsize, param->nOUTs, param->nMICs );
	if( !strncmp( param->init_matrix, "IDENTITY", 9 ) )
		load_init_matrix_identity( W_ICA );
	else
		load_init_matrix( param->init_matrix, W_ICA );

	/** load input signlas **/
	D2_VECTOR wave_obs  (param->nMICs,D_VECTOR(1));   /* observed signals wav_obs[input channel][samples] **/
	if( !param->isQuiet ) {
		fprintf(stderr,"Loading input signals ... \n");
	}
	for( i = 0 ; i < param->nMICs ; i++ ){
		if( rawread<short>( wave_obs[ i ], param->inputs[ i ], param->isSwap ) != RAWT_NOERROR ) {
			fprintf( stderr, "Cannot open input file %s\n", param->inputs[ i ] ); 
			return 1;
		}
	}
	
	/** Generate analysis window **/
	D_VECTOR hw;
	hanning( hw, param->wsize );


	/** short-time Fourier Transform **/
	int nFrame = (int)( (FLOAT)(wave_obs[0].size() - param->wsize ) / (FLOAT)param->shift ) + 1;

	C3_VECTOR Xst, Yst; /** Xst[input channel][frames][bin], Yst[input channel][frames][bin] **/
	init_3d( Xst, param->nMICs, nFrame, param->FFTsize );

	C_VECTOR xst(param->FFTsize);

	for( frame = 0 ; frame < nFrame ; frame++ ) {
		for(  j = 0; j < param->nMICs; j++ ) {
			for( i = 0; i < param->FFTsize ; i++ )
				xst[i] = c_zero();
			for( i = 0; i < param->wsize ; i++ ) {
				xst[i] = c_val( wave_obs[j][i + frame * param->shift] * hw[i], 0.0 );
			}
			fft( param->FFTsize, xst, Xst[j][frame]  );
		}
	}

	/** infomax (updating unmixing matrices) **/
	infomax( W_ICA, Xst, param->FFTsize, param->stepsize, param->iteration, param->isQuiet );

	/**
		 Permutation solver
		 Direction-of-arrival (DOA) based permutation solving method.

		 This approach is presented by S. Kurita, et al.
	 **/
	if( param->solve_perm_type == SOLVE_PERM_DOA) {
		fprintf( stderr, "DOA-based permutation solving\n");
		SolvePerm_by_Phase( W_ICA );
	}

	/**
		 Remove amplitute ambiguity among frequnecy bins (based on inverse matrix).
		 This approach is presented by N. Murata and S. Ikeda.
	**/
	if(!param->isQuiet)
		fprintf(stderr,"Output separated signals ...\n");


	D2_VECTOR wave_output( param->nMICs, D_VECTOR(nFrame * param->shift + param->FFTsize * 2) );
	for( i = 0 ; i < param->nOUTs; i++ ) {
		char fname[PATH_MAX];

		ProjectionBack_eachSource( W_ICA, W_PB, i );
		linear_separate( W_PB, Xst, param->shift, wave_output );

		for( j = 0 ; j < param->nMICs; j++ ) {
			sprintf( fname, "%s%d%d.raw", param->out_prefix, j + 1, i + 1 );
			if( rawwrite<short>( wave_output[j], fname, param->isSwap ) != RAWT_NOERROR ){
				fprintf( stderr, "Cannot write separated signal %s\n", fname );
			}
		}
	}
	
	/** Output optimized unmixing matrices **/
	if( param->out_matrix[0] != '\0' )
		output_filter( param->out_matrix, W_ICA );

	return NO_ERROR;
}

/** Removing ambiguity of amplitude based on inverse filter of unmixing matrix. (We call this method "Projection back");
 * 
 *  @param in_W   unmixing matrix
 *  @param out_PB output of projection back
 *  @param in_out_idx source number of desired source
 *
 */
void ProjectionBack_eachSource( C3_VECTOR &in_W, C3_VECTOR &out_WPB, int in_out_idx )
{
	int j,k,bin;
	
	int FFTsize = in_W.size();
	int nOUTs   = in_W[0].size();
	int nMICs   = in_W[0][0].size();

	C_VECTOR fftin( FFTsize );
	C_VECTOR fftout( FFTsize );

	C3_VECTOR W_INV;
	init_3d(W_INV, FFTsize, nOUTs, nMICs );

	/** Projection back **/
	for( bin = 1; bin < FFTsize/2+1 ; bin++ ){
		pinv( in_W[bin], W_INV[bin], 1.0e-6 );
		if( in_out_idx == 0 ) {
			out_WPB[bin][0][0] = c_mul( W_INV[bin][0][0], in_W[bin][0][0]);
			out_WPB[bin][0][1] = c_mul( W_INV[bin][0][0], in_W[bin][0][1]);
			out_WPB[bin][1][0] = c_mul( W_INV[bin][1][0], in_W[bin][0][0]);
			out_WPB[bin][1][1] = c_mul( W_INV[bin][1][0], in_W[bin][0][1]);
		} else {
			out_WPB[bin][0][0] = c_mul( W_INV[bin][0][1], in_W[bin][1][0]);
			out_WPB[bin][0][1] = c_mul( W_INV[bin][0][1], in_W[bin][1][1]);
			out_WPB[bin][1][0] = c_mul( W_INV[bin][1][1], in_W[bin][1][0]);
			out_WPB[bin][1][1] = c_mul( W_INV[bin][1][1], in_W[bin][1][1]);
		}
	}

	/** Make fullband filter **/
	for( bin = FFTsize/2 + 1; bin < FFTsize ; bin++ ){
		for( j = 0 ; j < nOUTs ; j++ )
			for( k = 0 ; k < nMICs; k++ )
				out_WPB[ bin ][ j ][ k ] = c_conj( out_WPB[ FFTsize - bin ][ j ][ k ] );
	}
	/** output filter **/
	for( j = 0 ; j < nOUTs; j++ ) {
		for( k = 0 ; k < nMICs; k++ ) {
			for( bin = 0 ; bin < FFTsize; bin++ ){
				fftin[ bin ] = out_WPB[bin][j][k];
			}

#if 0 /** debug output: filters in time domain **/
			char strfname[256];
			fft( -FFTsize, fftin, fftout );

			swap_filter( fftout );

			sprintf( strfname, "filter%d%d.txt", j, k );
			FILE *fp = fopen( strfname, "w");
			for( bin = 0 ; bin < FFTsize; bin++ )
				fprintf(fp, "%d %f\n", bin, fftout[bin].re );
			fclose(fp);
#endif
		}
	}
}


/** Sovling Permutation based on phase difference (2x2 only)
 *
 * @param W target umixing matrices W[FFTsize][output channels][input channels]
 *
 */
void SolvePerm_by_Phase( C3_VECTOR &W )
{
	int bins = W.size();
	int nOUTs = W[0].size();

	D_VECTOR phase_diff( nOUTs );
	COMPLEX c_tmp;
	for ( int bin = 1 ; bin < bins/2+1 ; bin++ ){
		for( int j = 0; j < nOUTs ; j++ )
			phase_diff[j] = c_angle( c_div( W[bin][j][0], W[bin][j][1] ) );
		if( phase_diff[ 0 ] > phase_diff[ 1 ] ){
			c_tmp = W[bin][0][0];
			W[bin][0][0] = W[bin][1][0];
			W[bin][1][0] = c_tmp;

			c_tmp = W[bin][0][1];
			W[bin][0][1] = W[bin][1][1];
			W[bin][1][1] = c_tmp;
		}
	}
}

/** Load initial unmixing matrix.
 *
 * @param in_fname filename of initial unmixing matrix
 * @param out_W this [out_W] must be, out_W[bin][output channels][input channels] 
 */
void load_init_matrix( char *in_fname, C3_VECTOR &out_W )
{
	FILE *fp = fopen( in_fname, "rb");
	unsigned int j,k,bin;
	FLOAT tmp[2];

	if( fp == NULL ) {
		fprintf(stderr,"Cannot load initial matrix file: %s\n", in_fname );
		exit( 1 );
	}

	for( j = 0 ; j <  out_W[0].size(); j++ ) {
		for( k = 0 ; k < out_W[0][0].size() ; k++) {
			for( bin = 0 ; bin < out_W.size() ; bin++ ) {
				if( fread( &tmp, sizeof(FLOAT), 2, fp )  == 2 ){
					out_W[bin][j][k].re = tmp[0];
					out_W[bin][j][k].im = tmp[1];
				} else {
					fprintf( stderr, "Error in initial matrix: it may be FFT size is different from the initial matrix\n");
					exit(1);
				}
			}
		}
	}
	fread(tmp, sizeof(FLOAT), 1, fp );
	if( !feof(fp) ) {
		fprintf( stderr, "Error in initial matrix: it may FFT size is different from the initial matrix\n");
		exit(1);
	}
	fclose( fp );

}
/** Load initial unmixing matrix as Identity matrix
 *
 * @param out_W this [out_W] must be, out_W[bin][output channels][input channels] 
 */
void load_init_matrix_identity( C3_VECTOR &out_W )
{
	unsigned int j,k,bin;

	for( j = 0 ; j < out_W[0].size(); j++ ){
		for( k = 0 ; k < out_W[0][0].size(); k++ ){
			for (bin = 0 ; bin < out_W.size(); bin++ ){
				if( j == k ) out_W[bin][j][k] = c_one();
				else out_W[bin][j][k] = c_zero();
			} 
		}
	}
}
/** Output optimized filter
 *
 * @param out_fname output filename
 * @param out_W this [out_W] must be, out_W[bin][output channels][input channels] 
 */
void output_filter( char *out_fname, C3_VECTOR &out_W )
{
	FILE *fp = fopen( out_fname, "wb");
	unsigned int j,k,bin;
	FLOAT tmp;

	if( fp == NULL ) {
		fprintf(stderr,"Cannot write filter to %s\n", out_fname );
		exit( 1 );
	}

	for( j = 0 ; j <  out_W[0].size(); j++ ) {
		for( k = 0 ; k < out_W[0][0].size() ; k++) {
			for( bin = 0 ; bin < out_W.size() ; bin++ ) {
				tmp = out_W[bin][j][k].re;
				fwrite( &tmp, sizeof(FLOAT), 1, fp );
				tmp = out_W[bin][j][k].im;
				fwrite( &tmp, sizeof(FLOAT), 1, fp );
			}
		}
	}
}
