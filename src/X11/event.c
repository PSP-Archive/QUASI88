/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/

/* ----------------------------------------------------------------------
 *		ヘッダファイル部
 * ---------------------------------------------------------------------- */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "keyboard.h"
#include "joystick.h"

#include "graph.h"	/* now_screen_size		*/

#include "drive.h"

#include "emu.h"
#include "device.h"
#include "status.h"
#include "screen.h"
#include "event.h"
#include "file-op.h"



int	use_xdnd = TRUE;		/* XDnD に対応するかどうか	*/
int	use_joydevice = TRUE;		/* ジョイスティックデバイスを開く? */

int	get_focus;			/* 現在、フォーカスありかどうか	*/

static	int mouse_jumped = FALSE;


/*----------------------------------------------------------------------
 * X11 の KeySym を QUASI88 の キーコードに変換するテーブル
 *----------------------------------------------------------------------*/
static int keysym2keycode_misc[ KEY88_END ] =
{
  0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_BS,		/*	XK_BackSpace		0xFF08	*/
  KEY88_TAB,		/*	XK_Tab			0xFF09	*/
  KEY88_RETURN,		/*	XK_Linefeed		0xFF0A	*/
  KEY88_HOME,		/*	XK_Clear		0xFF0B	*/
  0,
  KEY88_RETURNL,	/*	XK_Return		0xFF0D	*/
  0, 0,

  0, 0, 0,
  KEY88_STOP,		/*	XK_Pause		0xFF13	*/
  KEY88_KANA,		/*	XK_Scroll_Lock		0xFF14	*/
  KEY88_COPY,		/*	XK_Sys_Req		0xFF15	*/
  0, 0,
  0, 0, 0,
  KEY88_ESC, 		/*	XK_Escape		0xFF1B	*/
  0, 0, 0, 0,

  0,
  0,			/*	XK_Kanji		0xFF21	*/
  KEY88_KETTEI,		/*	XK_Muhenkan		0xFF22	*/
  KEY88_HENKAN,		/*	XK_Henkan_Mode		0xFF23  */
  0,			/*	XK_Romaji		0xFF24  */
  0,			/*	XK_Hiragana		0xFF25  */
  0,			/*	XK_Katakana		0xFF26  */
  KEY88_KANA,		/*	XK_Hiragana_Katakana	0xFF27	*/
  0,			/*	XK_Zenkaku		0xFF28  */
  0,			/*	XK_Hankaku		0xFF29  */
  KEY88_ZENKAKU,	/*	XK_Zenkaku_Hankaku	0xFF2A	*/
  0,			/*	XK_Touroku		0xFF2B  */
  0,			/*	XK_Massyo		0xFF2C  */
  KEY88_KANA,		/*	XK_Kana_Lock		0xFF2D	*/
  0,			/*	XK_Kana_Shift		0xFF2E  */
  0,			/*	XK_Eisu_Shift		0xFF2F  */

  0,			/*	XK_Eisu_toggle		0xFF30  */
  0, 0, 0, 0, 0, 0,
  0,			/*	XK_Kanji_Bangou		0xFF37  */
  0, 0, 0, 0, 0,
  0,			/*	XK_Zen_Koho		0xFF3D	*/
  0,			/*	XK_Mae_Koho		0xFF3E	*/
  0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  KEY88_HOME,		/*	XK_Home			0xFF50	*/
  KEY88_LEFT,		/*	XK_Left			0xFF51	*/
  KEY88_UP,		/*	XK_Up			0xFF52	*/
  KEY88_RIGHT,		/*	XK_Right		0xFF53	*/
  KEY88_DOWN,		/*	XK_Down			0xFF54	*/
  KEY88_ROLLDOWN,	/*	XK_Prior		0xFF55	*/
  KEY88_ROLLUP,		/*	XK_Next			0xFF56	*/
  KEY88_HELP,		/*	XK_End			0xFF57	*/
  0, 0, 0, 0, 0, 0, 0, 0,

  0,
  KEY88_COPY,		/*	XK_Print		0xFF61	*/
  KEY88_COPY,		/*	XK_Execute		0xFF62	*/
  KEY88_INS, 		/*	XK_Insert		0xFF63	*/
  0, 0, 0, 0, 0, 0,
  KEY88_HELP,		/*	XK_Help			0xFF6A	*/
  KEY88_STOP,		/*	XK_Break		0xFF6B	*/
  0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_HENKAN,		/*	XK_Mode_switch		0xFF7E	*/
  0,			/*	XK_Num_Lock		0xFF7F	*/

  KEY88_SPACE,		/*	XK_KP_Space		0xFF80	*/
  0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_TAB,		/*	XK_KP_Tab		0xFF89	*/
  0, 0, 0,
  KEY88_RETURNR,	/*	XK_KP_Enter		0xFF8D	*/
  0, 0,

  0, 0, 0, 0, 0,
  KEY88_KP_7,		/*	XK_KP_Home		0xFF95	*/
  KEY88_KP_4,		/*	XK_KP_Left		0xFF96	*/
  KEY88_KP_8,		/*	XK_KP_Up		0xFF97	*/
  KEY88_KP_6,		/*	XK_KP_Right		0xFF98	*/
  KEY88_KP_2,		/*	XK_KP_Down		0xFF99	*/
  KEY88_KP_9,		/*	XK_KP_Page_Up		0xFF9A	*/
  KEY88_KP_3,		/*	XK_KP_Page_Down		0xFF9B	*/
  KEY88_KP_1,		/*	XK_KP_End		0xFF9C	*/
  KEY88_KP_5,		/*	XK_KP_Begin		0xFF9D	*/
  KEY88_KP_0,		/*	XK_KP_Insert		0xFF9E	*/
  KEY88_KP_PERIOD,	/*	XK_KP_Delete		0xFF9F	*/

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_KP_MULTIPLY,	/*	XK_KP_Multiply		0xFFAA	*/
  KEY88_KP_ADD,		/*	XK_KP_Add		0xFFAB	*/
  KEY88_KP_COMMA,	/*	XK_KP_Separator		0xFFAC	*/
  KEY88_KP_SUB,		/*	XK_KP_Subtract		0xFFAD	*/
  KEY88_KP_PERIOD,	/*	XK_KP_Decimal		0xFFAE	*/
  KEY88_KP_DIVIDE,	/*	XK_KP_Divide		0xFFAF	*/

  KEY88_KP_0,		/*	XK_KP_0			0xFFB0	*/
  KEY88_KP_1,		/*	XK_KP_1			0xFFB1	*/
  KEY88_KP_2,		/*	XK_KP_2			0xFFB2	*/
  KEY88_KP_3,		/*	XK_KP_3			0xFFB3	*/
  KEY88_KP_4,		/*	XK_KP_4			0xFFB4	*/
  KEY88_KP_5,		/*	XK_KP_5			0xFFB5	*/
  KEY88_KP_6,		/*	XK_KP_6			0xFFB6	*/
  KEY88_KP_7,		/*	XK_KP_7			0xFFB7	*/
  KEY88_KP_8,		/*	XK_KP_8			0xFFB8	*/
  KEY88_KP_9,		/*	XK_KP_9			0xFFB9	*/
  0, 0, 0, 0,
  KEY88_F1,		/*	XK_F1			0xFFBE	*/
  KEY88_F2,		/*	XK_F2			0xFFBF	*/

  KEY88_F3,		/*	XK_F3			0xFFC0	*/
  KEY88_F4,		/*	XK_F4			0xFFC1	*/
  KEY88_F5,		/*	XK_F5			0xFFC2	*/
  KEY88_F6,		/*	XK_F6			0xFFC3	*/
  KEY88_F7,		/*	XK_F7			0xFFC4	*/
  KEY88_F8,		/*	XK_F8			0xFFC5	*/
  KEY88_F9,		/*	XK_F9			0xFFC6	*/
  KEY88_F10,		/*	XK_F10			0xFFC7	*/
  KEY88_F11,		/*	XK_F11			0xFFC8	*/
  KEY88_F12,		/*	XK_F12			0xFFC9	*/
  0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  0,
  KEY88_SHIFTL,		/*	XK_Shift_L		0xFFE1	*/
  KEY88_SHIFTR,		/*	XK_Shift_R		0xFFE2	*/
  KEY88_CTRL,		/*	XK_Control_L		0xFFE3	*/
  KEY88_CTRL,		/*	XK_Control_R		0xFFE4	*/
  KEY88_CAPS,		/*	XK_Caps_Lock		0xFFE5	*/
  0,
  KEY88_GRAPH,		/*	XK_Meta_L			*/
  KEY88_GRAPH,		/*	XK_Meta_R		0xFFE8	*/
  KEY88_GRAPH,		/*	XK_Alt_L		0xFFE9	*/
  KEY88_PC,		/*	XK_Alt_R		0xFFEA	*/
  0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_DEL,		/*	XK_Delete		0xFFFF	*/
};

