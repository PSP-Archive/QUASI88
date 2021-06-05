/***********************************************************************
 * ����ե��å����� (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� graph.h ����
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


#include <pspgu.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psprtc.h>


#include "quasi88.h"
#include "graph.h"
#include "device.h"

#include "screen.h"
#include "emu.h"	/* get_emu_mode() */

#include "ptk_video.h"
#include "ptk_softkey.h"
#include "ptk_menu.h"
#include "ptk_popup.h"
#include "ptk_font.h"


#define	BIT_OF_DEPTH	16		/* ���ϡ�16bpp �Ǹ���		*/
#define	SIZE_OF_DEPTH	2

/************************************************************************/

int	now_screen_size;		/* ���ߤΡ����̥�����		*/

int	enable_fullscreen = TRUE;	/* ������ɽ����ǽ���ɤ���	*/
int	now_fullscreen	  = FALSE;	/* ���ߡ�������ɽ����ʤ鿿	*/

int	enable_half_interp = TRUE;	/* HALF��������ֲ�ǽ���ɤ���	*/
int	now_half_interp    = FALSE;	/* ���ߡ����䴰��ʤ鿿		*/

static	int	screen_bx;		/* �ܡ�����(��)������ x(�ɥå�)	*/
static	int	screen_by;		/*		      y(�ɥå�)	*/

	int	SCREEN_DX = 0;		/* ������ɥ�����ȡ�		*/
	int	SCREEN_DY = 0;		/* ���̥��ꥢ����ȤΥ��ե��å�	*/



int	now_status = FALSE;		/* ���ߡ����ơ�����ɽ����ʤ鿿	*/
int	status_fg = 0x000000;		/* ���ơ��������ʿ�		*/
int	status_bg = 0xd6d6d6;		/* ���ơ������طʿ�		*/


int	mouse_rel_move;			/* �ޥ������а�ư�̸��β�ǽ��	*/
int	use_hwsurface	= TRUE;		/* HW SURFACE ��Ȥ����ɤ���	*/
int	use_doublebuf	= FALSE;	/* ���֥�Хåե���Ȥ����ɤ���	*/
int	use_swcursor	= TRUE;		/* ��˥塼�ǥ�������ɽ�����뤫	*/


/******************************************************************************

                          WIDTH
	 ������������������������������������������
	�������������������������������������������� ��
	��              ��                        �� ��
	��              ��SCREEN_DY               �� ��
	��              ��                        �� ��
	��������������������������������������    �� ��
	��   SCREEN_DX  ��  ��              ��    �� ��
	��              ����������������������    �� ��HEIGHT
	��              ��  ��   SCREEN_SX  ��    �� ��
	��              ��  ��              ��    �� ��
	��              ��  ��SCREEN_SY     ��    �� ��
	��              ��  ��              ��    �� ��
	��              ����������������������    �� ��
	��                                        �� ��
	�������������������������������������������� ��
	�����ơ�����0 �����ơ�����1 �����ơ�����2 �� ��STATUS_HEIGHT
	�������������������������������������������� ��
	    ���ơ�����0��2�Υ�������Ψ�ϡ� 1:3:1

******************************************************************************/

static	int	open_display( int first_time );
/*static void	close_display( void );*/


/********
 PSP Video Functions
 ********/
 
 
#define printf	pspDebugScreenPrintf
#define gotoxy(x,y)  pspDebugScreenSetXY((x),(y))
 
typedef struct
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
} t_vertex;

/************************************************************************/
/* PSP �����ƥ�����							*/
/************************************************************************/


u64 psp_lasttick;
u32 psp_tickpersec;

void	psp_system_init( void )
{
	video_init();
	video_on();
	video_pixclear();
	
	sceKernelDcacheWritebackAll();
	
	sceRtcGetCurrentTick(&psp_lasttick);
	psp_tickpersec = sceRtcGetTickResolution();
}




/************************************************************************/
/* PSP �����ƥཪλ							*/
/************************************************************************/
void	psp_system_term( void )
{
}




/************************************************************************/
/* ����ե��å������ƥ�ν����						*/
/************************************************************************/
int	graphic_system_init( void )
{
  int w, h;

  black_pixel   = 0x00000000;
  DEPTH         = BIT_OF_DEPTH;


  /* screen_size, WIDTH, HEIGHT �˥��ޥ�ɥ饤��ǻ��ꤷ��������ɥ���������
     ���åȺѤߤʤΤǡ�������Ȥ˥ܡ�����(��)�Υ������򻻽Ф��� */

  screen_size = SCREEN_SIZE_HALF;
  WIDTH = BUF_WIDTH;
  HEIGHT = SCR_HEIGHT - STATUS_HEIGHT;

  w = screen_size_tbl[ screen_size ].w;
  h = screen_size_tbl[ screen_size ].h;

  screen_bx = ( ( MAX( WIDTH,  w ) - w ) / 2 ) & ~7;	/* 8���ܿ� */
  screen_by = ( ( MAX( HEIGHT, h ) - h ) / 2 ) & ~1;	/* 2���ܿ� */

  return open_display( TRUE );
}


