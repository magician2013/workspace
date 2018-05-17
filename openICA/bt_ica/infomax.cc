/**
 * @file infomax.cc
 * @brief implementation of infomax for ICA
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <cstdio>
#include "infomax.hh"

/** Update complex-valued unmixing matrix by infomax (nonlinear function is tanh).
 *  
 * @param W This is unmixing matrix. [W] must be W[bin][input channel][output channel]
 * @param Xst Time-frequnecy-domain observed signal. This is must be Xst[channel][frame][bin].
 * @param tapsize filter length of ICA, which is equal to FFT size
 * @param stepsize stepsize parameter for update unmixing matrix.
 * @param iterations the number of iterations for updating unmixing matrix
 * @param isQuiet if set to be 1, work silent.
 */
int infomax( C3_VECTOR &W, C3_VECTOR &Xst, int tapsize, FLOAT stepsize, int iterations, int isQuiet )
{
	int i,j,k,frame,iteration,bin;
	int nFrame = Xst[0].size(); /** the number of frames **/
	int nMICs  = Xst.size();    /** the number of input channels **/
	int nOUTs  = nMICs;         /** Now, the number of outputs is fixed to input channel **/
	
	int nan_flag = 0;

	/** temporal memories**/
	C2_VECTOR corr (nMICs, C_VECTOR(nMICs) );
	C2_VECTOR delta(nMICs, C_VECTOR(nMICs) );
	D_VECTOR  norm (nMICs);

	/** separated signal for update unmixing matrix **/
	C3_VECTOR Yst;
	init_3d( Yst, nMICs, nFrame, tapsize );

	/** ----- INFOMAX ----- **/
	/** If bin == 0, this means direct current (DC) component. So we do not update in this band. **/
	for( bin = 1 ; bin < tapsize/2+1; bin++ ){
		/*
			Firstly, we normalize the unmixing matrix by L^2 norm of separated signal.
			This is equal to align the sensivity to stepsize parameter between whole frequency bins.
		*/
		/* try separate */
		for( frame = 0; frame < nFrame; frame++ ){
			for( j = 0 ; j < nOUTs ; j ++ ) {
				Yst[j][frame][bin] = c_zero();
				for( i = 0 ; i < nMICs ; i++ ) {
					Yst[j][frame][bin] = c_add( Yst[j][frame][bin], c_mul( W[bin][j][i], Xst[i][frame][bin] ) );
				}
			}
		}
		for( j = 0; j < nOUTs; j++ ){
			norm[ j ] = 0.0;
			for( frame = 0 ; frame < nFrame; frame++ ){
				norm[ j ] += c_power( Yst[j][frame][bin] );
			}
			norm[ j ] /= (FLOAT)nFrame;
			norm[ j ] = sqrt( norm[ j ] );
			if( norm[j] > 0 ){
				for( i = 0 ; i < nMICs ; i++ )
					W[bin][j][i] = c_mul_c( W[bin][j][i], 1.0/norm[j] );
			}
		}
		/*** iterative update ***/
		for( iteration = 0; iteration < iterations  ; iteration++ ){
			/** zero clear for NaN or INF flag**/
			nan_flag = 0;

			/* try separate */
			for( frame = 0; frame < nFrame; frame++ ){
				for( j = 0 ; j < nOUTs ; j ++ ) {
					Yst[j][frame][bin] = c_zero();
					for( i = 0 ; i < nMICs ; i++ ) {
						Yst[j][frame][bin] = c_add( Yst[j][frame][bin], c_mul( W[bin][j][i], Xst[i][frame][bin] ) );
					}
				}
			}
			/*
				calc E[ I - phi(y)*y' ] 
				
				[y'] means Hermitian transpose of vector [y].
				[phi] is the nonlinear function. 
				In this code, using sign function for real and imaginary part, respectively.

			 */
			for(  j = 0 ; j < nMICs ; j++ ) {
				for(  k = 0 ; k < nMICs ; k++ ){
					corr[j][k] = c_zero();
					for(  i = 0 ; i < nFrame ; i++ ) {
						corr[j][k] = c_add( corr[j][k], c_mul( c_sign(Yst[j][i][bin]), c_conj(Yst[k][i][bin]) ) );
						//corr[j][k] = c_add( corr[j][k], c_mul(c_val(tanh(100.0*Yst[j][i][bin].re),tanh(100.0*Yst[j][i][bin].im)), Yst[k][i][bin]));
					}
					corr[j][k] = c_mul_c( corr[j][k], 1.0/(FLOAT)nFrame );
					if ( j == k )
						corr[j][k] = c_sub( c_val(1.0,0.0), corr[j][k] );
					else
						corr[j][k] = c_neg( corr[j][k] );
				}
			}
			/*** determine update W ***/
			for( j = 0 ; j < nMICs ; j++ ) {
				for(  k = 0 ; k < nMICs ; k++ ) {
					delta[j][k] = c_zero();
					for( i = 0 ; i < nMICs ; i++ ) {
						delta[j][k] = c_add( delta[j][k], c_mul(corr[j][i], W[bin][i][k] ) );
					}
					 delta[j][k] = c_mul_c( delta[j][k], stepsize);
				}
			}
			/** update **/
			for(  j = 0 ; j < nMICs ; j++ ) {
				for( k = 0 ; k < nMICs ; k++ ) {
					W[bin][j][k] = c_add( W[bin][j][k], delta[j][k] );
					if( c_isnan(W[bin][j][k]) || c_isinf(W[bin][j][k]) ) 
						nan_flag = 1;
				}
			}
			if( nan_flag ){
				fprintf(stderr,"\r%4d - NaN or Inf is occurred in infomax \n", bin );
				for(  j = 0 ; j < nOUTs ; j++ ) {
					for( k = 0 ; k < nMICs ; k++ ) {
						if( j == k ) W[bin][j][k] = c_one();
						else W[bin][j][k] = c_zero();
					}
				}
				break; /* end of updating in this bin */
			}
		}
		if( !isQuiet )
			fprintf(stderr,"\r[Infomax] bin = %5d/%5d",bin,tapsize);
	}
	if( !isQuiet )
		fprintf(stderr, "\rInfomax is finished ...                      \n");

	return 0;
}
