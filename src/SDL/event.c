/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/

/* ----------------------------------------------------------------------
 *		ヘッダファイル部
 * ---------------------------------------------------------------------- */

#include <SDL.h>

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



int	use_joydevice = TRUE;		/* ジョイスティックデバイスを開く? */

static	SDL_Joystick *joy = NULL;	/* NULLならジョイスティック使用不可 */


/*----------------------------------------------------------------------
 * SDL の keysym を QUASI88 の キーコードに変換するテーブル
 *----------------------------------------------------------------------*/
static int sdlk2keycode[ SDLK_LAST ] =
{
  0,				/*	SDLK_UNKNOWN		= 0,	*/
  0, 0, 0, 0, 0, 0, 0, 
  KEY88_INS_DEL,		/*	SDLK_BACKSPACE		= 8,	*/
  KEY88_TAB,			/*	SDLK_TAB		= 9,	*/
  0, 0,
  KEY88_HOME,			/*	SDLK_CLEAR		= 12,	*/
  KEY88_RETURNL,		/*	SDLK_RETURN		= 13,	*/
  0, 0, 0, 0, 0,
  KEY88_STOP,			/*	SDLK_PAUSE		= 19,	*/
  0, 0, 0, 0, 0, 0, 0,
  KEY88_ESC,			/*	SDLK_ESCAPE		= 27,	*/
  0, 0, 0, 0,

  KEY88_SPACE,          	/*	SDLK_SPACE		= 32,	*/
  KEY88_EXCLAM,         	/*	SDLK_EXCLAIM		= 33,	*/
  KEY88_QUOTEDBL,       	/*	SDLK_QUOTEDBL		= 34,	*/
  KEY88_NUMBERSIGN,     	/*	SDLK_HASH		= 35,	*/
  KEY88_DOLLAR,         	/*	SDLK_DOLLAR		= 36,	*/
  KEY88_PERCENT,        
  KEY88_AMPERSAND,      	/*	SDLK_AMPERSAND		= 38,	*/
  KEY88_APOSTROPHE,     	/*	SDLK_QUOTE		= 39,	*/
  KEY88_PARENLEFT,      	/*	SDLK_LEFTPAREN		= 40,	*/
  KEY88_PARENRIGHT,     	/*	SDLK_RIGHTPAREN		= 41,	*/
  KEY88_ASTERISK,       	/*	SDLK_ASTERISK		= 42,	*/
  KEY88_PLUS,           	/*	SDLK_PLUS		= 43,	*/
  KEY88_COMMA,          	/*	SDLK_COMMA		= 44,	*/
  KEY88_MINUS,          	/*	SDLK_MINUS		= 45,	*/
  KEY88_PERIOD,         	/*	SDLK_PERIOD		= 46,	*/
  KEY88_SLASH,          	/*	SDLK_SLASH		= 47,	*/
  KEY88_0,              	/*	SDLK_0			= 48,	*/
  KEY88_1,              	/*	SDLK_1			= 49,	*/
  KEY88_2,              	/*	SDLK_2			= 50,	*/
  KEY88_3,              	/*	SDLK_3			= 51,	*/
  KEY88_4,              	/*	SDLK_4			= 52,	*/
  KEY88_5,              	/*	SDLK_5			= 53,	*/
  KEY88_6,              	/*	SDLK_6			= 54,	*/
  KEY88_7,              	/*	SDLK_7			= 55,	*/
  KEY88_8,              	/*	SDLK_8			= 56,	*/
  KEY88_9,              	/*	SDLK_9			= 57,	*/
  KEY88_COLON,          	/*	SDLK_COLON		= 58,	*/
  KEY88_SEMICOLON,      	/*	SDLK_SEMICOLON		= 59,	*/
  KEY88_LESS,           	/*	SDLK_LESS		= 60,	*/
  KEY88_EQUAL,          	/*	SDLK_EQUALS		= 61,	*/
  KEY88_GREATER,        	/*	SDLK_GREATER		= 62,	*/
  KEY88_QUESTION,       	/*	SDLK_QUESTION		= 63,	*/
  KEY88_AT,			/*	SDLK_AT			= 64,	*/
  KEY88_A,
  KEY88_B,
  KEY88_C,
  KEY88_D,
  KEY88_E,
  KEY88_F,
  KEY88_G,
  KEY88_H,
  KEY88_I,
  KEY88_J,
  KEY88_K,
  KEY88_L,
  KEY88_M,
  KEY88_N,
  KEY88_O,
  KEY88_P,
  KEY88_Q,
  KEY88_R,
  KEY88_S,
  KEY88_T,
  KEY88_U,
  KEY88_V,
  KEY88_W,
  KEY88_X,
  KEY88_Y,
  KEY88_Z,
  KEY88_BRACELEFT,      	/*	SDLK_LEFTBRACKET	= 91,	*/
  KEY88_BACKSLASH,      	/*	SDLK_BACKSLASH		= 92,	*/
  KEY88_BRACERIGHT,     	/*	SDLK_RIGHTBRACKET	= 93,	*/
  KEY88_CARET,          	/*	SDLK_CARET		= 94,	*/
  KEY88_UNDERSCORE,     	/*	SDLK_UNDERSCORE		= 95,	*/
  KEY88_BACKQUOTE,      	/*	SDLK_BACKQUOTE		= 96,	*/
  KEY88_a,              	/*	SDLK_a			= 97,	*/
  KEY88_b,              	/*	SDLK_b			= 98,	*/
  KEY88_c,              	/*	SDLK_c			= 99,	*/
  KEY88_d,              	/*	SDLK_d			= 100,	*/
  KEY88_e,              	/*	SDLK_e			= 101,	*/
  KEY88_f,              	/*	SDLK_f			= 102,	*/
  KEY88_g,              	/*	SDLK_g			= 103,	*/
  KEY88_h,              	/*	SDLK_h			= 104,	*/
  KEY88_i,              	/*	SDLK_i			= 105,	*/
  KEY88_j,              	/*	SDLK_j			= 106,	*/
  KEY88_k,              	/*	SDLK_k			= 107,	*/
  KEY88_l,              	/*	SDLK_l			= 108,	*/
  KEY88_m,              	/*	SDLK_m			= 109,	*/
  KEY88_n,              	/*	SDLK_n			= 110,	*/
  KEY88_o,              	/*	SDLK_o			= 111,	*/
  KEY88_p,              	/*	SDLK_p			= 112,	*/
  KEY88_q,              	/*	SDLK_q			= 113,	*/
  KEY88_r,              	/*	SDLK_r			= 114,	*/
  KEY88_s,              	/*	SDLK_s			= 115,	*/
  KEY88_t,              	/*	SDLK_t			= 116,	*/
  KEY88_u,              	/*	SDLK_u			= 117,	*/
  KEY88_v,              	/*	SDLK_v			= 118,	*/
  KEY88_w,              	/*	SDLK_w			= 119,	*/
  KEY88_x,              	/*	SDLK_x			= 120,	*/
  KEY88_y,              	/*	SDLK_y			= 121,	*/
  KEY88_z,              	/*	SDLK_z			= 122,	*/
  KEY88_BRACELEFT,
  KEY88_BAR,
  KEY88_BRACERIGHT,
  KEY88_TILDE,
  KEY88_DEL,			/*	SDLK_DELETE		= 127,	*/

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
	
  KEY88_KP_0,           	/*	SDLK_KP0		= 256,	*/
  KEY88_KP_1,           	/*	SDLK_KP1		= 257,	*/
  KEY88_KP_2,           	/*	SDLK_KP2		= 258,	*/
  KEY88_KP_3,           	/*	SDLK_KP3		= 259,	*/
  KEY88_KP_4,           	/*	SDLK_KP4		= 260,	*/
  KEY88_KP_5,           	/*	SDLK_KP5		= 261,	*/
  KEY88_KP_6,           	/*	SDLK_KP6		= 262,	*/
  KEY88_KP_7,           	/*	SDLK_KP7		= 263,	*/
  KEY88_KP_8,           	/*	SDLK_KP8		= 264,	*/
  KEY88_KP_9,           	/*	SDLK_KP9		= 265,	*/
  KEY88_KP_PERIOD,		/*	SDLK_KP_PERIOD		= 266,	*/
  KEY88_KP_DIVIDE,		/*	SDLK_KP_DIVIDE		= 267,	*/
  KEY88_KP_MULTIPLY,		/*	SDLK_KP_MULTIPLY	= 268,	*/
  KEY88_KP_SUB,			/*	SDLK_KP_MINUS		= 269,	*/
  KEY88_KP_ADD,			/*	SDLK_KP_PLUS		= 270,	*/
  KEY88_RETURNR,		/*	SDLK_KP_ENTER		= 271,	*/
  KEY88_KP_EQUIAL,		/*	SDLK_KP_EQUALS		= 272,	*/
  KEY88_UP,			/*	SDLK_UP			= 273,	*/
  KEY88_DOWN,			/*	SDLK_DOWN		= 274,	*/
  KEY88_RIGHT,			/*	SDLK_RIGHT		= 275,	*/
  KEY88_LEFT,			/*	SDLK_LEFT		= 276,	*/
  KEY88_INS, 			/*	SDLK_INSERT		= 277,	*/
  KEY88_HOME,			/*	SDLK_HOME		= 278,	*/
  KEY88_HELP,			/*	SDLK_END		= 279,	*/
  KEY88_ROLLDOWN,		/*	SDLK_PAGEUP		= 280,	*/
  KEY88_ROLLUP,			/*	SDLK_PAGEDOWN		= 281,	*/
  KEY88_F1,             	/*	SDLK_F1			= 282,	*/
  KEY88_F2,             	/*	SDLK_F2			= 283,	*/
  KEY88_F3,             	/*	SDLK_F3			= 284,	*/
  KEY88_F4,             	/*	SDLK_F4			= 285,	*/
  KEY88_F5,             	/*	SDLK_F5			= 286,	*/
  KEY88_F6,             	/*	SDLK_F6			= 287,	*/
  KEY88_F7,             	/*	SDLK_F7			= 288,	*/
  KEY88_F8,             	/*	SDLK_F8			= 289,	*/
  KEY88_F9,             	/*	SDLK_F9			= 290,	*/
  KEY88_F10,            	/*	SDLK_F10		= 291,	*/
  KEY88_F11,            	/*	SDLK_F11		= 292,	*/
  KEY88_F12,            	/*	SDLK_F12		= 293,	*/
  KEY88_F13,			/*	SDLK_F13		= 294,	*/
  KEY88_F14,			/*	SDLK_F14		= 295,	*/
  KEY88_F15,			/*	SDLK_F15		= 296,	*/
  0, 0, 0,
  0,				/*	SDLK_NUMLOCK		= 300,	*/
  KEY88_CAPS,			/*	SDLK_CAPSLOCK		= 301,	*/
  KEY88_KANA,			/*	SDLK_SCROLLOCK		= 302,	*/
  KEY88_SHIFTR,			/*	SDLK_RSHIFT		= 303,	*/
  KEY88_SHIFTL,			/*	SDLK_LSHIFT		= 304,	*/
  KEY88_CTRL,			/*	SDLK_RCTRL		= 305,	*/
  KEY88_CTRL,			/*	SDLK_LCTRL		= 306,	*/
  KEY88_PC,			/*	SDLK_RALT		= 307,	*/
  KEY88_GRAPH,			/*	SDLK_LALT		= 308,	*/
  KEY88_PC,			/*	SDLK_RMETA		= 309,	*/
  KEY88_GRAPH,			/*	SDLK_LMETA		= 310,	*/
  KEY88_GRAPH,			/*	SDLK_LSUPER		= 311,	*/
  KEY88_PC,			/*	SDLK_RSUPER		= 312,	*/
  0,				/*	SDLK_MODE		= 313,	*/
  0,				/*	SDLK_COMPOSE		= 314,	*/
  KEY88_HELP,			/*	SDLK_HELP		= 315,	*/
  KEY88_COPY,			/*	SDLK_PRINT		= 316,	*/
  KEY88_COPY,			/*	SDLK_SYSREQ		= 317,	*/
  KEY88_STOP,			/*	SDLK_BREAK		= 318,	*/
  0,				/*	SDLK_MENU		= 319,	*/
  0,				/*	SDLK_POWER		= 320,	*/
  0,				/*	SDLK_EURO		= 321,	*/
  0,				/*	SDLK_UNDO		= 322,	*/
};


