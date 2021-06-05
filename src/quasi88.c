/************************************************************************/
/* QUASI88 --- PC-8801 emulator						*/
/*	Copyright (c) 1998-2006 Showzoh Fukunaga			*/
/*	All rights reserved.						*/
/*									*/
/*	  ���Υ��եȤϡ�UNIX + X Window System �δĶ���ư��롢	*/
/*	PC-8801 �Υ��ߥ�졼���Ǥ���					*/
/*									*/
/*	  ���Υ��եȤκ����ˤ����ꡢMarat Fayzullin���� fMSX��	*/
/*	Nicola Salmoria�� ( MAME/XMAME project) ��� mame/xmame��	*/
/*	��ߤ������ PC6001V �Υ������򻲹ͤˤ����Ƥ�餤�ޤ�����	*/
/*									*/
/*	����ա�							*/
/*	  ������ɥɥ饤�Фϡ�mame/xmame �Υ�������ή�Ѥ��Ƥ��ޤ���	*/
/*	������ʬ�Υ�����������ϡ�mame/xmame �����ढ�뤤�ϥ�������	*/
/*	���ܤ��Ƥ�������Ԥˤ���ޤ���					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "initval.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "memory.h"
#include "file-op.h"

#include "emu.h"
#include "drive.h"
#include "event.h"
#include "keyboard.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "status.h"
#include "suspend.h"
#include "snapshot.h"
#include "soundbd.h"
#include "screen.h"
#include "menu.h"


int	verbose_level	= DEFAULT_VERBOSE;	/* ��Ĺ��٥�		*/
int	verbose_proc    = FALSE;		/* �����οʹԾ�����ɽ��	*/
int	verbose_z80	= FALSE;		/* Z80�������顼��ɽ��	*/
int	verbose_io	= FALSE;		/* ̤����I/O��������ɽ��*/
int	verbose_pio	= FALSE;		/* PIO ���������Ѥ�ɽ�� */
int	verbose_fdc	= FALSE;		/* FD���᡼���۾�����	*/
int	verbose_wait	= FALSE;		/* �������Ȼ��ΰ۾����� */
int	verbose_suspend = FALSE;		/* �����ڥ�ɻ��ΰ۾����� */
int	verbose_snd	= FALSE;		/* ������ɤΥ�å�����	*/

char	file_disk[2][QUASI88_MAX_FILENAME];	/*�ǥ��������᡼���ե�����̾*/
int	image_disk[2];	 	  		/*���᡼���ֹ�0��31,-1�ϼ�ư*/
int	readonly_disk[2];			/*�꡼�ɥ���꡼�ǳ����ʤ鿿*/

char	file_tape[2][QUASI88_MAX_FILENAME];	/* �ơ��������ϤΥե�����̾ */
char	file_prn[QUASI88_MAX_FILENAME];		/* �ѥ�����ϤΥե�����̾ */
char	file_sin[QUASI88_MAX_FILENAME];		/* ���ꥢ����ϤΥե�����̾ */
char	file_sout[QUASI88_MAX_FILENAME];	/* ���ꥢ�����ϤΥե�����̾ */

int	file_coding = 0;			/* �ե�����̾�δ���������   */
int	filename_synchronize = TRUE;		/* �ե�����̾��ƱĴ������   */



static	void	disk_set( void );
static	void	bootup_work_init( void );

/*----------------------------------------------------------------------*/
/* �����뤿��ν��������դ�						*/
/*----------------------------------------------------------------------*/

#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
FILE	*LOG = NULL;
#endif

static	void	debug_log_init( void )
{
#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
  LOG = fopen("log","w");
#endif
  if( verbose_proc ){
#if	defined( PIO_DISP ) || defined( PIO_FILE )
    printf("+ Support PIO logging. set variable \"pio_debug\" to 1.\n");
#endif
#if	defined( FDC_DISP ) || defined( FDC_FILE )
    printf("+ Support FDC logging. set variable \"fdc_debug\" to 1.\n");
#endif
#if	defined( MAIN_DISP ) || defined( MAIN_FILE )
    printf("+ Support Main Z80 logging. set variable \"main_debug\" to 1.\n");
#endif
#if	defined( SUB_DISP ) || defined( SUB_FILE )
    printf("+ Support Sub Z80 logging. set variable \"sub_debug\" to 1.\n");
#endif
  }
}

static	void	debug_log_finish( void )
{
#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
  if( LOG ) fclose(LOG);
#endif
}



/***********************************************************************
 *
 *			QUASI88 �ᥤ��ؿ�
 *
 ************************************************************************/
