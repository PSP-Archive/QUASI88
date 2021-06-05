/******************************************************************************
 *	VRAM / TEXT �Ρ��������褫�鹹�����줿��ʬ����������
 *****************************************************************************/

#define	VRAM2SCREEN_DIFF( vram2screen_diff_function )			      \
int	vram2screen_diff_function( void )				      \
{									      \
  int	x0 = COLUMNS-1, x1 = 0, y0 = ROWS-1, y1 = 0;			      \
  int	i, j, k;							      \
  int	changed_line;		/* ��������饤���ӥå�(0��COLUMN)��ɽ�� */ \
									      \
  unsigned short text, *text_attr= &text_attr_buf[ text_attr_flipflop   ][0]; \
  unsigned short old,  *old_attr = &text_attr_buf[ text_attr_flipflop^1 ][0]; \
  T_GRYPH fnt;				/* �ե���Ȥλ��� 1ʸ��ʬ   */	      \
  int	fnt_idx;			/* �ե���Ȥλ��� ���Ȱ���  */	      \
  bit8	style = 0;			/* �ե���Ȥλ��� 8�ɥå�ʬ */	      \
  int	tpal;				/* �ե���Ȥο�������       */	      \
  int	tcol;				/* �ե���Ȥο�             */	      \
									      \
  char	*up = &screen_update[0];	/* VRAM�����ե饰�ؤΥݥ��� */      \
  bit32	*src  = main_vram4;		/* VRAM�ؤΥݥ���           */      \
  DST_DEFINE()				/* ���襨�ꥢ�ؤΥݥ���     */      \
  WORK_DEFINE()				/* ������ɬ�פʥ��         */      \
									      \
									      \
  for( i=0; i<ROWS; i++ ){  /*=============================================*/ \
					/* 1ʸ��ñ�̤����褷�Ƥ����Τǡ�   */ \
					/* ʸ���� (�ԡ���) ʬ���롼�פ�����*/ \
				/*-----------------------------------------*/ \
    for( j=0; j<COLUMNS; j+=COLUMN_SKIP ){	/* 40��ʤ�2ʸ����˽���   */ \
									      \
      text = *text_attr;  text_attr += COLUMN_SKIP;	/* �ƥ����ȥ����� */  \
      old  = *old_attr;   old_attr  += COLUMN_SKIP;	/* ��°�������   */  \
									      \
      if( text != old ){			/* �ƥ����ȿ����԰��� ?    */ \
	changed_line = ~(0);			    /* yes 1ʸ��ʬ�������� */ \
      }									      \
      else{					    /* no �����Ľ�����å� */ \
	changed_line = 0;						      \
	for( k=0; k<CHARA_LINES; k++ ){					      \
	  if( up[ k*80 ] || 						      \
	      up[ k*80 + (COLUMN_SKIP-1) ] ){ changed_line |= (1<<k); }	      \
	}								      \
      }									      \
									      \
      if( changed_line ){	    /* �����줫�Υ饤������褹����      */ \
        if(i<y0) y0=i;  if(i>y1) y1=i;  if(j<x0) x0=j;  if(j>x1) x1=j;	      \
									      \
	get_font_gryph( text, &fnt, &tpal );	/* �ե���Ȥη������      */ \
	tcol = COLOR_PIXEL( tpal );					      \
	fnt_idx = 0;							      \
					/*-------------------------------*/   \
	for( k=0; k<CHARA_LINES; k++ ){	    /* ʸ���Υ饤���ʬ�롼�פ���*/   \
									      \
	  IF_LINE200_OR_EVEN_LINE()	    /* 200line��400line�Ƕ���line*/   \
	    style = fnt.b[ fnt_idx++ ];	    /* �λ���,�ե����8�ɥåȼ���*/   \
									      \
	  if( changed_line & (1<<k) ){	    /* ���Υ饤������褹����*/     \
									      \
	    if      ( style==0xff ){		/* TEXT�� �Τߤ�����   */     \
	      MASK_DOT();			/*                     */     \
	    }else if( style==0x00 ){		/* VRAM�� �Τߤ�����   */     \
	      TRANS_DOT();			/*                     */     \
	    }else{				/* TEXT/VRAM��������   */     \
	      STORE_DOT();			/*                     */     \
	    }					/*                     */     \
	    COPY_DOT();				/* �饤��� ������   */     \
	  }				    /*                         */     \
									      \
	  DST_NEXT_LINE();		    /* ���Υ饤����֤˿ʤ�      */   \
	}				/*-------------------------------*/   \
									      \
	DST_RESTORE_LINE();		/* �饤����Ƭ���᤹                */ \
      }				    /*                                     */ \
									      \
									      \
      up  += COLUMN_SKIP;						      \
      src += COLUMN_SKIP;						      \
      DST_NEXT_CHARA();			/* ����ʸ�����֤˿ʤ�              */ \
    }				/*-----------------------------------------*/ \
									      \
    up  += (CHARA_LINES-1)* 80;						      \
    src += (CHARA_LINES-1)* 80;						      \
    DST_NEXT_TOP_CHARA();		/* ���ιԤ���Ƭʸ�����֤˿ʤ�      */ \
  }                         /*=============================================*/ \
									      \
  if( x0 <= x1 ){							      \
    return ( ((x0            )<<24) | (((y0  )*(200/ROWS))<<16) |	      \
             ((x1+COLUMN_SKIP)<< 8) | (((y1+1)*(200/ROWS))    ) );	      \
  }else{								      \
    return -1;								      \
  }									      \
}



