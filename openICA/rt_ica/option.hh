/**
 * @file option.hh
 * @brief definition for OPTION structure for bt_ica
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-28
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
#include "rt_ica.hh"

#include "types.hh"

/** \struct DEVPARAM
 */
typedef struct {
	//! frame samples = mic_num * FFTsize
	int frame_size;
	
	//! read samples = shift size * mic_num
	int read_size;
	
	//! read size in bytes
	int read_size_byte;
	
	//! ring buffer samples = read_size * RING_SIZE
	int ring_size;
	
	//! ring size in bytes
	int ring_size_byte;
} DEVPARAM;

/*! \struct OPTION
 *  \brief structure for application options
*/
typedef struct {
	//! Input file name for raw-input mode
	char rawinput[PATH_MAX];
	
	//! Output file name for raw-output
	char rawoutput[PATH_MAX];

	//! the number of microphone elements (input channel)
	int nMICs;

	//! the number of iteration of ICA
	int ICA_iteration;

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

	//! initial matrix file
	char init_matrix[PATH_MAX];

	//! type permutation solver
	int solve_perm_type;

	//! sound device
	char device[PATH_MAX];

	//! device params
	DEVPARAM dev;

	//! ICA block length
	FLOAT ICA_blklen;
	
	//! ICA block power threshold
	FLOAT ICA_power_th;

	//! ICA block by frame
	int ICA_frames;
	int ICA_ring_frames;

	//! ICA thread msg
	int ICA_msg;

	//! ICA reset block length
	int ICA_reset_blk;

	//! Default mode
	int defaultMode;

	//! Default target source number for ICA
	int defaultSource;

} OPTION;

#endif
/* end of option.h */
