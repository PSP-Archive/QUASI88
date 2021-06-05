/***********************************************************************
 * ����ե��å����� (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� graph.h ����
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"

#include "screen.h"
#include "emu.h"	/* get_emu_mode() */




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



static	SDL_Surface	*display;
	SDL_Surface	*offscreen;

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



/************************************************************************/
/* SDL �����ƥ�����							*/
/************************************************************************/
void	sdl_system_init( void )
{
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ){
    fprintf( stderr, "SDL Error: %s\n",SDL_GetError() );
  }
}




/************************************************************************/
/* SDL �����ƥཪλ							*/
/************************************************************************/
void	sdl_system_term( void )
{
  SDL_Quit();
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
  Uint32 flags;

  if( verbose_proc ){
    if( first_time ) printf("Initializing Graphic System (SDL)");
    else             printf("Restarting Graphic System (SDL)");
  }


	/* �ǥ����ץ쥤�򳫤� */

  if( ! SDL_WasInit( SDL_INIT_VIDEO ) ){
    if( SDL_InitSubSystem( SDL_INIT_VIDEO ) != 0 ){
      if( verbose_proc ) printf(" ... FAILED\n");
      return 0;
    }
  }
  if( verbose_proc ) printf("\n");


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

    if( use_hwsurface ) flags = SDL_HWPALETTE | SDL_HWSURFACE | SDL_FULLSCREEN;
    else                flags = SDL_HWPALETTE | SDL_SWSURFACE | SDL_FULLSCREEN;

    if( use_doublebuf )
      flags |= SDL_DOUBLEBUF;

    display = SDL_SetVideoMode( WIDTH, HEIGHT, DEPTH, flags );

    if( verbose_proc ) printf( "%s\n", (display ? "OK" : "FAILED") );

    if( display ){ now_fullscreen = TRUE;   HEIGHT -= STATUS_HEIGHT; }
    else         { use_fullscreen = FALSE; }
  }

  if( ! use_fullscreen ){			/* ������ɥ�ɽ���ξ�� */

    if( verbose_proc ) printf( "  Opening window ... " );

    size      = screen_size;
    SCREEN_W  = screen_size_tbl[ size ].w;
    SCREEN_H  = screen_size_tbl[ size ].h;
    SCREEN_DX = screen_bx;
    SCREEN_DY = screen_by;
    WIDTH     = SCREEN_W + SCREEN_DX * 2;
    HEIGHT    = SCREEN_H + SCREEN_DY * 2;

    if( use_hwsurface ) flags = SDL_HWPALETTE | SDL_HWSURFACE;
    else                flags = SDL_HWPALETTE | SDL_SWSURFACE;

    if( use_doublebuf )
      flags |= SDL_DOUBLEBUF;

    display = SDL_SetVideoMode( WIDTH,
				HEIGHT + ((show_status) ? STATUS_HEIGHT : 0),
				DEPTH, flags );

    if( verbose_proc ) printf( "%s\n", (display ? "OK" : "FAILED") );

    if( display ){ now_fullscreen = FALSE; }
    else         { return 0;               }
  }

  now_screen_size = size;

  if( verbose_proc )
    printf("    VideoMode %dx%d->%dx%dx%d(%d)  %c%c%c%c  R:%x G:%x B:%x\n",
	   WIDTH, HEIGHT,
	   display->w, display->h,
	   display->format->BitsPerPixel, display->format->BytesPerPixel,
	   (display->flags & SDL_SWSURFACE) ? 'S' : '-',
	   (display->flags & SDL_HWSURFACE) ? 'H' : 'S',
	   (display->flags & SDL_DOUBLEBUF) ? 'D' : '-',
	   (display->flags & SDL_FULLSCREEN) ? 'F' : '-',
	 display->format->Rmask,display->format->Gmask,display->format->Bmask);


  /* ������ɥ��Υ����ȥ��ɽ�� */
  SDL_WM_SetCaption( Q_TITLE " ver " Q_VERSION, Q_TITLE " ver " Q_VERSION );

  /* ������������ꤹ��ʤ餳���ǡ�WIN32�ξ�硢32x32 �˸¤� */
  /* SDL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask); */


  /* �����꡼��Хåե������ */
  if( verbose_proc ) printf( "  Allocating screen buffer ... " );

  offscreen = SDL_CreateRGBSurface( SDL_SWSURFACE,
				    WIDTH,
				    HEIGHT + STATUS_HEIGHT,
				    DEPTH,
				    0, 0, 0, 0 );

  if( verbose_proc ) printf( "%s\n", (offscreen ? "OK" : "FAILED") );

  if( offscreen ){ screen_buf = (char *)offscreen->pixels; }
  else           { return 0;                               }


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
  SDL_WM_GrabInput( SDL_GRAB_OFF );

  SDL_QuitSubSystem( SDL_INIT_VIDEO );
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

