/************************************************************************/
/*									*/
/* �����ߤΥ��ߥ�졼��						*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "initval.h"
#include "intr.h"

#include "pc88cpu.h"
#include "pc88main.h"
#include "screen.h"
#include "crtcdmac.h"
#include "keyboard.h"
#include "soundbd.h"

#include "event.h"
#include "emu.h"
#include "snddrv.h"
#include "wait.h"
#include "suspend.h"



int	intr_level;			/* OUT[E4] �����ߥ�٥�	*/
int	intr_priority;			/* OUT[E4] ������ͥ����	*/
int	intr_sio_enable;		/* OUT[E6] �����ߥޥ��� SIO	*/ 
int	intr_vsync_enable;		/* OUT[E6] �����ߥޥ��� VSYNC	*/ 
int	intr_rtc_enable;		/* OUT[E6] �����ߥޥ��� RTC	*/ 



double	cpu_clock_mhz   = DEFAULT_CPU_CLOCK_MHZ;   /* MAIN CPU Clock   [MHz] */
double	sound_clock_mhz = DEFAULT_SOUND_CLOCK_MHZ; /* SOUND chip Clock [MHz] */
double	vsync_freq_hz   = DEFAULT_VSYNC_FREQ_HZ;   /* VSYNC �������   [Hz]  */

int	wait_rate     = 100;			/* ��������Ĵ�� ��Ψ    [%]  */
int	wait_by_sleep = TRUE;			/* ��������Ĵ���� sleep ���� */

long	wait_sleep_min_us = 100;		/* �Ĥ� idle���֤����� us�ʲ���
						   ���ϡ� sleep �������Ԥġ�
						   (MAX 1�� = 1,000,000us) */



#define	CPU_CLOCK_MHZ		cpu_clock_mhz
#define	SOUND_CLOCK_MHZ		sound_clock_mhz
#define	VSYNC_FREQ_HZ		vsync_freq_hz

#if 1			/* VSYNC�����ߤ� VRTC�Υ����ߥ󥰤򤺤餷�Ƥߤ� */

/* 1�饤�� (160���ơ��Ȥ��餤) �������餹 */
#define	VRTC_TOP		((  47 ) /448.0)
#define	VRTC_DISP		(( 400 ) /448.0)
#define	VRTC_BOTTOM		((   1 ) /448.0)

/* 7�饤�󤺤餹�Τ����������Τ� ?
#define	VRTC_TOP		((8 + 33)/448.0)
#define	VRTC_DISP		(( 400 ) /448.0)
#define	VRTC_BOTTOM		((  7  ) /448.0)
*/

#else			/* �ʲ����ͤ� ver0.6.0 �����Υ����ߥ󥰤ˤʤ�Ϥ� */
#define	VRTC_TOP		((  48 ) /448.0)
#define	VRTC_DISP		(( 402 ) /448.0)	/* ǰ�Τ��� +2 ;ʬ��*/
#define	VRTC_BOTTOM		((   0 ) /448.0)
#endif

#define	RTC_FREQ_HZ		(600)

#define	TIMER_A_CONST		(12)
#define	TIMER_B_CONST		(192)

#define	CPU_CLOCK		(CPU_CLOCK_MHZ * 1000000)

/*****************************************************************************/

int	state_of_cpu = 0;	/* �ᥤ��CPU�������������ơ��ȿ�	*/
				/*	VSYNC���ȯ�����˽��������     */
				/*	main_INT_update() �ƽл��˲û�	*/
				/*	 (���δؿ���������˸ƽФ����)	*/
				/*					*/
				/*	VSYNCȯ�����鸽�������˽�������	*/
				/*	���ơ��ȿ��ϡ��ʲ��μ��ˤʤ롣*/
				/*					*/
				/*	state_of_cpu+z80main_cpu.state0 */

int	state_of_vsync;		/* VSYNC 1����������Υ��ơ��ȿ�	*/

