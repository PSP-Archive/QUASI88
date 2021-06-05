/************************************************************************/
/*									*/
/* CRTC �� DMAC �ν���							*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "crtcdmac.h"

#include "screen.h"
#include "suspend.h"


int		text_display = TEXT_ENABLE;	/* �ƥ�����ɽ���ե饰	*/

int		blink_cycle;		/* ���Ǥμ���	8/16/24/32	*/
int		blink_counter = 0;	/* �������楫����		*/

int		dma_wait_count = 0;	/* DMA�Ǿ��񤹤륵�������	*/


static	int	crtc_command;
static	int	crtc_param_num;

static	byte	crtc_status;
static	byte	crtc_light_pen[2];
static	byte	crtc_load_cursor_position;


	int	crtc_active;		/* CRTC�ξ��� 0:CRTC��ư 1:CRTC��� */
	int	crtc_intr_mask;		/* CRTC�γ���ޥ��� ==3 ��ɽ��	    */
	int	crtc_cursor[2];		/* ����������� ��ɽ���λ���(-1,-1) */
	byte	crtc_format[5];		/* CRTC �������Υե����ޥå�	    */


	int	crtc_reverse_display;	/* ����ȿžɽ�� / �����̾�ɽ��	*/

	int	crtc_skip_line;		/* ����1�����Ф�ɽ�� / �����̾� */
	int	crtc_cursor_style;	/* �֥�å� / ������饤��	*/
	int	crtc_cursor_blink;	/* �������Ǥ��� �������Ǥ��ʤ�	*/
	int	crtc_attr_non_separate;	/* ����VRAM��ATTR ����ߤ��¤�	*/
	int	crtc_attr_color;	/* ���ĥ��顼 �������		*/
	int	crtc_attr_non_special;	/* ���ĹԤν���� ATTR ���¤�	*/

	int	CRTC_SZ_LINES	   =20;	/* ɽ�������� (20/25)		*/
#define		CRTC_SZ_COLUMNS	   (80)	/* ɽ������Կ� (80����)	*/

	int	crtc_sz_lines      =20;	/* ��� (20��25)		*/
	int	crtc_sz_columns    =80;	/* �Կ� (2��80)			*/
	int	crtc_sz_attrs      =20;	/* °���� (1��20)		*/
	int	crtc_byte_per_line=120;	/* 1�Ԥ�����Υ��� �Х��ȿ�	*/
	int	crtc_font_height   =10;	/* �ե���Ȥι⤵ �ɥåȿ�(8/10)*/



/******************************************************************************

			���������������� crtc_byte_per_line  ����������������
			������   crtc_sz_columns  ������ ��  crtc_sz_attrs ��
			+-------------------------------+-------------------+
		      ��|				|��		    |
		      ��|	+--+ ��			|��		    |
		      ��|	|  | crtc_font_height	|��		    |
			|	+--+ ��			|		    |
	   CRTC_SZ_LINES|				|crtc_sz_lines	    |
			|				|		    |
		      ��|				|��		    |
		      ��|				|��		    |
		      ��|				|��		    |
			+-------------------------------+-------------------+
			������   CRTC_SZ_COLUMNS  ������ 

	crtc_sz_columns		���	2��80
	crtc_sz_attrs		°����	1��20
	crtc_byte_per_line	1�Ԥ�����Υ�����	columns + attrs*2
	crtc_sz_lines		�Կ�	20��25
	crtc_font_height	�ե���Ȥι⤵�ɥå���	8/10
	CRTC_SZ_COLUMNS		ɽ��������	80
	CRTC_SZ_LINES		ɽ������Կ�	20/25

******************************************************************************/









/* ���ͤޤǤˡġġ� 						*/
/*	SORCERIAN          �� 1�����Ф�����			*/
/*	Marchen Veil       �� ���ȥ�ӥ塼�Ȥʤ��⡼��		*/
/*	Xanadu II (E disk) ��             ��			*/
/*	Wizardry V         �� �Υ�ȥ�󥹥ڥ���������⡼��	*/


enum{
  CRTC_RESET		= 0,
  CRTC_STOP_DISPLAY	= 0,
  CRTC_START_DISPLAY,
  CRTC_SET_INTERRUPT_MASK,
  CRTC_READ_LIGHT_PEN,
  CRTC_LOAD_CURSOR_POSITION,
  CRTC_RESET_INTERRUPT,
  CRTC_RESET_COUNTERS,
  CRTC_READ_STATUS,
  EndofCRTC
};
#define CRTC_STATUS_VE	(0x10)		/* ����ɽ��ͭ��		*/
#define CRTC_STATUS_U	(0x08)		/* DMA����������	*/
#define CRTC_STATUS_N	(0x04)		/* �ü�����ʸ�����ȯ�� */
#define CRTC_STATUS_E	(0x02)		/* ɽ����λ���ȯ��	*/
#define CRTC_STATUS_LP	(0x01)		/* �饤�ȥڥ����� 	*/


