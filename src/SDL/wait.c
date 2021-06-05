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

#include <SDL.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"
#include "suspend.h"

#include "screen.h"	/* auto_skip... */



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





/****************************************************************************
 * �������Ƚ��������
 *****************************************************************************/
int	wait_vsync_init( void )
{
  if( ! SDL_WasInit( SDL_INIT_TIMER ) ){
    if( SDL_InitSubSystem( SDL_INIT_TIMER ) != 0 ){
      if( verbose_wait ) printf( "Error Wait (SDL)\n" );
      return FALSE;
    }
  }

  wait_vsync_reset();
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
  wait_counter = 0;

  delta_time = (T_WAIT_TICK)(1000000.0/( CONST_VSYNC_FREQ * wait_rate / 100 ));
  next_time  = GET_TICK() + delta_time;

  /* delta_time >= 1000000us (1sec) �ˤʤ�ȡ�����äȤޤ��� */
}



/****************************************************************************
 * �������Ƚ���
 *****************************************************************************/
void	wait_vsync( void )
{
  int	on_time = FALSE;
  int	slept   = FALSE;
  T_WAIT_TICK	diff_ms;


  diff_ms = ( next_time - GET_TICK() ) / 1000;

  if( diff_ms > 0 ){	    /* �ޤ����֤�;�äƤ���ʤ� */
			    /* diff_ms �ߥ��á��������� */

    if( wait_by_sleep ){	/* ���֤����ޤ� sleep ������ */

#if 1	/* ��ˡ 1) */
      SDL_Delay( diff_ms );		/* diff_ms �ߥ��á��ǥ��쥤   */
      slept = TRUE;

#else	/* ��ˡ 2) */
      if( diff_ms < 10 ){		/* 10ms̤���ʤ�ӥ����������� */
	while( GET_TICK() <= next_time )
	  ;
      }else{				/* 10ms�ʾ�ʤ�ǥ��쥤       */
	SDL_Delay( diff_ms );
	slept = TRUE;
      }
#endif

    }else{			/* ���֤����ޤ� Tick ��ƻ뤹���� */

      while( GET_TICK() <= next_time )
	;
    }

    on_time = TRUE;
  }


  if( slept == FALSE ){		/* ���٤� SDL_Delay ���ʤ��ä���� */
    SDL_Delay( 1 );				/* for AUDIO thread ?? */
  }


  next_time += delta_time;


  if( on_time ){			/* ������˽����Ǥ��� */
    wait_counter = 0;
  }else{				/* ������˽����Ǥ��Ƥ��ʤ� */
    wait_counter ++;
    if( wait_counter >= wait_count_max ){	/* �٤줬�Ҥɤ����� */
      wait_vsync_reset();			/* �������Ȥ�����   */
    }
  }



  /*
   * ��ư�ե졼�ॹ���å׽���		( by floi, thanks ! )
   */
  if( use_auto_skip ){
    if( on_time ){			/* ������˽����Ǥ��� */
      skip_counter = 0;
      do_skip_draw = FALSE;
      if( already_skip_draw ){		/* ��������򥹥��åפ��Ƥ����� */
	already_skip_draw = FALSE;
	reset_frame_counter();		/* ����ɬ�����褹�� */
      }
    }else{				/* ������˽����Ǥ��Ƥ��ʤ� */
      skip_counter++;
      if( skip_counter >= skip_count_max ){	/* �����åפ����� */
	skip_counter = 0;
	do_skip_draw = FALSE;
	already_skip_draw = FALSE;
	reset_frame_counter();			/* ����ɬ�����褹�� */
      }else{
	do_skip_draw = TRUE;			/* ����򥹥��åפ���ɬ��ͭ��*/
      }
    }
  }

  return;
}



/****************************************************************************
 * ��˥塼�ѤΥ�������
 *	�� 1/60 �äۤ��Ԥġ����٤����פ�������ǽ�ʤ�ɬ�� sleep ������٤�
 *****************************************************************************/
void	wait_menu( void )
{
  SDL_Delay( 20 );
}