int	no_wait	      = FALSE;		/* �������Ȥʤ�			*/


	int	RS232C_flag    = FALSE;	/* RS232C */
static	int	rs232c_intr_base;
static	int	rs232c_intr_timer;

	int	VSYNC_flag     = FALSE;	/* VSYNC */
static	int	vsync_intr_base;
static	int	vsync_intr_timer;

	int	ctrl_vrtc      = 1;	/* VRTC (��ľ������:1or3 / ɽ����:2) */
static	int	vrtc_base;
static	int	vrtc_base2;
static	int	vrtc_timer;

	int	RTC_flag       = FALSE;	/* RTC */
static	int	rtc_intr_base;
static	int	rtc_intr_timer;

	int	SOUND_flag     = FALSE;	/* SOUND Timer-A/Timer-B */
static	int	sd_A_intr_base;
static	int	sd_A_intr_timer;
static	int	sd_B_intr_base;
static	int	sd_B_intr_timer;
static	int	sd2_BRDY_intr_base;
static	int	sd2_BRDY_intr_timer;
static	int	sd2_EOS_intr_base;
static	int	sd2_EOS_intr_timer;




/*------------------------------------------------------
 * �����ޡ������ߥ��ߥ�졼�ȤΥ��������
 *	VSYNC / VRTC / RTC         ����ϵ�ư���˽����
 *	RS232C / Timer-A / TImer-B �����������˽����
 *------------------------------------------------------*/

/*
 * �����ߥߥ�졼�Ƚ���� �� Z80 �ε�ư���˸Ƥ�
 */
static	void	interval_work_init_generic( void )
{
  vsync_intr_timer = vsync_intr_base = CPU_CLOCK / VSYNC_FREQ_HZ;
  vrtc_timer       = vrtc_base       = (vsync_intr_base * VRTC_TOP);
                     vrtc_base2      = (vsync_intr_base * VRTC_DISP);

  rtc_intr_timer   = rtc_intr_base   = CPU_CLOCK / RTC_FREQ_HZ;

  state_of_vsync = vsync_intr_base;
  state_of_cpu   = 0;
}


/*
 * RS232C�����ߥ��ߥ�졼�Ƚ���� �� Z80��ư���˸Ƥ�
 */
static	void	interval_work_init_RS232C( void )
{
  interval_work_set_RS232C( 0, 0 );
}

/*
 * ������ɳ����ߥ��ߥ�졼�Ƚ���� �� Z80��ư���˸Ƥ�
 */
static	void	interval_work_init_TIMER_A( void )
{
  interval_work_set_TIMER_A();
  sd_A_intr_timer = sd_A_intr_base;
}
static	void	interval_work_init_TIMER_B( void )
{
  interval_work_set_TIMER_B();
  sd_B_intr_timer = sd_B_intr_base;
}




/*
 * �������ߥ��ߥ�졼�Ƚ���� �� Z80��ư���䡢����å��ѹ����ʤɤ˸Ƥ�
 */
void	interval_work_init_all( void )
{
  interval_work_init_generic();
  interval_work_init_RS232C();
  interval_work_init_TIMER_A();
  interval_work_init_TIMER_B();
}



/*
 * RS232C�����ߥ��ߥ�졼�Ⱥƽ���� �� RS232C���������˸Ƥ�
 */
void	interval_work_set_RS232C( int bps, int framesize )
{
  if( bps == 0 || framesize == 0 ){
    rs232c_intr_base = 0x7fffffff;
  }else{
    rs232c_intr_base = CPU_CLOCK / ( (double)bps / (double)framesize );
    if( rs232c_intr_base < 100 ) rs232c_intr_base = 100;
  }
  rs232c_intr_timer = rs232c_intr_base;
}




/************************************************************************/
/* �����ߤ˴ؤ�� ������ɥ쥸�����������ν���			*/
/************************************************************************/
/*
 * ������ɤΥ����ޥץꥻ�å��� �ѹ����˸Ƥ�
 */
