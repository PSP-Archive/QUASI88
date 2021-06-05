/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "quasi88.h"
#include "initval.h"

#include "file-op.h"
#include "getconf.h"

#include "device.h"
#include "snddrv.h"	/* xmame_system_init(), xmame_system_term() */
#include "suspend.h"
#include "menu.h"


static	void	the_end( void );

int	main( int argc, char *argv[] )
{
  int	x = 1;

	/* ����ǥ�����ͥ������å� */

#ifdef LSB_FIRST
  if( *(char *)&x != 1 ){
    fprintf( stderr,
	     "%s CAN'T EXCUTE !\n"
	     "This machine is Big-Endian.\n"
	     "Compile again comment-out 'LSB_FIRST = 1' in Makefile.\n",
	     argv[0] );
    return -1;
  }
#else
  if( *(char *)&x == 1 ){
    fprintf( stderr,
	     "%s CAN'T EXCUTE !\n"
	     "This machine is Little-Endian.\n"
	     "Compile again comment-in 'LSB_FIRST = 1' in Makefile.\n",
	     argv[0] );
    return -1;
  }
#endif


  sdl_system_init();		/* SDL��Ϣ�ν���� */

  xmame_system_init();		/* XMAME������ɴ�Ϣ����� (��������������!) */

  quasi88_atexit( the_end );


  if( osd_environment() ){		/* �Ķ�����	*/

    if( config_init( argc, argv ) ){	/* ��������	*/

      quasi88();			/* PC-8801 ���ߥ�졼����� */
    }
  }


  xmame_system_term();		/* XMAME������ɴ�Ϣ����� */

  sdl_system_term();		/* SDL��Ϣ����� */

  return 0;
}



/*
 * ͽ��������λ��
 */
static	void	the_end( void )
{
  xmame_system_term();		/* XMAME������ɴ�Ϣ����� */
  sdl_system_term();		/* SDL��Ϣ����� */
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
#ifdef	USE_SOUND

const char *about_msg( void )
{
  static const char *about_en[] =
  {
#ifdef	USE_SOUND
    "(Caution) Volume control is not supported.",
#endif
  };


  static const char *about_jp[] =
  {
#ifdef	USE_SOUND
    "(���) �����ѹ���ǽ�ϥ��ݡ��Ȥ���Ƥ��ޤ��� ",
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

#else
const char *about_msg( void ){ return NULL; }
#endif
