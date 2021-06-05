#include "quasi88.h"
#include "keyboard.h"
#include "romaji.h"
#include "event.h"

#include "soundbd.h"	/* sound_reg[]			*/
#include "graph.h"	/* set_key_and_mouse()		*/

#include "drive.h"

#include "emu.h"
#include "status.h"
#include "pause.h"
#include "menu.h"
#include "screen.h"
#include "snapshot.h"

#include "suspend.h"

/******************************************************************************
 *
 *****************************************************************************/

/*
 *	�������� (���ơ��ȥ���������?)
 */

int		mouse_x;		/* ���ߤ� �ޥ��� x��ɸ		*/
int		mouse_y;		/* ���ߤ� �ޥ��� y��ɸ		*/

static	int	mouse_dx;		/* �ޥ��� x������ư��		*/
static	int	mouse_dy;		/* �ޥ��� y������ư��		*/


unsigned char	key_scan[ 0x10 ];	/* IN(00h)��(0Eh) �����������	*/
					/* key_scan[0]��[14]�ϥ�����	*/
					/* key_scan[15]�� �ѥåɤ˻���	*/

static	int	key_func[ KEY88_END ];	/* ���������إ�����,��ǽ����	*/


/*
 *	PC88���� / ��������
 */

static	int	jop1_step;	/* ����I/O�ݡ��ȤΥ꡼�ɥ��ƥå�	*/
static	int	jop1_dx;	/* ����I/O�ݡ��Ȥ��� (�ޥ��� x�����Ѱ�)	*/
static	int	jop1_dy;	/* ����I/O�ݡ��Ȥ��� (�ޥ��� y�����Ѱ�)	*/

	int	romaji_input_mode = FALSE;	/* ��:���޻�������	*/



/*
  mouse_x, mouse_dx, jop1_dx �δط�
	�ޥ�������ư������
		mouse_x �ˤ������к�ɸ�򥻥åȤ��롣
		�����ɸ�Ȥ��Ѱ� mouse_dx �˥��åȤ��롣
	����I/O�ݡ��Ȥ���ޥ�����ɸ��꡼�ɤ�������
		mouse_dx �򥯥�åԥ󥰤����ͤ� jop1_dx �˥��åȤ��롣
*/



/*
 *	����
 */

int	mouse_mode	= 0;		/* �ޥ��������祤���ƥå�����	*/


int	mouse_key_mode	= 0;		/* �ޥ������Ϥ򥭡���ȿ��	*/
int	mouse_key_assign[6];		/*     0:�ʤ� 1:�ƥ󥭡� 2:Ǥ�� */
static const int mouse_key_assign_tenkey[6] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
  KEY88_x,    KEY88_z,
};


int	joy_key_mode	= 0;		/* ���祤���Ϥ򥭡���ȿ��	*/
int	joy_key_assign[12];		/*     0:�ʤ� 1:�ƥ󥭡� 2:Ǥ�� */
static const int joy_key_assign_tenkey[12] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
  KEY88_x,    KEY88_z,    0, 0, 0, 0, 0, 0,
};
int	joy_swap_button   = FALSE;	/* �ܥ����AB�������ؤ���  	*/


int	cursor_key_mode = 0;		/* �������륭�����̥�����ȿ��	*/
int	cursor_key_assign[4];		/*     0:�ʤ� 1:�ƥ󥭡� 2:Ǥ�� */
static const int cursor_key_assign_tenkey[4] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
};		/* Cursor KEY -> 10 KEY , original by funa. (thanks!) */
		/* Cursor Key -> Ǥ�դΥ��� , original by floi. (thanks!) */



int	tenkey_emu      = FALSE;	/* ��:����������ƥ󥭡���	*/
int	numlock_emu     = FALSE;	/* ��:���եȥ�����NumLock��Ԥ�	*/




int	function_f[ 1 + 20 ] =		/* �ե��󥯥���󥭡��ε�ǽ     */
{
  FN_FUNC,    /* [0] �ϥ��ߡ� */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f1 ��f5  */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f6 ��f10 */
  FN_STATUS,  FN_MENU,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f11��f15 */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f16��f20 */
};


int	romaji_type = 0;		/* ���޻��Ѵ��Υ�����		*/





/*
 *	������� ��Ͽ������
 */

char	*file_rec	= NULL;		/* �������ϵ�Ͽ�Υե�����̾ */
char	*file_pb	= NULL;		/* �������Ϻ����Υե�����̾ */

static	OSD_FILE *fp_rec;
static	OSD_FILE *fp_pb;

static struct {				/* �������ϵ�Ͽ��¤��		*/
  Uchar	key[16];			/*	I/O 00H��0FH 		*/
   char	dx_h;				/*	�ޥ��� dx ���		*/
  Uchar	dx_l;				/*	�ޥ��� dx ����		*/
   char	dy_h;				/*	�ޥ��� dy ���		*/
  Uchar	dy_l;				/*	�ޥ��� dy ����		*/
   char	image[2];			/*	���᡼��No -1��,0Ʊ,1��	*/
   char resv[2];
} key_record;				/* 24 bytes			*/





/*---------------------------------------------------------------------------
 *	���������� �� I/O �ݡ��Ȥ��б�
 *---------------------------------------------------------------------------*/

#define	Port0	0x00
#define	Port1	0x01
#define	Port2	0x02
#define	Port3	0x03
#define	Port4	0x04
#define	Port5	0x05
#define	Port6	0x06
#define	Port7	0x07
#define	Port8	0x08
#define	Port9	0x09
#define	PortA	0x0a
#define	PortB	0x0b
#define	PortC	0x0c
#define	PortD	0x0d
#define	PortE	0x0e
#define	PortX	0x0f

#define	Bit0	0x01
#define	Bit1	0x02
#define	Bit2	0x04
#define	Bit3	0x08
#define	Bit4	0x10
#define	Bit5	0x20
#define	Bit6	0x40
#define	Bit7	0x80

#define	PadA	Bit4
#define	PadB	Bit5

#define	PadU	Bit0
#define	PadD	Bit1
#define	PadL	Bit2
#define	PadR	Bit3

    
enum {
  /* �������������̾��� */

  KEY88_EXT_F6		= KEY88_END + 0,
  KEY88_EXT_F7		= KEY88_END + 1,
  KEY88_EXT_F8		= KEY88_END + 2,
  KEY88_EXT_F9		= KEY88_END + 3,
  KEY88_EXT_F10		= KEY88_END + 4,
  KEY88_EXT_BS		= KEY88_END + 5,
  KEY88_EXT_INS		= KEY88_END + 6,
  KEY88_EXT_DEL		= KEY88_END + 7,
  KEY88_EXT_HENKAN	= KEY88_END + 8,
  KEY88_EXT_KETTEI	= KEY88_END + 9,
  KEY88_EXT_PC		= KEY88_END + 10,
  KEY88_EXT_ZENKAKU	= KEY88_END + 11,
  KEY88_EXT_RETURNL	= KEY88_END + 12,
  KEY88_EXT_RETURNR	= KEY88_END + 13,
  KEY88_EXT_SHIFTL	= KEY88_END + 14,
  KEY88_EXT_SHIFTR	= KEY88_END + 15,

  KEY88_EXT_END		= KEY88_END + 16
};


typedef struct {
  unsigned char port;
  unsigned char mask;
} T_KEYPORT;