#define	SET_STATUS_COLOR( n, r, g, b )				\
		status_pixel[ n ] = (((r)>>3) & 0x1f) << 11 |	\
				    (((g)>>3) & 0x1f) <<  6 |	\
				    (((b)>>3) & 0x1f)



static	int	open_display( int first_time )
{
  int size;
  int flags;

	/* ������ɥ��򳫤� */

  if( use_fullscreen ){				/* �ե륹���꡼��ɽ���ξ�� */

    if( verbose_proc ) printf( "  Trying full screen mode ... " );

    size      = SCREEN_SIZE_FULL;
    SCREEN_W  = screen_size_tbl[ size ].w;
    SCREEN_H  = screen_size_tbl[ size ].h;
    SCREEN_DX = screen_size_tbl[ size ].dw;
    SCREEN_DY = screen_size_tbl[ size ].dh;
    WIDTH     = SCREEN_W + SCREEN_DX * 2;
    HEIGHT    = SCREEN_H + SCREEN_DY * 2;
    }

  if( ! use_fullscreen ){			/* ������ɥ�ɽ���ξ�� */

    size      = screen_size;
    SCREEN_W  = screen_size_tbl[ size ].w;
    SCREEN_H  = screen_size_tbl[ size ].h;
    SCREEN_DX = screen_bx;
    SCREEN_DY = screen_by;
    WIDTH     = SCREEN_W + SCREEN_DX * 2;
    HEIGHT    = SCREEN_H + SCREEN_DY * 2;

	now_fullscreen = FALSE;
	
  }

  now_screen_size = size;

  screen_buf = (char *)video_getteximage();
  
  /* �����꡼��Хåե��Ρ����賫�ϰ��֤�����	*/
  
  screen_start = &screen_buf[ (WIDTH*SCREEN_DY + SCREEN_DX) * SIZE_OF_DEPTH ];



  /* ���ơ������ѤΥХåե��ʤɤ򻻽� */
  {
    status_sx[0] = WIDTH / 5;
    status_sx[1] = WIDTH - status_sx[0]*2;
    status_sx[2] = WIDTH / 5;

    status_sy[0] = 
    status_sy[1] = 
    status_sy[2] = STATUS_HEIGHT - 3;

    status_buf = &screen_buf[ WIDTH * HEIGHT * SIZE_OF_DEPTH ];

    status_start[0] = status_buf + 3*(WIDTH * SIZE_OF_DEPTH);	/* 3�饤�� */
    status_start[1] = status_start[0] + ( status_sx[0] * SIZE_OF_DEPTH );
    status_start[2] = status_start[1] + ( status_sx[1] * SIZE_OF_DEPTH );
  }


  /* �ޥ���ɽ��������֤����� (�Ĥ��Ǥ˥�����ԡ��Ȥ�) */
  set_mouse_state();


  /* ���ơ������Ѥο��ԥ��������� */

  SET_STATUS_COLOR( STATUS_BG,    ((status_bg>>16)&0xff),
				  ((status_bg>> 8)&0xff),
				  ((status_bg    )&0xff) );
  SET_STATUS_COLOR( STATUS_FG,    ((status_fg>>16)&0xff),
				  ((status_fg>> 8)&0xff),
				  ((status_fg    )&0xff) );
  SET_STATUS_COLOR( STATUS_BLACK, 0x00, 0x00, 0x00 );
  SET_STATUS_COLOR( STATUS_WHITE, 0xff, 0xff, 0xff );
  SET_STATUS_COLOR( STATUS_RED,   0xff, 0x00, 0x00 );
  SET_STATUS_COLOR( STATUS_GREEN, 0x00, 0xff, 0x00 );


  /* �ɤ����ǽ�������ͤ� */
  if( use_joydevice ){
    joy_init();
  }

  /* HALF���������ο��䴰̵ͭ������ */
  set_half_interp();

  now_status = show_status;

  return(1);
}



/************************************************************************/
/* ����ե��å������ƥ�ν�λ						*/
/************************************************************************/
void	graphic_system_term( void )
{
}



/************************************************************************/
/* ����ե��å������ƥ�κƽ����					*/
/*	screen_size, use_fullscreen ��ꡢ�����ʥ�����ɥ����������롣	*/
/*	�ƽ�����˼��Ԥ����Ȥ��ϡ��ɤ����褦��ʤ��Τǡ�������λ���롣	*/
/************************************************************************/
int	graphic_system_restart( void )
{
  if( now_fullscreen && use_fullscreen ){	/* �����̤ΤޤޤǤ� */
    set_half_interp();
    return 0;

  }else if( now_fullscreen != use_fullscreen ){	/* �����̢���������ɥ����� */



  }else{					/* ������ɥ��������ѹ� */

    if( now_screen_size == screen_size ){	/* ������Ʊ���ʤ餳�Τޤ� */
      set_half_interp();
      return 0;
    }

    /* VIDEO ��λ����ȥ�����ɥ����˴������������ݵƫ�����ΤǤ��ʤ� */
    /* SDL_QuitSubSystem( SDL_INIT_VIDEO ); */
  }

  if( ! open_display( FALSE ) ){
    fprintf(stderr,"Sorry : Graphic System Fatal Error !!!\n");

    quasi88_exit();
  }

  return 1;
}