static	int	proc;
int	quasi88( void )
{
  verbose_proc	= verbose_level & 0x01;
  verbose_z80	= verbose_level & 0x02;
  verbose_io	= verbose_level & 0x04;
  verbose_pio	= verbose_level & 0x08;
  verbose_fdc	= verbose_level & 0x10;
  verbose_wait	= verbose_level & 0x20;
  verbose_suspend=verbose_level & 0x40;
  verbose_snd	= verbose_level & 0x80;

  if( verbose_proc ) printf("\n"); fflush(NULL);
  proc = 0;


  stateload_init();			/* ���ơ��ȥ��ɴ�Ϣ�����         */
  screen_snapshot_init();		/* ���̥��ʥåץ���åȴ�Ϣ�����   */

  status_init();			/* ���ơ�����ɽ���Υ�������     */
  drive_init();				/* �ե�åԡ��ɥ饤�֤Υ������� */

  event_handle_init();


  if( memory_allocate() ){		/* ���ߥ�졼���ѥ���γ���	*/
    if( verbose_proc ) printf("\n"); fflush(NULL);
    proc = 1;

    set_signal();			/* INT�����ʥ�ν���������	*/

    if( resume_flag ){			/* ���ơ��ȥ���		*/
      if( stateload() == FALSE ){
	fprintf( stderr, "stateload: Failed ! (filename = %s)\n", file_state );
	quasi88_exit();
      }
      if( verbose_suspend ) printf("\n"); fflush(NULL);
    }

    if( graphic_system_init() ){	/* ����ե��å������ƥ�����	*/
      if( verbose_proc ) printf("\n"); fflush(NULL);
      proc = 2;

      screen_buf_init();

      if( xmame_sound_start() ){	/* ������ɥɥ饤�н����	*/
	if( verbose_proc ) printf("\n"); fflush(NULL);
	proc = 3;

	disk_set();
	bootup_work_init();

	if( file_tape[CLOAD][0] ) sio_open_tapeload( file_tape[CLOAD] );
	if( file_tape[CSAVE][0] ) sio_open_tapesave( file_tape[CSAVE] );
	if( file_sin[0] )         sio_open_serialin( file_sin );
	if( file_sout[0] )        sio_open_serialout( file_sout );
	if( file_prn[0] )         printer_open( file_prn );
					/* ���ơ��ȥ��ɻ��� �ơ�SEEK ? */

	key_record_playback_init();

	pc88main_init( (resume_flag) ? INIT_STATELOAD : INIT_POWERON );
	pc88sub_init(  (resume_flag) ? INIT_STATELOAD : INIT_POWERON );


	if( wait_vsync_init() ){	/* ���������ѥ����ޡ������	*/
	  if( verbose_proc ) printf("\n"); fflush(NULL);
	  proc = 4;

	  if( verbose_proc ) printf( "Running QUASI88...\n" );

	  if( resume_flag == 0 ) indicate_bootup_logo();
	  else                   indicate_stateload_logo();

	  debug_log_init();
	  {
	    emu();			/* ���ߥ�졼�� �ᥤ�� */
	  }
	  debug_log_finish();

	  if( verbose_proc ) printf( "Shutting down.....\n" );
	  wait_vsync_term();
	}

	pc88main_term();
	pc88sub_term();

	disk_eject( 0 );
	disk_eject( 1 );

	sio_close_tapeload();
	sio_close_tapesave();
	sio_close_serialin();
	sio_close_serialout();
	printer_close();

	key_record_playback_term();

	xmame_sound_stop();
      }
      graphic_system_term();
    }
    memory_free();
  }



	/* �ܺ�ɽ���򤷤ʤ����ä����Ρ����顼ɽ�� */

  if( !verbose_proc ){
    switch( proc ){
    case 0: printf( "memory allocate failed!\n" );		break;
    case 1: printf( "graphic system initialize failed!\n" );	break;
    case 2: printf( "sound system initialize failed!\n" );	break;
    case 3: printf( "timer initialize failed!\n" );		break;
    }
  }

  return 0;
}




/***********************************************************************
 * QUASI88 ���潪λ�����ؿ�
 *	exit() ������˸Ƥܤ���
 ************************************************************************/
static	void (*exit_function)(void) = NULL;

void	quasi88_atexit( void (*function)(void) )
{
  exit_function = function;
}

void	quasi88_exit( void )
{
  switch( proc ){
  case 4:
    debug_log_finish();
    wait_vsync_term();			/* FALLTHROUGH */
  case 3:
    pc88main_term();
    pc88sub_term();
    disk_eject( 0 );
    disk_eject( 1 );
    sio_close_tapeload();
    sio_close_tapesave();
    sio_close_serialin();
    sio_close_serialout();
    printer_close();
    key_record_playback_term();
    xmame_sound_stop();			/* FALLTHROUGH */
  case 2:
    graphic_system_term();
  case 1:
    memory_free();			/* FALLTHROUGH */
  }

  if( exit_function ){
    (*exit_function)();
  }

  exit( -1 );
}





/***********************************************************************
 * QUASI88 ��ư��Υꥻ�åȽ����ؿ�
 *
 ************************************************************************/
void	quasi88_reset( void )
{
  int	empty[2];

  pc88main_term();
  pc88sub_term();

  bootup_work_init();

  pc88main_init( INIT_RESET );
  pc88sub_init(  INIT_RESET );

  empty[0] = drive_check_empty(0);
  empty[1] = drive_check_empty(1);
  drive_reset();
  if( empty[0] ) drive_set_empty(0);
  if( empty[1] ) drive_set_empty(1);
  if( use_sound ) xmame_sound_reset();

  emu_reset();
}




/***********************************************************************
 * QUASI88 ��ư��Υ��ơ��ȥ��ɽ����ؿ�
 *
 ************************************************************************/
int	quasi88_stateload( void )
{
  int now_board, success;

  sio_close_tapeload();			/* ���᡼���ե�����������Ĥ��� */
  sio_close_tapesave();
  sio_close_serialin();
  sio_close_serialout();
  printer_close();

  disk_eject( 0 );
  disk_eject( 1 );
  quasi88_reset();


  now_board = sound_board;

  success = stateload();		/* ���ơ��ȥ��ɼ¹� */

  if( now_board != sound_board ){ 	/* ������ɥܡ��ɤ��Ѥ�ä��� */
    xmame_sound_resume();		/* ���Ǥ���������ɤ�������� */
    xmame_sound_stop();			/* ������ɤ���ߤ����롣     */
    xmame_sound_start();		/* �����ơ�������ɺƽ����   */
  }


  if( success ){			/* ���ơ��ȥ������������顦���� */

    disk_set();					/* ���᡼���ե���������Ƴ���*/
    bootup_work_init();

    if( file_tape[CLOAD][0] ) sio_open_tapeload( file_tape[CLOAD] );
    if( file_tape[CSAVE][0] ) sio_open_tapesave( file_tape[CSAVE] );
    if( file_sin[0] )         sio_open_serialin( file_sin );
    if( file_sout[0] )        sio_open_serialout( file_sout );
    if( file_prn[0] )         printer_open( file_prn );

    pc88main_init( INIT_STATELOAD );		/* �Ƽ�������� */
    pc88sub_init(  INIT_STATELOAD );

  }else{				/* ���ơ��ȥ��ɼ��Ԥ����顦���� */

    quasi88_reset();				/* �Ȥꤢ�����ꥻ�å� */
  }

  return success;
}








