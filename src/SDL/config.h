#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* SDL �С�������ͭ�����						*/
/*----------------------------------------------------------------------*/

#include <SDL_main.h>
#include <SDL_byteorder.h>


/* ����ǥ�����ͥ�������å� */
#if	( SDL_BYTEORDER == SDL_LIL_ENDIAN )
#define	LSB_FIRST
#else
#undef	LSB_FIRST
#endif



/* SDL�� QUASI88 �Τ���μ����� */

#ifndef	QUASI88_SDL
#define	QUASI88_SDL
#endif



/* ��˥塼�� �����ȥ롦�С������ɽ���ˤ��ɲä�ɽ��������� */

#define	Q_COMMENT	"SDL version"



/* SDL�Ǥ� 16bpp(bit per pixel) ����Ȥ��� */

#ifndef	SUPPORT_16BPP
#define	SUPPORT_16BPP
#endif
#undef	SUPPORT_8BPP
#undef	SUPPORT_32BPP



/*
  MAME/XMAME �Υ�����ɤ��Ȥ߹����硢
	USE_SOUND
  ��������Ƥ�����

  FMGEN ���Ȥ߹�����ϡ�
	USE_FMGEN
  ��������Ƥ�����

  �嵭�ϡ�����ѥ�����˰ʲ��Τ褦�ˤ���������롣
  gcc  �ξ�硢����ѥ���˥��ץ���� -DUSE_SOUND   �ʤɤȻ��ꤹ�롣
  VC++ �ξ�硢����ѥ���˥��ץ���� /D"USE_SOUND" �ʤɤȻ��ꤹ�롣
  MWP  �ξ�硢����ѥ���˥��ץ���� -d USE_SOUND  �ʤɤȻ��ꤹ�롣
*/




/*
 *	VC++ depend
 */

#ifdef	_MSC_VER

/* VC �Υ���饤�󥭡���� */
#define	INLINE		__inline


/* ������ɥɥ饤���Ѥˡ�PI(��)��M_PI(��)����� ��  MSC�ʳ��Ǥ�����? ɬ��? */
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif

#endif



/*
 *	SC depend
 */

#ifdef	macintosh

/* ������ɥɥ饤���Ѥˡ�PI(��)��M_PI(��)����� ��  SC�Ǥ�ɬ��? */
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif

#endif



#endif	/* CONFIG_H_INCLUDED */
