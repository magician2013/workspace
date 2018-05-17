/**
 * @file defs.hh
 * @brief some default difinition for ICA program
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef _DEFS_H_INCLUDED_
#define _DEFS_H_INCLUDED_

//! Maximum inputs
#define MAX_INPUT_CHANNEL    9

/** Window Type**/
//! Hanning window
#define WTYPE_HANNING    0
//! Hamming window
#define WTYPE_HAMMING    1

//! No error
#define NO_ERROR  0

//! Some Error occurred in command line
#define ERR_CMDLINE 1

//! Default sampling frequnecy
#define DEFAULT_SAMPLE_FREQ    16000

//! Default FFT size
#define DEFAULT_FFT_SIZE       1024

//! Default window size
#define DEFAULT_WINDOW_SIZE    1024

//! Default shift size
#define DEFAULT_SHIFT_SIZE     512

//! Default window type
#define DEFAULT_WINDOW_TYPE   WTYPE_HANNING


//! Deafault stepsize parameter for ICA
#define DEFAULT_ICA_STEPSIZE  0.01

//! Default interations of updating unmixing matrix for ICA
#define DEFAULT_ICA_ITERATION 50

//! Default initial matrix for ICA
#define DEFAULT_ICA_INIT_MATRIX "IDENTITY"

//! Default output prefix
#define DEFAULT_OUTPUT_PREFIX   "y"

//! Default matrix output filename
#define DEFAULT_MATRIX_OUTPUT   ""

//! Default input channels
#define DEFAULT_INPUT_CHANNEL 2

//! Default output channels
#define DEFAULT_OUTPUT_CHANNEL 2

//! Default byte swapping policy
#define DEFAULT_IS_SWAP        0

//! Default sound velocity
#define DEFAULT_SOUND_VELOCITY 340.0

//! Default Microphone position
#define DEFAULT_MICPOS_1 0.0
#define DEFAULT_MICPOS_2 0.021

//! Permutation solver type
#define SOLVE_PERM_NONE    0
#define SOLVE_PERM_DOA     1
#define SOLVE_PERM_POWER   2

//! Default permuation solver type
#define DEFAULT_PERM_TYPE SOLVE_PERM_NONE


#endif
/* end of defs.hh */