/*----------------------------------------------------------------------
 * file_disk[][] �����ꤵ��Ƥ���ǥ��������᡼���򥻥åȤ���
 *	( ��ư��������ӥ��ơ��ȥ��ɻ� )
 *----------------------------------------------------------------------*/
static	void	disk_set( void )
{
  int err0 = TRUE;
  int err1 = TRUE;

  if( file_disk[0][0] &&	/* �ɥ饤��1,2 �Ȥ⥤�᡼������Ѥߤξ�� */
      file_disk[1][0] ){		/*	% quasi88 file file       */
					/*	% quasi88 file m m        */
					/*	% quasi88 file n file     */
					/*	% quasi88 file file m     */
					/*	% quasi88 file n file m   */
    int same = (strcmp( file_disk[0], file_disk[1] )==0) ? TRUE : FALSE;

    err0 = disk_insert( DRIVE_1,		/* �ɥ饤�� 1 �򥻥å� */
			file_disk[0],
			(image_disk[0]<0) ? 0 : image_disk[0],
			readonly_disk[0] );

    if( same ){					/* Ʊ��ե�����ξ��� */

      if( err0 == FALSE ){				/* 1: �� 2: ž�� */
	err1 = disk_insert_A_to_B( DRIVE_1, DRIVE_2, 
				   (image_disk[1]<0) ? 0 : image_disk[1] );
      }

    }else{					/* �̥ե�����ξ��� */

      err1 = disk_insert( DRIVE_2,			/* �ɥ饤��2 ���å� */
			  file_disk[1],
			  (image_disk[1]<0) ? 0 : image_disk[1],
			  readonly_disk[1] );
    }

    /* ξ�ɥ饤�֤�Ʊ���ե����� ���� ���᡼�����꼫ư�ξ��ν��� */
    if( err0 == FALSE && err1 == FALSE &&
	drive[DRIVE_1].fp == drive[DRIVE_2].fp  && 
	image_disk[0] < 0 && image_disk[1] < 0 ){
      disk_change_image( DRIVE_2, 1 );			/* 2: �� ���᡼��2�� */
    }

  }else if( file_disk[0][0] ){	/* �ɥ饤��1 ���� ���᡼������Ѥߤξ�� */
					/*	% quasi88 file		 */
					/*	% quasi88 file num       */
    err0 = disk_insert( DRIVE_1,
			file_disk[0],
			(image_disk[0]<0) ? 0 : image_disk[0],
			readonly_disk[0] );

    if( err0 == FALSE ){
      if( image_disk[0] < 0 &&			/* ���᡼���ֹ����ʤ��ʤ� */
	  disk_image_num( DRIVE_1 ) >= 2 ){	/* �ɥ饤��2�ˤ⥻�å�      */

	err1 = disk_insert_A_to_B( DRIVE_1, DRIVE_2, 1 );
	if( err1 == FALSE ){
	  memcpy( file_disk[1], file_disk[0], QUASI88_MAX_FILENAME );
	}
      }
    }

  }else if( file_disk[1][0] ){	/* �ɥ饤��2 ���� ���᡼������Ѥߤξ�� */
					/*	% quasi88 noexist file	 */
    err1 = disk_insert( DRIVE_2,
			file_disk[1],
			(image_disk[1]<0) ? 0 : image_disk[1],
			readonly_disk[1] );
  }



  /* �����ץ󤷤ʤ��ä�(����ʤ��ä�)���ϡ��ե�����̾�򥯥ꥢ */
  if( err0 ) memset( file_disk[ 0 ], 0, QUASI88_MAX_FILENAME );
  if( err1 ) memset( file_disk[ 1 ], 0, QUASI88_MAX_FILENAME );


  /* �ե�����̾�ˤ��碌�ơ����ʥåץ���åȥե�����̾������ */
  if( filename_synchronize ){
    if( err0 == FALSE || err1 == FALSE ){
      set_state_filename( FALSE );
      set_snapshot_filename( FALSE );
    }
  }

  if( verbose_proc ){
    int i;
    for( i=0; i<2; i++ ){
      if( disk_image_exist(i) ){
	printf("DRIVE %d: <= %s [%d]\n", i+1,
	     /*drive[i].filename, disk_image_selected(i)+1 );*/
	       file_disk[i],      disk_image_selected(i)+1 );
      }else{
	printf("DRIVE %d: <= (empty)\n", i+1 );
      }
    }
  }
}




/*----------------------------------------------------------------------
 * �Ƽ��ѿ������ (������PC8801�ΥС������ˤ�äơ��Ѥ����)
 *
 *----------------------------------------------------------------------*/
static	void	bootup_work_init( void )
{

	/* V1�⡼�ɤΥС������ξ������ʲ������ѹ����� */

  if( set_version ) ROM_VERSION = set_version;



	/* ��ư�ǥХ���(ROM/DISK)̤��λ� */

  if( boot_from_rom==BOOT_AUTO ){
    if( disk_image_exist(0) ) boot_from_rom = FALSE; /* �ǥ�������������DISK */
    else                      boot_from_rom = TRUE;  /* ����ʳ��ϡ�    ROM  */
  }


	/* ��ư���� BASIC�⡼��̤��λ�	  */

  if( boot_basic==BASIC_AUTO ){			
    if( ROM_VERSION >= '4' )			/* SR �ʹߤϡ�V2	  */
      boot_basic = BASIC_V2;
    else					/* ��������ϡ�V1S	  */
      boot_basic = BASIC_V1S;
  }


	/* �������(I/II)�Υݡ��Ȥ�����	 */

  if( sound_board == SOUND_II ){

    if     ( ROM_VERSION >= '8' )		/* FH/MH �ʹߤϡ�44��47H */
      sound_port = SD_PORT_44_45 | SD_PORT_46_47;
    else if( ROM_VERSION >= '4' )		/* SR �ʹߤϡ�44��45,A8��ADH */
      sound_port = SD_PORT_44_45 | SD_PORT_A8_AD;
    else					/* ��������ϡ�  A8��ADH */
      sound_port = SD_PORT_A8_AD;

  }else{

    if( ROM_VERSION >= '4' )			/* SR�ʹߤϡ�44��45H	 */
      sound_port = SD_PORT_44_45;
    else					/* ��������ϡ�������	 */
      sound_port = SD_PORT_A8_AD;		/*	�б����ʤ��ʤ� 0 */
  }

}





