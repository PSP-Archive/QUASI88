/************************************************************************/
/*									*/
/* QUASI88 メニュー用 Tool Kit						*/
/*				Core lib				*/
/*									*/
/*	GTK+ の API を真似て作りました。ややこしすぎて、ドキュメントは	*/
/*	書けません………。						*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"

#include "q8tk.h"
#include "q8tk-glib.h"

#include "menu-screen.h"

#include "file-op.h"
#include "wait.h"

#include "keyboard.h"	/* mouse_x, mouse_y */
#include "event.h"
#include "menu.h"


/* デバッグ用 */
#define	Q8TK_ASSERT_DEBUG


/************************************************************************/
/* デバッグ								*/
/************************************************************************/

/*----------------------------------------------------------------------*/

#ifdef	Q8TK_ASSERT_DEBUG

#define	Q8tkAssert(e,s)						\
	((e) ? (void)0 : _Q8tkAssert(__FILE__, __LINE__, #e, s ))
static	void	_Q8tkAssert( char *file, int line, char *exp, const char *s )
{
  fprintf( stderr, "Fatal Error | %s <%s:%d>\n", exp, file, line );
  fprintf( stderr, "message = %s\n",(s)?s:"---");
  quasi88_exit();
}
#else

#define	Q8tkAssert(e,s)		((void)0)

#endif

/*----------------------------------------------------------------------*/

#ifdef	Q8TK_ASSERT_DEBUG

static const char *debug_type(int type){
  switch( type ){
  case Q8TK_TYPE_WINDOW:	return "window  :";
  case Q8TK_TYPE_BUTTON:	return "button  :";
  case Q8TK_TYPE_TOGGLE_BUTTON:	return "t-button:";
  case Q8TK_TYPE_CHECK_BUTTON:	return "c-button:";
  case Q8TK_TYPE_RADIO_BUTTON:	return "r-button:";
  case Q8TK_TYPE_FRAME:		return "frame   :";
  case Q8TK_TYPE_LABEL:		return "label   :";
  case Q8TK_TYPE_LOGO:		return "logo    :";
  case Q8TK_TYPE_NOTEBOOK:	return "notebook:";
  case Q8TK_TYPE_NOTEPAGE:	return "page    :";
  case Q8TK_TYPE_VBOX:		return "vbox    :";
  case Q8TK_TYPE_HBOX:		return "hbox    :";
  case Q8TK_TYPE_VSEPARATOR:	return "vsep    :";
  case Q8TK_TYPE_HSEPARATOR:	return "hsep    :";
  case Q8TK_TYPE_COMBO:		return "combo   :";
  case Q8TK_TYPE_LISTBOX:	return "listbox :";
  case Q8TK_TYPE_LIST_ITEM:	return "listitem:";
  case Q8TK_TYPE_ADJUSTMENT:	return "adjust  :";
  case Q8TK_TYPE_HSCALE:	return "hscale  :";
  case Q8TK_TYPE_VSCALE:	return "vscale  :";
  case Q8TK_TYPE_SCROLLED_WINDOW:return "scrolled:";
  case Q8TK_TYPE_ENTRY:		return "entry   :";
  case Q8TK_TYPE_DIALOG:	return "dialog  :";
  case Q8TK_TYPE_FILE_SELECTION:return "f-select:";
  }
  return "UNDEF TYPE:";
}

#else

#define	debug_type(t)		((void)0)

#endif


/************************************************************************/
/* エラー処理								*/
/************************************************************************/
#define	CHECK_MALLOC_OK(e,s)	((e) ? (void)0 : _CHECK_MALLOC_OK(s))

static	void	_CHECK_MALLOC_OK( const char *s )
{
  fprintf( stderr, "Fatal Error : %s exhaused!\n", s );
  quasi88_exit();
}


/************************************************************************/
/* ワーク								*/
/************************************************************************/

#define	MAX_WIDGET		(8192)
static	Q8tkWidget	*widget_table[ MAX_WIDGET ];


#define	MAX_LIST		(2048)
static	Q8List		*list_table[ MAX_LIST ];


#define	MAX_WINDOW_LEVEL	(8)
static	Q8tkWidget	*window_level[ MAX_WINDOW_LEVEL ];
static	int		window_level_now;
static	Q8tkWidget	*event_widget[ MAX_WINDOW_LEVEL ];

#define	set_event_widget(w)	event_widget[ window_level_now ] = (w)
#define	get_event_widget()	event_widget[ window_level_now ]



static	struct{
  int	x, y;
  int	x_old, y_old;
} mouse;


static	int	kanji_code = Q8TK_KANJI_ANK;
static	int	disp_cursor = FALSE;

static	int	now_shift_on;
static	int	now_mouse_on;

/*----------------------------------------------------------------------*/

static	void	widget_map( Q8tkWidget *widget );
static	void	widget_construct( void );
static	void	widget_signal_do( Q8tkWidget *widget, const char *name );
static	void	widget_redraw_now( void );



/************************************************************************/
/* 動的ワークの確保／開放						*/
/************************************************************************/
/*--------------------------------------------------------------
 *	Widget
 *--------------------------------------------------------------*/
static	Q8tkWidget	*malloc_widget( void )
{
  int		i;
  Q8tkWidget	*w;

  for( i=0; i<MAX_WIDGET; i++ ){
    if( widget_table[i] == NULL ){
      w = (Q8tkWidget *)calloc( 1, sizeof(Q8tkWidget) );
      if( w ){
	widget_table[i] = w;
	return w;
      }else{
	Q8tkAssert(FALSE,"memory exhoused");
	CHECK_MALLOC_OK( FALSE, "memory" );
	break;
      }
    }
  }
  Q8tkAssert(FALSE,"work 'widget' exhoused");
  CHECK_MALLOC_OK( FALSE, "work 'widget'" );
  return NULL;
}
static	void	free_widget( Q8tkWidget *w )
{
  int	i;
  for( i=0; i<MAX_WIDGET; i++ ){
    if( widget_table[i] == w ){
      free( w );
      widget_table[i] = NULL;
      return;
    }
  }
  Q8tkAssert(FALSE,"pointer is not malloced widget");
}
/*--------------------------------------------------------------
 *	List
 *--------------------------------------------------------------*/
static	Q8List	*malloc_list( void )
{
  int		i;
  Q8List	*l;

  for( i=0; i<MAX_LIST; i++ ){
    if( list_table[i] == NULL ){
      l = (Q8List *)calloc( 1, sizeof(Q8List ) );
      if( l ){
	list_table[i] = l;
	return l;
      }else{
	Q8tkAssert(FALSE,"memory exhoused");
	CHECK_MALLOC_OK( FALSE, "memory" );
	break;
      }
    }
  }
  Q8tkAssert(FALSE,"work 'list' exhoused");
  CHECK_MALLOC_OK( FALSE, "work 'list'" );
  return NULL;
}
static	void	free_list( Q8List *l )
{
  int	i;
  for( i=0; i<MAX_LIST; i++ ){
    if( list_table[i] == l ){
      free( l );
      list_table[i] = NULL;
      return;
    }
  }
  Q8tkAssert(FALSE,"pointer is not malloced list");
}



#if 0
/************************************************************************/
/* Q8TK 専用 文字列処理							*/
/*	q8_strncpy( s, ct, n )						*/
/*		文字列 ct を 文字列 s に コピーする。			*/
/*		s の文字列終端は、必ず '\0' となり、s の長さは n-1 文字	*/
/*		以下に収まる。						*/
/*	q8_strncat( s, ct, n )						*/
/*		文字列 ct を 文字列 s に 付加する。			*/
/*		s の文字列終端は、必ず '\0' となり、s の長さは n-1 文字	*/
/*		以下に収まる。						*/
/************************************************************************/
static	void	q8_strncpy( char *s, const char *ct, size_t n )
{
  strncpy( s, ct, n-1 );
  s[ n-1 ] = '\0';
}
static	void	q8_strncat( char *s, const char *ct, size_t n )
{
  if( n > strlen(s) + 1 )
    strncat( s, ct, n - strlen(s) -1 );
}
#endif


/************************************************************************/
/* リスト処理								*/
/************************************************************************/
/*--------------------------------------------------------------
 * リストの末尾にあたらしい要素を追加 (またはリストを新規作成)
 *	戻り値は、リストの先頭
 *--------------------------------------------------------------*/
Q8List	*q8_list_append( Q8List *list, void *data )
{
  Q8List	*new_list;

  new_list  = malloc_list();		/* ワーク確保 */

  if( list ){				/* 既存のリストにつなげる場合 */
    list = q8_list_last( list );		/* 末尾とリンクする */
    list->next     = new_list;
    new_list->prev = list;
    new_list->next = NULL;
  }else{				/* 新規のリストの場合 */
    new_list->prev = NULL;
    new_list->next = NULL;
  }
  new_list->data   = data;

  return q8_list_first( new_list );	/* 処理後のリスト先頭を返す   */
}
/*--------------------------------------------------------------
 * リストの途中にあたらしい要素を追加 (position==0で先頭)
 *	戻り値は、リストの先頭
 *--------------------------------------------------------------*/
#if 0
Q8List	*q8_list_insert( Q8List *list, void *data, int position )
{
  /* 未作成 */
  return NULL;
}
#endif
/*--------------------------------------------------------------
 * リストの要素をひとつだけ削除
 *	戻り値は、リストの先頭
 *--------------------------------------------------------------*/
Q8List	*q8_list_remove( Q8List *list, void *data )
{
  list = q8_list_first( list );

  while( list ){			/* リスト先頭から順にたどって */
    if( list->data == data ){		/* data の一致するものを探す  */
      list->prev->next = list->next;		/* みつかればリンクを */
      list->next->prev = list->prev;		/* つなぎ変えて       */
      free_list( list );			/* 自身は削除         */
      break;
    }
    list = list->next;
  }

  return q8_list_first( list );		/* 処理後のリスト先頭を返す   */
}
/*--------------------------------------------------------------
 * リストの消去
 *--------------------------------------------------------------*/
void	q8_list_free( Q8List *list )
{
  Q8List *l;

  list = q8_list_first( list );		/* リストの先頭から */

  while( list ){			/* たどりながら解放 */
    l = list->next;
    free_list( list );
    list = l;
  }
}
/*--------------------------------------------------------------
 * リスト先頭へ
 *--------------------------------------------------------------*/
Q8List	*q8_list_first( Q8List *list )
{
  if( list ){
    while( list->prev ){		/* リストを前にたどる */
      list = list->prev;
    }					/* たどれなくなったら */
  }					/* それが先頭         */
  return list;
}
/*--------------------------------------------------------------
 * リスト末尾へ
 *--------------------------------------------------------------*/
Q8List	*q8_list_last( Q8List *list )
{
  if( list ){
    while( list->next ){		/* リストを後にたどる */
      list = list->next;
    }					/* たどれなくなったら */
  }					/* それが末尾         */
  return list;
}


/************************************************************************/
/* 漢字コード								*/
/************************************************************************/
int	q8tk_set_kanjicode( int code )
{
  int ret = kanji_code;

  if( code != Q8TK_KANJI_ANK &&
      code != Q8TK_KANJI_EUC &&
      code != Q8TK_KANJI_SJIS ) code = Q8TK_KANJI_ANK;

  kanji_code = code;

  return ret;
}
/************************************************************************/
/* カーソル表示の有無							*/
/************************************************************************/
void	q8tk_set_cursor( int enable )
{
  disp_cursor = (enable) ? TRUE : FALSE;
}





/************************************************************************/
/* 初期化と終了								*/
/************************************************************************/

static	int		q8tk_main_loop_flag;
static	int		q8tk_construct_flag;

#define	set_main_loop_flag(f)	q8tk_main_loop_flag = (f)
#define	get_main_loop_flag()	q8tk_main_loop_flag
#define	set_construct_flag(f)	q8tk_construct_flag = (f)
#define	get_construct_flag()	q8tk_construct_flag


static	Q8tkWidget	*q8tk_drag_widget;
#define	set_drag_widget(w)	q8tk_drag_widget = (w)
#define	get_drag_widget()	q8tk_drag_widget

static	int		active_widget_flag;
#define	check_active( widget )						\
		do{							\
		  if( (widget)==get_event_widget() ) active_widget_flag=TRUE; \
		}while(0)
#define	check_active_finish()	active_widget_flag=FALSE
#define	is_active_widget()	(active_widget_flag)


static	Q8tkWidget	*q8tk_tab_top_widget;
#define	set_tab_top_widget(w)	q8tk_tab_top_widget = (w)
#define	get_tab_top_widget()	q8tk_tab_top_widget


#define	NR_SCROLL_ADJ	(8)
static	struct{
  int		drawn;
  Q8tkWidget	*widget;
} q8tk_scroll_adj[ NR_SCROLL_ADJ ];

static	void	init_scroll_adj_widget( void )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    q8tk_scroll_adj[i].drawn    = FALSE;
    q8tk_scroll_adj[i].widget   = NULL;
  }
}

static	void	set_scroll_adj_widget( Q8tkWidget *w )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    if( q8tk_scroll_adj[i].widget==w ) return;
  }
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    if( q8tk_scroll_adj[i].widget==NULL ){
      q8tk_scroll_adj[i].drawn  = FALSE;
      q8tk_scroll_adj[i].widget = w;
      return;
    }
  }
}
static	void	check_scroll_adj_widget( Q8tkWidget *w )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    if( q8tk_scroll_adj[i].widget==w ){
      q8tk_scroll_adj[i].drawn = TRUE;
      return;
    }
  }
}




/*--------------------------------------------------------------
 *	初期化
 *--------------------------------------------------------------*/
void	q8tk_init( void )
{
  int	i;
  for( i=0; i<MAX_WIDGET; i++ ){
    widget_table[i] = NULL;
  }

  for( i=0; i<MAX_LIST; i++ ){
    list_table[i] = NULL;
  }

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){
    window_level[i] = NULL;
    event_widget[i] = NULL;
  }
  window_level_now = -1;

  set_main_loop_flag( FALSE );
  set_construct_flag( FALSE );

  set_drag_widget( NULL );

  init_scroll_adj_widget();


  q8gr_init();


  mouse.x_old = mouse.x = mouse_x;
  mouse.y_old = mouse.y = mouse_y;

  now_shift_on = FALSE;
  now_mouse_on = FALSE;

  wait_vsync_reset();
}
/*--------------------------------------------------------------
 *	終了
 *--------------------------------------------------------------*/
void	q8tk_term( void )
{
  int	i;
  for( i=0; i<MAX_WIDGET; i++ ){
    if( widget_table[i] ){
      if( widget_table[i]->name ) free( widget_table[i]->name );
      free( widget_table[i] );
    }
  }
  for( i=0; i<MAX_LIST; i++ ){
    if( list_table[i] ){
      free( list_table[i] );
    }
  }
}



/************************************************************************/
/* モーダルの設定							*/
/************************************************************************/
/*--------------------------------------------------------------
 *	モーダル設定
 *--------------------------------------------------------------*/
void	q8tk_grab_add( Q8tkWidget *widget )
{
  int	i;
  Q8tkAssert(widget->type==Q8TK_TYPE_WINDOW,"grab add not window");

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){
    if( window_level[i] == NULL ){
      window_level[i] = widget;
      window_level_now= i;
      set_construct_flag( TRUE );
      return;
    }
  }
  Q8tkAssert(FALSE,"overflow window level");
}
/*--------------------------------------------------------------
 *	モーダル解除
 *--------------------------------------------------------------*/
void	q8tk_grab_remove( Q8tkWidget *widget )
{
  int	i;

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){
    if( window_level[i] == widget ){
      break;
    }
  }
  Q8tkAssert(i<MAX_WINDOW_LEVEL,"grab remove not widget");
  for(    ; i<MAX_WINDOW_LEVEL-1; i++ ){
    window_level[i] = window_level[i+1];
    event_widget[i] = event_widget[i+1];
  }
  window_level[i] = NULL;
  event_widget[i] = NULL;
  window_level_now --;
  set_construct_flag( TRUE );
}



/********************************************************/
/* ウィジットの作成					*/
/********************************************************/

/*---------------------------------------------------------------------------
 * ウインドウ (WINDOW)
 *---------------------------------------------------------------------------
 *  ・全てのウィジットの最も先祖になるコンテナウィジット。
 *  ・子を一つ持てる。
 *  ・子を持つには、q8tk_container_add() を使用する。
 *  ・このウィジットを表示するには、q8tk_grab_add()にて明示的に表示を指示する。
 *  ・WINDOW は最大、MAX_WINDOW_LEVEL個作成できるが、シグナルを受け付けるのは、
 *    最後に q8tk_grab_add() を発行したウインドウの子孫のみである。
 *  ・Q8TK_WINDOW_TOPLEVEL のウィンドウが必ず必要。
 *  ・シグナル
 *	なし
 *  ・内部シグナル (ユーザ利用不可)
 *	"inactivate"	ポップアップウインドウにてウインドウ外をクリック時発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_window_new( int window_type )
 *	ウインドウの生成
 *	引数により、ウインドウの種類が決まる
 *		Q8TK_WINDOW_TOPLEVEL … 一番基礎のウインドウで、唯一つのみ
 *					生成可能。ウインドウ枠を持たない
 *		Q8TK_WINDOW_DIALOG   … 出っぱったウインドウ枠をもつ
 *		Q8TK_WINDOW_POPUP    … 単純なウインドウ枠をもつ
 *  -------------------------------------------------------------------------
 *	【WINDOW】←→ [xxxx]		さまざまなウィジットを子に持てる
 *
 *---------------------------------------------------------------------------*/
/* ポップアップウインドウの範囲外にてマウスクリックしたら、シグナル発生 */
static	void	window_event_button_on( Q8tkWidget *widget )
{
  widget_signal_do( widget, "inactivate" );
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_window_new( int window_type )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_WINDOW;
  w->attr   = Q8TK_ATTR_CONTAINER;

  switch( window_type ){
  case Q8TK_WINDOW_TOPLEVEL:
    Q8tkAssert(window_level[0]==NULL,"Redefine TOP window");
    q8tk_grab_add( w );
    w->stat.window.no_frame = TRUE;
    break;

  case Q8TK_WINDOW_DIALOG:
    w->stat.window.no_frame = FALSE;
    w->stat.window.shadow_type = Q8TK_SHADOW_OUT;
    break;

  case Q8TK_WINDOW_POPUP:
    w->stat.window.no_frame = FALSE;
    w->stat.window.shadow_type = Q8TK_SHADOW_ETCHED_OUT;
    w->event_button_on = window_event_button_on;
    break;
  }

  return	w;
}




/*---------------------------------------------------------------------------
 * ボタン (BUTTON)
 *---------------------------------------------------------------------------
 *  ・マウスなどでボタンを押している間は引っ込んで、マウスを離すと戻る。
 *  ・子を一つ持てる。 (但し、ラベル以外の子を持った場合の動作は未保証)
 *  ・子を持つには、q8tk_container_add() を使用する。
 *  ・シグナル
 *	"clicked"	ボタンが押された時に発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_button_new( void )
 *	ボタンの生成。
 *
 *  Q8tkWidget	*q8tk_button_new_with_label( const char *label )
 *	文字列 label が描かれたボタンの生成。
 *	内部的には、ボタンとラベルを作り、親子にする。
 *  -------------------------------------------------------------------------
 *	【BUTTON】←→ [LABEL]		ラベルを子に持つ
 *
 *---------------------------------------------------------------------------*/
/* ボタンをマウスクリックしたら、シグナル発生 */
static	void	button_event_button_on( Q8tkWidget *widget )
{
  widget->stat.button.active = Q8TK_BUTTON_ON;
  widget_redraw_now();				/* 凹んだ状態を一旦描画する */
  widget->stat.button.active = Q8TK_BUTTON_OFF;

  widget_signal_do( widget, "clicked" );

  set_construct_flag( TRUE );
}

/* リターン・スペースを押したら、マウスクリックと同じ処理をする */
static	void	button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    button_event_button_on( widget );
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_button_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on  = button_event_button_on;
  w->event_key_on     = button_event_key_on;

  return	w;
}

Q8tkWidget	*q8tk_button_new_with_label( const char *label )
{
  Q8tkWidget	*b = q8tk_button_new();
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}




/*---------------------------------------------------------------------------
 * トグルボタン (TOGGLE BUTTON)
 *---------------------------------------------------------------------------
 *  ・マウスなどでボタンを押すと引っ込む。もう一度ボタンを押すと戻る。
 *  ・子を一つ持てる。 (但し、ラベル以外の子を持った場合の動作は未保証)
 *  ・子を持つには、q8tk_container_add() を使用する。
 *  ・シグナル
 *	"clicked"	ボタンを押して引っ込んだ時に発生
 *	"toggled"	ボタンの状態が変化した時に発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_toggle_button_new( void )
 *	トグルボタンの生成。
 *
 *  Q8tkWidget	*q8tk_toggle_button_new_with_label( const char *label )
 *	文字列 label が描かれたグルボタンの生成。
 *	内部的には、トグルボタンとラベルを作り、親子にする。
 *
 *  void	q8tk_toggle_button_set_state( Q8tkWidget *widget, int status )
 *	ボタンの状態を変更する。
 *	status が真ならボタンを押した状態にする (シグナルが発生する)。
 *	status が偽ならなにもしない。
 *  -------------------------------------------------------------------------
 *	【TOGGLE BUTTON】←→ [LABEL]		ラベルを子に持つ
 *
 *---------------------------------------------------------------------------*/
/* ボタンをマウスクリックしたら、シグナル発生 */
static	void	toggle_button_event_button_on( Q8tkWidget *widget )
{
  if( widget->stat.button.active == Q8TK_BUTTON_ON ){
    widget->stat.button.active = Q8TK_BUTTON_OFF;
    widget_signal_do( widget, "toggled" );
  }else{
    widget->stat.button.active = Q8TK_BUTTON_ON;
    widget_signal_do( widget, "clicked" );
    widget_signal_do( widget, "toggled" );
  }
  set_construct_flag( TRUE );
}

/* リターン・スペースを押したら、マウスクリックと同じ処理をする */
static	void	toggle_button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    toggle_button_event_button_on( widget );
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_toggle_button_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_TOGGLE_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on = toggle_button_event_button_on;
  w->event_key_on    = toggle_button_event_key_on;

  return	w;
}

Q8tkWidget	*q8tk_toggle_button_new_with_label( const char *label )
{
  Q8tkWidget	*b = q8tk_toggle_button_new();
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}

void	q8tk_toggle_button_set_state( Q8tkWidget *widget, int status )
{
  if( status ){
    if( widget->event_button_on ){
      (*widget->event_button_on)( widget );
    }
  }else{
    widget->stat.button.active = Q8TK_BUTTON_OFF;
    set_construct_flag( TRUE );
  }
}




/*---------------------------------------------------------------------------
 * チェックボタン (CHECK BUTTON)
 *---------------------------------------------------------------------------
 *  ・ボタンを押すと、チェックボックスが塗りつぶされる。
 *    もう一度ボタンを押すと戻る。
 *  ・子を一つ持てる。 (但し、ラベル以外の子を持った場合の動作は未保証)
 *  ・子を持つには、q8tk_container_add() を使用する。
 *  ・シグナル
 *	"clicked"	ボタンが押された時に発生
 *	"toggled"	ボタンの状態が変化した時に発生
 *		現在OFF状態の場合は、 "toggled" が発生
 *		現在 ON状態の場合は、 "clicked" → "toggled" の順に発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_check_button_new( void )
 *	チェックボタンの生成。
 *
 *  Q8tkWidget	*q8tk_check_button_new_with_label( const char *label )
 *	文字列 label が描かれたチェックボタンの生成。
 *	内部的には、チェックボタンとラベルを作り、親子にする。
 *  -------------------------------------------------------------------------
 *	見た目は違うが、機能的にはトグルボタンと同じである。
 *	トグルボタンと同様、q8tk_toggle_button_set_state() により、
 *	ボタンを押した状態にすることができる (シグナルが発生する)。
 *  -------------------------------------------------------------------------
 *	【CHECKE BUTTON】←→ [LABEL]		ラベルを子に持つ
 *
 *---------------------------------------------------------------------------*/