void	interval_work_set_TIMER_A( void )
{
  sd_A_intr_base  = TIMER_A_CONST * sound_prescaler * (1024 - sound_TIMER_A)
					* ( CPU_CLOCK_MHZ / SOUND_CLOCK_MHZ );
}
void	interval_work_set_TIMER_B( void )
{
  sd_B_intr_base  = TIMER_B_CONST * sound_prescaler * (256 - sound_TIMER_B)
					* ( CPU_CLOCK_MHZ / SOUND_CLOCK_MHZ );
}

/*
 * ������ɤ� �Ƽ�ե饰 ����� �ץꥹ�����顼���ѹ����˸Ƥ�
 */
static	int	sound_flags_update     = 0;
static	int	sound_prescaler_update = 0;

void	change_sound_flags( int port )
{
  sound_flags_update = (int)port;
  if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
}
void	change_sound_prescaler( int new_prescaler )
{
  sound_prescaler_update = new_prescaler;
  if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
}




/*
 * ������ɤΥץꥹ�����顼�͡��Ƽ�ե饰�ѹ����κݤˡ��ե饰���������롣
 *	�����߹�����˸ƤФ�롣
 */
static	void	check_sound_parm_update( void )
{
  byte data;

  if( sound_prescaler_update ){		/* ʬ�� �ѹ������ä���		    */
					/* �������ͤ� (�ѹ���/�ѹ���)�ܤ��� */
					/* �������ͤΤĤ��Ĥޤ򤢤碌�롣   */
    sd_A_intr_base  = sd_A_intr_base  * sound_prescaler_update/sound_prescaler;
    sd_A_intr_timer = sd_A_intr_timer * sound_prescaler_update/sound_prescaler;
    sd_B_intr_base  = sd_B_intr_base  * sound_prescaler_update/sound_prescaler;
    sd_B_intr_timer = sd_B_intr_timer * sound_prescaler_update/sound_prescaler;
    sound_prescaler = sound_prescaler_update;
    sound_prescaler_update = 0;
  }


  switch( sound_flags_update ){		/* �ե饰�ѹ������ä��顢���� */

  case 0x27:		/*---------------- RESET_B/A | ENABLE_B/A | LOAD_B/A */
    data = sound_reg[0x27];

					/* LOAD��Ω���夬��ˡ��������͹��� */
    if( (sound_LOAD_A==0) && (data&0x01) ) sd_A_intr_timer = sd_A_intr_base;
    if( (sound_LOAD_B==0) && (data&0x02) ) sd_B_intr_timer = sd_B_intr_base;
    sound_LOAD_A = data & 0x01;
    sound_LOAD_B = data & 0x02;

					/* ENABLE ����¸ */
    sound_ENABLE_A = ( data & 0x04 ) ? 1 : 0;
    sound_ENABLE_B = ( data & 0x08 ) ? 1 : 0;

					/* RESET �� 1 �ʤ顢�ե饰���ꥢ */
    if( data & 0x10 ) sound_FLAG_A = 0;
    if( data & 0x20 ) sound_FLAG_B = 0;
    break;

  case 0x29:		/*---------------- EN_ZERO/BRDY/EOS/TB/TA  */
    if( sound_board==SOUND_II ){
      data = sound_reg[0x29];
      sound2_EN_TA   = data & 0x01;
      sound2_EN_TB   = data & 0x02;
      sound2_EN_EOS  = data & 0x04;
      sound2_EN_BRDY = data & 0x08;
      sound2_EN_ZERO = data & 0x10;
    }
    break;

  case 0x10:		/*---------------- IRQ_RET | MSK_ZERO/BRDY/EOS/TB/TA */
    data = sound2_reg[0x10];
    if( data & 0x80 ){
      sound_FLAG_A     = 0;
      sound_FLAG_B     = 0;
      sound2_FLAG_EOS  = 0;
      sound2_FLAG_BRDY = 0;
      sound2_FLAG_ZERO = 0;
    }else{
      sound2_MSK_TA   = data & 0x01;
      sound2_MSK_TB   = data & 0x02;
      sound2_MSK_EOS  = data & 0x04;
      sound2_MSK_BRDY = data & 0x08;
      sound2_MSK_ZERO = data & 0x10;
    }
    break;

  }

  sound_flags_update = 0;

}


