/************************************************************************/
/*									*/
/* �����꡼�� ���ʥåץ���å�						*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "screen.h"
#include "screen-func.h"
#include "graph.h"

#include "crtcdmac.h"
#include "memory.h"
#include "file-op.h"
#include "snapshot.h"
#include "initval.h"



char	file_snap[QUASI88_MAX_FILENAME];/* ���ʥåץ���åȥ١�����	*/
int	snapshot_format  = 0;		/* ���ʥåץ���åȥե����ޥå�	*/

char	snapshot_cmd[ SNAPSHOT_CMD_SIZE ];/* ���ʥåץ���åȸ女�ޥ��	*/
char	snapshot_cmd_do  = FALSE;	/* ���ޥ�ɼ¹Ԥ�̵ͭ		*/

#ifdef	USE_SSS_CMD
char	snapshot_cmd_enable = TRUE;	/* ���ޥ�ɼ¹Ԥβ���		*/
#else
char	snapshot_cmd_enable = FALSE;	/* ���ޥ�ɼ¹Ԥβ���		*/
#endif


/* ���ʥåץ���åȤ�����������Хåե��ȡ����Υ���ǥå��� */

char			screen_snapshot[ 640*400 ];
static SYSTEM_PALETTE_T	pal[16 +1];





/***********************************************************************
 * ���ʥåץ���åȥե�����̾�ʤɤ�����
 ************************************************************************/
void	screen_snapshot_init( void )
{
  const char *s;

  if( file_snap[0] == '\0' ){
    set_snapshot_filename( TRUE );
  }


  memset( snapshot_cmd, 0, SNAPSHOT_CMD_SIZE );
  s = getenv( "QUASI88_SSS_CMD" );			/* �¹ԥ��ޥ�� */
  if( s  &&  (strlen(s) < SNAPSHOT_CMD_SIZE) ){
    strcpy( snapshot_cmd, s );
  }

  snapshot_cmd_do = FALSE;	/* ����ͤϡ����ޥ�ɼ¹ԡؤ��ʤ��٤ˤ��� */
}





/*----------------------------------------------------------------------*/
/* ���̥��᡼������������						*/
/*----------------------------------------------------------------------*/

typedef	int		( *SNAPSHOT_FUNC )( void );

static	void	make_snapshot( void )
{
  int vram_mode, text_mode;
  SNAPSHOT_FUNC		(*list)[4][2];


  /* skipline �ξ��ϡ�ͽ�� snapshot_clear() ��ƤӽФ��Ƥ��� */

  if     ( use_interlace == 0 ){ list = snapshot_list_normal; }
  else if( use_interlace >  0 ){ list = snapshot_list_itlace; }
  else                         { snapshot_clear();
				 list = snapshot_list_skipln; }



	/* VRAM/TEXT �����Ƥ� screen_snapshot[] ��ž�� */

  if( sys_ctrl & SYS_CTRL_80 ){
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_80x25; }
    else                     { text_mode = V_80x20; }
  }else{
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_40x25; }
    else                     { text_mode = V_40x20; }
  }

  if( grph_ctrl & GRPH_CTRL_VDISP ){
    if( grph_ctrl & GRPH_CTRL_COLOR ){		/* ���顼 */
        vram_mode = V_COLOR;
    }else{
      if( grph_ctrl & GRPH_CTRL_200 ){		/* ��� */
	vram_mode = V_MONO;
      }else{					/* 400�饤�� */
	vram_mode = V_HIRESO;
      }
    }
  }else{					/* ��ɽ�� */
        vram_mode = V_UNDISP;
  }

  (list[ vram_mode ][ text_mode ][ V_ALL ])();


	/* �ѥ�åȤ����Ƥ� pal[] ��ž�� */

  setup_palette( pal );

  pal[16].red   = 0;		/* pal[16] �Ϲ�����ǻ��Ѥ��� */
  pal[16].green = 0;
  pal[16].blue  = 0;
}






