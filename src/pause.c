/************************************************************************/
/*									*/
/* �����߽��� (OS��¸)						*/
/*									*/
/*	�ѿ� pause_by_focus_out �ˤ��������Ѥ��			*/
/*	��pause_by_focus_out == 0 �λ�					*/
/*		ESC���������Ȳ����	��������� PAUSE��ɽ��		*/
/*	��pause_by_focus_out != 0 �λ�					*/
/*		X �Υޥ����������������Ȳ��				*/
/*									*/
/* �ڴؿ���								*/
/* void pause_init( void )						*/
/*									*/
/* void pause_main( void )						*/
/*									*/
/************************************************************************/
#include <stdio.h>

#include "quasi88.h"
#include "pause.h"

#include "emu.h"
#include "initval.h"
#include "status.h"
#include "screen.h"
#include "wait.h"
#include "event.h"


int	need_focus = FALSE;			/* �ե���������������ߤ��� */


static	int	pause_by_focus_out = FALSE;

/*
 * ���ߥ������ˡ��ե���������̵���ʤä� (-focus������ϡ��ݡ�������)
 */
void	pause_event_focus_out_when_exec( void )
{
  if( need_focus ){				/* -focus ������� */
    pause_by_focus_out = TRUE;
    set_emu_mode( PAUSE );			/* ������ PAUSE ���� */
  }
}

/*
 * �ݡ�����ˡ��ե�������������
 */
void	pause_event_focus_in_when_pause( void )
{
  if( pause_by_focus_out ){
    set_emu_mode( GO );
  }
}

/*
 * �ݡ�����ˡ��ݡ�����λ�Υ���(ESC����)�������Τ���
 */
void	pause_event_key_on_esc( void )
{
  set_emu_mode( GO );
}

/*
 * �ݡ�����ˡ���˥塼���ϤΥ����������Τ���
 */
void	pause_event_key_on_menu( void )
{
  set_emu_mode( MENU );
}









static	void	pause_init( void )
{
  status_message( 0, 0, " PAUSE " );
  status_message( 1, 0, "<ESC> key to return" );
  status_message( 2, 0, NULL );
  draw_screen_force();
}


void	pause_main( void )
{
  pause_init();

  while( next_emu_mode() == PAUSE ){

    event_handle();	/* ���٥�Ȥ�������� */
    wait_menu();	/* ���Ф������Ԥ�     */

  }

  pause_by_focus_out = FALSE;
}
