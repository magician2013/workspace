/*
 * @file config.hh
 * @brief build configurations
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * @date 2007-12-28
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef _CONFIG_H_INCLUDED_
#define _CONFIG_H_INCLUDED_

#define __VERSION_MAJOR    0
#define __VERSION_MINOR    0
#define __VERSION_REV      5

#define __DEBUG /* for debug */

#ifdef   __DEBUG
#define __BUILD_STRING     "debug"
#else
#define __BUILD_STRING     "release"
#endif


/** setting accuracy of floating point **/
typedef double FLOAT;
//typedef float FLOAT;

#endif
/* end of config.hh */