#if 0		/* XPM �ϥ��ݡ����оݳ� */
/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�xpm �����ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_xpm( OSD_FILE *fp )
{
  unsigned char buf[80];
  int i, j, c;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  sprintf( buf,
	   "/* XPM */\n"
	   "static char * quasi88_xpm[] = {\n"
	   "\"640 400 16 1\",\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  for( i=0; i<16; i++ ){
    sprintf( buf, "\"%1X      c #%04X%04X%04X\",\n",
	     i,
	     (unsigned short)pal[i].red   << 8,
	     (unsigned short)pal[i].green << 8,
	     (unsigned short)pal[i].blue  << 8 );
    osd_fwrite( buf, sizeof(char), strlen(buf), fp );
  }


  for( i=0; i<400; i++ ){

    osd_fputc( '\"', fp );

    for( j=0; j<640; j++ ){
      c = *p++;
      if( c < 10 ) c += '0';
      else         c += 'A' - 10;
      osd_fputc( c, fp );
    }

    sprintf( buf, "\",\n" );
    osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  }

  sprintf( buf, "};\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  return 1;
}
#endif



/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�ppm ����(raw)�ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm( OSD_FILE *fp )
{
  unsigned char buf[32];
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  strcpy( (char *)buf, 
	  "P6\n"
	  "# QUASI88\n"
	  "640 400\n"
	  "255\n" );
  osd_fwrite( buf, sizeof(char), strlen((char *)buf), fp );


  for( i=0; i<400; i++ ){
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].red;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].blue;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



#if 0		/* PPM(ascii)  �ϥ��ݡ����оݳ� */
/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�ppm ����(ascii)�ǥե�����˽���		*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm_ascii( OSD_FILE *fp )
{
  unsigned char buf[32];
  int i, j, k;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  strcpy( buf, 
	  "P3\n"
	  "# QUASI88\n"
	  "640 400\n"
	  "255\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  
  for( i=0; i<400; i++ ){
    for( j=0; j<640; j+=5 ){
      for( k=0; k<5; k++ ){
	sprintf( buf, "%3d %3d %3d ",
		 pal[ (int)*p ].red,
		 pal[ (int)*p ].green,
		 pal[ (int)*p ].blue );
	osd_fwrite( buf, sizeof(char), strlen(buf), fp );
	p++;
      }
      osd_fputc( '\n', fp );
    }

  }

  return 1;
}
#endif



/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�bmp ����(win)�ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_bmp( OSD_FILE *fp )
{
  static const char header[] =
  {
    'B', 'M',			/* BM */
    0x36, 0xb8, 0x0b, 0x00,	/* �ե����륵���� 0xbb836 */
    0x00, 0x00,
    0x00, 0x00,
    0x36, 0x00, 0x00, 0x00,	/* �����ǡ������ե��å� 0x36 */

    0x28, 0x00, 0x00, 0x00,	/* ���󥵥��� 0x28 */
    0x80, 0x02, 0x00, 0x00,	/* ��	0x280 */
    0x90, 0x01, 0x00, 0x00,	/* �⤵	0x190 */
    0x01, 0x00,
    0x18, 0x00,			/* ������ */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	/* ����������?	0xbb800 */
    0x00, 0x00, 0x00, 0x00,	/* ������������?	*/
    0x00, 0x00, 0x00, 0x00,	/* ������������?	*/
    0x00, 0x00, 0x00, 0x00,	/* ���ѥѥ�åȿ�	*/
    0x00, 0x00, 0x00, 0x00,	/* ����?		*/
  };

  unsigned char buf[4];
  int i, j;
  char *p;

  if( fp==NULL ) return 0;

  osd_fwrite( header, sizeof(char), sizeof(header), fp );


  for( i=0; i<400; i++ ){
    p = screen_snapshot + (399-i)*640;
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].blue;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].red;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�raw�����ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_raw( OSD_FILE *fp )
{
  unsigned char buf[4];
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  for( i=0; i<400; i++ ){
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].red;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].blue;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



/************************************************************************/
/* ���̤Υ��ʥåץ���åȤ򥻡��֤���					*/
/*	��������VRAM���Ȥ˥��ʥåץ���åȤ�������롣		*/
/*	����ɽ������Ƥ�����̤򥻡��֤���櫓�ǤϤʤ���		*/
/*									*/
/*	�Ķ��ѿ� ${QUASI88_SSS_CMD} ���������Ƥ����硢�����ָ��	*/
/*	�������Ƥ򥳥ޥ�ɤȤ��Ƽ¹Ԥ��롣���κݡ�%a ���ե�����̾�ˡ�	*/
/*	%b ���ե�����̾���饵�ե��å�������������Τˡ��֤�����롣	*/
/*									*/
/*	��) setenv QUASI88_SSS_CMD 'ppmtopng %a > %b.png'		*/
/*									*/
/************************************************************************/


int	save_screen_snapshot( void )
{
  static	int snapshot_no = 0;		/* Ϣ�� */

  static const char *suffix[] = { ".bmp", ".ppm", ".raw", };
  static const char *checksuffix[] = { ".ppm",  ".PPM", 
				       ".xpm",  ".XPM", 
				       ".png",  ".PNG", 
				       ".bmp",  ".BMP", 
				       ".rgb",  ".RGB", 
				       ".raw",  ".RAW", 
				       ".gif",  ".GIF", 
				       ".xwd",  ".XWD", 
				       ".pict", ".PICT",
				       ".tiff", ".TIFF",
				       ".tif",  ".TIF", 
				       ".jpeg", ".JPEG",
				       ".jpg",  ".JPG", 
				       NULL
  };

  OSD_FILE *fp;

  char  *filename;
  char  number[8];
  int   i, j, len, success;

  if( snapshot_format >= COUNTOF(suffix) ) return FALSE;
  if( file_snap[0] == '\0' ) return FALSE;


	/* �ե�����̾�ΥХåե������ */


  filename = (char*)malloc( strlen(file_snap) + sizeof( "NNNN.suffix" ) );
  if( filename==NULL ){
    printf( "screen-snapshot : malloc failed\n" );
    return FALSE;
  }


	/* �ե�����̾����ü�� NNNN.suffix �ʤ��� */

  for( i=0; checksuffix[i]; i++ ){

    if( strlen(file_snap) > strlen(checksuffix[i]) + 4 ){

      char *p = &file_snap[ strlen(file_snap) - strlen(checksuffix[i]) - 4 ];
      if( isdigit( *(p+0) ) &&
	  isdigit( *(p+1) ) &&
	  isdigit( *(p+2) ) &&
	  isdigit( *(p+3) ) &&
	  strcmp( p+4, checksuffix[i] ) == 0 ){

	*p = '\0';

	printf( "screen-snapshot : filename truncated (%s)\n", file_snap );
	break;
      }
    }
  }


	/* ¸�ߤ��ʤ��ե�����̾��õ������ (0000.suffix�� 9999.suffix) */

  success = FALSE;
  for( j=0; j<10000; j++ ){

    sprintf( number, "%04d", snapshot_no );
    if( ++ snapshot_no > 9999 ) snapshot_no = 0;
    strcpy( filename, file_snap );
    strcat( filename, number );
    len = strlen( filename );

    for( i=0; checksuffix[i]; i++ ){
      filename[ len ] = '\0';
      strcat( filename, checksuffix[ i ] );
      if( osd_file_stat( filename ) != FILE_STAT_NOEXIST ) break;
    }
    if( checksuffix[i] == NULL ){	    /* ���Ĥ��ä� */
      filename[ len ] = '\0';
      strcat( filename, suffix[ snapshot_format ] );
      success = TRUE;
      break;
    }
  }


	/* �ե�����򳫤��ơ����ʥåץ���åȥǡ�����񤭹��� */
  if( success ){

    success = FALSE;
    if( (fp = osd_fopen( FTYPE_SNAPSHOT_PPM, filename, "wb" ) ) ){

      make_snapshot();

      switch( snapshot_format ){
      case 0:	success = save_snapshot_bmp( fp );		break;
      case 1:	success = save_snapshot_ppm( fp );		break;
      case 2:	success = save_snapshot_raw( fp );		break;
      }

      osd_fclose( fp );
    }
/*
    printf( "screen-snapshot : %s ... %s\n", 
				filename, (success) ? "OK" : "FAILED" );
*/

#ifdef	USE_SSS_CMD

	/* �񤭹��������塢���ޥ�ɤ�¹Ԥ��� */

    if( success ){
      if( snapshot_cmd_enable &&
	  snapshot_cmd_do     &&
	  snapshot_cmd[0]     ){

	int	a_len, b_len;
	char	*cmd, *s, *d;

	a_len = strlen( filename );
	b_len = a_len - 4;		/* ���ե��å��� ".???" ��4ʸ��ʬ���� */

	len = 0;
	s = snapshot_cmd;
	while( *s ){
	  if( *s == '%' ){
	    switch( *(s+1) ){
	    case '%': len ++;		s++;	break; 
	    case 'a': len += a_len;	s++;	break; 
	    case 'b': len += b_len;	s++;	break; 
	    default:  len ++;			break; 
	    }
	  }else       len ++;

	  s++;
	}

	cmd = (char *)malloc( len + 1 );
	if( cmd ){

	  s = snapshot_cmd;
	  d = cmd;
	  while( *s ){
	    if( *s == '%' ){
	      switch( *(s+1) ){
	      case '%': *d++ = *s;				s++;	break; 
	      case 'a': memcpy( d, filename, a_len ); d+=a_len;	s++;	break; 
	      case 'b': memcpy( d, filename, b_len ); d+=b_len;	s++;	break;
	      default:  *d++ = *s;
	      }
	    }else       *d++ = *s;
	    s++;
	  }
	  *d = '\0';

	  printf( "[SNAPSHOT command]%% %s\n", cmd );
	  system( cmd );

	  free(cmd);
	}
      }
    }
#endif	/* USE_SSS_CMD */

  }

  free( filename );

  return success;
}