/******************************************************************************
 *	VRAM / TEXT ��������ʬ������
 *****************************************************************************/

#define	VRAM2SCREEN_ALL( vram2screen_all_function )			      \
int	vram2screen_all_function( void )				      \
{									      \
									      \
  int	i, j, k;							      \
									      \
									      \
  unsigned short text, *text_attr= &text_attr_buf[ text_attr_flipflop   ][0]; \
									      \
  T_GRYPH fnt;				/* �ե���Ȥλ��� 1ʸ��ʬ   */	      \
  int	fnt_idx;			/* �ե���Ȥλ��� ���Ȱ���  */	      \
  bit8	style = 0;			/* �ե���Ȥλ��� 8�ɥå�ʬ */	      \
  int	tpal;				/* �ե���Ȥο�������       */	      \
  int	tcol;				/* �ե���Ȥο�             */	      \
									      \
									      \
  bit32	*src  = main_vram4;		/* VRAM�ؤΥݥ���           */      \
  DST_DEFINE()				/* ���襨�ꥢ�ؤΥݥ���     */      \
  WORK_DEFINE()				/* ������ɬ�פʥ��         */      \
									      \
									      \
  for( i=0; i<ROWS; i++ ){  /*=============================================*/ \
					/* 1ʸ��ñ�̤����褷�Ƥ����Τǡ�   */ \
					/* ʸ���� (�ԡ���) ʬ���롼�פ�����*/ \
				/*-----------------------------------------*/ \
    for( j=0; j<COLUMNS; j+=COLUMN_SKIP ){	/* 40��ʤ�2ʸ����˽���   */ \
									      \
      text = *text_attr;  text_attr += COLUMN_SKIP;	/* �ƥ����ȥ����� */  \
									      \
									      \
									      \
									      \
									      \
									      \
									      \
									      \
									      \
									      \
									      \
									      \
									      \
      {				    /* 1ʸ��ñ�̤����褷�Ƥ���             */ \
									      \
									      \
	get_font_gryph( text, &fnt, &tpal );	/* �ե���Ȥη������      */ \
	tcol = COLOR_PIXEL( tpal );					      \
	fnt_idx = 0;							      \
					/*-------------------------------*/   \
	for( k=0; k<CHARA_LINES; k++ ){	    /* ʸ���Υ饤���ʬ�롼�פ���*/   \
									      \
	  IF_LINE200_OR_EVEN_LINE()	    /* 200line��400line�Ƕ���line*/   \
	    style = fnt.b[ fnt_idx++ ];	    /* �λ���,�ե����8�ɥåȼ���*/   \
									      \
	  {				    /* �饤��ñ�̤�����        */     \
									      \
	    if      ( style==0xff ){		/* TEXT�� �Τߤ�����   */     \
	      MASK_DOT();			/*                     */     \
	    }else if( style==0x00 ){		/* VRAM�� �Τߤ�����   */     \
	      TRANS_DOT();			/*                     */     \
	    }else{				/* TEXT/VRAM��������   */     \
	      STORE_DOT();			/*                     */     \
	    }					/*                     */     \
	    COPY_DOT();				/* �饤��� ������   */     \
	  }				    /*                         */     \
									      \
	  DST_NEXT_LINE();		    /* ���Υ饤����֤˿ʤ�      */   \
	}				/*-------------------------------*/   \
									      \
	DST_RESTORE_LINE();		/* �饤����Ƭ���᤹                */ \
      }				    /*                                     */ \
									      \
									      \
									      \
      src += COLUMN_SKIP;						      \
      DST_NEXT_CHARA();			/* ����ʸ�����֤˿ʤ�              */ \
    }				/*-----------------------------------------*/ \
									      \
									      \
    src += (CHARA_LINES-1)* 80;						      \
    DST_NEXT_TOP_CHARA();		/* ���ιԤ���Ƭʸ�����֤˿ʤ�      */ \
  }                         /*=============================================*/ \
									      \
									      \
  return ( ((0)<<24) | ((0)<<16) | ((COLUMNS)<<8) | (200) );		      \
}
