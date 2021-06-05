#ifndef QUASI88_H_INCLUDED
#define QUASI88_H_INCLUDED


/*----------------------------------------------------------------------*/
/* �����ƥࡦ�Ķ���¸�����						*/
/*----------------------------------------------------------------------*/
#include "config.h"


/*----------------------------------------------------------------------*/
/* �ե����륷���ƥ��¸�����						*/
/*----------------------------------------------------------------------*/
#include "filename.h"


/* QUASI88 ���������ݻ���ǽ�ʥѥ����ߤΥե�����̾�Х��ȿ� (NUL�ޤ�) */
#define	QUASI88_MAX_FILENAME	(1024)


/*----------------------------------------------------------------------*/
/* �С���������							*/
/*----------------------------------------------------------------------*/
#include "version.h"



/*----------------------------------------------------------------------*/
/* �������								*/
/*----------------------------------------------------------------------*/

typedef	unsigned char	Uchar;
typedef	unsigned short	Ushort;
typedef	unsigned int	Uint;
typedef	unsigned long	Ulong;

typedef unsigned char  byte;
typedef unsigned short word;
typedef signed   char  offset;


typedef unsigned char  bit8;
typedef unsigned short bit16;
typedef unsigned int   bit32;


#define	COUNTOF(arr)	(int)(sizeof(arr)/sizeof((arr)[0]))
#define	OFFSETOF(s, m)	((size_t)(&((s *)0)->m))
#define	MAX(a,b)	(((a)>(b))?(a):(b))
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#define	ABS(x)		( ((x) >= 0)? (x) : -(x) )
#define	SGN( x )	( ((x)>0) ? 1 : ( ((x)<0) ? -1 : 0 ) ) 


#ifdef LSB_FIRST			/* ��ȥ륨��ǥ����� */

typedef union
{
  struct { byte l,h; }	B;
  word			W;
} pair;

#else					/* �ӥå�����ǥ����� */

typedef union
{
  struct { byte h,l; }	B;
  word			W;
} pair;

#endif



#ifndef TRUE
#define	TRUE	(1)
#endif
#ifndef FALSE
#define	FALSE	(0)
#endif



#ifndef	INLINE
#define	INLINE	static
#endif



/*----------------------------------------------------------------------*/
/* �ѿ� (verbose_*)���ؿ�						*/
/*----------------------------------------------------------------------*/
extern	int	verbose_level;		/* ��Ĺ��٥�			*/
extern	int	verbose_proc;		/* �����οʹԾ�����ɽ��		*/
extern	int	verbose_z80;		/* Z80�������顼��ɽ��		*/
extern	int	verbose_io;		/* ̤���� I/O�������������	*/
extern	int	verbose_pio;		/* PIO ���������Ѥ�ɽ��		*/
extern	int	verbose_fdc;		/* FD���᡼���۾�����		*/
extern	int	verbose_wait;		/* ���������Ԥ����ΰ۾�����	*/
extern	int	verbose_suspend;	/* �����ڥ�ɻ��ΰ۾�����	*/
extern	int	verbose_snd;		/* ������ɤΥ�å�����		*/


#define	INIT_POWERON	(0)
#define	INIT_RESET	(1)
#define	INIT_STATELOAD	(2)

int	quasi88( void );
void	quasi88_atexit( void (*function)(void) );
void	quasi88_exit( void );
void	quasi88_reset( void );
int	quasi88_stateload( void );



char	*alloc_diskname( const char *filename );
char	*alloc_romname( const char *filename );
char	*alloc_global_cfgname( void );
char	*alloc_local_cfgname( const char *imagename );
char	*alloc_state_filename( int init );
char	*alloc_snapshot_filename( int init );



/*----------------------------------------------------------------------*/
/* ����¾	(���Τϡ� quasi88.c  �ˤ�������Ƥ���)			*/
/*----------------------------------------------------------------------*/
void	sjis2euc( char *euc_p, const char *sjis_p );
void	euc2sjis( char *sjis_p, const char *euc_p );
int	euclen( const char *euc_p );
int	my_strcmp( const char *s, const char *d );
void	my_strncpy( char *s, const char *ct, unsigned long n );
void	my_strncat( char *s, const char *ct, unsigned long n );



/*----------------------------------------------------------------------*/
/*	�ǥХå��Ѥ˲��̤�ե�����˥������ (����®�٤��㲼���ޤ�)	*/
/*----------------------------------------------------------------------*/

/*#define PIO_DISP*/		/* PIO �ط��Υ�����̤�ɽ��     */
/*#define PIO_FILE*/		/*		   �ե�����˽��� */

/*#define FDC_DISP*/		/* FDC �ط��Υ�����̤�ɽ��     */
/*#define FDC_FILE*/		/*		   �ե�����˽��� */

/*#define MAIN_DISP*/		/* �ᥤ�� Z80 �ط��Υ�����̤�ɽ�� */
/*#define MAIN_FILE*/		/*		   �ե�����˽���    */

/*#define SUB_DISP*/		/* ���� Z80 �ط��Υ�����̤�ɽ�� */
/*#define SUB_FILE*/		/*		   �ե�����˽���  */


#ifndef	USE_MONITOR
#undef PIO_DISP
#undef PIO_FILE
#undef FDC_DISP
#undef FDC_FILE
#undef MAIN_DISP
#undef MAIN_FILE
#undef SUB_DISP
#undef SUB_FILE
#endif


/* ����ե�����˼����ϡ��ե�����򳫤� */
#if defined(PIO_FILE) || defined(FDC_FILE) || defined(MAIN_FILE) || defined(SUB_FILE)
#include <stdio.h>
extern	FILE	*LOG;
#endif


/* �����ϤΥޥ���							*/
/*	������Ĺ�����δؿ���ƤӽФ��ޥ���Ǥ��롣			*/
/*	���ޥ����Τ�Τ�̵���ˤ���ˤϡ����̤ˤ�			*/
/*		#define  logxxx   (void)				*/
/*		#define  logxxx   if(1){}else printf			*/
/*	�ʤɤ�Ȥ������ԤǤϥ�˥󥰤��Ф��������ΤǸ�Ԥ���Ѥ�����*/


#if	defined( PIO_DISP ) || defined( PIO_FILE )
void	logpio( const char *format, ... );
#else
#define	logpio	if(1){}else printf
#endif

#if	defined( FDC_DISP ) || defined( FDC_FILE )
void	logfdc( const char *format, ... );
#else
#define	logfdc	if(1){}else printf
#endif

#if	defined( MAIN_DISP ) || defined( MAIN_FILE ) || defined( SUB_DISP ) || defined( SUB_FILE )
void	logz80( const char *format, ... );
#else
#define	logz80	if(1){}else printf
#endif



#endif		/* QUASI88_H_INCLUDED */
