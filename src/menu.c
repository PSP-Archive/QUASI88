/************************************************************************/
/*									*/
/* ��˥塼�⡼��							*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "initval.h"
#include "menu.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "screen.h"

#include "emu.h"
#include "drive.h"
#include "image.h"
#include "status.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "file-op.h"
#include "suspend.h"
#include "snapshot.h"
#include "fdc.h"
#include "soundbd.h"

#include "event.h"
#include "q8tk.h"



int	menu_lang	= LANG_JAPAN;		/* ��˥塼�θ���           */
int	menu_readonly	= FALSE;		/* �ǥ������������������ */
						/* ������֤� ReadOnly ?    */
int	menu_swapdrv	= FALSE;		/* �ɥ饤�֤�ɽ�����       */


/*--------------------------------------------------------------*/
/* ��˥塼�Ǥ�ɽ����å����������ơ����Υե���������		*/
/*--------------------------------------------------------------*/
#include "message.h"




/****************************************************************/
/* ���							*/
/****************************************************************/

static	int	menu_last_page = 0;	/* ������Υ�˥塼�����򵭲� */

static	int	menu_boot_dipsw;	/* �ꥻ�åȻ�������򵭲� */
static	int	menu_boot_from_rom;
static	int	menu_boot_basic;
static	int	menu_boot_clock_4mhz;
static	int	menu_boot_version;
static	int	menu_boot_baudrate;
static	int	menu_boot_sound_board;

					/* ��ư�ǥХ����������ɬ�� */
static	Q8tkWidget	*widget_reset_boot;
static	Q8tkWidget	*widget_dipsw_b_boot_disk;
static	Q8tkWidget	*widget_dipsw_b_boot_rom;

static	Q8tkWidget	*menu_accel;	/* �ᥤ���˥塼�Υ������ */





/*===========================================================================
 * �ե��������顼��å������Υ�����������
 *===========================================================================*/
static	void	cb_file_error_dialog_ok( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dialog_destroy();
}

static	void	start_file_error_dialog( int drv, int result )
{
  char wk[128];
  const t_menulabel *l = (drv<0) ? data_err_file : data_err_drive;

  if( result==ERR_NO ) return;
  if( drv<0 ) sprintf( wk, GET_LABEL( l, result ) );
  else        sprintf( wk, GET_LABEL( l, result ), drv+1 );

  dialog_create();
  {
    dialog_set_title( wk );
    dialog_set_separator();
    dialog_set_button( GET_LABEL( l, ERR_NO ),
		       cb_file_error_dialog_ok, NULL );
  }
  dialog_start();
}

/*===========================================================================
 * �ǥ��������� & �ӽ�
 *===========================================================================*/
static void sub_misc_suspend_change( void );
static void sub_misc_snapshot_change( void );

/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ꥻ�å�
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						 /* BASIC�⡼���ڤ��ؤ� */
static	int	get_reset_basic( void )
{
  return menu_boot_basic;
}
static	void	cb_reset_basic( Q8tkWidget *dummy, void *p )
{
  menu_boot_basic = (int)p;
}


static	Q8tkWidget	*menu_reset_basic( void )
{
  Q8tkWidget	*box;

  box = PACK_VBOX( NULL );
  {
    PACK_RADIO_BUTTONS( box,
			data_reset_basic, COUNTOF(data_reset_basic),
			get_reset_basic(), cb_reset_basic );
  }

  return box;
}

/*----------------------------------------------------------------------*/
						       /* CLOCK�ڤ��ؤ� */
static	int	get_reset_clock( void )
{
  if( menu_boot_clock_4mhz ) return CLOCK_4MHZ;
  else                       return CLOCK_8MHZ;
}
static	void	cb_reset_clock( Q8tkWidget *dummy, void *p )
{
  if( (int)p == CLOCK_4MHZ ) menu_boot_clock_4mhz = TRUE;
  else                       menu_boot_clock_4mhz = FALSE;
}


static	Q8tkWidget	*menu_reset_clock( void )
{
  Q8tkWidget	*box;

  box = PACK_VBOX( NULL );
  {
    PACK_RADIO_BUTTONS( box,
			data_reset_clock, COUNTOF(data_reset_clock),
			get_reset_clock(), cb_reset_clock );
  }

  return box;
}

/*----------------------------------------------------------------------*/
						  /* �С�������ڤ��ؤ� */
static	int	get_reset_version( void )
{
  return menu_boot_version;
}
static	void	cb_reset_version( Q8tkWidget *widget, void *dummy )
{
  menu_boot_version = *(q8tk_combo_get_text(widget));
}


static	Q8tkWidget	*menu_reset_version( void )
{
  Q8tkWidget	*combo;
  char		wk[4];

  wk[0] = get_reset_version();
  wk[1] = '\0';

  combo = PACK_COMBO( NULL,
		      data_reset_version, COUNTOF(data_reset_version),
		      get_reset_version(), wk, 4,
		      cb_reset_version, NULL );
  return combo;
}

/*----------------------------------------------------------------------*/
						/* �ǥ��åץ����å����� */
static	void	dipsw_create( void );
static	void	dipsw_start( void );
static	void	dipsw_finish( void );

static	void	cb_reset_dipsw( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dipsw_start();
}

static	void	set_reset_dipsw_boot( void )
{
  const t_menulabel  *l = data_reset_boot;

  if( widget_reset_boot ){
    q8tk_label_set( widget_reset_boot,
		    ( menu_boot_from_rom ? GET_LABEL( l, 1 )
					 : GET_LABEL( l, 0 ) ) );
  }
}


static	Q8tkWidget	*menu_reset_dipsw( void )
{
  Q8tkWidget	*vbox;
  Q8tkWidget	*button;
  const t_menulabel *l = data_reset;

  vbox = PACK_VBOX( NULL );
  {
    button = PACK_BUTTON( vbox,
			  GET_LABEL( l, DATA_RESET_DIPSW_BTN ),
			  cb_reset_dipsw, NULL );
    q8tk_misc_set_placement( button, Q8TK_PLACEMENT_X_CENTER,
				     Q8TK_PLACEMENT_Y_CENTER );

    widget_reset_boot = PACK_LABEL( vbox, "" );
    set_reset_dipsw_boot();
  }
  return vbox;
}

/*----------------------------------------------------------------------*/
								/* ����¾ */
static	int	get_reset_misc_sb( void )
{
  return menu_boot_sound_board;
}
static	void	cb_reset_misc_sb( Q8tkWidget *dummy, void *p )
{
  menu_boot_sound_board = (int)p;
}


static	Q8tkWidget	*menu_reset_misc( void )
{
  Q8tkWidget	*box;

  box = PACK_VBOX( NULL );
  {
    PACK_RADIO_BUTTONS( box,
			data_reset_misc_sb, COUNTOF(data_reset_misc_sb),
			get_reset_misc_sb(), cb_reset_misc_sb );
  }

  return box;
}

/*----------------------------------------------------------------------*/
							    /* �ꥻ�å� */
static	void	cb_reset_now( Q8tkWidget *dummy_0, void *dummy_1 )
{
  int now_board = sound_board;

  boot_dipsw      = menu_boot_dipsw;
  boot_from_rom   = menu_boot_from_rom;
  boot_basic      = menu_boot_basic;
  boot_clock_4mhz = menu_boot_clock_4mhz;
  set_version     = menu_boot_version;
  baudrate_sw     = menu_boot_baudrate;
  sound_board     = menu_boot_sound_board;

  quasi88_reset();

  if( now_board != sound_board ){
    if( memory_allocate_additional() == FALSE ){
      quasi88_exit();	/* ���ԡ� */
    }
    xmame_sound_resume();		/* ���Ǥ���������ɤ�������� */
    xmame_sound_stop();			/* ������ɤ���ߤ����롣     */
    xmame_sound_start();		/* �����ơ�������ɺƽ����   */
  }

  set_emu_mode( GO );

  q8tk_main_quit();

#if 0
printf(  "boot_dipsw      %04x\n",boot_dipsw    );
printf(  "boot_from_rom   %d\n",boot_from_rom   );
printf(  "boot_basic      %d\n",boot_basic      );
printf(  "boot_clock_4mhz %d\n",boot_clock_4mhz );
printf(  "ROM_VERSION     %c\n",ROM_VERSION     );
printf(  "baudrate_sw     %d\n",baudrate_sw     );
#endif
}

/*----------------------------------------------------------------------*/
						   /* ���ߤ�BASIC�⡼�� */
static	Q8tkWidget	*menu_reset_current( void )
{
  static const char *type[] = {
    "PC-8801",
    "PC-8801",
    "PC-8801",
    "PC-8801mkII",
    "PC-8801mkIISR",
    "PC-8801mkIITR/FR/MR",
    "PC-8801mkIITR/FR/MR",
    "PC-8801mkIITR/FR/MR",
    "PC-8801FH/MH",
    "PC-8801FA/MA/FE/MA2/FE2/MC",
  };
  static const char *basic[] = { " N ", "V1S", "V1H", " V2", };
  static const char *clock[] = { "8MHz", "4MHz", };
  const char *t = "";
  const char *b = "";
  const char *c = "";
  int i;
  char wk[80], ext[40];

  i = (get_reset_version() & 0xff) - '0';
  if( 0 <= i  &&  i< COUNTOF(type) ) t = type[ i ];

  i = get_reset_basic();
  if( 0 <= i  &&  i< COUNTOF(basic) ) b = basic[ i ];

  i = get_reset_clock();
  if( 0 <= i  &&  i< COUNTOF(clock) ) c = clock[ i ];

  ext[0] = 0;
  {
    if( sound_port ){
      if( ext[0]==0 ) strcat( ext, "(" );
      else            strcat( ext, ", " );
      if( sound_board == SOUND_I ) strcat( ext, "OPN" );
      else                         strcat( ext, "OPNA" );
    }

    if( use_extram ){
      if( ext[0]==0 ) strcat( ext, "(" );
      else            strcat( ext, ", " );
      sprintf( wk, "%dKB", use_extram * 128 );
      strcat( ext, wk );
      strcat( ext, GET_LABEL(data_reset_current,0) );/* ExtRAM*/
    }

    if( use_jisho_rom ){
      if( ext[0]==0 ) strcat( ext, "(" );
      else            strcat( ext, ", " );
      strcat( ext, GET_LABEL(data_reset_current,1) );/*DictROM*/
    }
  }
  if( ext[0] ) strcat( ext, ")" );


  sprintf( wk, " %-30s  %4s  %4s  %30s ",
	   t, b, c, ext );

  return PACK_LABEL( NULL, wk );
}

/*======================================================================*/

