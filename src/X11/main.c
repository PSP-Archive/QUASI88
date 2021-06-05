/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>	/* setuid, getuid */

#include "quasi88.h"
#include "initval.h"

#include "file-op.h"
#include "getconf.h"

#include "device.h"
#include "snddrv.h"	/* xmame_system_init(), xmame_system_term() */
#include "suspend.h"
#include "snapshot.h"
#include "menu.h"


static	void	the_end( void );

int	main( int argc, char *argv[] )
{
  int	x = 1;

	/* root���¤�ɬ�פʽ��� (X11��Ϣ) �򿿤���˹Ԥ� */

  x11_system_init();  /* �����ǥ��顼���ФƤ⥪�ץ������Ϥ���Τ���˿ʤ� */


	/* �ǡ����줬�����С� root ���¤������ */

  if( setuid( getuid() ) != 0 ){
    fprintf( stderr, "%s : setuid error\n", argv[0] );
    x11_system_term();
    return -1;
  }

  if( getuid() == 0 ) snapshot_cmd_enable = FALSE;


	/* ����ǥ�����ͥ������å� */

#ifdef LSB_FIRST
  if( *(char *)&x != 1 ){
    fprintf( stderr,
	     "%s CAN'T EXCUTE !\n"
	     "This machine is Big-Endian.\n"
	     "Compile again comment-out 'LSB_FIRST = 1' in Makefile.\n",
	     argv[0] );
    x11_system_term();
    return -1;
  }
#else
  if( *(char *)&x == 1 ){
    fprintf( stderr,
	     "%s CAN'T EXCUTE !\n"
	     "This machine is Little-Endian.\n"
	     "Compile again comment-in 'LSB_FIRST = 1' in Makefile.\n",
	     argv[0] );
    x11_system_term();
    return -1;
  }
#endif


  xmame_system_init();		/* XMAME������ɴ�Ϣ����� (��������������!) */

  quasi88_atexit( the_end );


  if( osd_environment() ){		/* �Ķ�����	*/

    if( config_init( argc, argv ) ){	/* ��������	*/

      quasi88();			/* PC-8801 ���ߥ�졼����� */
    }
  }


  xmame_system_term();		/* XMAME������ɴ�Ϣ����� */

  x11_system_term();		/* X11��Ϣ����� */

  return 0;
}



/*
 * ͽ��������λ��
 */
static	void	the_end( void )
{
  xmame_system_term();		/* XMAME������ɴ�Ϣ����� */
  x11_system_term();		/* X11��Ϣ����� */
}








/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

/*	¾�ξ��󤹤٤Ƥ����� or �����֤��줿��˸ƤӽФ���롣
 *	ɬ�פ˱����ơ������ƥ��ͭ�ξ�����ղä��Ƥ⤤�����ȡ�
 */

int	stateload_system( void )
{
  return TRUE;
}
int	statesave_system( void )
{
  return TRUE;
}



/***********************************************************************
 * ��˥塼���̤�ɽ�����롢�����ƥ��ͭ��å�����
 ************************************************************************/

static int about_lang;
static int about_line;

/*	���� japanese �����ʤ顢���ܸ�ɽ�����׵�
 *	���� ���������� 0��2��-1�ʤ����ʤ�
 */
int	about_msg_init( int japanese )
{
  about_lang = ( japanese ) ? 1 : 0;
  about_line = 0;

  return -1;		/* ʸ�������ɻ���ʤ� */
}

/*	��å�������ʸ������֤�
 *	����ʾ��å�������̵�����ϡ�NULL
 */
const char *about_msg( void )
{
  static const char *about_en[] =
  {
#ifdef	MITSHM
    "MIT-SHM ... Supported",
#endif  

#ifdef	USE_DGA
    "X11 DGA ... Supported",
#endif

#if	defined( JOY_NOTHING )

    "JOYSTICK ... Not supported",

#elif	defined( JOY_SDL )

    "JOYSTICK (SDL) ... Supported",

#elif	defined( JOY_LINUX_USB )

    "JOYSTICK (Linux USB-joystick) ... Supported",

#elif	defined( JOY_LINUX_USB )

    "JOYSTICK (BSD USB-joystick) ... Supported",
#endif  
  };


  static const char *about_jp[] =
  {
#ifdef	MITSHM
    "MIT-SHM �����ݡ��Ȥ���Ƥ��ޤ�",
#endif  

#ifdef	USE_DGA
    "X11 DGA �����ݡ��Ȥ���Ƥ��ޤ�",
#endif

#if	defined( JOY_NOTHING )

    "���祤���ƥ��å� �ϥ��ݡ��Ȥ���Ƥ��ޤ���",

#elif	defined( JOY_SDL )

    "���祤���ƥ��å� (SDL) �����ݡ��Ȥ���Ƥ��ޤ�",

#elif	defined( JOY_LINUX_USB )

    "���祤���ƥ��å� (Linux USB-joystick) �����ݡ��Ȥ���Ƥ��ޤ�",

#elif	defined( JOY_LINUX_USB )

    "���祤���ƥ��å� (BSD USB-joystick) �����ݡ��Ȥ���Ƥ��ޤ�",
#endif  
  };


  if( about_lang ){
    if( about_line >= COUNTOF(about_jp) ) return NULL;
    else                                  return about_jp[ about_line ++ ];
  }else{
    if( about_line >= COUNTOF(about_en) ) return NULL;
    else                                  return about_en[ about_line ++ ];
  }
}