static const T_KEYPORT keyport[ KEY88_EXT_END ] =
{
  { 0,0 },			/*	  KEY88_INVALID		= 0,	*/

  { 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { Port9, Bit6 },		/*	  KEY88_SPACE		= 32,	*/
  { Port6, Bit1 },		/*	  KEY88_EXCLAM		= 33,	*/
  { Port6, Bit2 },		/*	  KEY88_QUOTEDBL	= 34,	*/
  { Port6, Bit3 },		/*	  KEY88_NUMBERSIGN	= 35,	*/
  { Port6, Bit4 },		/*	  KEY88_DOLLAR		= 36,	*/
  { Port6, Bit5 },		/*	  KEY88_PERCENT		= 37,	*/
  { Port6, Bit6 },		/*	  KEY88_AMPERSAND	= 38,	*/
  { Port6, Bit7 },		/*	  KEY88_APOSTROPHE	= 39,	*/
  { Port7, Bit0 },		/*	  KEY88_PARENLEFT	= 40,	*/
  { Port7, Bit1 },		/*	  KEY88_PARENRIGHT	= 41,	*/
  { Port7, Bit2 },		/*	  KEY88_ASTERISK	= 42,	*/
  { Port7, Bit3 },		/*	  KEY88_PLUS		= 43,	*/
  { Port7, Bit4 },		/*	  KEY88_COMMA		= 44,	*/
  { Port5, Bit7 },		/*	  KEY88_MINUS		= 45,	*/
  { Port7, Bit5 },		/*	  KEY88_PERIOD		= 46,	*/
  { Port7, Bit6 },		/*	  KEY88_SLASH		= 47,	*/
  { Port6, Bit0 },		/*	  KEY88_0		= 48,	*/
  { Port6, Bit1 },		/*	  KEY88_1		= 49,	*/
  { Port6, Bit2 },		/*	  KEY88_2		= 50,	*/
  { Port6, Bit3 },		/*	  KEY88_3		= 51,	*/
  { Port6, Bit4 },		/*	  KEY88_4		= 52,	*/
  { Port6, Bit5 },		/*	  KEY88_5		= 53,	*/
  { Port6, Bit6 },		/*	  KEY88_6		= 54,	*/
  { Port6, Bit7 },		/*	  KEY88_7		= 55,	*/
  { Port7, Bit0 },		/*	  KEY88_8		= 56,	*/
  { Port7, Bit1 },		/*	  KEY88_9		= 57,	*/
  { Port7, Bit2 },		/*	  KEY88_COLON		= 58,	*/
  { Port7, Bit3 },		/*	  KEY88_SEMICOLON	= 59,	*/
  { Port7, Bit4 },		/*	  KEY88_LESS		= 60,	*/
  { Port5, Bit7 },		/*	  KEY88_EQUAL		= 61,	*/
  { Port7, Bit5 },		/*	  KEY88_GREATER		= 62,	*/
  { Port7, Bit6 },		/*	  KEY88_QUESTION	= 63,	*/
  { Port2, Bit0 },		/*	  KEY88_AT		= 64,	*/
  { Port2, Bit1 },		/*	  KEY88_A		= 65,	*/
  { Port2, Bit2 },		/*	  KEY88_B		= 66,	*/
  { Port2, Bit3 },		/*	  KEY88_C		= 67,	*/
  { Port2, Bit4 },		/*	  KEY88_D		= 68,	*/
  { Port2, Bit5 },		/*	  KEY88_E		= 69,	*/
  { Port2, Bit6 },		/*	  KEY88_F		= 70,	*/
  { Port2, Bit7 },		/*	  KEY88_G		= 71,	*/
  { Port3, Bit0 },		/*	  KEY88_H		= 72,	*/
  { Port3, Bit1 },		/*	  KEY88_I		= 73,	*/
  { Port3, Bit2 },		/*	  KEY88_J		= 74,	*/
  { Port3, Bit3 },		/*	  KEY88_K		= 75,	*/
  { Port3, Bit4 },		/*	  KEY88_L		= 76,	*/
  { Port3, Bit5 },		/*	  KEY88_M		= 77,	*/
  { Port3, Bit6 },		/*	  KEY88_N		= 78,	*/
  { Port3, Bit7 },		/*	  KEY88_O		= 79,	*/
  { Port4, Bit0 },		/*	  KEY88_P		= 80,	*/
  { Port4, Bit1 },		/*	  KEY88_Q		= 81,	*/
  { Port4, Bit2 },		/*	  KEY88_R		= 82,	*/
  { Port4, Bit3 },		/*	  KEY88_S		= 83,	*/
  { Port4, Bit4 },		/*	  KEY88_T		= 84,	*/
  { Port4, Bit5 },		/*	  KEY88_U		= 85,	*/
  { Port4, Bit6 },		/*	  KEY88_V		= 86,	*/
  { Port4, Bit7 },		/*	  KEY88_W		= 87,	*/
  { Port5, Bit0 },		/*	  KEY88_X		= 88,	*/
  { Port5, Bit1 },		/*	  KEY88_Y		= 89,	*/
  { Port5, Bit2 },		/*	  KEY88_Z		= 90,	*/
  { Port5, Bit3 },		/*	  KEY88_BRACKETLEFT	= 91,	*/
  { Port5, Bit4 },		/*	  KEY88_BACKSLASH	= 92,	*/
  { Port5, Bit5 },		/*	  KEY88_BRACKETRIGHT	= 93,	*/
  { Port5, Bit6 },		/*	  KEY88_CARET		= 94,	*/
  { Port7, Bit7 },		/*	  KEY88_UNDERSCORE	= 95,	*/
  { Port2, Bit0 },		/*	  KEY88_BACKQUOTE	= 96,	*/
  { Port2, Bit1 },		/*	  KEY88_a		= 97,	*/
  { Port2, Bit2 },		/*	  KEY88_b		= 98,	*/
  { Port2, Bit3 },		/*	  KEY88_c		= 99,	*/
  { Port2, Bit4 },		/*	  KEY88_d		= 100,	*/
  { Port2, Bit5 },		/*	  KEY88_e		= 101,	*/
  { Port2, Bit6 },		/*	  KEY88_f		= 102,	*/
  { Port2, Bit7 },		/*	  KEY88_g		= 103,	*/
  { Port3, Bit0 },		/*	  KEY88_h		= 104,	*/
  { Port3, Bit1 },		/*	  KEY88_i		= 105,	*/
  { Port3, Bit2 },		/*	  KEY88_j		= 106,	*/
  { Port3, Bit3 },		/*	  KEY88_k		= 107,	*/
  { Port3, Bit4 },		/*	  KEY88_l		= 108,	*/
  { Port3, Bit5 },		/*	  KEY88_n		= 110,	*/
  { Port3, Bit6 },		/*	  KEY88_o		= 111,	*/
  { Port3, Bit7 },		/*	  KEY88_p               = 112,	*/
  { Port4, Bit0 },		/*	  KEY88_q               = 113,	*/
  { Port4, Bit1 },		/*        KEY88_q               = 113,  */
  { Port4, Bit2 },		/*        KEY88_r               = 114,  */
  { Port4, Bit3 },		/*        KEY88_s               = 115,  */
  { Port4, Bit4 },		/*        KEY88_t               = 116,  */
  { Port4, Bit5 },		/*        KEY88_u               = 117,  */
  { Port4, Bit6 },		/*        KEY88_v               = 118,  */
  { Port4, Bit7 },		/*        KEY88_w               = 119,  */
  { Port5, Bit0 },		/*	  KEY88_x		= 120,	*/
  { Port5, Bit1 },		/*	  KEY88_y		= 121,	*/
  { Port5, Bit2 },		/*	  KEY88_z		= 122,	*/
  { Port5, Bit3 },		/*	  KEY88_BRACELEFT	= 123,	*/
  { Port5, Bit4 },		/*	  KEY88_BAR		= 124,	*/
  { Port5, Bit5 },		/*	  KEY88_BRACERIGHT	= 125,	*/
  { Port5, Bit6 },		/*	  KEY88_TILDE		= 126,	*/
  {     0,    0 },
  { Port0, Bit0 },		/*	  KEY88_KP_0		= 128,	*/
  { Port0, Bit1 },		/*	  KEY88_KP_1		= 129,	*/
  { Port0, Bit2 },		/*	  KEY88_KP_2		= 130,	*/
  { Port0, Bit3 },		/*	  KEY88_KP_3		= 131,	*/
  { Port0, Bit4 },		/*	  KEY88_KP_4		= 132,	*/
  { Port0, Bit5 },		/*	  KEY88_KP_5		= 133,	*/
  { Port0, Bit6 },		/*	  KEY88_KP_6		= 134,	*/
  { Port0, Bit7 },		/*	  KEY88_KP_7		= 135,	*/
  { Port1, Bit0 },		/*	  KEY88_KP_8		= 136,	*/
  { Port1, Bit1 },		/*	  KEY88_KP_9		= 137,	*/
  { Port1, Bit2 },		/*	  KEY88_KP_MULTIPLY	= 138,	*/
  { Port1, Bit3 },		/*	  KEY88_KP_ADD		= 139,	*/
  { Port1, Bit4 },		/*	  KEY88_KP_EQUIAL	= 140,	*/
  { Port1, Bit5 },		/*	  KEY88_KP_COMMA	= 141,	*/
  { Port1, Bit6 },		/*	  KEY88_KP_PERIOD	= 142,	*/
  { PortA, Bit5 },		/*	  KEY88_KP_SUB		= 143,	*/
  { PortA, Bit6 },		/*	  KEY88_KP_DIVIDE	= 144,	*/

  { Port1, Bit7 },		/*	  KEY88_RETURN		= 145,	*/
  { Port8, Bit0 },		/*	  KEY88_HOME		= 146,	*/
  { Port8, Bit1 },		/*	  KEY88_UP		= 147,	*/
  { Port8, Bit2 },		/*	  KEY88_RIGHT		= 148,	*/
  { Port8, Bit3 },		/*	  KEY88_INS_DEL		= 149,	*/
  { Port8, Bit4 },		/*	  KEY88_GRAPH		= 150,	*/
  { Port8, Bit5 },		/*	  KEY88_KANA		= 151,	*/
  { Port8, Bit6 },		/*	  KEY88_SHIFT		= 152,	*/
  { Port8, Bit7 },		/*	  KEY88_CTRL		= 153,	*/
  { Port9, Bit0 },		/*	  KEY88_STOP		= 154,	*/
  { Port9, Bit6 },		/*	  KEY88_SPACE		= 155,	*/
  { Port9, Bit7 },		/*	  KEY88_ESC		= 156,	*/
  { PortA, Bit0 },		/*	  KEY88_TAB		= 157,	*/
  { PortA, Bit1 },		/*	  KEY88_DOWN		= 158,	*/
  { PortA, Bit2 },		/*	  KEY88_LEFT		= 159,	*/
  { PortA, Bit3 },		/*	  KEY88_HELP		= 160,	*/
  { PortA, Bit4 },		/*	  KEY88_COPY		= 161,	*/
  { PortA, Bit7 },		/*	  KEY88_CAPS		= 162,	*/
  { PortB, Bit0 },		/*	  KEY88_ROLLUP		= 163,	*/
  { PortB, Bit1 },		/*	  KEY88_ROLLDOWN	= 164,	*/

  { Port9, Bit1 },		/*	  KEY88_F1		= 165,	*/
  { Port9, Bit2 },		/*	  KEY88_F2		= 166,	*/
  { Port9, Bit3 },		/*	  KEY88_F3		= 167,	*/
  { Port9, Bit4 },		/*	  KEY88_F4		= 168,	*/
  { Port9, Bit5 },		/*	  KEY88_F5		= 169,	*/

  { 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { Port9, Bit1 },	/*f-1*/	/*	  KEY88_F6		= 180,	*/
  { Port9, Bit2 },	/*f-2*/	/*	  KEY88_F7		= 181,	*/
  { Port9, Bit3 },	/*f-3*/	/*	  KEY88_F8		= 182,	*/
  { Port9, Bit4 },	/*f-4*/	/*	  KEY88_F9		= 183,	*/
  { Port9, Bit5 },	/*f-5*/	/*	  KEY88_F10		= 184,	*/
  { Port8, Bit3 },	/*del*/	/*	  KEY88_BS		= 185,	*/
  { Port8, Bit3 },	/*del*/	/*	  KEY88_INS		= 186,	*/
  { Port8, Bit3 },	/*del*/	/*	  KEY88_DEL		= 187,	*/
  { Port9, Bit6 },	/*spc*/	/*	  KEY88_HENKAN		= 188,	*/
  { Port9, Bit6 },	/*spc*/	/*	  KEY88_KETTEI		= 189,	*/
  {     0,    0 },		/*	  KEY88_PC		= 190,	*/
  {     0,    0 },		/*	  KEY88_ZENKAKU		= 191,	*/
  { Port1, Bit7 },	/*ret*/	/*	  KEY88_RETURNL		= 192,	*/
  { Port1, Bit7 },	/*ret*/	/*	  KEY88_RETURNR		= 193,	*/
  { Port8, Bit6 },	/*sft*/	/*	  KEY88_SHIFTL		= 194,	*/
  { Port8, Bit6 },	/*sft*/	/*	  KEY88_SHIFTR		= 195,	*/

  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  {     0,    0 },		/*	  KEY88_MOUSE_UP        = 208,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_DOWN      = 209,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_LEFT      = 210,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_RIGHT     = 211,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_L         = 212,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_M         = 213,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_R         = 214,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_WUP       = 215,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_WDN       = 216,	*/

  { 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { PortX, PadU }, 		/*	  KEY88_PAD_UP          = 224,	*/
  { PortX, PadD }, 		/*	  KEY88_PAD_DOWN        = 225,	*/
  { PortX, PadL }, 		/*	  KEY88_PAD_LEFT        = 226,	*/
  { PortX, PadR }, 		/*	  KEY88_PAD_RIGHT       = 227,	*/
  { PortX, PadA }, 		/*	  KEY88_PAD_A           = 228,	*/
  { PortX, PadB }, 		/*	  KEY88_PAD_B           = 229,	*/
  {     0,    0 },		/*	  KEY88_PAD_C           = 230,	*/
  {     0,    0 },		/*	  KEY88_PAD_D           = 232,	*/
  {     0,    0 },		/*	  KEY88_PAD_E           = 233,	*/
  {     0,    0 },		/*	  KEY88_PAD_F           = 234,	*/
  {     0,    0 },		/*	  KEY88_PAD_G           = 235,	*/
  {     0,    0 },		/*	  KEY88_PAD_H           = 236,	*/

  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },     /* KEY88_END */


  { PortC, Bit0 },		/*	  KEY88_EXT_F6		= 256,	*/
  { PortC, Bit1 },		/*	  KEY88_EXT_F7		= 257,	*/
  { PortC, Bit2 },		/*	  KEY88_EXT_F8		= 258,	*/
  { PortC, Bit3 },		/*	  KEY88_EXT_F9		= 259,	*/
  { PortC, Bit4 },		/*	  KEY88_EXT_F10		= 260,	*/
  { PortC, Bit5 },		/*	  KEY88_EXT_BS		= 261,	*/
  { PortC, Bit6 },		/*	  KEY88_EXT_INS		= 262,	*/
  { PortC, Bit7 },		/*	  KEY88_EXT_DEL		= 263,	*/
  { PortD, Bit0 },		/*	  KEY88_EXT_HENKAN	= 264,	*/
  { PortD, Bit1 },		/*	  KEY88_EXT_KETTEI	= 265,	*/
  { PortD, Bit2 },		/*	  KEY88_EXT_PC		= 266,	*/
  { PortD, Bit3 },		/*	  KEY88_EXT_ZENKAKU	= 267,	*/
  { PortE, Bit0 },		/*	  KEY88_EXT_RETURNL	= 268,	*/
  { PortE, Bit1 },		/*	  KEY88_EXT_RETURNR	= 269,	*/
  { PortE, Bit2 },		/*	  KEY88_EXT_SHIFTL	= 270,	*/
  { PortE, Bit3 },		/*	  KEY88_EXT_SHIFTR	= 271,	*/
};


/*---------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/

#define	PAD_PRESS(pad)		key_scan[ PortX ] &= ~(pad)
#define	PAD_RELEASE(pad)	key_scan[ PortX ] |=  (pad)
#define	PAD_TOGGLE(pad)		key_scan[ PortX ] ^=  (pad)
#define	IS_PAD_STATUS()		key_scan[ PortX ]


#define	KEY88_PRESS(code)	\
	key_scan[ keyport[(code)].port ] &= ~keyport[(code)].mask

#define	KEY88_RELEASE(code)	\
	key_scan[ keyport[(code)].port ] |=  keyport[(code)].mask

#define	KEY88_TOGGLE(code)	\
	key_scan[ keyport[(code)].port ] ^=  keyport[(code)].mask

#define	IS_KEY88_PRESS(code)	\
	(~key_scan[ keyport[(code)].port ] & keyport[(code)].mask)

#define	IS_KEY88_RELEASE(code)	\
	( key_scan[ keyport[(code)].port ] & keyport[(code)].mask)


#define	IS_KEY88_PRINTTABLE(c)	(32 <= (c) && (c) <= 144)
#define	IS_KEY88_FUNCTION(c)	(KEY88_F1 <= (c) && (c) <= KEY88_F20)
#define	IS_KEY88_LATTERTYPE(c)	(KEY88_F6 <= (c) && (c) <= KEY88_SHIFTR)



static	void	key_record_playback( void );


/*---------------------------------------------------------------------------
 * �����ΥХ���ǥ����ѹ� (���������ɡ���ǽ)
 *---------------------------------------------------------------------------*/
INLINE	void	clr_key_function( void )
{
  int i;
  for( i=0; i<COUNTOF(key_func); i++ ){ key_func[i] = 0; }
}


INLINE	void	set_key_function( int keycode, int func_no )
{
  key_func[ keycode ] = func_no;
}




/****************************************************************************
 * �����ΥХ���ǥ����ѹ� / ����
 *	���ץ����䡢��˥塼�Ǥ�����˴�Ť��������Х���ɤ��ѹ�����/�᤹��
 *	keyboard_start() �� ���ߥ�졼�ȥ⡼�ɤγ��ϻ��˸ƤФ�롣
 *	keyboard_stop()  �� ���ߥ�졼�ȥ⡼�ɰʳ��γ��ϻ��˸ƤФ�롣
 *****************************************************************************/

void	keyboard_start( void )
{
  const int *p;

  clr_key_function();

  if( numlock_emu ){
    numlock_on();			/* �����ƥ�� NUM LOCK ������ */
  }

  if( tenkey_emu ){			/* ����������ƥ󥭡������� */
    set_key_function( KEY88_1, KEY88_KP_1 );
    set_key_function( KEY88_2, KEY88_KP_2 );
    set_key_function( KEY88_3, KEY88_KP_3 );
    set_key_function( KEY88_4, KEY88_KP_4 );
    set_key_function( KEY88_5, KEY88_KP_5 );
    set_key_function( KEY88_6, KEY88_KP_6 );
    set_key_function( KEY88_7, KEY88_KP_7 );
    set_key_function( KEY88_8, KEY88_KP_8 );
    set_key_function( KEY88_9, KEY88_KP_9 );
    set_key_function( KEY88_0, KEY88_KP_0 );
  }

  if( cursor_key_mode ){		/* ������������ϳ�������ѹ� */
    if( cursor_key_mode == 1 ) p = cursor_key_assign_tenkey;
    else                       p = cursor_key_assign;

    set_key_function( KEY88_UP,    *p );	p ++;
    set_key_function( KEY88_DOWN,  *p );	p ++;
    set_key_function( KEY88_LEFT,  *p );	p ++;
    set_key_function( KEY88_RIGHT, *p );
  }

  switch( mouse_mode ){			/* �ޥ��������ϳ�������ѹ� */
  case MOUSE_NONE:
  case MOUSE_JOYSTICK:
    if( mouse_key_mode ){
      if( mouse_key_mode == 1 ) p = mouse_key_assign_tenkey;
      else                      p = mouse_key_assign;

      set_key_function( KEY88_MOUSE_UP,    *p );	p ++;
      set_key_function( KEY88_MOUSE_DOWN,  *p );	p ++;
      set_key_function( KEY88_MOUSE_LEFT,  *p );	p ++;
      set_key_function( KEY88_MOUSE_RIGHT, *p );	p ++;
      set_key_function( KEY88_MOUSE_L,     *p );	p ++;
      set_key_function( KEY88_MOUSE_R,     *p );
    }
    break;

  case MOUSE_JOYMOUSE:
    set_key_function( KEY88_MOUSE_UP,    KEY88_PAD_UP    );
    set_key_function( KEY88_MOUSE_DOWN,  KEY88_PAD_DOWN  );
    set_key_function( KEY88_MOUSE_LEFT,  KEY88_PAD_LEFT  );
    set_key_function( KEY88_MOUSE_RIGHT, KEY88_PAD_RIGHT );
    set_key_function( KEY88_MOUSE_L,     KEY88_PAD_A     );
    set_key_function( KEY88_MOUSE_R,     KEY88_PAD_B     );
    break;

  case MOUSE_MOUSE:
    set_key_function( KEY88_MOUSE_L,     KEY88_PAD_A     );
    set_key_function( KEY88_MOUSE_R,     KEY88_PAD_B     );
    break;
  }

  switch( mouse_mode ){			/* ���祤���ƥ��å����ϳ�������ѹ� */
  case MOUSE_NONE:
  case MOUSE_MOUSE:
  case MOUSE_JOYMOUSE:
    if( joy_key_mode ){
      if( joy_key_mode == 1 ) p = joy_key_assign_tenkey;
      else                    p = joy_key_assign;

      set_key_function( KEY88_PAD_UP,    *p );		p ++;
      set_key_function( KEY88_PAD_DOWN,  *p );		p ++;
      set_key_function( KEY88_PAD_LEFT,  *p );		p ++;
      set_key_function( KEY88_PAD_RIGHT, *p );		p ++;
      set_key_function( KEY88_PAD_A,     *p );		p ++;
      set_key_function( KEY88_PAD_B,     *p );		p ++;
      set_key_function( KEY88_PAD_C,     *p );		p ++;
      set_key_function( KEY88_PAD_D,     *p );		p ++;
      set_key_function( KEY88_PAD_E,     *p );		p ++;
      set_key_function( KEY88_PAD_F,     *p );		p ++;
      set_key_function( KEY88_PAD_G,     *p );		p ++;
      set_key_function( KEY88_PAD_H,     *p );		p ++;
    }
    break;

  case MOUSE_JOYSTICK:
    break;
  }

					/* �ե��󥯥���󥭡��ε�ǽ������� */
  set_key_function( KEY88_F1,  function_f[  1 ] );
  set_key_function( KEY88_F2,  function_f[  2 ] );
  set_key_function( KEY88_F3,  function_f[  3 ] );
  set_key_function( KEY88_F4,  function_f[  4 ] );
  set_key_function( KEY88_F5,  function_f[  5 ] );
  set_key_function( KEY88_F6,  function_f[  6 ] );
  set_key_function( KEY88_F7,  function_f[  7 ] );
  set_key_function( KEY88_F8,  function_f[  8 ] );
  set_key_function( KEY88_F9,  function_f[  9 ] );
  set_key_function( KEY88_F10, function_f[ 10 ] );
  set_key_function( KEY88_F11, function_f[ 11 ] );
  set_key_function( KEY88_F12, function_f[ 12 ] );
  set_key_function( KEY88_F13, function_f[ 13 ] );
  set_key_function( KEY88_F14, function_f[ 14 ] );
  set_key_function( KEY88_F15, function_f[ 15 ] );
  set_key_function( KEY88_F16, function_f[ 16 ] );
  set_key_function( KEY88_F17, function_f[ 17 ] );
  set_key_function( KEY88_F18, function_f[ 18 ] );
  set_key_function( KEY88_F19, function_f[ 19 ] );
  set_key_function( KEY88_F20, function_f[ 20 ] );


				/* �ޥ������Ϥ������ơ��ޥ������֤������ */
  init_mouse_position( &mouse_x, &mouse_y );
  mouse_dx = 0;
  mouse_dy = 0;
}





void	keyboard_stop( void )
{
  romaji_clear();		/* ���޻��Ѵ���������� */

  numlock_off();		/* �����ƥ�� NUM LOCK ���� */

				/* ��˥塼�������ơ��ޥ������֤������ */
  init_mouse_position( &mouse_x, &mouse_y );
}





/****************************************************************************
 * �ڵ����¸����ꡢ�����������˸ƤӽФ�����
 *
 *	code �ϡ����������ɤǡ� KEY88_SPACE <= code <= KEY88_SHIFTR
 *	on   �ϡ����������ʤ鿿�����������ʤ鵶
 *****************************************************************************/
static	void	do_lattertype( int code, int on );
static	int	do_func( int func, int on );

void	pc88_key( int code, int on )
{
  if( get_emu_mode() == EXEC ){		/*===================================*/

    if( key_func[ code ] ){			/* �ü����������ƺѤξ�� */
      code = do_func( key_func[ code ], on );	/*	�ü쵡ǽ������¹�  */
      if( code==0 ) return;			/*	���ͤ� ������������ */
    }

    if( romaji_input_mode && on ){		/* ���޻����ϥ⡼�ɤξ�� */
      if( romaji_input( code )==0 ){		/*	�Ѵ�������¹�      */
	return;
      }
    }

    if( IS_KEY88_LATTERTYPE( code ) ){		/* ����������ܡ��ɤν���   */
      do_lattertype( code, on );
    }
						/* ����������IO�ݡ��Ȥ�ȿ�� */
    if( on ) KEY88_PRESS( code );
    else     KEY88_RELEASE( code );

/*
if(code==KEY88_RETURNL){
  if( on ) printf("+%d\n",code);
  else     printf("-%d\n",code);
}
*/

  }else
  if( get_emu_mode() == MENU ){		/*===================================*/
/*printf("%d\n",code);*/
						/* ��˥塼�Ѥ��ɤ��ؤ��� */
    switch( code ){
    case KEY88_KP_0:		code = KEY88_0;		break;
    case KEY88_KP_1:		code = KEY88_1;		break;
    case KEY88_KP_2:		code = KEY88_2;		break;
    case KEY88_KP_3:		code = KEY88_3;		break;
    case KEY88_KP_4:		code = KEY88_4;		break;
    case KEY88_KP_5:		code = KEY88_5;		break;
    case KEY88_KP_6:		code = KEY88_6;		break;
    case KEY88_KP_7:		code = KEY88_7;		break;
    case KEY88_KP_8:		code = KEY88_8;		break;
    case KEY88_KP_9:		code = KEY88_9;		break;
    case KEY88_KP_MULTIPLY:	code = KEY88_ASTERISK;	break;
    case KEY88_KP_ADD:		code = KEY88_PLUS;	break;
    case KEY88_KP_EQUIAL:	code = KEY88_EQUAL;	break;
    case KEY88_KP_COMMA:	code = KEY88_COMMA;	break;
    case KEY88_KP_PERIOD:	code = KEY88_PERIOD;	break;
    case KEY88_KP_SUB:		code = KEY88_MINUS;	break;
    case KEY88_KP_DIVIDE:	code = KEY88_SLASH;	break;

    case KEY88_INS_DEL:		code = KEY88_BS;	break;
    case KEY88_DEL:		code = KEY88_BS;	break;
    case KEY88_KETTEI:		code = KEY88_SPACE;	break;
    case KEY88_HENKAN:		code = KEY88_SPACE;	break;
    case KEY88_RETURNL:		code = KEY88_RETURN;	break;
    case KEY88_RETURNR:		code = KEY88_RETURN;	break;
    case KEY88_SHIFTL:		code = KEY88_SHIFT;	break;
    case KEY88_SHIFTR:		code = KEY88_SHIFT;	break;
    }
    if( on ) q8tk_event_key_on( code );
    else     q8tk_event_key_off( code );

  }else
  if( get_emu_mode() == PAUSE ){	/*===================================*/

    if( on ){
      if( key_func[ code ] ){
	if( key_func[ code ] == FN_MENU ){

	  pause_event_key_on_menu();

	}else if( key_func[ code ] == FN_PAUSE ){

	  pause_event_key_on_esc();

	}

      }else if( code == KEY88_ESC ){

	pause_event_key_on_esc();

      }

    }
  }
}





/*----------------------------------------------------------------------
 * ����������ܡ��ɤΥ�������/�������ν���
 *		���եȥ����䡢Ʊ�쵡ǽ�����Υݡ��Ȥ�Ʊ����������
 *----------------------------------------------------------------------*/
static	void	do_lattertype( int code, int on )
{
				  /* KEY88_XXX �� KEY88_EXT_XXX ���Ѵ� */
  int code2 = code - KEY88_F6 + KEY88_END;

  switch( code ){
  case KEY88_F6:
  case KEY88_F7:
  case KEY88_F8:
  case KEY88_F9:
  case KEY88_F10:
  case KEY88_INS:		/* KEY88_SHIFTR, KEY88_SHIFTL �� ? */
    if( on ){ KEY88_PRESS  ( KEY88_SHIFT );  KEY88_PRESS  ( code2 ); }
    else    { KEY88_RELEASE( KEY88_SHIFT );  KEY88_RELEASE( code2 ); }
    break;

  case KEY88_DEL:
  case KEY88_BS:
  case KEY88_HENKAN:
  case KEY88_KETTEI:
  case KEY88_ZENKAKU:
  case KEY88_PC:
  case KEY88_RETURNR:		/* KEY88_RETURNL �� ? */
  case KEY88_RETURNL:		/* KEY88_RETURNR �� ? */
  case KEY88_SHIFTR:		/* KEY88_SHIFTL  �� ? */
  case KEY88_SHIFTL:		/* KEY88_SHIFTR  �� ? */
    if( on ){ KEY88_PRESS  ( code2 ); }
    else    { KEY88_RELEASE( code2 ); }
    break;

  default:
    return;
  }
}

/*----------------------------------------------------------------------
 * �ե��󥯥���󥭡��˳�����Ƥ���ǽ�ν���
 *		����ͤϡ������ʥ��������� (0�ʤ饭�������ʤ�����)
 *----------------------------------------------------------------------*/
static int do_func( int func, int on )
{
  switch( func ){
  case FN_FUNC:					/* ��ǽ�ʤ� */
    return 0;

  case FN_FRATE_UP:				/* �ե졼�� */
    if( on ) quasi88_framerate_up();
    return 0;
  case FN_FRATE_DOWN:				/* �ե졼�� */
    if( on ) quasi88_framerate_down();
    return 0;

  case FN_VOLUME_UP:				/* ���� */
    if( on ) quasi88_volume_up();
    return 0;
  case FN_VOLUME_DOWN:				/* ���� */
    if( on ) quasi88_volume_down();
    return 0;

  case FN_PAUSE:				/* ������ */
    if( on ) quasi88_pause();
    return 0;

  case FN_RESIZE:				/* �ꥵ���� */
    if( on ){
      if( ++ screen_size >= SCREEN_SIZE_END ) screen_size = 0;
      quasi88_change_screen();
    }
    return 0;

  case FN_NOWAIT:				/* �������� */
    if( on ) quasi88_wait_none();
    return 0;
  case FN_SPEED_UP:
    if( on ) quasi88_wait_up();
    return 0;
  case FN_SPEED_DOWN:
    if( on ) quasi88_wait_down();
    return 0;

  case FN_MOUSE_HIDE:				/* �ޥ���ɽ�� */
    if( on ){
      hide_mouse ^= 1;
      set_mouse_state();
    }
    return 0;

  case FN_FULLSCREEN:				/* �������ڤ��ؤ� */
    if( on ){
      if( enable_fullscreen ){
	use_fullscreen = (now_fullscreen) ? FALSE : TRUE;
	quasi88_change_screen();
      }
    }
    return 0;

  case FN_IMAGE_NEXT1:				/* DRIVE1: ���᡼���ѹ� */
    if( on ) quasi88_drv1_image_empty();
    else     quasi88_drv1_image_next();
    return 0;
  case FN_IMAGE_PREV1:
    if( on ) quasi88_drv1_image_empty();
    else     quasi88_drv1_image_prev();
    return 0;
  case FN_IMAGE_NEXT2:				/* DRIVE2: ���᡼���ѹ� */
    if( on ) quasi88_drv2_image_empty();
    else     quasi88_drv2_image_next();
    return 0;
  case FN_IMAGE_PREV2:
    if( on ) quasi88_drv2_image_empty();
    else     quasi88_drv2_image_prev();
    return 0;

  case FN_NUMLOCK:				/* NUM lock */
    if( on ){
      if( numlock_emu ) numlock_off();
      numlock_emu ^= 1;
      keyboard_start();
    }
    return 0;

  case FN_RESET:				/* �ꥻ�å� */
    if( on ) quasi88_reset();
    return 0;

  case FN_KANA:					/* ���� */
    if( on ){
      KEY88_TOGGLE( KEY88_KANA );
      romaji_input_mode = FALSE;
    }
    return 0;

  case FN_ROMAJI:				/* ����(���޻�) */
    if( on ){
      KEY88_TOGGLE( KEY88_KANA );
      if( IS_KEY88_PRESS( KEY88_KANA ) ){
	status_message( 1, 60, "Romaji ON" );
	romaji_input_mode = TRUE;
	romaji_clear();
      }else{
	status_message( 1, 60, "Romaji OFF" );
	romaji_input_mode = FALSE;
      }
    }
    return 0;

  case FN_CAPS:					/* CAPS */
    if( on ){
      KEY88_TOGGLE( KEY88_CAPS );
    }
    return 0;

  case FN_KETTEI:	return KEY88_KETTEI;	/* ���� */
  case FN_HENKAN:	return KEY88_HENKAN;	/* �Ѵ� */
  case FN_ZENKAKU:	return KEY88_ZENKAKU;	/* ���� */
  case FN_PC:		return KEY88_PC;	/* �У� */

  case FN_SNAPSHOT:				/* �����꡼�󥹥ʥåץ���å�*/
    if( on ) quasi88_snapshot();
    return 0;

  case FN_STOP:		return KEY88_STOP;	/* STOP */
  case FN_COPY:		return KEY88_COPY;	/* COPY */

  case FN_STATUS:				/* FDD���ơ�����ɽ�� */
    if( on ) quasi88_status();
    return 0;
  case FN_MENU:					/* ��˥塼�⡼�� */
    if( on ) quasi88_menu();
    return 0;

  }
  return func;
}










/******************************************************************************
 * �ڵ����¸����ꡢ�ޥ����Υܥ��󲡲����˸ƤӽФ�����
 *
 *	code �ϡ����������ɤǡ� KEY88_MOUSE_L <= code <= KEY88_MOUSE_DOWN
 *	on   �ϡ����������ʤ鿿�����������ʤ鵶
 *****************************************************************************/
void	pc88_mouse( int code, int on )
{
  switch( get_emu_mode() ){

  case EXEC:				/*===================================*/
/*
if( on ) printf("+%d\n",code);
else     printf("-%d\n",code);
*/

    if( key_func[ code ] ){			/* ���������ɤ��ɤ��ؤ�      */
      code = do_func( key_func[ code ], on );	/*	�ü쵡ǽ������С�   */
      if( code==0 ) return;			/*	���������Τ����   */
    }						/*  ���޻����Ϥ䡢         */
						/*  ����������Ͻ������ʤ�   */

    if( on ) KEY88_PRESS( code );		/* I/O�ݡ��Ȥ�ȿ��           */
    else     KEY88_RELEASE( code );
    break;

  case MENU:				/*===================================*/

    if( on ) q8tk_event_mouse_on( code );
    else     q8tk_event_mouse_off( code );
    break;
  }
}




/******************************************************************************
 * �ڵ����¸����ꡢ���祤���ƥ��å����ϻ��˸ƤӽФ�����
 *
 *	code �ϡ����������ɤǡ� KEY88_PAD_UP <= code <= KEY88_PAD_H
 *	on   �ϡ����������ʤ鿿�����������ʤ鵶
 *****************************************************************************/
void	pc88_pad( int code, int on )
{
  if( get_emu_mode() == EXEC ){		/*===================================*/

    if( joy_swap_button ){			/* A/B �ܥ��������ؤ�        */
      if     ( code == KEY88_PAD_A ) code = KEY88_PAD_B;
      else if( code == KEY88_PAD_B ) code = KEY88_PAD_A;
    }

    if( key_func[ code ] ){			/* ���������ɤ��ɤ��ؤ�      */
      code = do_func( key_func[ code ], on );	/*	�ü쵡ǽ������С�   */
      if( code==0 ) return;			/*	���������Τ����   */
    }						/*  ���޻����Ϥ䡢         */
						/*  ����������Ͻ������ʤ�   */

    if( on ) KEY88_PRESS( code );		/* I/O�ݡ��Ȥ�ȿ��           */
    else     KEY88_RELEASE( code );

  }
}






/****************************************************************************
 * �ڵ����¸����ꡢ�ޥ�����ư���˸ƤӽФ���롣��
 *
 *	abs_coord �������ʤ顢x,y �ϥޥ����ΰ�ư��κ�ɸ�򼨤���
 *		��ɸ�ϡ������ΰ�� (0,0)-(640,400) �Ȥ������κ�ɸ�Ȥ���
 *		���̥�������ܡ������ˤ���ɸ�Τ���ϸƤӽФ������������Ƥ���
 *		����������åԥ󥰤����פǡ��ϰϳ����ͤǤ�ġ�
 *
 *	abs_coord �� ���ʤ顢x,y �ϥޥ����ΰ�ư�̤򼨤���
 *		���̥�������ޥ���®�٤ˤ�������ϡ��ƤӽФ����Ǥ��Ƥ�����
 *****************************************************************************/
void	pc88_mouse_move( int x, int y, int abs_coord )
{
  switch( get_emu_mode() ){

  case EXEC:				/*===================================*/
    if( abs_coord ){
      mouse_dx += x - mouse_x;
      mouse_dy += y - mouse_y;
      mouse_x = x;
      mouse_y = y;

    }else{
      mouse_dx += x;
      mouse_dy += y;
    }
    break;

  case MENU:				/*===================================*/
    if( abs_coord ){
      mouse_x = x;
      mouse_y = y;

    }else{
      mouse_x += x;	
      if( mouse_x <   0 ) mouse_x = 0;
      if( mouse_x > 640 ) mouse_x = 640;

      mouse_y += y;
      if( mouse_y <   0 ) mouse_y = 0;
      if( mouse_y > 400 ) mouse_y = 400;
    }

    q8tk_event_mouse_moved( mouse_x, mouse_y );
  }
}



/****************************************************************************
 * �ڵ����¸����ꡢ�ե����������������˸ƤӽФ�����
 *****************************************************************************/
void	pc88_focus_in( void )
{
  if( get_emu_mode() == PAUSE ){	/*===================================*/

    pause_event_focus_in_when_pause();

  }
}

void	pc88_focus_out( void )
{
  if( get_emu_mode() == EXEC ){		/*===================================*/

    pause_event_focus_out_when_exec();

  }
}



/****************************************************************************
 * �ڵ����¸����ꡢ������λ�������˸ƤӽФ�����
 *****************************************************************************/
void	pc88_quit( void )
{
  set_emu_mode( QUIT );

  switch( get_emu_mode() ){
  case EXEC:				/*===================================*/
    break;

  case MENU:				/*===================================*/
    q8tk_event_quit();
    break;

  case PAUSE:				/*===================================*/
    break;
  }
}






/****************************************************************************
 *
 *	��������������
 *
 *****************************************************************************/

void	keyboard_init( void )
{
  size_t i;
  for( i=0; i<sizeof(key_scan); i++ )  key_scan[i] = 0xff;

  romaji_init();
}



void	scan_keyboard( void )
{
	/* ���޻����ϥ⡼�ɻ��ϡ����޻��Ѵ���Υ��ʤ� key_scan[] ��ȿ�� */

  if( romaji_input_mode ) romaji_output();

  
	/* �ޥ������Ϥ� key_scan[] ��ȿ�� */

  switch( mouse_mode ){
    int status;

  case MOUSE_NONE:
  case MOUSE_JOYSTICK:
    if( mouse_key_mode == 0 ){	/* �ޥ��������������̵���ʤ� */
      mouse_dx = 0;		/* �ޥ�����ư�̤�����         */
      mouse_dy = 0;
      break;
    }
    /* FALLTHROUGH */		/* �ޥ��������������ͭ�꤫�� */
				/* ���祤���ƥ��å��⡼�ɤʤ� */
  case MOUSE_JOYMOUSE:		/* �ޥ�����ư�̤�ݡ��Ȥ�ȿ�� */
    if( mouse_dx==0 ){
      if     ( mouse_dy ==0 ) status = 0;		/* ---- */
      else if( mouse_dy > 0 ) status = ( PadD );	/* ��   */
      else                    status = ( PadU );	/* ��   */
    }else if( mouse_dx > 0 ){
      int a = mouse_dy*100/mouse_dx;
      if     ( a >  241 ) status = ( PadD );		/* ��   */
      else if( a >   41 ) status = ( PadD | PadR );	/* ���� */
      else if( a >  -41 ) status = (        PadR );	/*   �� */
      else if( a > -241 ) status = ( PadU | PadR );	/* ���� */
      else                status = ( PadU );		/* ��   */
    }else{
      int a = -mouse_dy*100/mouse_dx;
      if     ( a >  241 ) status = ( PadD );		/* ��   */
      else if( a >   41 ) status = ( PadD | PadL );	/* ���� */
      else if( a >  -41 ) status = (        PadL );	/*   �� */
      else if( a > -241 ) status = ( PadU | PadL );	/* ���� */
      else                status = ( PadU );		/* ��   */
    }

    pc88_mouse( KEY88_MOUSE_UP,    (status & PadU) );
    pc88_mouse( KEY88_MOUSE_DOWN,  (status & PadD) );
    pc88_mouse( KEY88_MOUSE_LEFT,  (status & PadL) );
    pc88_mouse( KEY88_MOUSE_RIGHT, (status & PadR) );

    mouse_dx = 0;		/* �ݡ���ȿ�Ǹ�ϰ�ư�̥��ꥢ */
    mouse_dy = 0;
    break;

  case MOUSE_MOUSE:		/* �ޥ���������ʤ顢         */
    break;			/* �ޥ�����ư�̤��ݻ����Ƥ��� */
  }


	/* �������(scan_key[], mouse_dx, mouse_dy) ��Ͽ������ */

  key_record_playback();


	/* key_scan[0x0f] (���祤���ƥ��å�) �򥵥�����������ϥݡ��Ȥ�ȿ�� */

  switch( mouse_mode ){
  case	MOUSE_NONE:
    sound_reg[ 0x0e ] = 0xff;
    sound_reg[ 0x0f ] = 0xff;
    break;

  case	MOUSE_MOUSE:
    sound_reg[ 0x0f ] = ( IS_PAD_STATUS() >> 4 ) | 0xfc;
    break;

  case	MOUSE_JOYMOUSE:
  case	MOUSE_JOYSTICK:
    sound_reg[ 0x0e ] = ( IS_PAD_STATUS()      ) | 0xf0;
    sound_reg[ 0x0f ] = ( IS_PAD_STATUS() >> 4 ) | 0xfc;
/*printf("%02x\n",sound_reg[ 0x0e ]&0xff);*/
    break;
  }

}





/****************************************************************************
 *
 *	�������� ��Ͽ������
 *
 *****************************************************************************/

void	key_record_playback_init( void )
{
  int i;

  for( i=0; i<16; i++ ) key_record.key[i]     = 0xff;
  key_record.dx_h = 0;
  key_record.dx_l = 0;
  key_record.dy_h = 0;
  key_record.dy_l = 0;
  key_record.image[0] = -1;
  key_record.image[1] = -1;


  fp_pb  = NULL;
  fp_rec = NULL;

  if( file_pb && file_pb[0] ){			/* �����ѥե�����򥪡��ץ� */

    fp_pb = osd_fopen( FTYPE_KEY_PB, file_pb, "rb" );

    if( fp_pb ){
      if( verbose_proc )
	printf( "Key-Input Playback file <%s> ... OK\n", file_pb );
    }else{
      printf( "Can't open <%s>\nKey-Input PlayBack is invalid\n", file_pb );
    }
  }

  if( file_rec && file_rec[0] ){		/* ��Ͽ�ѥե�����򥪡��ץ� */

    fp_rec = osd_fopen( FTYPE_KEY_REC, file_rec, "wb" );

    if( fp_rec ){
      if( verbose_proc )
	printf( "Key-Input Record file <%s> ... OK\n", file_rec );
    }else{
      printf( "Can't open <%s>\nKey-Input Record is invalid\n", file_rec );
    }
  }
}


void	key_record_playback_term( void )
{
  if( fp_pb ){
    osd_fclose( fp_pb );
    fp_pb = NULL;
    if( file_pb ) file_pb[0] = '\0';
  }
  if( fp_rec ){
    osd_fclose( fp_rec );
    fp_rec = NULL;
    if( file_rec ) file_rec[0] = '\0';
  }
}


static	void	key_record_playback( void )
{
  int i, img;

  if( get_emu_mode() != EXEC ) return;

  if( fp_rec ){
    for( i=0; i<0x10; i++ )
      key_record.key[i] = key_scan[i];

    key_record.dx_h = (mouse_dx>>8) & 0xff;
    key_record.dx_l =  mouse_dx     & 0xff;
    key_record.dy_h = (mouse_dy>>8) & 0xff;
    key_record.dy_l =  mouse_dy     & 0xff;

    for( i=0; i<2; i++ ){
      if( disk_image_exist( i ) &&
	  drive_check_empty( i ) == FALSE )
	img = disk_image_selected(i) + 1;
      else
	img = -1;
      if( key_record.image[i] != img ) key_record.image[i] = img;
      else                             key_record.image[i] = 0;
    }

    if( osd_fwrite( &key_record, sizeof(char), sizeof(key_record), fp_rec )
							== sizeof(key_record)){
      ;
    }else{
      printf( "Can't write Record file <%s>\n", file_rec );
      osd_fclose( fp_rec );
      fp_rec = NULL;
    }
  }


  if( fp_pb ){

    if( osd_fread( &key_record, sizeof(char), sizeof(key_record), fp_pb )
							== sizeof(key_record)){
      for( i=0; i<0x10; i++ )
	key_scan[i] = key_record.key[i];

      mouse_dx  = (int)key_record.dx_h << 8;
      mouse_dx |=      key_record.dx_l;
      mouse_dy  = (int)key_record.dy_h << 8;
      mouse_dy |=      key_record.dy_l;

      for( i=0; i<2; i++ ){
	if( key_record.image[i]==-1 ){
	  drive_set_empty( i );
	}else if( disk_image_exist( i ) &&
		  key_record.image[i] > 0 &&
		  key_record.image[i] <= disk_image_num( i ) ){
	  drive_unset_empty( i );
	  disk_change_image( i, key_record.image[i]-1 );
	}
      }

    }else{
      printf(" (( %s : Playback file EOF ))\n", file_pb );
      osd_fclose( fp_pb );
      fp_pb = NULL;
    }
  }
}




/****************************************************************************
 *
 *	���ߥ�ǥޥ����򤤤�����˸ƤӽФ������ʬ
 *
 *****************************************************************************/

/*
 * ���� I/O �ݡ��Ƚ����
 *		���� I/O �ݡ��Ȥ������Ϣ����� ���ڤ��ؤ�ä����ν���
 */

void	jop1_init( void )
{
#if 1	/* - - - - - - - - - - - - - - - - - �ޥ�����Ϣ�Υ������������  */

#if 0						/* �ޥ�����ɸ�����������ۤ�*/
  init_mouse_position( &mouse_x, &mouse_y );	/* ���������⤷��ʤ����ɡ�  */
#endif						/* �����ޤǤ��ʤ��Ƥ⤤�ä�  */
  mouse_dx = 0;
  mouse_dy = 0;

#endif	/* - - - - - - - - �Ǥ⡢�ºݤˤϤ���򤷤ʤ��Ƥ�³��ϤǤʤ��Ȼפ���*/

  jop1_step = 0;
  jop1_dx = 0;
  jop1_dy = 0;
}



/*
 * ���� I/O �ݡ��� ���ȥ��� ON/OFF
 */

void	jop1_strobe( void )
{
  if( mouse_mode==MOUSE_MOUSE       &&		/* �ޥ��� ͭ�� */
      (sound_reg[ 0x07 ] & 0x80)==0 ){		/* ����I/O ��������� */

    switch( jop1_step ){

    case 0:		/* �ǽ�Υ��ȥ���(ON)�ǡ��ޥ�����ư�̤��ͤ���ꤷ  */
			/* 2���ܰʹߤΥ��ȥ��֤ǡ����γ��ꤷ���ͤ�ž������ */
		{
		  int dx = mouse_dx;			/* x ���� �Ѱ� */
		  int dy = mouse_dy;			/* y ���� �Ѱ� */

#if 1			/* �Ѱ̤��127���ϰ���˥���åԥ󥰤��� */
		  int f = 0;

			/* x��y�Τ��� �Ѱ̤� ��127��Ķ���Ƥ�������õ����     */
		  	/* �Ȥ��Ķ���Ƥ��顢�Ѱ̤��礭���ۤ���Ķ�����Ȥ��롣*/
		  if( dx < -127 || 127 < dx ) f |= 0x01;
		  if( dy < -127 || 127 < dy ) f |= 0x02;
		  if( f==0x03 ){
		    if( ABS(dx) > ABS(dy) ) f = 0x01;
		    else                    f = 0x02;
		  }
		  if( f==0x01 ){		/* x�Ѱ̤� ��127��Ķ������� */
		    				/* x�Ѱ̤�max�ͤˤ���y������ */
		    dy = 127 * SGN(dx) * dy / dx;
		    dx = 127 * SGN(dx);
		  }
		  else if( f==0x02 ){		/* y�Ѱ̤� ��127��Ķ������� */
						/* y�Ѱ̤�max�ͤˤ���x������ */
		    dx = 127 * SGN(dy) * dx / dy;
		    dy = 127 * SGN(dy);
		  }
#endif
		  mouse_dx -= dx;
		  mouse_dy -= dy;

		  jop1_dx = dx;
		  jop1_dy = dy;
		  /*printf("%d,%d\n",jop1_dx,jop1_dy);*/
		}
		sound_reg[ 0x0e ] = ((-jop1_dx)>>4) & 0x0f;	break;
    case 1:	sound_reg[ 0x0e ] =  (-jop1_dx)     & 0x0f;	break;
    case 2:	sound_reg[ 0x0e ] = ((-jop1_dy)>>4) & 0x0f;	break;
    case 3:	sound_reg[ 0x0e ] =  (-jop1_dy)     & 0x0f;	break;
    }

  }else{
    		sound_reg[ 0x0e ] = 0xff;
  }

  jop1_step = (jop1_step + 1) & 0x03;
}



/****************************************************************************
 *
 *	��˥塼�ǥ����򤤤�����˸ƤӽФ������ʬ
 *
 *****************************************************************************/

int	is_key_pressed( int code )
{
  if( IS_KEY88_LATTERTYPE( code ) ){
    code = code - KEY88_F6 + KEY88_END;
  }
  return((key_scan[ keyport[(code)].port ] & keyport[(code)].mask) == 0);
}
void	do_key_press( int code )
{
  if( IS_KEY88_LATTERTYPE( code ) ){
    do_lattertype( code, TRUE );
  }
  KEY88_PRESS(code);
}
void	do_key_release( int code )
{
  if( IS_KEY88_LATTERTYPE( code ) ){
    do_lattertype( code, FALSE );
  }
  KEY88_RELEASE(code);
}
void	do_key_all_release( void )
{
  int i;
  for( i=0; i<0x10; i++ ) key_scan[i] = 0xff;
}
void	do_key_bug( void )
{
  int	 my_port, your_port;
  byte my_val,  your_val,  save_val;

  save_val = key_scan[8] & 0xf0;	/* port 8 �� ��� 4bit ���оݳ� */
  key_scan[8] |= 0xf0;

  for( my_port=0; my_port<12; my_port++ ){
    for( your_port=0; your_port<12; your_port++ ){

      if( my_port==your_port ) continue;

      my_val   = key_scan[ my_port ];
      your_val = key_scan[ your_port ];

      if( ( my_val | your_val ) != 0xff ){
	key_scan[ my_port ]   =
	  key_scan[ your_port ] = my_val & your_val;
      }

    }
  }

  key_scan[8] &= ~0xf0;
  key_scan[8] |= save_val;
}







/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"KYBD"

static	T_SUSPEND_W	suspend_keyboard_work[] =
{
  { TYPE_INT,	&jop1_step		},
  { TYPE_INT,	&jop1_dx		},
  { TYPE_INT,	&jop1_dy		},

  { TYPE_INT,	&romaji_input_mode	},

  { TYPE_INT,	&mouse_mode		},
  { TYPE_INT,	&mouse_key_mode		},
  { TYPE_INT,	&mouse_key_assign[ 0]	},
  { TYPE_INT,	&mouse_key_assign[ 1]	},
  { TYPE_INT,	&mouse_key_assign[ 2]	},
  { TYPE_INT,	&mouse_key_assign[ 3]	},
  { TYPE_INT,	&mouse_key_assign[ 4]	},
  { TYPE_INT,	&mouse_key_assign[ 5]	},

  { TYPE_INT,	&joy_key_mode		},
  { TYPE_INT,	&joy_key_assign[ 0]	},
  { TYPE_INT,	&joy_key_assign[ 1]	},
  { TYPE_INT,	&joy_key_assign[ 2]	},
  { TYPE_INT,	&joy_key_assign[ 3]	},
  { TYPE_INT,	&joy_key_assign[ 4]	},
  { TYPE_INT,	&joy_key_assign[ 5]	},
  { TYPE_INT,	&joy_key_assign[ 6]	},
  { TYPE_INT,	&joy_key_assign[ 7]	},
  { TYPE_INT,	&joy_key_assign[ 8]	},
  { TYPE_INT,	&joy_key_assign[ 9]	},
  { TYPE_INT,	&joy_key_assign[10]	},
  { TYPE_INT,	&joy_key_assign[11]	},
  { TYPE_INT,	&joy_swap_button	},

  { TYPE_INT,	&cursor_key_mode	},
  { TYPE_INT,	&cursor_key_assign[0]	},
  { TYPE_INT,	&cursor_key_assign[1]	},
  { TYPE_INT,	&cursor_key_assign[2]	},
  { TYPE_INT,	&cursor_key_assign[3]	},

  { TYPE_INT,	&tenkey_emu		},
  { TYPE_INT,	&numlock_emu		},

  { TYPE_INT,	&function_f[ 1]		},
  { TYPE_INT,	&function_f[ 2]		},
  { TYPE_INT,	&function_f[ 3]		},
  { TYPE_INT,	&function_f[ 4]		},
  { TYPE_INT,	&function_f[ 5]		},
  { TYPE_INT,	&function_f[ 6]		},
  { TYPE_INT,	&function_f[ 7]		},
  { TYPE_INT,	&function_f[ 8]		},
  { TYPE_INT,	&function_f[ 9]		},
  { TYPE_INT,	&function_f[10]		},
  { TYPE_INT,	&function_f[11]		},
  { TYPE_INT,	&function_f[12]		},
  { TYPE_INT,	&function_f[13]		},
  { TYPE_INT,	&function_f[14]		},
  { TYPE_INT,	&function_f[15]		},
  { TYPE_INT,	&function_f[16]		},
  { TYPE_INT,	&function_f[17]		},
  { TYPE_INT,	&function_f[18]		},
  { TYPE_INT,	&function_f[19]		},
  { TYPE_INT,	&function_f[20]		},

  { TYPE_INT,	&romaji_type		},

  { TYPE_END,	0			},
};


int	statesave_keyboard( void )
{
  if( statesave_table( SID, suspend_keyboard_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}

int	stateload_keyboard( void )
{
  if( stateload_table( SID, suspend_keyboard_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}