/***********************************************************************
 *	��¿�ʴؿ�
 ************************************************************************/
#include <string.h>

/*===========================================================================
 * ��ʸ������ʸ���ζ��̤ʤ���ʸ������� (stricmp/strcasecmp ?)
 *	�����: ���׻� == 0, �԰��׻� != 0 (�羮��ӤϤʤ�)
 *===========================================================================*/
int	my_strcmp( const char *s, const char *d )
{
  if( s==NULL || d==NULL ) return 1;

  while( tolower(*s) == tolower(*d) ){
    if( *s == '\0' ) return 0;
    s++;
    d++;
  }
  return 1;
}

/*===========================================================================
 * ʸ���� ct �� ʸ���� s �� ���ԡ� (strlcpy ?)
 *	s ��ʸ����ü�ϡ�ɬ�� '\0' �Ȥʤꡢs ��Ĺ���� n-1 ʸ���ʲ��˼��ޤ롣
 *	;ʬ���ΰ�� \0 �������ʤ���
 *	�����: �ʤ�
 *===========================================================================*/
void	my_strncpy( char *s, const char *ct, unsigned long n )
{
  s[0] = '\0';
  strncat( s, ct, n-1 );
}

/*===========================================================================
 * ʸ���� ct �� ʸ���� s �� Ϣ�� (strlcat ?)
 *	s ��ʸ����ü�ϡ�ɬ�� '\0' �Ȥʤꡢs ��Ĺ���� n-1 ʸ���ʲ��˼��ޤ롣
 *	�����: �ʤ�
 *===========================================================================*/
void	my_strncat( char *s, const char *ct, unsigned long n )
{
  size_t used = strlen(s) + 1;

  if( n > used )
    strncat( s, ct, n - used );
}

/*===========================================================================
 * SJIS �� EUC ���Ѵ� (���ʤ�Ŭ��)
 *	*sjis_p ��ʸ����� EUC ���Ѵ����ơ�*euc_p �˳�Ǽ���롣
 *
 *	��ա��ˤ��δؿ��ϡ��Хåե����դ������å����Ƥ��ʤ���
 *		*euc_p �ϡ�*sjis_p ���ܰʾ��Ĺ�����ʤ��ȴ�
 *===========================================================================*/
void	sjis2euc( char *euc_p, const char *sjis_p )
{
  int	h,l, h2, l2;

  while( ( h = (unsigned char)*sjis_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      *euc_p ++ = h;

    }else if( 0xa1 <= h && h <= 0xdf ){		/* Ⱦ�ѥ��� */

      *euc_p ++ = (char)0x8e;
      *euc_p ++ = h;

    }else{					/* ����ʸ�� */

      if( ( l = (unsigned char)*sjis_p++ ) ){

	if( l <= 0x9e ){
	  if( h <= 0x9f ) h2 = (h - 0x71) *2 +1;
	  else            h2 = (h - 0xb1) *2 +1;
	  if( l >= 0x80 ) l2 = l - 0x1f -1;
	  else            l2 = l - 0x1f;
	}else{
	  if( h <= 0x9f ) h2 = (h - 0x70) *2;
	  else            h2 = (h - 0xb0) *2;
	  l2 = l - 0x7e;
	}
	*euc_p++ = 0x80 | h2;
	*euc_p++ = 0x80 | l2;

      }else{
	break;
      }

    }
  }

  *euc_p = '\0';
}


/*===========================================================================
 * EUC �� SJIS ���Ѵ� (���ʤ�Ŭ��)
 *	*euc_p ��ʸ����� SJIS ���Ѵ����ơ�*sjis_p �˳�Ǽ���롣
 *
 *	��ա��ˤ��δؿ��ϡ��Хåե����դ������å����Ƥ��ʤ���
 *		*sjis_p �ϡ�*euc_p ��Ʊ���ʾ��Ĺ�����ʤ��ȴ�
 *===========================================================================*/

void	euc2sjis( char *sjis_p, const char *euc_p )
{
  int	h,l;

  while( ( h = (unsigned char)*euc_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      *sjis_p ++ = h;

    }else if( h==0x8e ){			/* Ⱦ�ѥ��� */

      if( ( h = (unsigned char)*euc_p++ ) ){

	if( 0xa1 <= h && h <= 0xdf )
	  *sjis_p ++ = h;

      }else{
	break;
      }

    }else if( h & 0x80 ){			/* ����ʸ�� */

      if( ( l = (unsigned char)*euc_p++ ) ){

	if( l & 0x80 ){

	  h = (h & 0x7f) - 0x21;
	  l = (l & 0x7f) - 0x21;

	  if( h & 0x01 ) l += 0x9e;
	  else           l += 0x40;
	  if( l >= 0x7f ) l += 1;

	  h = (h>>1) + 0x81;

	  if( h >= 0xa0 ) h += 0x40;

	  *sjis_p++ = h;
	  *sjis_p++ = l;

	}

      }else{
	break;
      }

    }
  }

  *sjis_p = '\0';
}


/*===========================================================================
 * EUCʸ�����Ĺ����׻� (���ä���Ŭ��)
 *	ASCII��Ⱦ�ѥ��ʤ�1ʸ�������Ѵ�����2ʸ���Ȥ��롣
 *	ʸ�������Ρ�\0 ��Ĺ���˴ޤ�ʤ���
 *===========================================================================*/

