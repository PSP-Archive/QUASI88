/************************************************************************/
/*									*/
/* ���̤�ɽ��								*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "screen.h"
#include "screen-func.h"
#include "graph.h"

#include "crtcdmac.h"
#include "pc88main.h"
#include "memory.h"

#include "status.h"
#include "suspend.h"

#include "intr.h"



PC88_PALETTE_T	vram_bg_palette;	/* OUT[52/54-5B]		*/
PC88_PALETTE_T	vram_palette[8];	/*		�Ƽ�ѥ�å�	*/

byte	sys_ctrl;			/* OUT[30] SystemCtrl		*/
byte	grph_ctrl;			/* OUT[31] GraphCtrl		*/
byte	grph_pile;			/* OUT[53] �Ť͹�碌		*/



char	screen_update[ 0x4000*2 ];	/* ����ɽ����ʬ�����ե饰	*/
int	screen_update_force = TRUE;	/* ���̶��������ե饰		*/
int	screen_update_palette = TRUE;	/* �ѥ�åȹ����ե饰		*/



int	frameskip_rate  = 1;	/* ����ɽ���ι����ֳ�	*/
int	monitor_analog  = TRUE;			/* ���ʥ���˥���     */

int	use_auto_skip = TRUE;		/* �����ȥե졼�ॹ���åפ���Ѥ��� */
int	do_skip_draw = FALSE;		/* �����꡼��ؤ�����򥹥��åפ��� */
int	already_skip_draw = FALSE;	/* �����åפ�����		    */

static	int	frame_counter = 0;	/* �ե졼�ॹ���å��ѤΥ�����	*/
static	int	blink_ctrl_cycle   = 1;	/* ��������ɽ���ѤΥ�����	*/
static	int	blink_ctrl_counter = 0;	/*              ��		*/





int	hide_mouse = FALSE;		/* �ޥ����򱣤����ɤ���		*/
int	grab_mouse = FALSE;		/* ����֤��뤫�ɤ���		*/


int	use_interlace = 0;		/* ���󥿡��졼��ɽ�� (-1,0,1)	*/
int	use_half_interp = TRUE;		/* ���̥�����Ⱦʬ��������֤��� */


const SCREEN_SIZE_TABLE screen_size_tbl[ SCREEN_SIZE_END ] =
{
  {  320,  200,   0,  20, },		/* SCREEN_SIZE_HALF	320x200	*/
  {  640,  400,   0,  40, },		/* SCREEN_SIZE_FULL	640x400	*/
#ifdef	SUPPORT_DOUBLE
  { 1280,  800,   0,  80, },		/* SCREEN_SIZE_DOUBLE	1280x800*/
#endif
};

int	screen_size = SCREEN_SIZE_FULL;	/* ���̥���������		*/
int	use_fullscreen = FALSE;		/* ������ɽ������		*/


int	WIDTH  = 0;			/* ����Хåե���������		*/
int	HEIGHT = 0;			/* ����Хåե��ĥ�����		*/
int	DEPTH  = 8;			/* ���ӥåȿ�	(8 or 16 or 32)	*/
int	SCREEN_W = 0;			/* ���̲������� (320/640/1280)	*/
int	SCREEN_H = 0;			/* ���̽ĥ����� (200/400/800)	*/

char	*screen_buf;			/* ����Хåե���Ƭ		*/
char	*screen_start;			/* ������Ƭ			*/


int	show_status = TRUE;		/* ���ơ�����ɽ��̵ͭ		*/

char	*status_buf;			/* ���ơ��������� ��Ƭ		*/
char	*status_start[3];		/* ���ơ��������� ��Ƭ		*/
int	status_sx[3];			/* ���ơ��������襵����		*/
int	status_sy[3];


Ulong	color_pixel[16];			/* ��������		*/
Ulong	color_half_pixel[16][16];		/* ���䴰���ο�������	*/
Ulong	black_pixel;				/* ���ο�������		*/
Ulong	status_pixel[ STATUS_COLOR_END ];	/* ���ơ������ο�������	*/







/***********************************************************************
 * �ƥ��������� (�������뤪��ӡ�ʸ��°��������) �����Υ������
 *	CRTC �� frameskip_rate, blink_cycle ���ѹ�����뤿�Ӥ˸ƤӽФ�
 ************************************************************************/
void	blink_ctrl_update( void )
{
  int	wk;

  wk = blink_cycle / frameskip_rate;

  if( wk==0 ||
     !( blink_cycle -wk*frameskip_rate < (wk+1)*frameskip_rate -blink_cycle ) )
    wk++;
  
  blink_ctrl_cycle = wk;
  blink_ctrl_counter = blink_ctrl_cycle;
}



/***********************************************************************
 * �ե졼�५���󥿽����
 *	���ե졼��ϡ�ɬ��ɽ������롣(�����åפ���ʤ�)
 ************************************************************************/