#if 0
    if( SDL_WM_ToggleFullScreen( display ) ) return;
#endif
    SDL_QuitSubSystem( SDL_INIT_VIDEO );		/* ��ö VIDEO ��λ */

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

  for( i=0; i<16; i++ ){
    color_pixel[i] = (((syspal[i].red  >>3)&0x1f) <<11)|
		     (((syspal[i].green>>3)&0x1f) << 6)|
		     (((syspal[i].blue >>3)&0x1f));
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
	color_half_pixel[i][j]=((((hpal[i].red  +hpal[j].red  )>>3)&0x1f)<<11)|
			       ((((hpal[i].green+hpal[j].green)>>3)&0x1f)<< 6)|
			       ((((hpal[i].blue +hpal[j].blue )>>3)&0x1f));
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
void	put_image_all( void )
{
  SDL_Rect srect, drect;
  int h;

  if( now_fullscreen ) h = HEIGHT + STATUS_HEIGHT;
  else                 h = HEIGHT + ((now_status) ? STATUS_HEIGHT : 0);


  drect.x = srect.x = 0;	srect.w = WIDTH;
  drect.y = srect.y = 0;	srect.h = h;

  if( SDL_BlitSurface( offscreen, &srect, display, &drect ) < 0 ){
    fprintf( stderr, "SDL: Warn: Unsuccessful blitting\n" );
  }
  /* SDL_UpdateRect(display, 0,0,0,0); */
  SDL_Flip(display);
}


/*
 *	VRAM�� (x0,y0)-(x1,y1) ����� ���ꤵ�줿���ơ�������ɽ������
 */
void	put_image( int x0, int y0, int x1, int y1, int st0, int st1, int st2 )
{
  int      i, flag = 0, nr_update = 0;
  SDL_Rect srect[4], drect,  update[4];

  if( x0 >= 0 ){
    if      ( now_screen_size == SCREEN_SIZE_FULL ){
      ;
    }else if( now_screen_size == SCREEN_SIZE_HALF ){
      x0 /= 2;  x1 /= 2;  y0 /= 2;  y1 /= 2;
    }else  /* now_screen_size == SCREEN_SIZE_DOUBLE */ {
      x0 *= 2;  x1 *= 2;  y0 *= 2;  y1 *= 2;
    }

    flag |= 1;
    srect[0].x = SCREEN_DX + x0;	srect[0].w = x1 - x0;
    srect[0].y = SCREEN_DY + y0;	srect[0].h = y1 - y0;
  }
  if( now_status || now_fullscreen ){
    if( st0 ){
      flag |= 2;
      srect[1].x = 0;			srect[1].w = status_sx[0];
      srect[1].y = HEIGHT;		srect[1].h = STATUS_HEIGHT;
    }
    if( st1 ){
      flag |= 4;
      srect[2].x = status_sx[0];	srect[2].w = status_sx[1];
      srect[2].y = HEIGHT;		srect[2].h = STATUS_HEIGHT;
    }
    if( st2 ){
      flag |= 8;
      srect[3].x = status_sx[0] + status_sx[1];	srect[3].w = status_sx[2];
      srect[3].y = HEIGHT;			srect[3].h = STATUS_HEIGHT;
    }
  }

  for( i=0; i<4; i++ ){
    if( flag & (1<<i ) ){
      drect = srect[i];

      if( SDL_BlitSurface( offscreen, &srect[i], display, &drect ) < 0 ){
	fprintf( stderr, "SDL: Warn: Unsuccessful blitting\n" );
      }

      update[ nr_update ++ ] = srect[i];
    }
  }

  if( display->flags & SDL_DOUBLEBUF ){

    SDL_Flip( display );

    for( i=0; i<4; i++ ){
      if( flag & (1<<i ) ){
	drect = srect[i];
	SDL_BlitSurface( offscreen, &srect[i], display, &drect );
      }
    }

  }else{

    SDL_UpdateRects( display, nr_update, update );
  }
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
  if( now_status == show_status ){		/* ���ơ��������Τޤ�	*/
    return 0;
  }


  if( now_fullscreen == FALSE ){

    /* show_status �˱����ơ����̥��������Ѥ��� */

    if( ! open_display( FALSE ) ){
      fprintf(stderr,"Sorry : Graphic System Fatal Error !!!\n");

      quasi88_exit();
    }
    return 2;

  }else{

    /* �ե륹���꡼����ϡ����ơ�����ɽ��̵ͭ�˴ؤ�餺�����̥�������Ʊ�� */
    now_status = show_status;
    return 1;
  }
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
  SDL_WM_SetCaption( title, title );
}



/************************************************************************/
/* �ޥ����Υ���֡�ɽ�������ꤹ��ؿ�					*/
/*	�����Х��ѿ� grab_mouse �� hide_mouse �˴�Ť������ꤹ��	*/
/*									*/
/*	�Ĥ��Ǥʤ�ǡ�������ԡ��Ȥʤɤ����ꤷ�Ƥ��ޤ�����		*/
/************************************************************************/
int	set_mouse_state( void )
{
  int	repeat;		/* �����ȥ�ԡ��Ȥ�̵ͭ	*/
  int	mouse;		/* �ޥ���ɽ����̵ͭ	*/
  int	grab;		/* ����֤�̵ͭ		*/

  if( get_emu_mode() == EXEC ){

    repeat = FALSE;
    if( now_fullscreen || grab_mouse ){
      mouse  = FALSE;
      grab   = TRUE;
    }else{
      mouse  = (hide_mouse) ? FALSE : TRUE;
      grab   = FALSE;
    }

  }else{

    repeat = TRUE;
    mouse  = (now_fullscreen && use_swcursor) ? FALSE : TRUE;
    grab   = (now_fullscreen) ? TRUE : FALSE;
  }


  if( repeat )
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  else
    SDL_EnableKeyRepeat( 0, 0 );

  if( mouse ) SDL_ShowCursor( SDL_ENABLE );
  else        SDL_ShowCursor( SDL_DISABLE );

  if( grab ) SDL_WM_GrabInput( SDL_GRAB_ON );
  else       SDL_WM_GrabInput( SDL_GRAB_OFF );

  mouse_rel_move = (!mouse && grab) ? TRUE : FALSE;


/*printf( "K=%d M=%d G=%d\n",repeat,mouse,grab);*/

  return mouse;
}
/*
  �ե륹���꡼��������� (Win�ˤ�ȯ��)
  ���֥�Хåե��ξ�硢�ޥ����������ɽ������ʤ���
  ���󥰥�Хåե��ǥϡ��ɥ����������ե����ξ�硢
  �ޥ�����ON�ˤ����ִ֡��ޥ�����ɽ�����٤����֤˥��ߤ��Ĥ롩

  ��
  ��˥塼�������ܤ���������ʤΤǡ����եȥ�������������Ǥ��ޤ�����
*/
