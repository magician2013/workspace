/**
 * @file oss.cc
 * @brief 
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-28
 *
 */
#include "oss.hh"

/** Constructor
 */
COSS::COSS()
{
	m_fd = -1; /** initialize file descripter for DSP **/
}

/** Copy constructor
 */
COSS::COSS( const COSS &object )
{
	this->m_fd = object.m_fd;
}

/** Destructor
 */
COSS::~COSS()
{
	this->close();
}

/** device open as read only mode
 *
 *  @param in_str_devname device name
 *  
 *  @retval opened file id.
 */
int COSS::open_as_read( const char *in_str_devname )
{
	m_fd = open( in_str_devname, O_RDONLY );
	return m_fd;
}

/** device open as write only mode
 *
 *  @param in_str_devname device name
 *  
 *  @retval opened file id.
 */
int COSS::open_as_write( const char *in_str_devname )
{
	m_fd = open( in_str_devname, O_WRONLY );
	return m_fd;
}

/** device open as write/read mode
 *
 *  @param in_str_devname device name
 *  
 *  @retval opened file id.
 */
int COSS::open_as_duplex( const char *in_str_devname )
{
	int caps;

	m_fd = open( in_str_devname, O_RDWR );
	
	if( m_fd < 0 )
		return m_fd;
	
	ioctl( m_fd, SNDCTL_DSP_SETDUPLEX, 0 );

	/** Get DSP Capability **/
	if( ioctl( m_fd, SNDCTL_DSP_GETCAPS, &caps ) == (-1) ) {
		perror("ioctl(SNDCTRL_DSP_GETCAPS)");
		return (-1);
	}
	/** Check for duplex support **/
	if( !(caps & DSP_CAP_DUPLEX)  )
		return (-1);

	fsync(m_fd);
	return m_fd;
}

/** close device
 *
 */
int COSS::close( void )
{
	if( m_fd  > 0 )
		::close( m_fd );
	return 0;
}

/** set sampling frequency
 */
int COSS::set_sampling_rate( int in_iRate )
{
	if( ioctl( m_fd, SOUND_PCM_WRITE_RATE, &in_iRate ) == -1 ) {
		perror("ioctl(SOUND_PCM_WRITE_RATE");
		return -1;
	}
	return 0;
}

/** set audio format 
 *
 *  @param in_fmt sound format
 */
int COSS::set_fmt( int in_fmt )
{
	if( ioctl( m_fd, SOUND_PCM_SETFMT, &in_fmt ) == -1 ) {
		perror("ioctl(SOUND_PCM_SETFMT)");
		return -1;
	}
	return 0;
}

/** set number of channel
 *
 *  @param in_iChannels channels
 */
int COSS::set_channel( int in_iChannels )
{
	if( ioctl( m_fd, SOUND_PCM_WRITE_CHANNELS, &in_iChannels ) == -1 ) {
		perror("ioctl(SOUND_PCM_WRITE_CHANNELS)");
		return -1;
	}
	return 0;
}

/** initialize audio device with fmt, sampling rate, channesl
 *
 *  @param in_fmt sound format
 *  @param in_iRate sampling frequency
 *  @param in_iCh channels
 *
 *  @retval 0 no error
 *  @retval (-1) some errors occurred
 *
 */
int COSS::init_dsp( int in_fmt, int in_iRate, int in_iCh )
{
	if( this->set_fmt( in_fmt ) < 0 ) return -1;
	if( this->set_sampling_rate( in_iRate ) < 0 ) return -1;
	if( this->set_channel( in_iCh ) < 0 ) return -1;
	return 0;
}

/** read from device
 *
 *  @param out_buf output buffer
 *  @param in_rsize read size

 *  @retval read size
 *
 */
int COSS::read( void *out_buf, const size_t in_rsize )
{
	return ::read( m_fd, out_buf, in_rsize );
}

/** write to device
 *
 *  @param in_buf write buffer
 *  @param in_rsize write size

 *  @retval write size
 *
 */
int COSS::write( void *in_buf, const size_t in_wsize )
{
	return ::write( m_fd, in_buf, in_wsize );
}

int COSS::sync()
{
	return ioctl( m_fd, SNDCTL_DSP_POST, 0 );
}
/** get fragement size of device
 *
 *  @retval fragment size
 */
int COSS::get_fragment_size()
{
	int fragment;

	if( ioctl( m_fd, SNDCTL_DSP_GETBLKSIZE, &fragment ) < 0 ){
		perror("ioctl(SNDCTL_DSP_GETBLKSIZE)");
		return (-1);
	}
	return fragment;
}

/** set fragement size of device
 *
 *  @param fragment fragment size
 *
 *  @retval 0 no error
 *  @retval -1 error
 */
int COSS::set_fragment_size( int fragment )
{
	if( ioctl( m_fd, SNDCTL_DSP_SETFRAGMENT, &fragment ) < 0 ){
		perror("ioctl(SNDCTL_DSP_SETFRAGMENT)");
		return (-1);
	}
	return 0;
}

/** Get file descripter
 *
 * @retval file descriptor
 */
int COSS::get_descriptor( void )
{
	return m_fd;
}
