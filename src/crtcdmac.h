#ifndef CRTCDMAC_H_INCLUDED
#define CRTCDMAC_H_INCLUDED



		/* CRTC¦���鸫�����ȥ�ӥ塼��	*/

#define MONO_SECRET	0x01
#define MONO_BLINK	0x02
#define MONO_REVERSE	0x04
#define MONO_UPPER	0x10
#define MONO_UNDER	0x20
#define MONO_GRAPH	0x80
#define COLOR_SWITCH	0x08
#define COLOR_GRAPH	0x10
#define COLOR_B		0x20
#define COLOR_R		0x40
#define COLOR_G		0x80

		/* ����ɽ���ǻ��Ѥ��륢�ȥ�ӥ塼�� */

#define ATTR_REVERSE	0x01			/* ȿž			*/
#define ATTR_SECRET	0x02			/* ɽ��/��ɽ��		*/
#define ATTR_UPPER	0x04			/* ���åѡ��饤��	*/
#define ATTR_LOWER	0x08			/* ��������饤��	*/
#define ATTR_GRAPH	0x10			/* ����ե��å��⡼��	*/
#define ATTR_B		0x20			/* �� Blue		*/
#define ATTR_R		0x40			/* �� Reg		*/
#define ATTR_G		0x80			/* �� Green		*/

#define MONO_MASK	0x0f
#define COLOR_MASK	0xe0



extern	int	crtc_active;		/* CRTC�ξ��� 0:CRTC��ư 1:CRTC��� */
extern	int	crtc_intr_mask;		/* CRTC�γ���ޥ��� ==3 ��ɽ��	    */
extern	int	crtc_cursor[2];		/* ����������� ��ɽ���λ���(-1,-1) */
extern	byte	crtc_format[5];		/* CRTC �������Υե����ޥå�	    */

extern	int	crtc_reverse_display;	/* ����ȿžɽ�� / �����̾�ɽ��	*/
extern	int	crtc_skip_line;		/* ����1�����Ф�ɽ�� / �����̾� */
extern	int	crtc_cursor_style;	/* �֥�å� / ������饤��	*/
extern	int	crtc_cursor_blink;	/* �������Ǥ��� �������Ǥ��ʤ�	*/
extern	int	crtc_attr_non_separate;	/* ����VRAM��ATTR ����ߤ��¤�	*/
extern	int	crtc_attr_color;	/* ���ĥ��顼 �������		*/
extern	int	crtc_attr_non_special;	/* ���ĹԤν���� ATTR ���¤�	*/

extern	int	CRTC_SZ_LINES;		/* ɽ�������� (20/25)		*/
#define		CRTC_SZ_COLUMNS	(80)	/* ɽ������Կ� (80����)	*/
extern	int	crtc_sz_lines;		/* ��� (20��25)		*/
extern	int	crtc_sz_columns;	/* �Կ� (2��80)			*/
extern	int	crtc_sz_attrs;		/* °���� (1��20)		*/
extern	int	crtc_byte_per_line;	/* 1�Ԥ�����Υ��� �Х��ȿ�	*/
extern	int	crtc_font_height;	/* �ե���Ȥι⤵ �ɥåȿ�(8/10)*/


extern	pair	dmac_address[4];
extern	pair	dmac_counter[4];
#define	text_dma_addr	dmac_address[2]

extern	int	dmac_mode;



	/**** �ƥ�����ɽ�� ****/

enum {
  TEXT_DISABLE,		/* �ƥ�����ɽ���ʤ�				*/
  TEXT_ATTR_ONLY,	/*  ��    â��°���Τ�ͭ�� (�������ե��å���)	*/
  TEXT_ENABLE,		/* �ƥ�����ɽ������				*/
  End_of_TEXT
};
extern	int	text_display;		/* �ƥ�����ɽ�����֥ե饰	*/
extern	int	blink_cycle;		/* ���Ǥμ���	8/16/24/32	*/
extern	int	blink_counter;		/* �������楫����		*/


/* ���Υޥ���ϡ�CRTC,DMAC���������ӡ�I/O 31H / 53H ���ϻ��˸Ƥ�	*/

#define	set_text_display()						\
	do{								\
	  if( (dmac_mode & 0x4) && (crtc_active) && crtc_intr_mask==3){	\
	    if( !(grph_pile & GRPH_PILE_TEXT) ){			\
	      text_display = TEXT_ENABLE;				\
	    }else{							\
	      if( grph_ctrl & GRPH_CTRL_COLOR )				\
		text_display = TEXT_DISABLE;				\
	      else							\
		text_display = TEXT_ATTR_ONLY;				\
	    }								\
	  }else{							\
	    text_display = TEXT_DISABLE;				\
	  }								\
	}while(0)



extern	int	dma_wait_count;		/* DMA�Ǿ��񤹤륵�������	*/

#define	SET_DMA_WAIT_COUNT()	dma_wait_count =			\
					crtc_byte_per_line * crtc_sz_lines

#define	RESET_DMA_WAIT_COUNT()	dma_wait_count = 0



void	crtc_init( void );

void	crtc_out_command( byte data );
void	crtc_out_parameter( byte data );
byte	crtc_in_status( void );
byte	crtc_in_parameter( void );


void	dmac_init( void );

void	dmac_out_mode( byte data );
byte	dmac_in_status( void );
void	dmac_out_address( byte addr, byte data );
void	dmac_out_counter( byte addr, byte data );
byte	dmac_in_address( byte addr );
byte	dmac_in_counter( byte addr );





#undef	SUPPORT_CRTC_SEND_SYNC_SIGNAL

#ifdef	SUPPORT_CRTC_SEND_SYNC_SIGNAL

void	crtc_send_sync_signal( int flag );
#define	set_crtc_sync_bit()	crtc_send_sync_signal( 1 )
#define	clr_crtc_sync_bit()	crtc_send_sync_signal( 0 )

#else

#define	set_crtc_sync_bit()	((void)0)
#define	clr_crtc_sync_bit()	((void)0)

#endif


#endif	/* CRTCDMAC_H_INCLUDED */
