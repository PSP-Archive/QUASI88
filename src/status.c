/************************************************************************/
/*									*/
/* ���ơ���������ɽ�� (FDDɽ�����ۤ���å�����ɽ��)			*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "status.h"

#include "pc88main.h"		/* boot_basic	*/
#include "graph.h"
#include "memory.h"		/* font_rom	*/
#include "screen.h"

#include "drive.h"
#include "snddrv.h"		/* xmame_get_sound_volume()	*/
#include "wait.h"		/* wait_rate			*/
#include "intr.h"		/* no_wait			*/
#include "menu.h"		/* menu_lang			*/

#include "emu.h"
#include "q8tk.h"


/*---------------------------------------------------------------------------*/

T_STATUS_INFO	status_info[3];			/* ���ơ��������᡼�� */

/*---------------------------------------------------------------------------*/

/*
 * ������ʥ��
 */
static	int	stat_draw[3];
static	int	stat_msg_exist[3];
static	int	stat_msg_timer[3];
static	int	stat_var[3] = { -1, -1, -1 };

#define		stat_mode	stat_var[0]
#define		stat_fdd	stat_var[2]

/*
 * ɽ�����륹�ơ������Υ��᡼���ѥХåե�
 *	�Хåե���8�ɥåȥե����48ʸ��ʬ���뤬���ºݤ�ɽ���Ϥ�äȾ�����
 */
#define	PIXMAP_WIDTH	(48*8)
#define	PIXMAP_HEIGHT	(16)

static	byte	pixmap[3][ PIXMAP_WIDTH * PIXMAP_HEIGHT ];



/*
 * ������ʥե����
 */
enum {
  FNT_START = 0xe0-1,

  FNT_2__1,  FNT_2__2,  FNT_2__3,	/* �ɥ饤��2 */
  FNT_2D_1,  FNT_2D_2,  FNT_2D_3,

  FNT_1__1,  FNT_1__2,  FNT_1__3,	/* �ɥ饤��1 */
  FNT_1D_1,  FNT_1D_2,  FNT_1D_3,

  FNT_T__1,  FNT_T__2,  FNT_T__3,	/* �ơ��� */
  FNT_TR_1,  FNT_TR_2,  FNT_TR_3,
  FNT_TW_1,  FNT_TW_2,  FNT_TW_3,

  FNT_CAP_1, FNT_CAP_2,			/* CAPS     */
  FNT_KAN_1, FNT_KAN_2,			/* ����     */
  FNT_RMJ_1, FNT_RMJ_2,			/* ���޻� */
  FNT_NUM_1, FNT_NUM_2,			/* NUMlock  */

  FNT_END
};