static int keysym2keycode_jis[ KEY88_END ] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  KEY88_SPACE,		/*	XK_space               0x020	*/
  KEY88_EXCLAM,		/*	XK_exclam              0x021	*/
  KEY88_QUOTEDBL,	/*	XK_quotedbl            0x022	*/
  KEY88_NUMBERSIGN,	/*	XK_numbersign          0x023	*/
  KEY88_DOLLAR,		/*	XK_dollar              0x024	*/
  KEY88_PERCENT,	/*	XK_percent             0x025	*/
  KEY88_AMPERSAND,	/*	XK_ampersand           0x026	*/
  KEY88_APOSTROPHE,	/*	XK_apostrophe          0x027	*/
  KEY88_PARENLEFT,	/*	XK_parenleft           0x028	*/
  KEY88_PARENRIGHT,	/*	XK_parenright          0x029	*/
  KEY88_ASTERISK,	/*	XK_asterisk            0x02a	*/
  KEY88_PLUS,		/*	XK_plus                0x02b	*/
  KEY88_COMMA,		/*	XK_comma               0x02c	*/
  KEY88_MINUS,		/*	XK_minus               0x02d	*/
  KEY88_PERIOD,		/*	XK_period              0x02e	*/
  KEY88_SLASH,		/*	XK_slash               0x02f	*/

  KEY88_0,		/*	XK_0                   0x030	*/
  KEY88_1,		/*	XK_1                   0x031	*/
  KEY88_2,		/*	XK_2                   0x032	*/
  KEY88_3,		/*	XK_3                   0x033	*/
  KEY88_4,		/*	XK_4                   0x034	*/
  KEY88_5,		/*	XK_5                   0x035	*/
  KEY88_6,		/*	XK_6                   0x036	*/
  KEY88_7,		/*	XK_7                   0x037	*/
  KEY88_8,		/*	XK_8                   0x038	*/
  KEY88_9,		/*	XK_9                   0x039	*/
  KEY88_COLON,		/*	XK_colon               0x03a	*/
  KEY88_SEMICOLON,	/*	XK_semicolon           0x03b	*/
  KEY88_LESS,		/*	XK_less                0x03c	*/
  KEY88_EQUAL,		/*	XK_equal               0x03d	*/
  KEY88_GREATER,	/*	XK_greater             0x03e	*/
  KEY88_QUESTION,	/*	XK_question            0x03f	*/

  KEY88_AT,		/*	XK_at                  0x040	*/
  KEY88_A,		/*	XK_A                   0x041	*/
  KEY88_B,		/*	XK_B                   0x042	*/
  KEY88_C,		/*	XK_C                   0x043	*/
  KEY88_D,		/*	XK_D                   0x044	*/
  KEY88_E,		/*	XK_E                   0x045	*/
  KEY88_F,		/*	XK_F                   0x046	*/
  KEY88_G,		/*	XK_G                   0x047	*/
  KEY88_H,		/*	XK_H                   0x048	*/
  KEY88_I,		/*	XK_I                   0x049	*/
  KEY88_J,		/*	XK_J                   0x04a	*/
  KEY88_K,		/*	XK_K                   0x04b	*/
  KEY88_L,		/*	XK_L                   0x04c	*/
  KEY88_M,		/*	XK_M                   0x04d	*/
  KEY88_N,		/*	XK_N                   0x04e	*/
  KEY88_O,		/*	XK_O                   0x04f	*/

  KEY88_P,		/*	XK_P                   0x050	*/
  KEY88_Q,		/*	XK_Q                   0x051	*/
  KEY88_R,		/*	XK_R                   0x052	*/
  KEY88_S,		/*	XK_S                   0x053	*/
  KEY88_T,		/*	XK_T                   0x054	*/
  KEY88_U,		/*	XK_U                   0x055	*/
  KEY88_V,		/*	XK_V                   0x056	*/
  KEY88_W,		/*	XK_W                   0x057	*/
  KEY88_X,		/*	XK_X                   0x058	*/
  KEY88_Y,		/*	XK_Y                   0x059	*/
  KEY88_Z,		/*	XK_Z                   0x05a	*/
  KEY88_BRACELEFT,	/*	XK_bracketleft         0x05b	*/
  KEY88_BACKSLASH,	/*	XK_backslash           0x05c	*/
  KEY88_BRACERIGHT,	/*	XK_bracketright        0x05d	*/
  KEY88_CARET,		/*	XK_asciicircum         0x05e	*/
  KEY88_UNDERSCORE,	/*	XK_underscore          0x05f	*/

  KEY88_BACKQUOTE,	/*	XK_grave               0x060	*/
  KEY88_a,		/*	XK_a                   0x061	*/
  KEY88_b,		/*	XK_b                   0x062	*/
  KEY88_c,		/*	XK_c                   0x063	*/
  KEY88_d,		/*	XK_d                   0x064	*/
  KEY88_e,		/*	XK_e                   0x065	*/
  KEY88_f,		/*	XK_f                   0x066	*/
  KEY88_g,		/*	XK_g                   0x067	*/
  KEY88_h,		/*	XK_h                   0x068	*/
  KEY88_i,		/*	XK_i                   0x069	*/
  KEY88_j,		/*	XK_j                   0x06a	*/
  KEY88_k,		/*	XK_k                   0x06b	*/
  KEY88_l,		/*	XK_l                   0x06c	*/
  KEY88_m,		/*	XK_m                   0x06d	*/
  KEY88_n,		/*	XK_n                   0x06e	*/
  KEY88_o,		/*	XK_o                   0x06f	*/

  KEY88_p,		/*	XK_p                   0x070	*/
  KEY88_q,		/*	XK_q                   0x071	*/
  KEY88_r,		/*	XK_r                   0x072	*/
  KEY88_s,		/*	XK_s                   0x073	*/
  KEY88_t,		/*	XK_t                   0x074	*/
  KEY88_u,		/*	XK_u                   0x075	*/
  KEY88_v,		/*	XK_v                   0x076	*/
  KEY88_w,		/*	XK_w                   0x077	*/
  KEY88_x,		/*	XK_x                   0x078	*/
  KEY88_y,		/*	XK_y                   0x079	*/
  KEY88_z,		/*	XK_z                   0x07a	*/
  KEY88_BRACELEFT,	/*	XK_braceleft           0x07b	*/
  KEY88_BAR,		/*	XK_bar                 0x07c	*/
  KEY88_BRACERIGHT,	/*	XK_braceright          0x07d	*/
  KEY88_TILDE,		/*	XK_asciitilde          0x07e	*/
  0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


