/**
 * @file inkey.cc
 * @brief implementation of no-wait reading keyboard
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2008-01-06
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>

static struct termios init_tio;

/** initizalize for no-wait reading keyboard
 *
 */
void init_keyboard()
{
	struct termios tio;
	tcgetattr(0,&init_tio);

	memcpy(&tio, &init_tio, sizeof(struct termios) );
	tio.c_lflag &= ~(ICANON|ECHO|ISIG);
	tcsetattr(0, TCSANOW, &tio );

}

/** finalize for no-wait reading keyboard
 *
 */
void close_keyboard()
{
	tcsetattr(0, TCSANOW, &init_tio );
}

/** confirm the keyboard buffer
 *
 * @retval 1 data exists in keyboard buffer
 * @retval 0 data does not exist keyboard buffer
 */
int kbhit()
{
	//	struct termios tio;
	struct timeval tv;
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(0,&rfds);
	tv.tv_usec = 0;
	tv.tv_sec = 0;
	select(1,&rfds,NULL,NULL,&tv);
	//	tcsetattr( 0, TCSANOW, &init_tio );
	return (FD_ISSET(0,&rfds)?1:0);
}

/** no-wait reading keyboard
 *
 * @retval input code
 */
int inkey()
{
	int ch=0;

	/** input **/
	read(0, &ch, 1 );
	
	return ch;
}
