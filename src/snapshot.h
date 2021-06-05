#ifndef SNAPSHOT_H_INCLUDED
#define SNAPSHOT_H_INCLUDED

#include <stdio.h>
#include "file-op.h"

extern	int	set_snapshot_filename( int init );


#if 0	/* �� file-op.h */
extern char file_snap[QUASI88_MAX_FILENAME];	/* ���ʥåץ���åȥ١����� */
#endif



enum {
  SNAPSHOT_FMT_BMP,
  SNAPSHOT_FMT_PPM,
  SNAPSHOT_FMT_RAW
};
extern	int	snapshot_format;	/* ���ʥåץ���åȥե����ޥå�	*/


					/* ���ʥåץ���åȥ��ޥ��	*/
#define	SNAPSHOT_CMD_SIZE	(1024)
extern	char	snapshot_cmd[ SNAPSHOT_CMD_SIZE ];
extern	char	snapshot_cmd_do;	/* ���ޥ�ɼ¹Ԥ�̵ͭ		*/

extern	char	snapshot_cmd_enable;	/* ���ޥ�ɼ¹Ԥβ���		*/


void	screen_snapshot_init( void );
int	save_screen_snapshot( void );


#endif	/* SNAPSHOT_H_INCLUDED */