/*
 * ������ɥܡ���II��Ϣ
 */
void	interval_work_set_BDRY( void )
{
  sd2_BRDY_intr_base  = sound2_intr_base * 2 * ( CPU_CLOCK_MHZ / 4.0 );
  sd2_BRDY_intr_timer = sd2_BRDY_intr_base;

/*printf("%d\n",sd2_BRDY_intr_base);*/
}
void	interval_work_set_EOS( int length )
{
  sd2_EOS_intr_base  = sd2_BRDY_intr_base * length;
  sd2_EOS_intr_timer = sd2_EOS_intr_base;

/*printf("%d\n",sd2_EOS_intr_base);*/
}



/************************************************************************/
/* 1/60 sec �γ�������˹Ԥʤ�����					*/
/************************************************************************/

/* #define ���ȡ�VSYNC���ϻ���ɽ����#undef ����VBLANK ��λ����ɽ�� */
#undef	DRAW_SCREEN_AT_VSYNC_START

static	void	vsync( void )
{
  xmame_sound_update();			/* ������ɽ��� */

  if( !no_wait ) wait_vsync();		/* �»��֤ǥ������Ȥ��� */

  xmame_update_video_and_audio();	/* ������ɽ��� ����2 */

  event_handle();			/* ���٥�Ƚ���		*/

  scan_keyboard();			/* ��������������	*/

#ifdef	DRAW_SCREEN_AT_VSYNC_START
  draw_screen();			/* ���̽���		*/
#endif

  SET_DMA_WAIT_COUNT();			/* DMA���񥵥���������å� */


  state_of_cpu -= state_of_vsync;	/* (== vsync_intr_base) */
}






INLINE	void	set_INT_active( void )
{
  if( intr_level==0 ){				/* ��٥����� 0 */
    z80main_cpu.INT_active = FALSE;		/*    �����ߤϼ����դ��ʤ� */
  }
  else if( intr_level>=1 &&			/* ��٥����� 1 */
	   intr_sio_enable && RS232C_flag ){	/*    RS232S ���� ������ */
    z80main_cpu.INT_active = TRUE;
  }
  else if( intr_level>=2 &&			/* ��٥����� 2 */
	   intr_vsync_enable && VSYNC_flag ){	/*    VSYNC ������ */
    z80main_cpu.INT_active = TRUE;
  }
  else if( intr_level>=3 &&			/* ��٥����� 3 */
	   intr_rtc_enable && RTC_flag ){	/*    1/600�� RTC ������ */
    z80main_cpu.INT_active = TRUE;
  }
  else if( intr_level>=5 &&			/* ��٥����� 5 */
	   intr_sound_enable && SOUND_flag ){	/*    SOUND TIMER ������ */
    z80main_cpu.INT_active = TRUE;
  }
  else{
    z80main_cpu.INT_active = FALSE;
  }
}

