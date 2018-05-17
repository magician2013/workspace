/**
 * @file nbf.hh
 * @brief definition for nbf.cc
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * @date 2006-01-23
 * @version $Id: nbf.hh,v 0.1 2006/01/23 Yu Exp $
 *
 * Copyright (C) Yu TAKAHASHI. SPALAB. All right reserved.
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/

#ifndef __NBF_H_INCLUDED_
#define __NBF_H_INCLUDED_

#include "mathtool.hh"

/** \def NBF_NOERROR
 *  \breif No error
 */
#define NBF_NOERROR 0

#define DEFAULT_DEG_1 -45.0
#define DEFAULT_DEG_2 45.0


//=========================================================================
//
// parameter for Null Beamformer structure NBFINFO
//
//-------------------------------------------------------------------------
/** 
 *  \brief Setups for NBF
 */
typedef struct {
	//! Null directions
	D_VECTOR deg;

	//! Sampling frequency
	int Fs;          

	//! Microphone positions
	D_VECTOR micpos;     

	//! the number of microphones
	int nMICs;

	//! the number of using microphones in each bands
	int nUseMICs;

	//! filter length
	int tapsize;

	//! sound velocity
	FLOAT c;

	//! frequency bands setups
	D_VECTOR freq;

	//! flag for using pseudo inverse matrix for constructing NBF
	int isPinv;

} NBFINFO;

extern int makeNBF( C3_VECTOR &out_filter , NBFINFO *nbfinfo );
#endif