/******************************************************************************
 * イベントハンドリング
 *
 *	1/60毎に呼び出される。
 *****************************************************************************/
static	int	*keysym2keycode_latin1;

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
      101キーの対応
   */


  if( TRUE ){
    keysym2keycode_latin1 = keysym2keycode_jis;
  }else{
    ;
  }
}



/*
 * 約 1/60 毎に呼ばれる
 */
void	event_handle( void )
{
  XEvent E;
  KeySym keysym;
  char	 dummy[4];
  int    key88, x, y;

  while( XPending( display ) ){

    XNextEvent( display, &E );

    switch( E.type ){


    case ButtonPress:		/*------------------------------------------*/
    case ButtonRelease:

      /* マウス移動イベントも同時に処理する必要があるなら、
	 pc88_mouse_moved_abs/rel 関数をここで呼び出しておく */

      if     ( E.xbutton.button==Button1 ) key88 = KEY88_MOUSE_L;
      else if( E.xbutton.button==Button2 ) key88 = KEY88_MOUSE_M;
      else if( E.xbutton.button==Button3 ) key88 = KEY88_MOUSE_R;
      else if( E.xbutton.button==Button4 ) key88 = KEY88_MOUSE_WUP;
      else if( E.xbutton.button==Button5 ) key88 = KEY88_MOUSE_WDN;
      else break;

      pc88_mouse( key88, (E.type==ButtonPress) );
      break;


    case MotionNotify:		/*------------------------------------------*/
      switch( mouse_rel_move ){

      case 1:
	/* DGAの場合、マウス移動イベントは常に相対移動量がセットされる */

	x = E.xbutton.x;
	y = E.xbutton.y;

	if( x || y ){
	  pc88_mouse_moved_rel( x, y );
	}
	break;

      case -1:
	/* ウインドウグラブ時は、マウスがウインドウの端にたどり着くとそれ以上
	   動けなくなる。そこで、マウスを常にウインドウの中央にジャンプさせる
	   ことで、無限にマウスを動かすことができるかのようにする。
	   この時、マウスを非表示にしておかないと、無様な様子が見えるので注意。
	   なお、XWarpPointer でポインタを移動させると、それに応じて
	   MotionNotify が発生するので、無視すること。*/

	if( mouse_jumped ){ mouse_jumped = FALSE; break; }

	x = E.xbutton.x - WIDTH/2;
	y = E.xbutton.y - HEIGHT/2;
#if 0
	if( get_emu_mode()==EXEC ){
	  x /= 10;			/* ここで、マウスの速度が調整できる */
	  y /= 10;
	}
#endif
	if( x || y ){
	  pc88_mouse_moved_rel( x, y );
	}
      
	XWarpPointer( display, None, window, 0, 0, 0, 0, WIDTH/2, HEIGHT/2 );
	mouse_jumped = TRUE;
	break;

      default:
	/* 通常、マウス移動イベントは、マウスのウインドウ座標がセットされる */

	x = E.xbutton.x;
	y = E.xbutton.y;

	x -= SCREEN_DX;
	y -= SCREEN_DY;
	if     ( now_screen_size == SCREEN_SIZE_HALF )  { x *= 2; y *= 2; }
#ifdef	SUPPORT_DOUBLE
	else if( now_screen_size == SCREEN_SIZE_DOUBLE ){ x /= 2; y /= 2; }
#endif
	pc88_mouse_moved_abs( x, y );
      }
/*printf("%d %d\n",E.xbutton.x,E.xbutton.y);*/
      break;


    case KeyPress:		/*------------------------------------------*/
    case KeyRelease:

	/* 以下のどっちを使うのが正解？ 取得出来る keysym が違うのだが …… */
#if 0
      keysym = XLookupKeysym( (XKeyEvent *)&E, 0 );
	/* ・Shift + TAB は XK_TAB が返る				*/
	/* ・Shift + \ (シフトの左のキー) は XK_backslash が返る	*/
#else
      XLookupString( (XKeyEvent *)&E, dummy, 0, &keysym, NULL );
	/* ・Shift + TAB は XK_ISO_Left_Tab が返る			*/
	/* ・Shift + \ (シフトの左のキー) は XK_underscore が返る	*/
#endif

      if( (keysym & 0xff00) == 0xff00 ){		/* XK_MISCELLANY */

	key88 = keysym2keycode_misc[ keysym & 0x00ff ];

      }else if( (keysym & 0xff00) == 0x0000 ){		/* XK_LATIN1 */

	key88 = keysym2keycode_latin1[ keysym ];

      }else if( keysym == XK_ISO_Left_Tab ){		/* 0xFE20 */

	key88 = KEY88_TAB;

      }else{						/* else... */

	key88 = 0;

      }

      if( key88 ){
	pc88_key( key88, (E.type==KeyPress) );
      }
      break;


    case ClientMessage:		/*------------------------------------------*/
      /* ウインドウが破壊された場合は、ここで終了 */
      if(       E.xclient.message_type == atom_WM_close_type &&
	  (Atom)E.xclient.data.l[0]    == atom_WM_close_data ){

	if( verbose_proc ) printf( "Window Closed.....\n" );
	pc88_quit();
      }

      /* Drag & Drop のチェック */
      if( use_xdnd )
	xdnd_receive_drag( (XClientMessageEvent *)&E );
      break;


    case SelectionNotify:	/*------------------------------------------*/
      /* Drag & Drop のチェック */
      if( use_xdnd )
	xdnd_receive_drop( (XSelectionEvent *)&E );
      break; 


    case FocusIn:		/*------------------------------------------*/
      pc88_focus_in();
      get_focus = TRUE;
      set_mouse_state();		/* オートリピート再設定のため */
      break;


    case FocusOut:		/*------------------------------------------*/
      pc88_focus_out();
      get_focus = FALSE;
      set_mouse_state();		/* オートリピート再設定のため */
      break;


    case Expose:		/*------------------------------------------*/
      put_image_all();			/* EXPOSE 時は 勝手に再描画しておく */
      break;
    }
  }

  /* X11 には標準でジョイステックイベントが発生しない。
     なので、ここでジョイステック状態をポーリングして処理する */

  joystick_event();
}



