/************************************************************************/
/*									*/
/* �⡼�ɤ˱����ơ������ؿ���ƤӽФ���					*/
/*	�⡼�ɤϡ��¹�(EXEC)����˥塼(MENU)����˥���(MONITOR)��	*/
/*	���(PAUSE) ���礭���狼��롣					*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "initval.h"
#include "emu.h"

#include "pc88cpu.h"

#include "screen.h"
#include "keyboard.h"
#include "intr.h"
#include "event.h"
#include "menu.h"
#include "monitor.h"
#include "pause.h"
#include "wait.h"
#include "suspend.h"
#include "status.h"
#include "graph.h"
#include "snddrv.h"




break_t		break_point[2][NR_BP];	/* �֥졼���ݥ����		*/
break_drive_t	break_point_fdc[NR_BP];	/* FDC �֥졼���ݥ����		*/


int	cpu_timing	= DEFAULT_CPU;		/* SUB-CPU ��ư����	*/

int	select_main_cpu = TRUE;			/* -cpu 0 �¹Ԥ���CPU	*/
						/* ���ʤ� MAIN CPU��¹�*/

int	dual_cpu_count	= 0;			/* -cpu 1 Ʊ������STEP��*/
int	CPU_1_COUNT	= 4000;			/* ���Ρ������		*/

int	cpu_slice_us    = 5;			/* -cpu 2 ������ʬ��(us)*/
						/* 10>��SILPHEED��ư����*/

int	trace_counter	= 1;			/* TRACE ���Υ�����	*/


static	int	main_state   = 0;
static	int	sub_state    = 0;
#define	JACKUP	(256)


static	int	emu_mode	= EXEC;		/* ���ߥ�졼����������	*/
static	int	emu_mode_execute= GO;
static	int	emu_mode_next	= EXEC;
static	int	emu_rest_step;

void	set_emu_mode( int mode )
{
  if( mode == EXEC ) mode = GO;

  if( mode == GO      ||
      mode == STEP    ||
      mode == TRACE   ||
      mode == TRACE_CHANGE ){

    emu_mode_next    = EXEC;
    emu_mode_execute = mode;

  }else{

    emu_mode_next    = mode;
    emu_rest_step    = 1;

  }

  CPU_BREAKOFF();
}

int	get_emu_mode( void )
{
  return emu_mode;
}

int	next_emu_mode( void )
{
  return emu_mode_next;
}




/***********************************************************************
 * ���ߥ�졼�Ƚ���������
 *	�¹� / ��˥塼 / ��˥��� / �����ߤ����ؤ�
 *
 ************************************************************************/

static	void	emu_init( void );
static	void	emu_main( void );

void	emu_reset( void )
{
  select_main_cpu = TRUE;
  dual_cpu_count  = 0;

  main_state   = 0;
  sub_state    = 0;
}


void	emu( void )
{
  int	i, j;
	/* �֥졼���ݥ���ȤΥ������� (��˥����⡼����) */
  for( j=0; j<2; j++ )
    for( i=0; i<NR_BP; i++ )
      break_point[j][i].type = BP_NONE;

  for( i=0; i<NR_BP; i++ )
    break_point_fdc[i].type = BP_NONE;



  for( ;; ){

    if( emu_mode != emu_mode_next ){
      emu_mode = emu_mode_next;
      if( emu_mode == EXEC ){
	emu_init();
      }
    }

    wait_vsync_reset();		/* �⡼�����ؤκǽ�ˡ����ַ�¬������ */

    switch( emu_mode ){
    case EXEC:		/* ------------------------------ CPU��¹Ԥ���	*/
      set_window_title( Q_TITLE " ver " Q_VERSION );
      xmame_sound_resume();
      event_init();
      keyboard_start();
      emu_main();
      break;

    case MONITOR:	/* ----------------------------- ��˥����⡼��	*/
#ifdef	USE_MONITOR
      set_window_title( Q_TITLE " (MONITOR)" );
      xmame_sound_suspend();
      event_init();
      keyboard_stop();
      monitor_main();
#else
      set_emu_mode( PAUSE );
#endif
      break;

    case MENU:		/* ----------------------------- ��˥塼�⡼��	*/
      set_window_title( Q_TITLE " ver " Q_VERSION );
      xmame_sound_suspend();
      event_init();
      keyboard_stop();
      menu_main();
      break;

    case PAUSE:		/* --------------------------------- ��������	*/
      set_window_title( Q_TITLE " (PAUSE)" );
      xmame_sound_suspend();
      event_init();
      keyboard_stop();
      pause_main();
      break;

    case QUIT:		/* --------------------------------------- ��λ	*/
      return;
    }

  }
}



