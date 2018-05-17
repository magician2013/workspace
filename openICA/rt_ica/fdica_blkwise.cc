/**
 * @file fdica_blkwise.cc
 * @brief implementation of FDICA by block wise batch algorithm
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * @date $Date: $
 * 
 * $Id: $
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <pthread.h>

#include "config.hh"
#include "defs.hh"
#include "debug.hh"

#include "mathtool.hh"
#include "sigpro.hh"
#include "svdcmp.hh"
#include "option.hh"
#include "oss.hh"
#include "mathtool.hh"
#include "fft.hh"
#include "inkey.hh"

#include "fdica_blkwise.hh"

#define SOUND_VELOCITY 340.0

int filtering( S_VECTOR &readbuf, D_VECTOR &writebuf, D_VECTOR window, OPTION *param );
void *ICA_Thread( void *arg );
void *ICA_Thread_offline( void *arg );
void ProjectionBack_eachSource( C3_VECTOR &in_W, C3_VECTOR &out_WPB, int in_out_idx );

void load_init_matrix( char *in_fname, C3_VECTOR &out_W );
void load_init_matrix_identity( C3_VECTOR &out_W );

/** ring buffer **/
int read_ringbuffer( S_VECTOR &ringbuf, S_VECTOR &outbuf, int size, int sp, int ringsize );
int read_ringbuffer( D_VECTOR &ringbuf, D_VECTOR &outbuf, int size, int sp, int ringsize );
int read_ringbuffer( D_VECTOR &ringbuf, C_VECTOR &outbuf, int size, int sp, int ringsize );
int overlap_ringbuffer( D_VECTOR &ringbuf, D_VECTOR &inbuf, int size, int sp, int ringsize );
int overlap_ringbuffer( D_VECTOR &ringbuf, C_VECTOR &inbuf, int size, int sp, int ringsize );

#define RING_INCLEMENT( p, rsize, ring_size ) ( p = ( p + rsize ) & ( ring_size - 1 ) )


/** Global valiables **/
/** Highpass filter **/
C_VECTOR g_HPF;

/** FFT workspace **/
static C_VECTOR g_cv_fftin, g_cv_fftout;

/** mutex **/
pthread_mutex_t g_state_mutex;
pthread_mutex_t g_filter_mutex;

/** flag **/
static int g_Mode    = DEMO_MODE_DEFAULT;
static int g_ICA_out = 0;

/** Separation filter **/
static C3_VECTOR W_ICA;
static C3_VECTOR W_INIT;
static C3_VECTOR W_INIT_PB;
static C3_VECTOR W_TMP;

/** Time-Frequnecy domain Ring Buffer for ICA **/
static C3_VECTOR g_ring_Xst;
static I_VECTOR g_ring_flags;
static int g_Xst_sp = 0;
static int g_Xst_rp = 0;

/** initialize globale variable 
 *
 * @param FFTsize FFT size
 */
int init_globals( int FFTsize)
{
	g_cv_fftin.resize( FFTsize );
	g_cv_fftout.resize( FFTsize );

	return 0;
}

/** Frequency-domain ICA by block wise batch algorithm
 *
 * @param device Audio device object
 * @param param parameter structure
 *
 * @retval
 */