static	Q8tkWidget	*menu_reset( void )
{
  Q8tkWidget *hbox, *vbox;
  Q8tkWidget *w, *f;
  const t_menulabel *l = data_reset;

  dipsw_create();		/* �ǥ��åץ����å�������ɥ����� */

  vbox = PACK_VBOX( NULL );
  {
    f = PACK_FRAME( vbox, "", menu_reset_current() );
    q8tk_frame_set_shadow_type( f, Q8TK_SHADOW_ETCHED_OUT );

    hbox = PACK_HBOX( vbox );
    {
      PACK_FRAME( hbox,
		  GET_LABEL( l, DATA_RESET_BASIC ), menu_reset_basic() );

      PACK_FRAME( hbox,
		  GET_LABEL( l, DATA_RESET_CLOCK ), menu_reset_clock() );

      PACK_FRAME( hbox,
		  GET_LABEL( l, DATA_RESET_VERSION ), menu_reset_version() );

      PACK_FRAME( hbox,
		  GET_LABEL( l, DATA_RESET_DIPSW ), menu_reset_dipsw() );
    }

    PACK_FRAME( vbox,
		GET_LABEL( l, DATA_RESET_MISC ), menu_reset_misc() );

    PACK_LABEL( vbox, GET_LABEL( l, DATA_RESET_NOTICE ) );

    w = PACK_BUTTON( vbox,
		     GET_LABEL( data_reset, DATA_RESET_NOW ),
		     cb_reset_now, NULL );
    q8tk_misc_set_placement( w, Q8TK_PLACEMENT_X_RIGHT, 0 );
  }

  return vbox;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	���֥�����ɥ�	DIPSW
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static	Q8tkWidget	*dipsw_window;
static	Q8tkWidget	*dipsw[4];
static	Q8tkWidget	*dipsw_accel;

enum {
  DIPSW_WIN,
  DIPSW_FRAME,
  DIPSW_VBOX,
  DIPSW_QUIT
};

/*----------------------------------------------------------------------*/
					    /* �ǥ��åץ����å��ڤ��ؤ� */
static	int	get_dipsw_b( int p )
{
  int	shift = data_dipsw_b[p].val;

  return ( (p<<1) | ( (menu_boot_dipsw >> shift) & 1 ) );
}
static	void	cb_dipsw_b( Q8tkWidget *dummy, void *p )
{
  int	shift = data_dipsw_b[ (int)p >> 1 ].val;
  int	on    = (int)p & 1;

  if( on ) menu_boot_dipsw |=  (1 << shift );
  else     menu_boot_dipsw &= ~(1 << shift );
}
static	int	get_dipsw_b2( void )
{
  return ( menu_boot_from_rom ? TRUE : FALSE );
}
static	void	cb_dipsw_b2( Q8tkWidget *dummy, void *p )
{
  if( (int)p ) menu_boot_from_rom = TRUE;
  else         menu_boot_from_rom = FALSE;

  set_reset_dipsw_boot();
}


static	Q8tkWidget	*menu_dipsw_b( void )
{
  int	i;
  Q8tkWidget	*vbox, *hbox;
  Q8tkWidget	*b = NULL;
  const t_dipsw *pd;
  const t_menudata *p;


  vbox = PACK_VBOX( NULL );
  {
    pd = data_dipsw_b;
    for( i=0; i<COUNTOF(data_dipsw_b); i++, pd++ ){

      hbox = PACK_HBOX( vbox );
      {
	PACK_LABEL( hbox, GET_LABEL( pd, 0 ) );

	PACK_RADIO_BUTTONS( hbox,
			    pd->p, 2,
			    get_dipsw_b(i), cb_dipsw_b );
      }
    }

    hbox = PACK_HBOX( vbox );
    {
      pd = data_dipsw_b2;
      p  = pd->p;

      PACK_LABEL( hbox, GET_LABEL( pd, 0 ) );

      for( i=0; i<2; i++, p++ ){
	b = PACK_RADIO_BUTTON( hbox,
			       b,
			       GET_LABEL( p, 0 ), 
			       (get_dipsw_b2() == p->val) ? TRUE : FALSE,
			       cb_dipsw_b2, (void *)(p->val) );

	if( i==0 ) widget_dipsw_b_boot_disk = b;	/* �����Υܥ���� */
	else       widget_dipsw_b_boot_rom  = b;	/* �Ф��Ƥ���       */
      }
    }
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
				   /* �ǥ��åץ����å��ڤ��ؤ�(RS-232C) */
static	int	get_dipsw_r( int p )
{
  int	shift = data_dipsw_r[p].val;

  return ( (p<<1) | ( (menu_boot_dipsw >> shift) & 1 ) );
}
static	void	cb_dipsw_r( Q8tkWidget *dummy, void *p )
{
  int	shift = data_dipsw_r[ (int)p >> 1 ].val;
  int	on    = (int)p & 1;

  if( on ) menu_boot_dipsw |=  (1 << shift );
  else     menu_boot_dipsw &= ~(1 << shift );
}
static	int	get_dipsw_r_baudrate( void )
{
  return menu_boot_baudrate;
}
static	void	cb_dipsw_r_baudrate( Q8tkWidget *widget, void *dummy )
{
  int	i;
  const t_menudata *p = data_dipsw_r_baudrate;
  const char       *combo_str = q8tk_combo_get_text(widget);

  for( i=0; i<COUNTOF(data_dipsw_r_baudrate); i++, p++ ){
    if( strcmp( p->str[menu_lang], combo_str ) == 0 ){
      menu_boot_baudrate = p->val;
      return;
    }
  }
}


static	Q8tkWidget	*menu_dipsw_r( void )
{
  int	i;
  Q8tkWidget	*vbox, *hbox;
  const t_dipsw *pd;

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );
    {
      PACK_LABEL( hbox, GET_LABEL( data_dipsw_r2, 0 ) );

      PACK_COMBO( hbox,
		  data_dipsw_r_baudrate,
		  COUNTOF(data_dipsw_r_baudrate),
		  get_dipsw_r_baudrate(), NULL, 8,
		  cb_dipsw_r_baudrate, NULL );
    }

    pd = data_dipsw_r;
    for( i=0; i<COUNTOF(data_dipsw_r); i++, pd++ ){

      hbox = PACK_HBOX( vbox );
      {
	PACK_LABEL( hbox, GET_LABEL( data_dipsw_r, i ) );

	PACK_RADIO_BUTTONS( hbox,
			    pd->p, 2,
			    get_dipsw_r(i), cb_dipsw_r );
      }
    }
  }

  return vbox;
}

/*----------------------------------------------------------------------*/

static	void	dipsw_create( void )
{
  Q8tkWidget *vbox;
  const t_menulabel *l = data_dipsw;

  vbox = PACK_VBOX( NULL );
  {
    PACK_FRAME( vbox, GET_LABEL( l, DATA_DIPSW_B ), menu_dipsw_b() );

    PACK_FRAME( vbox, GET_LABEL( l, DATA_DIPSW_R ), menu_dipsw_r() );
  }

  dipsw_window = vbox;
}

static	void	cb_reset_dipsw_end( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dipsw_finish();
}

static	void	dipsw_start( void )
{
  Q8tkWidget	*w, *f, *x, *b;
  const t_menulabel *l = data_reset;

  {						/* �ᥤ��Ȥʤ륦����ɥ� */
    w = q8tk_window_new( Q8TK_WINDOW_DIALOG );
    dipsw_accel = q8tk_accel_group_new();
    q8tk_accel_group_attach( dipsw_accel, w );
  }
  {						/* �ˡ��ե졼���褻�� */
    f = q8tk_frame_new( GET_LABEL( l, DATA_RESET_DIPSW_SET ) );
    q8tk_container_add( w, f );
    q8tk_widget_show( f );
  }
  {						/* ����˥ܥå�����褻�� */
    x = q8tk_vbox_new();
    q8tk_container_add( f, x );
    q8tk_widget_show( x );
							/* �ܥå����ˤ�     */
    {							/* DIPSW��˥塼 �� */
      q8tk_box_pack_start( x, dipsw_window );
    }
    {							/* ��λ�ܥ�������� */
      b = PACK_BUTTON( x,
		       GET_LABEL( l, DATA_RESET_DIPSW_QUIT ),
		       cb_reset_dipsw_end, NULL );

      q8tk_accel_group_add( dipsw_accel, Q8TK_KEY_ESC, b, "clicked" );
    }
  }

  q8tk_widget_show( w );
  q8tk_grab_add( w );

  q8tk_widget_grab_default( b );


  dipsw[ DIPSW_WIN   ] = w;	/* �����������Ĥ����Ȥ��������� */
  dipsw[ DIPSW_FRAME ] = f;	/* �������åȤ�Ф��Ƥ����ޤ�     */
  dipsw[ DIPSW_VBOX  ] = x;
  dipsw[ DIPSW_QUIT  ] = b;
}

/* �ǥ��åץ����å����ꥦ����ɥ��ξõ� */

static	void	dipsw_finish( void )
{
  q8tk_widget_destroy( dipsw[ DIPSW_QUIT ] );
  q8tk_widget_destroy( dipsw[ DIPSW_VBOX ] );
  q8tk_widget_destroy( dipsw[ DIPSW_FRAME ] );

  q8tk_grab_remove( dipsw[ DIPSW_WIN ] );
  q8tk_widget_destroy( dipsw[ DIPSW_WIN ] );
  q8tk_widget_destroy( dipsw_accel );
}





/*===========================================================================
 *
 *	�ᥤ��ڡ���	CPU����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						     /* CPU�����ڤ��ؤ� */
static	int	get_cpu_cpu( void )
{
  return cpu_timing;
}
static	void	cb_cpu_cpu( Q8tkWidget *dummy, void *p )
{
  cpu_timing = (int)p;
}


static	Q8tkWidget	*menu_cpu_cpu( void )
{
  Q8tkWidget	*vbox;

  vbox = PACK_VBOX( NULL );
  {						/* radio_button ... */
    PACK_RADIO_BUTTONS( vbox,
			data_cpu_cpu, COUNTOF(data_cpu_cpu),
			get_cpu_cpu(), cb_cpu_cpu );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
								/* ���� */
static	Q8tkWidget	*help_window;
static	Q8tkWidget	*help_widget[5];
static	Q8tkWidget	*help_string[40];
static	int		help_string_cnt;	
static	Q8tkWidget	*help_accel;

enum {
  HELP_WIN,
  HELP_VBOX,
  HELP_SWIN,
  HELP_BOARD,
  HELP_QUIT
};

static	void	help_finish( void );
static	void	cb_cpu_help_end( Q8tkWidget *dummy_0, void *dummy_1 )
{
  help_finish();
}

static	void	cb_cpu_help( Q8tkWidget *dummy_0, void *dummy_1 )
{
  Q8tkWidget	*w, *swin, *x, *b, *z;

  {						/* �ᥤ��Ȥʤ륦����ɥ� */
    w = q8tk_window_new( Q8TK_WINDOW_DIALOG );
    help_accel = q8tk_accel_group_new();
    q8tk_accel_group_attach( help_accel, w );
  }
  {						/* ����˥ܥå�����褻�� */
    x = q8tk_vbox_new();
    q8tk_container_add( w, x );
    q8tk_widget_show( x );
							/* �ܥå����ˤ�     */
    {							/* SCRL������ɥ��� */
      swin  = q8tk_scrolled_window_new( NULL, NULL );
      q8tk_widget_show( swin );
      q8tk_scrolled_window_set_policy( swin, Q8TK_POLICY_NEVER,
				       Q8TK_POLICY_AUTOMATIC );
      q8tk_misc_set_size( swin, 71, 20 );
      q8tk_box_pack_start( x, swin );
    }
    {							/* ��λ�ܥ�������� */
      b = PACK_BUTTON( x,
		       " O K ",
		       cb_cpu_help_end, NULL );
      q8tk_misc_set_placement( b, Q8TK_PLACEMENT_X_CENTER,
			       Q8TK_PLACEMENT_Y_CENTER );

      q8tk_accel_group_add( help_accel, Q8TK_KEY_ESC, b, "clicked" );
    }
  }

  {							/* SCRL������ɥ��� */
    int i;
    const char **s = (menu_lang==LANG_JAPAN) ? help_jp : help_en;
    z = q8tk_vbox_new();				/* VBOX���ä�     */
    q8tk_container_add( swin, z );
    q8tk_widget_show( z );

    for( i=0; i<COUNTOF(help_string); i++ ){		/* ������٥������ */
      if( s[i] == NULL ) break;
      help_string[i] = q8tk_label_new( s[i] );
      q8tk_widget_show( help_string[i] );
      q8tk_box_pack_start( z, help_string[i] );
    }
    help_string_cnt = i;
  }

  q8tk_widget_show( w );
  q8tk_grab_add( w );

  q8tk_widget_grab_default( b );


  help_widget[ HELP_WIN   ] = w;	/* �����������Ĥ����Ȥ��������� */
  help_widget[ HELP_VBOX  ] = x;	/* �������åȤ�Ф��Ƥ����ޤ�     */
  help_widget[ HELP_SWIN  ] = swin;
  help_widget[ HELP_BOARD ] = z;
  help_widget[ HELP_QUIT  ] = b;
}

/* �ǥ��åץ����å����ꥦ����ɥ��ξõ� */

static	void	help_finish( void )
{
  int i;
  for( i=0; i<help_string_cnt; i++ )
    q8tk_widget_destroy( help_string[ i ] );

  q8tk_widget_destroy( help_widget[ HELP_QUIT  ] );
  q8tk_widget_destroy( help_widget[ HELP_BOARD ] );
  q8tk_widget_destroy( help_widget[ HELP_SWIN  ] );
  q8tk_widget_destroy( help_widget[ HELP_VBOX  ] );

  q8tk_grab_remove( help_widget[ DIPSW_WIN ] );
  q8tk_widget_destroy( help_widget[ DIPSW_WIN ] );
  q8tk_widget_destroy( help_accel );
}



static	Q8tkWidget	*menu_cpu_help( void )
{
  Q8tkWidget	*button;
  const t_menulabel *l = data_cpu;

  button = PACK_BUTTON( NULL,
			GET_LABEL( l, DATA_CPU_HELP ),
			cb_cpu_help, NULL );
  q8tk_misc_set_placement( button, Q8TK_PLACEMENT_X_CENTER,
				   Q8TK_PLACEMENT_Y_CENTER );
  return button;
}

/*----------------------------------------------------------------------*/
						 /* CPU����å��ڤ��ؤ� */
static	Q8tkWidget	*cpu_clock_entry;

static	double	get_cpu_clock( void )
{
  return cpu_clock_mhz;
}
static	void	cb_cpu_clock( Q8tkWidget *dummy, void *p )
{
  int	cpumhz = (int)p;	/* -1:������ / 0:RET���� / 0<:���ꥯ��å� */
  char	buf[16], *conv_end;
  double clk;

  if( cpumhz <= 0 ){	/* ������ or ���ϴ�λ */
    strncpy( buf, q8tk_entry_get_text( cpu_clock_entry ), 15 );
    buf[15] = '\0';
  }else{		/* 4MHz or 8MHz�ܥ��󲡲� */
    sprintf( buf, "%8.4f", (double)cpumhz / 1000000.0 );
    q8tk_entry_set_text( cpu_clock_entry, buf );
  }

  clk = strtod( buf, &conv_end );
  if( *conv_end != '\0' ||			/* �Ѵ����顼 */
              clk < 0.1 || clk > 100.0 ){
    if( p==0 ){						/* RET���ϻ���ư���� */
      sprintf( buf, "%8.4f", CONST_4MHZ_CLOCK /*get_cpu_clock()*/ );
      q8tk_entry_set_text( cpu_clock_entry, buf );
    }	/* ʸ��������ϡ���ư�������ʤ��Τ����ġġĻ����ʤ��� */

  }else{					/* �Ѵ����� */
    cpu_clock_mhz = clk;
    interval_work_init_all();
  }
}


static	Q8tkWidget	*menu_cpu_clock( void )
{
  Q8tkWidget	*vbox, *hbox;
  Q8tkWidget	*l;
  const t_menudata *p = data_cpu_clock;
  char	buf[16];

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );
    {						/* entry, label, btn, btn */
      sprintf( buf, "%8.4f", get_cpu_clock() );
      cpu_clock_entry = PACK_ENTRY( hbox,
				    8, 0, buf,
				    cb_cpu_clock, (void *)0,
				    cb_cpu_clock, (void *)-1 );

      l = PACK_LABEL( hbox, GET_LABEL(p, DATA_CPU_CLOCK_MHZ) );
      q8tk_misc_set_placement( l, 0, Q8TK_PLACEMENT_Y_CENTER );

      PACK_BUTTON( hbox,
		   GET_LABEL(p, DATA_CPU_CLOCK_4MHZ),
		   cb_cpu_clock, (void *)(p[DATA_CPU_CLOCK_4MHZ].val));

      PACK_BUTTON( hbox,
		   GET_LABEL(p, DATA_CPU_CLOCK_8MHZ),
		   cb_cpu_clock, (void *)(p[DATA_CPU_CLOCK_8MHZ].val));
    }
						/* label */
    PACK_LABEL( vbox, GET_LABEL(p, DATA_CPU_CLOCK_INFO) );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
							/* ���������ѹ� */
static	Q8tkWidget	*cpu_wait_entry;

static	int	get_cpu_nowait( void )
{
  return no_wait;
}
static	void	cb_cpu_nowait( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
  no_wait = key;
}

static	int	get_cpu_wait( void )
{
  return wait_rate;
}
static	void	cb_cpu_wait( Q8tkWidget *dummy, void *p )
{
  /* (int)p ... -1:������ / 0:RET���� */

  char buf[16], *conv_end;
  int rate;

  strncpy( buf, q8tk_entry_get_text( cpu_wait_entry ), 15 );
  buf[15] = '\0';

  rate = strtoul( buf, &conv_end, 10 );
  if( *conv_end != '\0' ||			/* �Ѵ����顼 */
               rate < 5 || rate > 5000 ){
    if( p==0 ){					/* RET���ϻ���ư���� */
      sprintf( buf, "%d", 100 /*get_cpu_wait()*/ );
      q8tk_entry_set_text( cpu_wait_entry, buf );
    }	/* ʸ��������ϡ���ư�������ʤ��Τ����ġġĻ����ʤ��� */

  }else{					/* �Ѵ����� */
    wait_rate = rate;
    if( p==0 ){
      sprintf( buf, "%d", rate );
      q8tk_entry_set_text( cpu_wait_entry, buf );
    }
  }
}


static	Q8tkWidget	*menu_cpu_wait( void )
{
  Q8tkWidget	*vbox, *hbox;
  const t_menulabel *p = data_cpu_wait;
  char	buf[16];

  vbox = PACK_VBOX( NULL );
  {
    PACK_LABEL( vbox, "" );			/* ���� */

    hbox = PACK_HBOX( vbox );			/* label, entry, label */
    {
      PACK_LABEL( hbox, GET_LABEL( p, DATA_CPU_WAIT_RATE ) );

      sprintf( buf, "%d", get_cpu_wait() );
      cpu_wait_entry = PACK_ENTRY( hbox,
				   4, 5, buf,
				   cb_cpu_wait, (void *)0,
				   cb_cpu_wait, (void *)-1 );

      PACK_LABEL( hbox, GET_LABEL( p, DATA_CPU_WAIT_PERCENT ) );

      PACK_CHECK_BUTTON( hbox,			/* check_button */
			 GET_LABEL( p, DATA_CPU_WAIT_NOWAIT ),
			 get_cpu_nowait(),
			 cb_cpu_nowait, NULL );
    }

    PACK_LABEL( vbox, "" );			/* ���� */
						/* label */
    PACK_LABEL( vbox, GET_LABEL( p, DATA_CPU_WAIT_INFO ) );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
						      /* �Ƽ�������ѹ� */
static	int	get_cpu_misc( int type )
{
  switch( type ){
  case DATA_CPU_MISC_FDCWAIT:	return ( fdc_wait==0 ) ? FALSE : TRUE;
  case DATA_CPU_MISC_HSBASIC:	return highspeed_mode;
  case DATA_CPU_MISC_MEMWAIT:	return memory_wait;
  }
  return FALSE;
}
static	void	cb_cpu_misc( Q8tkWidget *widget, void *p )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  switch( (int)p ){
  case DATA_CPU_MISC_FDCWAIT: fdc_wait       = (key)? 1 : 0;        return;
  case DATA_CPU_MISC_HSBASIC: highspeed_mode = (key)? TRUE :FALSE;  return;
  case DATA_CPU_MISC_MEMWAIT: memory_wait    = (key)? TRUE :FALSE;  return;
  }
}


static	Q8tkWidget	*menu_cpu_misc( void )
{
  int	i;
  Q8tkWidget	*vbox, *l;
  const t_menudata *p = data_cpu_misc;

  vbox = PACK_VBOX( NULL );
  {						/* check_button, label ... */
    for( i=0; i<COUNTOF(data_cpu_misc); i++, p++ ){
      if( p->val >= 0 ){
	PACK_CHECK_BUTTON( vbox,
			   GET_LABEL( p, 0 ),
			   get_cpu_misc( p->val ),
			   cb_cpu_misc, (void *)( p->val ) );
      }else{
	l = PACK_LABEL( vbox, GET_LABEL( p, 0 ) );
	q8tk_misc_set_placement( l, Q8TK_PLACEMENT_X_RIGHT, 0 );
      }
    }
  }

  return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_cpu( void )
{
  Q8tkWidget *vbox, *hbox, *vbox2;
  Q8tkWidget *f;
  const t_menulabel *l = data_cpu;

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );
    {
      PACK_FRAME( hbox, GET_LABEL( l, DATA_CPU_CPU ), menu_cpu_cpu() );

      f = PACK_FRAME( hbox, "              ", menu_cpu_help() );
      q8tk_frame_set_shadow_type( f, Q8TK_SHADOW_NONE );
    }

    hbox = PACK_HBOX( vbox );
    {
      vbox2 = PACK_VBOX( hbox );
      {
	PACK_FRAME( vbox2, GET_LABEL( l, DATA_CPU_CLOCK ), menu_cpu_clock() );

	PACK_FRAME( vbox2, GET_LABEL( l, DATA_CPU_WAIT ), menu_cpu_wait() );
      }

      f = PACK_FRAME( hbox, "", menu_cpu_misc() );
      q8tk_frame_set_shadow_type( f, Q8TK_SHADOW_NONE );
    }
  }

  return vbox;
}








/*===========================================================================
 *
 *	�ᥤ��ڡ���	����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						  /* �ե졼��졼���ѹ� */
static	int	get_graph_frate( void )
{
  return	frameskip_rate;
}
static	void	cb_graph_frate( Q8tkWidget *widget, void *label )
{
  int	i;
  const t_menudata *p = data_graph_frate;
  const char       *combo_str = q8tk_combo_get_text(widget);
  char  str[32];

  for( i=0; i<COUNTOF(data_graph_frate); i++, p++ ){
    if( strcmp( p->str[menu_lang], combo_str )==0 ){
      sprintf( str, " fps (-frameskip %d)", p->val );
      q8tk_label_set( (Q8tkWidget*)label, str );

      frameskip_rate = p->val;
      blink_ctrl_update();
      return;
    }
  }
}
						/* thanks floi ! */
static	int	get_graph_autoskip( void )
{
  return use_auto_skip;
}
static	void	cb_graph_autoskip( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
  use_auto_skip = key;
}


static	Q8tkWidget	*menu_graph_frate( void )
{
  Q8tkWidget	*vbox, *hbox, *combo, *label;
  char		wk[32];

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );			/* combo, label */
    {
      label = q8tk_label_new(" fps");
      {
	sprintf( wk, "%6.3f", 60.0f / get_graph_frate() );
	combo = PACK_COMBO( hbox,
			    data_graph_frate, COUNTOF(data_graph_frate),
			    get_graph_frate(), wk, 6,
			    cb_graph_frate, label );
      }
      {
	q8tk_box_pack_start( hbox, label );
	q8tk_widget_show( label );
	cb_graph_frate( combo, (void*)label );
      }
    }

    PACK_LABEL( vbox, "" );			/* ���� */
							/* thanks floi ! */
    PACK_CHECK_BUTTON( vbox,			/* check_button */
		       GET_LABEL( data_graph_autoskip, 0 ),
		       get_graph_autoskip(),
		       cb_graph_autoskip, NULL );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
						  /* ���̥������ڤ��ؤ� */
static	int	get_graph_resize( void )
{
  return screen_size;
}
static	void	cb_graph_resize( Q8tkWidget *dummy, void *p )
{
  if( screen_size != (int)p &&
      (int)p < SCREEN_SIZE_END ){
    screen_size = (int)p;

    quasi88_change_screen();
  }
}
static	int	get_graph_fullscreen( void )
{
  return use_fullscreen;
}
static	void	cb_graph_fullscreen( Q8tkWidget *widget, void *dummy )
{
  use_fullscreen = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  quasi88_change_screen();
  q8tk_set_cursor( set_mouse_state() ? FALSE : TRUE );
}


static	Q8tkWidget	*menu_graph_resize( void )
{
  Q8tkWidget	*vbox;

  vbox = PACK_VBOX( NULL );
  {						/* radio_button .... */
    PACK_RADIO_BUTTONS( PACK_HBOX( vbox ),	
			data_graph_resize, COUNTOF(data_graph_resize),
			get_graph_resize(), cb_graph_resize );

    PACK_LABEL( vbox, "" );			/* ���� */

    PACK_CHECK_BUTTON( vbox,			/* check_button */
		       GET_LABEL( data_graph_fullscreen, 0 ),
		       get_graph_fullscreen(),
		       cb_graph_fullscreen, NULL );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
						      /* �Ƽ�������ѹ� */
static	int	get_graph_misc( int type )
{
  switch( type ){
  case DATA_GRAPH_MISC_HIDE_MOUSE: return hide_mouse;
  case DATA_GRAPH_MISC_15K:        return (monitor_15k==0x02) ? TRUE : FALSE;
  case DATA_GRAPH_MISC_DIGITAL:    return ( !monitor_analog ) ? TRUE : FALSE;
  case DATA_GRAPH_MISC_NOINTERP:   return ( !use_half_interp) ? TRUE : FALSE;
  }
  return FALSE;
}
static	void	cb_graph_misc( Q8tkWidget *widget, void *p )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  switch( (int)p ){
  case DATA_GRAPH_MISC_HIDE_MOUSE:hide_mouse     = key;                return;
  case DATA_GRAPH_MISC_15K:	  monitor_15k    = (key)? 0x02 : 0x00; return;
  case DATA_GRAPH_MISC_DIGITAL:   monitor_analog = (key)? FALSE: TRUE; return;

  case DATA_GRAPH_MISC_NOINTERP:  use_half_interp= (key)? FALSE: TRUE;
				  set_half_interp();		       return;
  }
}
static	int	get_graph_misc2( void )
{
  return use_interlace;
}
static	void	cb_graph_misc2( Q8tkWidget *button, void *p )
{
  use_interlace = (int)p;
}


static	Q8tkWidget	*menu_graph_misc( void )
{
  Q8tkWidget	*hbox, *vbox;

  hbox = PACK_HBOX( NULL );
  {
    PACK_LABEL( hbox, " " );

    vbox = PACK_VBOX( hbox );
    {						/* check_button ... */
      vbox = PACK_VBOX( vbox );
      PACK_CHECK_BUTTONS( vbox,
			  data_graph_misc, COUNTOF(data_graph_misc),
			  get_graph_misc, cb_graph_misc );
      PACK_LABEL( vbox, " " );
						/* radio_button ... */
      PACK_RADIO_BUTTONS( vbox,
			  data_graph_misc2, COUNTOF(data_graph_misc2),
			  get_graph_misc2(), cb_graph_misc2 );
    }

    PACK_LABEL( hbox, " " );
  }
  return hbox;
}

/*----------------------------------------------------------------------*/
						     /* PCG̵ͭ�ڤ��ؤ� */
static	int	get_graph_pcg( void )
{
  return use_pcg;
}
static	void	cb_graph_pcg( Q8tkWidget *dummy, void *p )
{
  use_pcg = (int)p;
  memory_set_font();
}


static	Q8tkWidget	*menu_graph_pcg( void )
{
  Q8tkWidget	*hbox;

  hbox = PACK_HBOX( NULL );
  {
    PACK_RADIO_BUTTONS( hbox,
			data_graph_pcg, COUNTOF(data_graph_pcg),
			get_graph_pcg(), cb_graph_pcg );
  }

  return hbox;
}

/*----------------------------------------------------------------------*/
						    /* �ե�����ڤ��ؤ� */
static	int	get_graph_font( void )
{
  return font_type;
}
static	void	cb_graph_font( Q8tkWidget *button, void *p )
{
  font_type = (int)p;
  memory_set_font();
}


static	Q8tkWidget	*menu_graph_font( void )
{
  Q8tkWidget	*vbox;
  t_menudata data_graph_font[3];

  data_graph_font[0] = data_graph_font1[ (font_loaded & 1) ? 1 : 0 ];
  data_graph_font[1] = data_graph_font2[ (font_loaded & 2) ? 1 : 0 ];
  data_graph_font[2] = data_graph_font3[ (font_loaded & 4) ? 1 : 0 ];

  vbox = PACK_VBOX( NULL );
  {						/* radio_button ... */
    PACK_RADIO_BUTTONS( vbox,
			data_graph_font, COUNTOF(data_graph_font),
			get_graph_font(), cb_graph_font );
  }

  return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_graph( void )
{
  Q8tkWidget *vbox, *hbox, *vbox2;
  Q8tkWidget *f;
  const t_menulabel *l = data_graph;

  vbox = PACK_VBOX( NULL );
  {
    PACK_FRAME( vbox, GET_LABEL( l, DATA_GRAPH_FRATE ), menu_graph_frate() );

    PACK_FRAME( vbox, GET_LABEL( l, DATA_GRAPH_RESIZE ), menu_graph_resize() );

    hbox = PACK_HBOX( vbox );
    {
      q8tk_box_pack_start( hbox, menu_graph_misc() ); /*�ե졼��ˤϾ褻�ʤ�*/

      vbox2 = PACK_VBOX( hbox );
      {
	PACK_FRAME( vbox2, GET_LABEL( l, DATA_GRAPH_PCG  ), menu_graph_pcg() );
	PACK_FRAME( vbox2, GET_LABEL( l, DATA_GRAPH_FONT ), menu_graph_font());
      }
    }
  }

  return vbox;
}








/*===========================================================================
 *
 *	�ᥤ��ڡ���	����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						      /* �ܥ�塼���ѹ� */
static	int	get_volume( int type )
{
  switch( type ){
  case VOL_TOTAL:  return  xmame_get_sound_volume();
  case VOL_FM:     return  xmame_get_mixer_volume( XMAME_MIXER_FM );
  case VOL_PSG:    return  xmame_get_mixer_volume( XMAME_MIXER_PSG );
  case VOL_BEEP:   return  xmame_get_mixer_volume( XMAME_MIXER_BEEP );
  case VOL_RHYTHM: return  xmame_get_mixer_volume( XMAME_MIXER_RHYTHM );
  case VOL_ADPCM:  return  xmame_get_mixer_volume( XMAME_MIXER_ADPCM );
  case VOL_FMPSG:  return  xmame_get_mixer_volume( XMAME_MIXER_FMPSG );
  }
  return 0;
}
static	void	cb_volume( Q8tkWidget *widget, void *p )
{
  int	vol = Q8TK_ADJUSTMENT( widget )->value;

  switch( (int)p ){
  case VOL_TOTAL:   xmame_set_sound_volume( vol );			break;
  case VOL_FM:      xmame_set_mixer_volume( XMAME_MIXER_FM, vol );	break;
  case VOL_PSG:     xmame_set_mixer_volume( XMAME_MIXER_PSG, vol );	break;
  case VOL_BEEP:    xmame_set_mixer_volume( XMAME_MIXER_BEEP, vol );	break;
  case VOL_RHYTHM:  xmame_set_mixer_volume( XMAME_MIXER_RHYTHM, vol );	break;
  case VOL_ADPCM:   xmame_set_mixer_volume( XMAME_MIXER_ADPCM, vol );	break;
  case VOL_FMPSG:   xmame_set_mixer_volume( XMAME_MIXER_FMPSG, vol );	break;
  }
}


INLINE	Q8tkWidget	*menu_volume_unit( const t_volume *p, int count )
{
  int	i;
  Q8tkWidget	*vbox, *hbox;

  vbox = PACK_VBOX( NULL );
  {
    for( i=0; i<count; i++, p++ ){

      hbox = PACK_HBOX( vbox );
      {
	PACK_LABEL( hbox, GET_LABEL( p, 0 ) );

	PACK_HSCALE( hbox,
		     p,
		     get_volume( p->val ),
		     cb_volume, (void*)( p->val ) );
      }
    }
  }

  return vbox;
}


static	Q8tkWidget	*menu_volume_total( void )
{
  return menu_volume_unit( data_volume_total, COUNTOF(data_volume_total) );
}
static	Q8tkWidget	*menu_volume_level( void )
{
  return menu_volume_unit( data_volume_level, COUNTOF(data_volume_level) );
}
static	Q8tkWidget	*menu_volume_rhythm( void )
{
  return menu_volume_unit( data_volume_rhythm, COUNTOF(data_volume_rhythm) );
}
static	Q8tkWidget	*menu_volume_fmgen( void )
{
  return menu_volume_unit( data_volume_fmgen, COUNTOF(data_volume_fmgen) );
}

/*----------------------------------------------------------------------*/
					    /* ������ɤʤ�����å����� */

static	Q8tkWidget	*menu_volume_no_available( void )
{
  int type;
  Q8tkWidget	*l;

#ifdef	USE_SOUND
  type = 2;
#else
  type = 0;
#endif

  if( sound_board==SOUND_II ){
    type |= 1;
  }

  if( 0 <= type && type <= 3 ){
    l = q8tk_label_new( GET_LABEL( data_volume_no, type ) );
  }else{
    l = q8tk_label_new( "???" );
  }

  q8tk_widget_show( l );

  return l;
}

/*----------------------------------------------------------------------*/
					    /* ������ɥɥ饤�м���ɽ�� */

static	Q8tkWidget	*menu_volume_type( void )
{
  int type;
  Q8tkWidget	*l;

#if	defined(USE_SOUND) && defined(USE_FMGEN)
  if( use_fmgen ){
    type = 2;
  }else
#endif
#if	defined(USE_SOUND)
  {
    type = 0;
  }
#else
  type = -1;
#endif

  if( sound_board==SOUND_II ){
    type |= 1;
  }

  if( 0 <= type && type <= 3 ){
    l = q8tk_label_new( GET_LABEL( data_volume_type, type ) );
  }else{
    l = q8tk_label_new( "???" );
  }

  q8tk_widget_show( l );

  return l;
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_volume( void )
{
  Q8tkWidget *vbox;
  Q8tkWidget *w;
  const t_menulabel *l = data_volume;

  if( xmame_sound_is_enable()==FALSE ){

    w = PACK_FRAME( NULL, "", menu_volume_no_available() );
    q8tk_frame_set_shadow_type( w, Q8TK_SHADOW_ETCHED_OUT );

    return w;
  }

  vbox = PACK_VBOX( NULL );
  {
    w = PACK_FRAME( vbox, "", menu_volume_type() );
    q8tk_frame_set_shadow_type( w, Q8TK_SHADOW_ETCHED_OUT );


    PACK_FRAME( vbox, GET_LABEL( l, DATA_VOLUME_TOTAL ), menu_volume_total() );

#if	defined(USE_SOUND) && defined(USE_FMGEN)

    if( use_fmgen ) w = menu_volume_fmgen();
    else            w = menu_volume_level();
    PACK_FRAME( vbox, GET_LABEL( l, DATA_VOLUME_LEVEL ), w );

    if( use_fmgen ){
      ;
    }else if( sound_board==SOUND_II ){
      PACK_FRAME( vbox, GET_LABEL( l, DATA_VOLUME_SD2 ), menu_volume_rhythm());
    }

#else

    PACK_FRAME( vbox, GET_LABEL( l, DATA_VOLUME_LEVEL ), menu_volume_level() );

    if( sound_board==SOUND_II ){
      PACK_FRAME( vbox, GET_LABEL( l, DATA_VOLUME_SD2 ), menu_volume_rhythm());
    }

#endif
  }

  return vbox;
}




/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ǥ�����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/

typedef struct{
  Q8tkWidget	*list;			/* ���᡼�������Υꥹ��	*/
  Q8tkWidget	*button[2];		/* �ܥ����		*/
  Q8tkWidget	*label[2];		/* ���Υ�٥� (2��)	*/
  int		func[2];		/* �ܥ���ε�ǽ IMG_xxx	*/
  Q8tkWidget	*stat_label;		/* ���� - Busy/Ready	*/
  Q8tkWidget	*attr_label;		/* ���� - RO/RW°��	*/
  Q8tkWidget	*num_label;		/* ���� - ���᡼����	*/
} T_DISK_INFO;

static	T_DISK_INFO	disk_info[2];	/* 2�ɥ饤��ʬ�Υ��	*/

static	char		disk_filename[ QUASI88_MAX_FILENAME ];

static	int		disk_drv;	/* ����ɥ饤�֤��ֹ� */
static	int		disk_img;	/* ���륤�᡼�����ֹ� */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

static	void	set_disk_widget( void );


/* BOOT from DISK �ǡ�DISK �� CLOSE �������䡢
   BOOT from ROM  �ǡ�DISK �� OPEN �������ϡ� DIP-SW ��������ѹ� */
static	void	disk_update_dipsw_b_boot( void )
{
  if( disk_image_exist(0) || disk_image_exist(1) ){
    q8tk_toggle_button_set_state( widget_dipsw_b_boot_disk, TRUE );
  }else{
    q8tk_toggle_button_set_state( widget_dipsw_b_boot_rom,  TRUE );
  }
  set_reset_dipsw_boot();

  /* �ꥻ�åȤ��ʤ��ǥ�˥塼�⡼�ɤ�ȴ��������꤬��¸����ʤ��Τǡ����� */
  boot_from_rom = menu_boot_from_rom;			/* thanks floi ! */
}


/*----------------------------------------------------------------------*/
/* °���ѹ��γƼ����							*/

enum {
  ATTR_RENAME,		/* drive[drv] �Υ��᡼�� img ���͡���		*/
  ATTR_PROTECT,		/* drive[drv] �Υ��᡼�� img ��ץ�ƥ���	*/
  ATTR_FORMAT,		/* drive[drv] �Υ��᡼�� img �򥢥�ե����ޥå�	*/
  ATTR_UNFORMAT,	/* drive[drv] �Υ��᡼�� img ��ե����ޥå�	*/
  ATTR_APPEND,		/* drive[drv] �˺Ǹ�˥��᡼�����ɲ�		*/
  ATTR_CREATE		/* �����˥ǥ��������᡼���ե���������		*/
};

static	void	sub_disk_attr_file_ctrl( int drv, int img, int cmd, char *c )
{
  int	ro = FALSE;
  int	result = -1;
  OSD_FILE *fp;


  if( cmd != ATTR_CREATE ){		/* �ɥ饤�֤Υե�������ѹ������� */

    fp = drive[ drv ].fp;			/* ���Υե�����ݥ��󥿤����*/
    if( drive[ drv ].read_only ){
      ro = TRUE;
    }

  }else{				/* �̤Υե�����򹹿������� */

    fp = osd_fopen( FTYPE_DISK, c, "r+b" );		/* "r+b" �ǥ����ץ� */
    if( fp == NULL ){
      fp = osd_fopen( FTYPE_DISK, c, "rb" );		/* "rb" �ǥ����ץ� */
      if( fp ) ro = TRUE;
    }

    if( fp ){						/* �����ץ�Ǥ����� */
      if     ( fp == drive[ 0 ].fp ) drv = 0;		/* ���Ǥ˥ɥ饤�֤� */
      else if( fp == drive[ 1 ].fp ) drv = 1;		/* �����Ƥʤ�����   */
      else                           drv = -1;		/* �����å�����     */
    }
    else{						/* �����ץ�Ǥ��ʤ� */
      fp = osd_fopen( FTYPE_DISK, c, "ab" );		/* ���ϡ������˺��� */
      drv = -1;
    }

  }


  if( fp == NULL ){			/* �����ץ��� */
    start_file_error_dialog( drv, ERR_CANT_OPEN );
    return;
  }
  else if( ro ){			/* �꡼�ɥ���꡼�ʤΤǽ����Բ� */
    if( drv < 0 ) osd_fclose( fp );
    if( cmd != ATTR_CREATE ) start_file_error_dialog( drv, ERR_READ_ONLY );
    else                     start_file_error_dialog(  -1, ERR_READ_ONLY );
    return;
  }
  else if( drv>=0 &&			/* ���줿���᡼�����ޤޤ��Τ��Բ� */
	   drive[ drv ].detect_broken_image ){
    start_file_error_dialog( drv, ERR_MAYBE_BROKEN );
    return;
  }


#if 0
  if( cmd==ATTR_CREATE || cmd==ATTR_APPEND ){
    /* ���ν����˻��֤�������褦�ʾ�硢��å�������������� */
    /* ���ν���������ʤ˻��֤������뤳�ȤϤʤ����� */
  }
#endif

		/* �������ե�������Ф��ơ����� */

  switch( cmd ){
  case ATTR_RENAME:	result = d88_write_name( fp, drv, img, c );	break;
  case ATTR_PROTECT:	result = d88_write_protect( fp, drv, img, c );	break;
  case ATTR_FORMAT:	result = d88_write_format( fp, drv, img );	break;
  case ATTR_UNFORMAT:	result = d88_write_unformat( fp, drv, img );	break;
  case ATTR_APPEND:
  case ATTR_CREATE:	result = d88_append_blank( fp, drv );		break;
  }

		/* ���η�� */

  switch( result ){
  case D88_SUCCESS:	result = ERR_NO;			break;
  case D88_NO_IMAGE:	result = ERR_MAYBE_BROKEN;		break;
  case D88_BAD_IMAGE:	result = ERR_MAYBE_BROKEN;		break;
  case D88_ERR_READ:	result = ERR_MAYBE_BROKEN;		break;
  case D88_ERR_SEEK:	result = ERR_SEEK;			break;
  case D88_ERR_WRITE:	result = ERR_WRITE;			break;
  default:		result = ERR_UNEXPECTED;		break;
  }

		/* ��λ�������ʤ������顼���ϥ�å�������Ф� */

  if( drv < 0 ){		/* ���������ץ󤷤��ե�����򹹿�������� */
    osd_fclose( fp );			/* �ե�������Ĥ��ƽ����	  */

  }else{			/* �ɥ饤�֤Υե�����򹹿��������	  */
    if( result == ERR_NO ){		/* ��˥塼���̤򹹿����ͤ�	  */
      set_disk_widget();
      if( cmd != ATTR_CREATE ) disk_update_dipsw_b_boot();
    }
  }

  if( result != ERR_NO ){
    start_file_error_dialog( drv, result );
  }

  return;
}

/*----------------------------------------------------------------------*/
/* �֥�͡���ץ�������						*/

static	void	cb_disk_attr_rename_activate( Q8tkWidget *dummy, void *p )
{
  char	wk[16 + 1];

  if( (int)p ){			/* dialog_destroy() �����˥���ȥ�򥲥å� */
    strncpy( wk, dialog_get_entry(), 16 );
    wk[16] = '\0';
  }

  dialog_destroy();

  if( (int)p ){
    sub_disk_attr_file_ctrl( disk_drv, disk_img, ATTR_RENAME, wk );
  }
}
static	void	sub_disk_attr_rename( const char *image_name )
{
  int save_code;
  const t_menulabel *l = data_disk_attr_rename;


  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_RENAME_TITLE1 +disk_drv ) );

    save_code = q8tk_set_kanjicode( Q8TK_KANJI_SJIS );
    dialog_set_title( image_name );
    q8tk_set_kanjicode( save_code );

    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_RENAME_TITLE2 ) );

    dialog_set_separator();

    save_code = q8tk_set_kanjicode( Q8TK_KANJI_SJIS );
    dialog_set_entry( drive[disk_drv].image[disk_img].name,
		      16,
		      cb_disk_attr_rename_activate, (void*)TRUE );
    q8tk_set_kanjicode( save_code );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_RENAME_OK ),
		       cb_disk_attr_rename_activate, (void*)TRUE );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_RENAME_CANCEL ),
		       cb_disk_attr_rename_activate, (void*)FALSE );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*----------------------------------------------------------------------*/