void	reset_frame_counter( void )
{
  frame_counter = 0;
}



/***********************************************************************
 * ����κݤ˻��Ѥ��롢�ºݤΥѥ�åȾ������� syspal �˥��åȤ���
 ************************************************************************/
void	setup_palette( SYSTEM_PALETTE_T syspal[16] )
{
  int     i;

	/* VRAM �� ���顼�ѥ�å�����   syspal[0]��[7] */

  if( grph_ctrl & GRPH_CTRL_COLOR ){		/* VRAM ���顼 */

    if( monitor_analog ){
      for( i=0; i<8; i++ ){
	syspal[i].red   = vram_palette[i].red   * 73 / 2;
	syspal[i].green = vram_palette[i].green * 73 / 2;
	syspal[i].blue  = vram_palette[i].blue  * 73 / 2;
      }
    }else{
      for( i=0; i<8; i++ ){
	syspal[i].red   = vram_palette[i].red   ? 0xff : 0;
	syspal[i].green = vram_palette[i].green ? 0xff : 0;
	syspal[i].blue  = vram_palette[i].blue  ? 0xff : 0;
      }
    }

  }else{					/* VRAM ��� */

    if( monitor_analog ){
      syspal[0].red   = vram_bg_palette.red   * 73 / 2;
      syspal[0].green = vram_bg_palette.green * 73 / 2;
      syspal[0].blue  = vram_bg_palette.blue  * 73 / 2;
    }else{
      syspal[0].red   = vram_bg_palette.red   ? 0xff : 0;
      syspal[0].green = vram_bg_palette.green ? 0xff : 0;
      syspal[0].blue  = vram_bg_palette.blue  ? 0xff : 0;
    }
    for( i=1; i<8; i++ ){
      syspal[i].red   = 0;
      syspal[i].green = 0;
      syspal[i].blue  = 0;
    }

  }


	/* TEXT �� ���顼�ѥ�å�����   syspal[8]��[15] */

  if( grph_ctrl & GRPH_CTRL_COLOR ){		/* VRAM ���顼 */

    for( i=8; i<16; i++ ){				/* TEXT ����ξ��� */
      syspal[i].red   = (i&0x02) ? 0xff : 0;		/* ��=[8],��=[15] �� */
      syspal[i].green = (i&0x04) ? 0xff : 0;		/* �Ȥ��Τ�����ʤ�  */
      syspal[i].blue  = (i&0x01) ? 0xff : 0;
    }

  }else{					/* VRAM ���   */

    if( misc_ctrl & MISC_CTRL_ANALOG ){			/* ���ʥ��ѥ�åȻ�*/

      if( monitor_analog ){
	for( i=8; i<16; i++ ){
	  syspal[i].red   = vram_palette[i&0x7].red   * 73 / 2;
	  syspal[i].green = vram_palette[i&0x7].green * 73 / 2;
	  syspal[i].blue  = vram_palette[i&0x7].blue  * 73 / 2;
	}
      }else{
	for( i=8; i<16; i++ ){
	  syspal[i].red   = vram_palette[i&0x7].red   ? 0xff : 0;
	  syspal[i].green = vram_palette[i&0x7].green ? 0xff : 0;
	  syspal[i].blue  = vram_palette[i&0x7].blue  ? 0xff : 0;
	}
      }

    }else{						/* �ǥ�����ѥ�åȻ�*/
      for( i=8; i<16; i++ ){
	syspal[i].red   = (i&0x02) ? 0xff : 0;
	syspal[i].green = (i&0x04) ? 0xff : 0;
	syspal[i].blue  = (i&0x01) ? 0xff : 0;
      }
    }

  }
}



/*======================================================================
 * �ƥ�����VRAM�Υ��ȥ�ӥ塼�Ȥ����ѥ�������ꤹ��
 *
 *	�Хåե���2�Ĥ��ꡢ��ߤ����ؤ��ƻ��Ѥ��롣
 *	���̽񤭴����κݤϡ�����2�ĤΥХåե�����Ӥ����Ѳ���
 *	���ä���ʬ�����򹹿����롣
 *
 *	����ϡ�16bit�ǡ����8bit��ʸ�������ɡ����̤�°����
 *		��������ե��å��⡼�ɡ���������饤��
 *		���åѡ��饤�󡢥�������åȡ���С���
 *		+---------------------+--+--+--+--+--+--+--+--+
 *		|    ASCII 8bit       |��|��|��|GR|LO|UP|SC|RV|
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	BLINK°���ϡ���������̵�롢�������ϥ�������åȡ�
 *
 *	����ˡ���������å�°���ξ��� ʸ�������ɤ� 0 ���ִ����롣
 *	(ʸ��������==0��̵���Ƕ���Ȥ��Ƥ���Τ�)
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	     ��	|    ASCII == 0       |��|��|��|��|LO|UP|��|RV|
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	        ����ե��å��⡼�ɤȥ�������å�°����ä��Ƥ�OK������
 *		������������åѡ��饤�󡢥�С�����ͭ���ʤΤǻĤ���
 *
 *======================================================================*/

