/**

 * @file nbf.cc
 * @brief Implementation of NBF
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <vector>
#include <iostream>
#include "nbf.hh"
#include "fft.hh"
#include "mathtool.hh"
#include "debug.hh"
#include "svdcmp.hh"

/** Generate null beamformer (NBF)
 * @param out_filter Output filter 
 * @param nbfinfo Setups for NBF
 *
 * @retval 
 */
int makeNBF( C3_VECTOR &out_filter , NBFINFO *nbfinfo )
{
	int nMICs  = nbfinfo->nUseMICs;                    // using microphones
	int nPairs = nbfinfo->nMICs / (nbfinfo->nUseMICs); // the number of bands 
	int tapsize = nbfinfo->tapsize;                    // tapsize
	D_VECTOR d(nMICs);                                 // microphones positions
	int fbin_low,fbin_high;                            // max freq. bin and min. freq. bin
	C3_VECTOR A;                                       // steering vector
	int i,j,k,pair;                                    // worker
	FLOAT td,dtd=(FLOAT)tapsize/(FLOAT)nbfinfo->Fs*0.5; // default pulse position

	DBGSTR("makeNBF","Start");

	init_3d(A,tapsize,nMICs,nMICs); // allocate memory for steering vector
	init_3d(out_filter,tapsize,nMICs,nMICs);

	for( pair = 0 ; pair < nPairs ; pair++ ) {
	  // setup bandwidth
		fbin_low  = (int)(nbfinfo->freq[pair*2]*(FLOAT)tapsize/(FLOAT)nbfinfo->Fs);
		fbin_high = (int)(nbfinfo->freq[pair*2+1]*(FLOAT)tapsize/(FLOAT)nbfinfo->Fs);

		// determine microphone poistion
		DBGSTR("makeNBF","Setup Microphone position");
		for(j=0;j<nMICs;j++){
			d[j] = nbfinfo->micpos[pair*nMICs+j];
		}
		// generate steering vector
		DBGSTR("makeNBF","Make steering vector");
		for(i=fbin_low;i<=fbin_high;i++){
			for(k=0;k<nMICs;k++){
				for(j=0;j<nMICs;j++){
					td = d[j]*sin( nbfinfo->deg[k]*PI/180.0 )/nbfinfo->c+dtd;
					A[i][j][k]=c_exp( 2.0*PI*(FLOAT)i/(FLOAT)tapsize*(FLOAT)nbfinfo->Fs*td);
				}
			}
		}
		// generate null beamformer based on inversion of steering vector matrix.
		DBGSTR("makeNBF","Make inverse filter");
		
		if( nbfinfo->isPinv ) {
			for(i=fbin_low;i<=fbin_high;i++){
				pinv( A[i], out_filter[i], 1.0e-12 );
			}
		} else {
			for(i=fbin_low;i<=fbin_high;i++){
				c_inv( A[i], out_filter[i] );
			}
		}
	}
	// generate full-band beamformer by conjugation
	DBGSTR("makeNBF","Conjugate to make full band filter");

	for( i = tapsize/2+1 ; i < tapsize ; i++ )
		for(j=0;j<nMICs;j++)
			for(k=0;k<nMICs;k++)
				out_filter[i][j][k]=c_conj(out_filter[tapsize-i][j][k]);

	// Make direct current (DC) component zero
	for(j=0;j<nMICs;j++)
		for(k=0;k<nMICs;k++){
			out_filter[0][j][k]=c_zero();
		}
	
	return NBF_NOERROR;
}