/******************************************************************************
 * イベントハンドリング
 *
 *	1/60毎に呼び出される。
 *****************************************************************************/

/*
 * これは 起動時に1回だけ呼ばれる
 */
void	event_handle_init( void )
{

  function_f[ 11 ] = FN_STATUS;
  function_f[ 12 ] = FN_MENU;

  /*
    TODO 
      ファイルからキーバインディングを読み込み
      ファイルからソフトNumLockのバインディングを読み込み
   */

}



/*
 * 約 1/60 毎に呼ばれる
 */
void	event_handle( void )
{
  static enum {
    AXIS_U = 0x01,
    AXIS_D = 0x02,
    AXIS_L = 0x04,
    AXIS_R = 0x08
  } pad_axis = 0x00;


  SDL_Event E;
  SDLKey keysym;
  int    key88, x, y;


  SDL_PumpEvents();		/* イベントを汲み上げる */

  while( SDL_PeepEvents(&E, 1, SDL_GETEVENT,
			SDL_EVENTMASK(SDL_KEYDOWN)        | 
			SDL_EVENTMASK(SDL_KEYUP)          |
			SDL_EVENTMASK(SDL_MOUSEMOTION)    |
			SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)|
			SDL_EVENTMASK(SDL_MOUSEBUTTONUP)  |
			SDL_EVENTMASK(SDL_JOYAXISMOTION)  |
			SDL_EVENTMASK(SDL_JOYBUTTONDOWN)  |
			SDL_EVENTMASK(SDL_JOYBUTTONUP)    |
			SDL_EVENTMASK(SDL_VIDEOEXPOSE)    |
			SDL_EVENTMASK(SDL_ACTIVEEVENT)    |
    			SDL_EVENTMASK(SDL_QUIT)) ) {

    switch( E.type ){

    case SDL_KEYDOWN:		/*------------------------------------------*/
    case SDL_KEYUP:

      keysym  = E.key.keysym.sym;

/*    if( keysym < COUNTOF(sdlk2keycode) )*/
      {
	if( get_emu_mode()==MENU ){
	  if( E.key.keysym.unicode <= 0xff &&
	      isprint( E.key.keysym.unicode ) ){
	    keysym = E.key.keysym.unicode;
	  }
	}

	key88 = sdlk2keycode[ keysym ];

	pc88_key( key88, (E.type==SDL_KEYDOWN) );
      }
      break;

    case SDL_MOUSEMOTION:	/*------------------------------------------*/
      if( mouse_rel_move ){		/* マウスがウインドウの端に届いても */
					/* 相対的な動きを検出できる場合     */
	x = E.motion.xrel;
	y = E.motion.yrel;
#if 0
	if( get_emu_mode()==EXEC ){	/* マウスの速度調整が可能だけども */
	  x /= 10;			/* マウスを表示している場合、     */
	  y /= 10;			/* 表示位置と座標がずれるので注意 */
	}
#endif
	pc88_mouse_moved_rel( x, y );
      

      }else{

	x = E.motion.x;
	y = E.motion.y;

	x -= SCREEN_DX;
	y -= SCREEN_DY;
	if     ( now_screen_size == SCREEN_SIZE_HALF )  { x *= 2; y *= 2; }
#ifdef	SUPPORT_DOUBLE
	else if( now_screen_size == SCREEN_SIZE_DOUBLE ){ x /= 2; y /= 2; }
#endif
	pc88_mouse_moved_abs( x, y );
      }
      break;


    case SDL_MOUSEBUTTONDOWN:	/*------------------------------------------*/
    case SDL_MOUSEBUTTONUP:

      /* マウス移動イベントも同時に処理する必要があるなら、
	 pc88_mouse_moved_abs/rel 関数をここで呼び出しておく */

      if     ( E.button.button==SDL_BUTTON_LEFT      ) key88 = KEY88_MOUSE_L;
      else if( E.button.button==SDL_BUTTON_MIDDLE    ) key88 = KEY88_MOUSE_M;
      else if( E.button.button==SDL_BUTTON_RIGHT     ) key88 = KEY88_MOUSE_R;
      else if( E.button.button==SDL_BUTTON_WHEELUP   ) key88 = KEY88_MOUSE_WUP;
      else if( E.button.button==SDL_BUTTON_WHEELDOWN ) key88 = KEY88_MOUSE_WDN;
      else break;

      pc88_mouse( key88, (E.type==SDL_MOUSEBUTTONDOWN) );
      break;


    case SDL_JOYAXISMOTION:	/*------------------------------------------*/
/*printf("%d %d %d\n",E.jaxis.which,E.jaxis.axis,E.jaxis.value);*/
      if( E.jbutton.which == 0 ){
	int now, chg;

	if( E.jaxis.axis == 0 ){	/* 左右方向 */

	  now = pad_axis & ~(AXIS_L|AXIS_R);

	  if     ( E.jaxis.value < -0x4000 ) now |= AXIS_L;
	  else if( E.jaxis.value >  0x4000 ) now |= AXIS_R;

	  chg = pad_axis ^ now;
	  if( chg & AXIS_L ) pc88_pad( KEY88_PAD_LEFT,  (now & AXIS_L) );
	  if( chg & AXIS_R ) pc88_pad( KEY88_PAD_RIGHT, (now & AXIS_R) );

	}else{				/* 上下方向 */

	  now = pad_axis & ~(AXIS_U|AXIS_D);

	  if     ( E.jaxis.value < -0x4000 ) now |= AXIS_U;
	  else if( E.jaxis.value >  0x4000 ) now |= AXIS_D;

	  chg = pad_axis ^ now;
	  if( chg & AXIS_U ) pc88_pad( KEY88_PAD_UP,    (now & AXIS_U) );
	  if( chg & AXIS_D ) pc88_pad( KEY88_PAD_DOWN,  (now & AXIS_D) );
	}
	pad_axis = now;
      }
      break;


    case SDL_JOYBUTTONDOWN:	/*------------------------------------------*/
    case SDL_JOYBUTTONUP:
/*printf("%d %d\n",E.jbutton.which,E.jbutton.button);*/
      if( E.jbutton.which == 0 ){
	if( E.jbutton.button < 8 ){
	  key88 = KEY88_PAD_A + E.jbutton.button;
	  pc88_pad( key88, ( E.type==SDL_JOYBUTTONDOWN ) );
	}
      }
      break;


    case SDL_QUIT:		/*------------------------------------------*/
      if( verbose_proc ) printf( "Window Closed.....\n" );
      pc88_quit();
      break;


    case SDL_ACTIVEEVENT:	/*------------------------------------------*/
      /* -focus オプションを機能させたいなら、 
	 pc88_focus_in / pc88_focus_out を適宜呼び出す必要がある。 */

      if( E.active.state & SDL_APPINPUTFOCUS ){
	if( E.active.gain ){
	  pc88_focus_in();
	}else{
	  pc88_focus_out();
	}
      }
      break;


    case SDL_VIDEOEXPOSE:	/*------------------------------------------*/
      put_image_all();			/* EXPOSE 時は 勝手に再描画しておく */
      break;
    }
  }
}