int	euclen( const char *euc_p )
{
  int	i = 0, h;

  while( ( h = (unsigned char)*euc_p++ ) ){

    if( h < 0x80 ){				/* ASCII */

      i++;

    }else if( h == 0x8e ){			/* Ⱦ�ѥ��� */

      euc_p ++;
      i++;

    }else{					/* ���� */

      euc_p ++;
      i += 2;

    }
  }

  return i;
}





/***********************************************************************
 * �Ƽ�ư��ѥ�᡼�����ѹ�
 *	�����δؿ��ϡ����硼�ȥ��åȥ��������䡢�����¸���Υ��٥��
 *	�����ʤɤ���ƤӽФ���뤳�Ȥ� *���* ���ꤷ�Ƥ��롣
 *
 *	��˥塼���̤�ɽ����˸ƤӽФ��ȡ���˥塼ɽ���ȿ����㤤��������
 *	�Τǡ���˥塼��ϸƤӽФ��ʤ��褦�ˡ����ߥ�¹���˸ƤӽФ��Τ�
 *	���ְ����������󡢤��ޤ�����
 *
 *	if( get_emu_mode() == EXEC ){
 *	    quasi88_disk_insert_and_reset( file, FALSE );
 *	}
 *
 ************************************************************************/

#include "intr.h"
#include "q8tk.h"

/*======================================================================
 * ����ɽ���ڤ��ؤ�			MENU/MONITOR �Ǥ��
 *======================================================================*/
void	quasi88_change_screen( void )
{
  if( graphic_system_restart() ){
    screen_buf_init();
  }

  if( get_emu_mode() == MENU ) q8tk_misc_redraw();
  else                         draw_screen_force();
}

/*======================================================================
 * ���̥��ʥåץ���å���¸
 *======================================================================*/
void	quasi88_snapshot( void )
{
  if( get_emu_mode() == EXEC ){
    if( save_screen_snapshot() ){
      status_message( 1, 60, "Snapshot saved" );
    }else{
      status_message( 1, 60, "Snapshot failed!" );
    }
  }
}

/*======================================================================
 * ���ơ�����ɽ���ڤ��ؤ�		MENU/MONITOR �Ǥ��
 *======================================================================*/
void	quasi88_status( void )
{
  int result;

  show_status = ( now_status ) ? FALSE : TRUE;	/* ɽ��̵ͭ���֤�ȿž */

  result = set_status_window();			/* ���ơ�����(��)ɽ�� */

  if( result ){					/* ���η�̡����� */

    status_reset( now_status );	/* ���ơ������Υ����ƽ���� */

    status_buf_init();		/* �����̻����ơ�����̵���ʤ顢�ä�ɬ�פ��� */

    if( get_emu_mode() == MENU ){		/* ���ͤ˽��������������� */
      if( result > 1 ) q8tk_misc_redraw();
      else             draw_status();
    }else{
      if( result > 1 ) draw_screen_force();
    }
  }
}

/*======================================================================
 * ��˥塼���ݡ���
 *======================================================================*/
void	quasi88_menu( void )
{
  if( get_emu_mode() != MENU ){
    set_emu_mode( MENU );
  }
}

void	quasi88_pause( void )
{
  if( get_emu_mode() == EXEC ){
    set_emu_mode( PAUSE );
  }
}




/*======================================================================
 * �ե졼�ॹ���å׿��ѹ� : -frameskip ���ͤ��ѹ����롣
 *		�ѹ�������ϡ����Ф餯���̤˥ե졼��졼�Ȥ�ɽ��������
 *======================================================================*/
static void change_framerate( int sign )
{
  int	i;
  char	str[32];

  static const char next[] = { 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60, };
  static const char prev[] = { 60, 30, 20, 15, 12, 10, 6, 5, 4, 3, 2, 1, };

  if( sign < 0 ){

    for( i=0; i<COUNTOF(next)-1; i++ )
      if( frameskip_rate <= next[i] ){ frameskip_rate = next[i+1];  break; }
    if( i==COUNTOF(next)-1 ) frameskip_rate = next[0];	/* �롼�פ����褦 */

  }else if( sign > 0 ){

    for( i=0; i<COUNTOF(prev)-1; i++ )
      if( frameskip_rate >= prev[i] ){ frameskip_rate = prev[i+1]; break; }
    if( i==COUNTOF(prev)-1 ) frameskip_rate = prev[0];	/* �롼�פ����褦 */

  }

  blink_ctrl_update();
  reset_frame_counter();

  sprintf( str, "FRAME RATE = %2d/sec", 60/frameskip_rate );
  status_message( 1, 60, str );					/* 1 sec */
}
void	quasi88_framerate_up( void )  { change_framerate( +1 ); }
void	quasi88_framerate_down( void ){ change_framerate( -1 ); }


/*======================================================================
 * �ܥ�塼���ѹ� : -vol ���ͤ��ѹ����롣
 *		�ѹ�������ϡ����Ф餯���̤˲��̤�ɽ��������
 *======================================================================*/
static void change_volume( int sign )
{
  char	str[32];

#ifdef USE_SOUND
  if( use_sound ){
    int diff = (sign>0) ? +1 : ( (sign<0)?-1:0);
    if (diff){
      int vol = xmame_get_sound_volume() + diff;
      if( vol >   0 ) vol = 0;
      if( vol < -32 ) vol = -32;
      xmame_set_sound_volume( vol );
    }
    
    sprintf( str, "VOLUME  %3d[db]", xmame_get_sound_volume() );
  }else
#endif
  {
    sprintf( str, "Sound not available !" );
  }
  status_message( 1, 60, str );					/* 1 sec */
}
void	quasi88_volume_up( void )  { change_volume( -1 ); }
void	quasi88_volume_down( void ){ change_volume( +1 ); }


