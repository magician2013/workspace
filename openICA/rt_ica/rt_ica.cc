/**
 * @file rt_ica.cc
 * @brief including main function of fdica by block wise batch algorithm
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2008-08-18
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

#include "rt_ica.hh"
#include "oss.hh"

#include "fdica_blkwise.hh"

/** Prototyping **/
void set_default_option( OPTION *out_opt );
void set_device_params( OPTION *param );

void init_device( COSS *device, OPTION *param );

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
	COSS *device = new COSS;
	int res;

	set_default_option( opt );

	/** parsing command line arguments **/
	parse_cmdline( opt, argc, argv );
	
	set_device_params( opt );
	/** show options **/
	if( !opt->isQuiet )
		show_options( opt );

	/** Open device **/
	if( opt->rawinput[0] != '\0' ){
		if( opt->rawoutput[0] != '\0' ) {
			if( ( res = device->open_as_read( opt->rawinput ) ) < 0 ){
				fprintf(stderr, "Cannot open %s as input file\n", opt->rawinput );
			}
		} else {
			fprintf( stderr, "Rawfile-input mode requires output file name.\n");
			res = -1;
		}
	} else {
		if( ( res = device->open_as_duplex( opt->device ) ) < 0 ) {
			fprintf(stderr, "Cannot open audio device %s\n", opt->device );
			exit(1);
		}
		init_device( device, opt );
	}

	if( res >= 0 ){
		/** run FDICA by block wise algorithm **/
		fdica_blkbatch( device, opt );

		/** close device **/
		device->close();
	} else {
		perror("Open deivce");
		//fprintf( stderr, "Cannot open audio device %s\n", opt->device );
	}
	/** exit **/
	SAFE_DELETE( device );
	SAFE_DELETE( opt );
	return NO_ERROR;
}

/** Initialize DSP device **/
void init_device( COSS *device, OPTION *param )
{
	int fragment, fragment_bit;

	/** Setup fragment size **/
	fragment = 1;
	fragment_bit = 1;
	if (fragment == param->dev.read_size_byte ){
		while( fragment != ( 1 << fragment_bit ) )
			fragment_bit++;
	} else {
		while( fragment != param->dev.read_size_byte ) {
			fragment_bit++;
			fragment = ( 1 << fragment_bit );
		}
	}
	fragment = ( ( 0x7FFF ) << 16 ) | (fragment_bit) ;

	if( device->set_fragment_size( fragment ) == 0 ){
		/** Set sound format, frequency, and channels **/
		if( device->init_dsp( RTICA_SAMPLE_FMT, param->Fs, param->nMICs ) < 0 ) {
			fprintf(stderr,"Cannot initialize audio device as duplex mode: AFMT_S16_LE, %d Hz, %dch\n", param->Fs, param->nMICs );
			exit(1);
		}
		fragment = device->get_fragment_size();
		if(!param->isQuiet)
			fprintf(stderr, "Set fragment size: %d bytes [%d bit]\n", fragment, fragment_bit );
		//		device->sync();
	} else {
		fprintf(stderr, "Cannot set fragment size %d\n", fragment );
		exit(1);
	}
	
}


/** Set default options
 *
 * @param out_opt target option structure
 */
void set_default_option( OPTION *out_opt )
{
	/** default audio device **/
	strncpy( out_opt->device, RTICA_AUDIO_DEVICE, PATH_MAX );

	/** analysis option **/
	out_opt->Fs      = RTICA_SAMPLE_FREQ;
	out_opt->FFTsize = RTICA_FFT_SIZE;
	out_opt->wsize   = RTICA_WINDOW_SIZE;
	out_opt->wtype   = DEFAULT_WINDOW_TYPE;
	out_opt->shift   = RTICA_SHIFT_SIZE;

	/** I/O option **/
	out_opt->nMICs     = RTICA_INPUT_CHANNEL;
	out_opt->nOUTs     = RTICA_OUTPUT_CHANNEL;
	out_opt->rawinput[0]  = '\0';
	out_opt->rawoutput[0] = '\0';

	/** ICA option **/
	out_opt->stepsize  = DEFAULT_ICA_STEPSIZE;
	out_opt->solve_perm_type = DEFAULT_PERM_TYPE;
	strncpy( out_opt->init_matrix, DEFAULT_ICA_INIT_MATRIX, PATH_MAX );

	/** Another option **/
	out_opt->isQuiet = 0;
	out_opt->isSwap  = DEFAULT_IS_SWAP;
	out_opt->defaultMode = DEMO_MODE_DEFAULT;
	out_opt->defaultSource = DEMO_SOURCE_DEFAULT;

	/** Microphone position **/
	out_opt->micpos.resize( MAX_INPUT_CHANNEL );
	out_opt->micpos[0] = DEFAULT_MICPOS_1;
	out_opt->micpos[1] = DEFAULT_MICPOS_2;

	/** Block wise ICA params **/
	out_opt->ICA_blklen = RTICA_BLOCK_LEN;
	out_opt->ICA_frames = (int)( ( (FLOAT)out_opt->Fs * out_opt->ICA_blklen ) / ( (FLOAT)out_opt->wsize / 2 ) - 1 );
	out_opt->ICA_ring_frames = out_opt->ICA_frames * 3;
	out_opt->ICA_power_th = RTICA_BLOCK_POW_THRES;
	out_opt->ICA_reset_blk = RTICA_RESET_BLOCK;
}
/** Set device parameteres
 *
 * @param out_opt target option structure
 */
