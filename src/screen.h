#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED


/*
 *	PC-88 Related
 */

typedef struct{
  unsigned	char	blue;			/* B�̵��� (0��7)	*/
  unsigned	char	red;			/* R�̵��� (0��7)	*/
  unsigned 	char	green;			/* G�̵��� (0��7)	*/
  unsigned 	char	padding;
} PC88_PALETTE_T;

extern	PC88_PALETTE_T	vram_bg_palette;	/* �طʥѥ�å�	*/
extern	PC88_PALETTE_T	vram_palette[8];	/* �Ƽ�ѥ�å�	*/

extern	byte	sys_ctrl;		/* OUT[30] SystemCtrl     	*/
extern	byte	grph_ctrl;		/* OUT[31] GraphCtrl      	*/
extern	byte	grph_pile;		/* OUT[53] �Ť͹�碌     	*/

#define	SYS_CTRL_80		(0x01)		/* TEXT COLUMN80 / COLUMN40*/
#define	SYS_CTRL_MONO		(0x02)		/* TEXT MONO     / COLOR   */

#define	GRPH_CTRL_200		(0x01)		/* VRAM-MONO 200 / 400 line*/
#define	GRPH_CTRL_64RAM		(0x02)		/* RAM   64K-RAM / ROM-RAM */
#define	GRPH_CTRL_N		(0x04)		/* BASIC       N / N88     */
#define GRPH_CTRL_VDISP		(0x08)		/* VRAM  DISPLAY / UNDISP  */
#define GRPH_CTRL_COLOR		(0x10)		/* VRAM  COLOR   / MONO    */
#define	GRPH_CTRL_25		(0x20)		/* TEXT  LINE25  / LINE20  */

#define	GRPH_PILE_TEXT		(0x01)		/* �Ť͹�碌 ��ɽ�� TEXT  */
#define	GRPH_PILE_BLUE		(0x02)		/*		       B   */
#define	GRPH_PILE_RED		(0x04)		/*		       R   */
#define	GRPH_PILE_GREEN		(0x08)		/*		       G   */



/*
 *	��������ѥ��
 */

	/* ���躹ʬ���� */

extern	char	screen_update[ 0x4000*2 ];	/* ����ɽ����ʬ�����ե饰 */
extern	int	screen_update_force;		/* ���̶��������ե饰	  */
extern	int	screen_update_palette;		/* �ѥ�åȹ����ե饰	  */

#define	set_screen_update( x )		screen_update[x] = 1
#define	set_screen_update_force()	screen_update_force = TRUE
#define	set_screen_update_palette()	screen_update_palette = TRUE


	/* �ƥ����Ƚ��� */

extern	int	text_attr_flipflop;
extern	Ushort	text_attr_buf[2][2048];


	/* ����¾ */

extern	int	frameskip_rate;		/* ����ɽ���ι����ֳ�		*/
extern	int	monitor_analog;		/* ���ʥ���˥���		*/

extern	int	use_auto_skip;		/* �����ȥե졼�ॹ���åפ���Ѥ��� */
extern	int	do_skip_draw;		/* �����꡼��ؤ�����򥹥��åפ��� */
extern	int	already_skip_draw; 	/* �����åפ�����		    */



/*
 *	ɽ���ǥХ����ѥ��
 */


extern	int	hide_mouse;		/* �ޥ����򱣤����ɤ���		*/
extern	int	grab_mouse;		/* ����֤��뤫�ɤ���		*/

extern	int	use_interlace;		/* ���󥿡��졼��ɽ��		*/
extern	int	use_half_interp;	/* ���̥�����Ⱦʬ��������֤��� */


enum {					/* ���̥�����			*/
  SCREEN_SIZE_HALF,			/*		320 x 200	*/
  SCREEN_SIZE_FULL,			/*		640 x 400	*/
#ifdef	SUPPORT_DOUBLE
  SCREEN_SIZE_DOUBLE,			/*		1280x 800	*/
#endif
  SCREEN_SIZE_END
};

typedef	struct{				/* ���̥������Υꥹ��		*/
  int	w,  h;
  int	dw, dh;
} SCREEN_SIZE_TABLE;

extern const SCREEN_SIZE_TABLE screen_size_tbl[ SCREEN_SIZE_END ];

#define	STATUS_HEIGHT	(20)


extern	int	screen_size;		/* ���̥���������		*/
extern	int	use_fullscreen;		/* ������ɽ������		*/