/*----------------------------------------------------------------------*/
/* �����ߤ��������롣��Ʊ���ˡ����γ����ߤޤǤΡ��Ǿ� state ��׻�	*/
/*	�����ͤϡ�Z80����������λ�Υե饰(TRUE/FALSE)			*/
/*----------------------------------------------------------------------*/
void	main_INT_update( void )
{

  int	icount = rtc_intr_base;		/* ���γ�����ȯ���ޤǤκǾ�state�� */
					/* �Ȥꤢ������RTC����μ����ǽ���� */


		/* -------- RS232C ������ -------- */

  rs232c_intr_timer -= z80main_cpu.state0;
  if( rs232c_intr_timer < 0 ){
    rs232c_intr_timer += rs232c_intr_base;
    if( sio_intr() ){
      RS232C_flag = TRUE;
    }
  }
  icount = MIN( icount, rs232c_intr_timer );


		/* -------- VSYNC ������ -------- */

  state_of_cpu += z80main_cpu.state0;

  vsync_intr_timer -= z80main_cpu.state0;
  if( vsync_intr_timer < 0 ){
    vsync();					/* �������ȡ�ɽ�������� */
    VSYNC_flag = TRUE;				/* VSYNC������	*/
    vsync_intr_timer += vsync_intr_base;

    ctrl_vrtc  = 1;
    vrtc_timer = vrtc_base + z80main_cpu.state0;
  }
  icount = MIN( icount, vsync_intr_timer );


		/* -------- VRTC ���� -------- */

  if( ctrl_vrtc == 1 ){				/* VSYNC ���� ������� */
    vrtc_timer -= z80main_cpu.state0;		/* �в�ǡ�ɽ�����֤�  */
    if( vrtc_timer < 0 ){
      ctrl_vrtc = 2;
      vrtc_timer += vrtc_base2;

      /*VSYNC_flag = FALSE;*/			/* VSYNC�����OFF���ʤ� */
		/* OFF����ȸ��껦�ͻ���Ȥ���ư���ʤ�   thanks peach ! */
#ifndef	DRAW_SCREEN_AT_VSYNC_START
      draw_screen();				/* ���̽���		*/
#endif
    }

  }else if( ctrl_vrtc == 2 ){			/* ɽ�����֤��������� */
    vrtc_timer -= z80main_cpu.state0;		/* �в�ǡ�VBLANK���֤� */
    if( vrtc_timer < 0 ){
      ctrl_vrtc = 3;
      vrtc_timer = 0xffff; /* ǰ�Τ��� */
    }
  }

  if( ctrl_vrtc < 3 ) icount = MIN( icount, vrtc_timer );


		/* -------- RTC ������ -------- */

  rtc_intr_timer -= z80main_cpu.state0;
  if( rtc_intr_timer < 0 ){
    RTC_flag = TRUE;
    rtc_intr_timer += rtc_intr_base;
  }
  icount = MIN( icount, rtc_intr_timer );


		/* -------- SOUND TIMER A ������ -------- */

  if( sound_LOAD_A ){
    sd_A_intr_timer -= z80main_cpu.state0;
    if( sd_A_intr_timer < 0 ){
      xmame_sound_timer_over(0);
      sd_A_intr_timer += sd_A_intr_base;
      if( sound_ENABLE_A ){
	if( sound2_MSK_TA ) sound_FLAG_A = 0;
	else                sound_FLAG_A = 1;
	if( sound_FLAG_A && sound2_EN_TA ) SOUND_flag = TRUE;
      }
    }
    icount = MIN( icount, sd_A_intr_timer );
  }


		/* -------- SOUND TIMER B ������ -------- */

  if( sound_LOAD_B ){
    sd_B_intr_timer -= z80main_cpu.state0;
    if( sd_B_intr_timer < 0 ){
      xmame_sound_timer_over(1);
      sd_B_intr_timer += sd_B_intr_base;
      if( sound_ENABLE_B ){
	if( sound2_MSK_TB ) sound_FLAG_B = 0;
	else                sound_FLAG_B = 1;
	if( sound_FLAG_B && sound2_EN_TB ) SOUND_flag = TRUE;
      }
    }
    icount = MIN( icount, sd_B_intr_timer );
  }

  if( sound2_FLAG_PCMBSY ){

    sd2_BRDY_intr_timer -= z80main_cpu.state0;
    if( sd2_BRDY_intr_timer < 0 ){
      sd2_BRDY_intr_timer += sd2_BRDY_intr_base;
      {
	if( sound2_MSK_BRDY ) sound2_FLAG_BRDY = 0;
	else                  sound2_FLAG_BRDY = 1;
	if( sound2_FLAG_BRDY && sound2_EN_BRDY ) SOUND_flag = TRUE;
      }
    }

    if( sound2_notice_EOS ){
      sd2_EOS_intr_timer -= z80main_cpu.state0;
      if( sd2_EOS_intr_timer < 0 ){
	sd2_EOS_intr_timer += sd2_EOS_intr_base;
	if( sound2_MSK_EOS ) sound2_FLAG_EOS = 0;
	else                 sound2_FLAG_EOS = 1;
	if( sound2_FLAG_EOS && sound2_EN_EOS ) SOUND_flag = TRUE;
	if( !sound2_repeat )  sound2_FLAG_PCMBSY = 0;
	sound2_notice_EOS = FALSE;
      }
    }

  }

	/* ������ȯ�������顢z80->INT_active �˿��򥻥åȤ��� */

  set_INT_active();

	/* ������ɤΡ������ߤ˴ؤ��쥸�������ѹ�����Ƥʤ�����ǧ */

  check_sound_parm_update();


	/* ���γ�����ȯ���ޤǤΡ����ơ��ȿ��򥻥å� */

  z80main_cpu.icount = icount;


	/* ��˥塼�ؤ����ܤʤɤϡ������ǳ�ǧ */

  if( next_emu_mode() != EXEC ) CPU_BREAKOFF();
}






