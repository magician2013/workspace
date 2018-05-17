/**
 * @file mk_nbf.cc
 * @brief Generate null beamformer
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-1-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cctype>

#include "defs.hh"
#include "option.hh"
#include "debug.hh"
#include "mem_util.hh"
#include "nbf.hh"

//!Default output
#define MKNBF_DEFAULT_OUTPUT "NBF.dat"

/** Prototyping **/
void set_default_option( OPTION *out_opt );
void destroy_nbfinfo( NBFINFO *nbfinfo );
int parse_cmdline( OPTION *in_opt, int argc, char **argv );

void show_version( void );
void show_help( void );
void show_help( const char *in_errstr );
void show_options( OPTION *in_opt );
void show_defaults( void );

void output_filter( char *out_fname, C3_VECTOR &out_W );
/** Global valiables **/

/**********************************************************************/

/** Main function of fdica_batch
 */
int main( int argc, char **argv )
{
	OPTION *opt = new OPTION;
	opt->nbfinfo = new NBFINFO;

	set_default_option( opt );

	/** parsing command line arguments **/
	parse_cmdline( opt, argc, argv );

	/** show options **/
	if( !opt->isQuiet )
		show_options( opt );

	/** Generate NBF **/
	opt->nbfinfo->freq[0] = 0.0;
	opt->nbfinfo->freq[1] = opt->nbfinfo->Fs/2;

	C3_VECTOR W_NBF;
	if( !opt->isQuiet  ){
		fprintf( stderr, "Generate NBF\n");
	}
	makeNBF( W_NBF, opt->nbfinfo );
	output_filter( opt->out_fname, W_NBF );

	/** exit **/
	SAFE_DELETE( opt->nbfinfo );
	SAFE_DELETE( opt );
	return NO_ERROR;
}


/** Set default options
 *
 * @param out_opt target option structure
 */
void set_default_option( OPTION *out_opt )
{
	/** analysis option **/
	out_opt->nbfinfo->Fs      = DEFAULT_SAMPLE_FREQ;
	out_opt->nbfinfo->tapsize = DEFAULT_FFT_SIZE;
	
	/** I/O option **/
	out_opt->nbfinfo->nMICs   = DEFAULT_INPUT_CHANNEL;
	out_opt->nbfinfo->nUseMICs   = DEFAULT_INPUT_CHANNEL;

	/** NBF parameters **/
	out_opt->nbfinfo->deg.resize( out_opt->nbfinfo->nMICs );
	out_opt->nbfinfo->deg[0]  = DEFAULT_DEG_1;
	out_opt->nbfinfo->deg[1]  = DEFAULT_DEG_2;

	out_opt->nbfinfo->micpos.resize( out_opt->nbfinfo->nMICs );
	out_opt->nbfinfo->micpos[0] = DEFAULT_MICPOS_1;
	out_opt->nbfinfo->micpos[1] = DEFAULT_MICPOS_2;

	out_opt->nbfinfo->freq.resize( out_opt->nbfinfo->nMICs );
	out_opt->nbfinfo->freq[0] = 0.0;
	out_opt->nbfinfo->freq[1] = out_opt->nbfinfo->Fs/2;

	out_opt->nbfinfo->c = 340.0;
	out_opt->nbfinfo->isPinv = 0; /** default is not using PINV **/

	/** Another option **/
	out_opt->isQuiet = 0;
	strncpy( out_opt->out_fname, MKNBF_DEFAULT_OUTPUT, PATH_MAX );
}

/** Destroy NBFINFO structure
 *
 * @param out_opt target NBFINFO structure
 */
void destroy_nbfinfo( NBFINFO *nbfinfo )
{
}

/** Parse command line
 *  @param out_opt target option structure
 *  @param argc the number of input arguments
 *  @param argv array of the input arguments
 *
 *  @retval NO_ERROR(0) Parsing is successfully completed.
 */
