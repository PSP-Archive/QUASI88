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

/* select, usleep, nanosleep �Τ����줫�Υ����ƥॳ�������Ѥ���Τǡ�
   �ʲ��Τɤ줫��Ĥ�Ĥ��ơ�¾�ϥ����ȥ����Ȥ��� */

#define USE_SELECT
/* #define USE_USLEEP */
/* #define USE_NANOSLEEP */


#include <stdio.h>

#include <sys/types.h>		/* select                        */
#include <sys/time.h>		/* select           gettimeofday */
#include <unistd.h>		/* select usleep                 */
#include <time.h>		/*        nanosleep clock        */

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
   64bit��(long long)�ˤ��������Ϥʤ�������ɤ⡢ɸ�स��ʤ��Τ���α */

typedef	long		T_WAIT_TICK;

static	T_WAIT_TICK	next_time;		/* ���ե졼��λ��� */
static	T_WAIT_TICK	delta_time;		/* 1 �ե졼��λ��� */

static	T_WAIT_TICK	sleep_min_time = 100;	/* sleep ��ǽ�ʺǾ����� */



/* ---- ���ꤵ�줿���� (usecñ��) sleep ���� ---- */

INLINE	void	delay_usec( unsigned int usec )
{
#if	defined( USE_SELECT )		/* select ��Ȥ� */

  struct timeval tv;
  tv.tv_sec  = 0;
  tv.tv_usec = usec;
  select( 0, NULL, NULL, NULL, &tv );

#elif	defined( USE_USLEEP )		/* usleep ��Ȥ� */

  usleep( usec );

#elif	defined( USE_NANOSLEEP )	/* nanosleep ��Ȥ� */

  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = usec * 1000;
  nanosleep( &ts, NULL );

#else					/* �ɤ��Ȥ��ʤ� ! */
  wait_by_sleep = FALSE; /* X_X; */
#endif
}



/* ---- ���߻����������� (usecñ��) ---- */
static int tick_error = FALSE;


#ifdef  HAVE_GETTIMEOFDAY		/* gettimeofday() ��Ȥ� */

static struct timeval start_tv;

INLINE	void		set_tick( void )
{
  if( gettimeofday( &start_tv, 0 ) ){
    if( verbose_wait ) printf( "Clock Error\n" );
    tick_error = TRUE;
    start_tv.tv_sec  = 0;
    start_tv.tv_usec = 0;
  }
}

INLINE	T_WAIT_TICK	get_tick( void )
{
  struct timeval tv;
  if( gettimeofday( &tv, 0 ) ){
    if( verbose_wait ){ if( tick_error==FALSE ) printf( "Clock Error\n" ); }
    tick_error = TRUE;
    tv.tv_sec  = 1;
    tv.tv_usec = 1;
  }

  return ( (T_WAIT_TICK)(tv.tv_sec  - start_tv.tv_sec) * 1000000 +
	   (T_WAIT_TICK)(tv.tv_usec - start_tv.tv_usec) );
}


#else					/* clock() ��Ȥ� */

/* #define CLOCK_SLICE	CLK_TCK */		/* ���줸�����ܡ� */
#define	CLOCK_SLICE	CLOCKS_PER_SEC		/* ���ä������� */

INLINE	void		set_tick( void )
{
}

INLINE	T_WAIT_TICK	get_tick( void )
{
  clock_t t = clock();
  if( t == (clock_t)-1 ){
    if( verbose_wait ){ if( tick_error==FALSE ) printf( "Clock Error\n" ); }
    tick_error = TRUE;
    t = CLOCK_SLICE;
  }

  return ( (T_WAIT_TICK) (t / CLOCK_SLICE) * 1000000 +
	   (T_WAIT_TICK)((t % CLOCK_SLICE) * 1000000.0 / CLOCK_SLICE) );
}

#endif





/****************************************************************************
 * �������Ƚ��������
 *****************************************************************************/
int	wait_vsync_init( void )
{
  if( verbose_proc ){
#ifdef  HAVE_GETTIMEOFDAY
    printf("\nTimer start (gettimeofday(2) - based)\n" );
#else
    printf("\nTimer start (clock(3) - based)\n" );
#endif
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
  set_tick();

  sleep_min_time = wait_sleep_min_us;
  wait_counter = 0;

  delta_time = (T_WAIT_TICK)(1000000.0/( CONST_VSYNC_FREQ * wait_rate / 100 ));
  next_time  = get_tick() + delta_time;

  /* delta_time >= 1000000us (1sec) �ˤʤ�ȡ�����äȤޤ��� */
}



/****************************************************************************
 * �������Ƚ���
 *****************************************************************************/
void	wait_vsync( void )
{
  int	on_time = FALSE;
  T_WAIT_TICK	diff_us;


  diff_us = next_time - get_tick();

  if( tick_error == FALSE ){

    if( diff_us > 0 ){	    /* �ޤ����֤�;�äƤ���ʤ� */
			    /* diff_us �̥ߥ��å������� */

      if( wait_by_sleep ){	/* ���֤����ޤ� sleep ������ */

	if( diff_us < sleep_min_time ){	/* �Ĥ�Ϥ��ʤ�ӥ����������� */
	  while( tick_error == FALSE ){
	    if( next_time <= get_tick() )
	      break;
	  }
	}else{				/* �Ĥ�¿����Хǥ��쥤       */
	  delay_usec( diff_us );
	}

      }else{			/* ���֤����ޤ� Tick ��ƻ뤹���� */

	while( tick_error == FALSE ){
	  if( next_time <= get_tick() )
	    break;
	}
      }

      on_time = TRUE;
    }
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
/*if(skip_counter)printf("%x\n",skip_counter);*/
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
#if defined( USE_SELECT ) || defined( USE_USLEEP ) || defined( USE_NANOSLEEP )

  delay_usec( 1000000 / 60 );

#else

  next_time = get_tick() + (T_WAIT_TICK)(1000000/60);

  while( tick_error == FALSE ){
    if( next_time <= get_tick() )
      break;
  }

#endif
}














#ifdef  HAVE_GETTIMEOFDAY
void print_gettimeofday(void)
{
  struct timeval tv;
  if( gettimeofday( &tv, 0 ) == 0 ){
    printf( "%ld.%06ld\n", tv.tv_sec, tv.tv_usec );
  }
}

void print_lapse(int flag)
{
  static struct timeval t0, t1, dt;

  if( t0.tv_sec == 0 || flag < 0 ){
    gettimeofday( &t0, 0 );
  }else{
    gettimeofday( &t1, 0 );

    if( flag > 0 ){
      dt.tv_sec  = t1.tv_sec  - t0.tv_sec;
      dt.tv_usec = t1.tv_usec - t0.tv_usec;
      if( dt.tv_usec < 0 ){
	dt.tv_sec --;
	dt.tv_usec += 1000000; 
      }
      printf( "%ld.%06ld\n", dt.tv_sec, dt.tv_usec );
    }

    t0.tv_sec  = t1.tv_sec;
    t0.tv_usec = t1.tv_usec;
  }
}
#endif
