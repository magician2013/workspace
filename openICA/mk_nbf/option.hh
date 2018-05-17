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
#include "nbf.hh"

/*! \struct OPTION
 *  \brief structure for application options
*/
typedef struct {
	//! silent mode flag
	int isQuiet;
	
	//! ouput filename
	char out_fname[PATH_MAX];

	//! parameters for NBF
	NBFINFO *nbfinfo;

} OPTION;

#endif
/* end of option.h */