/* ボタンをマウスクリックしたら、シグナル発生 */
static	void	check_button_event_button_on( Q8tkWidget *widget )
{
  if( widget->stat.button.active == Q8TK_BUTTON_ON ){
    widget->stat.button.active = Q8TK_BUTTON_OFF;
    widget_signal_do( widget, "toggled" );
  }else{
    widget->stat.button.active = Q8TK_BUTTON_ON;
    widget_signal_do( widget, "clicked" );
    widget_signal_do( widget, "toggled" );
  }
  set_construct_flag( TRUE );
}

/* リターン・スペースを押したら、マウスクリックと同じ処理をする */
static	void	check_button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    check_button_event_button_on( widget );
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_check_button_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_CHECK_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on  = check_button_event_button_on;
  w->event_key_on     = check_button_event_key_on;

  return	w;
}

Q8tkWidget	*q8tk_check_button_new_with_label( const char *label )
{
  Q8tkWidget	*b = q8tk_check_button_new();
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}




/*---------------------------------------------------------------------------
 * ラジオボタン (RADIO BUTTON)
 *---------------------------------------------------------------------------
 *  ・いくつかのラジオボタン同士でグルーピングできる。
 *  ・ボタンを押すとチェックされるが、同じグルーピングされた
 *    他のラジオボタンは、チェックが外れる。
 *  ・子を一つ持てる。 (但し、ラベル以外の子を持った場合の動作は未保証)
 *  ・子を持つには、q8tk_container_add() を使用する。
 *  ・シグナル
 *	"clicked"	ボタンが押された時に発生
 *	"toggled"	ボタンの状態が変化した時に発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_radio_button_new( Q8tkWidget *group )
 *	ラジオボタンの生成。
 *	同じグループとするラジオボタンを引数で与える。(最初のボタンなら NULL)
 *	最初のボタンがチェックされる。次以降のボタンはチェックが外れる。
 *	以下は、3個のラジオボタンを生成する例
 *		button[0] = q8tk_radio_button_new( NULL );
 *		button[1] = q8tk_radio_button_new( button[0] );
 *		button[2] = q8tk_radio_button_new( button[1] );
 *
 *  Q8tkWidget	*q8tk_radio_button_new_with_label( Q8List *list,
 *						   const char *label )
 *	文字列 label が描かれたラジオボタンの生成。
 *	内部的には、ラジオボタンとラベルを作り、親子にする。
 *  -------------------------------------------------------------------------
 *	グルーピングされているが、機能的にはトグルボタンと同じである。
 *	トグルボタンと同様、q8tk_toggle_button_set_state() により、
 *	ボタンを押した状態にすることができる。なお、この時グルーピングされた
 *	すべてのラジオボタンに "toggled" シグナルが発生する。
 *  -------------------------------------------------------------------------
 *	【RADIO BUTTON】←→ [LABEL]			ラベルを子に持つ
 *		↑					LIST 先頭を内部で保持
 *		└──────────	LIST
 *					↑｜
 *	【RADIO BUTTON】←→ [LABEL]	｜｜
 *		↑			｜↓
 *		└──────────	LIST
 *
 *	各ラジオボタンは独立したウィジットであるが、リスト情報によりグループを
 *	なしている。各ラジオボタンは、このリストの先頭を内部で保持している。
 *	ボタンを押すとグループを生成している全ウィジットにシグナルが発生する。
 *
 *---------------------------------------------------------------------------*/
/* ボタンをマウスクリックしたら、シグナル発生 */
static	void	radio_button_event_button_on( Q8tkWidget *widget )
{
  Q8List	*list;

					/* 自分自身は clicked シグナルを発生 */
  widget_signal_do( widget, "clicked" );
  if( widget->stat.button.active == Q8TK_BUTTON_ON ) return;

					/* リストをたどって自分以外のボタン  */
  list = widget->stat.button.list;	/* をオフにし、 toggled シグナル発生 */
  while( list ){
    if( list->data != widget ){
      ((Q8tkWidget *)(list->data))->stat.button.active = Q8TK_BUTTON_OFF;
      widget_signal_do( (Q8tkWidget *)(list->data), "toggled" );
    }
    list = list->next;
  }
						/* 自身もボタン状態変化なら */
						/* toggled シグナル発生     */
  widget->stat.button.active = Q8TK_BUTTON_ON;
  widget_signal_do( widget, "toggled" );

  set_construct_flag( TRUE );
}

/* リターン・スペースを押したら、マウスクリックと同じ処理をする */
static	void	radio_button_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    radio_button_event_button_on( widget );
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_radio_button_new( Q8tkWidget *group )
{
  Q8tkWidget	*w;
  Q8List *list;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_RADIO_BUTTON;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->event_button_on  = radio_button_event_button_on;
  w->event_key_on     = radio_button_event_key_on;

  if( group == NULL ){					/* 最初の1個目 */
    list = NULL;
    w->stat.button.active = TRUE;
  }else{						/* 2個目以降   */
    list = group->stat.button.list;
    w->stat.button.active = FALSE;
  }
  w->stat.button.list = q8_list_append( list, w );

  return	w;
}

Q8tkWidget	*q8tk_radio_button_new_with_label( Q8tkWidget *group,
						   const char *label )
{
  Q8tkWidget	*b = q8tk_radio_button_new( group );
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( b, l );

  b->with_label = TRUE;

  return	b;
}




/*---------------------------------------------------------------------------
 * コンボボックス (COMBO BOX)
 *---------------------------------------------------------------------------
 *  ・エントリ領域を持つ。
 *  ・子は持てない。
 *  ・q8tk_combo_append_popdown_strings() により、その文字列をラベルに持った
 *    リストアイテムを内部で生成する。これは一覧選択のポップアップウインドウ
 *    にて表示される。
 *  ・シグナル
 *	"changed"	エントリ領域の文字列に変更があった時に発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_combo_new( void )
 *	コンボボックスの生成。生成時は、入力は不可になっている。
 *
 *  void	q8tk_combo_append_popdown_strings( Q8tkWidget *combo, 
 *						   const char *entry_str,
 *						   const char *disp_str   )
 *	一覧選択時のポップアップウインドウを構成するための、リストアイテムを
 *	生成する。このリストアイテムは、 disp_str の文字列をラベルに持つ。
 *		一覧選択時のポップアップウインドウにてこのリストアイテムが選択
 *		された場合、 コンボボックスのエントリ領域に entry_str がセット
 *		される。(シグナルが発生する)
 *	disp_str == NULL の場合は、 disp_str は entry_str と同じになる。
 *
 *  const char	*q8tk_combo_get_text( Q8tkWidget *combo )
 *	エントリ領域に入力されている文字列を返す
 *
 *  void	q8tk_combo_set_text( Q8tkWidget *combo, const char *text )
 *	文字列をエントリ領域に設定する (シグナルが発生する)。
 *
 *  void	q8tk_combo_set_editable( Q8tkWidget *combo, int editable )
 *	エントリ領域の入力可・不可を設定する。 editable が 真なら入力可。
 *
 *  void	q8tk_misc_set_size( Q8tkWidget *widget, int width, int height )
 *	コンボボックスの表示サイズ width を指定する。 height は無効。
 *  -------------------------------------------------------------------------
 *	【COMBO BOX】
 *		｜
 *		└──【ENTRY】
 *
 *	q8tk_combo_append_popdown_strings にて LIST ITEM と LABEL を生成する。
 *
 *	【COMBO BOX】
 *	    ｜  ｜
 *	    ｜  └──【ENTRY】
 *	    ｜
 *	    └────  LIST  →【LIST ITEM】←→【LABEL】
 *			↑↓
 *			LIST  →【LIST ITEM】←→【LABEL】
 *			↑↓
 *			LIST  →【LIST ITEM】←→【LABEL】
 *			↑↓
 *
 *	コンボボックスがマウスでクリックされると、以下のようなウインドウ
 *	(Q8TK_WINDOW_POPUP) を自動的に生成し、そのウインドウが q8tk_grab_add()
 *	される。つまり、このウインドウへの操作以外は、出来なくなる。
 *
 *	【WINDOW】←→【LIST BOX】←→【LIST ITEM】←→【LABEL】
 *	    ｜				  ↑↓
 *	    ｜			  ←  【LIST ITEM】←→【LABEL】
 *	    ｜				  ↑↓
 *	    ｜			  ←  【LIST ITEM】←→【LABEL】
 *	    ｜				  ↑↓
 *	    ｜
 *	    └─【ACCEL GROUP】←→【ACCEL KEY】 → 【BUTTON(ダミー)】
 *
 *	この LIST ITEM と LABEL は q8tk_combo_append_popdown_strings() にて
 *	自動生成されたリストアイテム (とラベル) である。
 *
 *	リストアイテムのいずれか一つがが選択されると、ここで自動生成した
 *	ウィジット(リストアイテムは除く)は全て削除され、もとの表示に戻る。
 *	この時、選択したリストアイテムの持つ文字列が、エントリ領域に
 *	セットされ、同時にシグナルが発生する。
 *
 *	ESCキーが押された場合も、元の表示の戻る。(この場合は当然シグナルなし)
 *	これを実現するためにアクセラレータキーを利用しているが、アクセラレータ
 *	キーは、ウィジットにシグナルを送ることしか出来ないので、ダミーのボタン
 *	(非表示) を生成して、このボタンにシグナルを送るようにしている。
 *	このボタンはシグナルと受けると、全てのウィジットを削除する。
 *
 *	リストが長くなった場合、リストボックスの表示が画面に収まらないことが
 *	あるので、その場合は WINDOW と LIST BOX の間にスクロールドウインドウ
 *	を生成する。(画面に収まるかどうかの判断は結構いい加減)
 *
 *---------------------------------------------------------------------------*/

/* ポップアップウインドウで、LIST ITEM 選択 or ESCキー押下 or ウインドウ外を
   マウスクリックした時のコールバック関数。ポップアップウインドウを削除する */
static	void	combo_fake_callback( Q8tkWidget *dummy, Q8tkWidget *parent )
{
  Q8List     *l;

  l = parent->stat.combo.list;			/* l->data は LIST ITEM */
  while( l ){
    q8tk_signal_handlers_destroy( (Q8tkWidget *)(l->data) );
    l = l->next;
  }

  q8tk_grab_remove( parent->stat.combo.popup_window );

  if( parent->stat.combo.popup_scrolled_window )
    q8tk_widget_destroy( parent->stat.combo.popup_scrolled_window );

  /* LIST BOX は削除するが、 LIST ITEM は削除せずに残す */
  q8tk_widget_destroy( parent->stat.combo.popup_list );
  q8tk_widget_destroy( parent->stat.combo.popup_window );
  q8tk_widget_destroy( parent->stat.combo.popup_fake );
  q8tk_widget_destroy( parent->stat.combo.popup_accel_group );
}

/* ポップアップウインドウの LIST ITEM をマウスクリックした時のコールバック */
static	void	combo_event_list_callback( Q8tkWidget *list_item,
					   Q8tkWidget *parent )
{
  if( strcmp( parent->stat.combo.entry->name, list_item->child->name )==0 ){
    /* 現在の入力と、選択したリストの文字列が同じなので、シグナルは無し */
  }else{
    q8tk_entry_set_text( parent->stat.combo.entry,
			 list_item->child->name );
    widget_signal_do( parent, "changed" );
  }

  combo_fake_callback( NULL, parent );
}

/* コンボの一覧表示ボタンをマウスクリックしたら、ポップアップウインドウ生成 */
static	void	combo_event_button_on( Q8tkWidget *widget )
{
  int	selected = FALSE;
  Q8List     *l;

  widget->stat.combo.popup_window = q8tk_window_new( Q8TK_WINDOW_POPUP );
  widget->stat.combo.popup_scrolled_window = NULL;
  widget->stat.combo.popup_list   = q8tk_listbox_new();

  l = widget->stat.combo.list;			/* l->data は LIST ITEM */
  while( l ){
    q8tk_container_add( widget->stat.combo.popup_list,
			(Q8tkWidget *)(l->data) );

    /* エントリの文字列の一致するリストアイテムを、選択状態とする。          */
    /* q8tk_listbox_select_child()はシグナル発生するのでシグナル登録前に処理 */
    if( selected == FALSE ){			/* l->data->child は LABEL */
      if( strcmp( widget->stat.combo.entry->name,
		  ((Q8tkWidget*)(l->data))->child->name )==0 ){
	q8tk_listbox_select_child( widget->stat.combo.popup_list,
				   (Q8tkWidget*)(l->data) );
	selected = TRUE;
      }
    }

    q8tk_signal_connect( (Q8tkWidget *)(l->data), "select",
			 combo_event_list_callback, widget );
    l = l->next;
  }


  q8tk_widget_show( widget->stat.combo.popup_list );


		/* 画面からはみ出そうな時は、SCROLLED WINDOW を生成 */
  if( widget->y + widget->stat.combo.nr_items + 2 > 24 ){

    int height = 24 - 2 - widget->y;
    if( height < 3 ) height = 3;

    widget->stat.combo.popup_scrolled_window = 
					q8tk_scrolled_window_new( NULL, NULL );
    q8tk_container_add( widget->stat.combo.popup_scrolled_window, 
			widget->stat.combo.popup_list );

    q8tk_scrolled_window_set_policy( widget->stat.combo.popup_scrolled_window,
				   Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS );

    if( widget->stat.combo.width ){
      q8tk_misc_set_size( widget->stat.combo.popup_scrolled_window, 
			  widget->stat.combo.width +3, height );
    }else{
      q8tk_misc_set_size( widget->stat.combo.popup_scrolled_window, 
			  widget->stat.combo.length +3, height );
    }

    q8tk_widget_show( widget->stat.combo.popup_scrolled_window );
    q8tk_container_add( widget->stat.combo.popup_window, 
			widget->stat.combo.popup_scrolled_window );

  }else{	/* 通常はシンプルな WINDOW を生成 */

    q8tk_container_add( widget->stat.combo.popup_window, 
			widget->stat.combo.popup_list );

  }

  q8tk_widget_show( widget->stat.combo.popup_window );
  q8tk_grab_add( widget->stat.combo.popup_window );
  q8tk_widget_grab_default( widget->stat.combo.popup_list );

  if( widget->stat.combo.width ){
    q8tk_misc_set_size( widget->stat.combo.popup_list,
			widget->stat.combo.width, 0 );
  }

  widget->stat.combo.popup_window->stat.window.set_position = TRUE;
  widget->stat.combo.popup_window->stat.window.x = 
		widget->x -1 -((widget->stat.combo.popup_scrolled_window)?1:0);
  widget->stat.combo.popup_window->stat.window.y = 
		widget->y +1;

  q8tk_signal_connect( widget->stat.combo.popup_window,
		       "inactivate", combo_fake_callback,widget );


	/* ESC キーを押した時リストを消去するための、ダミーを生成 */

  widget->stat.combo.popup_fake = q8tk_button_new();
  q8tk_signal_connect( widget->stat.combo.popup_fake, "clicked",
		       combo_fake_callback, widget );

  widget->stat.combo.popup_accel_group = q8tk_accel_group_new();
  q8tk_accel_group_attach( widget->stat.combo.popup_accel_group,
			   widget->stat.combo.popup_window );
  q8tk_accel_group_add( widget->stat.combo.popup_accel_group, Q8TK_KEY_ESC,
		        widget->stat.combo.popup_fake, "clicked" );
}

/* リターン・スペースを押したら、一覧表示ボタンマウスクリックと同じ処理をする*/
static	void	combo_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    combo_event_button_on( widget );
  }
}

/* エントリ部に入力があったとき、シグナル発生 */
static	void	combo_event_entry_changed( Q8tkWidget *dummy,
					   Q8tkWidget *parent )
{
  widget_signal_do( parent, "changed" );
}


Q8tkWidget	*q8tk_combo_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_COMBO;

  w->stat.combo.entry  = q8tk_entry_new();
  w->stat.combo.list   = NULL;
  w->stat.combo.width  = 0;
  w->stat.combo.length = 0;

  q8tk_combo_set_editable( w, FALSE );
  q8tk_widget_show( w->stat.combo.entry );
  q8tk_signal_connect( w->stat.combo.entry, "changed",
		       combo_event_entry_changed, w );

  w->event_button_on  = combo_event_button_on;
  w->event_key_on     = combo_event_key_on;

  return	w;
}
void		q8tk_combo_append_popdown_strings( Q8tkWidget *combo, 
						   const char *entry_str,
						   const char *disp_str   )
{
  int l0, l1, l2;
  Q8tkWidget *list_item;

  if( combo->stat.combo.list == NULL ){		/* 念のため初期化 */
    combo->stat.combo.length   = 0;
    combo->stat.combo.nr_items = 0;
  }

		/* 文字列長をチェック */

  if( entry_str == NULL ) entry_str = "";
  if( disp_str  == NULL ) disp_str  = entry_str;

  l0 = combo->stat.combo.length;
  l1 = (disp_str) ? strlen(disp_str) : 0;
  l2 = strlen( entry_str )
		+ ( (combo->stat.combo.entry->stat.entry.editable) ? +1 : 0 );

  combo->stat.combo.length = Q8TKMAX( Q8TKMAX(l0, l1), l2 );


		/* ラベル付き LIST ITEM を生成し、そのポインタをリストに保持 */

  list_item = q8tk_list_item_new_with_label( disp_str );
  q8tk_widget_show( list_item );
  q8tk_list_item_set_string( list_item, entry_str );

  combo->stat.combo.list = q8_list_append( combo->stat.combo.list, list_item );
  combo->stat.combo.nr_items ++;


		/* コンボの入力部に、この文字列をセット */

  q8tk_entry_set_text( combo->stat.combo.entry, entry_str );

  if( combo->stat.combo.width ){
    q8tk_misc_set_size( combo->stat.combo.entry, combo->stat.combo.width, 0 );
  }else{
    q8tk_misc_set_size( combo->stat.combo.entry, combo->stat.combo.length, 0 );
  }
}

#if 0
void		q8tk_combo_clear_popdown_strings( Q8tkWidget *combo, 
						  int start, int end )
{
  /* 未作成 */
}
#endif

const	char	*q8tk_combo_get_text( Q8tkWidget *combo )
{
  return combo->stat.combo.entry->name;
}

void		q8tk_combo_set_text( Q8tkWidget *combo, const char *text )
{
  q8tk_entry_set_text( combo->stat.combo.entry, text );
}

void	   	q8tk_combo_set_editable( Q8tkWidget *combo, int editable )
{
  q8tk_entry_set_editable( combo->stat.combo.entry, editable );
}




/*---------------------------------------------------------------------------
 * リストボックス (LIST BOX)
 *---------------------------------------------------------------------------
 *  ・垂直ボックスのようなウィジット。
 *  ・複数の子をもてる。  但し、リストアイテム以外の子を持ってはいけない。
 *  ・子を持つには、q8tk_container_add() を使用する。
 *    子は、q8tk_container_add() を呼び出した順番に下へ下へと並んでいく。
 *  ・リストアイテムのいずれか一つを選択状態にすることができる。
 *    (選択なしにすることもできるが、複数を選択状態にすることはできない)
 *  ・シグナル
 *	"selection_change"	リストアイテムをマウスクリックした時、
 *				現在選択状態となっているリストアイテムが
 *				変化したら発生する。
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_listbox_new( void )
 *	リストボックスの生成。
 *
 *  void    q8tk_listbox_clear_items( Q8tkWidget *wlist, int start, int end )
 *	子のリストアイテムを削除する。 
 *	リストアイテムは、q8tk_container_add()を呼び出した順に 0 からの番号が
 *	振られるので、この番号で、削除するリストアイテムを指定する。
 *	end が start よりも小さい場合は、全てのリストアイテムが削除される。
 *	削除したリストアイテムは消滅するので後から再利用することはできない。
 *	後で利用したい場合はひとつずつ q8tk_container_remove() にて削除すべし。
 *
 *  void    q8tk_listbox_select_item( Q8tkWidget *wlist, int item )
 *	item の番号のリストアイテムを選択状態にする (シグナルが発生する)。
 *
 *  void    q8tk_listbox_select_child( Q8tkWidget *wlist, Q8tkWidget *child )
 *	child のリストアイテムを選択状態にする (シグナルが発生する)。
 *
 *  void    q8tk_misc_set_size( Q8tkWidget *widget, int width, int height )
 *	リストボックスの表示サイズ width を指定する。 height は無効。
 *  -------------------------------------------------------------------------
 *	【LIST BOX】←→ [LIST ITEM] ←→ [LABEL]	リストアイテム (と、
 *	   ：		    ↑↓			ラベル) を子に持つ
 *	   ：	    ←   [LIST ITEM] ←→ [LABEL]
 *	   ：		    ↑↓
 *	   ：…->   ←   [LIST ITEM] ←→ [LABEL]
 *	キー入力	    ↑↓
 *	にて        ←   [LIST ITEM] ←→ [LABEL]
 *	いずれかを	    ↑↓
 *	選択状態に
 *	する
 *	    ……… → 選択された LIST ITEM は マウスクリックされたことになる
 *
 *
 *	リストボックスがスクロールドウインドウの子である場合、スクロール
 *	ウインドウにフォーカスされると直ちにリストボックスにフォーカスが
 *	移る。これにより、リストボックスはキー入力を受けつけるようになる。
 *
 *	キー入力にて、あるリストアイテムを選択した場合、
 *	そのリストアイテムがマウスでクリックされたのと同じ動作をする。
 *	
 *	q8tk_listbox_select_item() 、q8tk_listbox_select_child() で
 *	リストアイテムを選択した場合とは動作が違う？？？
 *
 *---------------------------------------------------------------------------*/

static void list_scroll_adjust( Q8tkWidget *widget );

/* 文字キーを押したら、その文字を先頭にもつリストアイテムがアクティブにする  */
/* ↑↓キーを押したら、上下のリストアイテムをアクティブにする                */
/* スペース・リターンを押したら、そのリストアイテムを選択する (シグナル発生) */
static	void	list_event_key_on( Q8tkWidget *widget, int key )
{
  Q8tkWidget *w = widget->stat.listbox.active;

  if( w ){
    if( key <= 0xff && isgraph(key) ){ /* スペース以外の文字が入力された場合 */

      Q8tkWidget *active = w;
      for( ;; ){		       /* アクティブなリストアイテムより後ろ */
	w = w->next;		       /* のリストアイテムの、子(ラベル)の   */
	if( w==NULL ){		       /* 先頭文字とキー入力のが一致するもの */
	  w = widget->child;	       /* を順次探していく。リストアイテムの */
	}			       /* 末尾の次は先頭に戻り、すべてを探す */

	if( w==active || w==NULL ) break;	/* なければあきらめる */

	if( w->child ){
	  if( (w->name        && w->name[0]        == key) ||
	      (w->child->name && w->child->name[0] == key) ){
							/* 見つかったら      */
	    widget->stat.listbox.active = w;		/* それをアクティブに*/

	    list_scroll_adjust( widget );		/* 表示位置を調節 */
	    break;
	  }
	}
      }

    }else{			       /* スペースや制御文字が入力された場合 */
      switch( key ){

      case Q8TK_KEY_RET:
      case Q8TK_KEY_SPACE:
	(*w->event_button_on)( w );
	break;

      case Q8TK_KEY_UP:
	if( w->prev ){
	  widget->stat.listbox.active = w->prev;	/* 1個前をアクティブ */

	  list_scroll_adjust( widget );			/* 表示位置を調節 */
	}
	break;
      case Q8TK_KEY_DOWN:
	if( w->next ){
	  widget->stat.listbox.active = w->next;	/* 1個後をアクティブ */

	  list_scroll_adjust( widget );			/* 表示位置を調節 */
	}
	break;

      case Q8TK_KEY_PAGE_UP:
	if( widget->parent &&
	    widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW &&
	    widget->parent->stat.scrolled.vscrollbar ){

	  int h = widget->parent->stat.scrolled.height -1;
	  if( widget->stat.scrolled.hscrollbar ) h -= 3;
	  else                                   h -= 2;
	  for( ; h>0; h-- ){
	    if( w->prev==NULL ) break;
	    w = w->prev;
	  }
	  widget->stat.listbox.active = w;		/* 1頁前をアクティブ */

	  list_scroll_adjust( widget );			/* 表示位置を調節 */
	}
	break;
      case Q8TK_KEY_PAGE_DOWN:
	if( widget->parent &&
	    widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW &&
	    widget->parent->stat.scrolled.vscrollbar ){

	  int h = widget->parent->stat.scrolled.height -1;
	  if( widget->stat.scrolled.hscrollbar ) h -= 3;
	  else                                   h -= 2;
	  for( ; h>0; h-- ){
	    if( w->next==NULL ) break;
	    w = w->next;
	  }
	  widget->stat.listbox.active = w;		/* 1頁後をアクティブ */

	  list_scroll_adjust( widget );			/* 表示位置を調節 */
	}
	break;
      }
    }
  }
}



