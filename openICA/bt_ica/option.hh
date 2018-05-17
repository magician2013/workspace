/**
 * @file option.hh
 * @brief definition for OPTION structure for bt_ica
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef _OPTION_H_INCLUDED_
#define _OPTION_H_INCLUDED_

#include <climits>
#include "config.hh"
#include "defs.hh"
#include "types.hh"

/*! \struct OPTION
 *  \brief structure for application options
*/
typedef struct {
	//! the number of microphone elements (input channel)
	int nMICs;
	//! the number of output
	int nOUTs;
	//! sampling frequency
	int Fs;
	//! FFT size
	int FFTsize;
	//! window size
	int wsize;
	//! window type
	int wtype;
	//! window shift size
	int shift;

	//! microphone positions
	D_VECTOR micpos;

	//! stepsize parameter of ICA
	FLOAT stepsize;

	//! the number of iteration of ICA
	int iteration;

	//! silent mode flag
	int isQuiet;
	//! swapping byte order (Now, not implemented)
	int isSwap;

	//! input file names
	char inputs[MAX_INPUT_CHANNEL][PATH_MAX];

	//! initial matrix file
	char init_matrix[PATH_MAX];

	//! outname prifix
	char out_prefix[PATH_MAX];

	//! output matrices filanem
	char out_matrix[PATH_MAX];

	//! type permutation solver
	int solve_perm_type;
} OPTION;

#endif
/* end of option.h */