/*======================================================================
 * �����������ѹ� : -nowait, -speed ���ͤ��ѹ����롣
 *		�ѹ�������ϡ�®�����ɽ�������롣����
 *		ɽ�����֤�®�����¸�ʤΤǡ����ɽ�����֤�����ˤʤ롣
 *======================================================================*/
static void change_wait( int sign )
{
  int	time = 60;
  char	str[32];

  if( sign==0 ){

    no_wait ^= 1;
    if( no_wait ){sprintf( str, "WAIT  OFF" ); time *= 10; }
    else          sprintf( str, "WAIT  ON" );

  }else{

    if( sign < 0 ){
      wait_rate -= 10;
      if( wait_rate < 10 ) wait_rate = 10;
    }else{
      wait_rate += 10;
      if( wait_rate > 200 ) wait_rate = 200;
    }

    sprintf( str, "WAIT  %4d[%%]", wait_rate );

    wait_vsync_reset();
  }

  status_message( 1, time, str );				/* 1 sec */
}
void	quasi88_wait_up( void )  { change_wait( +1 ); }
void	quasi88_wait_down( void ){ change_wait( -1 ); }
void	quasi88_wait_none( void ){ change_wait(  0 ); }



/*======================================================================
 * �ɥ饤�֤���Ū�˶��ξ��֤ˤ���
 *		���̤˥�å���������ɽ�����Ƥ���
 *======================================================================*/
static void change_image_empty( int drv )
{
  char	str[48];

  if( disk_image_exist( drv ) ){

    drive_set_empty( drv );
    sprintf( str, "DRIVE %d:  <<<< Eject >>>>         ", drv+1 );
  }else{
    sprintf( str, "DRIVE %d:   --  No Disk  --        ", drv+1 );
  }

  status_message( 1, 0, str );				/* �� sec */
}
void	quasi88_drv1_image_empty( void ){ change_image_empty( 0 ); }
void	quasi88_drv2_image_empty( void ){ change_image_empty( 1 ); }

/*======================================================================
 * �ɥ饤�֤Υ��᡼����(��)�Υ��᡼�����ѹ�����
 *		�ѹ�������ϡ����Ф餯���̤˥��᡼��̾��ɽ��������
 *======================================================================*/
static void change_image_change( int drv, int direction )
{
  char	str[48];
  int	img;

  if( disk_image_exist( drv ) ){

    img = disk_image_selected(drv);
    img += direction;
    if( img < 0 ) img = disk_image_num(drv)-1;
    if( img >= disk_image_num(drv) ) img = 0;

    drive_unset_empty( drv );
    disk_change_image( drv, img );


    sprintf( str, "DRIVE %d:  %-16s   %s  ",
	     drv+1,
	     drive[drv].image[ disk_image_selected(drv) ].name,
	     (drive[drv].image[ disk_image_selected(drv) ].protect)
							? "(p)" : "   " );
  }else{
    sprintf( str, "DRIVE %d:   --  No Disk  --        ", drv+1 );
  }

  status_message( 1, 60+60+30, str );			/* 2.5 sec */
}
void	quasi88_drv1_image_next( void ){ change_image_change( 0, +1 ); }
void	quasi88_drv1_image_prev( void ){ change_image_change( 0, -1 ); }
void	quasi88_drv2_image_next( void ){ change_image_change( 1, +1 ); }
void	quasi88_drv2_image_prev( void ){ change_image_change( 1, -1 ); }


/*======================================================================
 * �ǥ��������᡼���ե���������
 *		��ξ�ɥ饤�֤��������ơ��ꥻ�å�
 *		��ξ�ɥ饤�֤�����
 *		������ɥ饤�֤�����
 *		��ȿ�Хɥ饤�֤Υ��᡼���ե����������
 *		��ξ�ɥ饤�ּ��Ф�
 *		������ɥ饤�ּ��Ф�
 *======================================================================*/
int	quasi88_disk_insert_and_reset( const char *filename, int ro )
{
  if( quasi88_disk_insert_all( filename, ro ) ){
    quasi88_reset();
    return TRUE;
  }
  return FALSE;
}
int	quasi88_disk_insert_all( const char *filename, int ro )
{
  quasi88_disk_eject_all();

  if( quasi88_disk_insert( DRIVE_1, filename, 0, ro ) ){

    boot_from_rom = FALSE;

    if( disk_image_num( DRIVE_1 ) > 1 ){
      quasi88_disk_insert_A_to_B( DRIVE_1, DRIVE_2, 1 );
    }
    return TRUE;
  }
  return FALSE;
}
int	quasi88_disk_insert( int drv, const char *filename, int image, int ro )
{
  int err;

  quasi88_disk_eject( drv );

  if( strlen( filename ) < QUASI88_MAX_FILENAME ){

    err = disk_insert( drv, filename, image, ro );

    if( err == FALSE ){
      strcpy( file_disk[ drv ], filename );
      readonly_disk[ drv ] = ro;

      if( filename_synchronize ){
	set_state_filename( FALSE );
	set_snapshot_filename( FALSE );
      }
    }

    return err ? FALSE : TRUE;
  }

  return FALSE;
}
int	quasi88_disk_insert_A_to_B( int src, int dst, int img )
{
  int err;

  quasi88_disk_eject( dst );

  err = disk_insert_A_to_B( src, dst, img );

  if( err == FALSE ){
    strcpy( file_disk[ dst ], file_disk[ src ] );
    readonly_disk[ dst ] = readonly_disk[ src ];

    if( filename_synchronize ){
      set_state_filename( FALSE );
      set_snapshot_filename( FALSE );
    }
  }

  return err ? FALSE : TRUE;
}
void	quasi88_disk_eject_all( void )
{
  int drv;

  for( drv=0; drv<2; drv++ ){
    quasi88_disk_eject( drv );
  }

  boot_from_rom = TRUE;
}
void	quasi88_disk_eject( int drv )
{
  if( disk_image_exist( drv ) ){
    disk_eject( drv );
    memset( file_disk[ drv ], 0, QUASI88_MAX_FILENAME );

    if( filename_synchronize ){
      set_state_filename( FALSE );
      set_snapshot_filename( FALSE );
    }
  }
}