/***********************************************************************
 * 現在のマウス座標取得関数
 *
 *	現在のマウスの絶対座標を *x, *y にセット
 ************************************************************************/

void	init_mouse_position( int *x, int *y )
{
  if( mouse_rel_move ){

    /* DGAおよびウインドウグラブ時は、マウスの絶対座標は存在しない。 */

  }else{

    Window root_return;				/* ルートウインドウID     */
    Window child_return;			/* 子ウインドウID(あれば) */
    int    root_x, root_y;			/* ルートウインドウの座標 */
    int    win_x,  win_y;			/* 指定ウインドウの座標   */
    unsigned int button;			/* マウスボタンの押下状況 */

    if( XQueryPointer( display, window, &root_return, &child_return,
		       &root_x, &root_y, 
		       &win_x,  &win_y, 
		       &button ) ){
      win_x -= SCREEN_DX;
      win_y -= SCREEN_DY;
      if     ( now_screen_size == SCREEN_SIZE_HALF )  {win_x *= 2; win_y *= 2;}
#ifdef	SUPPORT_DOUBLE
      else if( now_screen_size == SCREEN_SIZE_DOUBLE ){win_x /= 2; win_y /= 2;}
#endif
      *x = win_x;
      *y = win_y;

      return;
    }

  }

  /* 絶対座標が無い場合は、とりあえず画面中央の座標を返すことにしよう */

  *x = 640/2;
  *y = 400/2;
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
    int xk_keysym;
    int original;
    int change;
  } binding[] = 
  {
    {	XK_7,		KEY88_7,		KEY88_KP_7,		},
    {	XK_8,		KEY88_8,		KEY88_KP_8,		},
    {	XK_9,		KEY88_9,		KEY88_KP_9,		},
    {	XK_0,		KEY88_0,		KEY88_KP_MULTIPLY,	},
    {	XK_U,		KEY88_U,		KEY88_KP_4,		},
    {	XK_I,		KEY88_I,		KEY88_KP_5,		},
    {	XK_O,		KEY88_O,		KEY88_KP_6,		},
    {	XK_P,		KEY88_P,		KEY88_KP_ADD,		},
    {	XK_J,		KEY88_J,		KEY88_KP_1,		},
    {	XK_K,		KEY88_K,		KEY88_KP_2,		},
    {	XK_L,		KEY88_L,		KEY88_KP_3,		},
    {	XK_semicolon,	KEY88_SEMICOLON,	KEY88_KP_EQUIAL,	},
    {	XK_M,		KEY88_M,		KEY88_KP_0,		},
    {	XK_comma,	KEY88_COMMA,		KEY88_KP_COMMA,		},
    {	XK_period,	KEY88_PERIOD,		KEY88_KP_PERIOD,	},
    {	XK_slash,	KEY88_SLASH,		KEY88_RETURNR,		},

    {	XK_apostrophe,	KEY88_APOSTROPHE,	KEY88_KP_7,		},
    {	XK_parenleft,	KEY88_PARENLEFT,	KEY88_KP_8,		},
    {	XK_parenright,	KEY88_PARENRIGHT,	KEY88_KP_9,		},
    {	XK_u,		KEY88_u,		KEY88_KP_4,		},
    {	XK_i,		KEY88_i,		KEY88_KP_5,		},
    {	XK_o,		KEY88_o,		KEY88_KP_6,		},
    {	XK_p,		KEY88_p,		KEY88_KP_ADD,		},
    {	XK_j,		KEY88_j,		KEY88_KP_1,		},
    {	XK_k,		KEY88_k,		KEY88_KP_2,		},
    {	XK_l,		KEY88_l,		KEY88_KP_3,		},
    {	XK_plus,	KEY88_PLUS,		KEY88_KP_EQUIAL,	},
    {	XK_m,		KEY88_m,		KEY88_KP_0,		},
    {	XK_less,	KEY88_LESS,		KEY88_KP_COMMA,		},
    {	XK_greater,	KEY88_GREATER,		KEY88_KP_PERIOD,	},
    {	XK_question,	KEY88_QUESTION,		KEY88_RETURNR,		},
  };

  for (i=0; i<COUNTOF(binding); i++ ){
    if( enable ){
      keysym2keycode_latin1[ binding[i].xk_keysym ] = binding[i].change;
    }else{
      keysym2keycode_latin1[ binding[i].xk_keysym ] = binding[i].original;
    }
  }
}