/***********************************************************************
 * 現在のマウス座標取得関数
 *
 *	現在のマウスの絶対座標を *x, *y にセット
 ************************************************************************/

void	init_mouse_position( int *x, int *y )
{
  int win_x, win_y;

  SDL_PumpEvents();
  SDL_GetMouseState( &win_x, &win_y );

  win_x -= SCREEN_DX;
  win_y -= SCREEN_DY;

  if     ( now_screen_size == SCREEN_SIZE_HALF )  { win_x *= 2;  win_y *= 2; }
#ifdef	SUPPORT_DOUBLE
  else if( now_screen_size == SCREEN_SIZE_DOUBLE ){ win_x /= 2;  win_y /= 2; }
#endif
  *x = win_x;
  *y = win_y;
}




/******************************************************************************
 * ソフトウェア NumLock 有効／無効
 *
 *	TODO  設定ファイルなどで、任意に配置できるといいなぁ
 *****************************************************************************/

INLINE	void	numlock_setup( int enable )
{
  int i;

  struct{
    int keysym;
    int original;
    int change;
  } binding[] = 
  {
    {	SDLK_7,		KEY88_7,		KEY88_KP_7,		},
    {	SDLK_8,		KEY88_8,		KEY88_KP_8,		},
    {	SDLK_9,		KEY88_9,		KEY88_KP_9,		},
    {	SDLK_0,		KEY88_0,		KEY88_KP_MULTIPLY,	},
    {	SDLK_u - 32,	KEY88_U,		KEY88_KP_4,		},
    {	SDLK_i - 32,	KEY88_I,		KEY88_KP_5,		},
    {	SDLK_o - 32,	KEY88_O,		KEY88_KP_6,		},
    {	SDLK_p - 32,	KEY88_P,		KEY88_KP_ADD,		},
    {	SDLK_j - 32,	KEY88_J,		KEY88_KP_1,		},
    {	SDLK_k - 32,	KEY88_K,		KEY88_KP_2,		},
    {	SDLK_l - 32,	KEY88_L,		KEY88_KP_3,		},
    {	SDLK_SEMICOLON,	KEY88_SEMICOLON,	KEY88_KP_EQUIAL,	},
    {	SDLK_m - 32,	KEY88_M,		KEY88_KP_0,		},
    {	SDLK_COMMA,	KEY88_COMMA,		KEY88_KP_COMMA,		},
    {	SDLK_PERIOD,	KEY88_PERIOD,		KEY88_KP_PERIOD,	},
    {	SDLK_SLASH,	KEY88_SLASH,		KEY88_RETURNR,		},

    {	SDLK_QUOTE,	KEY88_APOSTROPHE,	KEY88_KP_7,		},
    {	SDLK_LEFTPAREN,	KEY88_PARENLEFT,	KEY88_KP_8,		},
    {	SDLK_RIGHTPAREN,KEY88_PARENRIGHT,	KEY88_KP_9,		},
    {	SDLK_u,		KEY88_u,		KEY88_KP_4,		},
    {	SDLK_i,		KEY88_i,		KEY88_KP_5,		},
    {	SDLK_o,		KEY88_o,		KEY88_KP_6,		},
    {	SDLK_p,		KEY88_p,		KEY88_KP_ADD,		},
    {	SDLK_j,		KEY88_j,		KEY88_KP_1,		},
    {	SDLK_k,		KEY88_k,		KEY88_KP_2,		},
    {	SDLK_l,		KEY88_l,		KEY88_KP_3,		},
    {	SDLK_PLUS,	KEY88_PLUS,		KEY88_KP_EQUIAL,	},
    {	SDLK_m,		KEY88_m,		KEY88_KP_0,		},
    {	SDLK_LESS,	KEY88_LESS,		KEY88_KP_COMMA,		},
    {	SDLK_GREATER,	KEY88_GREATER,		KEY88_KP_PERIOD,	},
    {	SDLK_QUESTION,	KEY88_QUESTION,		KEY88_RETURNR,		},
  };

  for (i=0; i<COUNTOF(binding); i++ ){
    if( enable ){
      sdlk2keycode[ binding[i].keysym ] = binding[i].change;
    }else{
      sdlk2keycode[ binding[i].keysym ] = binding[i].original;
    }
  }
}