/* �֥ץ�ƥ��ȡץ�������						*/

static	void	cb_disk_attr_protect_clicked( Q8tkWidget *dummy, void *p )
{
  char	c;

  dialog_destroy();

  if( (int)p ){
    if( (int)p == 1 ) c = DISK_PROTECT_TRUE;
    else              c = DISK_PROTECT_FALSE;

    sub_disk_attr_file_ctrl( disk_drv, disk_img, ATTR_PROTECT, &c );
  }
}
static	void	sub_disk_attr_protect( const char *image_name )
{
  int save_code;
  const t_menulabel *l = data_disk_attr_protect;

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_PROTECT_TITLE1 +disk_drv ));

    save_code = q8tk_set_kanjicode( Q8TK_KANJI_SJIS );
    dialog_set_title( image_name );
    q8tk_set_kanjicode( save_code );

    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_PROTECT_TITLE2 ) );

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_PROTECT_SET ),
		       cb_disk_attr_protect_clicked, (void*)1 );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_PROTECT_UNSET ),
		       cb_disk_attr_protect_clicked, (void*)2 );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_PROTECT_CANCEL ),
		       cb_disk_attr_protect_clicked, (void*)0 );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*----------------------------------------------------------------------*/
/* �֥ե����ޥåȡץ�������						*/

