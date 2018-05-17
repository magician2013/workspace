/**
 * @file sigpro.hh
 * @brief functions for signal processing
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/

#ifndef _SIGPRO_H_INCLUDED_
#define _SIGPRO_H_INCLUDED_

extern void hanning( D_VECTOR &hw, int in_length );
extern void hamming( D_VECTOR &hw, int in_length );

extern void swap_filter( C_VECTOR &filter );
extern void swap_filter( C3_VECTOR &filter );

extern void linear_separate( C3_VECTOR &W, C3_VECTOR &Xst, int shift, D2_VECTOR &out );
extern void makeHP( C_VECTOR &filter, int filter_len, FLOAT Fs, FLOAT cutFreq );


#endif
/* end of sigpro.hh */