int	numlock_on ( void ){ numlock_setup( TRUE );  return TRUE; }
void	numlock_off( void ){ numlock_setup( FALSE ); }



/******************************************************************************
 * エミュレート/メニュー/ポーズ/モニターモード の 開始時の処理
 *
 *****************************************************************************/

void	event_init( void )
{
#if 1
  /* 既存のイベントをすべて破棄 */
  XEvent E;
  while( XCheckWindowEvent( display, window,
			   ExposureMask|KeyPressMask|KeyReleaseMask|
			   ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
			   &E) );
#endif


  /* マウス表示、グラブの設定 (ついでにキーリピートも) */
  set_mouse_state();


  /* ウインドウグラブ中は、マウスを画面中央へ移動 */
  if( mouse_rel_move == -1 ){
    XWarpPointer( display, None, window, 0, 0, 0, 0, WIDTH/2, HEIGHT/2 );
    mouse_jumped = TRUE;
  }

  XFlush( display );
}



#if 0
/***********************************************************************
 * 強制終了のイベントを生成
 ************************************************************************/
void	event_quit_signal( void )
{
  XClientMessageEvent E;

  E.type    = ClientMessage;
  E.display = display;
  E.window  = window;
  E.message_type = atom_sigint;
  E.format       = 32;

  if( XSendEvent( display, window, False, ExposureMask, (XEvent*)&E ) ){
    XFlush( display );
  }
}
#endif