static	void	cb_disk_attr_format_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    if( (int)p == 1 )
      sub_disk_attr_file_ctrl( disk_drv, disk_img, ATTR_FORMAT,   NULL );
    else 
      sub_disk_attr_file_ctrl( disk_drv, disk_img, ATTR_UNFORMAT, NULL );
  }
}
static	void	sub_disk_attr_format( const char *image_name )
{
  int save_code;
  const t_menulabel *l = data_disk_attr_format;

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_FORMAT_TITLE1 +disk_drv ) );

    save_code = q8tk_set_kanjicode( Q8TK_KANJI_SJIS );
    dialog_set_title( image_name );
    q8tk_set_kanjicode( save_code );

    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_FORMAT_TITLE2 ) );

    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_FORMAT_WARNING ) );

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_FORMAT_DO ),
		       cb_disk_attr_format_clicked, (void*)1 );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_FORMAT_NOT ),
		       cb_disk_attr_format_clicked, (void*)2 );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_FORMAT_CANCEL ),
		       cb_disk_attr_format_clicked, (void*)0 );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*----------------------------------------------------------------------*/
/* �֥֥�󥯥ǥ������ץ�������					*/

static	void	cb_disk_attr_blank_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    sub_disk_attr_file_ctrl( disk_drv, disk_img, ATTR_APPEND, NULL );
  }
}
static	void	sub_disk_attr_blank( void )
{
  const t_menulabel *l = data_disk_attr_blank;

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_BLANK_TITLE1 +disk_drv ) );

    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_BLANK_TITLE2 ) );

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_BLANK_OK ),
		       cb_disk_attr_blank_clicked, (void*)TRUE );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_BLANK_CANCEL ),
		       cb_disk_attr_blank_clicked, (void*)FALSE );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*----------------------------------------------------------------------*/
/* ��°���ѹ��� �ܥ��󲡲����ν���  -  �ܺ�����Υ��������򳫤�	*/

static char disk_attr_image_name[20];
static	void	cb_disk_attr_clicked( Q8tkWidget *dummy, void *p )
{
  char *name = disk_attr_image_name;

  dialog_destroy();

  switch( (int)p ){
  case DATA_DISK_ATTR_RENAME:	sub_disk_attr_rename( name );	break;
  case DATA_DISK_ATTR_PROTECT:	sub_disk_attr_protect( name );	break;
  case DATA_DISK_ATTR_FORMAT:	sub_disk_attr_format( name );	break;
  case DATA_DISK_ATTR_BLANK:	sub_disk_attr_blank();		break;
  }
}


static void sub_disk_attr( void )
{
  int save_code;
  const t_menulabel *l = data_disk_attr;

  sprintf( disk_attr_image_name,		/* ���᡼��̾�򥻥å� */
	   "\"%-16s\"", drive[disk_drv].image[disk_img].name );

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_TITLE1 +disk_drv ) );

    save_code = q8tk_set_kanjicode( Q8TK_KANJI_SJIS );
    dialog_set_title( disk_attr_image_name );
    q8tk_set_kanjicode( save_code );

    dialog_set_title( GET_LABEL( l, DATA_DISK_ATTR_TITLE2 ) );

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_RENAME ),
		       cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_RENAME );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_PROTECT ),
		       cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_PROTECT );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_FORMAT ),
		       cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_FORMAT );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_BLANK ),
		       cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_BLANK );

    dialog_set_button( GET_LABEL( l, DATA_DISK_ATTR_CANCEL ),
		       cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_CANCEL );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}




/*----------------------------------------------------------------------*/
/* �֥��᡼���ե�����򳫤��� �ܥ��󲡲����ν���			*/

static	int	disk_open_ro;
static	int	disk_open_cmd;
static void sub_disk_open_ok( void );

static void sub_disk_open( int cmd )
{
  const char *initial;
  int	num;
  const t_menulabel *l = (disk_drv == 0) ? data_disk_open_drv1
					 : data_disk_open_drv2;

  disk_open_cmd = cmd;
  num = (cmd==IMG_OPEN) ? DATA_DISK_OPEN_OPEN : DATA_DISK_OPEN_BOTH;


  if     ( file_disk[disk_drv  ][0] != '\0' ) initial = file_disk[disk_drv  ];
  else if( file_disk[disk_drv^1][0] != '\0' ) initial = file_disk[disk_drv^1];
  else{
    initial = osd_dir_disk();
    if( initial==NULL ) initial = osd_dir_cwd();
  }


  START_FILE_SELECTION( GET_LABEL( l, num ),
			(menu_readonly) ? 1 : 0,    /* ReadOnly �����򤬲� */
			initial,

			sub_disk_open_ok,
			disk_filename,
			QUASI88_MAX_FILENAME,
			&disk_open_ro );
}

static void sub_disk_open_ok( void )
{
  if( disk_open_cmd == IMG_OPEN ){

    if( quasi88_disk_insert( disk_drv, disk_filename, 0, disk_open_ro )
	== FALSE ){
      start_file_error_dialog( disk_drv, ERR_CANT_OPEN );
    }
    else{
      if( disk_same_file() ){		/* ȿ��¦��Ʊ���ե�������ä���� */
	int dst = disk_drv;
	int src = disk_drv^1;
	int img;

	if( drive[ src ].empty ){		/* ȿ��¦�ɥ饤�� ���ʤ� */
	  img = 0;				/*        �ǽ�Υ��᡼�� */
	}else{
	  if( disk_image_num( src ) == 1 ){	/* ���᡼����1�Ĥξ��� */
	    img = -1;				/*        �ɥ饤�� ����  */

	  }else{				/* ���᡼����ʣ�������  */
						/*        ��(��)���᡼�� */
	    img = disk_image_selected( src ) + ((dst==DRIVE_1) ? -1 : +1);
	    if( (img < 0) || 
		(disk_image_num( dst ) -1 < img) ) img = -1;
	  }
	}
	if( img < 0 ) drive_set_empty( dst );
	else          disk_change_image( dst, img );
      }
    }

  }else{	/*   IMG_BOTH */

    if( quasi88_disk_insert_all( disk_filename, disk_open_ro ) == FALSE ){

      disk_drv = 0;
      start_file_error_dialog( disk_drv, ERR_CANT_OPEN );

    }

  }

  if( filename_synchronize ){
    sub_misc_suspend_change();
    sub_misc_snapshot_change();
  }
  set_disk_widget();
  disk_update_dipsw_b_boot();
}

/*----------------------------------------------------------------------*/
/* �֥��᡼���ե�������Ĥ���� �ܥ��󲡲����ν���			*/

static void sub_disk_close( void )
{
  quasi88_disk_eject( disk_drv );

  if( filename_synchronize ){
    sub_misc_suspend_change();
    sub_misc_snapshot_change();
  }
  set_disk_widget();
  disk_update_dipsw_b_boot();
}

/*----------------------------------------------------------------------*/
/* ��ȿ�Хɥ饤�֤�Ʊ���ե�����򳫤��� �ܥ��󲡲����ν���		*/

static void sub_disk_copy( void )
{
  int	dst = disk_drv;
  int	src = disk_drv^1;
  int	img;

  if( ! disk_image_exist( src ) ) return;

  if( drive[ src ].empty ){			/* ȿ��¦�ɥ饤�� ���ʤ� */
    img = 0;					/*        �ǽ�Υ��᡼�� */
  }else{
    if( disk_image_num( src ) == 1 ){		/* ���᡼����1�Ĥξ��� */
      img = -1;					/*        �ɥ饤�� ����  */

    }else{					/* ���᡼����ʣ�������  */
						/*        ��(��)���᡼�� */
      img = disk_image_selected( src ) + ((dst==DRIVE_1) ? -1 : +1);
      if( (img < 0) || 
	  (disk_image_num( dst ) -1 < img) ) img = -1;
    }
  }

  if( quasi88_disk_insert_A_to_B( src, dst, img ) == FALSE ){
    start_file_error_dialog( disk_drv, ERR_CANT_OPEN );
  }

  if( filename_synchronize ){
    sub_misc_suspend_change();
    sub_misc_snapshot_change();
  }
  set_disk_widget();
  disk_update_dipsw_b_boot();
}





/*----------------------------------------------------------------------*/
/* ���᡼���Υꥹ�ȥ����ƥ�������Ρ�������Хå��ؿ�			*/

static	void	cb_disk_image( Q8tkWidget *dummy, void *p )
{
  int	drv = ( (int)p ) & 0xff;
  int	img = ( (int)p ) >> 8;

  if( img < 0 ){			/* img==-1 �� <<�ʤ�>> */
    drive_set_empty( drv );
  }else{				/* img>=0 �ʤ� ���᡼���ֹ� */
    drive_unset_empty( drv );
    disk_change_image( drv, img );
  }
}

/*----------------------------------------------------------------------*/
/* �ɥ饤�����¸�ߤ���ܥ���Ρ�������Хå��ؿ�			*/

static	void	cb_disk_button( Q8tkWidget *dummy, void *p )
{
  int	drv    = ( (int)p ) & 0xff;
  int	button = ( (int)p ) >> 8;

  disk_drv = drv;
  disk_img = disk_image_selected( drv );

  switch( disk_info[drv].func[button] ){
  case IMG_OPEN:
  case IMG_BOTH:
    sub_disk_open( disk_info[drv].func[button] );
    break;
  case IMG_CLOSE:
    sub_disk_close();
    break;
  case IMG_COPY:
    sub_disk_copy();
    break;
  case IMG_ATTR:
    if( ! drive_check_empty( drv ) ){	     /* ���᡼��<<�ʤ�>>�������̵�� */
      sub_disk_attr();
    }
    break;
  }
}

/*----------------------------------------------------------------------*/
/* �ե�����򳫤���ˡ�disk_info[] �˾���򥻥å�			*/
/*		(���᡼���Υꥹ���������ܥ��󡦾���Υ�٥�򥻥å�)	*/

static	void	set_disk_widget( void )
{
  int	i, drv, save_code;
  Q8tkWidget *item;
  T_DISK_INFO	*w;
  const t_menulabel *inf = data_disk_info;
  const t_menulabel *l   = data_disk_image;
  const t_menulabel *btn;
  char	wk[40], wk2[20];
  const char *s;


  for( drv=0; drv<2; drv++ ){
    w   = &disk_info[drv];

    if( menu_swapdrv ){
      btn = (drv==0) ? data_disk_button_drv1swap : data_disk_button_drv2swap;
    }else{
      btn = (drv==0) ? data_disk_button_drv1 : data_disk_button_drv2;
    }

		/* ���᡼��̾�� LIST ITEM ���� */

    q8tk_listbox_clear_items( w->list, 0, -1 );

    item = q8tk_list_item_new_with_label( GET_LABEL(l, DATA_DISK_IMAGE_EMPTY));
    q8tk_widget_show( item );
    q8tk_container_add( w->list, item );		/* <<�ʤ�>> ITEM */
    q8tk_signal_connect( item, "select",
			 cb_disk_image, (void *)( (-1 <<8) +drv ) );

    if( disk_image_exist( drv ) ){		/* ---- �ǥ����������� ---- */
      save_code = q8tk_set_kanjicode( Q8TK_KANJI_SJIS );
      {
	for( i=0; i<disk_image_num(drv); i++ ){
	  sprintf( wk, "%3d  %-16s  %s ",	/* ���᡼��No ���᡼��̾ RW */
		   i+1,
		   drive[drv].image[i].name,
		   (drive[drv].image[i].protect) ? "RO" : "RW" );

	  item = q8tk_list_item_new_with_label( wk );
	  q8tk_widget_show( item );
	  q8tk_container_add( w->list, item );
	  q8tk_signal_connect( item, "select",
			       cb_disk_image, (void *)( (i<<8) +drv ) );
	}
      }
      q8tk_set_kanjicode( save_code );

				/* <<�ʤ�>> or ����image �� ITEM �򥻥쥯�� */
      if( drive_check_empty( drv ) ) i = 0;
      else                           i = disk_image_selected( drv ) + 1;
      q8tk_listbox_select_item( w->list, i );

    }else{					/* ---- �ɥ饤�ֶ��ä� ---- */
      q8tk_listbox_select_item( w->list, 0 );		    /* <<�ʤ�>> ITEM */
    }

		/* �ܥ���ε�ǽ ���Ĥ���ס�°���ѹ��� / �ֳ����סֳ����� */

    if( disk_image_exist( drv ) ){
      w->func[0] = IMG_CLOSE;
      w->func[1] = IMG_ATTR;
    }else{
      w->func[0] = ( disk_image_exist( drv^1 ) ) ? IMG_COPY : IMG_BOTH;
      w->func[1] = IMG_OPEN;
    }
    q8tk_label_set( w->label[0], GET_LABEL( btn, w->func[0] ) );
    q8tk_label_set( w->label[1], GET_LABEL( btn, w->func[1] ) );

		/* ���� - Busy/Ready */

    if( get_drive_ready(drv) ) s = GET_LABEL( inf, DATA_DISK_INFO_STAT_READY );
    else                       s = GET_LABEL( inf, DATA_DISK_INFO_STAT_BUSY  );
    q8tk_label_set( w->stat_label, s );
    q8tk_label_set_reverse( w->stat_label,	/* BUSY�ʤ�ȿžɽ�� */
			  ( get_drive_ready(drv) ) ? FALSE : TRUE );

		/* ���� - RO/RW°�� */

    if( disk_image_exist( drv ) ){
      if( drive[drv].read_only ) s = GET_LABEL( inf, DATA_DISK_INFO_ATTR_RO );
      else                       s = GET_LABEL( inf, DATA_DISK_INFO_ATTR_RW );
    }else{
      s = "";
    }
    q8tk_label_set( w->attr_label, s );
    q8tk_label_set_color( w->attr_label,	/* ReadOnly�ʤ��ֿ�ɽ�� */
			  ( drive[drv].read_only ) ? Q8GR_PALETTE_RED : -1 );

		/* ���� - ���᡼���� */

    if( disk_image_exist( drv ) ){
      if( drive[drv].detect_broken_image ){		/* ��»���� */
	s = GET_LABEL( inf, DATA_DISK_INFO_NR_BROKEN );
      }else
      if( drive[drv].over_image ||			/* ���᡼��¿�᤮ */
	  disk_image_num( drv ) > 99 ){
	s = GET_LABEL( inf, DATA_DISK_INFO_NR_OVER );
      }else{
	s = "";
      }
      sprintf( wk, "%2d%s",
	       (disk_image_num(drv)>99) ? 99 : disk_image_num(drv), s );
      sprintf( wk2, "%9.9s", wk );			/* 9ʸ�����ͤ���Ѵ� */
    }else{
      wk2[0] = '\0';
    }
    q8tk_label_set( w->num_label,  wk2 );
  }
}


