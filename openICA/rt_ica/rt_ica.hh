/**
 * @file rt_ica.hh
 * @brief definition for rt_ica
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-28
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/

#ifndef __RT_ICA_H_
#define __RT_ICA_H_

//! Analysis defaults(Sampling Frequency)
#define RTICA_SAMPLE_FREQ    16000
//! Analysis defaults(FFT size, must be pow of 2)
#define RTICA_FFT_SIZE       512
//! Analysis defaults(Window size, must be FFT_SIZE/2)
#define RTICA_WINDOW_SIZE    (RTICA_FFT_SIZE/2)
//! Analysis defaults(Window shift size, must be WINDOW_SIZE/2)
#define RTICA_SHIFT_SIZE     (RTICA_WINDOW_SIZE/2)

//! Block power thrshold for ignoring the block
#define RTICA_BLOCK_POW_THRES 50.0

//! ICA block size
#define RTICA_BLOCK_LEN       1.5

//! Reset matrix every specified blocks
#define RTICA_RESET_BLOCK     200

//! I/O Channels [fixed]
#define RTICA_INPUT_CHANNEL   2
//! I/O Channels [fixed]
#define RTICA_OUTPUT_CHANNEL  2

//! Default sound device
#define RTICA_AUDIO_DEVICE "/dev/dsp"

//! sound format
#define RTICA_SAMPLE_FMT   AFMT_S16_LE
//! Bytes per sample
#define RTICA_SAMPLE_BYTE  2

//! ring size for read/write ring buffer
#define RTICA_RING_SIZE    16

//! ring size for ICA's FFTed ring buffer
#define RTICA_RING_LEN     16

//! default iterations when raw-file input
#define RTICA_ICA_ITERATION       60

#endif /* end of rt_ica.hh */
