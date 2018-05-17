/**
 * @file mem_util.hh
 * @brief some macros for memory allocatings
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2006-01-23
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __MEMUTIL_H_
#define __MEMUTIL_H_

/*! \def SAFE_DELETE(x)
 *  check the specified x, and delete x when x !=NULL.
 */
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)       { if(x){delete x; x=NULL;} }
#endif

/*! \def SAFE_DELETE_ARRAY(x)
 *  check the specified array x, delete [] x when x != NULL.
 */
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) { if(x){delete [] x; x=NULL;} }
#endif

#endif