/****************************************************************************
 *
 *	XDnD を適当に実装してみた。実装にはあまり自信なし。
 *
 *****************************************************************************/

#include <X11/Xatom.h>

static int drop_filename( const char *buf, int size );

static const int my_xdnd_ver = 5;

static Atom XA_XdndAware;
static Atom XA_XdndProxy;
static Atom XA_XdndEnter;
static Atom XA_XdndPosition;
static Atom XA_XdndStatus;
static Atom XA_XdndLeave;
static Atom XA_XdndDrop;
static Atom XA_XdndFinished;
static Atom XA_XdndTypeList;
static Atom XA_XdndSelection;
static Atom XV_DND_SELECTION;
static Atom XA_XdndActionCopy;
static Atom XA_XdndActionMove;
static Atom XA_XdndActionLink;
static Atom XA_XdndActionPrivate;
static Atom XA_XdndActionAsk;
static Atom MIME_text_plain;
static Atom MIME_text_uri_list;

/*
 *	起動時に1回だけ呼び出す。(Atomを取得or設定する)
 */
void	xdnd_initialize( void )
{
  if( use_xdnd == FALSE ) return;

  XA_XdndAware		= XInternAtom(display, "XdndAware",		False);
  XA_XdndProxy		= XInternAtom(display, "XdndProxy",		False);
  XA_XdndEnter		= XInternAtom(display, "XdndEnter",		False);
  XA_XdndPosition	= XInternAtom(display, "XdndPosition",		False);
  XA_XdndStatus		= XInternAtom(display, "XdndStatus",		False);
  XA_XdndLeave		= XInternAtom(display, "XdndLeave",		False);
  XA_XdndDrop		= XInternAtom(display, "XdndDrop",		False);
  XA_XdndFinished	= XInternAtom(display, "XdndFinished",		False);

  XA_XdndActionCopy	= XInternAtom(display, "XdndActionCopy",	False);
  XA_XdndActionMove	= XInternAtom(display, "XdndActionMove",	False);
  XA_XdndActionLink	= XInternAtom(display, "XdndActionLink",	False);
  XA_XdndActionPrivate	= XInternAtom(display, "XdndActionPrivate",	False);
  XA_XdndActionAsk	= XInternAtom(display, "XdndActionAsk",		False);

  XA_XdndTypeList	= XInternAtom(display, "XdndTypeList",		False);
  XA_XdndSelection	= XInternAtom(display, "XdndSelection",		False);
  XV_DND_SELECTION	= XInternAtom(display, "XV_DND_SELECTION",	False);

  MIME_text_plain	= XInternAtom(display, "text/plain",		False);
  MIME_text_uri_list	= XInternAtom(display, "text/uri-list",		False);

#if 0
  printf("XA_XdndAware          %d\n",XA_XdndAware          );
  printf("XA_XdndProxy          %d\n",XA_XdndProxy          );
  printf("XA_XdndEnter          %d\n",XA_XdndEnter          );
  printf("XA_XdndPosition       %d\n",XA_XdndPosition       );
  printf("XA_XdndStatus         %d\n",XA_XdndStatus         );
  printf("XA_XdndLeave          %d\n",XA_XdndLeave          );
  printf("XA_XdndDrop           %d\n",XA_XdndDrop           );
  printf("XA_XdndFinished       %d\n",XA_XdndFinished       );
  printf("XA_XdndActionCopy     %d\n",XA_XdndActionCopy     );
  printf("XA_XdndActionMove     %d\n",XA_XdndActionMove     );
  printf("XA_XdndActionLink     %d\n",XA_XdndActionLink     );
  printf("XA_XdndActionPrivate  %d\n",XA_XdndActionPrivate  );
  printf("XA_XdndActionAsk      %d\n",XA_XdndActionAsk      );
  printf("XA_XdndTypeList       %d\n",XA_XdndTypeList       );
  printf("XA_XdndSelection      %d\n",XA_XdndSelection      );
  printf("XV_DND_SELECTION      %d\n",XV_DND_SELECTION      );
  printf("MIME_text_plain       %d\n",MIME_text_plain       );
  printf("MIME_text_uri_list    %d\n",MIME_text_uri_list    );
#endif
}


