/**
 * @file bt_ica.cc
 * @brief implementation of blind source separation (BSS) based on frequency-domain ICA (FDICA)
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
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
#include "fdica_batch.hh"

/** Prototyping **/
void set_default_option( OPTION *out_opt );

int parse_cmdline( OPTION *in_opt, int argc, char **argv );

void show_version( void );
void show_help( void );
void show_help( const char *in_errstr );
void show_options( OPTION *in_opt );
void show_defaults( void );

/** Global valiables **/

/**********************************************************************/

/** Main function of fdica_batch
 */
int main( int argc, char **argv )
{
	OPTION *opt = new OPTION;
	set_default_option( opt );

	/** parsing command line arguments **/
	parse_cmdline( opt, argc, argv );

	/** show options **/
	if( !opt->isQuiet )
		show_options( opt );

	/** run FDICA **/
	if( !opt->isQuiet  ){
		fprintf( stderr, "=== Start FDICA ===\n");
	}
	FDICA( opt );

	/** exit **/
	SAFE_DELETE( opt );
	return NO_ERROR;
}


/** Set default options
 *
 * @param out_opt target option structure
 */
void set_default_option( OPTION *out_opt )
{
	/** output options**/
	strncpy( out_opt->out_prefix, DEFAULT_OUTPUT_PREFIX, PATH_MAX );
	out_opt->out_matrix[0] = '\0';

	/** analysis option **/
	out_opt->Fs      = DEFAULT_SAMPLE_FREQ;
	out_opt->FFTsize = DEFAULT_FFT_SIZE;
	out_opt->wsize   = DEFAULT_WINDOW_SIZE;
	out_opt->wtype   = DEFAULT_WINDOW_TYPE;
	out_opt->shift   = DEFAULT_SHIFT_SIZE;

	/** ICA option **/
	out_opt->stepsize  = DEFAULT_ICA_STEPSIZE;
	out_opt->iteration = DEFAULT_ICA_ITERATION;
	out_opt->solve_perm_type = DEFAULT_PERM_TYPE;
	strncpy( out_opt->init_matrix, DEFAULT_ICA_INIT_MATRIX, PATH_MAX );

	/** I/O option **/
	out_opt->nMICs     = DEFAULT_INPUT_CHANNEL;
	out_opt->nOUTs     = DEFAULT_OUTPUT_CHANNEL;

	/** Another option **/
	out_opt->isQuiet = 0;
	out_opt->isSwap  = DEFAULT_IS_SWAP;

	/** Microphone position **/
	out_opt->micpos.resize( MAX_INPUT_CHANNEL );
	out_opt->micpos[0] = DEFAULT_MICPOS_1;
	out_opt->micpos[1] = DEFAULT_MICPOS_2;
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
				out_opt->Fs = atoi(argv[0]);
			}
			/** FFT size **/
			else if ( !strncmp( argv[0], "-fft", 5 ) || !strncmp( argv[0], "--fftsize", 10 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: FFT size");
				out_opt->FFTsize = atoi(argv[0]);
			}
			/** Window size **/
			else if ( !strncmp( argv[0], "-wsize", 7 ) || !strncmp( argv[0], "--window-size", 14 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: Window size");
				out_opt->wsize = atoi(argv[0]);
				out_opt->shift = out_opt->wsize / 2;
			}
#if 0
			/** hanning or hamming **/
			else if ( !strncmp( argv[0], "--hanning", 10 ) ) {
				out_opt->wtype = WTYPE_HANNING;
			}
			else if ( !strncmp( argv[0], "--hamming", 10 ) ) {
				out_opt->wtype = WTYPE_HAMMING;
			}
#endif
			/** iterations for ICA **/
			else if ( !strncmp( argv[0], "-it", 4 ) || !strncmp( argv[0], "--iteration", 13 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: iteration for ICA");
				out_opt->iteration= atoi(argv[0]);
			}
			/** stepsize for ICA **/
			else if ( !strncmp( argv[0], "-mu", 4 ) || !strncmp( argv[0], "--stepsize", 11 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: stepsize parameter for ICA");
				out_opt->stepsize = (FLOAT)atof(argv[0]);
			}
			/** silent mode **/
			else if ( !strncmp( argv[0], "-q", 4 ) || !strncmp( argv[0], "--quiet", 8 ) ) {
				out_opt->isQuiet = 1;
			}			
			/** multi-channel input **/
			else if ( !strncmp( argv[0], "-m", 3 ) || !strncmp( argv[0], "--multi-input", 14 ) ) {
				fprintf(stderr, "********************************************\n");
				fprintf(stderr, "Now, multi-channel input is not implemented.\n");
				fprintf(stderr, "********************************************\n");
				exit( ERR_CMDLINE );
			}			
			/** init_matrix **/
			else if ( !strncmp( argv[0], "-init", 6 ) || !strncmp( argv[0], "--init-matrix", 14 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: --init-matrix requires filename");
				strncpy( out_opt->init_matrix, argv[0], PATH_MAX );
			}			
			/** separated signal prefix **/
			else if ( !strncmp( argv[0], "-out", 5 ) ){
				if( ++argv, --argc == 0 ) show_help("Error in option: -out requires filename");
				strncpy( out_opt->out_prefix, argv[0], PATH_MAX );
			}			
			/** output optimized unmixing matrices **/
			else if ( !strncmp( argv[0], "-mat", 5 ) ){
				if( ++argv, --argc == 0 ) show_help("Error in option: -mat requires filename");
				strncpy( out_opt->out_matrix, argv[0], PATH_MAX );
			}
			/** DOA-based permutation solver**/
			else if ( !strncmp( argv[0], "--solve-perm-doa", 17 ) ){
				out_opt->solve_perm_type = SOLVE_PERM_DOA;
			}
			/** no solving permutation **/
			else if ( !strncmp( argv[0], "--no-solve-perm", 16 ) ){
				out_opt->solve_perm_type = SOLVE_PERM_NONE;
			}
			/** the number of input channel **/
			else if ( !strncmp( argv[0], "-ic", 3 ) || !strncmp( argv[0], "--input-channels", 17 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: input channels");
				out_opt->nMICs = atoi(argv[0]);
				out_opt->nOUTs = out_opt->nMICs;
				if( out_opt->nMICs < 2 ){
					fprintf(stderr, "**************************************\n");
					fprintf(stderr, "Inpupt channel must be greater than 2.\n");
					fprintf(stderr, "**************************************\n");
					exit( ERR_CMDLINE );
				}
			}
			/** the number of output channel **/
			else if ( !strncmp( argv[0], "-oc", 3 ) || !strncmp( argv[0], "--output-channels", 18 ) ) {
				fprintf(stderr, "***************************************************************\n");
				fprintf(stderr, "Now, option --output-channels is not enabled.\n");
				fprintf(stderr, "Output channels is set to be equal to input channels, forcedly.\n");
				fprintf(stderr, "***************************************************************\n");
				if( ++argv, --argc == 0 ) show_help("Error in option: input channels");
				//out_opt->nOUTs = atoi(argv[0]);
				out_opt->nOUTs = out_opt->nMICs;
			}
			/** show default parameters**/
			else if ( !strncmp( argv[0], "--default", 10 ) ) {
				show_defaults();
			}
			/** input file name **/
			else if ( !strncmp( argv[0], "-in", 3 ) && isdigit( argv[0][3] ) && !argv[0][4] && ( argv[0][3] - '0' ) > 0 ) {
				int channel = argv[0][3] - '0' - 1;
				if( ++argv, --argc == 0 ) show_help("Error in option: input file name is required");
				strncpy(out_opt->inputs[ channel ], argv[0], PATH_MAX );
			}
			/** microphone position **/
			else if ( !strncmp( argv[0], "-m", 2 ) && argv[0][2] && !argv[0][3] && ( argv[0][2] - '0' ) > 0 ){
				int channel = argv[0][2] - '0' - 1;
				if( ++argv, --argc == 0 ) show_help("Error in option: microphone position is required");
				out_opt->micpos[ channel ] = atof( argv[0] );
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

/** Show version of the program and exit
 */
void show_version()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Frequency-domain ICA (FDICA) by batch algorithm\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Copyright (c) 2007--2008 Yu Takahashi, Shikano Lab., Nara Institute of Science and Technology\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Try \'--help\' for run time options.\n");
	fprintf(stderr, "\n");

	exit( NO_ERROR );
}

/** Show help of the program with error message and exit.
 *  @param in_errstr error message
 */
void show_help( const char *in_errstr )
{
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Frequency-domain ICA (FDICA) by batch algorithm\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s\n",in_errstr );

	exit( ERR_CMDLINE );
}

/** Show help of the program and exit.
 */
void show_help()
{
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Frequency-domain ICA (FDICA) by batch algorithm\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Filename options\n");
	fprintf(stderr, "  -in1: channel-1 filename\n");
	fprintf(stderr, "  -in2: channel-2 filename\n");
	fprintf(stderr, "  -out: prefix for separated signal filename\n");
	fprintf(stderr, "  -mat, --matrices: output optimized filter filename\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Analysis options [default]\n");
	fprintf(stderr, "  -ic,    --input-channel: the number of input channel[%d]\n", DEFAULT_INPUT_CHANNEL);
	fprintf(stderr, "  -fs,    --sample-freq:   Sampling freqency [%d]\n", DEFAULT_SAMPLE_FREQ);
	fprintf(stderr, "  -fft,   --fftsize:       fft size. [%d]\n", DEFAULT_FFT_SIZE);
	fprintf(stderr, "  -wsize, --window-size:   window size. [%d]\n", DEFAULT_WINDOW_SIZE);
	fprintf(stderr, "\n");

	fprintf(stderr, "ICA options [default]\n");
	fprintf(stderr, "  -init,  --init-matrix: initial matrix for ICA [%s]\n",  DEFAULT_ICA_INIT_MATRIX);
	fprintf(stderr, "  -mu,    --stepsize:    stepsize parameter [%g]\n",  DEFAULT_ICA_STEPSIZE);
	fprintf(stderr, "  -it,    --iteration:   the number of iteration [%d]\n", DEFAULT_ICA_ITERATION);
	fprintf(stderr, "  -m1, -m2, ...:         microphone positions [%g,%g]\n", DEFAULT_MICPOS_1, DEFAULT_MICPOS_2 );
	fprintf(stderr, "  --solve-perm-doa:      DOA-based permuation solver\n");
	fprintf(stderr, "  --no-solve-perm:       no solving permutation\n");

	fprintf(stderr, "\n");
	
	fprintf(stderr, "Others:\n");
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
	fprintf(stderr, "Frequency-domain ICA (FDICA) by batch algorithm\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "=== Default parameters ===\n");
	fprintf(stderr, "Separated signal prefix   = %s\n", DEFAULT_OUTPUT_PREFIX );
	fprintf(stderr, "Optimized matrices output = %s\n", DEFAULT_MATRIX_OUTPUT );
	fprintf(stderr, "\n");
	fprintf(stderr, "Sampling frequency = %d\n", DEFAULT_SAMPLE_FREQ );
	fprintf(stderr, "FFT size           = %d\n", DEFAULT_FFT_SIZE );
	fprintf(stderr, "Window size        = %d\n", DEFAULT_WINDOW_SIZE );
	fprintf(stderr, "Window type        = %s\n", (DEFAULT_WINDOW_TYPE == WTYPE_HANNING ? "Hanning" : "Hamming" ) );
	fprintf(stderr, "Window shift size  = %d\n", DEFAULT_SHIFT_SIZE );
	fprintf(stderr, "\n");
	fprintf(stderr, "ICA iterations     = %d\n", DEFAULT_ICA_ITERATION );
	fprintf(stderr, "ICA stepsize       = %f\n", DEFAULT_ICA_STEPSIZE );
	fprintf(stderr, "\n");
	fprintf(stderr, "Input channels     = %d\n", DEFAULT_INPUT_CHANNEL );
	fprintf(stderr, "Output channels    = %d\n", DEFAULT_OUTPUT_CHANNEL );
	fprintf(stderr, "\n");
	fprintf(stderr, "Mic. positions     = %g, %g\n", DEFAULT_MICPOS_1, DEFAULT_MICPOS_2 );


	exit( NO_ERROR );
}

/** Show specified options of the program
 *
 *  @param in_opt specfied options
 */
void show_options( OPTION *in_opt )
{
	int i;
	//	printf("The number of microphones = %d\n", in_opt->nMICs );
	fprintf(stdout, "=== OPTIONS ===\n");
	fprintf(stdout, "Sampling frequency = %d\n", in_opt->Fs );
	fprintf(stdout, "FFT size           = %d\n", in_opt->FFTsize );
	fprintf(stdout, "Window size        = %d\n", in_opt->wsize );
	fprintf(stdout, "Window shift size  = %d\n", in_opt->shift );
	fprintf(stdout, "\n");
	fprintf(stdout, "ICA iterations     = %d\n", in_opt->iteration );
	fprintf(stdout, "ICA stepsize       = %f\n", in_opt->stepsize );
	fprintf(stdout, "\n");

	fprintf(stdout, "Init matrix        = %s\n\n", in_opt->init_matrix );
	for( i = 0 ;i < in_opt->nMICs; i++ ) {
		fprintf(stdout, "Input filename %d   = %s\n", i + 1, in_opt->inputs[ i ] );
	}
	fprintf(stdout, "\n");

	fprintf(stdout, "Separated signal prefix   = %s\n", in_opt->out_prefix );
	if( in_opt->out_matrix[0] != '\0' )
		fprintf(stdout, "Optimized matrices output = %s\n", in_opt->out_matrix );
	fprintf(stdout, "\n");
	if( in_opt->solve_perm_type == SOLVE_PERM_NONE )
		fprintf(stdout, "Without solving permutation\n");
	if( in_opt->solve_perm_type == SOLVE_PERM_DOA )
		fprintf(stdout, "DOA-based permutation solver\n");
	fprintf(stdout, "\n");
	for( i = 0 ;i < in_opt->nMICs; i++ ) {
		fprintf(stdout, "Microphone position %d   = %g\n", i + 1, in_opt->micpos[ i ] );
	}

	fprintf(stdout, "\n");
}
