/************************************************************************/
/* ���ߡ��� ���祤���ƥ��å����Ͻ���					*/
/*									*/
/*	���Υե�����ϡ� joystick.c ���饤�󥯥롼�ɤ���ޤ�		*/
/*									*/
/************************************************************************/
#if	defined( JOY_NOTHING )


#include <stdio.h>

#include "quasi88.h"
#include "device.h"
#include "keyboard.h"
#include "joystick.h"
#include "event.h"

int	enable_joystick = FALSE;	/* ���祤���ƥ��å��λ��Ѳ��� */



void	joystick_init( void )
{
  if( use_joydevice ){
    if( verbose_proc ) printf( "\nJoystick not supported\n" );
  }
  enable_joystick = FALSE;
}

void	joystick_term( void )
{
}

void	joystick_event( void )
{
}


#endif