/****************************************************************/
/* CRTC��Ʊ����������� (OUT 40H,A ... bit3)			*/
/*	�ä˥��ߥ�졼�Ȥ�ɬ�פʤ������������Ȼפ���		*/
/****************************************************************/
#ifdef	SUPPORT_CRTC_SEND_SYNC_SIGNAL
void	crtc_send_sync_signal( int flag )
{
}
#endif




/****************************************************************/
/*    CRTC ���ߥ�졼�����					*/
/****************************************************************/

/*-------- ����� --------*/

void	crtc_init( void )
{
  crtc_out_command( CRTC_RESET << 5 );
  crtc_out_parameter( 0xce );
  crtc_out_parameter( 0x98 );
  crtc_out_parameter( 0x6f );
  crtc_out_parameter( 0x58 );
  crtc_out_parameter( 0x53 );

  crtc_out_command( CRTC_LOAD_CURSOR_POSITION << 5 );
  crtc_out_parameter( 0 );
  crtc_out_parameter( 0 );
}

/*-------- ���ޥ�����ϻ� --------*/

void	crtc_out_command( byte data )
{
  crtc_command = data >> 5;
  crtc_param_num = 0;

  switch( crtc_command ){

  case CRTC_RESET:					/* �ꥻ�å� */
    crtc_status &= ~( CRTC_STATUS_VE | CRTC_STATUS_N | CRTC_STATUS_E );
    crtc_active = FALSE;
    set_text_display();
    set_screen_update_force();
    break;

  case CRTC_START_DISPLAY:				/* ɽ������ */
    crtc_reverse_display = data & 0x01;
    crtc_status |= CRTC_STATUS_VE;
    crtc_status &= ~( CRTC_STATUS_U );
    crtc_active = TRUE;
    set_text_display();
    set_screen_update_force();
    set_screen_update_palette();
    break;

  case CRTC_SET_INTERRUPT_MASK:
    crtc_intr_mask = data & 0x03;
    set_text_display();
    set_screen_update_force();
    break;

  case CRTC_READ_LIGHT_PEN:
    crtc_status &= ~( CRTC_STATUS_LP );
    break;

  case CRTC_LOAD_CURSOR_POSITION:			/* ������������ */
    crtc_load_cursor_position = data & 0x01;
    crtc_cursor[ 0 ] = -1;
    crtc_cursor[ 1 ] = -1;
    break;

  case CRTC_RESET_INTERRUPT:
  case CRTC_RESET_COUNTERS:
    crtc_status &= ~( CRTC_STATUS_N | CRTC_STATUS_E );
    break;

  }
}

/*-------- �ѥ�᡼�����ϻ� --------*/

void	crtc_out_parameter( byte data )
{
  switch( crtc_command ){
  case CRTC_RESET:
    if( crtc_param_num < 5 ){
      crtc_format[ crtc_param_num++ ] = data;
    }

    crtc_skip_line         = crtc_format[2] & 0x80;		/* bool  */

    crtc_attr_non_separate = crtc_format[4] & 0x80;		/* bool */
    crtc_attr_color        = crtc_format[4] & 0x40;		/* bool */
    crtc_attr_non_special  = crtc_format[4] & 0x20;		/* bool */

    crtc_cursor_style      =(crtc_format[2] & 0x40) ?ATTR_REVERSE :ATTR_LOWER;
    crtc_cursor_blink      = crtc_format[2] & 0x20;		/* bool */
    blink_cycle            =(crtc_format[1]>>6) * 8 +8;		/* 8,16,24,48*/

    crtc_sz_lines          =(crtc_format[1] & 0x3f) +1;		/* 1��25 */
    if     ( crtc_sz_lines <= 20 ) crtc_sz_lines = 20;
    else if( crtc_sz_lines >= 25 ) crtc_sz_lines = 25;
    else                           crtc_sz_lines = 24;

    crtc_sz_columns        =(crtc_format[0] & 0x7f) +2;		/* 2��80 */
    if( crtc_sz_columns > 80 ) crtc_sz_columns = 80;

    crtc_sz_attrs          =(crtc_format[4] & 0x1f) +1;		/* 1��20 */
    if     ( crtc_attr_non_special ) crtc_sz_attrs = 0;
    else if( crtc_sz_attrs > 20 )    crtc_sz_attrs = 20;

    crtc_byte_per_line  = crtc_sz_columns + crtc_sz_attrs * 2;	/*column+attr*/

    crtc_font_height    = (crtc_sz_lines>20) ?  8 : 10;
    CRTC_SZ_LINES	= (crtc_sz_lines>20) ? 25 : 20;

    blink_ctrl_update();
    break;

  case CRTC_LOAD_CURSOR_POSITION:
    if( crtc_param_num < 2 ){
      if( crtc_load_cursor_position ){
	crtc_cursor[ crtc_param_num++ ] = data;
      }else{
	crtc_cursor[ crtc_param_num++ ] = -1;
      }
    }
    break;

  }
}