static void list_scroll_adjust( Q8tkWidget *widget )
{
  int i;
  Q8tkWidget *c;

  if( widget->parent &&
      widget->parent->type==Q8TK_TYPE_SCROLLED_WINDOW ){

    c = widget->child;
    for( i=0; c ;i++ ){
      if( c==widget->stat.listbox.active ) break;
      c = c->next;
    }

    if( i < widget->parent->stat.scrolled.child_y0 ){
      Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value = i;
    }
    else if( i >=   widget->parent->stat.scrolled.child_y0
		  + widget->parent->stat.scrolled.child_sy ){
      Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value
			= i -widget->parent->stat.scrolled.child_sy +1;
    }
  }
  set_construct_flag( TRUE );
}





Q8tkWidget	*q8tk_listbox_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_LISTBOX;
  w->attr  = Q8TK_ATTR_CONTAINER;

  w->event_key_on = list_event_key_on;

  w->stat.listbox.active = NULL;

  return	w;
}

void	q8tk_listbox_clear_items( Q8tkWidget *wlist, int start, int end )
{
  int	i, count;
  Q8tkWidget *wk, *c = wlist->child;

  if( c==NULL ) return;
  if( start<0 ) return;

  if( end < start ) count = -1;
  else              count = end - start +1;

  for( i=0; i<start; i++ ){			/* 子の LIST ITEM を start個 */
    if( (c = c->next) == NULL ) return;		/* 目まで順にたどる          */
  }
  while( count ){				/* そこから end個目までを    */
    wk = c->next;				/* 切り離す                  */
    q8tk_container_remove( wlist, c );			/* LIST ITEM と LABEL*/
    q8tk_widget_destroy( c );				/* も一気に削除する  */
    if( (c = wk) == NULL ) break;
    if( count > 0 ) count --;
  }

  set_construct_flag( TRUE );
}

static	/* LIST BOX に繋がった LIST の、 name が s に一致するものを探す */
Q8tkWidget *q8tk_listbox_search_items( Q8tkWidget *wlist, const char *s )
{
  int i;
  Q8tkWidget *c = wlist->child;

  for( ;; ){
    if( c==NULL ) return NULL;

    if( c->name &&
	strcmp( c->name, s ) == 0 ) return c;

    c=c->next;
  }
}

void	q8tk_listbox_select_item( Q8tkWidget *wlist, int item )
{
  int	i;
  Q8tkWidget *c = wlist->child;

  if( c==NULL ) return;

  if( item < 0 ) c = NULL;
  else{
    for( i=0; i<item; i++ ){
      if( (c = c->next) == NULL ) break;
    }
  }

  q8tk_listbox_select_child( wlist, c );
}

void	q8tk_listbox_select_child( Q8tkWidget *wlist, Q8tkWidget *child )
{
  if( wlist->stat.listbox.selected != child ){
    wlist->stat.listbox.selected = child;
    wlist->stat.listbox.active   = child;
    widget_signal_do( wlist, "selection_changed" );
    set_construct_flag( TRUE );
  }
  if( child ){
    widget_signal_do( child, "select" );
    set_scroll_adj_widget( child );
  }
}


/*
 * これはちょっと特殊処理。
 *	LIST BOX が SCROLLED WINDOW の子の場合で、SCROLLED WINDOW のスクロール
 *	バー(縦方向) が動かされたとき、この関数が呼ばれる。
 *	ここでは、SCROLLED WINDOW の表示範囲に応じて、LIST BOX の active
 *	ウィジットを変更している。
 */
static	void	list_event_window_scrolled( Q8tkWidget *swin, int sy )
{
  Q8tkWidget	*widget = swin->child;	/* == LISTBOX */

  Q8tkWidget	*c = widget->child;
  int		nth = 0;

  if( c==NULL ) return;

  while( c ){
    if( c==widget->stat.listbox.active ) break;
    nth ++;
    c = c->next;
  }
  nth = nth - swin->stat.scrolled.vadj->stat.adj.value;

  if( 0 <= nth && nth < sy ){
    /* Ok, No Adjust */
  }else{

    if( nth < 0 ) nth = swin->stat.scrolled.vadj->stat.adj.value;
    else          nth = swin->stat.scrolled.vadj->stat.adj.value + sy -1;

    c = widget->child;
    while( nth-- ){
      if( (c = c->next) == NULL ) return;
    }
    widget->stat.listbox.active   = c;
    set_construct_flag( TRUE );

  }
}




/*---------------------------------------------------------------------------
 * リストアイテム (LIST ITEM)
 *---------------------------------------------------------------------------
 *  ・リストボックスの子になれる。
 *  ・子を一つ持てる。 (但し、ラベル以外の子を持った場合の動作は未保証)
 *  ・子を持つには、q8tk_container_add() を使用する。
 *  ・内部に文字列を保持できる。
 *  ・シグナル
 *	"select"	クリックした時 (すでに選択された状態でも発生)
 *			親のリストボックスが選択状態としているのが自身で無い
 *			場合、先に親に "selection_changed" イベントを発生させる
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_list_item_new( void )
 *	リストアイテムの生成。
 *
 *  Q8tkWidget	*q8tk_list_item_new_with_label( const char *label )
 *	文字列 label が描かれたリストアイテムの生成。
 *	内部的には、リストアイテムとラベルを作り、親子にする。
 *
 *  void	q8tk_list_item_set_string( Q8tkWidget *w, const char *str )
 *	内部に文字列 str を保持する。
 *  -------------------------------------------------------------------------
 *	LIST BOX ←→ 【LIST ITEM】←→ [LABEL]		ラベルを子に持つ
 *							親は、リストボックス
 *	   ↑		    ↑
 *	   ：		    ：
 *   "selection_change"     ：
 *	   ：		  "select"
 *	   ：………………   ：
 *	先に親に	    ：
 *	伝播する        ボタンで選択
 *
 *---------------------------------------------------------------------------*/
/* リストアイテムをマウスクリックしたら、シグナル発生 */
static	void	list_item_event_button_on( Q8tkWidget *widget )
{
  if( widget->parent ){
    q8tk_widget_grab_default( widget->parent );

    if( widget->parent->stat.listbox.selected != widget ){
      widget->parent->stat.listbox.selected = widget;
      widget->parent->stat.listbox.active   = widget;

      widget_redraw_now();			/* 選択した状態を一旦描画する*/
      widget_signal_do( widget->parent, "selection_changed" );
    }
  }
  widget_signal_do( widget, "select" );
  set_construct_flag( TRUE );
}

Q8tkWidget	*q8tk_list_item_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_LIST_ITEM;
  w->attr  = Q8TK_ATTR_CONTAINER | Q8TK_ATTR_LABEL_CONTAINER;

  w->event_button_on = list_item_event_button_on;

  return	w;
}

Q8tkWidget	*q8tk_list_item_new_with_label( const char *label )
{
  Q8tkWidget	*i = q8tk_list_item_new();
  Q8tkWidget	*l = q8tk_label_new( label );

  q8tk_widget_show( l );
  q8tk_container_add( i, l );

  i->with_label = TRUE;

  return	i;
}

void		q8tk_list_item_set_string( Q8tkWidget *w, const char *str )
{
  if( w->name ) free( w->name );
  w->name  = (char *)malloc( strlen(str)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, str );
  w->code = kanji_code;
}




/*---------------------------------------------------------------------------
 * ラベル (LABEL)
 *---------------------------------------------------------------------------
 *  ・(表示用の)文字列を保持できる
 *  ・さまざまなコンテナの子になる。
 *  ・シグナル … 無し
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_label_new( const char *label )
 *	文字列 label をもつラベルの生成。
 *
 *  void	q8tk_label_set( Q8tkWidget *w, const char *label )
 *	ラベルの文字列を label に変更する。
 *
 *  void	q8tk_label_set_reverse( Q8tkWidget *w, int reverse )
 *	ラベルの文字列の反転表示を設定する。 reverse が真なら反転する。
 *
 *  void	q8tk_label_set_color( Q8tkWidget *w, int foreground )
 *	ラベルの文字列の色を設定する。色は foreground で設定する。値は
 *	Q8GR_PALETTE_xxxx で指定する。この値が負なら、デフォルトの色に戻す。
 *  -------------------------------------------------------------------------
 *	【LABEL】			子は持てない
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_label_new( const char *label )
{
  Q8tkWidget	*w;

  if( label==NULL ) label="";

  w = malloc_widget();
  w->type  = Q8TK_TYPE_LABEL;
  w->name  = (char *)malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );
  w->code = kanji_code;

  w->stat.label.foreground = Q8GR_PALETTE_FOREGROUND;
  w->stat.label.background = Q8GR_PALETTE_BACKGROUND;
  w->stat.label.reverse    = FALSE;

  return	w;
}
void		q8tk_label_set( Q8tkWidget *w, const char *label )
{
  if( label==NULL ) label="";

  if( w->name ) free( w->name );
  w->name  = (char *)malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );
  w->code = kanji_code;
  set_construct_flag( TRUE );
}
void		q8tk_label_set_reverse( Q8tkWidget *w, int reverse )
{
  if( w->stat.label.reverse != reverse ){
    w->stat.label.reverse = reverse;
    set_construct_flag( TRUE );
  }
}
void		q8tk_label_set_color( Q8tkWidget *w, int foreground )
{
  if( foreground < 0 ) foreground = Q8GR_PALETTE_FOREGROUND;

  if( w->stat.label.foreground != foreground ){
    w->stat.label.foreground = foreground;
    set_construct_flag( TRUE );
  }
}


/*---------------------------------------------------------------------------
 * ロゴ (LOGO)
 *---------------------------------------------------------------------------
 *  ・タイトルロゴを表示できる(内容,サイズは固定)
 *  ・さまざまなコンテナの子になる。
 *  ・シグナル … 無し
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_logo_new( void )
 *	ロゴの生成。
 *
 *  -------------------------------------------------------------------------
 *	【LOGO】			子は持てない
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_logo_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_LOGO;

  return	w;
}


/*---------------------------------------------------------------------------
 * フレーム (FRAME)
 *---------------------------------------------------------------------------
 *  ・子をひとつもてる。
 *  ・子を持つには、q8tk_container_add() を使用する。
 *  ・(見出しの)文字列を保持できる。
 *  ・シグナル … 無し
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_frame_new( const char *label )
 *	文字列 label を見出しにもつフレームの生成。
 *  void       q8tk_frame_set_shadow_type( Q8tkWidget *frame, int shadow_type )
 *	フレームの種類を設定できる。
 *		Q8TK_SHADOW_NONE	枠線のないフレーム
 *		Q8TK_SHADOW_IN		全体がへこんだフレーム
 *		Q8TK_SHADOW_OUT		全体が盛り上がったフレーム
 *		Q8TK_SHADOW_ETCHED_IN	枠がへこんだフレーム
 *		Q8TK_SHADOW_ETCHED_OUT	枠が盛り上がったフレーム
 *  -------------------------------------------------------------------------
 *	【FRAME】 ←→ [xxxx]		いろんな子が持てる
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_frame_new( const char *label )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type  = Q8TK_TYPE_FRAME;
  w->attr  = Q8TK_ATTR_CONTAINER;

  w->stat.frame.shadow_type = Q8TK_SHADOW_OUT;

  w->name  = (char *)malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );
  w->code = kanji_code;

  return	w;
}

void	q8tk_frame_set_shadow_type( Q8tkWidget *frame, int shadow_type )
{
  frame->stat.frame.shadow_type = shadow_type;
}




/*---------------------------------------------------------------------------
 * 水平ボックス (HBOX)
 *---------------------------------------------------------------------------
 *  ・複数の子をもてる。
 *  ・子を持つには、q8tk_box_pack_start() / _end ()を使用する。
 *  ・シグナル … 無し
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hbox_new( void )
 *	水平ボックスの生成。
 *  -------------------------------------------------------------------------
 *	【HBOX】←→ [xxxx]		いろんな子が持てる
 *		      ↑↓
 *		←   [xxxx]
 *		      ↑↓
 *		←   [xxxx]
 *		      ↑↓
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_hbox_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_HBOX;
  w->attr   = Q8TK_ATTR_CONTAINER;

  return	w;
}




/*---------------------------------------------------------------------------
 * 垂直ボックス (VBOX)
 *---------------------------------------------------------------------------
 *  ・複数の子をもてる。
 *  ・子を持つには、q8tk_box_pack_start() / _end ()を使用する。
 *  ・シグナル … 無し
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_vbox_new( void )
 *	垂直ボックスの生成。
 *  -------------------------------------------------------------------------
 *	【VBOX】←→ [xxxx]		いろんな子が持てる
 *		      ↑↓
 *		←   [xxxx]
 *		      ↑↓
 *		←   [xxxx]
 *		      ↑↓
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_vbox_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_VBOX;
  w->attr   = Q8TK_ATTR_CONTAINER;

  return	w;
}




/*---------------------------------------------------------------------------
 * ノートブック (NOTE BOOK / NOTE PAGE )
 *---------------------------------------------------------------------------
 *  ・複数のノートページをもてる。
 *  ・各ノートページはコンテナであり、子を一つ持てる。
 *  ・q8tk_notebook_append() で子を持つが、その度に内部でノートページを生成し
 *    これが子を持つことになる。
 *  ・NOTE PAGE は、(見出しの)文字列を保持できる。
 *  ・シグナル
 *	"switch_page"	別のページに切り替わった時に発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_notebook_new( void )
 *	ノートボックの生成
 *
 *  void	q8tk_notebook_append( Q8tkWidget *notebook,
 *				      Q8tkWidget *widget, const char *label )
 *	ノートブック noteboot  に新たなページを作成する。
 *	そのページに widget を追加する。見出しの文字列は label。
 *	内部的には、ノートページを作り、親子にしたうえでさらに、
 *	ノートページと wiget を親子にする、
 *
 *  int	q8tk_notebook_current_page( Q8tkWidget *notebook )
 *	現在のページ番号を取得する。
 *	なお、ノートブックにページを追加した順に、0 からの番号が振られる。
 *	全くページを追加していない場合は -1 。
 *
 *  void	q8tk_notebook_set_page( Q8tkWidget *notebook, int page_num )
 *	ノートブックのページを指定する(シグナルが発生する)。
 *
 *  void	q8tk_notebook_next_page( Q8tkWidget *notebook )
 *	ノートブックのページを次頁にする(シグナルが発生する)。
 *
 *  void	q8tk_notebook_prev_page( Q8tkWidget *notebook )
 *	ノートブックのページを前頁にする(シグナルが発生する)。
 *  -------------------------------------------------------------------------
 *	【NOTE BOOX】←→【NOTE PAGE】←→ [xxxx]	ノートページを子に持ち
 *			      ↑↓			さらにノートページは
 *		     ←  【NOTE PAGE】←→ [xxxx]	いろんな子が持てる
 *			      ↑↓
 *		     ←  【NOTE PAGE】←→ [xxxx]
 *			      ↑↓
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_notebook_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_NOTEBOOK;
  w->attr   = Q8TK_ATTR_CONTAINER;
  w->stat.notebook.page = NULL;

  return	w;
}

/* ノートページのタグをマウスクリックしたら、シグナル発生 */
static	void	notepage_event_button_on( Q8tkWidget *widget )
{
  if( (widget->parent)->stat.notebook.page != widget ){
    (widget->parent)->stat.notebook.page = widget;
    widget_signal_do( widget->parent, "switch_page" );
    set_construct_flag( TRUE );
  }
}

/* リターン・スペースを押したら、マウスクリックと同じ処理をする */
static	void	notepage_event_key_on( Q8tkWidget *widget, int key )
{
  if( key==Q8TK_KEY_RET || key==Q8TK_KEY_SPACE ){
    notepage_event_button_on( widget );
  }
}

void		q8tk_notebook_append( Q8tkWidget *notebook,
				      Q8tkWidget *widget, const char *label )
{
  Q8tkWidget	*w;

  Q8tkAssert(notebook->type==Q8TK_TYPE_NOTEBOOK,NULL);

  w = malloc_widget();
  w->type   = Q8TK_TYPE_NOTEPAGE;
  w->attr   = Q8TK_ATTR_CONTAINER;
  w->name   = (char *)malloc( strlen(label)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, label );
  w->code = kanji_code;

  if( notebook->child == NULL ){		/* 最初の NOTE PAGE の場合は*/
    notebook->stat.notebook.page = w;		/* それを 選択状態にする    */
  }
  q8tk_box_pack_start( notebook, w );		/* コンテナ処理は BOXと同じ */

  q8tk_container_add( w, widget );
  q8tk_widget_show(w);

  w->event_button_on  = notepage_event_button_on;
  w->event_key_on     = notepage_event_key_on;
}

int	q8tk_notebook_current_page( Q8tkWidget *notebook )
{
  Q8tkWidget *child = notebook->child;
  int	i = 0;

  while( child ){
    if( child == notebook->stat.notebook.page ){
      return i;
    }
    child = child->next;
    i++;
  }
  return -1;
}

void	q8tk_notebook_set_page( Q8tkWidget *notebook, int page_num )
{
  Q8tkWidget *child = notebook->child;
  int	i = 0;

  while( child ){
    if( i==page_num ){
      if( notebook->stat.notebook.page != child ){
	notebook->stat.notebook.page = child;
	widget_signal_do( notebook, "switch_page" );
	set_construct_flag( TRUE );
      }
      break;
    }else{
      child = child->next;
      i++;
    }
  }
}

void	q8tk_notebook_next_page( Q8tkWidget *notebook )
{
  if( notebook->child ){
    Q8tkWidget *page = notebook->stat.notebook.page;
    if( page && page->next ){
      notebook->stat.notebook.page = page->next;
      set_construct_flag( TRUE );
    }
  }
}

void	q8tk_notebook_prev_page( Q8tkWidget *notebook )
{
  if( notebook->child ){
    Q8tkWidget *page = notebook->stat.notebook.page;
    if( page && page->prev ){
      notebook->stat.notebook.page = page->prev;
      set_construct_flag( TRUE );
    }
  }
}



/*---------------------------------------------------------------------------
 * 垂直セパレータ (VSEPARATOR)
 *---------------------------------------------------------------------------
 *  ・子は持てない。
 *  ・長さが、親ウィジットの大きさにより、動的に変わる。
 *  ・シグナル … なし
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_vseparator_new( void )
 *	垂直セパレータの生成
 *  -------------------------------------------------------------------------
 *	【VSEPARATOR】			子は持てない
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_vseparator_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_VSEPARATOR;

  return	w;
}




/*---------------------------------------------------------------------------
 * 水平セパレータ (HSEPARATOR)
 *---------------------------------------------------------------------------
 *  ・子は持てない。
 *  ・長さが、親ウィジットの大きさにより、動的に変わる。
 *  ・シグナル … なし
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hseparator_new( void )
 *	水平セパレータの生成
 *  -------------------------------------------------------------------------
 *	【VSEPARATOR】			子は持てない
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_hseparator_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_HSEPARATOR;

  return	w;
}




/*---------------------------------------------------------------------------
 * アジャストメント (ADJUSTMENT)
 *---------------------------------------------------------------------------
 *  ・表示できない
 *  ・子は持てない。子になることもできない。
 *  ・スケールやスクロールドウインドウを生成する際に、必要となる。
 *    単独では使用することはない。
 *  ・レンジ(値の範囲)と増分(小さい増分、大きい増分の2種類) を持つが、
 *    いずれも整数に限る。
 *  ・スケールを生成する場合は、前もってアジャストメントを生成しておくのが
 *    おくのが一般的。
 *  ・スクロールドウインドウを生成する場合は、その時にアジャストメントを自動
 *    生成させるのが一般的。
 *  ・シグナル
 *	"value_changed"		値が変わった時に発生
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_adjustment_new( int value, int lower, int upper,
 *				      int step_increment, int page_increment )
 *	アジャストメントの生成。
 *	値の範囲は lower 〜 upper 、現在値は value 、
 *	大きい増分は step_increment 、小さい増分は page_increment 。
 *
 *  void  q8tk_adjustment_set_value( Q8tkWidget *adj, int value )
 *	アジャストメントの現在値を設定する。シグナルは発生しない。
 *
 *  void  q8tk_adjustment_clamp_page( Q8tkWidget *adj, int lower, int upper )
 *	値の範囲を lower 〜 upper に設定しなおす。この時、現在値が範囲外の場合
 *	最大値ないし最小値に設定される。シグナルは発生しない。
 *
 *  void	q8tk_adjustment_set_arrow( Q8tkWidget *adj, int arrow )
 *	arrow が真の場合、矢印を表示する。
 *	なお、矢印がクリックされた場合は ± step_increment 分、増減する。
 *	背景部分がクリックされた場合は ± page_increment 分、増減する。
 *
 *  void	q8tk_adjustment_set_length( Q8tkWidget *adj, int length )
 *
 *   ------------------------------------------------------------------------
 *	【ADJUSTMENT】			子は持てないし、子にもなれない
 *
 *---------------------------------------------------------------------------*/
/* アジャストメントの調整
   アジャストメントをマウスクリック、ドラッグした際に呼び出される */
enum{ ADJ_STEP_DEC, ADJ_STEP_INC, ADJ_PAGE_DEC, ADJ_PAGE_INC, ADJ_SLIDER_DRAG};
static	void	adjustment_do( Q8tkWidget *widget, int mode )
{
  Q8Adjust	*adj = &widget->stat.adj;
  int	value_old = adj->value;
  int	repeat = TRUE;
  static int old = -1;

  if( now_mouse_on == FALSE ) old = -1;

  switch( mode ){
  case ADJ_STEP_DEC:
    adj->value -= adj->step_increment;
    if( adj->value < adj->lower ){ adj->value = adj->lower;   repeat = FALSE; }
    break;
  case ADJ_STEP_INC:
    adj->value += adj->step_increment;
    if( adj->value > adj->upper ){ adj->value = adj->upper;   repeat = FALSE; }
    break;
  case ADJ_PAGE_DEC:
    if( old == ADJ_PAGE_INC ){
      mode = -1;
      repeat = FALSE;
      break;
    }
    adj->value -= adj->page_increment;
    if( adj->value < adj->lower ){ adj->value = adj->lower;   repeat = FALSE; }
    break;
  case ADJ_PAGE_INC:
    if( old == ADJ_PAGE_DEC ){
      mode = -1;
      repeat = FALSE;
      break;
    }
    adj->value += adj->page_increment;
    if( adj->value > adj->upper ){ adj->value = adj->upper;   repeat = FALSE; }
    break;
  case ADJ_SLIDER_DRAG:
    set_drag_widget( widget );
    return;
  }

  if( value_old != adj->value ){
    widget_signal_do( widget, "value_changed" );
  }
  set_construct_flag( TRUE );

  if( repeat ) now_mouse_on = TRUE;
  else         now_mouse_on = FALSE;

  old = mode;
}

/* アジャストメントの上にてマウスクリックしたら、値調整しシグナル発生 */
static	void	adjustment_event_button_on( Q8tkWidget *widget )
{
  Q8Adjust	*adj = &widget->stat.adj;
  int	slider_x, slider_y;
  int	arrow_lower_x, arrow_lower_y;
  int	arrow_upper_x, arrow_upper_y;
  int	m_x = mouse.x / 8;
  int	m_y = mouse.y / 16;

  if( adj->horizontal ){			/* HORIZONTAL ADJUSTMENT */

    slider_x = adj->x + adj->pos;
    slider_y = adj->y;

    if( m_y == slider_y ){

      if( adj->arrow ){
	arrow_lower_x = adj->x;		slider_x ++;
	arrow_upper_x = adj->x + adj->length +1;

	if      ( m_x == arrow_lower_x ){
	  adjustment_do( widget, ADJ_STEP_DEC );	return;
	}else if( m_x == arrow_upper_x ){
	  adjustment_do( widget, ADJ_STEP_INC );	return;
	}
      }

      if      ( m_x == slider_x ){
	adjustment_do( widget, ADJ_SLIDER_DRAG );	return;
      }else if( m_x < slider_x ){
	adjustment_do( widget, ADJ_PAGE_DEC );		return;
      }else{
	adjustment_do( widget, ADJ_PAGE_INC );		return;
      }

    }else{
      return;
    }

  }else{					/* VIRTICAL ADJUSTMENT */

    slider_x = adj->x;
    slider_y = adj->y + adj->pos;

    if( m_x == slider_x ){

      if( adj->arrow ){
	arrow_lower_y = adj->y;		slider_y ++;
	arrow_upper_y = adj->y + adj->length +1;
	if      ( m_y == arrow_lower_y ){
	  adjustment_do( widget, ADJ_STEP_DEC );	return;
	}else if( m_y == arrow_upper_y ){
	  adjustment_do( widget, ADJ_STEP_INC );	return;
	}
      }

      if      ( m_y == slider_y ){
	adjustment_do( widget, ADJ_SLIDER_DRAG );	return;
      }else if( m_y < slider_y ){
	adjustment_do( widget, ADJ_PAGE_DEC );		return;
      }else{
	adjustment_do( widget, ADJ_PAGE_INC );		return;
      }

    }else{
      return;
    }

  }
}

