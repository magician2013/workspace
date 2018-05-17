/**
 * @file infomax.hh
 * @brief implementation of infomax for ICA
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef _INFOMAX_H_INCLUDED_
#define _INFOMAX_H_INCLUDED_

#include "mathtool.hh"
#include "debug.hh"

extern int infomax( C3_VECTOR &W, C3_VECTOR &Xst, int tapsize, FLOAT stepsize, int iterations, int isQuiet );

#endif
/* end of infomax.hh */