static	const	byte	status_font[ 0x20 ][ 8*16 ] =
{
#define X	STATUS_BG
#define F	STATUS_FG
#define W	STATUS_WHITE
#define B	STATUS_BLACK
#define R	STATUS_RED
#define G	STATUS_GREEN


  {				/* �ɥ饤��2(��¦) ���������ݡ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,W,W,W,
    X,X,X,X,W,W,W,W,
    X,X,X,W,W,W,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,W,B,B,
    X,X,X,X,W,W,W,W,
    X,X,X,X,X,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��2(��¦) ����������� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��2(��¦) �������ݡݱ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    W,W,W,W,W,W,W,W,
    B,B,B,B,B,B,W,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��2(��¦) ���������ݡ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,W,W,W,
    X,X,X,X,W,W,W,W,
    X,X,X,W,W,W,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,W,R,R,
    X,X,X,X,W,W,W,W,
    X,X,X,X,X,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��2(��¦) ����������� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��2(��¦) �������ݡݱ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    W,W,W,W,W,W,W,W,
    R,R,R,R,R,R,W,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��1(��¦) ���������ݡ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,W,B,B,B,B,B,B,
    W,W,W,W,W,W,W,W,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��1(��¦) ����������� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��1(��¦) �������ݡݱ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,X,X,X,X,X,
    W,W,W,W,X,X,X,X,
    B,B,W,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,W,W,W,X,X,X,
    W,W,W,W,X,X,X,X,
    W,W,W,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��1(��¦) ���������ݡ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,W,R,R,R,R,R,R,
    W,W,W,W,W,W,W,W,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��1(��¦) ����������� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ɥ饤��1(��¦) �������ݡݱ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,X,X,X,X,X,
    W,W,W,W,X,X,X,X,
    R,R,W,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,W,W,W,X,X,X,
    W,W,W,W,X,X,X,X,
    W,W,W,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� ���ꡧ���ݡ� */
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,W,F,F,F,F,F,F,
    X,W,F,F,X,X,X,X,
    X,W,F,X,F,X,X,X,
    X,W,F,X,X,F,X,X,
    X,W,F,X,X,X,F,F,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,F,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,X,
    X,W,F,F,F,F,F,F,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� ���ꡧ����� */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    F,X,X,X,X,X,X,F,
    F,F,X,X,X,X,F,F,
    F,X,X,X,X,X,X,F,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� ���ꡧ�ݡݱ� */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    F,F,F,F,F,F,W,X,
    X,X,X,X,F,F,W,X,
    X,X,X,F,X,F,W,X,
    X,X,F,X,X,F,W,X,
    F,F,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    F,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    F,F,F,F,F,F,W,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� ���������ݡ� */
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,W,X,X,X,X,X,X,
    X,W,X,X,F,F,F,F,
    X,W,X,F,X,F,F,F,
    X,W,X,F,F,X,F,F,
    X,W,X,F,F,F,X,X,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,X,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,F,
    X,W,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� ����������� */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    F,F,F,F,F,F,F,F,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    X,F,F,F,F,F,F,X,
    X,X,F,F,F,F,X,X,
    X,F,F,F,F,F,F,X,
    F,F,F,F,F,F,F,F,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� �������ݡݱ� */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,W,X,
    F,F,F,F,X,X,W,X,
    F,F,F,X,F,X,W,X,
    F,F,X,F,F,X,W,X,
    X,X,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    X,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    X,X,X,X,X,X,W,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� Ͽ�������ݡ� */
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,W,X,X,X,X,X,X,
    X,W,X,X,R,R,R,R,
    X,W,X,R,X,R,R,R,
    X,W,X,R,R,X,R,R,
    X,W,X,R,R,R,X,X,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,X,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,R,
    X,W,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� Ͽ��������� */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    X,X,X,X,X,X,X,X,
    R,R,R,R,R,R,R,R,
    X,R,R,R,R,R,R,X,
    X,X,R,R,R,R,X,X,
    X,R,R,R,R,R,R,X,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* �ơ��� Ͽ�����ݡݱ� */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,W,X,
    R,R,R,R,X,X,W,X,
    R,R,R,X,R,X,W,X,
    R,R,X,R,R,X,W,X,
    X,X,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    X,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    X,X,X,X,X,X,W,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
  },



  {				/* CAPS������ */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* CAPS���ݱ�  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    X,F,F,X,X,X,X,F,
    F,X,X,F,X,X,X,F,
    F,X,X,F,X,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    F,F,F,F,F,X,X,F,
    X,X,X,X,X,F,X,F,
    X,X,X,X,X,F,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ���ʡ����� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* ���ʡ��ݱ�  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    F,X,X,X,X,X,X,F,
    F,F,F,F,F,X,X,F,
    F,X,X,X,F,X,X,F,
    F,X,X,X,F,X,X,F,
    F,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,F,X,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ���޻�������  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,F,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* ���޻����ݱ� */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    F,F,F,F,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ����������  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* �������ݱ�  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,F,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,F,F,F,X,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },



#undef X
#undef F
#undef W
#undef B
#undef R
#undef G
};




/*
 * ʸ����򥹥ơ������Υ��᡼���ѥХåե���ž��
 */
static	void	status_puts( int type, const unsigned char *str )
{
  int i, j, k, c, w, h16;
  const byte *p;
  byte mask;
  byte *dst = status_info[ type ].pixmap;
  
  if( str ){
    w = MIN( strlen((char *)str) * 8, PIXMAP_WIDTH );

    for( i=0; i<w; i+=8 ){
      c = *str ++;
      if( c=='\0' ) break;

      if( c < 0xe0 ){

	if( has_kanji_rom && 			/* ����ROM���� */
	    (( 0x20 <= c && c <= 0x7f ) ||	/* ASCII    */
	     ( 0xa0 <= c && c <= 0xdf ) )  ){	/* �������� */
	  p = &kanji_rom[0][ c*8 ][0];
	  h16 = TRUE;
	}else{
	  p = &font_mem[ c*8 ];
	  h16 = FALSE;
	}
	for( j=0; j<16; j++ ){
	  for( mask=0x80, k=0;  k<8;  k++, mask>>=1 ){
	    if( *p & mask ) dst[ j*w +i +k ] = STATUS_FG;
	    else            dst[ j*w +i +k ] = STATUS_BG;
	  }
	  if( h16 || j&1 ) p++;
	}

      }else{		/* 0xe0��0xff �� ������ʥե���Ȥ���� */

	p = &status_font[ (c-0xe0) ][0];
	for( j=0; j<16; j++ ){
	  for( k=0;  k<8;  k++ ){
	    dst[ j*w +i +k ] = *p;
	    p++;
	  }
	}

      }
    }
    status_info[ type ].w = i;
  }else{
    status_info[ type ].w = 0;
  }
}