/************************************************************************/
/* �ޥ����֥�����ߥ��ߥ�졼��					*/
/************************************************************************/
/*--------------------------------------*/
/* ����� (Z80�ꥻ�åȻ��˸Ƥ�)		*/
/*--------------------------------------*/
void	main_INT_init( void )
{
  RS232C_flag  = FALSE;
  VSYNC_flag   = FALSE;
  RTC_flag     = FALSE;
  SOUND_flag   = FALSE;

  interval_work_init_all();
  ctrl_vrtc = 1;

/*
printf("CPU    %f\n",cpu_clock_mhz);
printf("SOUND  %f\n",sound_clock_mhz);
printf("SYNC   %f\n",vsync_freq_hz);
printf("RS232C %d\n",rs232c_intr_base);
printf("VSYNC  %d\n",vsync_intr_base);
printf("VRTC   %d\n",vrtc_base);
printf("RTC    %d\n",rtc_intr_base);
printf("A      %d\n",sd_A_intr_base);
printf("B      %d\n",sd_B_intr_base);
*/
}

/*----------------------------------------------*/
/* �����å� (������Ļ� 1���ƥå���˸ƤФ��)	*/
/*						*/
/*	�棱��IM!=2 �λ��ϳ����ߤϤɤ��ʤ롩	*/
/*	�棲��intr_priority �����λ��ϡ�	*/
/*	      �����߾��֤Ϥɤ��ʤ롩		*/
/*		  (������̵��)			*/
/*						*/
/*----------------------------------------------*/
int	main_INT_chk( void )
{
  int	intr_no = -1;

  /*  if( z80main_cpu.IM!=2 ) return -1;*/


  if( intr_level==0 ){				/* ��٥����� 0 */
    {						/*    �����ߤϼ����դ��ʤ� */
      intr_no = -1;
    }
  }
  else if( intr_level>=1 &&			/* ��٥����� 1 */
	   intr_sio_enable && RS232C_flag ){	/*    RS232S ���� ������ */
    RS232C_flag = FALSE;
    intr_no = 0;
  }
  else if( intr_level>=2 &&			/* ��٥����� 2 */
	   intr_vsync_enable && VSYNC_flag ){	/*    VSYNC ������ */
    VSYNC_flag = FALSE;
    intr_no = 1;
  }
  else if( intr_level>=3 &&			/* ��٥����� 3 */
	   intr_rtc_enable && RTC_flag ){	/*    1/600�� RTC ������ */
    RTC_flag = FALSE;
    intr_no = 2;
  }
  else if( intr_level>=5 &&			/* ��٥����� 5 */
	   intr_sound_enable && SOUND_flag ){	/*    SOUND TIMER ������ */
    SOUND_flag = FALSE;
    intr_no = 4;
  }

	/* �����߼����դ����顢z80->INT_active �˵��򥻥åȤ��� */

  set_INT_active();

  if( intr_no >= 0 ){
    if( intr_priority ) intr_level = 7;
    else                intr_level = intr_no;
    return intr_no;
  }else{
    return -1;
  }
}








