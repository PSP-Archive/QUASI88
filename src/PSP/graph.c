/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *	詳細は、 graph.h 参照
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


#define	BIT_OF_DEPTH	16		/* 色は、16bpp で固定		*/
#define	SIZE_OF_DEPTH	2

/************************************************************************/

int	now_screen_size;		/* 現在の、画面サイズ		*/

int	enable_fullscreen = TRUE;	/* 全画面表示可能かどうか	*/
int	now_fullscreen	  = FALSE;	/* 現在、全画面表示中なら真	*/

int	enable_half_interp = TRUE;	/* HALF時、色補間可能かどうか	*/
int	now_half_interp    = FALSE;	/* 現在、色補完中なら真		*/

static	int	screen_bx;		/* ボーダー(枠)サイズ x(ドット)	*/
static	int	screen_by;		/*		      y(ドット)	*/

	int	SCREEN_DX = 0;		/* ウインドウ左上と、		*/
	int	SCREEN_DY = 0;		/* 画面エリア左上とのオフセット	*/



int	now_status = FALSE;		/* 現在、ステータス表示中なら真	*/
int	status_fg = 0x000000;		/* ステータス前景色		*/
int	status_bg = 0xd6d6d6;		/* ステータス背景色		*/


int	mouse_rel_move;			/* マウス相対移動量検知可能か	*/
int	use_hwsurface	= TRUE;		/* HW SURFACE を使うかどうか	*/
int	use_doublebuf	= FALSE;	/* ダブルバッファを使うかどうか	*/
int	use_swcursor	= TRUE;		/* メニューでカーソル表示するか	*/


/******************************************************************************

                          WIDTH
	 ←───────────────────→
	┌────────────────────┐ ↑
	│              ↑                        │ │
	│              │SCREEN_DY               │ │
	│              ↓                        │ │
	│←─────→┌─────────┐    │ │
	│   SCREEN_DX  │  ↑              │    │ │
	│              │←───────→│    │ │HEIGHT
	│              │  │   SCREEN_SX  │    │ │
	│              │  │              │    │ │
	│              │  │SCREEN_SY     │    │ │
	│              │  ↓              │    │ │
	│              └─────────┘    │ │
	│                                        │ ↓
	├──────┬──────┬──────┤ ↑
	│ステータス0 │ステータス1 │ステータス2 │ │STATUS_HEIGHT
	└──────┴──────┴──────┘ ↓
	    ステータス0〜2のサイズ比率は、 1:3:1

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
/* PSP システム初期化							*/
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
/* PSP システム終了							*/
/************************************************************************/
void	psp_system_term( void )
{
}