int fdica_blkbatch( COSS *device, OPTION *param )
{
	/** Worker **/
	int rsize, input_code;

	/** block counter **/
	int blk = 0;

	/** ICA thread_ID **/
	pthread_t thread_ICA;

	/** ringbuffer pointers **/
	int rp = 0; /** reading point **/
	int wp = 0; /** writing point **/

	/** Ring Buffer **/
	S_VECTOR readbuf( param->dev.ring_size );
	D_VECTOR writebuf( param->dev.ring_size );

	/** Short buffer **/
	S_VECTOR st_readbuf( param->dev.frame_size );
	D_VECTOR st_writebuf( param->dev.frame_size * 2 );
	S_VECTOR st_writebuf_short( param->dev.read_size );

	/** output file descriptor **/
	int fd_out;

	DBGSTR("fdica_blkbatch","Start FDICA-blkwise");
	
	/** Set default mode and source **/
	g_Mode = param->defaultMode;
	g_ICA_out = param->defaultSource;
#ifdef __DEBUG
	fprintf(stderr, "Mode = %s, Source = %d\n", (g_Mode==DEMO_MODE_THROUGH?"THORUGH":"ICA") ,g_ICA_out);
#endif

	/** Initialize globale variables **/
	DBGSTR("fdica_blkbatch","Initialize global variables");
	init_globals( param->FFTsize );

	g_HPF.resize( param->FFTsize );
	makeHP( g_HPF, param->FFTsize, (FLOAT)param->Fs, 100.0 );

	/** Prepare window function **/
	DBGSTR("fdica_blkbatch","Prepare hanning window");
	D_VECTOR hw, hw_tmp;
	hanning( hw_tmp, param->wsize+1 );
	hw.resize( param->FFTsize );

	/** centering**/
	for( int i = 0 ; i < param->FFTsize; i++ )
		hw[ i ] = 0.0;
	for( int i = 0 ; i < param->wsize; i++ )
		hw[ i ] = hw_tmp[ i ];
		//hw[ param->FFTsize/4+i ] = hw_tmp[ i ];

	/** load initial matrix **/
	DBGSTR("fdica_blkbatch:inital matrix",param->init_matrix);
	init_3d( W_INIT,  param->FFTsize, param->nOUTs, param->nMICs );
	if( !strncmp( param->init_matrix, "IDENTITY", 9 ) ) {
		load_init_matrix_identity( W_INIT );
	} else {
		load_init_matrix( param->init_matrix, W_INIT );
	}

	/** allocate memory to temporal filter**/
	DBGSTR("fdica_blkbatch","Allocate memories for separation matrices");
	init_3d( W_ICA,   param->FFTsize, param->nOUTs, param->nMICs );
	init_3d( W_TMP,   param->FFTsize, param->nOUTs, param->nMICs );

	ProjectionBack_eachSource( W_INIT, W_ICA, g_ICA_out );
	
	/** Initialize Time-Frequency Domain Ring Buffer **/
	DBGSTR("fdica_blkbatch","Allocate memories for ICA's ring buffer");
	init_3d( g_ring_Xst, param->nMICs, param->ICA_ring_frames, param->FFTsize );
	g_ring_flags.resize( param->ICA_ring_frames );

	/** Create ICA Thread **/
	param->ICA_msg = ICA_THREAD_WAIT;
	if( param->rawinput[0] == '\0' ) {
		DBGSTR("fdica_blkbatch","Create thread for on-line");
		if( pthread_create( &thread_ICA, NULL, ICA_Thread, (void*)param ) ) {
			fprintf(stderr, "Cannot create ICA thread\n");
			return ERR_IN_CREATE_THREAD;
		}
	/** create thread for rawinput mode**/
	} else {
		DBGSTR("fdica_blkbatch","Create thread for simulating on-line algorithm.");
		if( pthread_create( &thread_ICA, NULL, ICA_Thread_offline, (void*)param ) ) {
			fprintf(stderr, "Cannot create ICA thread\n");
			return ERR_IN_CREATE_THREAD;
		}
	}

	/** Try reading three readsizes ( READSIZE * 3 ) from device **/
	/** read from DSP **/
	DBGSTR("fdica_blkbatch","Reading audio device three times for filling buffer");
	for( int i = 0 ; i < 3 ; i++ ) {
		if( ( rsize = device->read( &readbuf.at(0)+rp, param->dev.read_size_byte) ) != param->dev.read_size_byte ) {
			perror("read()");
			return ERR_IN_DEVICE_READ;
		}
		RING_INCLEMENT( rp, param->dev.read_size, param->dev.ring_size );
	}
	/** Select output file descriptor **/
	if( param->rawoutput[0] == '\0' ){
		DBGSTR("fdica_blkbatch:out device",param->device);
		fd_out = device->get_descriptor();
	} else {
		DBGSTR("fdica_blkbatch:out device",param->rawoutput);
		fd_out = open( param->rawoutput, O_WRONLY|O_TRUNC|O_CREAT, S_IRGRP|S_IROTH|S_IRUSR|S_IWUSR );
		if( fd_out <= 0 ){
			fprintf( stderr, "Cannot open output file %s\n", param->rawoutput );
			perror("open : ");
			return (-1);
		}
	}
	/** create mutex **/
	DBGSTR("fdica_blkbatch","Prepare mutex");
	pthread_mutex_init( &g_state_mutex, NULL );
	pthread_mutex_init( &g_filter_mutex, NULL );
	/** keyboard initialization **/
	DBGSTR("fdica_blkbatch","Initialize keyboard");
	init_keyboard();
	DBGSTR("fdica_blkbatch","Entering main loop");
	/******************** Main loop ********************/
	blk = 0;
	while( 1 ) {
		if( ( rsize = device->read( &readbuf.at(0)+rp, param->dev.read_size_byte) ) != param->dev.read_size_byte ) {
			perror("read()");
			break; /** exit main loop**/
		}
		RING_INCLEMENT( rp, param->dev.read_size, param->dev.ring_size );
		blk ++;
		/** Filtering **/
		read_ringbuffer( readbuf,  st_readbuf,  param->dev.frame_size, wp, param->dev.ring_size );

		filtering( st_readbuf, st_writebuf, hw, param );
	
		overlap_ringbuffer( writebuf, st_writebuf, param->dev.frame_size*2, wp, param->dev.ring_size );
		
		/** write to DSP **/
		/** transform FLOAT data into short data **/
		for( int k = 0 ; k < param->dev.read_size; k ++ )
			st_writebuf_short[ k ] = (short) ( writebuf[ wp + k ] );

		/** write to device**/
		if( (rsize=write( fd_out, &st_writebuf_short.at(0), param->dev.read_size_byte )) < 0 ) {
			perror("write()");
			break; /** exit loop **/
		}

		/** flush written buffer **/
		for( int k = 0 ; k < param->dev.read_size ; k++ ) {
			writebuf[ wp + k ] = 0;
		}
		RING_INCLEMENT( wp, param->dev.read_size, param->dev.ring_size );

		/** Control ICA buffer (Send New Buffer) **/
		if( blk >= param->ICA_frames ) {
			pthread_mutex_lock( &g_state_mutex );
			param->ICA_msg = ICA_THREAD_SEND_NEW_BUFFER;
			pthread_mutex_unlock( &g_state_mutex );
			/** rawinput mode **/
			if( param->rawinput[0] != '\0' ){
				DBGSTR("fdica_blkwise:offline","waiting end of optimize");
				/** wait end of optimizing by ICA **/
				while( param->ICA_msg != ICA_THREAD_END_OPTIMIZE ) {
					usleep(1000);
				}
				pthread_mutex_lock( &g_state_mutex );
				param->ICA_msg = ICA_THREAD_WAIT;
				pthread_mutex_unlock( &g_state_mutex );
			}
			blk = 0;
		}
		/** Keyboard control **/
		if( kbhit() ) {
			input_code = inkey();
			if( input_code == 'q' || input_code == 'Q' )
				break; /** exit main loop **/
			else if( input_code == '0' )
				g_Mode = DEMO_MODE_THROUGH; /** set mode as THORUGH mode **/
			else if( input_code == '1' )
				g_Mode = DEMO_MODE_ICA; /** set mode as ICA mode **/
			else if( input_code == 's'  || input_code == 'S' )
				g_ICA_out = ( g_ICA_out + 1 ) % param->nMICs; /** swithing ICA's output **/
			else if( input_code == 'r' || input_code == 'R' ){
				pthread_mutex_lock( &g_state_mutex );
				param->ICA_msg = ICA_THREAD_RESET_MATRIX;
				pthread_mutex_unlock( &g_state_mutex );
			}
			if(!param->isQuiet)
				fprintf(stderr, "Mode = %s, Source = %d\n", (g_Mode==DEMO_MODE_THROUGH?"THORUGH":"ICA") ,g_ICA_out);
		}
	}
	/** stop ICA_Thread **/
	fprintf(stderr, "Stop ICA thread ..");
	do {
		pthread_mutex_lock( &g_state_mutex );
		param->ICA_msg = ICA_THREAD_STOP;
		pthread_mutex_unlock( &g_state_mutex );
		usleep( 10000 );
		fprintf(stderr, ".");
	} while( param->ICA_msg != ICA_THREAD_EXIT );
	fprintf(stderr, " OK\n");

	if( pthread_join( thread_ICA, NULL ) ) {
		if( !param->isQuiet )
			fprintf(stderr, "pthread join failed ... but may not be error.\n");
	}

	/** drop mutex **/
	DBGSTR("fdica_blkwise","Finalize keyboard");
	close_keyboard();
	pthread_mutex_destroy( &g_state_mutex );
	pthread_mutex_destroy( &g_filter_mutex );

	/** close output handle **/
	if( param->rawoutput[0] != '\0' ){
		close(fd_out);
	}
	return NO_ERROR;
}