/*----------------------------------------------------------------------*/
/* �֥֥�󥯺����� �ܥ��󲡲����ν���					*/

static	void	sub_disk_blank_ok( void );
static	void	cb_disk_blank_warn_clicked( Q8tkWidget *, void * );


static	void	cb_disk_blank( Q8tkWidget *dummy_0, void *dummy_1 )
{
  const char *initial;
  const t_menulabel *l = data_disk_blank;

  if     ( file_disk[DRIVE_1][0] != '\0' ) initial = file_disk[DRIVE_1];
  else if( file_disk[DRIVE_2][0] != '\0' ) initial = file_disk[DRIVE_2];
  else{
    initial = osd_dir_disk();
    if( initial==NULL ) initial = osd_dir_cwd();
  }


  START_FILE_SELECTION( GET_LABEL( l, DATA_DISK_BLANK_FSEL ),
			-1,	/* ReadOnly ��������Բ� */
			initial,

			sub_disk_blank_ok,
			disk_filename,
			QUASI88_MAX_FILENAME,
			NULL );
}

static	void	sub_disk_blank_ok( void )
{
  const t_menulabel *l = data_disk_blank;

  switch( osd_file_stat( disk_filename ) ){

  case FILE_STAT_NOEXIST:
    /* �ե�����򿷵��˺��������֥�󥯤���� */
    sub_disk_attr_file_ctrl( 0, 0, ATTR_CREATE, disk_filename );
    break;

  case FILE_STAT_DIR:
    /* �ǥ��쥯�ȥ�ʤΤǡ��֥�󥯤��ɲäǤ��ʤ� */
    start_file_error_dialog( -1, ERR_CANT_OPEN );
    break;

  default:
    /* ���Ǥ˥ե����뤬¸�ߤ��ޤ����֥�󥯤��ɲä��ޤ����� */
    dialog_create();
    {
      dialog_set_title( GET_LABEL( l, DATA_DISK_BLANK_WARN_0 ) );

      dialog_set_title( GET_LABEL( l, DATA_DISK_BLANK_WARN_1 ) );

      dialog_set_separator();

      dialog_set_button( GET_LABEL( l, DATA_DISK_BLANK_WARN_APPEND ),
			 cb_disk_blank_warn_clicked, (void*)TRUE );

      dialog_set_button( GET_LABEL( l, DATA_DISK_BLANK_WARN_CANCEL ),
			 cb_disk_blank_warn_clicked, (void*)FALSE );

      dialog_accel_key( Q8TK_KEY_ESC );
    }
    dialog_start();
    break;
  }
}

static	void	cb_disk_blank_warn_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    /* �ե�����ˡ��֥�󥯤��ɵ� */
    sub_disk_attr_file_ctrl( 0, 0, ATTR_CREATE, disk_filename );
  }
}

/*----------------------------------------------------------------------*/
/* �֥ե�����̾��ǧ�� �ܥ��󲡲����ν���				*/

static	void	cb_disk_fname_dialog_ok( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dialog_destroy();
}

static	void	cb_disk_fname( Q8tkWidget *dummy, void *p )
{
  const t_menulabel *l = data_disk_fname;
  char filename[72];
  int save_code;
  size_t len;
  const char *none = "(No Image File)";

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_DISK_FNAME_TITEL ) );
    dialog_set_title( GET_LABEL( l, DATA_DISK_FNAME_LINE ) );

    {
      save_code = q8tk_set_kanjicode( osd_kanji_code() );

      len = strlen( none );
      if( file_disk[0][0] ) len = MAX( len, strlen( file_disk[0] ) );
      if( file_disk[1][0] ) len = MAX( len, strlen( file_disk[1] ) );
      len = MIN( len, sizeof(filename)-1 - 5 );
      len += 5;					/* 5 == strlen("[1:] ") */

      strcpy( filename, "[1:] " );
      strncat( filename, ( file_disk[0][0] ) ? file_disk[0] : none,
	       len - strlen(filename) );
      while( strlen(filename) < len ){ strcat( filename, " " ); }
      dialog_set_title( filename );

      strcpy( filename, "[2:] " );
      strncat( filename, ( file_disk[1][0] ) ? file_disk[1] : none,
	       len - strlen(filename) );
      while( strlen(filename) < len ){ strcat( filename, " " ); }
      dialog_set_title( filename );

      q8tk_set_kanjicode( save_code );
    }

    if( disk_image_exist(0) && disk_same_file() ){
      dialog_set_title( GET_LABEL( l, DATA_DISK_FNAME_SAME ) );
    }


    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_DISK_FNAME_OK ),
		       cb_disk_fname_dialog_ok, NULL );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}


/*======================================================================*/

static	Q8tkWidget	*menu_disk( void )
{
  Q8tkWidget	*hbox, *vbox, *swin, *lab;
  Q8tkWidget	*f, *vx, *hx;
  T_DISK_INFO	*w;
  int	i,j,k;
  const t_menulabel *l;


  hbox = PACK_HBOX( NULL );
  {
    for( k=0; k<COUNTOF(disk_info); k++ ){

      if( menu_swapdrv ){ i = k ^ 1; }
      else              { i = k;     }

      w = &disk_info[i];
      {
	vbox = PACK_VBOX( hbox );
	{
	  lab = PACK_LABEL( vbox, GET_LABEL( data_disk_image_drive, i ) );

	  if( menu_swapdrv )
	    q8tk_misc_set_placement( lab, Q8TK_PLACEMENT_X_RIGHT, 0 );

	  {
	    swin  = q8tk_scrolled_window_new( NULL, NULL );
	    q8tk_widget_show( swin );
	    q8tk_scrolled_window_set_policy( swin, Q8TK_POLICY_NEVER,
					     Q8TK_POLICY_AUTOMATIC );
	    q8tk_misc_set_size( swin, 29, 11 );

	    w->list = q8tk_listbox_new();
	    q8tk_widget_show( w->list );
	    q8tk_container_add( swin, w->list );

	    q8tk_box_pack_start( vbox, swin );
	  }

	  for( j=0; j<2; j++ ){

	    w->label[j] = q8tk_label_new( "" );	/* ����٥�Υ������åȳ��� */
	    q8tk_widget_show( w->label[j] );
	    w->button[j] = q8tk_button_new();
	    q8tk_widget_show( w->button[j] );
	    q8tk_container_add( w->button[j], w->label[j] );
	    q8tk_signal_connect( w->button[j], "clicked",
				 cb_disk_button, (void *)( (j<<8) + i ) );

	    q8tk_box_pack_start( vbox, w->button[j] );
	  }
	}
      }

      PACK_VSEP( hbox );
    }

    {
      vbox = PACK_VBOX( hbox );
      {
	l = data_disk_info;
	for( i=0; i<COUNTOF(disk_info); i++ ){
	  w = &disk_info[i];

	  vx = PACK_VBOX( NULL );
	  {
	    hx = PACK_HBOX( vx );
	    {
	      PACK_LABEL( hx, GET_LABEL( l, DATA_DISK_INFO_STAT ) );
						/* ����٥�Υ������åȳ��� */
	      w->stat_label = PACK_LABEL( hx, "" );
	    }

	    hx = PACK_HBOX( vx );
	    {
	      PACK_LABEL( hx, GET_LABEL( l, DATA_DISK_INFO_ATTR ) );
						/* ����٥�Υ������åȳ��� */
	      w->attr_label = PACK_LABEL( hx, "" );
	    }

	    hx = PACK_HBOX( vx );
	    {
		PACK_LABEL( hx, GET_LABEL( l, DATA_DISK_INFO_NR ) );
						/* ����٥�Υ������åȳ��� */
		w->num_label = PACK_LABEL( hx, "" );
		q8tk_misc_set_placement( w->num_label, Q8TK_PLACEMENT_X_RIGHT,
					               0 );
	    }
	  }

	  f = PACK_FRAME( vbox, GET_LABEL( data_disk_info_drive, i ), vx );
	  q8tk_frame_set_shadow_type( f, Q8TK_SHADOW_IN );
	}

	hx = PACK_HBOX( vbox );
	{
	  PACK_BUTTON( hx, GET_LABEL( data_disk_fname, DATA_DISK_FNAME ),
		       cb_disk_fname, NULL );
	}

	for( i=0; i<2; i++ )			/* ����Ĵ���Τ�����ߡ��򲿸Ĥ� */
	  PACK_LABEL( vbox, "" );
						/* button */
	PACK_BUTTON( vbox,
		     GET_LABEL( data_disk_image, DATA_DISK_IMAGE_BLANK ),
		     cb_disk_blank, NULL );
      }
    }
  }


  set_disk_widget();

  return	hbox;
}



/*===========================================================================
 *
 *	�ᥤ��ڡ���	��������
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
				    /* �ե��󥯥���󥭡�������Ƥ��ѹ� */
static	int	get_key_fkey( int fn_key )
{
  return function_f[ fn_key ];
}
static	void	cb_key_fkey( Q8tkWidget *widget, void *fn_key )
{
  int	i;
  const t_menudata *p = data_key_fkey_fn;
  const char       *combo_str = q8tk_combo_get_text(widget);

  for( i=0; i<COUNTOF(data_key_fkey_fn); i++, p++ ){
    if( strcmp( p->str[menu_lang], combo_str )==0 ){
      function_f[ (int)fn_key ] = p->val;
      return;
    }
  }
}