void set_device_params( OPTION *param )
{
	DBGSTR("set_device_params","set device parameters");
	param->dev.frame_size = param->nMICs * param->FFTsize;
	param->dev.read_size  = param->nMICs * param->shift;
	param->dev.ring_size  = param->dev.read_size * RTICA_RING_SIZE;

	param->dev.read_size_byte = param->dev.read_size * sizeof(short);
	param->dev.ring_size_byte = param->dev.ring_size * sizeof(short);

	param->ICA_frames = (int)( ( (FLOAT)param->Fs * param->ICA_blklen ) / ( (FLOAT)param->wsize / 2 ) - 1 );
	param->ICA_ring_frames = param->ICA_frames * RTICA_RING_LEN;
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
	DBGSTR("parse_cmdline","parse command line");
	if( argc > 1 ) {
		while( argv++, --argc > 0 ) {
			DBGSTR("parse_cmdline:given",argv[0]);
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
				out_opt->wsize   = out_opt->FFTsize / 2;
				out_opt->shift   = out_opt->wsize / 2;
			}
			/** stepsize for ICA **/
			else if ( !strncmp( argv[0], "-mu", 4 ) || !strncmp( argv[0], "--stepsize", 11 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: stepsize parameter for ICA");
				out_opt->stepsize = (FLOAT)atof(argv[0]);
			}
			/** device **/
			else if ( !strncmp( argv[0], "-d", 3 ) || !strncmp( argv[0], "--device", 9 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: audio device\n");
				strncpy( out_opt->device, argv[0], PATH_MAX );
			}
			/** ICA block length **/
			else if ( !strncmp( argv[0], "-b", 3 ) || !strncmp( argv[0], "--block-len", 13 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: block length\n");
				out_opt->ICA_blklen = atof( argv[0] );
			}
			/** ICA block power threshold  **/
			else if ( !strncmp( argv[0], "-bt", 4 ) || !strncmp( argv[0], "--block-power-thres", 20 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: block power thredhold\n");
				out_opt->ICA_power_th = atof( argv[0] );
			}
			/** Reset unmixing matrix every specified block  **/
			else if ( !strncmp( argv[0], "-rb", 4 ) || !strncmp( argv[0], "--reset-blocks", 15 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: reset blocks\n");
				out_opt->ICA_reset_blk = atoi( argv[0] );
			}
			/** silent mode **/
			else if ( !strncmp( argv[0], "-q", 3 ) || !strncmp( argv[0], "--quiet", 8 ) ) {
				out_opt->isQuiet = 1;
			}			
			/** init_matrix **/
			else if ( !strncmp( argv[0], "-init", 6 ) || !strncmp( argv[0], "--init-matrix", 14 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: --init-matrix requires filename");
				strncpy( out_opt->init_matrix, argv[0], PATH_MAX );
			}			
			/** no solving permutation **/
			else if ( !strncmp( argv[0], "--no-solve-perm", 16 ) ){
				out_opt->solve_perm_type = SOLVE_PERM_NONE;
			}
			/** solving permutation by DOA**/
			else if ( !strncmp( argv[0], "--solve-perm-doa", 17 ) ){
				out_opt->solve_perm_type = SOLVE_PERM_DOA;
			}
			/** solving permutation by power**/
			else if ( !strncmp( argv[0], "--solve-perm-power", 19 ) ){
				out_opt->solve_perm_type = SOLVE_PERM_POWER;
			}
			/** show default parameters**/
			else if ( !strncmp( argv[0], "--default", 10 ) ) {
				show_defaults();
			}
			/** microphone position **/
			else if ( !strncmp( argv[0], "-m", 3 ) && argv[0][2] && !argv[0][3] && ( argv[0][2] - '0' ) > 0 ){
				int channel = argv[0][2] - '0' - 1;
				if( ++argv, --argc == 0 ) show_help("Error in option: microphone position is required");
				out_opt->micpos[ channel ] = atof( argv[0] );
			}			
			/** rawinput mode **/
			else if ( !strncmp( argv[0], "-f", 3 ) || !strncmp( argv[0], "--input", 8 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: input file\n");
				strncpy( out_opt->rawinput, argv[0], PATH_MAX );
			}
			/** raw output mode **/
			else if ( !strncmp( argv[0], "-o", 3 ) || !strncmp( argv[0], "--output", 9 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: output file\n");
				strncpy( out_opt->rawoutput, argv[0], PATH_MAX );
			}
			/** ICA iterations for raw-input mode  **/
			else if ( !strncmp( argv[0], "-it", 4 ) || !strncmp( argv[0], "--iteration", 12 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: iterations for raw-input mode\n");
				out_opt->ICA_iteration = atoi( argv[0] );
			}
			/** default mode  **/
			else if ( !strncmp( argv[0], "--mode", 7 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: default mode\n");
				if( !strncmp( argv[0], "ICA", 4) )
					out_opt->defaultMode = DEMO_MODE_ICA;
				else if( !strncmp( argv[0], "THROUGH", 8 )  )
					out_opt->defaultMode = DEMO_MODE_THROUGH;
				else
					show_help("Error in option: Unknown default mode");
			}
			/** default target source number(index) for ICA  **/
			else if ( !strncmp( argv[0], "--source", 9 ) ) {
				if( ++argv, --argc == 0 ) show_help("Error in option: default source index\n");
				out_opt->defaultSource = atoi( argv[0] );
				if( !(1 <= out_opt->defaultSource && out_opt->defaultSource <= 2 ) ) {
					show_help("Error in option: default source must be 1 or 2.");
				}
				out_opt->defaultSource--;
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
		DBGSTR("parse_cmd_line","all default options are used");
		//		show_version();
	}
	return NO_ERROR;
}

/** Show version of the program and exit
 */
void show_version()
{
	DBGSTR("show_version","version info");
	fprintf(stderr, "\n");
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Frequency-domain ICA (FDICA) by real-time (block-wise) alogorithm\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Copyright (c) 2007--2008 Shikano Lab., Nara Institute of Science and Technology\n");
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
	DBGSTR("show_help", in_errstr);
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Frequency-domain ICA (FDICA) by real-time (block-wise) alogorithm\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s\n",in_errstr );

	exit( ERR_CMDLINE );
}

/** Show help of the program and exit.
 */
void show_help()
{
	DBGSTR("show_help","show help");
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Frequency-domain ICA (FDICA) by real-time (block-wise) alogorithm\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Analysis options [default]\n");
	fprintf(stderr, "  -fs,   --sample-freq:   Sampling freqency [%d]\n", DEFAULT_SAMPLE_FREQ);
	fprintf(stderr, "  -fft,  --fftsize:       FFT size. [%d]\n", RTICA_FFT_SIZE);
	fprintf(stderr, "\n");

	fprintf(stderr, "ICA options [default]\n");
	fprintf(stderr, "  -init, --init-matrix:     initial matrix for ICA [%s]\n",  DEFAULT_ICA_INIT_MATRIX);
	fprintf(stderr, "  -mu,   --stepsize:        stepsize parameter [%g]\n",      DEFAULT_ICA_STEPSIZE);
	fprintf(stderr, "  -m1, -m2, ...:            microphone positions for DOA based permutation solver [%g,%g]\n", DEFAULT_MICPOS_1, DEFAULT_MICPOS_2 );
	fprintf(stderr, "  -b,    --block-len:       one block size for ICA in second(s) [%g].\n",   RTICA_BLOCK_LEN );
	fprintf(stderr, "  -bt,   --block-pow-thres: block power threshold for ignoring that block [%g].\n",   RTICA_BLOCK_POW_THRES );
	fprintf(stderr, "  -rb,   --reset-block:     reset matrix every specified blocks [%d].\n",   RTICA_RESET_BLOCK );
	fprintf(stderr, "  -it,   --iteration:       iterations for raw-input mode [%d].\n",   RTICA_ICA_ITERATION );
	fprintf(stderr, "  --solve-perm-doa:         solving permutation by DOA based method.\n");
	fprintf(stderr, "  --solve-perm-power:       solving permutation by power based method.\n");
	fprintf(stderr, "  --no-solve-perm:          no solving permutation [default]\n");
	fprintf(stderr, "  --mode:                   default output mode [ICA]\n");
	fprintf(stderr, "  --source:                 default target source index for ICA [%d]\n", DEMO_SOURCE_DEFAULT);

	fprintf(stderr, "\n");
	
	fprintf(stderr, "I/O option:\n");
	fprintf(stderr, "  -d, --device:  audio device[%s]\n",RTICA_AUDIO_DEVICE);
	fprintf(stderr, "  -f, --input:   use specified multichannel raw file instead of audio device\n");
	fprintf(stderr, "  -o, --output:  output to specified file instead of audio device\n");

	fprintf(stderr, "\n");

	fprintf(stderr, "Others:\n");
	fprintf(stderr, "  -q, --quiet:        silent mode\n");
	fprintf(stderr, "  -h, --help:         show this help.\n");
	fprintf(stderr, "  -v, --version:      show version.\n");

	exit( NO_ERROR );
}

/** Show default parameters
 *
 *  @param in_opt specfied options
 */
void show_defaults( void  )
{
	DBGSTR("show_default","show default");
	fprintf(stderr, "Open ICA ver %d.%d.%d built for %s\n", __VERSION_MAJOR, __VERSION_MINOR, __VERSION_REV, __BUILD_STRING );
	fprintf(stderr, "Frequency-domain ICA (FDICA) by block wise batch algorithm\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "=== Default parameters ===\n");
	fprintf(stderr, "Sampling frequency = %d\n", DEFAULT_SAMPLE_FREQ );
	fprintf(stderr, "FFT size           = %d\n", RTICA_FFT_SIZE );
	fprintf(stderr, "Window size        = %d\n", RTICA_WINDOW_SIZE );
	fprintf(stderr, "Window shift size  = %d\n", RTICA_SHIFT_SIZE );
	fprintf(stderr, "\n");
	fprintf(stderr, "ICA stepsize              = %g \n", DEFAULT_ICA_STEPSIZE );
	fprintf(stderr, "ICA block size            = %g seconds.\n", RTICA_BLOCK_LEN );
	fprintf(stderr, "ICA block power threshold = %g \n", RTICA_BLOCK_POW_THRES );
	fprintf(stderr, "\n");
	fprintf(stderr, "Input channels     = %d [fixed]\n", DEFAULT_INPUT_CHANNEL );
	fprintf(stderr, "Output channels    = %d [fixed]\n", DEFAULT_OUTPUT_CHANNEL );
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
	DBGSTR("show_options","show options");
	int i;
	printf("=== Specified options ===\n");
	printf("Sampling frequency = %d\n", in_opt->Fs );
	printf("FFT size           = %d\n", in_opt->FFTsize );
	printf("Window size        = %d\n", in_opt->wsize );
	printf("Window shift size  = %d\n", in_opt->shift );
	printf("\n");
	printf("ICA stepsize       = %f\n", in_opt->stepsize );
	printf("ICA blok length    = %g sec.\n", in_opt->ICA_blklen );
	printf("ICA blok pow thres = %g \n", in_opt->ICA_power_th );
	printf("ICA frame/block    = %d\n", in_opt->ICA_frames );
	printf("ICA frame/ring     = %d\n", in_opt->ICA_ring_frames );
	printf("Audio device       = %s\n", in_opt->device );
	printf("Read size          = %d\n", in_opt->dev.read_size );
	printf("Read size bytes    = %d\n", in_opt->dev.read_size_byte );
	printf("Ring size          = %d\n", in_opt->dev.ring_size );
	printf("Ring size bytes    = %d\n", in_opt->dev.ring_size_byte );
	printf("\n");

	printf("Init matrix        = %s\n\n", in_opt->init_matrix );
	if( in_opt->solve_perm_type == SOLVE_PERM_NONE )
		printf("Without solving permutation\n");
	else if( in_opt->solve_perm_type == SOLVE_PERM_DOA )
		printf("Solving permutation by DOA based method.\n");
	else if( in_opt->solve_perm_type == SOLVE_PERM_POWER )
		printf("Solving permutation by power based method.\n");
	for( i = 0 ;i < in_opt->nMICs; i++ ) {
		printf("Microphone position %d   = %g\n", i + 1, in_opt->micpos[ i ] );
	}
	printf("\n\n");
}