/***********************************************************************
 * CPU�¹Խ��� (EXEC) ������
 *	-cpu <n> �˱����ơ�ư����Ѥ��롣
 *
 *	STEP  ���ϡ�1step �����¹Ԥ��롣
 *	TRACE ���ϡ�������ʬ��1step �¹Ԥ��롣
 *
 *	�֥졼���ݥ���Ȼ�����ϡ�1step�¹Ԥ��٤� PC ���֥졼���ݥ���Ȥ�
 *	ã�������ɤ������ǧ���롣
 *
 ************************************************************************/

#define	INFINITY	(0)
#define	ONLY_1STEP	(1)

/*------------------------------------------------------------------------*/

/*
 * �֥졼���ݥ���� (������ PC) ��̵ͭ������å�����
 */

INLINE	int	check_break_point_PC( void )
{
  int	i, j;

  for( i=0; i<NR_BP; i++ ) if( break_point[BP_MAIN][i].type == BP_PC ) break;
  for( j=0; j<NR_BP; j++ ) if( break_point[BP_SUB][j].type  == BP_PC ) break;

  if( i==NR_BP && j==NR_BP ) return FALSE;
  else                       return TRUE;
}

/*------------------------------------------------------------------------*/

/*
 * CPU �� 1step �¹Ԥ��ơ�PC���֥졼���ݥ���Ȥ�ã�����������å�����
 *	�֥졼���ݥ����(������PC)̤����ʤ餳�δؿ��ϻȤ鷺��z80_emu()��Ȥ�
 */

static	int	z80_emu_with_breakpoint( z80arch *z80, int unused )
{
  int i, cpu, states;

  states = z80_emu( z80, 1 );		/* 1step �����¹� */

  if( z80==&z80main_cpu ) cpu = BP_MAIN;
  else                    cpu = BP_SUB;

  for( i=0; i<NR_BP; i++ ){
    if( break_point[cpu][i].type == BP_PC     &&
	break_point[cpu][i].addr == z80->PC.W ){

      if( i==BP_NUM_FOR_SYSTEM ){
	break_point[cpu][i].type = BP_NONE;
      }

      printf( "*** Break at %04x *** ( %s[#%d] : PC )\n",
	      z80->PC.W, (cpu==BP_MAIN)?"MAIN":"SUB", i+1 );
      z80_debug( &z80main_cpu, 
		 (cpu==BP_MAIN) ? "[MAIN CPU]\n" : "[SUB CPU]\n" );

      set_emu_mode( MONITOR );
    }							
  }							

  return states;
}

/*---------------------------------------------------------------------------*/
static	void	emu_init( void )
{
  status_message( 0, -1, NULL );
  status_message( 1, -1, NULL );
  status_message( 2, -1, NULL );

  draw_screen_force();
}
/*---------------------------------------------------------------------------*/

