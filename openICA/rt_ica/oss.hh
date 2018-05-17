#ifndef __OSS_H_
#define __OSS_H_

#include <fcntl.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

class COSS {
public:
	COSS();   /** Constructor **/
	COSS( const COSS &object ); 
	~COSS();  /** Deconstructor **/

	/** device open **/
	int open_as_read( const char *in_str_devname );
	int open_as_write( const char *in_str_devname );
	int open_as_duplex( const char *in_str_devname );

	/** device close **/
	int close( void );
	
	/** device control **/
	int init_dsp( int in_fmt, int in_iRate, int in_iCh );
	int set_sampling_rate( int in_iRate );
	int set_fmt( int in_fmt );
	int set_channel( int in_iChannels );
	int get_fragment_size();
	int set_fragment_size( int in_frament );

	/** I/O func **/
	int read( void *out_buf, size_t in_rsize );
	int write( void *in_buf, size_t in_wsize );
	int sync();

	/** FD getter **/
	int get_descriptor( void );

protected:
	int m_fd; /** A File Descripter for OSS device **/
	

private:
};
#endif
