/************************************************************************/
/*									*/
/* ��������Ĵ���Ѵؿ� (OS��¸)						*/
/*									*/
/* �ڴؿ���								*/
/*									*/
/* int  wait_vsync_init( void )		����� (��ư���˸ƤӽФ����)	*/
/* void wait_vsync_term( void )		��λ   (��λ���˸ƤӽФ����)	*/
/*									*/
/* void	wait_vsync_reset( void )	��¬�ꥻ�å� (�����ѹ���)	*/
/* void wait_vsync( void )		�������Ȥ��� (����ֳַв��Ԥ�)	*/
/*									*/
/* void wait_menu( void )		��˥塼�ѥ�������(1/60sec�Ԥ�)	*/
/*									*/
/************************************************************************/
#include <stdio.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"
#include "suspend.h"

#include "screen.h"	/* auto_skip... */

#include <pspgu.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <pspkernel.h>

/*
 * ��ư�ե졼�ॹ���å�		( by floi, thanks ! )
 */

static	int	skip_counter = 0;		/* Ϣ³���󥹥��åפ����� */
static	int	skip_count_max = 15;		/* ����ʾ�Ϣ³�����åפ�����
						   ��ö������Ū�����褹�� */


/*
 * �������Ƚ����ؿ���
 */

static	int	wait_counter = 0;		/* Ϣ³������֥����С�������*/
static	int	wait_count_max = 10;		/* ����ʾ�Ϣ³�����С�������
						   ��ö,����Ĵ������������ */

/* ����Ĵ���ϡ�usñ�̤ǹԤʤ����Ǥ��ѿ��η��� long �ʤΤ� 4295 �ä��ͤ�
   ��ä�(wrap)���ޤ������λ��� 1�ե졼��ϥ����ߥ󥰤�������
   �ʤΤǡ���ǽ�ʤ�� 64bit��(long long)�ˤ��Ƥߤ褦�� */

#ifdef SDL_HAS_64BIT_TYPE
typedef	Sint64		T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* ���ե졼��λ��� */
static	T_WAIT_TICK	delta_time;		/* 1 �ե졼��λ��� */



/* ---- ���߻����������� (usecñ��) ---- */

#define	GET_TICK()	( (T_WAIT_TICK)SDL_GetTicks() * 1000 )


u64 wait_lasttick;
u32 wait_tickperframe;



/****************************************************************************
 * �������Ƚ��������
 *****************************************************************************/
int	wait_vsync_init( void )
{
  skip_counter = 0;
  sceRtcGetCurrentTick(&wait_lasttick);
  wait_tickperframe = sceRtcGetTickResolution() / 60;

  return TRUE;
}



/****************************************************************************
 * �������Ƚ�����λ
 *****************************************************************************/
void	wait_vsync_term( void )
{
}



/****************************************************************************
 * �������Ƚ����ƽ����
 *****************************************************************************/
void	wait_vsync_reset( void )
{
}



/****************************************************************************
 * �������Ƚ���
 *****************************************************************************/
void	wait_vsync( void )
{
  u64 curtick;
  
  sceRtcGetCurrentTick(&curtick);

  if (curtick < wait_lasttick + wait_tickperframe)
  {
	skip_counter = 0;
	while(curtick < wait_lasttick + wait_tickperframe)
	  sceRtcGetCurrentTick(&curtick);


	do_skip_draw = FALSE;
  } else
  {
    if (skip_counter < skip_count_max)
	{
		skip_counter++;
		do_skip_draw = TRUE;
	} else
	{
		skip_counter = 0;
		do_skip_draw = FALSE;
		already_skip_draw = FALSE;
		reset_frame_counter();	
	}
  }
  wait_lasttick = curtick;
  

}



/****************************************************************************
 * ��˥塼�ѤΥ�������
 *	�� 1/60 �äۤ��Ԥġ����٤����פ�������ǽ�ʤ�ɬ�� sleep ������٤�
 *****************************************************************************/
void	wait_menu( void )
{
	sceDisplayWaitVblankStart();
}