int	numlock_on ( void ){ numlock_setup( TRUE );  return TRUE; }
void	numlock_off( void ){ numlock_setup( FALSE ); }



/******************************************************************************
 * エミュレート／メニュー／ポーズ／モニターモード の 開始時の処理
 *
 *****************************************************************************/

void	event_init( void )
{
  /* 既存のイベントをすべて破棄 */
  /* なんてことは、しない ? */

  if( get_emu_mode() == EXEC ){

    /* キー押下を ASCII コードに変換可能とする */
    SDL_EnableUNICODE( 0 );

  }else{

    /* キー押下を UNICODE に変換可能とする (処理が重いらしいので一時的に) */
    SDL_EnableUNICODE( 1 );

  }

  /* マウス表示、グラブの設定 (ついでにキーリピートも) */
  set_mouse_state();
}



/******************************************************************************
 * ジョイスティック
 *
 * 一度だけの初期化時点（ウインドウとか？）にて、オープンする。
 *****************************************************************************/

int	joy_init( void )
{
  joy = NULL;

  if( ! SDL_WasInit( SDL_INIT_JOYSTICK ) ){
    if( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) ){
      return FALSE;
    }
  }

  if( SDL_NumJoysticks() >= 1 ){	/* ジョイスティックがあるかチェック */

    joy = SDL_JoystickOpen(0);		/* ジョイスティックをオープン */

    if( joy )
      SDL_JoystickEventState( SDL_ENABLE );	/* イベント処理を有効にする */
  }

  if( joy ) return TRUE;
  else      return FALSE;
}



int	joystick_available( void )
{
  if( joy ) return TRUE;
  else      return FALSE;
}