/*
 * �ԥå����ޥåפ򥹥ơ������Υ��᡼���ѥХåե���ž��
 */
static	void	status_bitmap( int type, const byte bitmap[], int size )
{
  if( bitmap ){
    memcpy( status_info[ type ].pixmap, bitmap, size );
    status_info[ type ].w = size / PIXMAP_HEIGHT;
  }else{
    status_info[ type ].w = 0;
  }
}





/***************************************************************************
 * ���ơ������ط��Υ���������
 ****************************************************************************/
void	status_init( void )
{
  int i;
  for( i=0; i<3; i++ ){
    stat_draw[i] = TRUE;
    stat_msg_exist[i] = FALSE;
    stat_var[i] = -1;

    status_info[i].pixmap = &pixmap[i][0];
    status_info[i].w      = 0;
    status_info[i].h      = PIXMAP_HEIGHT;
  }
}



/***************************************************************************
 * ���ơ�����ɽ������ɽ�����ؤκݤΡ�����ƽ����
 ****************************************************************************/

void	status_reset( int show )
{
  if( show ){						/* ɽ������ʤ�  */
    stat_draw[0] = stat_draw[1] = stat_draw[2] = TRUE;	/* ����ե饰ON  */

  }else{						/* ��ɽ���ʤ�    */
    stat_var[0]  = stat_var[1]  = stat_var[2]  = -1;	/* ��å�����OFF */
  }
}




/***************************************************************************
 * ���ơ������˥�å�����(ʸ����)ɽ��
 *	kind	�� ���ơ��������ֹ� 0 �� 2
 *	frames	�� ɽ���������(�ե졼���) 60����1��
 *		   0 ��̵�¤�ɽ���� <0 �Ǿõ�
 *	msg	�� ɽ������ʸ���� NULL �� "" �Ȥߤʤ���
 ****************************************************************************/
void	status_message( int kind, int frames, const char *msg )
{
  status_puts( kind, (const unsigned char *)msg );

  if( frames >= 0 ){ stat_msg_exist[ kind ] = TRUE;   }
  else             { stat_msg_exist[ kind ] = FALSE;  stat_var[ kind ] = -1; }

  stat_msg_timer[ kind ] = frames;
  stat_draw[ kind ] = TRUE;
}





/***************************************************************************
 * ���ơ�����ɽ���ѤΥ��᡼���򹹿�
 *	ɽ�����᡼���Υԥå����ޥåפ� status_info ��ž�����롣
 *	���� force �� ���ξ��ϡ����󤫤��Ѳ��Τ��ä����ơ������Τ߹���
 *	     force �� ���ξ��ϡ�����ξ��֤Ȥϴؤ�餺���ơ������򹹿�
 *	����ͤϡ��ºݤ˹����������ơ������ֹ椬���ӥå� 0��2 ��
 *		  ���åȤ���롣(�ӥåȤ� 1 �ʤ餽�Υ��ơ������Ϲ���)
 ****************************************************************************/

INLINE void status_mode( void )
{
  int mode = 0;		/* bit :  ....  num kana caps 8mhz basic basic */
  byte buf[16];
  static const char *mode_str[] = {
    "N   4MHz       ", "V1S 4MHz       ", "V1H 4MHz       ", "V2  4MHz       ",
    "N   8MHz       ", "V1S 8MHz       ", "V1H 8MHz       ", "V2  8MHz       ",
  };
  switch( boot_basic ){
  case BASIC_N:		mode = 0;	break;
  case BASIC_V1S:	mode = 1;	break;
  case BASIC_V1H:	mode = 2;	break;
  case BASIC_V2:	mode = 3;	break;
  }
  if( boot_clock_4mhz == FALSE ) mode += 4;

  if( (key_scan[0x0a] & 0x80) == 0 ) mode += 8;
  if( (key_scan[0x08] & 0x20) == 0 ) mode += 16;
  if( numlock_emu ) mode += 32;

  if( stat_mode != mode ){	/* �⡼�ɤ��ѹ�������ɽ������ */
    stat_mode = mode;

    strcpy( (char *)buf, mode_str[ mode & 0x7 ] );
    if( mode & 8 ){
      buf[ 9] = FNT_CAP_1;
      buf[10] = FNT_CAP_2;
    }
    if( mode & 16 ){
      if( romaji_input_mode ){
	buf[11] = FNT_RMJ_1;
	buf[12] = FNT_RMJ_2;
      }else{
	buf[11] = FNT_KAN_1;
	buf[12] = FNT_KAN_2;
      }
    }
    if( mode & 32 ){
	buf[13] = FNT_NUM_1;
	buf[14] = FNT_NUM_2;
    }

    status_puts( 0, (const unsigned char *)buf );
    stat_draw[ 0 ] = TRUE;
  }
}

