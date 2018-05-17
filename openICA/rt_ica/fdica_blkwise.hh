/**
 * @file fdica_blkwise.cc
 * @brief implementation of FDICA by block wise batch algorithm
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-28
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/

#ifndef __FDICA_BLKWISE_H_
#define __FDCIA_BLKWISE_H_

#define ERR_IN_DEVICE_READ    1
#define ERR_IN_DEVICE_WRITE   2
#define ERR_IN_CREATE_THREAD  10

#define ICA_THREAD_STOP (-1)
#define ICA_THREAD_WAIT               (0)
#define ICA_THREAD_OPTIMIZE           (1)
#define ICA_THREAD_SEND_NEW_BUFFER    (2)
#define ICA_THREAD_RECEIVE_NEW_BUFFER (3)
#define ICA_THREAD_RESET_MATRIX       (4)
#define ICA_THREAD_END_OPTIMIZE       (5)

#define ICA_THREAD_EXIT               (999)

#define DEMO_MODE_THROUGH  (0)
#define DEMO_MODE_ICA      (3)
#define DEMO_MODE_STOP     (999)
#define DEMO_MODE_DEFAULT  DEMO_MODE_ICA

#define DEMO_SOURCE_DEFAULT 0

extern int fdica_blkbatch( COSS *device, OPTION *param );

#endif 