/* アジャストメントのスライダーをドラッグしたら、値調整しシグナル発生 */
static	void	adjustment_event_dragging( Q8tkWidget *widget )
{
  Q8Adjust	*adj = &widget->stat.adj;
  int	m_x = mouse.x / 8;
  int	m_y = mouse.y / 16;
  int	adj_x = adj->x;
  int	adj_y = adj->y;
  int	slider = -1;

  if( adj->upper <= adj->lower ) return;

  if( adj->horizontal ){
    if( adj->arrow ) adj_x ++;
    if( adj_x <= m_x && m_x < adj_x+adj->length ){
      slider = m_x - adj_x;
    }
  }else{
    if( adj->arrow ) adj_y ++;
    if( adj_y <= m_y && m_y < adj_y+adj->length ){
      slider = m_y - adj_y;
    }
  }

  if( slider >= 0 ){
    float val0 = ( slider -1 ) * adj->scale   + adj->lower;
    float val1 = ( slider    ) * adj->scale   + adj->lower;
    int   val;

    if     ( slider <= 0 )             val = adj->lower;
    else if( slider >= adj->length-1 ) val = adj->upper;
    else{
      float base = (float)(adj->upper-adj->lower) / (adj->length-1);
      int   val2 = (int)( val1 / base ) * base;
      if( val0<val2 && val2<=val1 ) val = val2;
      else                          val = ( val0 + val1 ) / 2;
    }

    if( adj->value != val ){
      adj->value = val;
      widget_signal_do( widget, "value_changed" );
    }
    set_construct_flag( TRUE );
  }
}

/* カーソルキーなどを押したら、マウスクリックと同じ処理をする */
static	void	adjustment_event_key_on( Q8tkWidget *widget, int key )
{
  Q8Adjust	*adj = &widget->stat.adj;

  if( adj->horizontal ){			/* HORIZONTAL ADJUSTMENT */
    if     ( key==Q8TK_KEY_LEFT  ) adjustment_do( widget, ADJ_STEP_DEC );
    else if( key==Q8TK_KEY_RIGHT ) adjustment_do( widget, ADJ_STEP_INC );
  }else{					/* VIRTICAL ADJUSTMENT */
    if     ( key==Q8TK_KEY_UP  )  adjustment_do( widget, ADJ_STEP_DEC );
    else if( key==Q8TK_KEY_DOWN ) adjustment_do( widget, ADJ_STEP_INC );
    else if( key==Q8TK_KEY_PAGE_UP )   adjustment_do( widget, ADJ_PAGE_DEC );
    else if( key==Q8TK_KEY_PAGE_DOWN ) adjustment_do( widget, ADJ_PAGE_INC );
  }
}

Q8tkWidget	*q8tk_adjustment_new( int value, int lower, int upper,
				      int step_increment, int page_increment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_ADJUSTMENT;

  w->event_button_on = adjustment_event_button_on;
  w->event_dragging  = adjustment_event_dragging;
  w->event_key_on    = adjustment_event_key_on;

  w->stat.adj.value = value;
  w->stat.adj.lower = lower;
  w->stat.adj.upper = upper;
  w->stat.adj.step_increment = step_increment;
  w->stat.adj.page_increment = page_increment;

  return	w;
}

void	q8tk_adjustment_set_value( Q8tkWidget *adj, int value )
{
  if( value < adj->stat.adj.lower ) value = adj->stat.adj.lower;
  if( value > adj->stat.adj.value ) value = adj->stat.adj.upper;

  if( adj->stat.adj.value != value ){
    adj->stat.adj.value = value;

    /* シグナルは発生させない */
    /*
    widget_signal_do( adj, "value_changed" );
    */

    set_construct_flag( TRUE );
  }
}

void	q8tk_adjustment_clamp_page( Q8tkWidget *adj, int lower, int upper )
{
  if( upper < lower ) upper = lower;

  adj->stat.adj.lower = lower;
  adj->stat.adj.upper = upper;
  if( adj->stat.adj.value < adj->stat.adj.lower )
				adj->stat.adj.value = adj->stat.adj.lower;
  if( adj->stat.adj.value > adj->stat.adj.upper )
				adj->stat.adj.value = adj->stat.adj.upper;

  /* シグナルは発生させない */

  set_construct_flag( TRUE );
}

void		q8tk_adjustment_set_arrow( Q8tkWidget *adj, int arrow )
{
  if( adj->stat.adj.arrow != arrow ){
    adj->stat.adj.arrow = arrow;
    set_construct_flag( TRUE );
  }
}

void		q8tk_adjustment_set_length( Q8tkWidget *adj, int length )
{
  if( adj->stat.adj.max_length != length ){
    adj->stat.adj.max_length = length;
    set_construct_flag( TRUE );
  }
}




/*---------------------------------------------------------------------------
 * 水平スケール (HSCALE)
 *---------------------------------------------------------------------------
 *  ・new()時に、引数でアジャストメントを指定する。
 *    スケールのレンジ(範囲)や増分は、このアジャストメントに依存する。
 *  ・new()時の引数が NULL の場合は、自動的にアジャストメントが生成されるが、
 *    この時のレンジは 0〜10 、増分は 1 と 2 に固定である。(変更可能)
 *  ・子は持てない
 *  ・シグナル … なし。
 *	ただし、アジャストメントはシグナルを受ける。
 *	アジャストメントを自動生成した場合、どうやってシグナルをセットする？
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hscale_new( Q8tkWidget *adjustment )
 *	水平スケールを生成。
 *	予め作成済みのアジャストメント adjustment を指定する。 
 *	NULL なら内部で自動生成される。
 *
 *  void	q8tk_scale_set_draw_value( Q8tkWidget *scale, int draw_value )
 *	draw_value が真なら、現在値をスライダーの横に表示する。
 *
 *  void	q8tk_scale_set_value_pos( Q8tkWidget *scale, int pos )
 *	現在値を表示する位置を決める。
 *		Q8TK_POS_LEFT	スライダーの左に表示
 *		Q8TK_POS_RIGHT	スライダーの右に表示
 *		Q8TK_POS_TOP	スライダーの上に表示
 *		Q8TK_POS_BOTTOM	スライダーの下に表示
 *  -------------------------------------------------------------------------
 *	【HSCALE】				子は持てない
 *	     └────	【ADJUSTMENT】
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_hscale_new( Q8tkWidget *adjustment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_HSCALE;

  if( adjustment ){
    w->stat.scale.adj = adjustment;
  }else{
    w->stat.scale.adj = q8tk_adjustment_new( 0, 0, 10, 1, 2 );
  }

  w->stat.scale.adj->stat.adj.horizontal = TRUE;
  w->stat.scale.adj->parent = w;

  return	w;
}




/*---------------------------------------------------------------------------
 * 垂直スケール (HSCALE)
 *---------------------------------------------------------------------------
 *  ・new()時に、引数でアジャストメントを指定する。
 *    スケールのレンジ(範囲)や増分は、このアジャストメントに依存する。
 *  ・new()時の引数が NULL の場合は、自動的にアジャストメントが生成されるが、
 *    この時のレンジは 0〜10 、増分は 1 と 2 に固定である。(変更可能)
 *  ・子は持てない
 *  ・シグナル … なし。
 *	ただし、アジャストメントはシグナルを受ける。
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hscale_new( Q8tkWidget *adjustment )
 *	垂直スケールを生成。
 *	予め作成済みのアジャストメント adjustment を指定する。 
 *	NULL なら内部で自動生成される。
 *
 *  void	q8tk_scale_set_draw_value( Q8tkWidget *scale, int draw_value )
 *	水平スケールと同じ。
 *
 *  void	q8tk_scale_set_value_pos( Q8tkWidget *scale, int pos )
 *	水平スケールと同じ。
 *  -------------------------------------------------------------------------
 *	【VSCALE】				子は持てない
 *	     └────	【ADJUSTMENT】
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_vscale_new( Q8tkWidget *adjustment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_VSCALE;

  if( adjustment ){
    w->stat.scale.adj = adjustment;
  }else{
    w->stat.scale.adj = q8tk_adjustment_new( 0, 0, 10, 1, 2 );
  }

  w->stat.scale.adj->stat.adj.horizontal = FALSE;
  w->stat.scale.adj->parent = w;

  return	w;
}

void		q8tk_scale_set_value_pos( Q8tkWidget *scale, int pos )
{
  scale->stat.scale.value_pos = pos;
  set_construct_flag( TRUE );
}

void		q8tk_scale_set_draw_value( Q8tkWidget *scale, int draw_value )
{
  scale->stat.scale.draw_value = draw_value;
  set_construct_flag( TRUE );
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 表示時のサイズ計算 (widget_size()内から呼ばれる)
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static	void	adjustment_size( Q8Adjust *adj, int *sx, int *sy )
{
  int i, limit;
  int range = ( adj->upper - adj->lower + 1 );

  if( adj->horizontal ) limit = Q8GR_SCREEN_X * 0.8;
  else                  limit = Q8GR_SCREEN_Y * 0.8;

  if( adj->max_length <= 2 ){
    for( i=1; ; i++ ) if( range / i < limit ) break;
    adj->length = range / i;
  }else{
    adj->length = adj->max_length;
  }
  adj->scale = (float)( range - 1 ) / (adj->length - 2 );
  if     ( adj->value == adj->lower ) adj->pos = 0;
  else if( adj->value == adj->upper ) adj->pos = adj->length-1;
  else{
    adj->pos = (adj->value-adj->lower) / adj->scale + 1;
    if( adj->pos >= adj->length-1 ) adj->pos = adj->length-2;
  }

  if( adj->horizontal ){
    *sx = adj->length + ((adj->arrow) ? +2: 0 );
    *sy = 1;
  }else{
    *sx = 1;
    *sy = adj->length + ((adj->arrow) ? +2: 0 );
  }
}




/*---------------------------------------------------------------------------
 * スクロールドウインドウ (SCROLLED WINDOW)
 *---------------------------------------------------------------------------
 *  ・new()時に、引数でアジャストメントを指定するが、スケールのレンジ(範囲)は
 *    このスクロールドウインドウの子の大きさによって、動的に変化する。
 *     (増分は引き継がれる)
 *  ・new()時の引数が NULL の場合は、自動的にアジャストメントが生成される。
 *    この時の増分は 1 と 10 である。
 *    特に理由がなければ、NULL による自動生成の方が簡単で便利である。
 *  ・子を一つ持てる。
 *    (但し、子や孫がスクロールウインドウを持つような場合の動作は未保証)
 *  ・シグナル … なし
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_scrolled_window_new( Q8tkWidget *hadjustment,
 *					   Q8tkWidget *vadjustment )
 *	スクロールドウインドウの生成。
 *	予め作成済みのアジャストメント adjustment を指定する。 
 *	NULL なら内部で自動生成される。
 *
 *  void	q8tk_scrolled_window_set_policy( Q8tkWidget *scrolledw,
 *						  int hscrollbar_policy,
 *						  int vscrollbar_policy )
 *	スクロールバーの表示方法を縦・横個別に設定する。
 *		Q8TK_POLICY_ALWAYS	常に表示する
 *		Q8TK_POLICY_AUTOMATIC	表示サイズが大きい場合のみ表示する
 *		Q8TK_POLICY_NEVER	表示しない
 *
 *  void	q8tk_misc_set_size( Q8tkWidget *widget, int width, int height )
 *	ウインドウのサイズ width 、 height を指定する。
 *  -------------------------------------------------------------------------
 *	【SCROLLED WINDOW】←→ [xxxx]		いろんな子が持てる
 *		｜｜
 *		｜└─	【ADJUSTMENT】
 *		└──	【ADJUSTMENT】
 *
 *
 *
 *	【SCROLLED WINDOW】←→ [LISTBOX]
 *
 *	スクロールドウインドウの子がリストボックスの時、
 *	スクロールドウインドウをクリックすると、リストボックスに
 *	フォーカスが設定される。つまり、キー入力は全てリストボックスに伝わる。
 *
 *---------------------------------------------------------------------------*/
/* ウインドウ内をマウスクリックした時
   直下の子がリストボックスの場合に限って、リストボックスをアクティブにする */
static	void	scrolled_window_event_button_on( Q8tkWidget *widget )
{
  if( widget->child && widget->child->type==Q8TK_TYPE_LISTBOX ){
    q8tk_widget_grab_default( widget->child );
    set_construct_flag( TRUE );
  }
}
Q8tkWidget	*q8tk_scrolled_window_new( Q8tkWidget *hadjustment,
					   Q8tkWidget *vadjustment )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_SCROLLED_WINDOW;
  w->attr   = Q8TK_ATTR_CONTAINER;

  w->stat.scrolled.width  = 10;
  w->stat.scrolled.height = 10;
  w->stat.scrolled.hpolicy = Q8TK_POLICY_ALWAYS;
  w->stat.scrolled.vpolicy = Q8TK_POLICY_ALWAYS;
  w->stat.scrolled.hscrollbar = TRUE;
  w->stat.scrolled.vscrollbar = TRUE;

  if( hadjustment ) w->stat.scrolled.hadj = hadjustment;
  else{             w->stat.scrolled.hadj = q8tk_adjustment_new( 0, 0,7, 1,10);
                    w->with_label = TRUE; 
  }
  q8tk_adjustment_set_arrow( w->stat.scrolled.hadj, TRUE );
  q8tk_adjustment_set_length( w->stat.scrolled.hadj, 7 );
  w->stat.scrolled.hadj->stat.adj.horizontal = TRUE;
  w->stat.scrolled.hadj->parent = w;

  if( vadjustment ) w->stat.scrolled.vadj = vadjustment;
  else{             w->stat.scrolled.vadj = q8tk_adjustment_new( 0, 0,7, 1,10);
		    w->with_label = TRUE;
  }
  q8tk_adjustment_set_arrow( w->stat.scrolled.vadj, TRUE );
  q8tk_adjustment_set_length( w->stat.scrolled.vadj, 7 );
  w->stat.scrolled.vadj->stat.adj.horizontal = FALSE;
  w->stat.scrolled.vadj->parent = w;

  w->event_button_on  = scrolled_window_event_button_on;

  return	w;
}
void		q8tk_scrolled_window_set_policy( Q8tkWidget *scrolledw,
						  int hscrollbar_policy,
						  int vscrollbar_policy )
{
  scrolledw->stat.scrolled.hpolicy = hscrollbar_policy;
  scrolledw->stat.scrolled.vpolicy = vscrollbar_policy;

  if      ( scrolledw->stat.scrolled.hpolicy==Q8TK_POLICY_ALWAYS ){
    scrolledw->stat.scrolled.hscrollbar = TRUE;
  }else if( scrolledw->stat.scrolled.hpolicy==Q8TK_POLICY_NEVER  ){
    scrolledw->stat.scrolled.hscrollbar = FALSE;
  }

  if      ( scrolledw->stat.scrolled.vpolicy==Q8TK_POLICY_ALWAYS ){
    scrolledw->stat.scrolled.vscrollbar = TRUE;
  }else if( scrolledw->stat.scrolled.vpolicy==Q8TK_POLICY_NEVER  ){
    scrolledw->stat.scrolled.vscrollbar = FALSE;
  }

  set_construct_flag( TRUE );
}
#if 0
void		q8tk_scrolled_window_set_width_height( Q8tkWidget *w, 
						       int width, int height )
{
  w->stat.scrolled.width  = width;
  w->stat.scrolled.height = height;

  set_construct_flag( TRUE );
}
#endif






/*---------------------------------------------------------------------------
 * エントリー (ENTRY)
 *---------------------------------------------------------------------------
 *  ・文字の入力が可能。但し、文字は ASCIIのみ。表示・削除は漢字も可
 *  ・子は持てない。
 *  ・シグナル
 *	"activate"	リターンキー入力があった時に発生
 *	"changed"	文字入力、文字削除があった時に発生
 *  ------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_entry_new( void )
 *	エントリを生成。入力可能な文字数は無制限。
 *
 *  Q8tkWidget	*q8tk_entry_new_with_max_length( int max )
 *	エントリを生成。 入力可能な文字数は、max 文字まで。(max==0で無制限)
 *
 *  void	q8tk_entry_set_text( Q8tkWidget *entry, const char *text )
 *	エントリに、文字列 text を設定する。シグナルは発生しない。
 *	カーソルは非表示となり、表示位置は文字列先頭となる。
 *	ただし、ウィジットがアクティブかつ編集可能な場合、カーソルは文字列末端
 *	に移動し、カーソルが表示されるように表示位置が変化する。
 *
 *  void	q8tk_entry_set_position( Q8tkWidget *entry, int position )
 *	エントリ内部の文字列のカーソル位置を position に設定する。
 *	position == -1 の場合は、カーソルは表示されない。
 *	position が文字列末尾より後ろの場合、カーソルは文字列末尾に表示される。
 *	カーソル表示する場合、カーソルが表示されるように表示位置が変化する。
 *
 *  void	q8tk_entry_set_max_length( Q8tkWidget *entry, int max )
 *	エントリに入力可能な文字数を max に変更する。(max==0で無制限)
 *
 *  void	q8tk_entry_set_editable( Q8tkWidget *entry, int editable )
 *	エントリ領域の入力可・不可を設定する。 editable が 真なら入力可。
 *
 *  void	q8tk_misc_set_size( Q8tkWidget *widget, int width, int height )
 *	エントリの表示サイズ width を指定する。 height は無効。
 *	カーソル表示する場合、カーソルが表示されるように表示位置が変化する。
 *  -------------------------------------------------------------------------
 *	【ENTRY】			子は持てない
 *
 *---------------------------------------------------------------------------*/
/*
 * strlen()の結果が strsize であるような文字列をセット可能なメモリを確保
 */
static	void	q8tk_entry_malloc( Q8tkWidget *entry, int strsize )
{
#if 0
	/* strsize より 1バイト余分に確保 ( '\0' の分を余分に ) */
  int malloc_size = strsize + 1;
#else
	/* 1バイト単位でなく、512バイト単位で確保するようにしてみよう */
  int malloc_size = (( (strsize + 1)/512 )+1)*512;
#endif

  if( entry->name == NULL ||
      entry->stat.entry.malloc_length < malloc_size ){
    if( entry->name == NULL ){
      entry->name = (char *)malloc( malloc_size );
    }else{
      entry->name = (char *)realloc( entry->name, malloc_size );
    }

    Q8tkAssert(entry->name,"memory exhoused");
    CHECK_MALLOC_OK( entry->name, "memory" );

    entry->stat.entry.malloc_length = malloc_size;
  }
}

/* エントリをマウスクリックしたら、カーソル移動 (編集可能時のみ) */
static	void	entry_event_button_on( Q8tkWidget *widget )
{
  int	m_x = mouse.x / 8;

  if( widget->stat.entry.editable == FALSE ) return;

  if( widget->stat.entry.cursor_pos < 0 ){	/* カーソル非表示なら       */
						/* 文字列末尾にカーソル移動 */
    q8tk_entry_set_position( widget, strlen( widget->name ) );

  }else						/* カーソル表示ありなら     */
  {						/* マウス位置にカーソル移動 */
    q8tk_entry_set_position( widget,		
			     m_x -widget->x +widget->stat.entry.disp_pos );
  }
  set_construct_flag( TRUE );
}

/* ←→キーを押したら、カーソル移動               (編集可能時のみ) */
/* 文字キーを押したら、文字列編集し、シグナル発生 (編集可能時のみ) */
/* リターンを押したら、シグナル発生               (編集可能時のみ) */
static	void	entry_event_key_on( Q8tkWidget *widget, int key )
{
  if( widget->stat.entry.editable == FALSE ) return;
  if( widget->stat.entry.cursor_pos < 0 ) return;

  switch( key ){
  case Q8TK_KEY_LEFT:
    if( widget->stat.entry.cursor_pos ){
      q8tk_entry_set_position( widget, widget->stat.entry.cursor_pos - 1 );
    }
    break;

  case Q8TK_KEY_RIGHT:
    {
      int w = 1;
      if( q8gr_strchk( widget->code, widget->name, 
		       widget->stat.entry.cursor_pos )==1 ){ w = 2; }
      q8tk_entry_set_position( widget, widget->stat.entry.cursor_pos + w );
    }
    break;

  case Q8TK_KEY_RET:
    widget_signal_do( widget, "activate" );
    break;

  case Q8TK_KEY_BS:
    if( widget->stat.entry.cursor_pos ){
      int w = q8gr_strdel( widget->code, widget->name,
			   widget->stat.entry.cursor_pos - 1 );
      q8tk_entry_set_position( widget, widget->stat.entry.cursor_pos - w );
      widget_signal_do( widget, "changed" );
    }
    break;

  default:
    if( key <= 0xff && isprint(key) ){
      int len = (int)strlen( widget->name ) + 1;
      if( widget->stat.entry.max_length &&		/* 文字列上限サイズ */
	  widget->stat.entry.max_length < len ){	/* を超えたら無視   */
	;
      }else{
	q8tk_entry_malloc( widget, len );	/* メモリが足りないなら確保 */

	memmove( &widget->name[ widget->stat.entry.cursor_pos+1 ],
		 &widget->name[ widget->stat.entry.cursor_pos ],
		 strlen( &widget->name[ widget->stat.entry.cursor_pos ])+1 );
	widget->name[ widget->stat.entry.cursor_pos ] = key;
	q8tk_entry_set_position( widget, widget->stat.entry.cursor_pos + 1 );
	widget_signal_do( widget, "changed" );
      }
    }
    break;
  }
}

Q8tkWidget	*q8tk_entry_new( void )
{
  return	q8tk_entry_new_with_max_length( 0 );
}

Q8tkWidget	*q8tk_entry_new_with_max_length( int max )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_ENTRY;

  q8tk_entry_malloc( w, max );

  w->name[0] = '\0';
  w->code = kanji_code;

  w->stat.entry.max_length    = max;
/*w->stat.entry.malloc_length = q8tk_entry_malloc()にて設定済み */
  w->stat.entry.disp_pos      = 0;
  w->stat.entry.cursor_pos    = 0;
  w->stat.entry.width         = 8;
  w->stat.entry.editable      = TRUE;

  w->event_button_on  = entry_event_button_on;
  w->event_key_on     = entry_event_key_on;

  return	w;
}

const	char	*q8tk_entry_get_text( Q8tkWidget *entry )
{
  return	entry->name;
}

void		q8tk_entry_set_text( Q8tkWidget *entry, const char *text )
{
  int len = strlen( text );

  if( entry->stat.entry.max_length == 0 ){	/* 上限なし */

    q8tk_entry_malloc( entry, len );	/* メモリが足りないなら、確保 */
    strcpy( entry->name, text );

  }else{					/* 上限あり */

    q8gr_strncpy( entry->code,
		  entry->name, text, entry->stat.entry.max_length );
    entry->name[ entry->stat.entry.max_length ] = '\0';

  }

  entry->stat.entry.disp_pos = 0;		/* 文字列先頭から表示 */
  q8tk_entry_set_position( entry, -1 );		/* カーソル非表示     */
}