static	Q8tkWidget	*menu_key_fkey( void )
{
  int	i;
  Q8tkWidget	*vbox, *hbox;
  const t_menudata *p = data_key_fkey;

  vbox = PACK_VBOX( NULL );
  {
    for( i=0; i<COUNTOF(data_key_fkey); i++, p++ ){

      hbox  = PACK_HBOX( vbox );
      {
	PACK_LABEL( hbox, GET_LABEL( p, 0 ) );

	PACK_COMBO( hbox,
		    data_key_fkey_fn, COUNTOF(data_key_fkey_fn),
		    get_key_fkey( p->val ), NULL, 40,
		    cb_key_fkey, (void*)( p->val ) );
      }
    }
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
						      /* ����������ѹ� */
static	int	get_key_cfg( int type )
{
  switch( type ){
  case DATA_KEY_CFG_TENKEY:	return	tenkey_emu;
  case DATA_KEY_CFG_NUMLOCK:	return	numlock_emu;
  }
  return FALSE;
}
static	void	cb_key_cfg( Q8tkWidget *widget, void *type )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  switch( (int)type ){
  case DATA_KEY_CFG_TENKEY:	tenkey_emu  = (key) ? TRUE : FALSE;	break;
  case DATA_KEY_CFG_NUMLOCK:	numlock_emu = (key) ? TRUE : FALSE;	break;
  }
}


static	Q8tkWidget	*menu_key_cfg( void )
{
  Q8tkWidget	*vbox;

  vbox = PACK_VBOX( NULL );
  {
    PACK_CHECK_BUTTONS( vbox,
			data_key_cfg, COUNTOF(data_key_cfg),
			get_key_cfg, cb_key_cfg );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
					      /* ���եȥ����������ܡ��� */
static	void	keymap_start( void );
static	void	keymap_finish( void );

static	void	cb_key_softkeyboard( Q8tkWidget *dummy_0, void *dummy_1 )
{
  keymap_start();
}

static	Q8tkWidget	*menu_key_softkeyboard( void )
{
  Q8tkWidget	*button;
  const t_menulabel *l = data_skey_set;

  button = PACK_BUTTON( NULL,
			GET_LABEL( l, DATA_SKEY_BUTTON_SETUP),
			cb_key_softkeyboard, NULL );

  return button;
}



/*----------------------------------------------------------------------*/
					    /* �������륭���������ޥ��� */
				     /* original idea by floi, thanks ! */
static	Q8tkWidget	*cursor_key_widget;
static	int	get_key_cursor_key_mode( void )
{
  return cursor_key_mode;
}
static	void	cb_key_cursor_key_mode( Q8tkWidget *dummy, void *p )
{
  int m = (int)p;

  if( cursor_key_mode != 2 && m == 2 ) q8tk_widget_show( cursor_key_widget );
  if( cursor_key_mode == 2 && m != 2 ) q8tk_widget_hide( cursor_key_widget );

  cursor_key_mode = m;
}
static	int	get_key_cursor_key( int type )
{
  return cursor_key_assign[ type ];
}
static	void	cb_key_cursor_key( Q8tkWidget *widget, void *type )
{
  int	i;
  const t_keymap *q = keymap_assign;
  const char     *combo_str = q8tk_combo_get_text(widget);

  for( i=0; i<COUNTOF(keymap_assign); i++, q++ ){
    if( strcmp( q->str, combo_str )==0 ){
      cursor_key_assign[ (int)type ] = q->code;
      return;
    }
  }
}


static	Q8tkWidget	*menu_key_cursor( void )
{
  Q8tkWidget	*hbox;

  hbox = PACK_HBOX( NULL );
  {						/* radio_button ... */
    PACK_RADIO_BUTTONS( PACK_VBOX( hbox ),
			data_key_cursor_mode, COUNTOF(data_key_cursor_mode),
			get_key_cursor_key_mode(), cb_key_cursor_key_mode );

    PACK_VSEP( hbox );				/* vseparator */

    cursor_key_widget = 
      PACK_KEY_ASSIN( hbox,			/* combo x 4 */
		      data_key_cursor, COUNTOF(data_key_cursor),
		      get_key_cursor_key, cb_key_cursor_key );

    if( get_key_cursor_key_mode() != 2 ){
      q8tk_widget_hide( cursor_key_widget );
    }
  }

  return hbox;
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_key( void )
{
  Q8tkWidget *vbox, *hbox, *w;
  const t_menulabel *l = data_key;

  vbox = PACK_VBOX( NULL );
  {
    PACK_FRAME( vbox, GET_LABEL( l, DATA_KEY_FKEY ), menu_key_fkey() );

    PACK_FRAME( vbox, GET_LABEL( l, DATA_KEY_CURSOR ), menu_key_cursor() );

    hbox = PACK_HBOX( vbox );
    {
      PACK_FRAME( hbox, GET_LABEL( l, DATA_KEY_CFG ), menu_key_cfg() );

      w = menu_key_softkeyboard();
      PACK_FRAME( hbox, GET_LABEL( l, DATA_KEY_SKEY ), w );
      q8tk_misc_set_placement( w, Q8TK_PLACEMENT_X_CENTER,
			          Q8TK_PLACEMENT_Y_CENTER );
    }
  }

  return vbox;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	���֥�����ɥ�	���եȥ����������ܡ���
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static	Q8tkWidget	*keymap[129];
static	int		keymap_num;
static	Q8tkWidget	*keymap_accel;

enum {			/* keymap[] �ϰʲ��Υ������åȤ���¸�˻Ȥ� */
  KEYMAP_WIN,

  KEYMAP_VBOX,
  KEYMAP_SCRL,
  KEYMAP_SEP,
  KEYMAP_HBOX,

  KEYMAP_BTN_1,
  KEYMAP_BTN_2,

  KEYMAP_LINES,
  KEYMAP_LINE_1,
  KEYMAP_LINE_2,
  KEYMAP_LINE_3,
  KEYMAP_LINE_4,
  KEYMAP_LINE_5,
  KEYMAP_LINE_6,

  KEYMAP_KEY
};

/*----------------------------------------------------------------------*/

static	int	get_key_softkey( int code )
{
  return is_key_pressed( code );
}
static	void	cb_key_softkey( Q8tkWidget *button, void *code )
{
  if( Q8TK_TOGGLE_BUTTON(button)->active ) do_key_press  ( (int)code );
  else                                     do_key_release( (int)code );
}

static	void	cb_key_softkey_release( Q8tkWidget *dummy_0, void *dummy_1 )
{
  do_key_all_release();		/* ���ƤΥ�����Υ�������֤ˤ���         */
  keymap_finish();		/* ���եȥ��������������������åȤ���� */
}

static	void	cb_key_softkey_end( Q8tkWidget *dummy_0, void *dummy_1 )
{
  do_key_bug();			/* ʣ������Ʊ���������Υϡ��ɥХ���Ƹ� */
  keymap_finish();		/* ���եȥ��������������������åȤ���� */
}


/* ���եȥ��ե��������ܡ��� ������ɥ�������ɽ�� */

static	void	keymap_start( void )
{
  Q8tkWidget *w, *v, *s, *l, *h, *b1, *b2, *vx, *hx, *n;
  int i,j;
  int model = ( ROM_VERSION < '8' ) ? 0 : 1;

  for( i=0; i<COUNTOF(keymap); i++ ) keymap[i] = NULL;

  {						/* �ᥤ��Ȥʤ륦����ɥ� */
    w = q8tk_window_new( Q8TK_WINDOW_DIALOG );
    keymap_accel = q8tk_accel_group_new();
    q8tk_accel_group_attach( keymap_accel, w );
  }

  {						/* �ˡ��ܥå�����褻�� */
    v = q8tk_vbox_new();
    q8tk_container_add( w, v );
    q8tk_widget_show( v );
  }

  {							/* �ܥå����ˤ�     */
    {							/* ���������� WIN */
      s = q8tk_scrolled_window_new( NULL, NULL );
      q8tk_box_pack_start( v, s );
      q8tk_misc_set_size( s, 80, 21 );
      q8tk_scrolled_window_set_policy( s, Q8TK_POLICY_AUTOMATIC,
					  Q8TK_POLICY_NEVER     );
      q8tk_widget_show( s );
    }
    {							/* ���ɤ��Τ���ζ���*/
      l = q8tk_label_new( "" );
      q8tk_box_pack_start( v, l );
      q8tk_widget_show( l );
    }
    {							/* �ܥ��������� HBOX */
      h = q8tk_hbox_new();
      q8tk_box_pack_start( v, h );
      q8tk_misc_set_placement( h, Q8TK_PLACEMENT_X_CENTER, 0 );
      q8tk_widget_show( h );

      {								/* HBOX�ˤ� */
	const t_menulabel *l = data_skey_set;
	{							/* �ܥ��� 1 */
	  b1 = q8tk_button_new_with_label( GET_LABEL(l,DATA_SKEY_BUTTON_OFF) );
	  q8tk_signal_connect( b1, "clicked", cb_key_softkey_release, NULL );
	  q8tk_box_pack_start( h, b1 );
	  q8tk_widget_show( b1 );
	}
	{							/* �ܥ��� 2 */
	  b2 = q8tk_button_new_with_label( GET_LABEL(l,DATA_SKEY_BUTTON_QUIT));
	  q8tk_signal_connect( b2, "clicked", cb_key_softkey_end, NULL );
	  q8tk_box_pack_start( h, b2 );
	  q8tk_widget_show( b2 );
	  q8tk_accel_group_add( keymap_accel, Q8TK_KEY_ESC, b2, "clicked" );
	}
      }
    }
  }

  /* ���������� WIN �ˡ������ȥåפ�ʸ���Τ����줿���ܥ�����¤٤� */

  vx = q8tk_vbox_new();			/* ����6��ʬ���Ǽ���� VBOX ������ */
  q8tk_container_add( s, vx );
  q8tk_widget_show( vx );

  keymap[ KEYMAP_WIN   ] = w;
  keymap[ KEYMAP_VBOX  ] = v;
  keymap[ KEYMAP_SCRL  ] = s;
  keymap[ KEYMAP_SEP   ] = l;
  keymap[ KEYMAP_HBOX  ] = h;
  keymap[ KEYMAP_BTN_1 ] = b1;
  keymap[ KEYMAP_BTN_2 ] = b2;
  keymap[ KEYMAP_LINES ] = vx;

  keymap_num = KEYMAP_KEY;


  for( j=0; j<6; j++ ){			/* ����6��ʬ�����֤� */

    const t_keymap *p = keymap_line[ model ][ j ];

    hx = q8tk_hbox_new();		/* ����ʣ���Ĥ��Ǽ���뤿���HBOX�� */
    q8tk_box_pack_start( vx, hx );
    q8tk_widget_show( hx );
    keymap[ KEYMAP_LINE_1 + j ] = hx;

    for( i=0; p[ i ].str; i++ ){	/* ������1�ĤŤ����֤��Ƥ���*/

      if( keymap_num >= COUNTOF( keymap ) )	/* �ȥ�å� */
	{ fprintf( stderr, "%s %d\n", __FILE__, __LINE__ ); break; }
      
      if( p[i].code )				/* �����ȥå�ʸ�� (�ܥ���) */
      {
	n = q8tk_toggle_button_new_with_label( p[i].str );
	if( get_key_softkey( p[i].code ) ){
	  q8tk_toggle_button_set_state( n, TRUE );
	}
	q8tk_signal_connect( n, "toggled", cb_key_softkey, (void *)p[i].code );
      }
      else					/* �ѥǥ����Ѷ��� (��٥�) */
      {
	n = q8tk_label_new( p[i].str );
      }
      q8tk_box_pack_start( hx, n );
      q8tk_widget_show( n );

      keymap[ keymap_num ++ ] = n;
    }
  }


  q8tk_widget_show( w );
  q8tk_grab_add( w );

  q8tk_widget_grab_default( b2 );
}


/* �����ޥåץ��������ν�λ������ */

static	void	keymap_finish( void )
{
  int	i;
  for( i=keymap_num-1; i; i-- ){
    if( keymap[i] ){
      q8tk_widget_destroy( keymap[i] );
    }
  }

  q8tk_grab_remove( keymap[ KEYMAP_WIN ] );
  q8tk_widget_destroy( keymap[ KEYMAP_WIN ] );
  q8tk_widget_destroy( keymap_accel );
}








/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ޥ���
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						/* �ޥ����⡼���ڤ��ؤ� */
static	int	get_mouse_mode( void )
{
  return mouse_mode;
}
static	void	cb_mouse_mode( Q8tkWidget *widget, void *dummy )
{
  int	i;
  const t_menudata *p = data_mouse_mode;
  const char       *combo_str = q8tk_combo_get_text(widget);

  for( i=0; i<COUNTOF(data_mouse_mode); i++, p++ ){
    if( strcmp( p->str[menu_lang], combo_str )==0 ){
      mouse_mode = p->val;
      return;
    }
  }
}


static	Q8tkWidget	*menu_mouse_mode( void )
{
  Q8tkWidget	*hbox;


  hbox  = PACK_HBOX( NULL );
  {
    PACK_LABEL( hbox, GET_LABEL( data_mouse_mode_msg, 0 ) );

    PACK_COMBO( hbox,
		data_mouse_mode, COUNTOF(data_mouse_mode),
		get_mouse_mode(), NULL, 0,
		cb_mouse_mode, NULL );
  }

  return hbox;
}

/*----------------------------------------------------------------------*/
						  /* �ޥ������������ѹ� */
static	Q8tkWidget	*mouse_mouse_widget;
static	int	get_mouse_mouse_key_mode( void )
{
  return mouse_key_mode;
}
static	void	cb_mouse_mouse_key_mode( Q8tkWidget *dummy, void *p )
{
  int m = (int)p;

  if( mouse_key_mode != 2 && m == 2 ) q8tk_widget_show( mouse_mouse_widget );
  if( mouse_key_mode == 2 && m != 2 ) q8tk_widget_hide( mouse_mouse_widget );

  mouse_key_mode = m;
}
static	int	get_mouse_mouse_key( int type )
{
  return mouse_key_assign[ type ];
}
static	void	cb_mouse_mouse_key( Q8tkWidget *widget, void *type )
{
  int	i;
  const t_keymap *q = keymap_assign;
  const char     *combo_str = q8tk_combo_get_text(widget);

  for( i=0; i<COUNTOF(keymap_assign); i++, q++ ){
    if( strcmp( q->str, combo_str )==0 ){
      mouse_key_assign[ (int)type ] = q->code;
      return;
    }
  }
}


static	Q8tkWidget	*menu_mouse_mouse( void )
{
  Q8tkWidget	*hbox;

  hbox = PACK_HBOX( NULL );
  {						/* radio_button ... */
    PACK_RADIO_BUTTONS( PACK_VBOX( hbox ),
			data_mouse_mouse_key_mode,
			COUNTOF(data_mouse_mouse_key_mode),
			get_mouse_mouse_key_mode(),
			cb_mouse_mouse_key_mode );

    PACK_VSEP( hbox );				/* vseparator */

    mouse_mouse_widget = 
      PACK_KEY_ASSIN( hbox,			/* combo x 6 */
		      data_mouse_mouse, COUNTOF(data_mouse_mouse),
		      get_mouse_mouse_key, cb_mouse_mouse_key );

    if( get_mouse_mouse_key_mode() != 2 ){
      q8tk_widget_hide( mouse_mouse_widget );
    }
  }

  return hbox;
}

/*----------------------------------------------------------------------*/
					/* ���祤���ƥ��å����������ѹ� */
static	Q8tkWidget	*mouse_joy_widget;
static	int	get_mouse_joy_key_mode( void )
{
  return joy_key_mode;
}
static	void	cb_mouse_joy_key_mode( Q8tkWidget *dummy, void *p )
{
  int m = (int)p;

  if( joy_key_mode != 2 && m == 2 ) q8tk_widget_show( mouse_joy_widget );
  if( joy_key_mode == 2 && m != 2 ) q8tk_widget_hide( mouse_joy_widget );

  joy_key_mode = m;
}
static	int	get_mouse_joy_key( int type )
{
  return joy_key_assign[ type ];
}
static	void	cb_mouse_joy_key( Q8tkWidget *widget, void *type )
{
  int	i;
  const t_keymap *q = keymap_assign;
  const char     *combo_str = q8tk_combo_get_text(widget);

  for( i=0; i<COUNTOF(keymap_assign); i++, q++ ){
    if( strcmp( q->str, combo_str )==0 ){
      joy_key_assign[ (int)type ] = q->code;
      return;
    }
  }
}
static	int	get_joystick_swap( void )
{
  return joy_swap_button;
}
static	void	cb_joystick_swap( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;

  joy_swap_button = key;
}


static	Q8tkWidget	*menu_mouse_joy( void )
{
  Q8tkWidget	*vbox, *hbox;

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );
    {
      PACK_RADIO_BUTTONS( PACK_VBOX( hbox ),		/* radio ... */
			  data_mouse_joy_key_mode,
			  COUNTOF(data_mouse_joy_key_mode),
			  get_mouse_joy_key_mode(),
			  cb_mouse_joy_key_mode );

      PACK_VSEP( hbox );				/* vseparator */

      mouse_joy_widget =
	PACK_KEY_ASSIN( hbox,				/* combo x 6 */
			data_mouse_joy, COUNTOF(data_mouse_joy),
			get_mouse_joy_key, cb_mouse_joy_key );

      if( get_mouse_joy_key_mode() != 2 ){
	q8tk_widget_hide( mouse_joy_widget );
      }
    }

    PACK_LABEL( vbox, "                            " );	/* ���� */

    PACK_CHECK_BUTTON( vbox,				/* check */
		       GET_LABEL( data_mouse_joystick_swap, 0 ),
		       get_joystick_swap(),
		       cb_joystick_swap, NULL );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_mouse( void )
{
  Q8tkWidget *vbox;
  const t_menulabel *l = data_mouse;

  vbox = PACK_VBOX( NULL );
  {
    PACK_FRAME( vbox, GET_LABEL( l, DATA_MOUSE_MODE ), menu_mouse_mode() );

    PACK_FRAME( vbox, GET_LABEL( l, DATA_MOUSE_MOUSE ), menu_mouse_mouse() );

    PACK_FRAME( vbox, GET_LABEL( l, DATA_MOUSE_JOYSTICK ), menu_mouse_joy() );
  }

  return vbox;
}



/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ơ���
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
				      /* ���ɥ��᡼���������֥��᡼�� */
int			tape_mode;
static	char		tape_filename[ QUASI88_MAX_FILENAME ];

static	Q8tkWidget	*tape_name[2];
static	Q8tkWidget	*tape_rate[2];

/*----------------------------------------------------------------------*/
static	void	set_tape_name( int c )
{
  q8tk_entry_set_text( tape_name[ c ],
		       ( (file_tape[ c ][0]=='\0') ? "(No File)"
						   : file_tape[ c ] ) );
}
static	void	set_tape_rate( int c  )
{
  char buf[16];
  long cur, end;

  if( c == CLOAD ){

    if( sio_tape_pos( &cur, &end ) ){
      if( end == 0 ){
	sprintf( buf, "   END " );
      }else{
	sprintf( buf, "   %3ld%%", cur * 100 / end );
      }
    }else{
      sprintf( buf, "   ---%%" );
    }

    q8tk_label_set( tape_rate[ c ], buf );
  }
}


/*----------------------------------------------------------------------*/
/* ��EJECT�ץܥ��󲡲����ν���						*/

static	void	cb_tape_eject_do( Q8tkWidget *dummy, void *c )
{
  if( (int)c == CLOAD ){
    quasi88_load_tape_eject();
  }else{
    quasi88_save_tape_eject();
  }

  set_tape_name( (int)c );
  set_tape_rate( (int)c );
}

/*----------------------------------------------------------------------*/
/* ��REW�ץܥ��󲡲����ν���						*/

static	void	cb_tape_rew_do( Q8tkWidget *dummy, void *c )
{
  if( (int)c == CLOAD ){
					/* ���᡼���򴬤��᤹ */
    if( quasi88_load_tape_rewind() ){				/* ���� */
      ;
    }else{							/* ���� */
      set_tape_name( (int)c );
    }
    set_tape_rate( (int)c );
  }
}

/*----------------------------------------------------------------------*/
/* ��OPEN�ץܥ��󲡲����ν���						*/

static	void	sub_tape_open( void );
static	void	sub_tape_open_do( void );
static	void	cb_tape_open_warn_clicked( Q8tkWidget *, void * );

static	void	cb_tape_open( Q8tkWidget *dummy, void *c )
{
  const char *initial;
  const t_menulabel *l = ( (int)c == CLOAD ) ? data_tape_load : data_tape_save;

				/* ��������������ե����륻�쥯������ */
  tape_mode = (int)c;		/* LOAD�� �� SAVE�Ѥ� ��Ф��Ƥ���      */

  if( file_tape[ (int)c ][0] != '\0' ) initial = file_tape[ (int)c ];
  else{
    initial = osd_dir_tape();
    if( initial==NULL ) initial = osd_dir_cwd();
  }


  START_FILE_SELECTION( GET_LABEL( l, DATA_TAPE_FSEL ),
			-1,	/* ReadOnly ��������Բ� */
			initial,

			sub_tape_open,
			tape_filename,
			QUASI88_MAX_FILENAME,
			NULL );
}

static	void	sub_tape_open( void )
{
  const t_menulabel *l = data_tape_save;

  switch( osd_file_stat( tape_filename ) ){

  case FILE_STAT_NOEXIST:
    if( tape_mode == CLOAD ){			/* �ե�����̵���Τǥ��顼   */
      start_file_error_dialog( -1, ERR_CANT_OPEN );
    }else{	      				/* �ե�����̵���Τǿ������� */
      sub_tape_open_do();
    }
    break;

  case FILE_STAT_DIR:
    /* �ǥ��쥯�ȥ�ʤΤǡ������������ */
    start_file_error_dialog( -1, ERR_CANT_OPEN );
    break;

  default:
    if( tape_mode == CSAVE ){
      /* ���Ǥ˥ե����뤬¸�ߤ��ޤ������᡼�����ɵ����ޤ����� */
      dialog_create();
      {
	dialog_set_title( GET_LABEL( l, DATA_TAPE_WARN_0 ) );

	dialog_set_title( GET_LABEL( l, DATA_TAPE_WARN_1 ) );

	dialog_set_separator();

	dialog_set_button( GET_LABEL( l, DATA_TAPE_WARN_APPEND ),
			   cb_tape_open_warn_clicked, (void*)TRUE );

	dialog_set_button( GET_LABEL( l, DATA_TAPE_WARN_CANCEL ),
			   cb_tape_open_warn_clicked, (void*)FALSE );

	dialog_accel_key( Q8TK_KEY_ESC );
      }
      dialog_start();
    }else{
      sub_tape_open_do();
    }
    break;
  }
}

static	void	sub_tape_open_do( void )
{
  int result, c = tape_mode;

  if( c == CLOAD ){			/* �ơ��פ򳫤� */
    result = quasi88_load_tape_insert( tape_filename );
  }else{
    result = quasi88_save_tape_insert( tape_filename );
  }

  set_tape_name( c );
  set_tape_rate( c );


  if( result == 0 ){
    start_file_error_dialog( -1, ERR_CANT_OPEN );
  }
}

static	void	cb_tape_open_warn_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    sub_tape_open_do();
  }
}




/*----------------------------------------------------------------------*/

INLINE	Q8tkWidget	*menu_tape_image_unit( const t_menulabel *l, int c )
{
  int save_code;
  Q8tkWidget	*vbox, *hbox, *w, *e;

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );
    {
      w = PACK_LABEL( hbox, GET_LABEL( l, DATA_TAPE_FOR ) );
      q8tk_misc_set_placement( w, Q8TK_PLACEMENT_X_CENTER,
			          Q8TK_PLACEMENT_Y_CENTER );

      {
	save_code = q8tk_set_kanjicode( osd_kanji_code() );

	e = PACK_ENTRY( hbox,
			QUASI88_MAX_FILENAME, 65, NULL,
			NULL, NULL, NULL, NULL );
	q8tk_entry_set_editable( e, FALSE );

	tape_name[ c ] = e;
	set_tape_name( c );

	q8tk_set_kanjicode( save_code );
      }
    }

    hbox = PACK_HBOX( vbox );
    {
      PACK_BUTTON( hbox,
		   GET_LABEL( l, DATA_TAPE_CHANGE ),
		   cb_tape_open, (void*)c );

      PACK_VSEP( hbox );

      PACK_BUTTON( hbox,
		   GET_LABEL( l, DATA_TAPE_EJECT ),
		   cb_tape_eject_do, (void*)c );

      if( c == CLOAD ){
	PACK_BUTTON( hbox,
		     GET_LABEL( l, DATA_TAPE_REWIND ),
		     cb_tape_rew_do, (void*)c );
      }
      if( c == CLOAD ){
	w = PACK_LABEL( hbox, "" );
	q8tk_misc_set_placement( w, Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER );
	tape_rate[ c ] = w;
	set_tape_rate( c );
      }
    }
  }

  return vbox;
}

static	Q8tkWidget	*menu_tape_image( void )
{
  Q8tkWidget *vbox, *w;

  vbox = PACK_VBOX( NULL );
  {
    q8tk_box_pack_start( vbox, menu_tape_image_unit( data_tape_load, CLOAD ) );

    PACK_HSEP( vbox );

    q8tk_box_pack_start( vbox, menu_tape_image_unit( data_tape_save, CSAVE ) );
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
					    /* �ơ��׽����⡼���ڤ��ؤ� */
static	int	get_tape_intr( void )
{
  return cmt_intr;
}
static	void	cb_tape_intr( Q8tkWidget *dummy, void *p )
{
  cmt_intr = (int)p;
}


static	Q8tkWidget	*menu_tape_intr( void )
{
  Q8tkWidget	*vbox;

  vbox = PACK_VBOX( NULL );
  {
    PACK_RADIO_BUTTONS( vbox,
			data_tape_intr, COUNTOF(data_tape_intr),
			get_tape_intr(), cb_tape_intr );
  }

  return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_tape( void )
{
  Q8tkWidget *vbox;
  const t_menulabel *l = data_tape;

  vbox = PACK_VBOX( NULL );
  {
    PACK_FRAME( vbox, GET_LABEL( l, DATA_TAPE_IMAGE ), menu_tape_image() );

    PACK_FRAME( vbox, GET_LABEL( l, DATA_TAPE_INTR ), menu_tape_intr() );
  }

  return vbox;
}



/*===========================================================================
 *
 *	�ᥤ��ڡ���	����¾
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
							  /* �����ڥ�� */

static	Q8tkWidget	*misc_suspend_entry;
static	Q8tkWidget	*misc_suspend_combo;

/*	�����ڥ�ɻ��Υ�å���������������ä�			  */
static	void	cb_misc_suspend_dialog_ok( Q8tkWidget *dummy, void *result )
{
  dialog_destroy();

  if( (int)result == DATA_MISC_RESUME_OK ){
    set_emu_mode( GO );
    q8tk_main_quit();
  }
}

/*	�����ڥ�ɼ¹Ը�Υ�å�������������				  */
static	void	sub_misc_suspend_dialog( int result )
{
  const t_menulabel *l = data_misc_suspend_err;
  char filename[72];
  int save_code;
  size_t len;
  const char *none = "(No Image File)";

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, result ) );	/* ���ɽ�� */

    if( result == DATA_MISC_SUSPEND_OK ||	/* �������ϥ��᡼��̾ɽ�� */
	result == DATA_MISC_RESUME_OK  ){

      dialog_set_title( GET_LABEL( l, DATA_MISC_SUSPEND_LINE ) );
      dialog_set_title( GET_LABEL( l, DATA_MISC_SUSPEND_INFO ) );

      save_code = q8tk_set_kanjicode( osd_kanji_code() );

      len = strlen( none );
      if( file_disk[0][0] ) len = MAX( len, strlen( file_disk[0] ) );
      if( file_disk[1][0] ) len = MAX( len, strlen( file_disk[1] ) );
      if( file_tape[0][0] ) len = MAX( len, strlen( file_tape[0] ) );
      if( file_tape[1][0] ) len = MAX( len, strlen( file_tape[1] ) );
      len = MIN( len, sizeof(filename)-1 - 11 );
      len += 11;				/* 11==strlen("[DRIVE 1:] ") */

      strcpy( filename, "[DRIVE 1:] " );
      strncat( filename, ( file_disk[0][0] ) ? file_disk[0] : none,
	       len - strlen(filename) );
      while( strlen(filename) < len ){ strcat( filename, " " ); }
      dialog_set_title( filename );

      strcpy( filename, "[DRIVE 2:] " );
      strncat( filename, ( file_disk[1][0] ) ? file_disk[1] : none,
	       len - strlen(filename) );
      while( strlen(filename) < len ){ strcat( filename, " " ); }
      dialog_set_title( filename );

      strcpy( filename, "[TapeLOAD] " );
      strncat( filename, ( file_tape[0][0] ) ? file_tape[0] : none,
	       len - strlen(filename) );
      while( strlen(filename) < len ){ strcat( filename, " " ); }
      dialog_set_title( filename );

      strcpy( filename, "[TapeSAVE] " );
      strncat( filename, ( file_tape[1][0] ) ? file_tape[1] : none,
	       len - strlen(filename) );
      while( strlen(filename) < len ){ strcat( filename, " " ); }
      dialog_set_title( filename );

      q8tk_set_kanjicode( save_code );
    }

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_MISC_SUSPEND_AGREE ),
		       cb_misc_suspend_dialog_ok, (void*)result );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*	�쥸�塼��¹����Υ�å�������������				  */
static	void	sub_misc_suspend_not_access( void )
{
  const t_menulabel *l = data_misc_suspend_err;

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_MISC_RESUME_CANTOPEN ) );

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_MISC_SUSPEND_AGREE ),
		       cb_misc_suspend_dialog_ok,
		       (void*)DATA_MISC_SUSPEND_AGREE );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*	�����ڥ�ɼ¹����Υ�å�������������				  */