extern	int	WIDTH;			/* ����Хåե���������		*/
extern	int	HEIGHT;			/* ����Хåե��ĥ�����		*/
extern	int	DEPTH;			/* ���ӥåȿ�	(8/16/32)	*/
extern	int	SCREEN_W;		/* ���̲������� (320/640/1280)	*/
extern	int	SCREEN_H;		/* ���̽ĥ����� (200/400/800)	*/

extern	char	*screen_buf;		/* ����Хåե���Ƭ		*/
extern	char	*screen_start;		/* ������Ƭ			*/


extern	int	show_status;		/* ���ơ�����ɽ��̵ͭ		*/

extern	char	*status_buf;		/* ���ơ��������� ��Ƭ		*/
extern	char	*status_start[3];	/* ���ơ��������� ��Ƭ		*/
extern	int	status_sx[3];		/* ���ơ��������襵����		*/
extern	int	status_sy[3];




extern	Ulong	color_pixel[16];		/* ��������		*/
extern	Ulong	color_half_pixel[16][16];	/* ���䴰���ο�������	*/
extern	Ulong	black_pixel;			/* ���ο�������		*/

enum {						/* ���ơ������˻Ȥ���	*/
  STATUS_BG,					/*	�طʿ�(��)	*/
  STATUS_FG,					/*	���ʿ�(��)	*/
  STATUS_BLACK,					/*	����		*/
  STATUS_WHITE,					/*	��		*/
  STATUS_RED,					/*	�ֿ�		*/
  STATUS_GREEN,					/*	�п�		*/
  STATUS_COLOR_END
};
extern	Ulong	status_pixel[STATUS_COLOR_END];	/* ���ơ������ο�������	*/


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
                          WIDTH
	 ������������������������������������������
	�������������������������������������������� ��
	��                                        �� ��
	��     ����������������������             �� ��
	��     ��  ��              ��             �� ��
	��     ����������������������             �� ��
	��     ��  ��   SCREEN_SX  ��             �� ��
	��     ��  ��              ��             �� ��HEIGHT
	��     ��  ��SCREEN_SY     ��             �� ��
	��     ��  ��              ��             �� ��
	��     ����������������������             �� ��
	��                                        �� ��
	��                                        �� ��
	��                                        �� ��
	�������������������������������������������� ��
	�����ơ�����0 �����ơ�����1 �����ơ�����2 �� ��STATUS_HEIGHT
	�������������������������������������������� ��

	screen_buf	����Хåե�����Ρ���Ƭ�ݥ���
	WIDTH		����Хåե�����Ρ����ԥ������
	HEIGHT		        ��          �ĥԥ������

	screen_size	���̥�����
	screen_start	���̥Хåե��Ρ���Ƭ�ݥ���
	SCREEN_SX	���̥Хåե��Ρ����ԥ������ (320/640/1280)
	SCREEN_SY	      ��        �ĥԥ������ (200/400/800)

	DEPTH		������ (screen_buf, screen_start �Υӥå�����8/16/32)

	status_buf	���ơ������Хåե�����Ρ���Ƭ�ݥ���
	status_start[3]	���ơ����� 0��2 �ΥХåե��Ρ���Ƭ�ݥ���
	status_sx[3]		��		      ���ԥ������
	status_sy[3]		��		      �ĥԥ������


	�� ������ɥ�ɽ���ξ�硢
		WIDTH x (HEIGHT + STATUS_HEIGHT) �Υ������ǡ�
		������ɥ����������ޤ���
			( ���ơ�������ɽ���ʤ顢 WIDTH x HEIGHT )

	�� ������ɽ���ξ�硢
		SCREEN_SX x (SCREEN_SY + STATUS_HEIGHT) �ʾ�Υ�������
		�����̲����ޤ���
			( ���ơ�������ɽ���ʤ顢������ʬ�Ϲ����ɤ�Ĥ֤� )

   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


/*
 *
 */

void	draw_screen( void );
void	draw_screen_force( void );

void	draw_status( void );


void	draw_menu_screen( void );
void	draw_menu_screen_force( void );



void	blink_ctrl_update( void );	/* �֥�󥯤Υ���򹹿�����	*/
void	reset_frame_counter( void );

void	screen_buf_init( void );
void	status_buf_init( void );

#endif	/* SCREEN_H_INCLUDED */
