/*
 * @file debug.hh
 * @brief some macros for debug
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * @date 2007-12-28
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef __DEBUG_H_
#define __DEBUG_H_

#define PRINT_ERR(function,cause,stat) { fprintf(stderr,"ERROR:[%s] %s: %s\n", function, cause, stat ); } 

#ifdef __DEBUG
#define DBGSTR(func,msg) { fprintf(stderr,"%s [%d] (%s): %s\n", __FILE__, __LINE__, func, msg); }
#else
#define DBGSTR(func,msg)
#endif

#endif