/*======================================================================
 * �ơ��ץ��᡼���ե���������
 *		�������ѥơ��ץ��᡼���ե����륻�å�
 *		�������ѥơ��ץ��᡼���ե����봬���ᤷ
 *		�������ѥơ��ץ��᡼���ե������곰��
 *		���������ѥơ��ץ��᡼���ե����륻�å�
 *		���������ѥơ��ץ��᡼���ե������곰��
 *======================================================================*/
int	quasi88_load_tape_insert( const char *filename )
{
  quasi88_load_tape_eject();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_tapeload( filename ) ){

    strcpy( file_tape[ CLOAD ], filename );
    return TRUE;

  }
  return FALSE;
}
int	quasi88_load_tape_rewind( void )
{
  if( sio_tape_rewind() ){

    return TRUE;

  }
  quasi88_load_tape_eject();
  return FALSE;
}
void	quasi88_load_tape_eject( void )
{
  sio_close_tapeload();
  memset( file_tape[ CLOAD ], 0, QUASI88_MAX_FILENAME );
}

int	quasi88_save_tape_insert( const char *filename )
{
  quasi88_save_tape_eject();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_tapesave( filename ) ){

    strcpy( file_tape[ CSAVE ], filename );
    return TRUE;

  }
  return FALSE;
}
void	quasi88_save_tape_eject( void )
{
  sio_close_tapesave();
  memset( file_tape[ CSAVE ], 0, QUASI88_MAX_FILENAME );
}

/*======================================================================
 * ���ꥢ�롦�ѥ��륤�᡼���ե���������
 *		�����ꥢ�������ѥե����륻�å�
 *		�����ꥢ�������ѥե������곰��
 *		�����ꥢ������ѥե����륻�å�
 *		�����ꥢ������ѥե������곰��
 *		���ץ�󥿽����ѥե����륻�å�
 *		���ץ�������ѥե����륻�å�
 *======================================================================*/
int	quasi88_serial_in_connect( const char *filename )
{
  quasi88_serial_in_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_serialin( filename ) ){

    strcpy( file_sin, filename );
    return TRUE;

  }
  return FALSE;
}
void	quasi88_serial_in_remove( void )
{
  sio_close_serialin();
  memset( file_sin, 0, QUASI88_MAX_FILENAME );
}
int	quasi88_serial_out_connect( const char *filename )
{
  quasi88_serial_out_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_serialout( filename ) ){

    strcpy( file_sout, filename );
    return TRUE;

  }
  return FALSE;
}
void	quasi88_serial_out_remove( void )
{
  sio_close_serialout();
  memset( file_sout, 0, QUASI88_MAX_FILENAME );
}
int	quasi88_printer_connect( const char *filename )
{
  quasi88_printer_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      printer_open( filename ) ){

    strcpy( file_prn, filename );
    return TRUE;

  }
  return FALSE;
}
void	quasi88_printer_remove( void )
{
  printer_close();
  memset( file_prn, 0, QUASI88_MAX_FILENAME );
}











/***********************************************************************
 * �Ƽ�ե�����Υե�ѥ������
 *	�Ƽ�����ν��� (�����¸��) ����ƤӽФ���롦�����ϥ�
 *
 *		����� �ǥ��������᡼��̾�Ρ��ե�ѥ������
 *		����� ROM���᡼��̾�Ρ�     �ե�ѥ������
 *		����� ��������ե�����̾�Ρ��ե�ѥ������
 *		����� ��������ե�����̾�Ρ��ե�ѥ������
 *
 *	�������ϡ� char * (malloc���줿�ΰ�)�����Ի��� NULL
 ************************************************************************/

/*
 * ���륤�᡼���Υե�����̾ imagename �Υǥ��쥯�ȥ����ȳ�ĥ������
 * ���������١���̾����Ф���
 * basedir �� �١���̾ �� suffix ���礷���ե�����̾���֤���
 * �����֤äƤ����ΰ�ϡ���Ū���ΰ�ʤΤ���� !
 */