static	void	emu_main( void )
{
  int	passed_step;		/* �¹Ԥ��� step�� */
  int	target_step;		/* ���� step����ã����ޤǼ¹Ԥ��� */

  int	infinity, only_1step;

  int	wk;
  int	(*z80_exec)( z80arch *, int );


	/* �֥졼���ݥ���������̵ͭ�ǡ��ƤӽФ��ؿ����Ѥ��� */
  if( check_break_point_PC() ) z80_exec = z80_emu_with_breakpoint;
  else                         z80_exec = z80_emu;


	/* GO/TRACE/STEP/CHANGE �˱����ƽ����η����֤��������� */

  passed_step = 0;

  switch( emu_mode_execute ){
  default:
  case GO:
    target_step = 0;			/* ̵�¤˼¹� */
    infinity    = INFINITY;
    only_1step  = ONLY_1STEP;
    break;

  case TRACE:
    target_step = trace_counter;	/* ���ꥹ�ƥå׿��¹� */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;

  case STEP:
    target_step = 1;			/* 1���ƥå׼¹� */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;

  case TRACE_CHANGE:
    target_step = 0;			/* ̵�¤˼¹� */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;
  }


  /* �¹Ԥ���Ĥꥹ�ƥå׿���
	TRACE / STEP �λ��ϡ����ꤵ�줿���ƥå׿���
	GO / TRACE_CHANGE �ʤ� ̵�¤ʤΤǡ� 0��
		�ʤ�������ǥ�˥塼�����ܤ�����硢����Ū�� 1 �����åȤ���롣
		����ˤ��̵�¤˽���������Ǥ⡢�롼�פ�ȴ����褦�ˤʤ롣 */
  emu_rest_step = target_step;




  switch( emu_mode_execute ){

  /*------------------------------------------------------------------------*/
  case GO:				/* �Ҥ�����¹Ԥ���           */
  case TRACE:				/* ���ꤷ�����ƥåס��¹Ԥ��� */
  case STEP:				/* 1���ƥåפ������¹Ԥ���    */

    switch( cpu_timing ){

    case 0:		/* select_main_cpu �ǻ��ꤵ�줿�ۤ���CPU��̵�¼¹� */
      for(;;){
	if( select_main_cpu ) (z80_exec)( &z80main_cpu, infinity );
	else                  (z80_exec)( &z80sub_cpu,  infinity );
	if( emu_rest_step ){
	  passed_step ++;
	  if( -- emu_rest_step <= 0 ) break;
	}
      }
      break;

    case 1:		/* dual_cpu_count==0 �ʤ�ᥤ��CPU��̵�¼¹ԡ�*/
			/*               !=0 �ʤ�ᥤ�󥵥֤��߼¹� */
      for(;;){
	if( dual_cpu_count==0 ) (z80_exec)( &z80main_cpu, infinity   );
	else{
	  (z80_exec)( &z80main_cpu, only_1step );
	  (z80_exec)( &z80sub_cpu,  only_1step );
	  dual_cpu_count --;
	}
	if( emu_rest_step ){
	  passed_step ++;
	  if( -- emu_rest_step <= 0 ) break;
	}
      }
      break;

    case 2:		/* �ᥤ��CPU������CPU���ߤ� 5us ���ļ¹� */
      for(;;){
	if( main_state < 1*JACKUP  &&  sub_state < 1*JACKUP ){
	  main_state += (cpu_clock_mhz * cpu_slice_us) * JACKUP;
	  sub_state  += (3.9936        * cpu_slice_us) * JACKUP;
	}
	if( main_state >= 1*JACKUP ){
	  wk = (infinity==INFINITY) ? main_state/JACKUP : ONLY_1STEP;
	  main_state -= (z80_exec( &z80main_cpu, wk ) ) * JACKUP;
	}
	if( sub_state >= 1*JACKUP ){
	  wk = (infinity==INFINITY) ? sub_state/JACKUP : ONLY_1STEP;
	  sub_state  -= (z80_exec( &z80sub_cpu, wk ) ) * JACKUP;
	}
	if( emu_rest_step ){
	  passed_step ++;
	  if( -- emu_rest_step <= 0 ) break;
	}
      }
      break;

    }

	/* GO �ξ�硢��˥塼���ܤʤɤǤ�����ȴ���Ƥ��� */
	/* ����¾�ξ�硢���ꥹ�ƥå׿������������⡢������ȴ���Ƥ��� */

    if( emu_mode_execute != GO ){
      if( passed_step >= target_step ){
				/* ���ꥹ�ƥå׼¹Դ�λ�����顢�쥸����ɽ�� */
	switch( cpu_timing ){
	case 0:
	  if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	  break;

	case 1:
	                       z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  if( dual_cpu_count ) z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	  break;

	case 2:
	  z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	  break;
	}

	set_emu_mode( MONITOR );
      }

    }
    break;

  /*------------------------------------------------------------------------*/
    case TRACE_CHANGE:			/* CPU���ڤ��ؤ��ޤǽ����򤹤� */
      if( cpu_timing >= 1 ){
	printf( "command 'trace change' can use when -cpu 0\n");
	set_emu_mode( MONITOR );
	break;
      }

      wk = select_main_cpu;
      while( wk==select_main_cpu ){
	if( select_main_cpu ) (z80_exec)( &z80main_cpu, infinity );
	else                  (z80_exec)( &z80sub_cpu,  infinity );
	if( emu_rest_step ){
	  passed_step ++;
	  if( -- emu_rest_step <= 0 ) break;
	}
      }
      if( wk != select_main_cpu ){
	if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
	else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
	set_emu_mode( MONITOR );
      }
      break;
  }
}
















/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"EMU "

static	T_SUSPEND_W	suspend_emu_work[] =
{
  { TYPE_INT,	&cpu_timing,		},
  { TYPE_INT,	&select_main_cpu,	},
  { TYPE_INT,	&dual_cpu_count,	},
  { TYPE_INT,	&CPU_1_COUNT,		},
  { TYPE_INT,	&cpu_slice_us,		},
  { TYPE_INT,	&main_state,		},
  { TYPE_INT,	&sub_state,		},

  { TYPE_END,	0			},
};


int	statesave_emu( void )
{
  if( statesave_table( SID, suspend_emu_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}

int	stateload_emu( void )
{
  if( stateload_table( SID, suspend_emu_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;

  /* �꡼�ɸ��ɬ�����ߥ�����ܤǤ����Τ� ? */
  set_emu_mode( EXEC );
}