int parse_cmdline( OPTION *out_opt, int argc, char **argv )
{
	if( argc > 1 ) {
		while( argv++, --argc > 0 ) {
			/** version **/
			if( !strncmp( argv[0], "-v", 3 ) || !strncmp( argv[0], "--version", 10 ) ) {
				show_version();
			}
			/** help **/
			else if ( !strncmp( argv[0], "-h", 3 ) || !strncmp( argv[0], "--help", 10 ) ) {
				show_help();
			}
			/** Sampling Frequnecy **/
			else if ( !strncmp( argv[0], "-fs", 4 ) || !strncmp( argv[0], "--sample-freq", 14 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: Sampling frequnecy");
				out_opt->nbfinfo->Fs = atoi(argv[0]);
			}
			/** FFT size **/
			else if ( !strncmp( argv[0], "-fft", 5 ) || !strncmp( argv[0], "--fftsize", 10 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: FFT size");
				out_opt->nbfinfo->tapsize = atoi(argv[0]);
			}
			/** silent mode **/
			else if ( !strncmp( argv[0], "-q", 4 ) || !strncmp( argv[0], "--quiet", 8 ) ) {
				out_opt->isQuiet = 1;
			}			
			/** using pseudo inverse matrix for constructing NBF **/
			else if ( !strncmp( argv[0], "-p", 3 ) || !strncmp( argv[0], "--pinv", 7 ) ) {
				out_opt->nbfinfo->isPinv = 1;
			}			
			/** null direction **/
			else if ( !strncmp( argv[0], "-d1", 4 ) || !strncmp( argv[0], "--null-direction1", 18 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: null direction.");
				out_opt->nbfinfo->deg[0] = atof(argv[0]);
			}			
			else if ( !strncmp( argv[0], "-d2", 4 ) || !strncmp( argv[0], "--null-direction2", 18 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: null direction.");
				out_opt->nbfinfo->deg[1] = atof(argv[0]);
			}			
			/** microphone position **/
			else if ( !strncmp( argv[0], "-m1", 4 ) || !strncmp( argv[0], "--micpos1", 10 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: null direction.");
				out_opt->nbfinfo->micpos[0] = atof(argv[0]);
			}			
			else if ( !strncmp( argv[0], "-m2", 4 ) || !strncmp( argv[0], "--micpos2", 10) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: null direction.");
				out_opt->nbfinfo->micpos[1] = atof(argv[0]);
			}			
			/** microphone position **/
			else if ( !strncmp( argv[0], "-o", 3 ) || !strncmp( argv[0], "--output", 9 ) ){
				if( ++argv, --argc == 0 ) show_help("Error in option: output filename.");
				strncpy( out_opt->out_fname, argv[0], PATH_MAX );
			}			
			/** show default parameters**/
			else if ( !strncmp( argv[0], "--default", 10 ) ) {
				show_defaults();
			}
			/** Unknown options**/
			else {
				PRINT_ERR("parse_cmdline", "Unknown option", argv[0] );
				fprintf(stderr,"\n");
				fprintf(stderr, "Try \'--help\' for run time options.\n");
				fprintf(stderr,"\n");
				exit(ERR_CMDLINE);
			}
		}
	} else {
		show_version();
	}
	return NO_ERROR;
}

/** Show version
 */
void show_version()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Generate null beamformer (NBF) for init matrix based on inversion of steering vector. \n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Copyright (c) 2007--2008 Shikano Lab., Nara Institute of Science and Technology\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Try \'--help\' for run time options.\n");
	fprintf(stderr, "\n");

	exit(NO_ERROR);
}

/** Show help of the program with error message and exit.
 *  @param in_errstr error message
 */
void show_help( const char *in_errstr )
{
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Generate null beamformer (NBF) for init matrix based on inversion of steering vector. \n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s\n",in_errstr );

	exit( ERR_CMDLINE );
}

/** Show help of the program and exit.
 */
void show_help()
{
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Generate null beamformer (NBF) for init matrix based on inversion of steering vector. \n");
	fprintf(stderr, "\n");

	fprintf(stderr, "  -fs,   --sample-freq:   Sampling freqency [%d]\n", DEFAULT_SAMPLE_FREQ);
	fprintf(stderr, "  -fft,  --fftsize:       FFT size. [%d]\n", DEFAULT_FFT_SIZE);
	fprintf(stderr, "  -o,    --output:        Output filename [%s]\n", MKNBF_DEFAULT_OUTPUT);
	fprintf(stderr, "  -p,    --pinv:          Using pseudo inverse for constructing null beamformer [no set]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -m1, -m2:               microphone positions for DOA based permutation solver [%g,%g]\n", DEFAULT_MICPOS_1, DEFAULT_MICPOS_2 );
	fprintf(stderr, "  -d1, -d2:               Null Directions [%g,%g]\n", DEFAULT_DEG_1, DEFAULT_DEG_2 );
	fprintf(stderr, "\n");
	
	fprintf(stderr, "  -q, --quiet:   silent mode\n");
	fprintf(stderr, "  -h, --help:    show this help.\n");
	fprintf(stderr, "  -v, --version: show version.\n");

	exit( NO_ERROR );
}

/** Show default parameters
 *
 *  @param in_opt specfied options
 */
void show_defaults( void  )
{
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Generate null beamformer (NBF) for init matrix based on inversion of steering vector. \n");
	fprintf(stderr, "\n");
	fprintf(stderr, "=== Default parameters ===\n");
	fprintf(stderr, "Sampling frequency = %d\n", DEFAULT_SAMPLE_FREQ );
	fprintf(stderr, "FFT size           = %d\n", DEFAULT_FFT_SIZE );

	fprintf(stderr, "Null directions    = %10.5f, %10.5f\n", DEFAULT_DEG_1, DEFAULT_DEG_2 );
	fprintf(stderr, "Micpos             = %10.5f, %10.5f\n", DEFAULT_MICPOS_1, DEFAULT_MICPOS_2 );
	exit( NO_ERROR );
}

/** Show specified options of the program
 *
 *  @param in_opt specfied options
 */
void show_options( OPTION *in_opt )
{
	fprintf( stderr, "=== OPTIONS ===\n");
	fprintf(stderr, "Output filename    = %s\n", in_opt->out_fname );
	fprintf(stderr, "Sampling frequency = %d\n", in_opt->nbfinfo->Fs );
	fprintf(stderr, "FFT size           = %d\n", in_opt->nbfinfo->tapsize );

	fprintf(stderr, "Null directions    = %10.5f, %10.5f\n", in_opt->nbfinfo->deg[0], in_opt->nbfinfo->deg[1] );
	fprintf(stderr, "Micpos             = %10.5f, %10.5f\n", in_opt->nbfinfo->micpos[0], in_opt->nbfinfo->micpos[1] );

	if( in_opt->nbfinfo->isPinv )
		fprintf(stderr, "Using pseudo inverse matrix\n");

	fprintf(stderr, "\n");
}


/** Output NBF
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