static	void	cb_misc_suspend_overwrite( Q8tkWidget *dummy_0, void *dummy_1);
static	void	sub_misc_suspend_really( void )
{
  const t_menulabel *l = data_misc_suspend_err;

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_MISC_SUSPEND_REALLY ) );

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_MISC_SUSPEND_OVERWRITE ),
		       cb_misc_suspend_overwrite, NULL );
    dialog_set_button( GET_LABEL( l, DATA_MISC_SUSPEND_CANCEL ),
		       cb_misc_suspend_dialog_ok,
		       (void*)DATA_MISC_SUSPEND_CANCEL );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

static	void	cb_misc_suspend_overwrite( Q8tkWidget *dummy_0, void *dummy_1 )
{
  dialog_destroy();
  {
    if( statesave() ){
      sub_misc_suspend_dialog( DATA_MISC_SUSPEND_OK );		/* ���� */
    }else{
      sub_misc_suspend_dialog( DATA_MISC_SUSPEND_ERR );		/* ���� */
    }
  }
}

/*----------------------------------------------------------------------*/
/*	�����ڥ�ɽ��� (�֥����֡ץ���å��� )				*/
static	void	cb_misc_suspend_save( Q8tkWidget *dummy_0, void *dummy_1 )
{
#if 0	/* ����������񤭳�ǧ���Ƥ���ΤϤ������� */
  if( statesave_check() ){				/* �ե����뤢�� */
    sub_misc_suspend_really();
  }else
#endif
  {
    if( statesave() ){
      sub_misc_suspend_dialog( DATA_MISC_SUSPEND_OK );		/* ���� */
    }else{
      sub_misc_suspend_dialog( DATA_MISC_SUSPEND_ERR );		/* ���� */
    }
  }
}

/*----------------------------------------------------------------------*/
/*	�����ڥ�ɽ��� (�֥��ɡץ���å��� )				*/
static	void	cb_misc_suspend_load( Q8tkWidget *dummy_0, void *dummy_1 )
{
  if( stateload_check() == FALSE ){			/* �ե�����ʤ� */
    sub_misc_suspend_not_access();
  }else{
    if( quasi88_stateload() ){
      sub_misc_suspend_dialog( DATA_MISC_RESUME_OK );		/* ���� */
    }else{
      sub_misc_suspend_dialog( DATA_MISC_RESUME_ERR );		/* ���� */
    }
  }
}

/*----------------------------------------------------------------------*/
/*	�ե�����̾�����å�						*/

static int get_misc_suspend_num( void )
{
  const char  *ssfx = STATE_SUFFIX;		/* ".sta" */
  const size_t nsfx = strlen(STATE_SUFFIX);	/* 4      */

  if( strlen(file_state) > nsfx &&
      my_strcmp( &file_state[ strlen(file_state)-nsfx ], ssfx ) == 0 ){

    if( strlen(file_state) > nsfx+2 &&	/* �ե�����̾�� xxx-N.sta */
	'-'  ==  file_state[ strlen(file_state) -nsfx -2 ]   &&
	isdigit( file_state[ strlen(file_state) -nsfx -1 ] ) ){

      return file_state[ strlen(file_state) -nsfx -1 ];    /* '0'��'9'���֤� */

    }else{				/* �ե�����̾�� xxx.sta */
      return 0;
    }
  }else{				/* �ե�����̾�� ����¾ */
    return -1;
  }
}

/*----------------------------------------------------------------------*/
/*	�ե�����̾���ѹ�������ȥ꡼ changed (����)���˸ƤФ�롣       */
/*		(�ե����륻�쥯�����Ǥ��ѹ����Ϥ���ϸƤФ�ʤ�)      */

static void cb_misc_suspend_entry_change( Q8tkWidget *widget, void *dummy )
{
  int i, j;
  char buf[4];

  strncpy( file_state, q8tk_entry_get_text( widget ),
	   QUASI88_MAX_FILENAME-1 );
  file_state[ QUASI88_MAX_FILENAME-1 ] = '\0';

  i = get_misc_suspend_num();		/* ̾���� .sta �ǽ���С�������ѹ� */
  if( i > 0 ){ buf[0] = i;   }
  else       { buf[0] = ' '; }
  buf[1] = '\0';
  if( *(q8tk_combo_get_text( misc_suspend_combo )) != buf[0] ){
    q8tk_combo_set_text( misc_suspend_combo, buf );
  }
}

/*----------------------------------------------------------------------*/
/*	�ե���������������ե����륻�쥯���������			*/

static void sub_misc_suspend_change( void );

static	void	cb_misc_suspend_fsel( Q8tkWidget *dummy_0, void *dummy_1 )
{
  const t_menulabel *l = data_misc_suspend;


  START_FILE_SELECTION( GET_LABEL( l, DATA_MISC_SUSPEND_FSEL ),
			-1,	/* ReadOnly ��������Բ� */
			file_state,

			sub_misc_suspend_change,
			file_state,
			QUASI88_MAX_FILENAME,
			NULL );
}

static void sub_misc_suspend_change( void )
{
  int i;
  char buf[4];

  q8tk_entry_set_text( misc_suspend_entry, file_state );

  i = get_misc_suspend_num();		/* ̾���� .sta �ǽ���С�������ѹ� */
  if( i > 0 ){ buf[0] = i;   }
  else       { buf[0] = ' '; }
  buf[1] = '\0';
  q8tk_combo_set_text( misc_suspend_combo, buf );
}


static void cb_misc_suspend_num( Q8tkWidget *widget, void *dummy )
{
  const char  *ssfx = STATE_SUFFIX;		/* ".sta"   */
  const size_t nsfx = strlen(STATE_SUFFIX);	/* 4        */
  char        buf[] = "-N" STATE_SUFFIX;	/* "-N.sta" */
  int len;

  int	i, chg = FALSE;
  const t_menudata *p = data_misc_suspend_num;
  const char       *combo_str = q8tk_combo_get_text(widget);

  for( i=0; i<COUNTOF(data_misc_suspend_num); i++, p++ ){
    if( strcmp( p->str[menu_lang], combo_str ) == 0 ){

      i = get_misc_suspend_num();
      buf[1] = p->val;

				/* �ե�����̾���� .sta �ʤ��� -N.sta ���� */
      len = strlen(file_state);
      if( i>0 ){						/* xxx-N.sta */
	file_state[ strlen(file_state) -nsfx -2 ] = '\0';
      }else if( i==0 ){						/* xxx.sta   */
	file_state[ strlen(file_state) -nsfx    ] = '\0';
      }else{							/* xxx       */
	;
      }

      if( i>0 ){			/* ���Υե�����̾�� xxx-N.sta */

	if( p->val == 0 ){			/* xxx-N.sta -> xxx.sta */
	  strcat( file_state, ssfx );
	}else{					/* xxx-N.sta -> xxx-M.sta */
	  strcat( file_state, buf );
	}
	chg = TRUE;

      }else if( i==0 ){			/* ���Υե�����̾�� xxx.sta */

	if( p->val ){				/* xxx.sta -> xxx-N.sta */
	  if( len + 2 < QUASI88_MAX_FILENAME ){
	    strcat( file_state, buf );
	    chg = TRUE;
	  }
	}

      }else{				/* ���Υե�����̾�� ����¾ xxx */

	if( p->val == 0 ){			/* xxx -> xxx.sta */
	  if( len + nsfx < QUASI88_MAX_FILENAME ){
	    strcat( file_state, ssfx );
	    chg = TRUE;
	  }
	}else{					/* xxx -> xxx-N.sta */
	  if( len + nsfx +2 < QUASI88_MAX_FILENAME ){
	    strcat( file_state, buf );
	    chg = TRUE;
	  }
	}
      }
      if( chg ){
	q8tk_entry_set_text( misc_suspend_entry, file_state );
	q8tk_entry_set_position( misc_suspend_entry, QUASI88_MAX_FILENAME );
      }
      return;
    }
  }
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_misc_suspend( void )
{
  Q8tkWidget	*vbox, *hbox;
  Q8tkWidget	*w, *e;
  const t_menulabel *l = data_misc_suspend;
  int save_code;

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );
    {
      {
	save_code = q8tk_set_kanjicode( osd_kanji_code() );

	e = PACK_ENTRY( hbox,
			QUASI88_MAX_FILENAME, 67, file_state,
			NULL, NULL,
			cb_misc_suspend_entry_change, NULL );
/*      q8tk_entry_set_position( e, 0 );*/
	misc_suspend_entry = e;

	q8tk_set_kanjicode( save_code );
      }

      PACK_LABEL( hbox, " " );

      PACK_BUTTON( hbox,
		   GET_LABEL( l, DATA_MISC_SUSPEND_CHANGE ),
		   cb_misc_suspend_fsel, NULL );
    }

/*  PACK_HSEP( vbox );*/

    hbox = PACK_HBOX( vbox );
    {
      PACK_BUTTON( hbox,
		   GET_LABEL( l, DATA_MISC_SUSPEND_SAVE ),
		   cb_misc_suspend_save, NULL );

      PACK_LABEL( hbox, " " );
      PACK_VSEP( hbox );
      PACK_LABEL( hbox, " " );

      PACK_BUTTON( hbox,
		   GET_LABEL( l, DATA_MISC_SUSPEND_LOAD ),
		   cb_misc_suspend_load, NULL );

      w = PACK_LABEL( hbox, GET_LABEL( l, DATA_MISC_SUSPEND_NUMBER ) );
      q8tk_misc_set_placement( w, Q8TK_PLACEMENT_X_CENTER,
			          Q8TK_PLACEMENT_Y_CENTER );

      w = PACK_COMBO( hbox,
		      data_misc_suspend_num, COUNTOF(data_misc_suspend_num),
		      get_misc_suspend_num(), " ", 0,
		      cb_misc_suspend_num, NULL );
      q8tk_misc_set_placement( w, Q8TK_PLACEMENT_X_CENTER,
			       Q8TK_PLACEMENT_Y_CENTER );
      misc_suspend_combo = w;
    }
  }

  return vbox;
}



/*----------------------------------------------------------------------*/
					 /* �����꡼�� ���ʥåץ���å� */

static	Q8tkWidget	*misc_snapshot_entry;


/*----------------------------------------------------------------------*/
/*	���ʥåץ���å� ������ (�ּ¹ԡץ���å��� )			*/
static	void	cb_misc_snapshot_do( void )
{
  size_t len = strlen( file_snap );

  save_screen_snapshot();

  if( len != strlen( file_snap ) ) 
    q8tk_entry_set_text( misc_snapshot_entry, file_snap );
}

/*----------------------------------------------------------------------*/
/*	�����ե����ޥå��ڤ��ؤ�					*/
static	int	get_misc_snapshot_format( void )
{
  return snapshot_format;
}
static	void	cb_misc_snapshot_format( Q8tkWidget *dummy, void *p )
{
  snapshot_format = (int)p;
}



/*----------------------------------------------------------------------*/
/*	�ե�����̾���ѹ�������ȥ꡼ changed (����)���˸ƤФ�롣	*/
/*		(�ե����륻�쥯�����Ǥ��ѹ����Ϥ���ϸƤФ�ʤ�)	*/

static void cb_misc_snapshot_entry_change( Q8tkWidget *widget, void *dummy )
{
  strncpy( file_snap, q8tk_entry_get_text( widget ),
	   QUASI88_MAX_FILENAME-1 );
  file_snap[ QUASI88_MAX_FILENAME-1 ] = '\0';
}

/*----------------------------------------------------------------------*/
/*	�ե���������������ե����륻�쥯���������			*/

static void sub_misc_snapshot_change( void );

static	void	cb_misc_snapshot_fsel( Q8tkWidget *dummy_0, void *dummy_1 )
{
  const t_menulabel *l = data_misc_snapshot;


  START_FILE_SELECTION( GET_LABEL( l, DATA_MISC_SNAPSHOT_FSEL ),
			-1,	/* ReadOnly ��������Բ� */
			file_snap,

			sub_misc_snapshot_change,
			file_snap,
			QUASI88_MAX_FILENAME,
			NULL );
}

static void sub_misc_snapshot_change( void )
{
  q8tk_entry_set_text( misc_snapshot_entry, file_snap );
}


/*----------------------------------------------------------------------*/
#ifdef	USE_SSS_CMD
/*	���ޥ�ɼ¹Ծ����ѹ� */
static	int	get_misc_snapshot_c_do( void )
{
  return snapshot_cmd_do;
}
static	void	cb_misc_snapshot_c_do( Q8tkWidget *widget, void *dummy )
{
  int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
  snapshot_cmd_do = key;
}