/*
 *	ウインドウ生成時に呼び出す (XDndの使用を宣言する)
 */
void	xdnd_start( void )
{
  if( use_xdnd == FALSE ) return;

  XChangeProperty( display, window, XA_XdndAware, XA_ATOM, 32,
		   PropModeReplace,(unsigned char*) &my_xdnd_ver, 1);
}



/*
 *	イベント処理
 */

static int	xdnd_ver;
static Window	xdnd_from;
static Window	xdnd_to;
static Atom	xdnd_type;
static int	xdnd_accept;
static enum{ YET, ENTER, POS, DROP } xdnd_step = YET;

void	xdnd_receive_drag( XClientMessageEvent *E )
{
  if( E->message_type == XA_XdndEnter ){
    /*
      ■■■■ 最初：マウスがウインドウに入ってきた時のイベント ■■■■

      E->data.l[0]	ソースウィンドウの XID
      E->data.l[1]	上位 8bit プロトコルバージョン
     			bit 0 が真なら3個以上のデータタイプあり
      E->data.l[2]-[4]	データタイプ 1個目、2個目、3個目
    */

    xdnd_from = (Window)E->data.l[0];
    xdnd_to   =         E->window;
    xdnd_ver  = E->data.l[1] >> 24;
    xdnd_step = ENTER;
    xdnd_type = None;

    if( (E->data.l[1] & 1) == 0 ){	/* データタイプは3個以内 */

      if      ( (Atom)E->data.l[2] == MIME_text_uri_list ||
		(Atom)E->data.l[3] == MIME_text_uri_list ||
		(Atom)E->data.l[4] == MIME_text_uri_list ){

	xdnd_type = MIME_text_uri_list;
      }else if( (Atom)E->data.l[2] == MIME_text_plain ||
		(Atom)E->data.l[3] == MIME_text_plain ||
		(Atom)E->data.l[4] == MIME_text_plain ){

	xdnd_type = MIME_text_plain;
      }

    }else{				/* データタイプもっとある */
      Atom type;
      int format;
      unsigned long nitems, bytes, i;
      Atom* prop;

      if( XGetWindowProperty( display, E->data.l[0], XA_XdndTypeList,
			      0, 8192, False, XA_ATOM, &type, &format,
			      &nitems, &bytes, (unsigned char **) &prop )
	  == Success ){

	for( i = 0; i < nitems; i ++ ){
	  if( prop[i] == MIME_text_uri_list ||
	      prop[i] == MIME_text_plain    ){
	    xdnd_type = prop[i];
	    break;
	  }
	}
	XFree( prop );
      }
    }

  }else if( E->message_type == XA_XdndPosition ){

    /*
      ■■■■ 途中：マウスがウインドウ内を動く時のイベント ■■■■

      E->data.l[0]	ソースウィンドウの XID
      E->data.l[1]	予約
      E->data.l[2]	マウスの座標 (x << 16) | y
      E->data.l[3]	タイムスタンプ
      E->data.l[4]	リクエストアクション
    */

    if( xdnd_from != (Window)E->data.l[0] ||
	xdnd_to   !=         E->window    ){
      /* ん？ 違うぞ？ */
      ;
    }else if( xdnd_step == ENTER || xdnd_step == POS ){

      xdnd_step   = POS;
      xdnd_accept = 0;

      if( get_emu_mode()==EXEC ){
	if( xdnd_type != None ){
	  if( (Atom)E->data.l[4] == XA_XdndActionCopy ){
	    xdnd_accept = 1;
	  }
	}
      }

      {
	XClientMessageEvent S;

	S.type		= ClientMessage;
	S.display	= display;
	S.window	= xdnd_from;
	S.message_type	= XA_XdndStatus;
	S.format	= 32;
	S.data.l[0]	= xdnd_to;
	S.data.l[1]	= xdnd_accept;
	S.data.l[2]	= 0; /* (x << 16) | y; */
	S.data.l[3]	= 0; /* (w << 16) | h; */
	S.data.l[4]	= (xdnd_accept) ? E->data.l[4] : None;

	XSendEvent( display, S.window, False, 0, (XEvent*)&S );
      }
    }

  }else if( E->message_type == XA_XdndLeave ){
    /*
      ■■■■ 最後：マウスがウインドウを出た時のイベント ■■■■

      E->data.l[0]	ソースウィンドウの XID
      E->data.l[1]	予約
    */

    if( xdnd_from != (Window)E->data.l[0] ||
	xdnd_to   !=         E->window    ){
      /* ん？ 違うぞ？ */
      ;
    }else{
      xdnd_step = YET;
    }

  }else if( E->message_type == XA_XdndDrop ){
    /*
      ■■■■ 実行：ウインドウ内でドロップされた時のイベント ■■■■

      E->data.l[0]	ソースウィンドウの XID
      E->data.l[1]	予約
    */

    if( xdnd_from != (Window)E->data.l[0] ||
	xdnd_to   !=         E->window    ){
      /* ん？ 違うぞ？ */
      ;
    }else{
      if( xdnd_step == POS ){
	XConvertSelection( display, XA_XdndSelection, xdnd_type,
			   XV_DND_SELECTION, xdnd_to, E->data.l[2] );

	xdnd_step = DROP;
      }else{
	if( xdnd_ver >= 5 ){
	  XClientMessageEvent S;

	  S.type	= ClientMessage;
	  S.display	= display;
	  S.window	= xdnd_from;
	  S.message_type= XA_XdndFinished;
	  S.format	= 32;
	  S.data.l[0]	= xdnd_to;
	  S.data.l[1]	= 0;
	  S.data.l[2]	= None;
	  S.data.l[3]	= 0;
	  S.data.l[4]	= 0;

	  XSendEvent( display, S.window, False, 0, (XEvent*)&S );
	}
	xdnd_step = YET;
      }
    }

  }else{
    /* 困った。わからない……… */
    ;
  }
}