INLINE void status_fdd( void )
{
  byte *p, buf[16];
  int fdd = 0;		/* bit :  ....  tape tape drv2 drv1 */

  if( ! get_drive_ready(0) ){ fdd |= 1<<0; }	/* FDD����ON */
  if( ! get_drive_ready(1) ){ fdd |= 1<<1; }
  /* drive_check_empty(n) �ǥǥ�������̵ͭ��狼�뤱�ɡ� */

  if     ( tape_writing() ) fdd |= 3<<2;
  else if( tape_reading() ) fdd |= 2<<2;
  else if( tape_exist() )   fdd |= 1<<2;

  if( stat_fdd != fdd ){
    stat_fdd = fdd;

    p = buf;

    switch( fdd >> 2 ){
    case 1:
      *p ++ = FNT_T__1;
      *p ++ = FNT_T__2;
      *p ++ = FNT_T__3;
      *p ++ = ' ';
      break;
    case 2:
      *p ++ = FNT_TR_1;
      *p ++ = FNT_TR_2;
      *p ++ = FNT_TR_3;
      *p ++ = ' ';
      break;
    case 3:
      *p ++ = FNT_TW_1;
      *p ++ = FNT_TW_2;
      *p ++ = FNT_TW_3;
      *p ++ = ' ';
      break;
    }

    if( fdd & (1<<1) ){
      *p ++ = FNT_2D_1;
      *p ++ = FNT_2D_2;
      *p ++ = FNT_2D_3;
    }else{
      *p ++ = FNT_2__1;
      *p ++ = FNT_2__2;
      *p ++ = FNT_2__3;
    }

    if( fdd & (1<<0) ){
      *p ++ = FNT_1D_1;
      *p ++ = FNT_1D_2;
      *p ++ = FNT_1D_3;
    }else{
      *p ++ = FNT_1__1;
      *p ++ = FNT_1__2;
      *p ++ = FNT_1__3;
    }

    *p = '\0';

    status_puts( 2, (const unsigned char *)buf );
    stat_draw[ 2 ] = TRUE;
  }
}



/*
 *
 */
int	status_update( int force )
{
  int i, ret = 0;

  for( i=0; i<3; i++ ){

    if( stat_msg_exist[i] ){		/* ��å��������ꤢ�� */
      if( stat_msg_timer[i] ){			/* �����ޡ��������   */
	if( -- stat_msg_timer[i]==0 ){		/* �����ॢ���Ȥ�     */
	  status_message( i, -1, NULL );	/* ��å���������ä� */
	  stat_var[i] = -1;
	}
      }
    }

    if( stat_msg_exist[i] == FALSE ){	/* ��å���������ʤ� */
      if     ( i==0 ){ status_mode(); }		/* 0 �ϥ⡼��ɽ��  */
      else if( i==2 ){ status_fdd();  }		/* 2 ��FDD����ɽ�� */
    }

  }

  if( now_status ){
    for( i=0; i<3; i++ ){
      if( stat_draw[i] || force ){
	stat_draw[i] = FALSE;
	ret |= 1<<i;
      }
    }
  }
  return ret;
}










/************************************************************************/
/* �����ȥ롦�С�������ɽ�����롣					*/
/*		��ư���ˤΤߡ����δؿ���ƤӤ�������			*/
/************************************************************************/
void	indicate_bootup_logo( void )
{
  status_message( 0, 60*4, Q_TITLE " " Q_VERSION );
  status_message( 1, 0,    "<F12> key to MENU" );
}


void	indicate_stateload_logo( void )
{
  status_message( 0, 60*4, Q_TITLE " " Q_VERSION );
  status_message( 1, 60*4, "State Load Successful !" );
}
