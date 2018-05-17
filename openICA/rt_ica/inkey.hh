/**
 * @file inkey.hh
 * @brief implementation of no-wait reading keyboard
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2008-1-6
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __INKEY_H_INCLUDED_
#define __INKEY_H_INCLUDED_

void init_keyboard();
void close_keyboard();
int kbhit();
int inkey();

#endif /** end of inkey.hh **/