void		q8tk_entry_set_position( Q8tkWidget *entry, int position )
{
  int disp_pos;
  int tail = q8gr_strlen( entry->code, entry->name );

  if( position < 0 ){
    entry->stat.entry.cursor_pos = -1;
    set_construct_flag( TRUE );
    return;
  }

  if( tail < position ) position = tail;
  if( q8gr_strchk( entry->code, entry->name, position )==2 ){
    position -= 1;
  }

	/* 表示範囲より前にカーソルセット */
  if( position < entry->stat.entry.disp_pos ){

    entry->stat.entry.disp_pos   = position;
    entry->stat.entry.cursor_pos = position;

  }	/* 表示範囲より後ろにカーソルセット */
  else if( entry->stat.entry.disp_pos + entry->stat.entry.width <= position ){

    disp_pos = position - entry->stat.entry.width + 1;
    if( q8gr_strchk( entry->code, entry->name, disp_pos )==2 ){
      disp_pos += 1;
    }
    entry->stat.entry.disp_pos   = disp_pos;
    entry->stat.entry.cursor_pos = position;

  }	/* 表示範囲の末尾 (全角文字の2バイト目)にカーソルセット */
  else if( (entry->stat.entry.disp_pos + entry->stat.entry.width - 1
								== position) &&
	    q8gr_strchk( entry->code, entry->name, position )==1 ){

    disp_pos = entry->stat.entry.disp_pos +1;
    if( q8gr_strchk( entry->code, entry->name, disp_pos )==2 ){
      disp_pos += 1;
    }
    entry->stat.entry.disp_pos   = disp_pos;
    entry->stat.entry.cursor_pos = position;
    
  }	/* それ以外 (表示範囲内にカーソルセット) */
  else{

    entry->stat.entry.cursor_pos = position;

  }
  q8gr_set_cursor_blink();
  set_construct_flag( TRUE );
}

void		q8tk_entry_set_max_length( Q8tkWidget *entry, int max )
{
  q8tk_entry_malloc( entry, max );	/* メモリが足りないなら、確保 */

  entry->stat.entry.max_length = max;
}

void   		q8tk_entry_set_editable( Q8tkWidget *entry, int editable )
{
  if( entry->stat.entry.editable != editable ){
    entry->stat.entry.editable = editable;
    set_construct_flag( TRUE );
  }
}





/*---------------------------------------------------------------------------
 * アクセラレータグループ (ACCEL GROUP)
 * アクセラレーターキー   (ACCEL KEY)
 *---------------------------------------------------------------------------
 *  ・子は持てないし、表示もできない。
 *  ・内部的には、アクセラレータグループが親、アクセラレータキーが子の
 *    形態をとる
 *  ・アクセラレータグループをウインドウに関連づけることで、アクセラレータ
 *    キーをそのウインドウにて有効にすることができる。
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_accel_group_new( void )
 *	アクセラレータグループの生成。
 *
 *  void	q8tk_accel_group_attach( Q8tkWidget *accel_group, 
 *						Q8tkWidget *window )
 *	アクセラレータグループを、ウインドウ window に関連づける。
 *
 *  void	q8tk_accel_group_detach( Q8tkWidget *accel_group, 
 *						Q8tkWidget *window )
 *	ウインドウ window に関連づけたアクセラレータグループを切り離す。
 *
 *  void	q8tk_accel_group_add( Q8tkWidget *accel_group, int accel_key,
 *				      Q8tkWidget *widget, const char *signal )
 *	アクセラレータキーを設定する。
 *	アクセラレータグループ accel_group に、キー accel_key を設定する。
 *	このキーが押下されたら、 ウィジット widget に シグナル signal を送る。
 *  -------------------------------------------------------------------------
 *	[WINDOW]
 *	   ｜
 *	   └──【ACCEL GROUP】←→【ACCEL KEY】
 *					↑↓
 *				←  【ACCEL KEY】
 *					↑↓
 *				←  【ACCEL KEY】
 *					↑↓
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_accel_group_new( void )
{
  Q8tkWidget	*w;

  w = malloc_widget();
  w->type   = Q8TK_TYPE_ACCEL_GROUP;

  return w;
}

void	q8tk_accel_group_attach( Q8tkWidget *accel_group, Q8tkWidget *window )
{
  Q8tkAssert(window->type==Q8TK_TYPE_WINDOW,NULL);
  window->stat.window.accel = accel_group;
}

void	q8tk_accel_group_detach( Q8tkWidget *accel_group, Q8tkWidget *window )
{
  Q8tkAssert(window->type==Q8TK_TYPE_WINDOW,NULL);

  if( window->stat.window.accel == accel_group ){
    window->stat.window.accel = NULL;
  }
}

void	q8tk_accel_group_add( Q8tkWidget *accel_group, int accel_key,
			      Q8tkWidget *widget,      const char *signal )
{
  Q8tkWidget	*w;
  w = malloc_widget();

  if( accel_group->child ){
    Q8tkWidget *c = accel_group->child;
    while( c->next ){
      c = c->next;
    }
    c->next      = w;
    w->prev = c;
    w->next = NULL;
  }else{
    accel_group->child = w;
    w->prev     = NULL;
    w->next     = NULL;
  }
  w->parent = accel_group;

  w->type  = Q8TK_TYPE_ACCEL_KEY;
  w->name  = (char *)malloc( strlen(signal)+1 );
  Q8tkAssert(w->name,"memory exhoused");
  CHECK_MALLOC_OK( w->name, "memory" );
  strcpy( w->name, signal );
  w->code = Q8TK_KANJI_ANK;

  w->stat.accel.key    = accel_key;
  w->stat.accel.widget = widget;
}




/*---------------------------------------------------------------------------
 * ダイアログ (DIALOG)
 *---------------------------------------------------------------------------
 *  ・ダイアログウインドウを開く。
 *		+-------------------+
 *		| WARING !          |↑
 *		| Quit, really?     |この部分は、vbox (垂直ボックス)
 *		| press button.     |↓
 *		| +------+ +------+ |
 *		| |  OK  | |CANCEL| |
 *		| +------+ +------+ |
 *		+-------------------+
 *		  ← この部分は  →
 *		     action_area (水平ボックス)
 *  ・ダイアログを生成した後は、 vbox と action_area に任意のウィジットを
 *    設定できる。q8tk_box_pack_start() / end() にて設定する。
 *  ・このウインドウはモーダレスなので注意。
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_dialog_new( void )
 *	ダイアログの生成。
 *
 *	ここで生成したダイアログの vbox にウィジットを追加する場合の例。
 *		d = q8tk_dialog_new();
 *		q8tk_box_pack_start( Q8TK_DIALOGE(d)->vbox, label );
 *
 *	ここで生成したダイアログの action_area にウィジットを追加する場合の例。
 *		d = q8tk_dialog_new();
 *		q8tk_box_pack_start( Q8TK_DIALOGE(d)->action_area, button );
 *  -------------------------------------------------------------------------
 *	    ：……→【DIALOG】
 *	    ↓
 *	【WINDOW】←→【FRAME】←→【VBOX】←→【VBOX(vbox)】
 *			 	                    ↑↓
 *				           ←  【HBOX(action_area)】
 *
 *	q8tk_dialog_new()の返り値は 【WINDOW】 のウィジットである。
 *	モーダルにする場合は、このウィジットを q8tk_grab_add() する。
 *
 *	vbox、action_area はユーザが任意にウィジットを追加できる。
 *
 *	q8tk_widget_destroy() の際は、この全てのウィジットが削除されるが、
 *	ユーザが vbox、action_area に追加したウィジットは削除されない。
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_dialog_new( void )
{
  Q8tkWidget	*dialog, *window, *vbox, *frame;

  dialog = malloc_widget();
  dialog->type = Q8TK_TYPE_DIALOG;

  window = q8tk_window_new( Q8TK_WINDOW_DIALOG );
  window->stat.window.work = dialog;

  frame = q8tk_frame_new("");
  q8tk_container_add( window, frame );
  q8tk_widget_show( frame );
  q8tk_frame_set_shadow_type( frame, Q8TK_SHADOW_NONE );

  vbox = q8tk_vbox_new();
  q8tk_container_add( frame, vbox );
  q8tk_widget_show( vbox );

  dialog->stat.dialog.vbox = q8tk_vbox_new();
  q8tk_box_pack_start( vbox, dialog->stat.dialog.vbox );
  q8tk_widget_show( dialog->stat.dialog.vbox );

  dialog->stat.dialog.action_area = q8tk_hbox_new();
  q8tk_box_pack_start( vbox, dialog->stat.dialog.action_area );
  q8tk_widget_show( dialog->stat.dialog.action_area );

  return window;
}




/*---------------------------------------------------------------------------
 * FILE SELECTION
 *---------------------------------------------------------------------------
 *  ・ウィジットを組み合わせて、ファイルセレクションを作り出す。
 *  ・扱えるファイル名の長さは、パスを含めて最大で
 *    Q8TK_MAX_FILENAME までである。(文字列終端の\0を含む)
 *  ・ファイル名の一覧は、システム依存関数にて処理する。
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_file_selection_new( const char *title, int select_ro )
 *	ファイルセレクションの生成。
 *	見出し表示用の文字列 title を指定する。
 *	select_ro が 1 なら、「Read Only」チェックボタンが表示され、チェック
 *	される。0 なら、「Read Only」チェックボタンが表示されるが、チェックは
 *	されない。-1 なら、「「Read Only」チェックボタンは表示されない。
 *	( チェックされたかどうかは、 q8tk_file_selection_get_readonly にて
 *	  取得できるので、呼び出し元でリードオンリーで開くかどうかを決定する )
 *
 *  const char	*q8tk_file_selection_get_filename( Q8tkWidget *fselect )
 *	選択(入力)されているファイル名を取得する。
 *
 *  void	q8tk_file_selection_set_filename( Q8tkWidget *fselect,
 *						  const char *filename )
 *	ファイル名 (ディレクトリ名でも可) を指定する。
 *
 *  int		q8tk_file_selection_get_readonly( Q8tkWidget *fselect )
 *	真 なら、「リードオンリーで開く」がチェックされている。
 *  -------------------------------------------------------------------------
 *	    ：……→【FILE SELECTION】
 *	    ↓
 *	【WINDOW】←→【VBOX】←→【LABEL】	q8tk_file_selection_new()の引数
 *				    ↑↓
 *			      ←  【HSEPARATOR】
 *				    ↑↓
 *			      ←  【LABEL】	ディレクトリ名
 *				    ↑↓
 *			      ←  【LABEL】	ファイル数
 *				    ↑↓
 *			      ←  【SCROLLED WINDOW】←→【LIST BOX】←→(A)
 *				    ↑｜    ｜｜
 *				    ｜｜    ｜└─  【ADJUSTMENT】
 *				    ｜｜    └──  【ADJUSTMENT】
 *				    ｜↓
 *			      ←  【LABEL】	filename: の見出し
 *				    ↑↓
 *			      ←  【ENTRY】	ファイル名入力
 *				    ↑↓
 *			      ←  【HBOX】←→【CHECK BUTTON】←→【LABEL】
 *						  ↑｜
 *						  ｜↓
 *					  ←  【BUTTON】←→【LABEL】
 *						  ↑｜	(ok_button)
 *						  ｜↓
 *					  ←  【BUTTON】←→【LABEL】
 *							(cancel_button)
 *
 *	q8tk_file_selection_new() の返り値は 【WINDOW】 のウィジットである。
 *	モーダルにする場合は、このウィジットを q8tk_grab_add() する。
 *
 *	cancel_button、ok_button はユーザが任意にシグナルを設定できる。
 *
 *		fsel = q8tk_file_selection_new();
 *		q8tk_widget_show( fsel );
 *		q8tk_grab_add( fsel );
 *		q8tk_signal_connect( Q8TK_FILE_SELECTION(fsel)->ok_button,
 *		 		     "clicked", ユーザ関数、ユーザ引数 );
 *		q8tk_signal_connect( Q8TK_FILE_SELECTION(fsel)->cancel_button,
 *		 		     "clicked", ユーザ関数、ユーザ引数 );
 *
 *	---------------------------------------------------------------------
 *		(A)  ←→【LIST ITEM】←→【LABEL】
 *		     	   ↑↓
 *		     ←  【LIST ITEM】←→【LABEL】
 *		     	   ↑↓
 *		     ←  【LIST ITEM】←→【LABEL】
 *		     	   ↑↓
 *
 *	q8tk_file_selection_set_filename() にてディレクトリ名をセットすると、
 *	そのディレクトリのファイル一覧を取得し、(この処理はシステム依存)、
 *	全てのファイル名をリストアイテムにして、リストボックスに乗せる。
 *	ファイル名をセットした場合も、そこからディレクトリ名を切り出して
 *	(ここの処理もシステム依存)、同様にディレクトリのファイル一覧を生成
 *	する。この場合、エントリにもファイル名をセットする。
 *
 *	---------------------------------------------------------------------
 *	以下の場合、ok_button に "clicked" シグナルが発生する。
 *		・OK のボタンをクリックした場合
 *		・ファイル名一覧のリストアイテムの、アクティブなのをクリックし
 *		  それがディレクトリでなかった場合。
 *		・エントリにて入力してリターンキーを押下し、
 *		  それがディレクトリでなかった場合。
 *		
 *	つまり、ok_button に "clicked" シグナルを設定しておけば、
 *	一般的な操作によるファイル選択(入力)を検知できる。
 *
 *	---------------------------------------------------------------------
 *	q8tk_widget_destroy() の際は、すべてのウィジット( 18個の
 *	ウィジット 及び 不定数の LIST ITEM ウイジット)を破壊する。
 *
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 *	以下の関数の引数 fselect は q8tk_file_selection_new() の戻り値の、
 *	ウインドウウィジットである。ファイルセレクションウィジットの変数に
 *	アクセスするには、 Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->変数名
 *	とすること。
 *---------------------------------------------------------------------------*/

static	char	file_selection_pathname[ Q8TK_MAX_FILENAME ];
static	char	file_selection_filename[ Q8TK_MAX_FILENAME ];

static	int	file_selection_width;

/* ファイルセレクションのリストボックス内のリストアイテムがクリックされた場合

   ・直前に選択していたリストボックスと違うのがクリックされた場合
	fsel_list_selection_changed() ("selection_changed"イベントによる)
	fsel_item_selected()          ("select"           イベントによる)
     の順に呼び出される。

   ・直前に選択していたリストボックスと同じのがクリックされた場合
	fsel_item_selected()          ("select"           イベントによる)
     だけが呼び出される。 ("selection_changed" イベントは発生しないので)     */


static void fsel_update( Q8tkWidget *fselect, const char *filename, int type );
static int fsel_opendir( Q8tkWidget *fselect, const char *filename, int type );


/* ファイル名一覧 LIST BOX にて、選択中のファイルが変わった時のコールバック */
static void fsel_selection_changed_callback( Q8tkWidget *dummy, void *fselect )
{
  Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = TRUE;
}


/* ファイル名の LIST ITEM にて、 クリックした時のコールバック */
static void fsel_selected_callback( Q8tkWidget *item, void *fselect )
{
  const char *name;
  if     ( item->name )                       name = item->name;
  else if( item->child && item->child->name ) name = item->child->name;
  else                                        name = "";


  if( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed ){

    /* 直前に fsel_selection_changed_callback() が呼び出されている場合、
       つまり『前とは違う』リストアイテムがクリックされた場合は、
       そのリストアイテムの保持するファイル名を、エントリにセットする */

    Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = FALSE;

    q8tk_entry_set_text( Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)
				->selection_entry, name );

  }else{

    /* 今回 fsel_selected_callback() だけが呼び出された場合、
       つまり『前と同じ』リストアイテムがクリックされた場合は、
       そのリストアイテムの保持するファイル名で、ファイル一覧更新 */

    fsel_update( (Q8tkWidget*)fselect, name, item->stat.any.data[0] );
  }
}


/* ファイル名入力の ENTRY にて、リターンキー押下時のコールバック */
static void fsel_activate_callback( Q8tkWidget *entry, void *fselect )
{
  const char *input_filename = q8tk_entry_get_text(entry);
  int   type = -1;	/* ディレクトリかどうかは不明 */

#if 1
  /* 入力したファイル名が、リストの name に一致すれば、
     ファイルタイプを確定することができる。リストを手繰るか？ */

  Q8tkWidget *match_list;
  match_list = q8tk_listbox_search_items( 
			 Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->file_list,
			 input_filename );

  if( match_list ) type = match_list->stat.any.data[0];
#endif

  /* 入力したファイル名で、ファイル一覧更新 */
  fsel_update( (Q8tkWidget*)fselect, input_filename, type );
}


/* ファイル一覧 LIST BOX を更新する。 ディレクトリ名なら表示更新、
   ファイル名が確定した場合は、OK ボタンを押したことにする */
static void fsel_update( Q8tkWidget *fselect, const char *filename, int type )
{
  char	wk[ Q8TK_MAX_FILENAME ];

  if( osd_path_join( file_selection_pathname, filename, wk, Q8TK_MAX_FILENAME )
      == FALSE ){
    strcpy( wk, "" );
  }

  if( fsel_opendir( fselect, wk, type ) != FILE_STAT_DIR ){
    widget_signal_do( Q8TK_FILE_SELECTION(fselect)->ok_button, "clicked" );
  }
}


/* 指定されたパス名を、ディレクトリ名とファイル名に分け、 ディレクトリを開いて
   リストにファイル一覧をセット、 ファイル名をエントリにセットする */
static int fsel_opendir_sub( Q8tkWidget *fselect, const char *dirname );
static int fsel_opendir( Q8tkWidget *fselect, const char *filename, int type )
{
  int	save_code = q8tk_set_kanjicode( osd_kanji_code() );

  char	path[ Q8TK_MAX_FILENAME ];
  char	dir [ Q8TK_MAX_FILENAME ];
  char	file[ Q8TK_MAX_FILENAME ];


  if( filename==NULL || filename[0] == '\0' ){	/* filenameが無効な場合は */
    filename = osd_dir_cwd();			/* カレントディレクトリを */
    type = FILE_STAT_DIR;			/* 代わりに使うことにする */
  }

  if( osd_path_normalize( filename, path, Q8TK_MAX_FILENAME )	/* 正規化 */
      == FALSE ){
    strcpy( path, "" );
    type = FILE_STAT_DIR;
  }


		/* filename をディレクトリ名とファイル名に分離する */

  if( type < 0 ){				/* ディレクトリかどうか不明 */
    type = osd_file_stat( path );
  }

  if( type == FILE_STAT_DIR ){			    /* filenameはディレクトリ*/
    strcpy( dir,  path );
    strcpy( file, ""   );

  }else{					    /* filename は ファイル  */

    if( osd_path_split( path, dir, file, Q8TK_MAX_FILENAME ) == FALSE ){
      strcpy( dir,  ""   );
      strcpy( file, path );
    }
  }

		/* ディレクトリ名でディレクトリを開く */

  if( fsel_opendir_sub( fselect, dir ) >= 0 ){		/* 開くのに成功した */

    file_selection_pathname[0] = '\0';
    strncat( file_selection_pathname, dir, Q8TK_MAX_FILENAME-1 );

    q8tk_entry_set_text( Q8TK_FILE_SELECTION(fselect)->selection_entry, file );

  }else{						/* 開けなかった…… */

    q8tk_entry_set_text( Q8TK_FILE_SELECTION(fselect)->selection_entry, path );

  }


  q8tk_set_kanjicode( save_code );

		/* 指定されたパス名がディレクトリだったかどうかを返す */
  return type;
}


static	int	fsel_opendir_sub( Q8tkWidget *fselect, const char *dirname )
{
  T_DIR_INFO	*dirp;
  T_DIR_ENTRY	*dirent;
  char		wk[ Q8TK_MAX_FILENAME ];
  int		nr, i;

		/* 既存の LIST ITEM 削除 */

  if( Q8TK_FILE_SELECTION(fselect)->file_list->child ){
    q8tk_listbox_clear_items( Q8TK_FILE_SELECTION(fselect)->file_list, 0, -1 );
  }
  q8tk_adjustment_set_value( Q8TK_FILE_SELECTION(fselect)->scrolled_window
						    ->stat.scrolled.hadj, 0 );
  q8tk_adjustment_set_value( Q8TK_FILE_SELECTION(fselect)->scrolled_window
						    ->stat.scrolled.vadj, 0 );


		/* ディレクトリを調べ、ファイル名を LIST ITEM として登録 */
		/* アクセス可能なファイル名も併せて LIST ITEM に保持しておく */
		/* ついでなので、ファイルのタイプも LIST ITEM に保持しておく */

  if( dirname[0] != '\0' &&
      (dirp = osd_opendir( dirname )) ){

    nr = 0;
    while( (dirent = osd_readdir( dirp )) ){
      Q8tkWidget *item;

      wk[0] = '\0';
      strncat( wk, dirent->str, Q8TK_MAX_FILENAME-1 );

      item = q8tk_list_item_new_with_label( wk );
      q8tk_list_item_set_string( item, dirent->name );
      item->stat.any.data[0] = dirent->type;
      q8tk_container_add( Q8TK_FILE_SELECTION(fselect)->file_list, item );
      q8tk_widget_show( item );
      q8tk_signal_connect( item, "select", fsel_selected_callback, fselect );

      nr ++;
    }
    osd_closedir( dirp );


    i = strlen(dirname);
    if( i + 6 > file_selection_width ){		/* 6 == sizeof("DIR = ") */
      i = i - (file_selection_width - 6 -3);	/* 3 == sizeof("...")    */
      if( q8gr_strchk( Q8TK_FILE_SELECTION(fselect)->dir_name->code, 
		       dirname, i )==2 ){ i ++; }	/* 漢字の途中は避ける*/
    }else{
      i = 0;
    }
    wk[0] = '\0';
    strncat( wk, "DIR = ", Q8TK_MAX_FILENAME-1 );
    if( i ){
      strncat( wk, "...", Q8TK_MAX_FILENAME-1 -strlen(wk) );
    }
    strncat( wk, &dirname[i], Q8TK_MAX_FILENAME-1 -strlen(wk) );


  }else{

    nr = -1;

    wk[0] = '\0';
    strncat( wk, "DIR = non existant", Q8TK_MAX_FILENAME-1 );

  }

  q8tk_label_set( Q8TK_FILE_SELECTION(fselect)->dir_name, wk );

  sprintf( wk, "%4d file(s)", (nr<0) ? 0 : nr );
  q8tk_label_set( Q8TK_FILE_SELECTION(fselect)->nr_files, wk );


  return	nr;
}




#if 0	/* ver 0.5.x までの表示形式 */