/*	���ޥ���ѹ�������ȥ꡼ changed (����)���˸ƤФ�롣  */
static void cb_misc_snapshot_c_entry_change( Q8tkWidget *widget, void *dummy )
{
  strncpy( snapshot_cmd, q8tk_entry_get_text( widget ),
	   SNAPSHOT_CMD_SIZE-1 );
  snapshot_cmd[ SNAPSHOT_CMD_SIZE-1 ] = '\0';
}
#endif

/*----------------------------------------------------------------------*/
static	Q8tkWidget	*menu_misc_snapshot( void )
{
  Q8tkWidget	*hbox, *vbox, *hbox2, *vbox2;
  Q8tkWidget	*e, *w;
  const t_menulabel *l = data_misc_snapshot;
  int save_code;

  vbox = PACK_VBOX( NULL );
  {
    hbox = PACK_HBOX( vbox );
    {
      {
	save_code = q8tk_set_kanjicode( osd_kanji_code() );

	e = PACK_ENTRY( hbox,
			QUASI88_MAX_FILENAME, 67, file_snap,
			NULL, NULL,
			cb_misc_snapshot_entry_change, NULL );
/*      q8tk_entry_set_position( e, 0 );*/
	misc_snapshot_entry = e; 

	q8tk_set_kanjicode( save_code );
      }
      PACK_LABEL( hbox, " " );

      PACK_BUTTON( hbox,
		   GET_LABEL( l, DATA_MISC_SNAPSHOT_CHANGE ),
		   cb_misc_snapshot_fsel, NULL );
    }

    hbox = PACK_HBOX( vbox );
    {
      PACK_BUTTON( hbox,
		   GET_LABEL( l, DATA_MISC_SNAPSHOT_BUTTON ),
		   cb_misc_snapshot_do, NULL );

      PACK_VSEP( hbox );

      vbox2 = PACK_VBOX( hbox );
      {
	if( snapshot_cmd_enable == FALSE ){
	  PACK_LABEL( vbox2, "" );
	}

	hbox2 = PACK_HBOX( vbox2 );
	{
	  PACK_LABEL( hbox2, GET_LABEL( l, DATA_MISC_SNAPSHOT_FORMAT ) );

	  PACK_RADIO_BUTTONS( PACK_HBOX( hbox2 ),
			      data_misc_snapshot_format,
			      COUNTOF(data_misc_snapshot_format),
			      get_misc_snapshot_format(),
			      cb_misc_snapshot_format );
	}

#ifdef	USE_SSS_CMD
	if( snapshot_cmd_enable ){

	  PACK_LABEL( vbox2, "" );			/* ���� */

	  hbox2 = PACK_HBOX( vbox2 );
	  {
	    PACK_CHECK_BUTTON( hbox2,
			       GET_LABEL( l, DATA_MISC_SNAPSHOT_CMD ),
			       get_misc_snapshot_c_do(),
			       cb_misc_snapshot_c_do, NULL );

	    PACK_ENTRY( hbox2,
			SNAPSHOT_CMD_SIZE, 41, snapshot_cmd,
			NULL, NULL,
			cb_misc_snapshot_c_entry_change, NULL );
	  }
	}
#endif	/* USE_SSS_CMD */
      }
    }
  }

  return vbox;
}

/*----------------------------------------------------------------------*/
						    /* �ե�����̾��碌 */
static	int	get_misc_sync( void )
{
  return filename_synchronize;
}
static	void	cb_misc_sync( Q8tkWidget *widget, void *dummy )
{
  filename_synchronize = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;
}



/*======================================================================*/

static	Q8tkWidget	*menu_misc( void )
{
  Q8tkWidget *vbox, *w;
  const t_menulabel *l = data_misc;

  vbox = PACK_VBOX( NULL );
  {
    PACK_FRAME( vbox, GET_LABEL( l, DATA_MISC_SUSPEND ), menu_misc_suspend() );

    PACK_FRAME( vbox, GET_LABEL( l, DATA_MISC_SNAPSHOT), menu_misc_snapshot());

    PACK_LABEL( vbox, "" );			/* ���� */

    PACK_CHECK_BUTTON( vbox,			/* check_button */
		       GET_LABEL( data_misc_sync, 0 ),
		       get_misc_sync(),
		       cb_misc_sync, NULL );
  }

  return vbox;
}










/*===========================================================================
 *
 *	�ᥤ��ڡ���	�С���������
 *
 *===========================================================================*/

static	Q8tkWidget	*menu_about( void )
{
  int i;
  Q8tkWidget *vx, *hx, *vbox, *swin, *hbox, *w;

  vx = PACK_VBOX( NULL );
  {
    hx = PACK_HBOX( vx );				/* ��Ⱦʬ�˥�ɽ�� */
    {
      PACK_LABEL( hx, " " );/*indent*/

      if( strcmp( Q_TITLE, "QUASI88" )==0 ){
	w = q8tk_logo_new();
	q8tk_widget_show( w );
	q8tk_box_pack_start( hx, w );
      }else{
	PACK_LABEL( hx, Q_TITLE );
      }

      vbox = PACK_VBOX( hx );
      {
	i = Q8GR_LOGO_H;
#ifdef	USE_MONITOR
	PACK_LABEL( vbox, "  (with monitor-mode)" );
	i--;
#endif
	for( ; i>1; i-- ) PACK_LABEL( vbox, "" );

	PACK_LABEL( vbox, "  ver. " Q_VERSION  "  < " Q_COMMENT " >" );
      }
    }
							/* ��Ⱦʬ�Ͼ���ɽ�� */
    swin  = q8tk_scrolled_window_new( NULL, NULL );
    {
      hbox = PACK_HBOX( NULL );
      {
	PACK_LABEL( hbox, " " );/*indent*/

	vbox = PACK_VBOX( hbox );
	{
	  {			/* ������ɤ˴ؤ������ɽ�� */
	    const char *(*s) = (menu_lang==0) ? data_about_en : data_about_jp;

	    for( i=0; s[i]; i++ ){
	      PACK_LABEL( vbox, s[i] );
	    }
	  }

	  {			/* �����ƥ��¸���˴ؤ������ɽ�� */
	    const char *s;
	    int save_code = 0, code = about_msg_init( menu_lang );

	    if( code >= 0 ){ save_code = q8tk_set_kanjicode( code ); }

	    PACK_LABEL( vbox, "" );
	    while( (s = about_msg() ) ){
	      PACK_LABEL( vbox, s );
	    }

	    if( code >= 0 ){ q8tk_set_kanjicode( save_code ); }
	  }
	}
      }
      q8tk_container_add( swin, hbox );
    }

    q8tk_scrolled_window_set_policy( swin, Q8TK_POLICY_AUTOMATIC,
					   Q8TK_POLICY_AUTOMATIC );
    q8tk_misc_set_size( swin, 78, 18-Q8GR_LOGO_H );
    q8tk_widget_show( swin );
    q8tk_box_pack_start( vx, swin );
  }

  return vx;
}










/*===========================================================================
 *
 *	�ᥤ�󥦥���ɥ�
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
				     /* NOTEBOOK ��ĥ���դ��롢�ƥڡ��� */
static	struct{
  int		data_num;
  Q8tkWidget	*(*menu_func)(void);
} menu_page[] =
{
  { DATA_TOP_RESET,	menu_reset,	},
  { DATA_TOP_CPU,	menu_cpu,	},
  { DATA_TOP_GRAPH,	menu_graph,	},
  { DATA_TOP_VOLUME,	menu_volume,	},
  { DATA_TOP_DISK,	menu_disk,	},
  { DATA_TOP_KEY,	menu_key,	},
  { DATA_TOP_MOUSE,	menu_mouse,	},
  { DATA_TOP_TAPE,	menu_tape,	},
  { DATA_TOP_MISC,	menu_misc,	},
  { DATA_TOP_ABOUT,	menu_about,	},
};

/*----------------------------------------------------------------------*/
/* NOTEBOOK �γƥڡ����򡢥ե��󥯥���󥭡�����������褦�ˡ�
   ��������졼�����������ꤹ�롣���Τ��ᡢ���ߡ��������å����� */

#define	cb_note_fake(fn,n)						     \
static	void	cb_note_fake_##fn( Q8tkWidget *dummy, Q8tkWidget *notebook ){\
  q8tk_notebook_set_page( notebook, n );				     \
}
cb_note_fake(f1,0)
cb_note_fake(f2,1)
cb_note_fake(f3,2)
cb_note_fake(f4,3)
cb_note_fake(f5,4)
cb_note_fake(f6,5)
cb_note_fake(f7,6)
cb_note_fake(f8,7)
cb_note_fake(f9,8)
cb_note_fake(f10,9)

     /* �ʲ��Υ�������졼�����������ϡ� floi�� �󶡡� Thanks ! */
static	void	cb_note_fake_prev( Q8tkWidget *dummy, Q8tkWidget *notebook ){
  int	n = q8tk_notebook_current_page( notebook ) - 1;
  if( n < 0 ) n = COUNTOF(menu_page) - 1;
  q8tk_notebook_set_page( notebook, n );
}

static	void	cb_note_fake_next( Q8tkWidget *dummy, Q8tkWidget *notebook ){
  int   n = q8tk_notebook_current_page( notebook ) + 1;
  if( COUNTOF(menu_page) <= n ) n = 0;
  q8tk_notebook_set_page( notebook, n );
}

static	struct{
  int	key;
  void	(*cb_func)(Q8tkWidget *, Q8tkWidget *);
} menu_fkey[] =
{
  { Q8TK_KEY_F1,  cb_note_fake_f1,  },
  { Q8TK_KEY_F2,  cb_note_fake_f2,  },
  { Q8TK_KEY_F3,  cb_note_fake_f3,  },
  { Q8TK_KEY_F4,  cb_note_fake_f4,  },
  { Q8TK_KEY_F5,  cb_note_fake_f5,  },
  { Q8TK_KEY_F6,  cb_note_fake_f6,  },
  { Q8TK_KEY_F7,  cb_note_fake_f7,  },
  { Q8TK_KEY_F8,  cb_note_fake_f8,  },
  { Q8TK_KEY_F9,  cb_note_fake_f9,  },
  { Q8TK_KEY_F10, cb_note_fake_f10, },

  { Q8TK_KEY_HOME,      cb_note_fake_prev,  },
  { Q8TK_KEY_END,       cb_note_fake_next,  },
};

/*----------------------------------------------------------------------*/
					/* �ᥤ�󥦥���ɥ������Υܥ��� */
static	void	sub_top_quit( void );

static	int	get_top_status( void ){ return now_status; }
static	void	cb_top_status( Q8tkWidget *widget, void *dummy )
{
  /*int	key = ( Q8TK_TOGGLE_BUTTON(widget)->active ) ? TRUE : FALSE;*/
  quasi88_status();
}

static	void	cb_top_button( Q8tkWidget *dummy, void *p )
{
  switch( (int)p ){
  case DATA_TOP_EXIT:
    set_emu_mode( GO );
    break;
  case DATA_TOP_MONITOR:
    set_emu_mode( MONITOR );
    break;
  case DATA_TOP_QUIT:
    sub_top_quit();
    return;
  }
  q8tk_main_quit();
}

static	Q8tkWidget	*menu_top_button( void )
{
  int	i;
  Q8tkWidget *hbox, *b;
  const t_menudata *p = data_top_button;

  hbox = PACK_HBOX( NULL );
  {
#if 1
    b = PACK_CHECK_BUTTON( hbox,
			   GET_LABEL( data_top_status, 0 ),
			   get_top_status(),
			   cb_top_status, NULL );
    q8tk_misc_set_placement( b, 0, Q8TK_PLACEMENT_Y_CENTER );
#endif

    for( i=0; i<COUNTOF(data_top_button); i++, p++ ){

      if( p->val == DATA_TOP_MONITOR  &&  debug_mode == FALSE ) continue;

      b = PACK_BUTTON( hbox, GET_LABEL( p, 0 ),
		       cb_top_button, (void *)(p->val) );

      if( p->val == DATA_TOP_QUIT ){
	q8tk_accel_group_add( menu_accel, Q8TK_KEY_F12, b, "clicked" );
      }
      if( p->val == DATA_TOP_EXIT ){
	q8tk_accel_group_add( menu_accel, Q8TK_KEY_ESC, b, "clicked" );
      }
    }
  }
  q8tk_misc_set_placement( hbox, Q8TK_PLACEMENT_X_RIGHT, 0 );

  return hbox;
}


/*----------------------------------------------------------------------*/
				  /* QUIT�ܥ��󲡲����Ρ���ǧ�������� */

static	void	cb_top_quit_clicked( Q8tkWidget *dummy, void *p )
{
  dialog_destroy();

  if( (int)p ){
    set_emu_mode( QUIT );
    q8tk_main_quit();
  }
}
static	void	sub_top_quit( void )
{
  const t_menulabel *l = data_top_quit;

  dialog_create();
  {
    dialog_set_title( GET_LABEL( l, DATA_TOP_QUIT_TITLE ) );

    dialog_set_separator();

    dialog_set_button( GET_LABEL( l, DATA_TOP_QUIT_OK ),
		       cb_top_quit_clicked, (void*)TRUE );

    dialog_accel_key( Q8TK_KEY_F12 );

    dialog_set_button( GET_LABEL( l, DATA_TOP_QUIT_CANCEL ),
		       cb_top_quit_clicked, (void*)FALSE );

    dialog_accel_key( Q8TK_KEY_ESC );
  }
  dialog_start();
}

/*======================================================================*/

static Q8tkWidget *menu_top( void )
{
  int	i;
  const t_menudata *l = data_top;
  Q8tkWidget *note_fake[ COUNTOF(menu_fkey) ];
  Q8tkWidget *win, *vbox, *notebook, *w;

  win = q8tk_window_new( Q8TK_WINDOW_TOPLEVEL );
  menu_accel = q8tk_accel_group_new();
  q8tk_accel_group_attach( menu_accel, win );
  q8tk_widget_show( win );

  vbox = PACK_VBOX( NULL );
  {
    {
		/* �ƥ�˥塼��Ρ��ȥڡ����˾褻�Ƥ��� */

      notebook = q8tk_notebook_new();
      {
	for( i=0; i<COUNTOF(menu_page); i++ ){

	  w = (*menu_page[i].menu_func)();
	  q8tk_notebook_append( notebook, w,
				GET_LABEL( l, menu_page[i].data_num ) );

	  if( i<COUNTOF(menu_fkey) ){
	    note_fake[i] = q8tk_button_new();
	    q8tk_signal_connect( note_fake[i], "clicked",
				 menu_fkey[i].cb_func, notebook );
	    q8tk_accel_group_add( menu_accel, menu_fkey[i].key,
				  note_fake[i], "clicked" );
	  }
	}

	for( ; i < COUNTOF(menu_fkey); i++ ){
	  note_fake[i] = q8tk_button_new();
	  q8tk_signal_connect( note_fake[i], "clicked",
			       menu_fkey[i].cb_func, notebook );
	  q8tk_accel_group_add( menu_accel, menu_fkey[i].key,
				note_fake[i], "clicked" );
	}
      }
      q8tk_widget_show( notebook );
      q8tk_box_pack_start( vbox, notebook );
    }
    {
	/* ���Ĥ��ϡ��ܥ��� */

      w = menu_top_button();
      q8tk_box_pack_start( vbox, w );
    }
  }
  q8tk_container_add( win, vbox );


	/* �Ρ��ȥ֥å����֤��ޤ� */
  return notebook;
}





/****************************************************************/
/* ��˥塼�⡼�� �ᥤ�����					*/
/****************************************************************/

static	void	menu_init( void )
{
  int	i;

  for( i=0; i<0x10; i++ ){			/* ������������������ */
    if     ( i==0x08 ) key_scan[i] |= 0xdf;		/* ���ʤϻĤ� */
    else if( i==0x0a ) key_scan[i] |= 0x7f;		/* CAPS��Ĥ� */
    else               key_scan[i]  = 0xff;
  }

  menu_boot_dipsw      = boot_dipsw;
  menu_boot_from_rom   = boot_from_rom;
  menu_boot_basic      = boot_basic;
  menu_boot_clock_4mhz = boot_clock_4mhz;
  menu_boot_version    = ROM_VERSION;
  menu_boot_baudrate   = baudrate_sw;
  menu_boot_sound_board= sound_board;

  widget_reset_boot    = NULL;


  /*screen_buf_init();*/	/* �ä��ʤ��Ƥ������񤭴�����Τ�OK ? */

  status_message( 0, 0, " MENU " );
  status_message( 1, 0, "<ESC> key to return" );
  status_message( 2, 0, NULL );
  draw_status();
}



void	menu_main( void )
{
  Q8tkWidget *notebook;
  int mouse_cursor_exist;
  int cpu_timing_save = cpu_timing;

  menu_init();


  q8tk_init();
  {
    if     ( strcmp( menu_kanji_code, menu_kanji_code_euc  )==0 )
    {
      q8tk_set_kanjicode( Q8TK_KANJI_EUC );
    }
    else if( strcmp( menu_kanji_code, menu_kanji_code_sjis )==0 )
    {
      q8tk_set_kanjicode( Q8TK_KANJI_SJIS );
    }else
    {
      q8tk_set_kanjicode( Q8TK_KANJI_ANK );
    }

    mouse_cursor_exist = set_mouse_state();
    q8tk_set_cursor( mouse_cursor_exist ? FALSE : TRUE );

    notebook = menu_top();

    q8tk_notebook_set_page( notebook, menu_last_page );
    {
      q8tk_main();
    }
    menu_last_page = q8tk_notebook_current_page( notebook );
  }
  q8tk_term();


  if( cpu_timing_save != cpu_timing ){
    emu_reset();
  }

  pc88main_bus_setup();
  pc88sub_bus_setup();

  if( next_emu_mode() != QUIT ){
    screen_buf_init();
  }
}