/*-------- ���ơ��������ϻ� --------*/

byte	crtc_in_status( void )
{
  return crtc_status;
}

/*-------- �ѥ�᡼�����ϻ� --------*/

byte	crtc_in_parameter( void )
{
  byte data = 0xff;

  switch( crtc_command ){
  case CRTC_READ_LIGHT_PEN:
    if( crtc_param_num < 2 ){
      data = crtc_light_pen[ crtc_param_num++ ];
    }
    return data;
  }

  return 0xff;
}





/****************************************************************/
/*    DMAC ���ߥ�졼�����					*/
/****************************************************************/

static	int	dmac_flipflop;
	pair	dmac_address[4];
	pair	dmac_counter[4];
	int	dmac_mode;


void	dmac_init( void )
{
  dmac_flipflop = 0;
  dmac_address[0].W = 0;
  dmac_address[1].W = 0;
  dmac_address[2].W = 0xf3c8;
  dmac_address[3].W = 0;
  dmac_counter[0].W = 0;
  dmac_counter[1].W = 0;
  dmac_counter[2].W = 0;
  dmac_counter[3].W = 0;
}


void	dmac_out_mode( byte data )
{
  dmac_flipflop = 0;
  dmac_mode = data;

  set_text_display();
  set_screen_update_force();
}
byte	dmac_in_status( void )
{
  return 0x1f;
}


void	dmac_out_address( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_address[ addr ].B.l=data;
  else                   dmac_address[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
  set_screen_update_force();	/* �����ϡ�addr==2�λ��Τߡġġ� */
}
void	dmac_out_counter( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_counter[ addr ].B.l=data;
  else                   dmac_counter[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
}


byte	dmac_in_address( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_address[ addr ].B.l;
  else                   data = dmac_address[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}
byte	dmac_in_counter( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_counter[ addr ].B.l;
  else                   data = dmac_counter[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}


/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"CRTC"

static	T_SUSPEND_W	suspend_crtcdmac_work[]=
{
  { TYPE_INT,	&text_display,		},
  { TYPE_INT,	&blink_cycle,		},
  { TYPE_INT,	&blink_counter,		},

  { TYPE_INT,	&dma_wait_count,	},

  { TYPE_INT,	&crtc_command,		},
  { TYPE_INT,	&crtc_param_num,	},
  { TYPE_BYTE,	&crtc_status,		},
  { TYPE_BYTE,	&crtc_light_pen[0],	},
  { TYPE_BYTE,	&crtc_light_pen[1],	},
  { TYPE_BYTE,	&crtc_load_cursor_position,	},
  { TYPE_INT,	&crtc_active,		},
  { TYPE_INT,	&crtc_intr_mask,	},
  { TYPE_INT,	&crtc_cursor[0],	},
  { TYPE_INT,	&crtc_cursor[1],	},
  { TYPE_BYTE,	&crtc_format[0],	},
  { TYPE_BYTE,	&crtc_format[1],	},
  { TYPE_BYTE,	&crtc_format[2],	},
  { TYPE_BYTE,	&crtc_format[3],	},
  { TYPE_BYTE,	&crtc_format[4],	},
  { TYPE_INT,	&crtc_reverse_display,	},
  { TYPE_INT,	&crtc_skip_line,	},
  { TYPE_INT,	&crtc_cursor_style,	},
  { TYPE_INT,	&crtc_cursor_blink,	},
  { TYPE_INT,	&crtc_attr_non_separate,},
  { TYPE_INT,	&crtc_attr_color,	},
  { TYPE_INT,	&crtc_attr_non_special,	},
  { TYPE_INT,	&CRTC_SZ_LINES,		},
  { TYPE_INT,	&crtc_sz_lines,		},
  { TYPE_INT,	&crtc_sz_columns,	},
  { TYPE_INT,	&crtc_sz_attrs,		},
  { TYPE_INT,	&crtc_byte_per_line,	},
  { TYPE_INT,	&crtc_font_height,	},

  { TYPE_INT,	&dmac_flipflop,		},
  { TYPE_PAIR,	&dmac_address[0],	},
  { TYPE_PAIR,	&dmac_address[1],	},
  { TYPE_PAIR,	&dmac_address[2],	},
  { TYPE_PAIR,	&dmac_address[3],	},
  { TYPE_PAIR,	&dmac_counter[0],	},
  { TYPE_PAIR,	&dmac_counter[1],	},
  { TYPE_PAIR,	&dmac_counter[2],	},
  { TYPE_PAIR,	&dmac_counter[3],	},
  { TYPE_INT,	&dmac_mode,		},

  { TYPE_END,	0			},
};


int	statesave_crtcdmac( void )
{
  if( statesave_table( SID, suspend_crtcdmac_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}

int	stateload_crtcdmac( void )
{
  if( stateload_table( SID, suspend_crtcdmac_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}