void	xdnd_receive_drop( XSelectionEvent *E )
{
  unsigned long	bytes_after;
  XTextProperty	prop;
  int		ret;

    /*
      ■■■■ 結末：ドロップ後に通知されるイベント ■■■■
    */

  if( E->property == XV_DND_SELECTION ){
    if( xdnd_step == DROP ){

      ret = XGetWindowProperty( display, E->requestor,
				E->property, 0L, 0x1fffffffL,
				True, AnyPropertyType,
				&prop.encoding, &prop.format, &prop.nitems,
				&bytes_after, &prop.value );

      if( ret           == Success &&
	  prop.encoding != None    &&
	  prop.format   == 8       &&
	  prop.nitems   != 0       &&
	  prop.value    != NULL    ){

	drop_filename( (char *)prop.value, (int)prop.nitems );

	XFree( prop.value );
      }

      {
	XClientMessageEvent S;
	xdnd_accept = TRUE;

	S.type		= ClientMessage;
	S.display	= display;
	S.window	= xdnd_from;
	S.message_type	= XA_XdndFinished;
	S.format	= 32;
	S.data.l[0]	= xdnd_to;
	S.data.l[1]	= xdnd_accept;
	S.data.l[2]	= xdnd_accept ? XA_XdndActionCopy : None;
	S.data.l[3]	= 0;
	S.data.l[4]	= 0;

	XSendEvent( display, S.window, False, 0, (XEvent*)&S );
      }

      xdnd_step = YET;
    }
  }
}



static int drop_filename( const char *buf, int size )
{
  char filename[ OSD_MAX_FILENAME ];
  char *d;
  const char *s;
  int i, len;

#if 0
  const char *c = buf;
  int l = 0;
    
  fprintf(stderr,"XdndSelection: dump property %d bytes.",size);
  while (l < size) {
    if (!(l & 15))
      fprintf(stderr, "\n  %04x|", l);
    if (*c > ' ' && *c <= '~')
      fprintf(stderr, "  %c", *c);
    else 
      fprintf(stderr, " %02x", *c);
    l ++;
    c ++;
  }
  fputc('\n', stderr);
#endif

  s = NULL;
  if      ( strncmp( buf, "file://localhost/", 17 )==0 ){
    s = &buf[ 16 ];
    size -= 16;
  }else if( strncmp( buf, "file:///",           8 )==0 ){
    s = &buf[ 7 ];
    size -= 7;
  }else if( strncmp( buf, "file://",            7 )==0 ){
    s = &buf[ 6 ];
    size -= 6;
  }else if( strncmp( buf, "file:/",             6 )==0 ){
    s = &buf[ 5 ];
    size -= 5;
  }

  if( s ){
    d = filename;
    len = sizeof(filename)-1;

    while( len && size ){
      if( *s == '\r' ||
	  *s == '\n' ||
	  *s == '\0' ) break;

      if( *s == '%' ){
	if( size <= 2 ) return 0;
	s ++;
	if     ( '0' <= (*s) && (*s) <= '9' ) i  = (*s) - '0';
	else if( 'A' <= (*s) && (*s) <= 'F' ) i  = (*s) - 'A' + 10;
	else return 0;
	i *= 16;
	s ++;
	if     ( '0' <= (*s) && (*s) <= '9' ) i += (*s) - '0';
	else if( 'A' <= (*s) && (*s) <= 'F' ) i += (*s) - 'A' + 10;
	else return 0;
	size -= 2;
      }else{
	i = *s;
      }
      *d ++ = i; len  --;
      s ++;      size --;
    }
    *d = '\0';

/*printf("%s\n", filename );*/

    if( get_emu_mode() == EXEC ){
      if( quasi88_disk_insert_and_reset( filename, FALSE ) ){
	status_message( 1, 180, "Disk Image Set and Reset" );
      }
    }

  }
  return 1;
}