/************************************************************************/
/* �ѥ�å�����								*/
/************************************************************************/
void	trans_palette( SYSTEM_PALETTE_T syspal[] )
{
  int     i, j;

	/* �ѥ�å��ͤ򥳥ԡ� */
	
  // PSP5551 = ABBBBBGGGGGRRRRR

  for( i=0; i<16; i++ ){
    color_pixel[i] = (((syspal[i].red  >>3)&0x1f))|
		     (((syspal[i].green>>3)&0x1f) << 5)|
		     (((syspal[i].blue >>3)&0x1f)<<10);
  }


	/* HALF�������ե��륿��󥰲�ǽ���ϥե��륿�ѥ�å��ͤ�׻� */

  if( now_half_interp ){
    SYSTEM_PALETTE_T hpal[16];
    for( i=0; i<16; i++ ){
      hpal[i].red   = syspal[i].red   >> 1;
      hpal[i].green = syspal[i].green >> 1;
      hpal[i].blue  = syspal[i].blue  >> 1;
    }

    for( i=0; i<16; i++ ){
      color_half_pixel[i][i] = color_pixel[i];
    }
    for( i=0; i<16; i++ ){
      for( j=i+1; j<16; j++ ){
	color_half_pixel[i][j]=((((hpal[i].red  +hpal[j].red  )>>3)&0x1f))|
			       ((((hpal[i].green+hpal[j].green)>>3)&0x1f)<< 5)|
			       ((((hpal[i].blue +hpal[j].blue )>>3)&0x1f)<<10);
	color_half_pixel[j][i] = color_half_pixel[i][j];
      }
    }
  }
}




/************************************************************************/
/* ����ɽ��								*/
/************************************************************************/
/*
 *	�ܡ�����(��)�����ơ�������ޤ�����Ƥ�ɽ������
 */

int total_cnt = 0;
int frame_cnt = 0;
int latest_fps = 0;
void	put_image_all( void )
{
	u64 psp_curtick;
	
	sceRtcGetCurrentTick(&psp_curtick);
	if (psp_curtick > psp_lasttick + psp_tickpersec)
	{
		latest_fps = frame_cnt;
		psp_lasttick = psp_curtick;
		frame_cnt = 0;
	}
	frame_cnt++;
	total_cnt++;

#if 0	
	if (pmenu_video_current != pmenu_type_fs)
	{
		video_clear();
		video_swapbuffer(0);
		video_clear();
		video_swapbuffer(0);
		pmenu_video_current = pmenu_type_fs;
	}
#endif

	switch(pmenu_type_fs)
	{
		case 2:
			if (pmenu_status())
			{
				video_side_clear();
			}

			video_putimage_truewide(SCREEN_DX,SCREEN_DY,SCREEN_W,SCREEN_H);
		break;
		case 1:
			video_putimage_full(SCREEN_DX,SCREEN_DY,SCREEN_W,SCREEN_H);
		break;
		default:
			video_putimage();
		break;
	}

	disp_swkbd();
	pmenu_disp();
	pop_disp();
	
	if (pmenu_showfps)
	{
		font_printf(0,0,"fps: %d",latest_fps);
		font_printf(0,8,"total: %d",total_cnt);
	}
	video_swapbuffer(0);
}


/*
 *	VRAM�� (x0,y0)-(x1,y1) ����� ���ꤵ�줿���ơ�������ɽ������
 */
void	put_image( int x0, int y0, int x1, int y1, int st0, int st1, int st2 )
{
}





/************************************************************************/
/* ���ơ�������ɽ������ɽ���ڤ��ؤ�					*/
/*	show_status �˴�Ť����ڤ��ؤ���				*/
/*		������ɥ����ϡ�������ɥ����������ѹ�����		*/
/*		�����̻��ϡ������ʤ��ΤϤ�������			*/
/*	�������Ѥʤ� 0 �򡢺��������פʤ� 1 �򡢺�����ɬ�פʤ� 2 ���֤�	*/
/************************************************************************/
int	set_status_window( void )
{
 return 0;
}



/************************************************************************/
/* HALF���������ο��䴰��ͭ����̵��������				*/
/************************************************************************/
int	set_half_interp( void )
{
  if( now_screen_size == SCREEN_SIZE_HALF &&
      use_half_interp ){

    now_half_interp = TRUE;

  }else{
    now_half_interp = FALSE;
  }

  return now_half_interp;
}



/************************************************************************/
/* ������ɥ��ΥС���ɽ�����륿���ȥ������				*/
/************************************************************************/
void	set_window_title( const char *title )
{
}



/************************************************************************/
/* �ޥ����Υ���֡�ɽ�������ꤹ��ؿ�					*/
/*	�����Х��ѿ� grab_mouse �� hide_mouse �˴�Ť������ꤹ��	*/
/*									*/
/*	�Ĥ��Ǥʤ�ǡ�������ԡ��Ȥʤɤ����ꤷ�Ƥ��ޤ�����		*/
/************************************************************************/
int	set_mouse_state( void )
{
  return 0;
}