/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"INTR"
#define	SID2	"INT2"

static	T_SUSPEND_W	suspend_intr_work[]=
{
  { TYPE_INT,	&intr_level,		},
  { TYPE_INT,	&intr_priority,		},
  { TYPE_INT,	&intr_sio_enable,	},
  { TYPE_INT,	&intr_vsync_enable,	},
  { TYPE_INT,	&intr_rtc_enable,	},

  { TYPE_DOUBLE,&cpu_clock_mhz,		},
  { TYPE_DOUBLE,&sound_clock_mhz,	},
  { TYPE_DOUBLE,&vsync_freq_hz,		},

  { TYPE_INT,	&wait_rate,		},
  { TYPE_INT,	&wait_by_sleep,		},
  { TYPE_LONG,	&wait_sleep_min_us,	},

  { TYPE_INT,	&state_of_cpu,		},
  { TYPE_INT,	&state_of_vsync,	},

  { TYPE_INT,	&no_wait,		},

  { TYPE_INT,	&RS232C_flag,		},
  { TYPE_INT,	&rs232c_intr_base,	},
  { TYPE_INT,	&rs232c_intr_timer,	},

  { TYPE_INT,	&VSYNC_flag,		},
  { TYPE_INT,	&vsync_intr_base,	},
  { TYPE_INT,	&vsync_intr_timer,	},

  { TYPE_INT,	&ctrl_vrtc,		},
  { TYPE_INT,	&vrtc_base,		},
  { TYPE_INT,	&vrtc_timer,		},

  { TYPE_INT,	&RTC_flag,		},
  { TYPE_INT,	&rtc_intr_base,		},
  { TYPE_INT,	&rtc_intr_timer,	},

  { TYPE_INT,	&SOUND_flag,		},
  { TYPE_INT,	&sd_A_intr_base,	},
  { TYPE_INT,	&sd_A_intr_timer,	},
  { TYPE_INT,	&sd_B_intr_base,	},
  { TYPE_INT,	&sd_B_intr_timer,	},

  { TYPE_INT,	&sound_flags_update,	},
  { TYPE_INT,	&sound_prescaler_update,},

  { TYPE_INT,	&sd2_BRDY_intr_base,	},
  { TYPE_INT,	&sd2_BRDY_intr_timer,	},
  { TYPE_INT,	&sd2_EOS_intr_base,	},
  { TYPE_INT,	&sd2_EOS_intr_timer,	},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_intr_work2[]=
{
  { TYPE_INT,	&vrtc_base2,		},
  { TYPE_END,	0			},
};


int	statesave_intr( void )
{
  if( statesave_table( SID, suspend_intr_work ) != STATE_OK ) return FALSE;

  if( statesave_table( SID2, suspend_intr_work2 ) != STATE_OK ) return FALSE;
  return TRUE;
}

int	stateload_intr( void )
{
  if( stateload_table( SID, suspend_intr_work ) != STATE_OK ) return FALSE;

  if( stateload_table( SID2, suspend_intr_work2 ) != STATE_OK ){

    /* ��С������ʤ顢�ߤΤ��� */

    printf( "stateload : Statefile is old. (ver 0.6.0?)\n" );
/*  printf( "stateload : Header revision is %d\n", statefile_revision() );*/

    vrtc_base2 = vsync_intr_base * VRTC_DISP;
    if( ctrl_vrtc == 0 ) ctrl_vrtc = 2;
    return TRUE;

  }

  return TRUE;
}
