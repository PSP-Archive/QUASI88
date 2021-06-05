/***********************************************************************
 * ���٥�Ƚ��� (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� event.h ����
 ************************************************************************/

/* ----------------------------------------------------------------------
 *		�إå��ե�������
 * ---------------------------------------------------------------------- */

#include <pspctrl.h>

#include <string.h>
#include <ctype.h>


#include "quasi88.h"
#include "keyboard.h"

#include "graph.h"	/* now_screen_size		*/

#include "drive.h"

#include "emu.h"
#include "device.h"
#include "screen.h"
#include "event.h"

#include "ptk_menu.h"
#include "ptk_softkey.h"
#include "ptk_popup.h"

extern int pmenu_bgrun;

int	use_joydevice = TRUE;		/* ���祤���ƥ��å��ǥХ����򳫤�? */

/******************************************************************************
 * ���٥�ȥϥ�ɥ��
 *
 *	1/60��˸ƤӽФ���롣
 *****************************************************************************/

/*
 * ����� ��ư����1������ƤФ��
 */
void	event_handle_init( void )
{

  function_f[ 11 ] = FN_STATUS;
  function_f[ 12 ] = FN_MENU;

  /*
    TODO 
      �ե����뤫�饭���Х���ǥ��󥰤��ɤ߹���
      �ե����뤫�饽�ե�NumLock�ΥХ���ǥ��󥰤��ɤ߹���
   */
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

}

int event_lock_conv(int keycode)
{
	switch(keycode)
	{
		case KEY88_SHIFTL:
		case KEY88_SHIFTR:
			return KEY88_SHIFT;
		break;
	}
	return keycode;
}

void event_anakey(int mask,int idx)
{
	if (state_akey_swkbd() & mask)
	{
		if (current_akey_swkbd() & mask)
			do_key_press(pmenu_kctbl[idx]);
		else
			do_key_release(pmenu_kctbl[idx]);
		
		set_stat_akey_swkbd(mask);
	}
}


long evt_oldBtn = 0;

/*
 * �� 1/60 ��˸ƤФ��
 */
