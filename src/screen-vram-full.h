#undef	IF_LINE200_OR_EVEN_LINE
#undef	DST_DEFINE
#undef	DST_NEXT_LINE
#undef	DST_RESTORE_LINE
#undef	DST_NEXT_CHARA
#undef	DST_NEXT_TOP_CHARA

#if	defined( LINE200 )
/*=============================================================================
  �������(200�饤��)�⡼�ɤξ�� (�ʲ��ϡ��ƥ�����25�Ԥ���)

	  vram		�Хåե�	vram     1�饤�� �ˤĤ�
	    +--+	    +--+	�Хåե� 2�饤�� �����褹�롣
	 8  |  |	16  |  |
	    +--+	    |  |	vram �����饤����� 8��
			    +--+	�ե���ȥǡ����� vram 1�饤�����ɬ��

=============================================================================*/
#define	IF_LINE200_OR_EVEN_LINE()	    /* nothing */

#define	DST_DEFINE()		int     dst_w = SCREEN_WIDTH;		\
				TYPE	*dst  = (TYPE *)SCREEN_START;	\
				TYPE	*dst2 = dst + dst_w;



#define	DST_NEXT_LINE()		dst  += (2*dst_w);			\
				dst2 += (2*dst_w);



#define	DST_RESTORE_LINE()	dst  -= CHARA_LINES * 2*dst_w;		\
				dst2 -= CHARA_LINES * 2*dst_w;



#define	DST_NEXT_CHARA()	dst  += 8*COLUMN_SKIP;			\
				dst2 += 8*COLUMN_SKIP;



#define	DST_NEXT_TOP_CHARA()	dst  += CHARA_LINES *2 *dst_w - 640;	\
				dst2 += CHARA_LINES *2 *dst_w - 640;



#elif	defined( LINE400 )
/*=============================================================================
  �������(400�饤��)�⡼�ɤξ�� (�ʲ��ϡ��ƥ�����25�Ԥ���)

	  vram		�Хåե�	vram     1�饤�� �ˤĤ�
	    +--+	    +--+	�Хåե� 1�饤�� �����褹�롣
	16  |  |	16  |  |
	    |  |	    |  |	vram �����饤����� 16��
	    +--+	    +--+	�ե���ȥǡ����� vram 2�饤�����ɬ��

=============================================================================*/
#define	IF_LINE200_OR_EVEN_LINE()	if( (k&1)==0)

#define	DST_DEFINE()		int	dst_w = SCREEN_WIDTH;		\
				TYPE	*dst  = (TYPE *)SCREEN_START;


#define	DST_NEXT_LINE()		dst  += dst_w;


#define	DST_RESTORE_LINE()	dst  -= CHARA_LINES * dst_w;


#define	DST_NEXT_CHARA()	dst  += 8*COLUMN_SKIP;


#define	DST_NEXT_TOP_CHARA()	dst  += CHARA_LINES * dst_w - 640;


#endif