Q8tkWidget	*q8tk_file_selection_new( const char *title, int select_ro )
{
  Q8tkWidget	*fselect, *window, *vbox, *wk;
  int	save_code;

  strcpy( file_selection_pathname, "" );
  strcpy( file_selection_filename, "" );


  fselect = malloc_widget();
  fselect->type = Q8TK_TYPE_FILE_SELECTION;

  window = q8tk_window_new( Q8TK_WINDOW_DIALOG );	/* ウインドウ生成 */
  window->stat.window.work = fselect;

  vbox = q8tk_vbox_new();				/* 以下 垂直に詰める */
  q8tk_container_add( window, vbox );
  q8tk_widget_show( vbox );

  wk = q8tk_label_new( title );				/* 見出し(引数より) */
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
  q8tk_misc_set_placement( wk, Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP );
  file_selection_width = MAX( q8gr_strlen( wk->code, wk->name ),
			      Q8GR_SCREEN_X * 0.5 );
  
  wk = q8tk_hseparator_new();				/* ---------------- */
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
							/* DIR = /...       */
  fselect->stat.fselect.dir_name = q8tk_label_new( "DIR =" );
  q8tk_box_pack_start( vbox, fselect->stat.fselect.dir_name );
  q8tk_widget_show( fselect->stat.fselect.dir_name );
  q8tk_misc_set_placement( fselect->stat.fselect.dir_name,
			   Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP );

							/*     99 file(s)   */
  fselect->stat.fselect.nr_files = q8tk_label_new( "0 file(s)" );
  q8tk_box_pack_start( vbox, fselect->stat.fselect.nr_files );
  q8tk_widget_show( fselect->stat.fselect.nr_files );
  q8tk_misc_set_placement( fselect->stat.fselect.nr_files,
			   Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP );

							/* [リストボックス] */
  fselect->stat.fselect.scrolled_window=q8tk_scrolled_window_new( NULL, NULL );
  q8tk_box_pack_start( vbox, fselect->stat.fselect.scrolled_window );
  q8tk_widget_show( fselect->stat.fselect.scrolled_window );

  fselect->stat.fselect.file_list = q8tk_listbox_new();
  q8tk_container_add( fselect->stat.fselect.scrolled_window,
		      fselect->stat.fselect.file_list );
  q8tk_widget_show( fselect->stat.fselect.file_list );
  q8tk_signal_connect( fselect->stat.fselect.file_list, "selection_changed",
		       fsel_selection_changed_callback, window );

  q8tk_misc_set_placement( fselect->stat.fselect.scrolled_window,
			   Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
  q8tk_scrolled_window_set_policy( fselect->stat.fselect.scrolled_window,
				   Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS );
  q8tk_misc_set_size( fselect->stat.fselect.scrolled_window, 40, 14 );
  q8tk_misc_set_size( fselect->stat.fselect.file_list, 40-3, 0 );

							/* File name:       */
  wk = q8tk_label_new( "File name:" );
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
  q8tk_misc_set_placement( wk, Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP );

							/* [エントリ部]     */
  save_code = q8tk_set_kanjicode( osd_kanji_code() );
  fselect->stat.fselect.selection_entry = q8tk_entry_new();
  q8tk_box_pack_start( vbox, fselect->stat.fselect.selection_entry );
  q8tk_widget_show( fselect->stat.fselect.selection_entry );
  q8tk_misc_set_size( fselect->stat.fselect.selection_entry,
		      file_selection_width, 0 );
  q8tk_signal_connect( fselect->stat.fselect.selection_entry, "activate",
		       fsel_activate_callback, window );
  q8tk_set_kanjicode( save_code );

  wk = q8tk_hbox_new();					/*     [OK][CANCEL] */
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
  q8tk_misc_set_placement( wk, Q8TK_PLACEMENT_X_RIGHT, Q8TK_PLACEMENT_Y_TOP );

  fselect->stat.fselect.ro_button =
				q8tk_check_button_new_with_label("Read only");
  q8tk_box_pack_start( wk, fselect->stat.fselect.ro_button );
  if( select_ro >= 0 ){
    if( select_ro > 0 ){
      q8tk_toggle_button_set_state( fselect->stat.fselect.ro_button, TRUE );
    }
    q8tk_widget_show( fselect->stat.fselect.ro_button );
  }
  q8tk_misc_set_placement( fselect->stat.fselect.ro_button,
			   Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER );

  fselect->stat.fselect.ok_button = q8tk_button_new_with_label("  OK  ");
  q8tk_box_pack_start( wk, fselect->stat.fselect.ok_button );
  q8tk_widget_show( fselect->stat.fselect.ok_button );

  fselect->stat.fselect.cancel_button = q8tk_button_new_with_label("CANCEL");
  q8tk_box_pack_start( wk, fselect->stat.fselect.cancel_button );
  q8tk_widget_show( fselect->stat.fselect.cancel_button );

  /*q8tk_file_selection_set_filename( window, file_selection_pathname );*/

  return window;
}

#else	/* ver 0.6.0 以降の表示形式 */

Q8tkWidget	*q8tk_file_selection_new( const char *title, int select_ro )
{
  Q8tkWidget	*fselect, *window, *vbox, *hbox, *vv, *wk;
  int	i, save_code;

  strcpy( file_selection_pathname, "" );
  strcpy( file_selection_filename, "" );


  fselect = malloc_widget();
  fselect->type = Q8TK_TYPE_FILE_SELECTION;

  window = q8tk_window_new( Q8TK_WINDOW_DIALOG );	/* ウインドウ生成 */
  window->stat.window.work = fselect;

  vbox = q8tk_vbox_new();				/* 以下 垂直に詰める */
  q8tk_container_add( window, vbox );
  q8tk_widget_show( vbox );

  wk = q8tk_label_new( title );				/* 見出し(引数より) */
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
  q8tk_misc_set_placement( wk, Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP );
  file_selection_width = MAX( q8gr_strlen( wk->code, wk->name ), 60 );
  
  wk = q8tk_hseparator_new();				/* ---------------- */
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );
							/* DIR = /...       */
  fselect->stat.fselect.dir_name = q8tk_label_new( "DIR =" );
  q8tk_box_pack_start( vbox, fselect->stat.fselect.dir_name );
  q8tk_widget_show( fselect->stat.fselect.dir_name );
  q8tk_misc_set_placement( fselect->stat.fselect.dir_name,
			   Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP );

  hbox = q8tk_hbox_new();
  q8tk_box_pack_start( vbox, hbox );
  q8tk_widget_show( hbox );
  {
    wk = q8tk_label_new( "    " );				/* すきま */
    q8tk_box_pack_start( hbox, wk );
    q8tk_widget_show( wk );

								/* [リスト] */
    fselect->stat.fselect.scrolled_window
				= q8tk_scrolled_window_new( NULL, NULL );
    q8tk_box_pack_start( hbox, fselect->stat.fselect.scrolled_window );
    q8tk_widget_show( fselect->stat.fselect.scrolled_window );

    fselect->stat.fselect.file_list = q8tk_listbox_new();
    q8tk_container_add( fselect->stat.fselect.scrolled_window,
			fselect->stat.fselect.file_list );
    q8tk_widget_show( fselect->stat.fselect.file_list );
    q8tk_signal_connect( fselect->stat.fselect.file_list, "selection_changed",
			 fsel_selection_changed_callback, window );

    q8tk_misc_set_placement( fselect->stat.fselect.scrolled_window,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
    q8tk_scrolled_window_set_policy( fselect->stat.fselect.scrolled_window,
				   Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS );
    q8tk_misc_set_size( fselect->stat.fselect.scrolled_window, 40, 18 );
    q8tk_misc_set_size( fselect->stat.fselect.file_list, 40-3, 0 );


    wk = q8tk_label_new( "  " );				/* すきま */
    q8tk_box_pack_start( hbox, wk );
    q8tk_widget_show( wk );


    vv = q8tk_vbox_new();
    q8tk_box_pack_start( hbox, vv );
    q8tk_widget_show( vv );
    {

      wk = q8tk_label_new( "" );				/* 空行 */
      q8tk_box_pack_start( vv, wk );
      q8tk_widget_show( wk );

								/* n file(s) */
      fselect->stat.fselect.nr_files = q8tk_label_new( "0000 file(s)" );
      q8tk_box_pack_start( vv, fselect->stat.fselect.nr_files );
      q8tk_widget_show( fselect->stat.fselect.nr_files );
      q8tk_misc_set_placement( fselect->stat.fselect.nr_files,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);

      for( i=0; i<9; i++ ){					/* 空行 */
	wk = q8tk_label_new( "" );
	q8tk_box_pack_start( vv, wk );
	q8tk_widget_show( wk );
      }

								/* Read only */
      fselect->stat.fselect.ro_button =
				q8tk_check_button_new_with_label("Read only");
      q8tk_box_pack_start( vv, fselect->stat.fselect.ro_button );
      q8tk_misc_set_placement( fselect->stat.fselect.ro_button,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
      if( select_ro >= 0 ){
	if( select_ro > 0 ){
	  q8tk_toggle_button_set_state( fselect->stat.fselect.ro_button, TRUE);
	}
	q8tk_widget_show( fselect->stat.fselect.ro_button );
      }

								/* [OK] */
      fselect->stat.fselect.ok_button = 
					q8tk_button_new_with_label( "  OK  " );
      q8tk_box_pack_start( vv, fselect->stat.fselect.ok_button );
      q8tk_widget_show( fselect->stat.fselect.ok_button );
      q8tk_misc_set_placement( fselect->stat.fselect.ok_button,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);

								/* [CANCEL] */
      fselect->stat.fselect.cancel_button =
					q8tk_button_new_with_label( "CANCEL" );
      q8tk_box_pack_start( vv, fselect->stat.fselect.cancel_button );
      q8tk_widget_show( fselect->stat.fselect.cancel_button );
      q8tk_misc_set_placement( fselect->stat.fselect.cancel_button,
			     Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
    }

//    wk = q8tk_label_new( " " );					/* すきま */
//    q8tk_box_pack_start( hbox, wk );
//    q8tk_widget_show( wk );
  }


  wk = q8tk_label_new( "" );				/* 空行 */
  q8tk_box_pack_start( vbox, wk );
  q8tk_widget_show( wk );

  hbox = q8tk_hbox_new();
  q8tk_box_pack_start( vbox, hbox );
  q8tk_widget_show( hbox );
  {
							/* File name:       */
    wk = q8tk_label_new( "Filename " );
    q8tk_box_pack_start( hbox, wk );
    q8tk_widget_show( wk );

							/* [エントリ部]     */
    save_code = q8tk_set_kanjicode( osd_kanji_code() );
    fselect->stat.fselect.selection_entry = q8tk_entry_new();
    q8tk_box_pack_start( hbox, fselect->stat.fselect.selection_entry );
    q8tk_widget_show( fselect->stat.fselect.selection_entry );
    q8tk_misc_set_size( fselect->stat.fselect.selection_entry, 50, 0 );
    q8tk_signal_connect( fselect->stat.fselect.selection_entry, "activate",
			 fsel_activate_callback, window );
    q8tk_set_kanjicode( save_code );
  }

  /*q8tk_file_selection_set_filename( window, file_selection_pathname );*/

  return window;
}

#endif

const	char	*q8tk_file_selection_get_filename( Q8tkWidget *fselect )
{
  if( osd_path_join( file_selection_pathname,
		     q8tk_entry_get_text(Q8TK_FILE_SELECTION(fselect)
							->selection_entry),
		     file_selection_filename,
		     Q8TK_MAX_FILENAME )
      == FALSE ){
    return "";
  }

  return file_selection_filename;
}

void		q8tk_file_selection_set_filename( Q8tkWidget *fselect,
						  const char *filename )
{
  fsel_opendir( fselect, filename, -1 );
}

int		q8tk_file_selection_get_readonly( Q8tkWidget *fselect )
{
  return Q8TK_TOGGLE_BUTTON( Q8TK_FILE_SELECTION(fselect)->ro_button )->active;
}







/************************************************************************/
/* 雑多な処理								*/
/************************************************************************/

/*---------------------------------------------------------------------------
 * 表示位置の変更
 *---------------------------------------------------------------------------
 *  ・HBOX、VBOX直下の子のウィジットのみ有効。他は無効 (無視)
 *  -------------------------------------------------------------------------
 *  void	q8tk_misc_set_placement( Q8tkWidget *widget,
 *					 int placement_x, int placement_y )
 *---------------------------------------------------------------------------*/
void	q8tk_misc_set_placement( Q8tkWidget *widget,
				 int placement_x, int placement_y )
{
  widget->placement_x = placement_x;
  widget->placement_y = placement_y;
}

/*---------------------------------------------------------------------------
 * 表示サイズの変更
 *---------------------------------------------------------------------------
 *  ・以下のウィジットのみ有効。他は無効 (無視)
 *	COMBO		… 文字列部分の表示幅
 *	LIST		… 文字列の表示幅
 *	SCROLLED WINDOW	… ウインドウの幅、高さ
 *	ENTRY		… 文字列の表示幅
 *  -------------------------------------------------------------------------
 *  void	q8tk_misc_set_size( Q8tkWidget *widget, int width, int height )
 *---------------------------------------------------------------------------*/
void	q8tk_misc_set_size( Q8tkWidget *widget, int width, int height )
{
  switch( widget->type ){

  case Q8TK_TYPE_COMBO:
    if( width > 0 ){
      widget->stat.combo.width = width;
      q8tk_misc_set_size( widget->stat.combo.entry,
			  widget->stat.combo.width, 0 );
    }else{
      widget->stat.combo.width = 0;
      q8tk_misc_set_size( widget->stat.combo.entry,
			  widget->stat.combo.length, 0 );
    }
    break;

  case Q8TK_TYPE_LISTBOX:
    if( width > 0 )  widget->stat.listbox.width  = width;
    else             widget->stat.listbox.width  = 0;
    break;

  case Q8TK_TYPE_SCROLLED_WINDOW:
    widget->stat.scrolled.width  = width;
    widget->stat.scrolled.height = height;
    break;

  case Q8TK_TYPE_ENTRY:
    widget->stat.entry.width = width;
    q8tk_entry_set_position( widget, widget->stat.entry.cursor_pos );
    break;

  default:
#ifdef	Q8TK_ASSERT_DEBUG
    fprintf( stderr, "Cant resize widget=%s\n", debug_type(widget->type));
#endif
    Q8tkAssert(FALSE,NULL);
    return;
  }
  set_construct_flag( TRUE );
}


/*---------------------------------------------------------------------------
 * 強制再描画
 *---------------------------------------------------------------------------
 *  ・再描画する
 *  -------------------------------------------------------------------------
 *  void	q8tk_misc_redraw( void )
 *---------------------------------------------------------------------------*/
void	q8tk_misc_redraw( void )
{
  menu_set_status();
  menu_trans_palette();

  q8gr_init();
  set_construct_flag( TRUE );
}




/************************************************************************/
/* コンテナ関係								*/
/************************************************************************/
/*---------------------------------------------------------------------------
 * コンテナ操作
 *---------------------------------------------------------------------------
 *  ・コンテナウィジットとウィジットを親子にする
 *  -------------------------------------------------------------------------
 *  void    q8tk_container_add( Q8tkWidget *container, Q8tkWidget *widget )
 *	コンテナウィジット container と ウィジット widget を親子にする。
 *
 *  void    q8tk_box_pack_start( Q8tkWidget *box, Q8tkWidget *widget )
 *	水平・垂直ボックス box の後方に、ウィジット widget を詰める。
 *
 *  void    q8tk_box_pack_end( Q8tkWidget *box, Q8tkWidget *widget )
 *	水平・垂直ボックス box の前方に、ウィジット widget を詰める。
 *
 *  void    q8tk_container_remove( Q8tkWidget *container, Q8tkWidget *widget )
 *	コンテナウィジット container から ウィジット widget を切り離す。
 *---------------------------------------------------------------------------*/
void	q8tk_container_add( Q8tkWidget *container, Q8tkWidget *widget )
{
  Q8tkAssert(container->attr&Q8TK_ATTR_CONTAINER,NULL);
  if( container->attr&Q8TK_ATTR_LABEL_CONTAINER ){
    Q8tkAssert(widget->type==Q8TK_TYPE_LABEL,NULL);
  }

  switch( container->type ){
  case Q8TK_TYPE_LISTBOX:			/* LIST BOX 例外処理 */
    if( container->child == NULL ){			/* 最初の LIST ITEM */
      container->stat.listbox.selected = widget;	/* の場合は、それを */
      container->stat.listbox.active   = widget;	/* 選択状態にする   */
    }
    q8tk_box_pack_start( container, widget );		/* 処理はVBOXと同じ */
    return;

  default:					/* 通常の処理 */
    container->child = widget;
    widget->parent   = container;
    widget->prev     = NULL;
    widget->next     = NULL;
    break;
  }

  if( widget->visible ){
    widget_map( widget );
  }
}

void	q8tk_box_pack_start( Q8tkWidget *box, Q8tkWidget *widget )
{
  Q8tkAssert(box->attr&Q8TK_ATTR_CONTAINER,NULL);

  if( box->child==NULL ){
    box->child     = widget;
    widget->prev   = NULL;
    widget->next   = NULL;
  }else{
    Q8tkWidget *c = box->child;
    while( c->next ){
      c = c->next;
    }
    c->next      = widget;
    widget->prev = c;
    widget->next = NULL;
  }
  widget->parent = box;

  if( widget->visible ){
    widget_map( widget );
  }
}

void	q8tk_box_pack_end( Q8tkWidget *box, Q8tkWidget *widget )
{
  Q8tkAssert(box->attr&Q8TK_ATTR_CONTAINER,NULL);

  if( box->child==NULL ){
    widget->prev   = NULL;
    widget->next   = NULL;
  }else{
    Q8tkWidget *c = box->child;
    Q8tkAssert(c->prev==NULL,NULL);
    c->prev      = widget;
    widget->next = c;
    widget->prev = NULL;
  }
  box->child     = widget;
  widget->parent = box;

  if( widget->visible ){
    widget_map( widget );
  }
}

void	q8tk_container_remove( Q8tkWidget *container, Q8tkWidget *widget )
{
  Q8tkAssert(container->attr&Q8TK_ATTR_CONTAINER,NULL);
  Q8tkAssert(widget->parent==container,NULL);

  if( widget->prev == NULL ){			/* 自分が親の直下の時 */
    Q8tkWidget *n = widget->next;
    if( n ) n->prev = NULL;
    container->child = n;

  }else{					/* そうじゃない時 */
    Q8tkWidget *p = widget->prev;
    Q8tkWidget *n = widget->next;
    if( n ) n->prev = p;
    p->next = n;
  }

  widget->parent = NULL;
  widget->prev = NULL;
  widget->next = NULL;

  switch( container->type ){

  case Q8TK_TYPE_LISTBOX:			/* LIST BOX 例外処理 */
    if( container->stat.listbox.selected == widget ){
      container->stat.listbox.selected = NULL;
      container->stat.listbox.active   = NULL;
    }
    break;

  }


  if( container->visible ){
    widget_map( container );
  }
}


/************************************************************************/
/* 表示関係								*/
/************************************************************************/
/*---------------------------------------------------------------------------
 * 可視化、不可視化
 *---------------------------------------------------------------------------
 *  ・ウィジットの表示・非表示
 *  -------------------------------------------------------------------------
 *  void	q8tk_widget_show( Q8tkWidget *widget )
 *	ウィジットを表示する。
 *
 *  void	q8tk_widget_hide( Q8tkWidget *widget )
 *	ウィジットを非表示にする。子も全て非表示になる。
 *---------------------------------------------------------------------------*/
void	q8tk_widget_show( Q8tkWidget *widget )
{
  widget->visible = TRUE;
  widget_map( widget );
}
void	q8tk_widget_hide( Q8tkWidget *widget )
{
  widget->visible = FALSE;
  widget_map( widget );
}


/************************************************************************/
/* ウィジットの消去							*/
/************************************************************************/
/*---------------------------------------------------------------------------
 * ウィジット消去
 *---------------------------------------------------------------------------
 *  ・ウィジットを消去する
 *	ダイアログ	     … q8tk_dialog_new にて生成したウィジット全て消去。
 *				vbox や action_area に載せたウィジットは消去
 *				しないのて、呼び出し元で消去する。
 *
 *	ファイルセレクション … 内部で生成した全てのウィジットを消去。
 *				ファイル選択途中に生成したウィジットも全て消去
 *
 *	その他		     … 内部で自動生成したウィジットもあわせて消去。
 *				q8tk_xxx_new_with_label で生成したラベルや、
 *				q8tk_scrolled_window_new で自動生成した
 *				アジャストメントなど
 *  -------------------------------------------------------------------------
 *  void	q8tk_widget_destroy( Q8tkWidget *widget )
 *	ウィジットの消去。以降、引数の widget は使用禁止
 *
 *---------------------------------------------------------------------------*/
static	void	widget_destroy_all( Q8tkWidget *widget );

void	q8tk_widget_destroy( Q8tkWidget *widget )
{
  Q8tkWidget *work;

  if( widget->type == Q8TK_TYPE_WINDOW &&	/* DIALOG と FILE SELECTION */
      (work = widget->stat.window.work) ){	/* の例外処理		    */

    switch( work->type ){
    case Q8TK_TYPE_DIALOG:
      Q8tkAssert(widget->child->type==Q8TK_TYPE_FRAME,NULL);
      Q8tkAssert(widget->child->child->type==Q8TK_TYPE_VBOX,NULL);
      q8tk_widget_destroy( Q8TK_DIALOG(widget)->action_area );
      q8tk_widget_destroy( Q8TK_DIALOG(widget)->vbox );
      q8tk_widget_destroy( widget->child->child );
      q8tk_widget_destroy( widget->child );
      q8tk_widget_destroy( work );
      break;

    case Q8TK_TYPE_FILE_SELECTION:
      {
	q8tk_widget_destroy( (Q8TK_FILE_SELECTION(widget)->scrolled_window)
							->stat.scrolled.hadj );
	q8tk_widget_destroy( (Q8TK_FILE_SELECTION(widget)->scrolled_window)
							->stat.scrolled.vadj );
	/* 関係がはっきりしているので、問答無用で全てを消去 */
	widget_destroy_all( widget->child );
      }
      q8tk_widget_destroy( work );
      break;
    }
  }

  if( widget->type == Q8TK_TYPE_COMBO ){	/* COMBO の例外処理 */
    Q8List     *l = widget->stat.combo.list;
    while( l ){
      q8tk_widget_destroy( (Q8tkWidget *)(widget->stat.combo.list->data) );
      l = l->next;
    }
    q8_list_free( widget->stat.combo.list );
    q8tk_widget_destroy( widget->stat.combo.entry );
  }

  if( widget->type == Q8TK_TYPE_ACCEL_GROUP &&	/* ACCEL の子は全て消去 */
      widget->child ){
    /* 関係がはっきりしているので、問答無用で全てを消去 */
    widget_destroy_all( widget->child );
  }


  if( widget->with_label &&			/* XXX_new_with_label()で */
      widget->child      &&			/* 生成された LABELの処理 */
      widget->child->type == Q8TK_TYPE_LABEL ){
    q8tk_widget_destroy( widget->child );
  }
  if( widget->with_label &&			/* 引数 NULL で生成された */
      widget->type==Q8TK_TYPE_SCROLLED_WINDOW ){/* SCROLLED WINDOW の処理 */
    q8tk_widget_destroy( widget->stat.scrolled.hadj );
    q8tk_widget_destroy( widget->stat.scrolled.vadj );
  }

  if( widget->name ){
    free( widget->name );
    widget->name = NULL;
  }
  free_widget( widget );
}


/* 自身と、子と、仲間を全て削除。
   自身以下のウィジットの削除にて例外処理が無いことがわかっている場合のみ可 */

static	void	widget_destroy_all( Q8tkWidget *widget )
{
  if( widget->next ) widget_destroy_all( widget->next );
  if( widget->child ) widget_destroy_all( widget->child );

  if( widget->name ){
    free( widget->name );
    widget->name = NULL;
  }
  free_widget( widget );
}




/************************************************************************/
/* 特定のウィジットにフォーカスを当てる。				*/
/*	通常 フォーカスは、直前にボタンやキー入力がなされたウィジット	*/
/*	になり、TAB により切替え可能だが、この関数で特定のウィジット	*/
/*	にフォーカスを設定することができる。				*/
/*	ただし、そのウィジットの一番先祖の WINDOW が、q8tk_grab_add()の	*/
/*	処理をなされたあとでなければ無効である。			*/
/************************************************************************/
void	q8tk_widget_grab_default( Q8tkWidget *widget )
{
  set_event_widget( widget );
  set_construct_flag( TRUE );
}


/************************************************************************/
/* シグナル関係								*/
/************************************************************************/
/*
 *	任意のウィジットに、任意のシグナルを送る
 */
static	void	widget_signal_do( Q8tkWidget *widget, const char *name )
{
  switch( widget->type ){
  case Q8TK_TYPE_WINDOW:			/* ウインドウ		*/
    if( strcmp( name, "inactivate" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;
  case Q8TK_TYPE_BUTTON:			/* ボタン		*/
    if( strcmp( name, "clicked" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;
  case Q8TK_TYPE_TOGGLE_BUTTON:			/* トグルボタン		*/
  case Q8TK_TYPE_CHECK_BUTTON:			/* チェックボタン	*/
  case Q8TK_TYPE_RADIO_BUTTON:			/* ラジオボタン		*/
    if      ( strcmp( name, "clicked" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }else if( strcmp( name, "toggled" )==0 ){
      if( widget->user_event_1 ){
	(*widget->user_event_1)( widget, widget->user_event_1_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_NOTEBOOK:			/* ノートブック		*/
    if( strcmp( name, "switch_page" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_COMBO:				/* コンボボックス	*/
    if( strcmp( name, "changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_LISTBOX:			/* リストボックス	*/
    if( strcmp( name, "selection_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
    if( strcmp( name, "select" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_ADJUSTMENT:			/* アジャストメント */
    if( strcmp( name, "value_changed" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }
    break;

  case Q8TK_TYPE_ENTRY:				/* エントリー		*/
    if      ( strcmp( name, "activate" )==0 ){
      if( widget->user_event_0 ){
	(*widget->user_event_0)( widget, widget->user_event_0_parm );
      }
      return;
    }else if( strcmp( name, "changed" )==0 ){
      if( widget->user_event_1 ){
	(*widget->user_event_1)( widget, widget->user_event_1_parm );
      }
      return;
    }
    break;

  }

#ifdef	Q8TK_ASSERT_DEBUG
  fprintf( stderr, "BAD signal %s '%s'\n", debug_type(widget->type),name);
#endif
  Q8tkAssert(FALSE,NULL);
  return;
}


/*---------------------------------------------------------------------------
 * シグナルの設定・削除
 *---------------------------------------------------------------------------
 *  ・ウィジットのシグナルに、コールバック関数を設定する。
 *    各ウィジットごとに発生するシグナルが決まっているので、そのシグナルが
 *    発生した時に呼び出される関数を設定する。
 *  ・ここで設定する関数内部で自分自身のウィジットを削除するようなことは
 *    してはいけない。内部で誤動作する可能性がある。
 *     (ただし、ボタンウィジットに限り、可能とする。なので、ダイアログや
 *	ファイルセレクションのボタンにて、自身を削除するのは大丈夫なはず)
 *
 *  -------------------------------------------------------------------------
 * int	q8tk_signal_connect( Q8tkWidget *widget, const char *name,
 *			     Q8tkSignalFunc func, void *func_data )
 *	widget に name で示されるシグナルに対するコールバック関数 func を
 *	設定する。シグナル発生時には、関数 func が 引数 widget, func_data を
 *	伴って呼び出される。
 *	なお、戻り値は常に 0 になる。
 *
 * void	q8tk_signal_handlers_destroy( Q8tkWidget *widget )
 *	widget に設定された全てのシグナルのコールバック関数を無効にする。
 *
 *---------------------------------------------------------------------------*/
void	q8tk_signal_handlers_destroy( Q8tkWidget *widget )
{
  widget->user_event_0 = NULL;
  widget->user_event_1 = NULL;
}

int	q8tk_signal_connect( Q8tkWidget *widget, const char *name,
			     Q8tkSignalFunc func, void *func_data )
{
  switch( widget->type ){
  case Q8TK_TYPE_WINDOW:			/* ウインドウ		*/
    if( strcmp( name, "inactivate" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;
  case Q8TK_TYPE_BUTTON:			/* ボタン		*/
    if( strcmp( name, "clicked" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;
  case Q8TK_TYPE_TOGGLE_BUTTON:			/* トグルボタン		*/
  case Q8TK_TYPE_CHECK_BUTTON:			/* チェックボタン	*/
  case Q8TK_TYPE_RADIO_BUTTON:			/* ラジオボタン		*/
    if      ( strcmp( name, "clicked" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }else if( strcmp( name, "toggled" )==0 ){
      widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_1_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_NOTEBOOK:			/* ノートブック		*/
    if( strcmp( name, "switch_page" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_COMBO:				/* コンボボックス	*/
    if( strcmp( name, "changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_LISTBOX:			/* リストボックス	*/
    if( strcmp( name, "selection_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
    if( strcmp( name, "select" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_ADJUSTMENT:			/* アジャストメント	*/
    if( strcmp( name, "value_changed" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }
    break;

  case Q8TK_TYPE_ENTRY:				/* エントリ		*/
    if      ( strcmp( name, "activate" )==0 ){
      widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_0_parm = func_data;
      return 0;
    }else if( strcmp( name, "changed" )==0 ){
      widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
      widget->user_event_1_parm = func_data;
      return 0;
    }
    break;

  }

#ifdef	Q8TK_ASSERT_DEBUG
  fprintf( stderr, "Undefined signal %s '%s'\n",debug_type(widget->type),name);
#endif
  Q8tkAssert(FALSE,NULL);
  return 0;
}



/************************************************************************/
/* イベントのチェック							*/
/************************************************************************/
/*
 *	DRAGGING
 */
static	void	widget_dragging( Q8tkWidget *focus )
{
  if( focus && focus->event_dragging ){
    (*focus->event_dragging)( focus );
  }
}
/*
 *	DRAG OFF
 */
static	void	widget_drag_off( Q8tkWidget *focus )
{
  if( focus && focus->event_drag_off ){
    (*focus->event_drag_off)( focus );
  }
}

/*
 *	MOUSE BUTTON ON
 */
static	void	widget_button_on( Q8tkWidget *focus )
{
  if( focus==NULL ){			  	/* ウインドウ外をクリック   */
    focus = window_level[ window_level_now ];	/* ウインドウにイベント通知 */
  }
  if( focus && focus->event_button_on ){
    (*focus->event_button_on)( focus );
  }
}
/*
 *	KEY ON
 */
static	void	widget_key_on( Q8tkWidget *focus, int key )
{
  if( focus && focus->event_key_on ){
    (*focus->event_key_on)( focus, key );
  }
}



/************************************************************************/
/* メイン								*/
/************************************************************************/
/*
 *	強制的に、即座に再描画
 *		ボタンが凹んだ時など、一瞬だけ表示する場合に呼び出す
 */
static	void	widget_redraw_now( void )
{
  if( get_main_loop_flag() ){

    widget_construct();
    menu_draw_screen();

#ifdef	RESET_EVENT				/* 先行入力をキャンセル */
    event_init();
#endif

    {
      int	i;
      for( i=0; i<6; i++ ){	/* 約 1/10秒間、表示を保持 */
	wait_menu();
      }
    }
  }
}


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_event_key_on( int code )
{
  now_mouse_on = FALSE;

  if( code == Q8TK_KEY_SHIFT ) now_shift_on = TRUE;

  if( !get_drag_widget() ){			/* ドラッグ中じゃない	*/
    if( code==Q8TK_KEY_TAB ){			/*	[TAB]キー */
      if( now_shift_on == FALSE ){				/* 単独 */
	if( get_event_widget()==NULL ){
	  if( q8tk_tab_top_widget )
	    set_event_widget( q8tk_tab_top_widget );
	}else{
	  set_event_widget( get_event_widget()->tab_next );
	}
      }else{	/* thanks! floi */				/* シフト */
	if( get_event_widget()==NULL ){
	  if( q8tk_tab_top_widget )
	    set_event_widget( q8tk_tab_top_widget->tab_prev );
	}else{
	  set_event_widget( get_event_widget()->tab_prev );
	}
      }
      {
	Q8tkWidget *w = get_event_widget();
	if( w && w->type != Q8TK_TYPE_ADJUSTMENT	/* スクロール*/
	      && w->type != Q8TK_TYPE_LISTBOX		/* 補正が    */
	      && w->type != Q8TK_TYPE_DIALOG		/* なされない*/
	      && w->type != Q8TK_TYPE_FILE_SELECTION ){	/* ウィジット*/
	  set_scroll_adj_widget( w );
	}
      }
      set_construct_flag( TRUE );
    }else{						/*	他のキー */

      int grab_flag = FALSE;
      if( window_level_now >= 0 ){				/* アクセラ */
	Q8tkWidget *w = window_level[ window_level_now ];	/* レーター */
	Q8tkAssert(w->type==Q8TK_TYPE_WINDOW,NULL);		/* キーが、 */
	if( w->stat.window.accel ){				/* 設定され */
	  w = (w->stat.window.accel)->child;			/* ていれば */
	  while( w ){						/* 処理する */
	    if( w->stat.accel.key == code ){
	      widget_signal_do( w->stat.accel.widget, w->name );
	      break;
	    }
	    w = w->next;
	  }
	}
      }
      if( grab_flag == FALSE ){
	widget_key_on( get_event_widget(), code );
      }
    }
  }
}

void	q8tk_event_key_off( int code )
{
  now_mouse_on = FALSE;

  if( code == Q8TK_KEY_SHIFT ) now_shift_on = FALSE;
}

void	q8tk_event_mouse_on( int code )
{
  if( get_construct_flag() ) return;	/* ウインドウ破棄直後のクリック回避 */

  if( code==Q8TK_BUTTON_L ){			/* 左ボタン ON		*/
    if( !get_drag_widget() ){			/* ドラッグ中じゃない	*/
      Q8tkWidget	*focus;
      focus = (Q8tkWidget *)q8gr_get_focus_screen( mouse.x/8, mouse.y/16 );
      if( focus ){
	set_event_widget( focus );
	widget_button_on( get_event_widget() );
      }else{
	widget_button_on( NULL );
      }
    }
  }else
  if( code==Q8TK_BUTTON_U ){			/* ホイール UP		*/
    widget_key_on( get_event_widget(), Q8TK_KEY_PAGE_UP );
  }else
  if( code==Q8TK_BUTTON_D ){			/* ホイール DOWN	*/
    widget_key_on( get_event_widget(), Q8TK_KEY_PAGE_DOWN );
  }
}

void	q8tk_event_mouse_off( int code )
{
  now_mouse_on = FALSE;

  if( code==Q8TK_BUTTON_L ){			/* 左ボタン OFF		*/
    if( get_drag_widget() ){			/* 只今 ドラッグ中	*/
      widget_drag_off( get_drag_widget() );
      set_drag_widget( NULL );
    }
  }
}

void	q8tk_event_mouse_moved( int x, int y )
{
  int block_moved;

  mouse.x_old = mouse.x;
  mouse.y_old = mouse.y;
  mouse.x     = x;
  mouse.y     = y;

  if( mouse.x/8  != mouse.x_old/8  ||		/* マウス 8dot以上 移動	*/
      mouse.y/16 != mouse.y_old/16 ){
    block_moved = TRUE;
    now_mouse_on = FALSE;

    if( disp_cursor ){
      set_construct_flag( TRUE );
    }
  }else{
    block_moved = FALSE;
  }

  if( get_drag_widget() ){			/* 只今 ドラッグ中 かつ	*/
    if( block_moved ){				/* マウス 8dot以上 移動	*/
      widget_dragging( get_drag_widget() );
    }
  }
}

void	q8tk_event_quit( void )
{
  q8tk_main_quit();
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
static	int	cursor_exist = 0;
static	int	cursor_timer = 0;

#define	Q8GR_CURSOR_BLINK		(20)
#define	Q8GR_CURSOR_CYCLE		(40)

#define	Q8GR_MOUSE_AUTO_REPEAT		(6)


void	q8gr_set_cursor_exist( int exist_flag )
{
  cursor_exist = exist_flag;
}
int	q8gr_get_cursor_exist( void )
{
  return	cursor_exist;
}
int	q8gr_get_cursor_blink( void )
{
  return (cursor_timer < Q8GR_CURSOR_BLINK)  ? TRUE : FALSE;
}
void	q8gr_set_cursor_blink( void )
{
  cursor_timer = 0;
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/

void	q8tk_main( void )
{
  int	mouse_frame = 0;


  widget_construct();			/* 最初の画面を表示 */
  set_construct_flag( FALSE );

  menu_trans_palette();
  menu_draw_screen();


  set_main_loop_flag( TRUE  );		/* 以降、無限ループで処理繰り返し */

  while( get_main_loop_flag() ){


    if( cursor_exist ){			/* カーソル表示中なら点滅タイマ更新 */
      cursor_timer ++;
      if( cursor_timer > Q8GR_CURSOR_CYCLE ){
	cursor_timer = 0;
      }
    }else{
      cursor_timer = 0;
    }


    event_handle();			/* イベントを処理する		  */
					/* (必要に応じて、内部で画面構築) */


    if( now_mouse_on ){			/* マウスボタンオートリピート処理 */
      ++ mouse_frame;
      if( mouse_frame >= Q8GR_MOUSE_AUTO_REPEAT ){
	q8tk_event_mouse_on( Q8TK_BUTTON_L );
	mouse_frame = 0;
      }
    }else{
      mouse_frame = 0;
    }


    if( cursor_exist ){			/* カーソル点滅切り替わりの	*/
      if( cursor_timer == 0 || 		/* タイミングをチェック		*/
	  cursor_timer == Q8GR_CURSOR_BLINK ){
	set_construct_flag( TRUE );
      }
    }


    if( get_construct_flag() ){		/* 画面構成変更時は、描画 */

      widget_construct();
      set_construct_flag( FALSE );

      menu_draw_screen();

#ifdef	RESET_EVENT			/* 先行入力をキャンセル */
      event_init();
#endif
    }

    wait_menu();			/* 約 1/60sec 間、sleep して待つ */
  }
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_main_quit( void )
{
  set_main_loop_flag( FALSE );
}







/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/*------------------------------------------------------*/
/* ウィジットの表示設定
	親ウィジットをつぎつぎチェックし、
	・全ての親ウィジットが可視状態
	・先祖ウィジットが WINDOW
	の場合、その WINDOW 以下を再計算して表示する
	ように、フラグをたてる。
	実際の再計算、表示は、q8tk_main() で行なう。	*/
/*------------------------------------------------------*/
static	void	widget_map( Q8tkWidget *widget )
{
  Q8tkWidget *ancestor, *parent;
  int	     size_calc = TRUE;

  if( widget->visible ){
    ancestor  = widget;
    parent    = widget->parent;
    while( parent ){
      if( parent->visible ){
	ancestor  = parent;
	parent    = parent->parent;
      }else{
	size_calc = FALSE;
	break;
      }
    }
    if( size_calc && ancestor->type == Q8TK_TYPE_WINDOW ){
      set_construct_flag( TRUE );
    }
  }
}


/*------------------------------------------------------*/
/* 子ウィジットのサイズを元に、自分自身のサイズを計算する。
   再帰的に、全ての子ウィジェットも同時に計算する。

   *widget … 一番親のウィジット
   *max_sx … 全ての子ウィジットのなかで最大サイズ x
   *max_sy … 全ての子ウィジットのなかで最大サイズ y
   *sum_sx … 子の仲間ウィジットのサイズ総和 x
   *sum_sy … 子の仲間ウィジットのサイズ総和 y          */
/*------------------------------------------------------*/
static	void	widget_resize( Q8tkWidget *widget, int max_sx, int max_sy );
static	void	widget_size( Q8tkWidget *widget, int *max_sx, int *max_sy,
						 int *sum_sx, int *sum_sy )
{
  int	n_msx, n_msy, n_ssx, n_ssy;

/*printf("%d \n",widget->type);fflush(stdout);*/


		/* 自分自身の仲間 (next) が存在すれば、再帰的に計算 */
  
  if( widget->next ){
    widget_size( widget->next, &n_msx, &n_msy, &n_ssx, &n_ssy );
  }else{
    n_msx = n_msy = n_ssx = n_ssy = 0;
  }


  if( widget->visible ){

    int	c_msx, c_msy, c_ssx, c_ssy;

		/* 子ウィジットのサイズ計算(再帰) */

    if( widget->child ){
      widget_size( widget->child, &c_msx, &c_msy, &c_ssx, &c_ssy );
    }else{
      c_msx = c_msy = c_ssx = c_ssy = 0;
    }

		/* 子ウィジットを元に、自身のサイズ計算 */

    switch( widget->type ){

    case Q8TK_TYPE_WINDOW:			/* ウインドウ		*/
      if( widget->stat.window.no_frame ){
	widget->sx = c_msx;
	widget->sy = c_msy;
      }else{
	widget->sx = c_msx +2;
	widget->sy = c_msy +2;
      }
      break;

    case Q8TK_TYPE_BUTTON:			/* ボタン		*/
    case Q8TK_TYPE_TOGGLE_BUTTON:		/* トグルボタン		*/
      widget->sx = c_msx +2;
      widget->sy = c_msy +2;
      break;

    case Q8TK_TYPE_CHECK_BUTTON:		/* チェックボタン	*/
    case Q8TK_TYPE_RADIO_BUTTON:		/* ラジオボタン		*/
      widget->sx = c_msx +3;
      widget->sy = Q8TKMAX( c_msy, 1 );
      break;

    case Q8TK_TYPE_FRAME:			/* フレーム		*/
      if( widget->name ){
	widget->sx = Q8TKMAX( c_msx,
			      q8gr_strlen( widget->code, widget->name ) ) + 2;
      }else{
	widget->sx = c_msx + 2;
      }
      widget->sy = c_msy +2;
      break;

    case Q8TK_TYPE_LABEL:			/* ラベル		*/
      if( widget->name ){
	widget->sx = q8gr_strlen( widget->code, widget->name );
      }else{
	widget->sx = 0;
      }
      widget->sy = 1;
      break;

    case Q8TK_TYPE_LOGO:			/* ロゴ			*/
      widget->sx = Q8GR_LOGO_W;
      widget->sy = Q8GR_LOGO_H;
      break;

    case Q8TK_TYPE_NOTEBOOK:			/* ノートブック		*/
      {
	int	len = 0;
	Q8tkWidget *n = widget->child;
	while( n ){
	  if( n->name ){
	    len += q8gr_strlen( n->code, n->name );
	  }
	  len += 1;
	  n = n->next;
	}
	len += 1;
	widget->sx = Q8TKMAX( c_msx +2, len );
      }
      widget->sy = c_msy + 4;
      break;

    case Q8TK_TYPE_NOTEPAGE:			/* ノートブックのページ	*/
      widget->sx = c_msx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_VBOX:			/* 垂直ボックス		*/
      widget->sx = c_msx;
      widget->sy = c_ssy;
      break;
    case Q8TK_TYPE_HBOX:			/* 水平ボックス		*/
      widget->sx = c_ssx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_VSEPARATOR:			/* 垂直区切り線		*/
      widget->sx = 1;
      widget->sy = 1;
      break;
    case Q8TK_TYPE_HSEPARATOR:			/* 水平区切り線		*/
      widget->sx = 1;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_COMBO:			/* コンボボックス	*/
      if( widget->stat.combo.width ){
	widget->sx = widget->stat.combo.width;
      }else{
	if( widget->stat.combo.length ){
	  widget->sx = widget->stat.combo.length;
	}else{
	  widget->sx = 8;
	}
      }
      widget->sx += 3;
      widget->sy = 1;
      break;

    case Q8TK_TYPE_LISTBOX:			/* リストボックス	*/
      widget->sx = Q8TKMAX( c_msx, widget->stat.listbox.width );
      widget->sy = c_ssy;
      break;

    case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
      widget->sx = c_msx;
      widget->sy = c_msy;
      break;

    case Q8TK_TYPE_ADJUSTMENT:			/* アジャストメント	*/
      Q8tkAssert(FALSE,NULL);
      break;

    case Q8TK_TYPE_HSCALE:			/* 水平スケール		*/
    case Q8TK_TYPE_VSCALE:			/* 垂直スケール		*/
      if( widget->stat.scale.adj ){
	int sx, sy;
	adjustment_size( &widget->stat.scale.adj->stat.adj, &sx, &sy );

	if( widget->stat.scale.draw_value ){
	  if( widget->stat.scale.value_pos == Q8TK_POS_LEFT ||
	      widget->stat.scale.value_pos == Q8TK_POS_RIGHT ){
	    widget->sx = sx + 4;
	    widget->sy = Q8TKMAX( sy, 1 );
	  }else{			/* Q8TK_POS_UP||Q8TK_POS_BOTTOM*/
	    widget->sx = Q8TKMAX( sx, 3 );
	    widget->sy = sy + 1;
	  }
	}else{
	  widget->sx = sx;
	  widget->sy = sy;
	}
      }else{
	widget->sx = 0;
	widget->sy = 0;
      }
      break;

    case Q8TK_TYPE_SCROLLED_WINDOW:		/* スクロールドウインドウ */
      if( widget->child ){

	/* 縦横スクロールバーを表示するかどうかは、
	   child のサイズ、scrolled のサイズ、scrolled の policy、
	   の組み合わせにより、以下の 9 パターンに分かれる。

			   0)              1)              2)
			c_msx < w-2	c_msx = w-2	c_msx > w-2
			(NEVER)				(ALLWAYS)
	0)
	  c_msy < h-2				  
	  (NEVER)					  ---
	1)
	  c_msy = h-2					    |
							  --+
	2)
	  c_msy > h-2	    |		    |		    |
	  (ALLWAYS)			  --+		  --+

	*/

	int	w, h, tmp;

	switch( widget->stat.scrolled.hpolicy ){
	case Q8TK_POLICY_NEVER:		w = 0;		break;
	case Q8TK_POLICY_ALWAYS:	w = 2;		break;
	default: /* AUTOMATIC */
	  if     ( c_msx <  widget->stat.scrolled.width - 2 ) w = 0;
	  else if( c_msx == widget->stat.scrolled.width - 2 ) w = 1;
	  else                                                w = 2;
	}

	switch( widget->stat.scrolled.vpolicy ){
	case Q8TK_POLICY_NEVER:		h = 0;		break;
	case Q8TK_POLICY_ALWAYS:	h = 2;		break;
	default: /* AUTOMATIC */
	  if     ( c_msy <  widget->stat.scrolled.height - 2 ) h = 0;
	  else if( c_msy == widget->stat.scrolled.height - 2 ) h = 1;
	  else                                                 h = 2;
	}

	if      ( (w==0||w==1) && (h==0||h==1) ){
	  widget->stat.scrolled.hscrollbar = FALSE;
	  widget->stat.scrolled.vscrollbar = FALSE;
	}else if( (w==2) && (h==0) ){
	  widget->stat.scrolled.hscrollbar = TRUE;
	  widget->stat.scrolled.vscrollbar = FALSE;
	}else if( (w==0) && (h==2) ){
	  widget->stat.scrolled.hscrollbar = FALSE;
	  widget->stat.scrolled.vscrollbar = TRUE;
	}else{
	  widget->stat.scrolled.hscrollbar = TRUE;
	  widget->stat.scrolled.vscrollbar = TRUE;
	}

	w = (widget->stat.scrolled.vscrollbar)
					? widget->stat.scrolled.width - 3
					: widget->stat.scrolled.width - 2;
	h = (widget->stat.scrolled.hscrollbar)
					? widget->stat.scrolled.height - 3
					: widget->stat.scrolled.height - 2;

	q8tk_adjustment_set_length( widget->stat.scrolled.hadj, w );
	q8tk_adjustment_clamp_page( widget->stat.scrolled.hadj, 0, c_msx - w );
	adjustment_size( &widget->stat.scrolled.hadj->stat.adj, &tmp, &tmp );

	q8tk_adjustment_set_length( widget->stat.scrolled.vadj, h );
	q8tk_adjustment_clamp_page( widget->stat.scrolled.vadj, 0, c_msy - h );
	adjustment_size( &widget->stat.scrolled.vadj->stat.adj, &tmp, &tmp );

	if( widget->stat.scrolled.vadj_value
			!= widget->stat.scrolled.vadj->stat.adj.value &&
	    widget->child->type == Q8TK_TYPE_LISTBOX ){
	  list_event_window_scrolled( widget, h );
	}
      }
      widget->stat.scrolled.vadj_value
			=  widget->stat.scrolled.vadj->stat.adj.value;
      widget->sx = widget->stat.scrolled.width;
      widget->sy = widget->stat.scrolled.height;
      break;

    case Q8TK_TYPE_ENTRY:			/* エントリ		*/
      widget->sx = widget->stat.entry.width;
      widget->sy = 1;
      break;


    case Q8TK_TYPE_DIALOG:			/* ダイアログ		*/
      Q8tkAssert(FALSE,NULL);
      break;
    case Q8TK_TYPE_FILE_SELECTION:		/* ファイルセレクション	*/
      Q8tkAssert(FALSE,NULL);
      break;

    default:
      Q8tkAssert(FALSE,"Undefined type");
    }

  }else{
    widget->sx = 0;
    widget->sy = 0;
  }


		/* サイズ情報更新 */

  *max_sx = Q8TKMAX( widget->sx, n_msx );
  *max_sy = Q8TKMAX( widget->sy, n_msy );
  *sum_sx = widget->sx + n_ssx;
  *sum_sy = widget->sy + n_ssy;


		/* 子ウィジットにセパレータが含まれる場合は、サイズ調整 */

  widget_resize( widget, widget->sx, widget->sy );


		/* リストボックスなどの場合、子ウィジットのサイズを調整 */


  if( widget->type == Q8TK_TYPE_LISTBOX ){
    Q8tkWidget *child = widget->child;
    while( child ){
      Q8tkAssert( child->type==Q8TK_TYPE_LIST_ITEM, NULL );
      child->sx = widget->sx;
      if( child->next ) child = child->next;
      else              break;
    }
  }


/*printf("%s (%02d,%02d) max{ %02d,%02d } sum{ %02d,%02d }\n",debug_type(widget->type),widget->sx,widget->sy,*max_sx,*max_sy,*sum_sx,*sum_sy);fflush(stdout);*/
}


/*
 * セパレータなど、親の大きさに依存するウィジットのサイズを再計算する
 */
static	void	widget_resize( Q8tkWidget *widget, int max_sx, int max_sy )
{
  if( widget->type == Q8TK_TYPE_WINDOW   ||
      widget->type == Q8TK_TYPE_NOTEPAGE ||
      widget->type == Q8TK_TYPE_VBOX     ||
      widget->type == Q8TK_TYPE_HBOX     ){

    Q8tkWidget *child = widget->child;
    
    if( widget->type == Q8TK_TYPE_WINDOW &&
	! widget->stat.window.no_frame ){
      max_sx -= 2;
      max_sy -= 2;
    }
    if( child ) widget_resize( child, max_sx, max_sy );

    while( child ){
      switch( child->type ){

      case Q8TK_TYPE_HSEPARATOR:
	if( widget->type != Q8TK_TYPE_HBOX ){
	  if( child->sx < max_sx ) child->sx = max_sx;
	}
	break;

      case Q8TK_TYPE_VSEPARATOR:
	if( widget->type != Q8TK_TYPE_VBOX ){
	  if( child->sy < max_sy ) child->sy = max_sy;
	}
	break;

      case Q8TK_TYPE_VBOX:
	if( widget->type == Q8TK_TYPE_VBOX ){
	  if( child->sx < max_sx ) child->sx = max_sx;
	}
	break;

      case Q8TK_TYPE_HBOX:
	if( widget->type == Q8TK_TYPE_HBOX ){
	  if( child->sy < max_sy ) child->sy = max_sy;
	}
	break;
      }

      if( child->next ) child = child->next;
      else              break;
    }
  }
}



/*------------------------------------------------------*/
/* スクロールドウインドウに含まれるウィジットの表示調整

   スクロールドウインドウに含まれるウィジットで、ウインドウ外に
   あるものに関して、特定のフラグが設定されていれば、強制的に
   スクロール位置を調整し、可能な限りウィジット全体を表示する
   処理を行なう。そのためにいろいろとワークの準備を行なう。

   スクロールドウインドウにあるウィジットが表示されたかどうかは、
   実際に表示するまでわからないので、実際に表示しながらワークを
   設定し、ワークの内容次第で、再表示ということになる。

   つまり、

	for( i=0; i<2; i++ ){
	  widget_size();
	  widget_scroll_adjust_init();
	  widget_draw();
	  if( ! widget_scroll_adjust() ) break;
	}

   となる。

  なお、処理を行なうのは、最初に見つかった 親の SCROLLED WINDOW のみ
  に対してだけなので、SCROLLED WINDOW の入れ子の場合はどうなるか
  わからない。						*/
/*------------------------------------------------------*/

#define	widget_scroll_adj_check_widget( w )	check_scroll_adj_widget( w )


static	void	widget_scroll_adjust_init( void )
{
  int	i;
  for( i=0; i<NR_SCROLL_ADJ; i++ ){
    q8tk_scroll_adj[i].drawn = FALSE;
  }
}

static	int	widget_scroll_adjust( void )
{
  int	i, result = 0;

  for( i=0; i<NR_SCROLL_ADJ; i++ ){

/*printf("%d %d %s\n",i,q8tk_scroll_adj[i].drawn,(q8tk_scroll_adj[i].widget)?debug_type(q8tk_scroll_adj[i].widget->type):"NULL");*/

    if( q8tk_scroll_adj[i].widget ){

      if( q8tk_scroll_adj[i].drawn ){

	Q8tkWidget *widget = q8tk_scroll_adj[i].widget;
	Q8tkWidget *p = widget->parent;
	/*int	x1 = widget->x;*/
	int	x1 = widget->x + widget->sx -1;
	int	y1 = widget->y + widget->sy -1;

	/*
	   SCROLLED WINDOW 内に乗せるウィジット全体を、左上を原点とした、
	   相対位置 real_y

	   y1 = widget->y + widget->sy -1;
	   p->y + 1 - p->stat.scrolled.child_y0 + [ real_y ] = y1;
	*/

	while( p ){
	  if( p->type==Q8TK_TYPE_SCROLLED_WINDOW ){

	    if( p->x +1 <= x1  &&  x1 < p->x +1 + p->stat.scrolled.child_sx ){
	      /* Ok, Expose */
	    }else{
	      Q8TK_ADJUSTMENT(p->stat.scrolled.hadj)->value
			= x1 - ( p->x+1 - p->stat.scrolled.child_x0 )
					- p->stat.scrolled.child_sx + 1;
	      result = 1;
	    }

	    if( p->y +1 <= y1  &&  y1 < p->y +1 + p->stat.scrolled.child_sy ){
	      /* Ok, Expose */
	    }else{
	      Q8TK_ADJUSTMENT(p->stat.scrolled.vadj)->value
			= y1 - ( p->y+1 - p->stat.scrolled.child_y0 )
					- p->stat.scrolled.child_sy + 1;
	      result = 1;
	    }

	    /*
	    printf("  %s %d %d %d %d  %d %d %d %d\n",debug_type(p->type),
		   p->x, p->y, p->sx, p->sy,
		   p->stat.scrolled.child_x0, p->stat.scrolled.child_y0,
		   p->stat.scrolled.child_sx, p->stat.scrolled.child_sy );
	    printf("  %s %d %d %d %d\n",debug_type(widget->type),
		   widget->x, widget->y, widget->sx, widget->sy );
	    printf("  %d\n",y1);
	    if( result ) printf("ADJ %d,%d\n",
				Q8TK_ADJUSTMENT(p->stat.scrolled.vadj)->value,
				Q8TK_ADJUSTMENT(p->stat.scrolled.hadj)->value);
	    */


	    break;
	  }
	  p = p->parent;
	}
	q8tk_scroll_adj[i].drawn  = FALSE;
	q8tk_scroll_adj[i].widget = NULL;
      }
    }
  }

  return result;
}






/*------------------------------------------------------*/
/* 自分自身の表示位置をもとに、描画し、
   再帰的に、全ての子ウィジェットも描画する。		*/
/*------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*
   ウィジットのTAB関連づけ
	TABキーによる、フォーカスの移動を実現するために、
	描画前、描画中、描画後に各種設定を行なう。

		widget_tab_pre()  … TABづけの初期化
		widget_draw()     … 描画。内部でTABづけを行なう
		widget_tab_post() … TABづけの後処理

	widget_draw() の内部では、TABづけを行ないたいウィジットの描画の
	際に、『widget_tab_set()』 関数を呼ぶ。

	また、TAB付けされたウィジットが、アクティブ状態かどうかを知るには、
	描画の前後を、『check_active()』と『check_active_finish()』関数で囲む。

	【一般的な例】

	    widget_tab_set( widget );
	    check_active( widget );
	    q8gr_draw_xxx( x, y, is_active_widget(), widget );
	    if( widget->child ){
	      widget_draw();
	    }
	    check_active_finish();

	is_active_widget() は、自身がアクティブ状態かどうかを調べる関数で
	あり、check_active()〜check_active_finish() の間で、有効である。

	上の場合、アクティブ状態は子ウィジットに伝播する。
	例えば、ボタンウィジットの場合、ボタンの子ウィジットであるラベルに、
	（アクティブな場合は）アンダーラインを引く、という風につかえる。
*/
/* BACKTAB 対応  thanks! floi */

static	Q8tkWidget	*tab_last_widget;
static	void	widget_tab_pre( void )
{
  tab_last_widget     = NULL;
  q8tk_tab_top_widget = NULL;
}
static	void	widget_tab_post( void )
{
  if( tab_last_widget ){
    tab_last_widget->tab_next     = q8tk_tab_top_widget;
    q8tk_tab_top_widget->tab_prev = tab_last_widget;
  }
}

#define	widget_tab_set( widget )				\
		do{						\
		  if( tab_last_widget == NULL ){		\
		    q8tk_tab_top_widget       = (widget);	\
		  }else{					\
		    tab_last_widget->tab_next = (widget);	\
		    (widget)->tab_prev = tab_last_widget;	\
		  }						\
		  tab_last_widget = widget;			\
		}while(0)
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */





static	void	widget_draw( Q8tkWidget *widget )
{
  int		x = widget->x;
  int		y = widget->y;
  Q8tkWidget	*child = widget->child;


  widget_scroll_adj_check_widget( widget );


		/* HBOX、VBOX の子の場合に限り、*/
		/* 配置を若干変更できる。	*/
		/* FRAME もついでにヨシとしよう */

  if( widget->parent ){
    switch( (widget->parent)->type ){
    case Q8TK_TYPE_FRAME:
      if      ( widget->placement_x == Q8TK_PLACEMENT_X_CENTER ){
	x += ( (widget->parent)->sx -2 - widget->sx ) / 2;
      }else if( widget->placement_x == Q8TK_PLACEMENT_X_RIGHT ){
	x += ( (widget->parent)->sx -2 - widget->sx );
      }
      widget->x = x;
      break;
    case Q8TK_TYPE_VBOX:
      if      ( widget->placement_x == Q8TK_PLACEMENT_X_CENTER ){
	x += ( (widget->parent)->sx - widget->sx ) / 2;
      }else if( widget->placement_x == Q8TK_PLACEMENT_X_RIGHT ){
	x += ( (widget->parent)->sx - widget->sx );
      }
      widget->x = x;
      break;
    case Q8TK_TYPE_HBOX:
      if      ( widget->placement_y == Q8TK_PLACEMENT_Y_CENTER ){
	y += ( (widget->parent)->sy - widget->sy ) / 2;
      }else if( widget->placement_y == Q8TK_PLACEMENT_Y_BOTTOM ){
	y += ( (widget->parent)->sy - widget->sy );
      }
      widget->y = y;
      break;
    }
  }


		/* 自分自身の typeをもとに枠などを書く。*/
		/* 子がいれば、x,y を求る。		*/
		/* 子の仲間(next)の、x,y も求める。	*/
		/* 直下の子に対してのみ再帰的に処理。	*/

/*printf("%s (%d,%d) %d %d\n",debug_type(widget->type),widget->sx,widget->sy,widget->x,widget->y);fflush(stdout);*/

  if( widget->visible ){

    switch( widget->type ){

    case Q8TK_TYPE_WINDOW:			/* ウインドウ		*/
      if( widget->stat.window.no_frame ){
	/* no frame */
      }else{
	q8gr_draw_window( x, y, widget->sx, widget->sy,
			  widget->stat.window.shadow_type );
      }
      if( child ){
	child->x = x  + (widget->stat.window.no_frame ? 0 : 1);
	child->y = y  + (widget->stat.window.no_frame ? 0 : 1);
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_BUTTON:			/* ボタン		*/
    case Q8TK_TYPE_TOGGLE_BUTTON:		/* トグルボタン		*/
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_button( x, y, widget->sx, widget->sy,
		        widget->stat.button.active, widget );
      if( child ){
	child->x = x + 1;
	child->y = y + 1;
	widget_draw( child );
      }
      check_active_finish();
      break;

    case Q8TK_TYPE_CHECK_BUTTON:		/* チェックボタン	*/
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_check_button( x, y, widget->stat.button.active, widget );
      if( child ){
	child->x = x + 3;
	child->y = y;
	widget_draw( child );
      }
      check_active_finish();
      break;

    case Q8TK_TYPE_RADIO_BUTTON:		/* ラジオボタン		*/
      widget_tab_set( widget );
      check_active( widget );
      q8gr_draw_radio_button( x, y, widget->stat.button.active, widget );
      if( child ){
	child->x = x + 3;
	child->y = y;
	widget_draw( child );
      }
      check_active_finish();
      break;

    case Q8TK_TYPE_FRAME:			/* フレーム		*/
      q8gr_draw_frame( x, y, widget->sx, widget->sy,
		       widget->stat.frame.shadow_type,
		       widget->code, widget->name );
      if( child ){
	child->x = x + 1;
	child->y = y + 1;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_LABEL:			/* ラベル		*/
      q8gr_puts( x, y,
		 widget->stat.label.foreground,
		 widget->stat.label.background,
		 widget->stat.label.reverse,
		 is_active_widget(),
		 (widget->name) ?(widget->code) : Q8TK_KANJI_ANK,
		 (widget->name) ?(widget->name) :"" );
      break;

    case Q8TK_TYPE_LOGO:			/* ロゴ			*/
      q8gr_draw_logo( x, y );
      break;

    case Q8TK_TYPE_NOTEBOOK:			/* ノートブック		*/
      q8gr_draw_notebook( x, y, widget->sx, widget->sy );
      if( child ){
	child->x = x;
	child->y = y;
	widget_draw( child );
      }
      break;
    case Q8TK_TYPE_NOTEPAGE:			/* ノートページ		*/
      {
	int select_flag = (widget==(widget->parent)->stat.notebook.page);
	check_active( widget );
	q8gr_draw_notepage( (widget->name) ? widget->code : Q8TK_KANJI_ANK,
			    (widget->name) ? widget->name : "",
			    select_flag,
			    (select_flag) ? FALSE : is_active_widget(),
			    (select_flag) ? NULL : widget );
	check_active_finish();
	if( child && select_flag ){
	  child->x = ((widget->parent)->x) + 1;
	  child->y = ((widget->parent)->y) + 3;
	  widget_draw( child );
	}else{
	  widget_tab_set( widget );
	}
      }
      break;

    case Q8TK_TYPE_VBOX:			/* 垂直ボックス		*/
      if( child ){
	child->x = x;	x += 0;
	child->y = y;	y += child->sy;
	while( child->next ){
	  child = child->next;
	  child->x = x;	x += 0;
	  child->y = y;	y += child->sy;
	}
	child = widget->child;
	widget_draw( child );
      }
      break;
    case Q8TK_TYPE_HBOX:			/* 水平ボックス		*/
      if( child ){
	child->x = x;	x += child->sx;
	child->y = y;	y += 0;
	while( child->next ){
	  child = child->next;
	  child->x = x;	x += child->sx;
	  child->y = y;	y += 0;
	}
	child = widget->child;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_VSEPARATOR:			/* 垂直区切り線		*/
      q8gr_draw_vseparator( x, y, widget->sy );
      break;
    case Q8TK_TYPE_HSEPARATOR:			/* 水平区切り線		*/
      q8gr_draw_hseparator( x, y, widget->sx );
      break;

    case Q8TK_TYPE_COMBO:			/* コンボボックス	*/
      if( widget->stat.combo.entry->stat.entry.editable ){
#if 1
	widget->stat.combo.entry->x  = x;
	widget->stat.combo.entry->y  = y;
	widget->stat.combo.entry->sx = widget->sx -3;
	widget_draw( widget->stat.combo.entry );
#else
	widget_tab_set( widget->stat.combo.entry );
	check_active( widget->stat.combo.entry );

	if( is_active_widget() &&
	    widget->stat.combo.entry->stat.entry.cursor_pos < 0 ){
	  q8tk_entry_set_position( widget->stat.combo.entry,
				   strlen( widget->stat.combo.entry->name ) );
	}

	widget->stat.combo.entry->x  = x;
	widget->stat.combo.entry->y  = y;
	widget->stat.combo.entry->sx = widget->sx -3;
	q8gr_draw_entry( x, y, widget->stat.combo.entry->sx,
			 widget->stat.combo.entry->code,
			 widget->stat.combo.entry->name,
			 widget->stat.combo.entry->stat.entry.disp_pos,
			 (is_active_widget())
			 ? widget->stat.combo.entry->stat.entry.cursor_pos: -1,
			 widget->stat.combo.entry );
	check_active_finish();
#endif
	widget_tab_set( widget );
	check_active( widget );
	q8gr_draw_combo( x, y, widget->sx -3,
			 is_active_widget(), widget );
	check_active_finish();
      }else{
	widget_tab_set( widget );
	check_active( widget );
	q8gr_draw_entry( x, y, widget->sx -3,
			 widget->stat.combo.entry->code,
			 widget->stat.combo.entry->name,
			 0, -1, widget );
	q8gr_draw_combo( x, y, widget->sx -3,
			 is_active_widget(), widget );
	check_active_finish();
      }
      break;

    case Q8TK_TYPE_LISTBOX:			/* リストボックス	*/
      widget_tab_set( widget );
      if( child ){
	child->x = x;	x += 0;
	child->y = y;	y += child->sy;
	while( child->next ){
	  child = child->next;
	  child->x = x;	x += 0;
	  child->y = y;	y += child->sy;
	}
	child = widget->child;
	widget_draw( child );
      }
      break;

    case Q8TK_TYPE_LIST_ITEM:			/* リストアイテム	*/
      check_active( widget->parent );
      if( child ){
	int rev   = (widget->parent->stat.listbox.selected==widget) ?TRUE :FALSE;
	int under = (widget->parent->stat.listbox.active  ==widget) ?TRUE :FALSE;
	if( rev && under ) under = FALSE;

	q8gr_draw_list_item( x, y, widget->sx, is_active_widget(), rev, under,
			     child->code, child->name, widget );
      }
      check_active_finish();
      break;

    case Q8TK_TYPE_ADJUSTMENT:			/* アジャストメント	*/
      Q8tkAssert(FALSE,NULL);
      break;

    case Q8TK_TYPE_HSCALE:			/* 水平スケール		*/
      if( widget->stat.scale.adj ){
	Q8Adjust *adj = &(widget->stat.scale.adj->stat.adj);
	widget_tab_set( widget->stat.scale.adj );
	widget->stat.scale.adj->x = x;
	widget->stat.scale.adj->y = y;
	check_active( widget->stat.scale.adj );
	q8gr_draw_hscale( x, y, adj, is_active_widget(),
			  widget->stat.scale.draw_value,
			  widget->stat.scale.value_pos,
			  widget->stat.scale.adj );
	check_active_finish();
      }
      break;
    case Q8TK_TYPE_VSCALE:			/* 垂直スケール		*/
      if( widget->stat.scale.adj ){
	Q8Adjust *adj = &(widget->stat.scale.adj->stat.adj);
	widget->stat.scale.adj->x = x;
	widget->stat.scale.adj->y = y;
	widget_tab_set( widget->stat.scale.adj );
	check_active( widget->stat.scale.adj );
	q8gr_draw_vscale( x, y, adj, is_active_widget(),
			  widget->stat.scale.draw_value,
			  widget->stat.scale.value_pos,
			  widget->stat.scale.adj );
	check_active_finish();
      }
      break;

    case Q8TK_TYPE_SCROLLED_WINDOW:		/* スクロールドウインドウ */
      if( child ){
	int sx = widget->sx;
	int sy = widget->sy;

	if( widget->stat.scrolled.hscrollbar ){
	  widget_tab_set( widget->stat.scrolled.hadj );
	  check_active( widget->stat.scrolled.hadj );
	  q8gr_draw_hscale( x, y + widget->sy -1,
			    &(widget->stat.scrolled.hadj->stat.adj),
			    is_active_widget(),
			    FALSE, 0, widget->stat.scrolled.hadj );
	  check_active_finish();
	  sy --;
	}
	if( widget->stat.scrolled.vscrollbar ){
	  widget_tab_set( widget->stat.scrolled.vadj );
	  check_active( widget->stat.scrolled.vadj );
	  q8gr_draw_hscale( x + widget->sx -1, y, 
			    &(widget->stat.scrolled.vadj->stat.adj),
			    is_active_widget(),
			    FALSE, 0, widget->stat.scrolled.vadj );
	  check_active_finish();
	  sx --;
	}
	q8gr_draw_scrolled_window( x,y, sx,sy, Q8TK_SHADOW_ETCHED_OUT, widget);
	q8gr_set_screen_mask( x+1, y+1, sx-2, sy-2 );
	widget->stat.scrolled.child_x0
				= widget->stat.scrolled.hadj->stat.adj.value;
	widget->stat.scrolled.child_y0
				= widget->stat.scrolled.vadj->stat.adj.value;
	widget->stat.scrolled.child_sx = sx -2;
	widget->stat.scrolled.child_sy = sy -2;

	child->x = x - widget->stat.scrolled.child_x0 +1;
	child->y = y - widget->stat.scrolled.child_y0 +1;

	widget_draw( child );
	q8gr_reset_screen_mask();
      }else{
	q8gr_draw_window( x,y, widget->sx,widget->sy, Q8TK_SHADOW_ETCHED_OUT );
      }
      break;

    case Q8TK_TYPE_ENTRY:			/* エントリ		*/
      {
	widget_tab_set( widget );
	check_active( widget );

	if( is_active_widget() &&
	    widget->stat.entry.editable &&
	    widget->stat.entry.cursor_pos < 0 ){
	  q8tk_entry_set_position( widget, strlen( widget->name ) );
	}

	q8gr_draw_entry( x, y, widget->sx, widget->code, widget->name,
			 widget->stat.entry.disp_pos,
			 ((is_active_widget() && widget->stat.entry.editable)
					? widget->stat.entry.cursor_pos: -1 ),
			 widget );
	check_active_finish();
      }
      break;

    case Q8TK_TYPE_DIALOG:			/* ダイアログ		*/
      Q8tkAssert(FALSE,NULL);
      break;
    case Q8TK_TYPE_FILE_SELECTION:		/* ファイルセレクション	*/
      Q8tkAssert(FALSE,NULL);
      break;

    default:
      Q8tkAssert(FALSE,"Undefined type");
    }
  }


	/* 自分自身の仲間 (next) が存在すれば、再帰的に処理 */
  
  if( widget->next ){
    widget = widget->next;
    widget_draw( widget );
  }
}














/*------------------------------------------------------*/
/* スクリーン画面を作成。
   q8tk_grab_add() で設定された WINDOW をトップとして、
   全ての子ウィジェットの大きさ、位置を計算し、
   menu_screen[][]に、表示内容を設定する。
   同時に、TAB キーを押された時の、フォーカスの変更の
   手順も決めておく。					*/
/*------------------------------------------------------*/

static	void	widget_construct( void )
{
  int		i, j, tmp;
  Q8tkWidget	*widget;

  q8gr_clear_screen();

  for( i=0; i<MAX_WINDOW_LEVEL; i++ ){

    widget = window_level[i];

    if( widget ){
      Q8tkAssert(widget->type==Q8TK_TYPE_WINDOW,NULL);

      for( j=0; j<2; j++ ){

	q8gr_clear_focus_screen();

	widget_size( widget, &tmp, &tmp, &tmp, &tmp );

	if( widget->stat.window.set_position==FALSE ){
	  widget->x = ( Q8GR_SCREEN_X - widget->sx ) / 2;
	  widget->y = ( Q8GR_SCREEN_Y - widget->sy ) / 2;
	}else{
	  widget->x = widget->stat.window.x;
	  widget->y = widget->stat.window.y;
	}

	widget_scroll_adjust_init();

	widget_tab_pre();
	widget_draw( widget );
	widget_tab_post();

	if( widget_scroll_adjust() ){
	  /* Redraw! */
	}else{
	  break;
	}

      }

    }else{
      break;
    }
  }


  if( get_drag_widget() ){
    /* none */
  }else{
    Q8tkWidget *w;
    int exist;

    w = q8tk_tab_top_widget;

    if( w ){

      do{				/* TAB TOP を NOTEPAGE 以外に設定 */
	if( w->type != Q8TK_TYPE_NOTEPAGE ) break;
	w = w->tab_next;
      } while( w != q8tk_tab_top_widget );
      q8tk_tab_top_widget = w;

      exist = FALSE;			/* event_widget が実在するかチェック */
      do{			
	if( w==get_event_widget() ){
	  exist = TRUE;
	  break;
	}
	w = w->tab_next;
      } while( w != q8tk_tab_top_widget );
      if( !exist ){			/*    実在しなければ NULL にしておく */
	set_event_widget( NULL );
      }
    }
  }

  if( disp_cursor ){
    q8gr_draw_mouse( mouse.x/8, mouse.y/16 );
  }


#if 0
for(tmp=0,i=0; i<MAX_WIDGET; i++ ){ if( widget_table[i] ) tmp++; }
printf("[TOTAL WIDGET] %d : ",tmp);
for(tmp=0,i=0; i<MAX_LIST; i++ ){ if( list_table[i] ) tmp++; }
printf("[TOTAL LIST] %d\n",tmp);
#endif

/*
{
  Q8tkWidget *w = q8tk_tab_top_widget;
  while( w ){
    printf("%s\n",debug_type(w->type));
    w = w->tab_next;
    if( w == q8tk_tab_top_widget ) break;
  }
}
*/
}