int	text_attr_flipflop = 0;
Ushort	text_attr_buf[2][2048];		/* ���ȥ�ӥ塼�Ⱦ���	*/
			/* �� 80ʸ��x25��=2000��­���Τ�����	*/
			/* ;ʬ�˻Ȥ��Τǡ�¿��˳��ݤ��롣	*/
				   

static	void	make_text_attr( void )
{
  int		global_attr  = (ATTR_G|ATTR_R|ATTR_B);
  int		global_blink = FALSE;
  int		i, j, tmp;
  int		column, attr, attr_rest;
  word		char_start_addr, attr_start_addr;
  word		c_addr, a_addr;
  Ushort	*text_attr = &text_attr_buf[ text_attr_flipflop ][0];


	/* CRTC �� DMAC ��ߤޤäƤ����� */
	/*  (ʸ���⥢�ȥ�ӥ塼�Ȥ�̵��)   */

  if( text_display==TEXT_DISABLE ){		/* ASCII=0���򿧡������ʤ� */
    for( i=0; i<CRTC_SZ_LINES; i++ ){		/* �ǽ�������롣	   */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ =  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* ������ȿž�䥫�������ʤ������������  */
  }



	/* �Υ󡦥ȥ�󥹥ڥ����ȷ��ξ�� */
	/* (1ʸ���֤��ˡ�VRAM��ATTR ������) */

			/* �ġġ� ���ܺ����� 				*/
			/*	CRTC������ѥ����󤫤餷�ơ�����˹Ԥ�	*/
			/*	�Ǹ��°����������⤢�ꤨ����������?	*/

  if( crtc_attr_non_separate ){

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + 1;

    for( i=0; i<crtc_sz_lines; i++ ){

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;

      for( j=0; j<CRTC_SZ_COLUMNS; j+=2 ){		/* °�������������ɤ�*/
	attr = main_ram[ a_addr ];			/* �Ѵ�����°�����*/
	a_addr += 2;					/* ���������롣    */
	global_attr =( global_attr & COLOR_MASK ) |
		     ((attr &  MONO_GRAPH) >> 3 ) |
		     ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
		     ((attr &  MONO_SECRET) << 1 );

					/* BLINK��OFF����SECRET����    */
	if( (attr & MONO_BLINK) && ((blink_counter&0x03)==0) ){
	  global_attr |= ATTR_SECRET;
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;
	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){
	if( ++i < crtc_sz_lines ){
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }
    for( ; i<CRTC_SZ_LINES; i++ ){		/* �Ĥ�ιԤϡ�SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24�������к�)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }else{

	/* �ȥ�󥹥ڥ����ȷ��ξ�� */
	/* (�ԤκǸ�ˡ�ATTR������)   */

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + crtc_sz_columns;

    for( i=0; i<crtc_sz_lines; i++ ){			/* ��ñ�̤�°������ */

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;


      attr_rest = 0;						/*°������� */
      for( j=0; j<=CRTC_SZ_COLUMNS; j++ ) text_attr[j] = 0;	/* [0]��[80] */


      for( j=0; j<crtc_sz_attrs; j++ ){			/* °����������ܤ� */
	column = main_ram[ a_addr++ ];			/* ����˳�Ǽ       */
	attr   = main_ram[ a_addr++ ];

	if( j!=0 && column==0    ) column = 0x80;		/* �ü����?*/
	if( j==0 && column==0x80 ){column = 0;
/*				   global_attr = (ATTR_G|ATTR_R|ATTR_B);
				   global_blink= FALSE;  }*/}

	if( column==0x80  &&  !attr_rest ){			/* 8bit�ܤ� */
	  attr_rest = attr | 0x100;				/* ���ѺѤ� */
	}							/* �ե饰   */
	else if( column <= CRTC_SZ_COLUMNS  &&  !text_attr[ column ] ){
	  text_attr[ column ] = attr | 0x100;
	}
      }


      if( !text_attr[0] && attr_rest ){			/* �����-1�ޤ�°����*/
	for( j=CRTC_SZ_COLUMNS; j; j-- ){		/* ͭ�����Ȥ�������*/
	  if( text_attr[j] ){				/* ������(�����ʹ� */
	    tmp          = text_attr[j];		/* °����ͭ�����Ȥ���*/
	    text_attr[j] = attr_rest;			/* �դ����¤��ؤ���) */
	    attr_rest    = tmp;
	  }
	}
	text_attr[0] = attr_rest;
      }


      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* °�������������ɤ�*/
							/* �Ѵ�����°�����*/
	if( ( attr = *text_attr ) ){			/* ���������롣    */
	  if( crtc_attr_color ){
	    if( attr & COLOR_SWITCH ){
	      global_attr =( global_attr & MONO_MASK ) |
			   ( attr & (COLOR_G|COLOR_R|COLOR_B|COLOR_GRAPH));
	    }else{
	      global_attr =( global_attr & (COLOR_MASK|ATTR_GRAPH) ) |
			   ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			   ((attr &  MONO_SECRET) << 1 );
	      global_blink= (attr & MONO_BLINK);
	    }
	  }else{
	    global_attr =( global_attr & COLOR_MASK ) |
			 ((attr &  MONO_GRAPH) >> 3 ) |
			 ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			 ((attr &  MONO_SECRET) << 1 );
	    global_blink= (attr & MONO_BLINK);
	  }
					/* BLINK��OFF����SECRET����    */
	  if( global_blink && ((blink_counter&0x03)==0) ){
	    global_attr =  global_attr | ATTR_SECRET;
	  }
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){				/* 1�����Ф��������*/
	if( ++i < crtc_sz_lines ){			/* ���ιԤ�SECRET�� */
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* ���롣         */
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }

    for( ; i<CRTC_SZ_LINES; i++ ){		/* �Ĥ�ιԤϡ�SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24�������к�)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }



	/* CRTC �� DMAC ��ư���Ƥ��뤱�ɡ� �ƥ����Ȥ���ɽ�� */
	/* ��VRAM����ξ�� (���ȥ�ӥ塼�Ȥο�������ͭ��)  */

  if( text_display==TEXT_ATTR_ONLY ){

    text_attr = &text_attr_buf[ text_attr_flipflop ][0];

    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ &=  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* ������ȿž�䥫����������ס������Ǥ����  */
  }




		/* ����ȿž���� */

  if( crtc_reverse_display && (grph_ctrl & GRPH_CTRL_COLOR)){
    text_attr = &text_attr_buf[ text_attr_flipflop ][0];
    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ ^= ATTR_REVERSE;
      }
    }
  }

		/* ��������ɽ������ */

  if( 0 <= crtc_cursor[0] && crtc_cursor[0] < crtc_sz_columns &&
      0 <= crtc_cursor[1] && crtc_cursor[1] < crtc_sz_lines   ){
    if( !crtc_cursor_blink || (blink_counter&0x01) ){
      text_attr_buf[ text_attr_flipflop ][ crtc_cursor[1]*80 + crtc_cursor[0] ]
							^= crtc_cursor_style;
    }
  }


	/* ��������å�°������ (ʸ�������� 0x00 ���ִ�) */

  text_attr = &text_attr_buf[ text_attr_flipflop ][0];
  for( i=0; i<CRTC_SZ_LINES; i++ ){
    for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
      if( *text_attr & ATTR_SECRET ){		/* SECRET °���ϡ�������00�� */
	*text_attr &= (COLOR_MASK|ATTR_UPPER|ATTR_LOWER|ATTR_REVERSE);
      }
      text_attr ++;
    }
  }

}






/***********************************************************************
 * ���ꤵ�줿ʸ��������(°����ʸ��)��ꡢ�ե���Ȼ����ǡ�������������
 *
 *	int attr	�� ʸ�������ɡ� text_attr_buf[]���ͤǤ��롣
 *	T_GRYPH *gryph	�� gryph->b[0]��[7] �� �ե���ȤΥӥåȥޥåפ�
 *			   ��Ǽ����롣(20�Ի��ϡ�b[0]��[9]�˳�Ǽ)
 *	int *color	�� �ե���Ȥο�����Ǽ����롣�ͤϡ� 8��15
 *
 *	�����ǡ����ϡ�char 8��10�ĤʤΤ�������©�ʹ�®���Τ���� long ��
 *	�����������Ƥ��롣���Τ��ᡢ T_GRYPH �Ȥ���̯�ʷ���ȤäƤ��롣
 *	(����ס���������͡�)
 *
 *	��)	���ȥ�ӥ塼�Ⱦ���ɬ�פʤΤǡ�
 *		ͽ�� make_text_attr_table( ) ��Ƥ�Ǥ�������
 ************************************************************************/

void	get_font_gryph( int attr, T_GRYPH *gryph, int *color )
{
  int	chara;
  bit32	*src;
  bit32	*dst = (bit32 *)gryph;

  *color = ((attr & COLOR_MASK) >> 5) | 8;


  if( ( attr & ~(COLOR_MASK|ATTR_REVERSE) )==0 ){

    if( ( attr & ATTR_REVERSE ) == 0 ){		/* ����ե���Ȼ� */

      *dst++ = 0;
      *dst++ = 0;
      *dst   = 0;

    }else{					/* �٥��ե���Ȼ� */

      *dst++ = 0xffffffff;
      *dst++ = 0xffffffff;
      *dst   = 0xffffffff;
    }

  }else{					/* �̾�ե���Ȼ� */

    chara = attr >> 8;

    if( attr & ATTR_GRAPH )
      src = (bit32 *)&font_rom[ (chara | 0x100)*8 ];
    else
      src = (bit32 *)&font_rom[ (chara        )*8 ];

					/* �ե���Ȥ�ޤ���������˥��ԡ� */
    *dst++ = *src++;
    *dst++ = *src;
    *dst   = 0;

					/* °���ˤ����������ե���Ȥ�ù�*/
    if( attr & ATTR_UPPER ) gryph->b[ 0 ] |= 0xff;
    if( attr & ATTR_LOWER ) gryph->b[ crtc_font_height-1 ] |= 0xff;
    if( attr & ATTR_REVERSE ){
      dst -= 2;
      *dst++ ^= 0xffffffff;
      *dst++ ^= 0xffffffff;
      *dst   ^= 0xffffffff;
    }
  }
}





/***********************************************************************
 * GVRAM/TVRAM������Хåե���ž������ؿ��Ρ��ꥹ�Ȥ����
 *	���δؿ��ϡ� bpp �� ������ �� ���ե����� ���ѹ����˸ƤӽФ���
 ************************************************************************/

static	int  ( *vram2screen_list[4][4][2] )( void );
static	void ( *screen_buf_init_p )( void );

static	int  ( *menu2screen_p )( void );

static	void ( *status2screen_p )( int kind, byte pixmap[], int w, int h );
static	void ( *status_buf_init_p )( void );
static	void ( *status_buf_clear_p )( void );


typedef	int		( *V2S_FUNC_TYPE )( void );
typedef	V2S_FUNC_TYPE	V2S_FUNC_LIST[4][4][2];

INLINE	void	set_vram2screen_list( void )
{
  V2S_FUNC_LIST *list = NULL;


  if( DEPTH <= 8 ){		/* ----------------------------------------- */

#ifdef	SUPPORT_8BPP
    switch( now_screen_size ){
    case SCREEN_SIZE_FULL:
      if     ( use_interlace == 0 ){ list = &vram2screen_list_F_N__8; }
      else if( use_interlace >  0 ){ list = &vram2screen_list_F_I__8; }
      else                         { list = &vram2screen_list_F_S__8; }
      menu2screen_p = menu2screen_F_N__8;
      break;
    case SCREEN_SIZE_HALF:
      if( now_half_interp )   { list = &vram2screen_list_H_P__8;
				menu2screen_p = menu2screen_H_P__8; }
      else                    { list = &vram2screen_list_H_N__8;
				menu2screen_p = menu2screen_H_N__8; }
      break;
#ifdef	SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:
      if     ( use_interlace == 0 ){ list = &vram2screen_list_D_N__8; }
      else if( use_interlace >  0 ){ list = &vram2screen_list_D_I__8; }
      else                         { list = &vram2screen_list_D_S__8; }
      menu2screen_p = menu2screen_D_N__8;
      break;
#endif
    }
    screen_buf_init_p = screen_buf_init__8;
    status2screen_p   = status2screen__8;
    status_buf_init_p = status_buf_init__8;
    status_buf_clear_p= status_buf_clear__8;
#else
    fprintf( stderr, "Error! This version is not support 8bpp !\n" );
    exit(1);
#endif

  } else if( DEPTH <= 16 ){	/* ----------------------------------------- */

#ifdef	SUPPORT_16BPP
    switch( now_screen_size ){
    case SCREEN_SIZE_FULL:
      if     ( use_interlace == 0 ){ list = &vram2screen_list_F_N_16; }
      else if( use_interlace >  0 ){ list = &vram2screen_list_F_I_16; }
      else                         { list = &vram2screen_list_F_S_16; }
      menu2screen_p = menu2screen_F_N_16;
      break;
    case SCREEN_SIZE_HALF:
      if( now_half_interp )   { list = &vram2screen_list_H_P_16;
				menu2screen_p = menu2screen_H_P_16; }
      else                    { list = &vram2screen_list_H_N_16;
				menu2screen_p = menu2screen_H_N_16; }
      break;
#ifdef	SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:
      if     ( use_interlace == 0 ){ list = &vram2screen_list_D_N_16; }
      else if( use_interlace >  0 ){ list = &vram2screen_list_D_I_16; }
      else                         { list = &vram2screen_list_D_S_16; }
      menu2screen_p = menu2screen_D_N_16;
      break;
#endif
    }
    screen_buf_init_p = screen_buf_init_16;
    status2screen_p   = status2screen_16;
    status_buf_init_p = status_buf_init_16;
    status_buf_clear_p= status_buf_clear_16;
#else
    fprintf( stderr, "Error! This version is not support 16bpp !\n" );
    exit(1);
#endif

  }else if( DEPTH <= 32 ){	/* ----------------------------------------- */

#ifdef	SUPPORT_32BPP
    switch( now_screen_size ){
    case SCREEN_SIZE_FULL:
      if     ( use_interlace == 0 ){ list = &vram2screen_list_F_N_32; }
      else if( use_interlace >  0 ){ list = &vram2screen_list_F_I_32; }
      else                         { list = &vram2screen_list_F_S_32; }
      menu2screen_p = menu2screen_F_N_32;
      break;
    case SCREEN_SIZE_HALF:
      if( now_half_interp )   { list = &vram2screen_list_H_P_32;
				menu2screen_p = menu2screen_H_P_32; }
      else                    { list = &vram2screen_list_H_N_32;
				menu2screen_p = menu2screen_H_N_32; }
      break;
#ifdef	SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:
      if     ( use_interlace == 0 ){ list = &vram2screen_list_D_N_32; }
      else if( use_interlace >  0 ){ list = &vram2screen_list_D_I_32; }
      else                         { list = &vram2screen_list_D_S_32; }
      menu2screen_p = menu2screen_D_N_32;
      break;
#endif
    }
    screen_buf_init_p = screen_buf_init_32;
    status2screen_p   = status2screen_32;
    status_buf_init_p = status_buf_init_32;
    status_buf_clear_p= status_buf_clear_32;
#else
    fprintf( stderr, "Error! This version is not support 32bpp !\n" );
    exit(1);
#endif

  }

  memcpy( vram2screen_list, list, sizeof(vram2screen_list) );
}



/***********************************************************************
 * GVRAM/TVRAM �� screen_buf ��ž������
 *
 *	int method == V_DIF �� screen_update �˴�Ť�����ʬ������ž��
 *		   == V_ALL �� ���̤��٤Ƥ�ž��
 *
 *	�����     == -1    �� ž���ʤ� (���̤��Ѳ��ʤ�)
 *		   != -1    �� ��̤��� 8�ӥåȤ��Ĥˡ�x0, y0, x1, y1 ��
 *			       4�Ĥ� unsigned �ͤ����åȤ���롣�����ǡ�
 *				    ( x0*8, y0*2 )-( x1*8, y1*2 )
 *			       ��ɽ������ϰϤ���ž�������ΰ�Ȥʤ롣
 *
 *	ͽ�ᡢ set_vram2screen_list �Ǵؿ��ꥹ�Ȥ��������Ƥ�������
 ************************************************************************/

static	int	vram2screen( int method )
{
  int vram_mode, text_mode;

  if( sys_ctrl & SYS_CTRL_80 ){			/* �ƥ����Ȥιԡ��� */
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_80x25; }
    else                     { text_mode = V_80x20; }
  }else{
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_40x25; }
    else                     { text_mode = V_40x20; }
  }

  if( grph_ctrl & GRPH_CTRL_VDISP ){		/* VRAM ɽ������ */

    if( grph_ctrl & GRPH_CTRL_COLOR ){			/* ���顼 */
      vram_mode = V_COLOR;
    }else{
      if( grph_ctrl & GRPH_CTRL_200 ){			/* ��� */
	vram_mode = V_MONO;
      }else{						/* 400�饤�� */
	vram_mode = V_HIRESO;
      }
    }

  }else{					/* VRAM ɽ�����ʤ� */

    vram_mode = V_UNDISP;
  }


  return (vram2screen_list[ vram_mode ][ text_mode ][ method ])();
}



/***********************************************************************
 * screen_buf �ν����
 *	screen_buf ����ǥ��ꥢ(�ܡ�������ʬ��ޤ�)����
 *	���ơ�����ɽ���κݤϤ����������ɽ�����롣
 *
 *	���δؿ��ϡ�������ɥ��������䡢ɽ�����å����ѹ����줿��
 *	( use_interlace, use_half_interp �ʤɤ��ѿ��ѹ��� ) �˸ƤӽФ���
 ************************************************************************/
void	screen_buf_init( void )
{
  int     i;
  SYSTEM_PALETTE_T	syspal[16];

  for( i=0; i<16; i++ ){		/* ���ߡ��ο��ǤȤ⤫������� */
    syspal[i].red   = (i&2) ? 255 : 0;
    syspal[i].green = (i&4) ? 255 : 0;
    syspal[i].blue  = (i&1) ? 255 : 0;
  }
  trans_palette( syspal );

					/* ž����ɽ���ؿ��Υꥹ�Ȥ����� */
  set_vram2screen_list();

  (screen_buf_init_p)();		/* ���������ꥢ(�ܡ�������) */

  if( now_status ){			/* ���ơ������������ꥢ */
    (status_buf_init_p)();
  }else{
    (status_buf_clear_p)();
  }

  put_image_all();
}



/***********************************************************************
 * status_buf �ν����
 *	status_buf �˽������ɽ��(ɽ����)���ʤ��� ���ǥ��ꥢ(��ɽ����)
 *
 *	���δؿ��ϡ����ơ�����ɽ������ɽ�����ػ��˸ƤӽФ���
 ************************************************************************/
void	status_buf_init( void )
{
  if( now_status ){
    (status_buf_init_p)();
  }else{
    (status_buf_clear_p)();
  }

//  put_image( -1, -1, -1, -1, TRUE, TRUE, TRUE );
}



/***********************************************************************
 * ���᡼��ž�� (ɽ��)
 *
 *	���δؿ��ϡ�ɽ�������ߥ� (�� 1/60����) �˸ƤӽФ���롣
 *		VRAM ��ɽ���ϻ��ꤵ�줿�ե졼����˹Ԥ���
 *		���ơ�������ɽ���ϡ�����Ԥ���
 ************************************************************************/

/*
 *
 */
INLINE	int	vram2screen_core( void )
{
  int rect = -1;
  SYSTEM_PALETTE_T syspal[16];

  if( screen_update_palette ){		/* �ѥ�åȤ򥷥��ƥ��ž�� */
    setup_palette( syspal );
    trans_palette( syspal );
    screen_update_palette = FALSE; 
  }

	/* VRAM�����ե饰 screen_update ���㳰����			*/
	/*	VRAM��ɽ���ξ�硢�����ե饰�ϰ�̵̣���Τǥ��ꥢ����	*/
	/*	400�饤��ξ�硢�����ե饰����̲�Ⱦʬ�ˤ��ĥ����	*/

  if( screen_update_force == FALSE ){
    if( ! (grph_ctrl & GRPH_CTRL_VDISP) ){			/* ��ɽ��    */
      memset( screen_update, 0, sizeof( screen_update )/2 );
    }
    if( ! (grph_ctrl & (GRPH_CTRL_COLOR|GRPH_CTRL_200)) ){	/* 400�饤�� */
      memcpy( &screen_update[80*200], screen_update, 80*200 );
    }
  }

  make_text_attr();			/* TVRAM �� °����������	  */
					/* VRAM/TEXT �� screen_buf ž��	  */
  rect = vram2screen( screen_update_force ? V_ALL : V_DIF );

  text_attr_flipflop ^= 1;		/* VRAM�����ե饰���򥯥ꥢ */
  memset( screen_update, 0, sizeof( screen_update ));
  screen_update_force = FALSE;

  return rect;
}

/*
 *
 */
INLINE	int	status2screen_core( int force_update )
{
  int i;
  int flag;

  flag = status_update( force_update );

  for( i=0; i<3; i++ ){
    if( flag & (1<<i) ){
      (status2screen_p)( i, status_info[i].pixmap,
			 status_info[i].w, status_info[i].h );
    }
  }
  return flag;
}



void	draw_screen( void )
{
  int rect = -1;
  int flag;

  flag = status2screen_core( FALSE );

  if( (frame_counter%frameskip_rate)==0 ){	/* ����ե졼�प���˽��� */

	/* ��������̵���ξ�硢�����ȥ����åפ�̵�� */

    if( no_wait || !use_auto_skip || !do_skip_draw ){

      rect = vram2screen_core();	/* VRAM���Хåե� ž������	*/
	  put_image_all();
    }else{

      already_skip_draw = TRUE;		/* ��ž�����ϡ������åץե饰ON	*/

    }

    if( --blink_ctrl_counter == 0 ){	/* �����������ǥ������	*/
      blink_ctrl_counter = blink_ctrl_cycle;
      blink_counter ++;
    }
  }

  ++ frame_counter;


}



/***********************************************************************
 * ���᡼��ž�� (����������)
 ************************************************************************/
void	draw_screen_force( void )
{
  int rect;
  int flag;

  flag = status2screen_core( TRUE );


  screen_update_force   = TRUE;
  screen_update_palette = TRUE;

  rect = vram2screen_core();

  put_image_all();

}




/***********************************************************************
 * ���᡼��ž�� (���ơ������Τ�)
 *
 *	���ߥ�졼�������ʳ��ǡ����ơ������Τߤ򹹿�����Ȥ��˸ƤӽФ�
 *	���ߥ�졼�������ϡ� draw_screen() ��ƤӽФ��Ф褤�� 
 ************************************************************************/

void	draw_status( void )
{
  int flag;

  flag = status2screen_core( FALSE );

}






/***********************************************************************
 * ���᡼��ž�� (��˥塼������)
 ************************************************************************/
void	draw_menu_screen( void )
{
  int rect = -1;

  rect = (menu2screen_p)();

  if( rect != -1 ){
/*
  int x0=rect>>24,y0=(rect>>16)&0xff,x1=(rect>>8)&0xff,y1 =rect&0xff;
  printf("%d %d (%d %d)\n",x0,y0,x1-x0,y1-y0);fflush(stdout);
*/
	put_image_all();
  }
}



/***********************************************************************
 * ���᡼��ž�� (��˥塼������  ����������)
 ************************************************************************/
void	draw_menu_screen_force( void )
{
  int rect = -1;
  int flag;

  flag = status2screen_core( TRUE );


  rect = (menu2screen_p)();

  put_image_all();

}




/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"SCRN"

static	T_SUSPEND_W	suspend_screen_work[]=
{
  { TYPE_CHAR,	&vram_bg_palette.blue,	},
  { TYPE_CHAR,	&vram_bg_palette.red,	},
  { TYPE_CHAR,	&vram_bg_palette.green,	},

  { TYPE_CHAR,	&vram_palette[0].blue,	},
  { TYPE_CHAR,	&vram_palette[0].red,	},
  { TYPE_CHAR,	&vram_palette[0].green,	},
  { TYPE_CHAR,	&vram_palette[1].blue,	},
  { TYPE_CHAR,	&vram_palette[1].red,	},
  { TYPE_CHAR,	&vram_palette[1].green,	},
  { TYPE_CHAR,	&vram_palette[2].blue,	},
  { TYPE_CHAR,	&vram_palette[2].red,	},
  { TYPE_CHAR,	&vram_palette[2].green,	},
  { TYPE_CHAR,	&vram_palette[3].blue,	},
  { TYPE_CHAR,	&vram_palette[3].red,	},
  { TYPE_CHAR,	&vram_palette[3].green,	},
  { TYPE_CHAR,	&vram_palette[4].blue,	},
  { TYPE_CHAR,	&vram_palette[4].red,	},
  { TYPE_CHAR,	&vram_palette[4].green,	},
  { TYPE_CHAR,	&vram_palette[5].blue,	},
  { TYPE_CHAR,	&vram_palette[5].red,	},
  { TYPE_CHAR,	&vram_palette[5].green,	},
  { TYPE_CHAR,	&vram_palette[6].blue,	},
  { TYPE_CHAR,	&vram_palette[6].red,	},
  { TYPE_CHAR,	&vram_palette[6].green,	},
  { TYPE_CHAR,	&vram_palette[7].blue,	},
  { TYPE_CHAR,	&vram_palette[7].red,	},
  { TYPE_CHAR,	&vram_palette[7].green,	},

  { TYPE_BYTE,	&sys_ctrl,		},
  { TYPE_BYTE,	&grph_ctrl,		},
  { TYPE_BYTE,	&grph_pile,		},

  { TYPE_INT,	&frameskip_rate,	},
  { TYPE_INT,	&monitor_analog,	},
  { TYPE_INT,	&use_auto_skip,		},
/*{ TYPE_INT,	&frame_counter,		},	����ͤǤ�����ʤ����� */
/*{ TYPE_INT,	&blink_ctrl_cycle,	},	����ͤǤ�����ʤ����� */
/*{ TYPE_INT,	&blink_ctrl_counter,	},	����ͤǤ�����ʤ����� */

  { TYPE_INT,	&use_interlace,		},
  { TYPE_INT,	&use_half_interp,	},

  { TYPE_END,	0			},
};


int	statesave_screen( void )
{
  if( statesave_table( SID, suspend_screen_work ) == STATE_OK ) return TRUE;
  else                                                          return FALSE;
}

int	stateload_screen( void )
{
  if( stateload_table( SID, suspend_screen_work ) == STATE_OK ) return TRUE;
  else                                                          return FALSE;
}
























/* �ǥХå��Ѥδؿ� */
void attr_misc(int line)
{
int i;

  text_attr_flipflop ^= 1;    
  for(i=0;i<80;i++){
    printf("%02X[%02X] ",
    text_attr_buf[text_attr_flipflop][line*80+i]>>8,
    text_attr_buf[text_attr_flipflop][line*80+i]&0xff );
  }
return;
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][9*80+i]>>8,
    text_attr_buf[text_attr_flipflop][9*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][10*80+i]>>8,
    text_attr_buf[text_attr_flipflop][10*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][11*80+i]>>8,
    text_attr_buf[text_attr_flipflop][11*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][12*80+i]>>8,
    text_attr_buf[text_attr_flipflop][12*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][13*80+i]>>8,
    text_attr_buf[text_attr_flipflop][13*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][14*80+i]>>8,
    text_attr_buf[text_attr_flipflop][14*80+i]&0xff );
  }
#if 0
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][15*80+i]>>8,
    text_attr_buf[0][15*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][16*80+i]>>8,
    text_attr_buf[0][16*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][17*80+i]>>8,
    text_attr_buf[0][17*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][18*80+i]>>8,
    text_attr_buf[0][18*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][19*80+i]>>8,
    text_attr_buf[0][19*80+i]&0xff );
  }
  printf("\n");
#endif
}