/************************************************************************/
/* グラフィックシステムの初期化						*/
/************************************************************************/
int	graphic_system_init( void )
{
  int w, h;

  black_pixel   = 0x00000000;
  DEPTH         = BIT_OF_DEPTH;


  /* screen_size, WIDTH, HEIGHT にコマンドラインで指定したウインドウサイズが
     セット済みなので、それをもとにボーダー(枠)のサイズを算出する */

  screen_size = SCREEN_SIZE_HALF;
  WIDTH = BUF_WIDTH;
  HEIGHT = SCR_HEIGHT - STATUS_HEIGHT;

  w = screen_size_tbl[ screen_size ].w;
  h = screen_size_tbl[ screen_size ].h;

  screen_bx = ( ( MAX( WIDTH,  w ) - w ) / 2 ) & ~7;	/* 8の倍数 */
  screen_by = ( ( MAX( HEIGHT, h ) - h ) / 2 ) & ~1;	/* 2の倍数 */

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

	/* ウインドウを開く */

  if( use_fullscreen ){				/* フルスクリーン表示の場合 */

    if( verbose_proc ) printf( "  Trying full screen mode ... " );

    size      = SCREEN_SIZE_FULL;
    SCREEN_W  = screen_size_tbl[ size ].w;
    SCREEN_H  = screen_size_tbl[ size ].h;
    SCREEN_DX = screen_size_tbl[ size ].dw;
    SCREEN_DY = screen_size_tbl[ size ].dh;
    WIDTH     = SCREEN_W + SCREEN_DX * 2;
    HEIGHT    = SCREEN_H + SCREEN_DY * 2;
    }

  if( ! use_fullscreen ){			/* ウインドウ表示の場合 */

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
  
  /* スクリーンバッファの、描画開始位置を設定	*/
  
  screen_start = &screen_buf[ (WIDTH*SCREEN_DY + SCREEN_DX) * SIZE_OF_DEPTH ];



  /* ステータス用のバッファなどを算出 */
  {
    status_sx[0] = WIDTH / 5;
    status_sx[1] = WIDTH - status_sx[0]*2;
    status_sx[2] = WIDTH / 5;

    status_sy[0] = 
    status_sy[1] = 
    status_sy[2] = STATUS_HEIGHT - 3;

    status_buf = &screen_buf[ WIDTH * HEIGHT * SIZE_OF_DEPTH ];

    status_start[0] = status_buf + 3*(WIDTH * SIZE_OF_DEPTH);	/* 3ライン下 */
    status_start[1] = status_start[0] + ( status_sx[0] * SIZE_OF_DEPTH );
    status_start[2] = status_start[1] + ( status_sx[1] * SIZE_OF_DEPTH );
  }


  /* マウス表示、グラブの設定 (ついでにキーリピートも) */
  set_mouse_state();


  /* ステータス用の色ピクセルを定義 */

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


  /* どこかで初期化せねば */
  if( use_joydevice ){
    joy_init();
  }

  /* HALFサイズ時の色補完有無を設定 */
  set_half_interp();

  now_status = show_status;

  return(1);
}



/************************************************************************/
/* グラフィックシステムの終了						*/
/************************************************************************/
void	graphic_system_term( void )
{
}



/************************************************************************/
/* グラフィックシステムの再初期化					*/
/*	screen_size, use_fullscreen より、新たなウインドウを生成する。	*/
/*	再初期化に失敗したときは、どうしようもないので、強制終了する。	*/
/************************************************************************/
int	graphic_system_restart( void )
{
  if( now_fullscreen && use_fullscreen ){	/* 全画面のままです */
    set_half_interp();
    return 0;

  }else if( now_fullscreen != use_fullscreen ){	/* 全画面←→ウインドウ切替 */



  }else{					/* ウインドウサイズ変更 */

    if( now_screen_size == screen_size ){	/* サイズ同じならこのまま */
      set_half_interp();
      return 0;
    }

    /* VIDEO を終了するとウインドウが破棄→生成されて鬱陶しいのでしない */
    /* SDL_QuitSubSystem( SDL_INIT_VIDEO ); */
  }

  if( ! open_display( FALSE ) ){
    fprintf(stderr,"Sorry : Graphic System Fatal Error !!!\n");

    quasi88_exit();
  }

  return 1;
}



/************************************************************************/
/* パレット設定								*/
/************************************************************************/
void	trans_palette( SYSTEM_PALETTE_T syspal[] )
{
  int     i, j;

	/* パレット値をコピー */
	
  // PSP5551 = ABBBBBGGGGGRRRRR

  for( i=0; i<16; i++ ){
    color_pixel[i] = (((syspal[i].red  >>3)&0x1f))|
		     (((syspal[i].green>>3)&0x1f) << 5)|
		     (((syspal[i].blue >>3)&0x1f)<<10);
  }


	/* HALFサイズフィルタリング可能時はフィルタパレット値を計算 */

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
/* 画面表示								*/
/************************************************************************/
/*
 *	ボーダー(枠)、ステータスも含めて全てを表示する
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
 *	VRAMの (x0,y0)-(x1,y1) および 指定されたステータスを表示する
 */
void	put_image( int x0, int y0, int x1, int y1, int st0, int st1, int st2 )
{
}





/************************************************************************/
/* ステータスを表示・非表示切り替え					*/
/*	show_status に基づいて切り替える				*/
/*		ウインドウ時は、ウインドウサイズを変更する		*/
/*		全画面時は、処理なしのはず・・・			*/
/*	状態不変なら 0 を、再描画不要なら 1 を、再描画必要なら 2 を返す	*/
/************************************************************************/
int	set_status_window( void )
{
 return 0;
}



/************************************************************************/
/* HALFサイズ時の色補完の有効・無効を設定				*/
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
/* ウインドウのバーに表示するタイトルを設定				*/
/************************************************************************/
void	set_window_title( const char *title )
{
}



/************************************************************************/
/* マウスのグラブ・表示を設定する関数					*/
/*	グローバル変数 grab_mouse 、 hide_mouse に基づき、設定する	*/
/*									*/
/*	ついでなんで、キーリピートなども設定してしまおう。		*/
/************************************************************************/
int	set_mouse_state( void )
{
  return 0;
}
