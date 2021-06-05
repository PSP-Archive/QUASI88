#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* UNIX/X11 �С�������ͭ�����					*/
/*----------------------------------------------------------------------*/

/*
  ����ǥ�����ͥ��ϡ�����ѥ������ (Makefile) Ϳ���롣
  ��ȥ륨��ǥ�����ʤ顢
	LSB_FIRST
  ��������Ƥ���
*/



/* X11�� QUASI88 �Τ���μ����� */

#ifndef	QUASI88_X11
#define	QUASI88_X11
#endif



/* ��˥塼�� �����ȥ롦�С������ɽ���ˤ��ɲä�ɽ��������� */

#define	Q_COMMENT	"UNIX/X11 version"



/*
  ���ݡ��Ȥ��� bpp �ϡ�����ѥ������ (Makefile) Ϳ���롣
	SUPPORT_8BPP
	SUPPORT_16BPP
	SUPPORT_32BPP
  �Τ�����ɬ�פʤ�Τ�������Ƥ�����
	SUPPORT_DOUBLE
  �⹥�ߤ�������Ƥ�����
*/


/*
  ������ɤΥ��ݡ��Ȥϡ�����ѥ������ (Makefile) Ϳ���롣

  MAME/XMAME �Υ�����ɤ��Ȥ߹����硢
	USE_SOUND
  ��������Ƥ�����

  FMGEN ���Ȥ߹�����ϡ�
	USE_FMGEN
  ��������Ƥ�����

  �ܺ٤ϡ� Makefile ��ߤƤ��硣
*/

#endif	/* CONFIG_H_INCLUDED */