static	char *get_concatenate_filename( const char *imagename,
					const char *basedir,
					const char *suffix )
{
  char *p;
  static char buf[ OSD_MAX_FILENAME ];
        char file[ OSD_MAX_FILENAME ];

  if( osd_path_split( imagename, buf, file, OSD_MAX_FILENAME ) ){

    size_t len = strlen( file );

    if( len >= 4 ){
      if      ( strcmp( &file[ len-4 ], ".d88" ) == 0 ||
		strcmp( &file[ len-4 ], ".D88" ) == 0 ){

	file[ len-4 ] = '\0';
      }
/*
      else if( strcmp( &file[ len-4 ], ".t88" ) == 0 ||
	       strcmp( &file[ len-4 ], ".T88" ) == 0 ||
	       strcmp( &file[ len-4 ], ".cmt" ) == 0 ||
	       strcmp( &file[ len-4 ], ".CMT" ) == 0 ){

	file[ len-4 ] = '\0';
      }
*/
    }

    if( strlen(file) + strlen(suffix) + 1 < OSD_MAX_FILENAME ){

      strcat( file, suffix );

      if( osd_path_join( basedir, file, buf, OSD_MAX_FILENAME ) ){
	return buf;
      }
    }
  }

  return NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

char	*alloc_diskname( const char *filename )
{
  char *p;
  char dir [ OSD_MAX_FILENAME ];
  char file[ OSD_MAX_FILENAME ];
  const char *base;
  OSD_FILE *fp;
  int step;

		/* filename �� dir �� file ��ʬ���� */

  if( osd_path_split( filename, dir, file, OSD_MAX_FILENAME ) ){

    if( dir[0] == '\0' ){
		/* dir �������Ĥޤ� filename �˥ѥ��ζ��ڤ꤬�ޤޤ�ʤ� */

      step = 0;		/* dir_disk + filename �� �ե�����̵ͭ��Ƚ��	*/

    }else{
		/* filename �˥ѥ����ڤ꤬�ޤޤ��	�ޤ���		*/
		/* �嵭 step 0 �ǡ��ե����뤬̵���ä����		*/

      step = 1;		/* dir_cwd + filename �� �ե�����̵ͭ������å� */
			/*	( filename�����Хѥ��ʤ顢 filename     */
			/*	  ���Τ�Τǥե�����̵ͭ�����å��Ȥʤ�)	*/
    }

  }else{
    return NULL;
  }


		/* step 0 �� step 1 �ν�ˡ��ե�����̵ͭ�����å� */

  for( ; step < 2; step ++ ){

    if( step == 0 ) base = osd_dir_disk();
    else            base = osd_dir_cwd();

    if( base==NULL ) continue;

    if( osd_path_join( base, filename, file, OSD_MAX_FILENAME ) == FALSE ){
      return NULL;
    }

			/* �ºݤ� open �Ǥ��뤫������å����� */
    fp = osd_fopen( FTYPE_DISK, file, "rb" );
    if( fp ){
      osd_fclose( fp );

      p = (char *)malloc( strlen(file) + 1 );
      if( p ){
	strcpy( p, file );
	return p;
      }
      break;
    }
  }

  return NULL;
}





char	*alloc_romname( const char *filename )
{
  char *p;
  char buf[ OSD_MAX_FILENAME ];
  OSD_FILE *fp;
  int step;
  const char *dir = osd_dir_rom(); 

	/* step 0 �� filename�����뤫�����å�			*/
	/* step 1 �� dir_rom �ˡ� filename �����뤫�����å�	*/

  for( step=0; step<2; step++ ){

    if( step==0 ){

      if( OSD_MAX_FILENAME <= strlen(filename) ) return NULL;
      strcpy( buf, filename );

    }else{

      if( dir == NULL ||
	  osd_path_join( dir, filename, buf, OSD_MAX_FILENAME ) == FALSE ){

	return NULL;
      }
    }

		/* �ºݤ� open �Ǥ��뤫������å����� */
    fp = osd_fopen( FTYPE_ROM, buf, "rb" );
    if( fp ){
      osd_fclose( fp );

      p = (char *)malloc( strlen(buf) + 1 );
      if( p ){
	strcpy( p, buf );
	return p;
      }
      break;
    }
  }
  return NULL;
}





char	*alloc_global_cfgname( void )
{
  const char *dir  = osd_dir_gcfg();
  const char *file = CONFIG_FILENAME  CONFIG_SUFFIX;
  char *p;
  char buf[ OSD_MAX_FILENAME ];


  if( dir == NULL ||
      osd_path_join( dir, file, buf, OSD_MAX_FILENAME ) == FALSE )

    return NULL;

  p = (char *)malloc( strlen(buf) + 1 );
  if( p ){
    strcpy( p, buf );
    return p;
  }

  return NULL;
}

char	*alloc_local_cfgname( const char *imagename )
{
  char *p   = NULL;
  char *buf;
  const char *dir = osd_dir_lcfg();

  if( dir==NULL ) return NULL;

  buf = get_concatenate_filename( imagename, dir, CONFIG_SUFFIX );

  if( buf ){
    p = (char *)malloc( strlen(buf) + 1 );
    if( p ){
      strcpy( p, buf );
    }
  }
  return p;
}




/***********************************************************************
 * ���ơ��ȥե����롢���ʥåץ���åȥե�����Υե�����̾�ˡ�
 * ���ʸ����򥻥åȤ��롣
 *
 *
 *
 *
 ************************************************************************/

int	set_state_filename( int init )
{
  int result = FALSE;
  char *s, *buf;
  const char *dir;

  dir = osd_dir_state();
  if( dir==NULL ) dir = osd_dir_cwd();

  memset( file_state, 0, QUASI88_MAX_FILENAME );

  if( init == FALSE ){
    if     ( file_disk[0][0]     != '\0' ) s = file_disk[0];
    else if( file_disk[1][0]     != '\0' ) s = file_disk[1];
/*  else if( file_tape[CLOAD][0] != '\0' ) s = file_tape[CLOAD];*/
    else                                   s = STATE_FILENAME;
  }else{
    s = STATE_FILENAME;
  }

  buf = get_concatenate_filename( s, dir, STATE_SUFFIX );

  if( buf ){
    if( strlen( buf ) < QUASI88_MAX_FILENAME ){

      strcpy( file_state, buf );
      result = TRUE;
    }
  }

  if( result == FALSE ){
    strcpy( file_state, STATE_FILENAME STATE_SUFFIX );
  }
  return result;
}



int	set_snapshot_filename( int init )
{
  int result = FALSE;
  char *s, *buf;
  const char *dir;

  dir = osd_dir_snap();
  if( dir==NULL ) dir = osd_dir_cwd();

  memset( file_snap, 0, QUASI88_MAX_FILENAME );

  if( init == FALSE ){
    if     ( file_disk[0][0]     != '\0' ) s = file_disk[0];
    else if( file_disk[1][0]     != '\0' ) s = file_disk[1];
/*  else if( file_tape[CLOAD][0] != '\0' ) s = file_tape[CLOAD];*/
    else                                   s = SNAPSHOT_FILENAME;
  }else{
    s = SNAPSHOT_FILENAME;
  }

  buf = get_concatenate_filename( s, dir, "" );

  if( buf ){
    if( strlen( buf ) < QUASI88_MAX_FILENAME ){

      strcpy( file_snap, buf );
      result = TRUE;
    }
  }

  if( result == FALSE ){
    strcpy( file_snap, SNAPSHOT_FILENAME );
  }
  return result;
}