void	event_handle( void )
{
	int i,code;
	SceCtrlData pad;
	
	
	sceCtrlReadBufferPositive(&pad,1);
	
	pop_input(pad.Buttons);
	pmenu_input(pad.Buttons);
	input_swkbd(pad.Buttons);
	
	input_akey_swkbd(pad.Lx,pad.Ly);
	
	
	if (pmenu_status())
	{
		if (pmenu_bgrun)
		{
			if (get_emu_mode() == PAUSE)
			set_emu_mode( EXEC );
		} 
		else
		{
			if (get_emu_mode() != PAUSE)
				set_emu_mode( PAUSE );
			else
			{
				put_image_all();
			}
		}
	} else
	{
		if (get_emu_mode() == PAUSE)
			set_emu_mode( EXEC );
	}
	
	
	if (status_swkbd())
	{
		if (is_change_swkbd())
			do_key_release(getrelcode_swkbd());
		
		if (is_change_squ_swkbd() && !is_lock_swkbd(KEY88_RETURN))
		{
			if (read_squ_swkbd())
				do_key_press( KEY88_RETURN );
			else
				do_key_release( KEY88_RETURN );
		}
		
		if (is_change_tri_swkbd() && !is_lock_swkbd(KEY88_SHIFTL) && !is_lock_swkbd(KEY88_SHIFTR) )
		{
			if (read_tri_swkbd())
				do_key_press( KEY88_SHIFT );
			else
				do_key_release( KEY88_SHIFT );
		}
		
		if (is_change_cro_swkbd())
		{
			if (read_cro_swkbd())
				change_curcode_swkbd();
		}
		
		for (i=0; i < 16; i++)
		{
			code = need_unlock_swkbd();
			if (code>= 0)
			{
			do_key_release( code );	
			set_ok_swkbd( code );		
			}
		}

		for (i=0; i < 16; i++)
		{
			code = need_lock_swkbd();
			if (code>= 0)
			{
			do_key_press( code );
			set_ok_swkbd( code );		
			}
		}

		if (is_press_swkbd())
		{
			code = getcode_swkbd();
			if (code >= 0 && !is_lock_swkbd(code))
				do_key_press( code);
		} 
		
	}
	else
	{
	
		if (pmenu_kc_aflag)
		{
			event_anakey(KBD_ANA_LEFT ,PMENU_ALE);
			event_anakey(KBD_ANA_RIGHT,PMENU_ARI);
			event_anakey(KBD_ANA_UP   ,PMENU_AUP);
			event_anakey(KBD_ANA_DOWN ,PMENU_ADW);
		}
		if (pad.Buttons ^ evt_oldBtn)
		{

			evt_oldBtn = pad.Buttons;
			
			if (pad.Buttons & PSP_CTRL_LEFT)
				do_key_press(pmenu_kctbl[PMENU_LE]);
			else
				do_key_release(pmenu_kctbl[PMENU_LE]);
			

			if (pad.Buttons & PSP_CTRL_RIGHT)
				do_key_press(pmenu_kctbl[PMENU_RI]);
			else
				do_key_release(pmenu_kctbl[PMENU_RI]);

			if (pad.Buttons & PSP_CTRL_UP)
				do_key_press(pmenu_kctbl[PMENU_UP]);
			else
				do_key_release(pmenu_kctbl[PMENU_UP]);
				
			if (pad.Buttons & PSP_CTRL_DOWN)
				do_key_press(pmenu_kctbl[PMENU_DW]);
			else
				do_key_release(pmenu_kctbl[PMENU_DW]);
				
			
			if (pad.Buttons & PSP_CTRL_CIRCLE)
				do_key_press(pmenu_kctbl[PMENU_A]);
			else
				do_key_release(pmenu_kctbl[PMENU_A]);


			if (pad.Buttons & PSP_CTRL_CROSS)
				do_key_press(pmenu_kctbl[PMENU_B]);
			else
				do_key_release(pmenu_kctbl[PMENU_B]);

			if (pad.Buttons & PSP_CTRL_SQUARE)
				do_key_press(pmenu_kctbl[PMENU_C]);
			else
				do_key_release(pmenu_kctbl[PMENU_C]);

			if (pad.Buttons & PSP_CTRL_TRIANGLE)
				do_key_press(pmenu_kctbl[PMENU_D]);
			else
				do_key_release(pmenu_kctbl[PMENU_D]);

			if (pad.Buttons & PSP_CTRL_START)
				do_key_press(pmenu_kctbl[PMENU_E]);
			else
				do_key_release(pmenu_kctbl[PMENU_E]);

			if (pad.Buttons & PSP_CTRL_START)
				do_key_press(pmenu_kctbl[PMENU_F]);
			else
				do_key_release(pmenu_kctbl[PMENU_F]);
		}
	}
	
}



/***********************************************************************
 * ���ߤΥޥ�����ɸ�����ؿ�
 *
 *	���ߤΥޥ��������к�ɸ�� *x, *y �˥��å�
 ************************************************************************/

void	init_mouse_position( int *x, int *y )
{
  int win_x, win_y;

  win_x = win_y = 0;

  *x = win_x;
  *y = win_y;
}




/******************************************************************************
 * ���եȥ����� NumLock ͭ����̵��
 *
 *	TODO  ����ե�����ʤɤǡ�Ǥ�դ����֤Ǥ���Ȥ����ʤ�
 *****************************************************************************/

INLINE	void	numlock_setup( int enable )
{
}

int	numlock_on ( void ){ numlock_setup( TRUE );  return TRUE; }
void	numlock_off( void ){ numlock_setup( FALSE ); }



/******************************************************************************
 * ���ߥ�졼�ȡ���˥塼���ݡ�������˥����⡼�� �� ���ϻ��ν���
 *
 *****************************************************************************/

void	event_init( void )
{
  /* �ޥ���ɽ��������֤����� (�Ĥ��Ǥ˥�����ԡ��Ȥ�) */
  set_mouse_state();
}



/******************************************************************************
 * ���祤���ƥ��å�
 *
 * ���٤����ν���������ʥ�����ɥ��Ȥ����ˤˤơ������ץ󤹤롣
 *****************************************************************************/

int	joy_init( void )
{
 return FALSE; 
}



int	joystick_available( void )
{
 return FALSE;
}