/** filtering routine **/
inline int filtering( S_VECTOR &readbuf, D_VECTOR &writebuf, D_VECTOR window, OPTION *param )
{
	int j,k,l;

	C2_VECTOR X( param->nMICs, C_VECTOR(param->FFTsize) );
	C2_VECTOR Y( param->nOUTs, C_VECTOR(param->FFTsize) );

	C_VECTOR f( param->FFTsize * 2);
	C_VECTOR F( param->FFTsize * 2);
	C_VECTOR xn( param->FFTsize * 2);
	C_VECTOR XN( param->FFTsize * 2);
	C_VECTOR O( param->FFTsize * 2);
	C2_VECTOR o( param->nMICs, C_VECTOR(param->FFTsize * 2) );
	COMPLEX tmp;

	for( j = 0 ; j < param->nMICs; j++ ) {
		for( k = param->FFTsize ; k < param->FFTsize * 2; k++ )
			writebuf[ k * param->nMICs + j ] = 0.0;
		for( k = 0 ; k < param->FFTsize; k++ ){
			writebuf[ k * param->nMICs + j ] = window[k]*((FLOAT)readbuf[ ( k * param->nMICs ) + j ] / 32767.0);
		}
	}

	/** N-channel FFT **/
	for( j = 0 ; j < param->nMICs; j++ ) {
		for( k = 0 ; k < param->FFTsize; k++ ){
			g_cv_fftin[ k ].re = writebuf[ k * param->nMICs + j ];
			g_cv_fftin[ k ].im = 0.0;
		}
		fft( param->FFTsize, g_cv_fftin, X[j] );
		/** copy to ring buffer **/
		for( k = 0 ; k < param->FFTsize; k++ ){
			//			X[j][k] = c_mul( X[j][k], g_HPF[k] ); // HPF
			g_ring_Xst[j][g_Xst_sp][k] = X[j][k];
		}
	}
	/** increment of read pointer of ICA ring buffer **/
	g_Xst_sp = ( g_Xst_sp + 1 ) % param->ICA_ring_frames;


	/** ICA or NBF filtering **/
	if( g_Mode == DEMO_MODE_ICA ) {
		/** copy **/
		pthread_mutex_lock( &g_filter_mutex );
		W_TMP = W_ICA;
		pthread_mutex_unlock( &g_filter_mutex );
		
		/** filtering **/
		for( l = 0 ; l < param->nOUTs ; l++ ) {
			for( k = 0 ; k < param->FFTsize * 2 ; k++ )
				O [ k ] = c_zero();
			for( j = 0 ; j < param->nMICs ; j++ ) {
				for( k = param->FFTsize ; k < param->FFTsize * 2; k++ )
					F[ k ] = f[ k ] = c_zero();
				for( k = 0; k < param->FFTsize ; k++ ) {
					F[ k ] = W_TMP[ k ][ l ][ j ];
				}
				fft( -param->FFTsize, F, f );

				for( k = 0 ; k < param->FFTsize / 2; k++ ){
					tmp = f [ k ];
					f[ k ] = f [ param->FFTsize/2 + k  ];
					f[ param->FFTsize/2 + k  ] = tmp;
				}

				fft( param->FFTsize * 2, f, F );

				for( k = 0 ; k < param->FFTsize * 2;  k++ )
					xn[ k ] = XN[k] = c_zero();
				fft( -param->FFTsize,     X[ j ], xn );
				fft(  param->FFTsize * 2, xn,     XN );
				
				for( k = 0 ; k < param->FFTsize + 1; k++ ) 
					O[ k ] = c_add( O[ k ], c_mul( F[k], XN[k] ) );
			}
			/** Make fullband output **/
			for( k = param->FFTsize + 1 ; k < param->FFTsize * 2; k++ )
				O[ k ] = c_conj( O[ param->FFTsize * 2- k ] );
			fft( -param->FFTsize * 2, O, o[l] );
		}
	}
	/** output to writebuffer **/
	if( g_Mode == DEMO_MODE_ICA ) {
		for( k = 0 ; k < param->FFTsize * 2; k++ ){
			writebuf[ k * param->nMICs ]     = o[g_ICA_out][k].re;
			writebuf[ k * param->nMICs + 1 ] = o[g_ICA_out][k].re;
		}
	}
 else {
		for( k = 0 ; k < param->FFTsize * 2; k++ ){
			writebuf[ k * param->nMICs + g_ICA_out ] = writebuf[ k * param->nMICs + ((g_ICA_out+1)%param->nMICs)];
		}
	}
	return 0;
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

/** Solving Permutation based on estimated Power (Now, 2 input 2 output only )
 * 
 *  @param W target unmixing matrices W[FFTsize][output channels][input channels]
 *  @param Yst for separating buffer
 *  @param param option structure
 *  @param bp current reading pointer of ring buffer of ICA
 *
 */
void SolvePerm_by_Power( C3_VECTOR &W, C3_VECTOR &Yst, OPTION *param, int bp  )
{
	int bin, frame, j, i;
	int bins = W.size();
	int outs = W[0].size();
	COMPLEX c_tmp;
	D_VECTOR tmp( outs );

	fprintf( stderr, "Power based method\n");
	/** Projection back each source **/
	ProjectionBack_eachSource( W, W_TMP, 0 );

	/* try separate */
	for( bin = 1; bin < bins/2+1; bin++ ){
		for( j = 0 ; j < param->nOUTs ; j++ )
			tmp[ j ] = 0.0;

		for( frame = 0; frame < param->ICA_frames; frame++ ){
			for( j = 0 ; j < param->nOUTs ; j ++ ) {
				Yst[j][frame][bin] = c_zero();
				for( i = 0 ; i < param->nMICs ; i++ ) {
					Yst[j][frame][bin] = c_add( Yst[j][frame][bin], c_mul( W_TMP[bin][j][i], g_ring_Xst[i][frame+bp][bin] ) );
				}
			}
			for( j = 0 ; j < param->nOUTs ; j++ )
				tmp[ j ] += c_power( Yst[ j ][ frame ][ bin ] );
		}
		if( 2.0 * tmp[ 0 ] < tmp[ 1 ] ){
			c_tmp = W[bin][0][0];
			W[bin][0][0] = W[bin][1][0];
			W[bin][1][0] = c_tmp;

			c_tmp = W[bin][0][1];
			W[bin][0][1] = W[bin][1][1];
			W[bin][1][1] = c_tmp;
		}
	}
	
	/** **/
	
}

/** ICA Thread function
 *  @param arg a void pointer to OPTION structure
 */
void *ICA_Thread( void *arg )
{
	OPTION *param = (OPTION*)arg;
	int i,j,k,bin,frame;
	int bp = 0;
	int it = 0;
	int blk = 0; /** block counter **/
	int nan_flag = 0;

	/** For ICA optimization **/
	C2_VECTOR corr(param->nMICs, C_VECTOR(param->nMICs) );
	C2_VECTOR delta(param->nMICs, C_VECTOR(param->nMICs) );
	C3_VECTOR W_PB;
	C3_VECTOR Yst;
	C3_VECTOR W;
	D_VECTOR norm(param->nMICs);

	/** DOA **/
	D2_VECTOR DOA_deg( param->nMICs, D_VECTOR(param->FFTsize/2+1) ); /** DOA in degrees **/
	I2_VECTOR DOA_isv( param->nMICs, I_VECTOR(param->FFTsize/2+1) ); /** valid DOA flag **/
	D_VECTOR  DOA_mean( param->nMICs );                     /** estimate doa **/

	/** power of Block **/
	D_VECTOR block_pow( param->nMICs );

	/** allocate buffer **/
	init_3d( Yst, param->nMICs, param->ICA_frames, param->FFTsize );
	init_3d( W,    param->FFTsize, param->nOUTs, param->nMICs );
	init_3d( W_PB, param->FFTsize, param->nOUTs, param->nMICs );

	/** set initial unmixing matrix **/
	W = W_INIT;

	fprintf(stderr, "Start ICA Thread\n");
	while( 1 ){
		/** receive new buffer **/
		if( param->ICA_msg == ICA_THREAD_SEND_NEW_BUFFER ) {
			DBGSTR("ThreadICA","Receive new buffer for ICA");
			bp = g_Xst_rp;
			/** increment pointer of ring buffer for ICA optimization **/
			g_Xst_rp = ( g_Xst_rp + param->ICA_frames ) % param->ICA_ring_frames;

			/** Solving Permuatation **/
			if( param->solve_perm_type == SOLVE_PERM_DOA )
				SolvePerm_by_Phase( W );
			else if( param->solve_perm_type == SOLVE_PERM_POWER )
				SolvePerm_by_Power( W, Yst, param, bp );

			/** Remove ambiguity of amplitude **/
			ProjectionBack_eachSource( W, W_PB, g_ICA_out );
			
			/** update filter **/
			pthread_mutex_lock( &g_filter_mutex );
			W_ICA = W_PB;
			pthread_mutex_unlock( &g_filter_mutex );


			/** **/
			blk++;
			if(!param->isQuiet)
				fprintf( stderr, "Update matrix [%3d iterations]\n", it);

			/** Set buffering flag zero **/
			it = 0;

			/** restart optimizing **/
			pthread_mutex_lock( &g_state_mutex );
			param->ICA_msg = ICA_THREAD_OPTIMIZE;
			pthread_mutex_unlock( &g_state_mutex );
		}
		/** Exit thread **/
		if( param->ICA_msg == ICA_THREAD_STOP ) {
			break; /** exit while loop **/
		}
		/** Reset Matrix **/
		if( param->ICA_msg == ICA_THREAD_RESET_MATRIX ) {
			if( !param->isQuiet )
				fprintf( stderr, "Reset unmixing matrix by user\n");
			W = W_INIT;
			ProjectionBack_eachSource( W_INIT, W_PB, g_ICA_out );
			pthread_mutex_lock( &g_filter_mutex );
			W_ICA = W_PB;
			pthread_mutex_unlock( &g_filter_mutex );
			pthread_mutex_lock( &g_state_mutex );
			param->ICA_msg = ICA_THREAD_WAIT;
			pthread_mutex_unlock( &g_state_mutex );
			blk = 0;
		}
		/** Wait buffer **/
		if( param->ICA_msg == ICA_THREAD_WAIT )
			usleep(10);
		/** Optimization **/
		if( param->ICA_msg == ICA_THREAD_OPTIMIZE ) {
			/**
				 In the first iteration, calc power of block, and stop update when power is low. 
			**/
			if( it == 0 ) {
				/** calc power **/
				for( j = 0 ; j < param->nMICs; j++ ) {
					block_pow[ j ] = 0.0;
					for( bin = 1 ; bin < param->FFTsize/2+1; bin++ ) {
						for( frame = 0 ; frame < param->ICA_frames; frame++ ){
							block_pow[ j ] += c_power( g_ring_Xst[ j ][ frame + bp ][ bin ] );
						}
					}
				}
				if( !param->isQuiet )
					fprintf(stderr, "Block power = %g\n", block_pow[0] );
				if( block_pow[ 0 ] < param->ICA_power_th && block_pow[ 1 ] < param->ICA_power_th ) {
					blk+=3;
					pthread_mutex_lock( &g_state_mutex );
					param->ICA_msg = ICA_THREAD_WAIT;
					pthread_mutex_unlock( &g_state_mutex );

					if( !param->isQuiet )
						fprintf( stderr, "Low power block\n");

					continue; /** exit ICA_THREAD_OPTIMIZE **/
				}
				if( blk >= param->ICA_reset_blk ) {
					if( !param->isQuiet )
						fprintf( stderr, "Reset unmixing matrix\n");
					W = W_INIT;
					blk = 0;
				}
				/*
					Firstly, we normalize the unmixing matrix by L^2 norm of separated signal.
					This is equal to align the sensivity to stepsize parameter between whole frequency bins.
				*/
				for( bin = 1 ; bin < param->FFTsize/2+1; bin++ ){
					/* try separate */
					for( frame = 0; frame < param->ICA_frames; frame++ ){
						for( j = 0 ; j < param->nOUTs ; j ++ ) {
							Yst[j][frame][bin] = c_zero();
							for( i = 0 ; i < param->nMICs ; i++ ) {
								Yst[j][frame][bin] = c_add( Yst[j][frame][bin], c_mul( W[bin][j][i], g_ring_Xst[i][frame+bp][bin] ) );
							}
						}
					}
					for( j = 0; j < param->nOUTs; j++ ){
						norm[ j ] = 0.0;
						for( frame = 0 ; frame < param->ICA_frames; frame++ ){
							if( c_power( Yst[j][frame][bin] ) > norm[j] )
								norm[j] = c_power(Yst[j][frame][bin]);
							//norm[j] += c_power( Yst[j][frame][bin] );
						}
						//norm[ j ] /= (FLOAT)param->ICA_frames;
						norm[ j ] = sqrt( norm[ j ] );
						if( norm[j] > 0 ){
							for( i = 0 ; i < param->nMICs ; i++ )
								W[bin][j][i] = c_mul_c( W[bin][j][i], 1.0/norm[j] );
						}
					}
				}
			}
			/** Infomax **/
			for( bin = 1 ; bin < param->FFTsize/2+1; bin++ ){
				nan_flag = 0;
				/* try separate */
				for( frame = 0; frame < param->ICA_frames; frame++ ){
					for( j = 0 ; j < param->nOUTs ; j ++ ) {
						Yst[j][frame][bin] = c_zero();
						for( i = 0 ; i < param->nMICs ; i++ ) {
							Yst[j][frame][bin] = c_add( Yst[j][frame][bin], c_mul( W[bin][j][i], g_ring_Xst[i][frame+bp][bin] ) );
						}
					}
				}
				/*** infomax ***/
				/*
					calc E[ I - phi(y)*y' ] 
					
					[y'] means Hermitian transpose of vector [y].
					[phi] is the nonlinear function. 
					In this code, using sign function for real and imaginary part, respectively.
					
				*/
				for(  j = 0 ; j < param->nMICs ; j++ ) {
					for(  k = 0 ; k < param->nMICs ; k++ ){
						corr[j][k] = c_zero();
						for(  frame = 0 ; frame < param->ICA_frames ; frame++ ) {
							corr[j][k] = c_add( corr[j][k], c_mul( c_sign(Yst[j][frame][bin]), c_conj(Yst[k][frame][bin]) ) );
						}
						corr[j][k] = c_mul_c( corr[j][k], 1.0/(FLOAT)param->ICA_frames );
						if ( j == k )
							corr[j][k] = c_sub( c_val(1.0,0.0), corr[j][k] );
						else
							corr[j][k] = c_neg( corr[j][k] );
					}
				}
				/*** determine update W ***/
				for( j = 0 ; j < param->nMICs ; j++ ) {
					for(  k = 0 ; k < param->nMICs ; k++ ) {
						delta[j][k] = c_zero();
						for( i = 0 ; i < param->nOUTs ; i++ ) {
							delta[j][k] = c_add( delta[j][k], c_mul(corr[j][i], W[bin][i][k] ) );
						}
						delta[j][k] = c_mul_c( delta[j][k], param->stepsize);
					}
				}

				/** update **/
				for(  j = 0 ; j < param->nOUTs ; j++ ) {
					for( k = 0 ; k < param->nMICs ; k++ ) {
						W[bin][j][k] = c_add( W[bin][j][k], delta[j][k] );
						if( c_isnan(W[bin][j][k]) || c_isinf(W[bin][j][k]) )
							nan_flag = 1;
					}
				}
			}
			it ++;
			if( nan_flag ){
				fprintf(stderr,"\r%4d - NaN or Inf is occurred in infomax. Reset matrix. \n", bin );
				ProjectionBack_eachSource( W_INIT, W_PB, g_ICA_out );
				pthread_mutex_lock( &g_filter_mutex );
				W = W_INIT;
				pthread_mutex_unlock( &g_filter_mutex );
				continue;
			}
		}/** end of ICA optimization**/
	}
	/** set exit status **/
	pthread_mutex_lock( &g_state_mutex );
	param->ICA_msg = ICA_THREAD_EXIT;
	pthread_mutex_unlock( &g_state_mutex );
	/** exit thread **/
	pthread_exit( NULL );
}

/** ICA Thread function for simulating real-time block-wise batch
 *  @param arg a void pointer to OPTION structure
 */
void *ICA_Thread_offline( void *arg )
{
	DBGSTR("ICA_Thread_offline","Offlinse thread start: Simulating real-time algorithm\n");
	OPTION *param = (OPTION*)arg;
	int i,j,k,bin,frame;
	int bp = 0;
	int it = 0;
	int blk = 0; /** block counter **/
	int nan_flag = 0;

	/** For ICA optimization **/
	C2_VECTOR corr(param->nMICs, C_VECTOR(param->nMICs) );
	C2_VECTOR delta(param->nMICs, C_VECTOR(param->nMICs) );
	C3_VECTOR W_PB;
	C3_VECTOR Yst;
	C3_VECTOR W;
	D_VECTOR norm(param->nMICs);

	/** DOA **/
	D2_VECTOR DOA_deg( param->nMICs, D_VECTOR(param->FFTsize/2+1) ); /** DOA in degrees **/
	I2_VECTOR DOA_isv( param->nMICs, I_VECTOR(param->FFTsize/2+1) ); /** valid DOA flag **/
	D_VECTOR  DOA_mean( param->nMICs );                     /** estimate doa **/

	/** power of Block **/
	D_VECTOR block_pow( param->nMICs );

	/** allocate buffer **/
	init_3d( Yst, param->nMICs, param->ICA_frames, param->FFTsize );
	init_3d( W,    param->FFTsize, param->nOUTs, param->nMICs );
	init_3d( W_PB, param->FFTsize, param->nOUTs, param->nMICs );

	/** set initial unmixing matrix **/
	W = W_INIT;

	fprintf(stderr, "Start ICA Thread\n");
	while( 1 ){
		//		printf("%d\n",param->ICA_msg);
		/** receive new buffer **/
		if( param->ICA_msg == ICA_THREAD_SEND_NEW_BUFFER ) {
			DBGSTR("ThreadICA_offline","Receive new buffer for ICA");
			bp = g_Xst_rp;
			/** increment pointer of ring buffer for ICA optimization **/
			g_Xst_rp = ( g_Xst_rp + param->ICA_frames ) % param->ICA_ring_frames;

			/** Solving Permuatation **/
			if( param->solve_perm_type == SOLVE_PERM_DOA )
				SolvePerm_by_Phase( W );
			else if( param->solve_perm_type == SOLVE_PERM_POWER )
				SolvePerm_by_Power( W, Yst, param, bp );

			/** Remove ambiguity of amplitude **/
			ProjectionBack_eachSource( W, W_PB, g_ICA_out );
			
			/** update filter **/
			pthread_mutex_lock( &g_filter_mutex );
			W_ICA = W_PB;
			pthread_mutex_unlock( &g_filter_mutex );


			/** **/
			blk++;
			if(!param->isQuiet)
				fprintf( stderr, "Update matrix [%3d iterations]\n", it);

			/** restart optimizing **/
			pthread_mutex_lock( &g_state_mutex );
			param->ICA_msg = ICA_THREAD_OPTIMIZE;
			pthread_mutex_unlock( &g_state_mutex );
		}
		/** Exit thread **/
		if( param->ICA_msg == ICA_THREAD_STOP ) {
			break; /** exit while loop **/
		}
		/** Reset Matrix **/
		if( param->ICA_msg == ICA_THREAD_RESET_MATRIX ) {
			if( !param->isQuiet )
				fprintf( stderr, "Reset unmixing matrix by user\n");
			W = W_INIT;
			ProjectionBack_eachSource( W_INIT, W_PB, g_ICA_out );
			pthread_mutex_lock( &g_filter_mutex );
			W_ICA = W_PB;
			pthread_mutex_unlock( &g_filter_mutex );
			pthread_mutex_lock( &g_state_mutex );
			param->ICA_msg = ICA_THREAD_WAIT;
			pthread_mutex_unlock( &g_state_mutex );
			blk = 0;
		}
		/** Wait buffer **/
		if( param->ICA_msg == ICA_THREAD_WAIT )
			usleep(10);
		/** Wait buffer (End optimize) **/
		if( param->ICA_msg == ICA_THREAD_END_OPTIMIZE )
			usleep(10);
		/** Optimization **/
		if( param->ICA_msg == ICA_THREAD_OPTIMIZE ) {
			DBGSTR("ThreadICA_offline","Optimizing start");
			/**
				 In the simulating algorithm, param->ICA_iteration loops are performed for optimization.
			 **/
			for( it = 0; it < param->ICA_iteration; it++ ){
			/**
				 In the first iteration, calc power of block, and stop update when power is low. 
			**/
			if( it == 0 ) {
				/** calc power **/
				for( j = 0 ; j < param->nMICs; j++ ) {
					block_pow[ j ] = 0.0;
					for( bin = 1 ; bin < param->FFTsize/2+1; bin++ ) {
						for( frame = 0 ; frame < param->ICA_frames; frame++ ){
							block_pow[ j ] += c_power( g_ring_Xst[ j ][ frame + bp ][ bin ] );
						}
					}
				}
				if( !param->isQuiet )
					fprintf(stderr, "Block power = %g\n", block_pow[0] );
				if( block_pow[ 0 ] < param->ICA_power_th && block_pow[ 1 ] < param->ICA_power_th ) {
					blk+=3;
					pthread_mutex_lock( &g_state_mutex );
					param->ICA_msg = ICA_THREAD_WAIT;
					pthread_mutex_unlock( &g_state_mutex );

					if( !param->isQuiet )
						fprintf( stderr, "Low power block\n");

					continue; /** exit ICA_THREAD_OPTIMIZE **/
				}
				if( blk >= param->ICA_reset_blk ) {
					if( !param->isQuiet )
						fprintf( stderr, "Reset unmixing matrix\n");
					W = W_INIT;
					blk = 0;
				}
				/*
					Firstly, we normalize the unmixing matrix by L^2 norm of separated signal.
					This is equal to align the sensivity to stepsize parameter between whole frequency bins.
				*/
				for( bin = 1 ; bin < param->FFTsize/2+1; bin++ ){
					/* try separate */
					for( frame = 0; frame < param->ICA_frames; frame++ ){
						for( j = 0 ; j < param->nOUTs ; j ++ ) {
							Yst[j][frame][bin] = c_zero();
							for( i = 0 ; i < param->nMICs ; i++ ) {
								Yst[j][frame][bin] = c_add( Yst[j][frame][bin], c_mul( W[bin][j][i], g_ring_Xst[i][frame+bp][bin] ) );
							}
						}
					}
					for( j = 0; j < param->nOUTs; j++ ){
						norm[ j ] = 0.0;
						for( frame = 0 ; frame < param->ICA_frames; frame++ ){
							if( c_power( Yst[j][frame][bin] ) > norm[j] )
								norm[j] = c_power(Yst[j][frame][bin]);
							//norm[j] += c_power( Yst[j][frame][bin] );
						}
						//norm[ j ] /= (FLOAT)param->ICA_frames;
						norm[ j ] = sqrt( norm[ j ] );
						if( norm[j] > 0 ){
							for( i = 0 ; i < param->nMICs ; i++ )
								W[bin][j][i] = c_mul_c( W[bin][j][i], 1.0/norm[j] );
						}
					}
				}
			}
			/** Infomax **/
			for( bin = 1 ; bin < param->FFTsize/2+1; bin++ ){
				nan_flag = 0;
				/* try separate */
				for( frame = 0; frame < param->ICA_frames; frame++ ){
					for( j = 0 ; j < param->nOUTs ; j ++ ) {
						Yst[j][frame][bin] = c_zero();
						for( i = 0 ; i < param->nMICs ; i++ ) {
							Yst[j][frame][bin] = c_add( Yst[j][frame][bin], c_mul( W[bin][j][i], g_ring_Xst[i][frame+bp][bin] ) );
						}
					}
				}
				/*** infomax ***/
				/*
					calc E[ I - phi(y)*y' ] 
					
					[y'] means Hermitian transpose of vector [y].
					[phi] is the nonlinear function. 
					In this code, using sign function for real and imaginary part, respectively.
					
				*/
				for(  j = 0 ; j < param->nMICs ; j++ ) {
					for(  k = 0 ; k < param->nMICs ; k++ ){
						corr[j][k] = c_zero();
						for(  frame = 0 ; frame < param->ICA_frames ; frame++ ) {
							corr[j][k] = c_add( corr[j][k], c_mul( c_sign(Yst[j][frame][bin]), c_conj(Yst[k][frame][bin]) ) );
						}
						corr[j][k] = c_mul_c( corr[j][k], 1.0/(FLOAT)param->ICA_frames );
						if ( j == k )
							corr[j][k] = c_sub( c_val(1.0,0.0), corr[j][k] );
						else
							corr[j][k] = c_neg( corr[j][k] );
					}
				}
				/*** determine update W ***/
				for( j = 0 ; j < param->nMICs ; j++ ) {
					for(  k = 0 ; k < param->nMICs ; k++ ) {
						delta[j][k] = c_zero();
						for( i = 0 ; i < param->nOUTs ; i++ ) {
							delta[j][k] = c_add( delta[j][k], c_mul(corr[j][i], W[bin][i][k] ) );
						}
						delta[j][k] = c_mul_c( delta[j][k], param->stepsize);
					}
				}

				/** update **/
				for(  j = 0 ; j < param->nOUTs ; j++ ) {
					for( k = 0 ; k < param->nMICs ; k++ ) {
						W[bin][j][k] = c_add( W[bin][j][k], delta[j][k] );
						if( c_isnan(W[bin][j][k]) || c_isinf(W[bin][j][k]) )
							nan_flag = 1;
					}
				}
			}
			if( nan_flag ){
				fprintf(stderr,"\r%4d - NaN or Inf is occurred in infomax. Reset matrix. \n", bin );
				ProjectionBack_eachSource( W_INIT, W_PB, g_ICA_out );
				pthread_mutex_lock( &g_filter_mutex );
				W = W_INIT;
				pthread_mutex_unlock( &g_filter_mutex );
				continue;
			}
			} /** end of iteration **/
			/** send end of ICA optimization **/
			pthread_mutex_lock( &g_state_mutex );
			param->ICA_msg = ICA_THREAD_END_OPTIMIZE;
			pthread_mutex_unlock( &g_state_mutex );

		}/** end of ICA optimization**/
	}
	/** set exit status **/
	pthread_mutex_lock( &g_state_mutex );
	param->ICA_msg = ICA_THREAD_EXIT;
	pthread_mutex_unlock( &g_state_mutex );
	/** exit thread **/
	pthread_exit( NULL );
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
		pinv( in_W[bin], W_INV[bin], 1.0e-12 );
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
#if 0 /** debug output: filters in time domain **/
	/** output filter **/
	for( j = 0 ; j < nOUTs; j++ ) {
		for( k = 0 ; k < nMICs; k++ ) {
			for( bin = 0 ; bin < FFTsize; bin++ ){
				fftin[ bin ] = out_WPB[bin][j][k];
			}

			char strfname[256];
			fft( -FFTsize, fftin, fftout );

			swap_filter( fftout );

			sprintf( strfname, "filter%d%d.txt", j, k );
			FILE *fp = fopen( strfname, "w");
			for( bin = 0 ; bin < FFTsize; bin++ )
				fprintf(fp, "%d %f\n", bin, fftout[bin].re );
			fclose(fp);
		}
	}
#endif
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
					fprintf( stderr, "Error in initial matrix: it may be FFT size is different from the initial matrix(too short)\n");
					exit(1);
				}
			}
		}
	}
	fread(tmp, sizeof(FLOAT), 1, fp );
	if( !feof(fp) ) {
		fprintf( stderr, "Error in initial matrix: it may FFT size is different from the initial matrix(too long)\n");
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

/** Read from ringbuffer as short data 
 * @param ringbuf ringbuffer
 * @param outbuf output buffer
 * @param size read size
 * @param sp read point
 * @param ringsize ring buffer size
 *
 * @retval always returns 0
 */
int read_ringbuffer( S_VECTOR &ringbuf, S_VECTOR &outbuf, int size, int sp, int ringsize )
{
	int k;
	int t;
	for( k = 0 ; k < size ; k++ ){
		t = ( sp + k ) & ( ringsize - 1 );
		outbuf[ k ] = ringbuf[ t ];
	}
	return 0;
}

/** Read from ringbuffer as FLOAT data 
 * @param ringbuf ringbuffer
 * @param outbuf output buffer
 * @param size read size
 * @param sp read point
 * @param ringsize ring buffer size
 *
 * @retval always returns 0
 */
int read_ringbuffer( D_VECTOR &ringbuf, D_VECTOR &outbuf, int size, int sp, int ringsize )
{
	int k;
	int t;
	for( k = 0 ; k < size ; k++ ){
		t = ( sp + k ) & ( ringsize - 1 );
		outbuf[ k ] = ringbuf[ t ];
	}
	return 0;
}

/** Read from ringbuffer as complex data( for real-part only )
 * @param ringbuf ringbuffer
 * @param outbuf output buffer
 * @param size read size
 * @param sp read point
 * @param ringsize ring buffer size
 *
 * @retval always returns 0
 */
int read_ringbuffer( D_VECTOR &ringbuf, C_VECTOR &outbuf, int size, int sp, int ringsize )
{
	int k;
	int t;
	for( k = 0 ; k < size ; k++ ){
		t = ( sp + k ) & ( ringsize - 1 );
		outbuf[ k ].re = ringbuf[ t ];
		outbuf[ k ].im = 0.0;
	}
	return 0;
}

/** overlap writing to ring buffer with normalzing from [-1,1] data to [-32767, 32767 ]
 * @param ringbuf ringbuffer
 * @param inbuf writing buffer
 * @param size read size
 * @param sp write point
 * @param ringsize ring buffer size
 * 
 * @retval always returns 0
 */
int overlap_ringbuffer( D_VECTOR &ringbuf, D_VECTOR &inbuf, int size, int sp, int ringsize )
{
	int k;
	int t;
	for( k = 0 ; k < size ; k++ ){
		t = ( sp + k ) & ( ringsize - 1 );
		ringbuf[ t ] += inbuf[ k ] * 32767.0;
	}
	return 0;
}

/** write ring buffer with overlap add tequnique (only real-part ) 
 * 
 */
int overlap_ringbuffer( D_VECTOR &ringbuf, C_VECTOR &inbuf, int size, int sp, int ringsize )
{
	int k;
	int t;
	for( k = 0 ; k < size ; k++ ){
		t = ( sp + k ) & ( ringsize - 1 );
		ringbuf[ t ] += inbuf[ k ].re;
	}
	return 0;
}

