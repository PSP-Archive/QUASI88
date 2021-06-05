/************************************************************************/
/*									*/
/* ��ưľ��ΰ����ν����ȡ�����ν����				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "getconf.h"
#include "device.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "screen.h"
#include "soundbd.h"
#include "fdc.h"

#include "emu.h"
#include "file-op.h"
#include "drive.h"
#include "menu.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "snapshot.h"
#include "suspend.h"


/*----------------------------------------------------------------------*/

/* ����ե�������Ρ�����������ʸ�� */
#define	CONFIG_ESC		'\\'

/*----------------------------------------------------------------------*/

/* -f6 .. -f10 ���ץ����ΰ����ȡ���ǽ���б����� */

static const struct {
  int	num;
  char	*str;
} fn_index[] =
{
  { FN_FUNC,        NULL,	   },
  { FN_FRATE_UP,    "FRATE-UP",    },
  { FN_FRATE_DOWN,  "FRATE-DOWN",  },
  { FN_VOLUME_UP,   "VOLUME-UP",   },
  { FN_VOLUME_DOWN, "VOLUME-DOWN", },
  { FN_PAUSE,       "PAUSE",       },
  { FN_RESIZE,      "RESIZE",      },
  { FN_NOWAIT,      "NOWAIT",      },
  { FN_SPEED_UP,    "SPEED-UP",    },
  { FN_SPEED_DOWN,  "SPEED-DOWN",  },
  { FN_MOUSE_HIDE,  "MOUSE-HIDE",  },
  { FN_FULLSCREEN,  "FULLSCREEN",  },
  { FN_FULLSCREEN,  "DGA",         },	/* �ߴ��Τ��� */
  { FN_SNAPSHOT,    "SNAPSHOT",    },
  { FN_IMAGE_NEXT1, "IMAGE-NEXT1", },
  { FN_IMAGE_PREV1, "IMAGE-PREV1", },
  { FN_IMAGE_NEXT2, "IMAGE-NEXT2", },
  { FN_IMAGE_PREV2, "IMAGE-PREV2", },
  { FN_NUMLOCK,     "NUMLOCK",     },
  { FN_RESET,       "RESET",       },
  { FN_KANA,        "KANA",        },
  { FN_ROMAJI,      "ROMAJI",      },
  { FN_CAPS,        "CAPS",        },
  { FN_KETTEI,      "KETTEI",      },
  { FN_HENKAN,      "HENKAN",      },
  { FN_ZENKAKU,     "ZENKAKU",     },
  { FN_PC,          "PC",          },
  { FN_STOP,        "STOP",        },
  { FN_COPY,        "COPY",        },
  { FN_STATUS,      "STATUS",      },
  { FN_MENU,        "MENU",        },
};


/*----------------------------------------------------------------------*/

/* �إ��ɽ�� */
static	char	*command = NULL;

#include "help.h"


/*----------------------------------------------------------------------*/

/* ���ץ����򤤤��Ĥ��Υ��롼�פ�ʬ���ơ�ͥ���٤����ꤹ�롣*/
static	signed char	opt_prioroty[ 256 ];

/*----------------------------------------------------------------------*/

static	char	*boot_file[2];
static	int	boot_image[2];

static	int	load_config = TRUE;

/*----------------------------------------------------------------------*/

static	int	tmp_arg;

static int o_baudrate( char *dummy )
{
  static const int table[] = {
    75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
  };
  int i;
  for( i=0; i<COUNTOF(table); i++ ){
    if( tmp_arg == table[i] ){
      baudrate_sw = i;
      return 0;
    }
  }
  return 1;
}

static int o_4mhz( char *dummy ){ cpu_clock_mhz = CONST_4MHZ_CLOCK; return 0; }
static int o_8mhz( char *dummy ){ cpu_clock_mhz = CONST_8MHZ_CLOCK; return 0; }

static int o_width ( char *dummy ){ WIDTH  &= ~7; return 0; }
static int o_height( char *dummy ){ HEIGHT &= ~1; return 0; }

static int o_set_version( char *dummy ){ set_version += '0'; return 0; }

static int o_menukey( char *dummy )
{
  function_f[  6 ] = FN_FRATE_UP;
  function_f[  7 ] = FN_FRATE_DOWN;
  function_f[  8 ] = FN_VOLUME_UP;
  function_f[  9 ] = FN_VOLUME_DOWN;
  function_f[ 10 ] = FN_PAUSE;
  return 0;
}
static int o_kanjikey( char *dummy )
{
  function_f[  6 ] = FN_KANA;
  function_f[  7 ] = FN_KETTEI;
  function_f[  8 ] = FN_HENKAN;
  function_f[  9 ] = FN_ZENKAKU;
  function_f[ 10 ] = FN_ROMAJI;
  return 0;
}
static int o_joyassign( char *dummy )
{
  switch( tmp_arg ){
  case 0:	joy_key_assign[0] = KEY88_INVALID;
		joy_key_assign[1] = KEY88_INVALID;
		joy_key_assign[2] = KEY88_INVALID;
		joy_key_assign[3] = KEY88_INVALID;
		joy_key_assign[4] = KEY88_INVALID;
		joy_key_assign[5] = KEY88_INVALID;
		joy_key_mode = 0;
		return 0;

  case 1:	joy_key_assign[4] = KEY88_x;
		joy_key_assign[5] = KEY88_z;		break;
  case 2:	joy_key_assign[4] = KEY88_SPACE;
		joy_key_assign[5] = KEY88_RETURN;	break;
  case 3:	joy_key_assign[4] = KEY88_SPACE;
		joy_key_assign[5] = KEY88_SHIFT;	break;
  case 4:	joy_key_assign[4] = KEY88_SHIFT;
		joy_key_assign[5] = KEY88_z;		break;
  default:	joy_key_assign[4] = KEY88_INVALID;
		joy_key_assign[5] = KEY88_INVALID;	break;
  }
  joy_key_assign[0] = KEY88_KP_8;
  joy_key_assign[1] = KEY88_KP_2;
  joy_key_assign[2] = KEY88_KP_4;
  joy_key_assign[3] = KEY88_KP_6;
  joy_key_mode = 2;

  return 0;
}
static int o_help( char *dummy )
{
  help_mess();
  xmame_show_option();
  exit(0);
  return 0;
}

static int o_menu   ( char *dummy ){ set_emu_mode( MENU );    return 0; }
static int o_monitor( char *dummy ){ set_emu_mode( MONITOR ); return 0; }

static int o_resume( char *dummy )
{
  resume_flag  = TRUE; 
  resume_force = FALSE;
  resume_file  = FALSE;
  strcpy( file_state, "" );
  return 0;
}
static int o_resumefile( char *filename )
{
  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", filename );
    resume_flag  = FALSE;
    resume_force = FALSE;
    resume_file  = FALSE;
  }else{
    resume_flag  = TRUE;
    resume_force = FALSE;
    resume_file  = TRUE;
    strcpy( file_state, filename );
  }
  return 0;
}
static int o_resumeforce( char *filename )
{
  memset( file_state, 0, QUASI88_MAX_FILENAME );

  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", filename );
    resume_flag  = FALSE;
    resume_force = FALSE;
    resume_file  = FALSE;
  }else{
    resume_flag  = TRUE;
    resume_force = TRUE;
    resume_file  = TRUE;
    strcpy( file_state, filename );
  }
  return 0;
}

static int o_romdir( char *dir )
{
  if( osd_set_dir_rom( dir ) == FALSE ){
    fprintf( stderr, "-romdir %s failed, ignored\n", dir );
  }
  return 0;
}
static int o_diskdir( char *dir )
{
  if( osd_set_dir_disk( dir ) == FALSE ){
    fprintf( stderr, "-diskdir %s failed, ignored\n", dir );
  }
  return 0;
}
static int o_tapedir( char *dir )
{
  if( osd_set_dir_tape( dir ) == FALSE ){
    fprintf( stderr, "-tapedir %s failed, ignored\n", dir );
  }
  return 0;
}
static int o_snapdir( char *dir )
{
  if( osd_set_dir_snap( dir ) == FALSE ){
    fprintf( stderr, "-snapdir %s failed, ignored\n", dir );
  }
  return 0;
}
static int o_statedir( char *dir )
{
  if( osd_set_dir_state( dir ) == FALSE ){
    fprintf( stderr, "-statedir %s failed, ignored\n", dir );
  }
  return 0;
}

static int o_tapeload( char *filename )
{
  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", filename );
  }else{
    strncpy( file_tape[CLOAD], filename, QUASI88_MAX_FILENAME );
  }
  return 0;
}
static int o_tapesave( char *filename )
{
  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", filename );
  }else{
    strncpy( file_tape[CSAVE], filename, QUASI88_MAX_FILENAME );
  }
  return 0;
}

static int o_printer( char *filename )
{
  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", filename );
  }else{
    strncpy( file_prn, filename, QUASI88_MAX_FILENAME );
  }
  return 0;
}

static int o_serialin( char *filename )
{
  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", filename );
  }else{
    strncpy( file_sin, filename, QUASI88_MAX_FILENAME );
  }
  return 0;
}
static int o_serialout( char *filename )
{
  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", filename );
  }else{
    strncpy( file_sout, filename, QUASI88_MAX_FILENAME );
  }
  return 0;
}

static	char	*tmp_boot_file;

static int o_diskimage( char *dummy )
{
  boot_file[0]  = boot_file[1]  = NULL;
  boot_image[0] = boot_image[1] = 0;

  if( strlen(tmp_boot_file) >= QUASI88_MAX_FILENAME ){
    fprintf( stderr, "filename %s too long, ignored\n", tmp_boot_file );
  }else{
    boot_file[0]  = tmp_boot_file;
  }
  return 0;
}

/*----------------------------------------------------------------------*/

static const struct {
  int	group;		/* ʬ�� (���礹�륪�ץ����ˤ�Ʊ���ͤ��꿶��)*/
  char	*name;		/* ���ץ�����̾�� (�ϥ��ե�ʤ�)		*/
  enum {		/* ���ץ�����³���ѥ�᡼���ν����ϡ�����	*/
    X_FIX,			/* ���:   *var = (int)val1 [���]	*/
    X_INT,			/* int:    *var = argv [�ϰ� var1��val2]*/
    X_DBL,			/* double: *var = argv [�ϰ� var1��val2]*/
    X_STR,			/* ʸ����: strcpy( var, argv );		*/
    X_FN,			/* FN��ǽ: *var = ��ǽ			*/
    X_NOP,			/* ����:   �ѥ�᡼�������ʤ�		*/
    X_INV			/* ̵��:   �ѥ�᡼���������ؿ���̵��	*/
  }	type;
  void	*var;		/* �����Ǽ����ѿ����ͤ򥻥åȤ���		*/
  float	val1;		/* �����ǽ�ʺǾ��� �ޤ��� ���åȤ������	*/
  float	val2;		/* �����ǽ�ʺ�����				*/

  int	(*func)( char *argv );	/* �����κǸ�˸ƤӽФ��ؿ�		*/
				/* �۾ｪλ���� 0 �Ǥʤ��ͤ��֤�	*/
}
option_table[] =
{
  /*  1��30 : PC-8801���ꥪ�ץ���� */

  {   1, "n",            X_FIX,  &boot_basic,      BASIC_N  ,             0,0},
  {   1, "v1s",          X_FIX,  &boot_basic,      BASIC_V1S,             0,0},
  {   1, "v1h",          X_FIX,  &boot_basic,      BASIC_V1H,             0,0},
  {   1, "v2",           X_FIX,  &boot_basic,      BASIC_V2 ,             0,0},
  {   2, "4mhz",         X_FIX,  &boot_clock_4mhz, TRUE , 0, o_4mhz,         },
  {   2, "8mhz",         X_FIX,  &boot_clock_4mhz, FALSE, 0, o_8mhz,         },
  {   3, "sd",           X_FIX,  &sound_board,     SOUND_I ,              0,0},
  {   3, "sd2",          X_FIX,  &sound_board,     SOUND_II,              0,0},
  {   4, "mouse",        X_FIX,  &mouse_mode,      1,                     0,0},
  {   4, "nomouse",      X_FIX,  &mouse_mode,      0,                     0,0},
  {   4, "joymouse",     X_FIX,  &mouse_mode,      2,                     0,0},
  {   4, "joystick",     X_FIX,  &mouse_mode,      3,                     0,0},
  {   0, "joykey",       X_INV,                                       0,0,0,0},
  {   5, "pcg",          X_FIX,  &use_pcg,         TRUE ,                 0,0},
  {   5, "nopcg",        X_FIX,  &use_pcg,         FALSE,                 0,0},
  {   6, "dipsw",        X_INT,  &boot_dipsw,      0x0000, 0xffff,          0},
  {   7, "baudrate",     X_INT,  &tmp_arg,         75, 19200, o_baudrate,    },
  {   8, "romboot",      X_FIX,  &boot_from_rom,   TRUE ,                 0,0},
  {   8, "diskboot",     X_FIX,  &boot_from_rom,   FALSE,                 0,0},
  {   9, "extram",       X_INT,  &use_extram,      0, 16,                   0},
  {   9, "noextram",     X_FIX,  &use_extram,      0,                     0,0},
  {  10, "jisho",        X_FIX,  &use_jisho_rom,   TRUE ,                 0,0},
  {  10, "nojisho",      X_FIX,  &use_jisho_rom,   FALSE,                 0,0},
  {  11, "analog",       X_FIX,  &monitor_analog,  TRUE ,                 0,0},
  {  11, "digital",      X_FIX,  &monitor_analog,  FALSE,                 0,0},
  {  12, "24k",          X_FIX,  &monitor_15k,     0x00,                  0,0},
  {  12, "15k",          X_FIX,  &monitor_15k,     0x02,                  0,0},
  {  13, "tapeload",     X_NOP,  &tmp_arg,         0, 0, o_tapeload,         },
  {  14, "tapesave",     X_NOP,  &tmp_arg,         0, 0, o_tapesave,         },
  {  15, "printer",      X_NOP,  &tmp_arg,         0, 0, o_printer           },
  {  16, "serialout",    X_NOP,  &tmp_arg,         0, 0, o_serialout,        },
  {  17, "serialin",     X_NOP,  &tmp_arg,         0, 0, o_serialin,         },
  {  18, "ro",           X_FIX,  &menu_readonly,   TRUE ,                 0,0},
  {  18, "rw",           X_FIX,  &menu_readonly,   FALSE,                 0,0},

  /*  31��60 : ���ߥ�졼��������ꥪ�ץ���� */

  {  31, "cpu",          X_INT,  &cpu_timing,      0, 2,                    0},
  {  32, "cpu1count",    X_INT,  &CPU_1_COUNT,     1, 65536,                0},
  {  33, "cpu2us",       X_INT,  &cpu_slice_us,    1, 1000,                 0},
  {  34, "fdc_wait",     X_FIX,  &fdc_wait,        1,                     0,0},
  {  34, "fdc_nowait",   X_FIX,  &fdc_wait,        0,                     0,0},
  {  35, "clock",        X_DBL,  &cpu_clock_mhz,   0.001, 65536.0,          0},
  {   0, "waitfreq",     X_INV,  &tmp_arg,                              0,0,0},
  {  36, "speed",        X_INT,  &wait_rate,       5, 5000,                 0},
  {  37, "nowait",       X_FIX,  &no_wait,         TRUE,                  0,0},
  {  37, "wait",         X_FIX,  &no_wait,         FALSE,                 0,0},
  {  38, "cmt_intr",     X_FIX,  &cmt_intr,        TRUE ,                 0,0},
  {  38, "cmt_poll",     X_FIX,  &cmt_intr,        FALSE,                 0,0},
  {  39, "cmt_speed",    X_INT,  &cmt_speed,       0, 0xffff,               0},
  {  40, "cmt_wait",     X_FIX,  &cmt_wait,        TRUE ,                 0,0},
  {  40, "cmt_nowait",   X_FIX,  &cmt_wait,        FALSE,                 0,0},
  {  41, "hsbasic",      X_FIX,  &highspeed_mode,  TRUE ,                 0,0},
  {  41, "nohsbasic",    X_FIX,  &highspeed_mode,  FALSE,                 0,0},
  {  42, "mem_wait",     X_FIX,  &memory_wait,     TRUE ,                 0,0},
  {  42, "mem_nowait",   X_FIX,  &memory_wait,     FALSE,                 0,0},
  {  43, "setver",       X_INT,  &set_version,     0, 9, o_set_version,      },
  {  44, "exchange",     X_FIX,  &disk_exchange,   TRUE ,                 0,0},
  {  44, "noexchange",   X_FIX,  &disk_exchange,   FALSE,                 0,0},

  /*  61��90 : ����ɽ�����ꥪ�ץ���� */

  {  61, "frameskip",    X_INT,  &frameskip_rate,  1, 65536,                0},
  {  62, "autoskip",     X_FIX,  &use_auto_skip,   TRUE ,                 0,0},
  {  62, "noautoskip",   X_FIX,  &use_auto_skip,   FALSE,                 0,0},
  {  63, "half",         X_FIX,  &screen_size,     SCREEN_SIZE_HALF  ,    0,0},
  {  63, "full",         X_FIX,  &screen_size,     SCREEN_SIZE_FULL  ,    0,0},
#ifdef	SUPPORT_DOUBLE
  {  63, "double",       X_FIX,  &screen_size,     SCREEN_SIZE_DOUBLE,    0,0},
#else
  {  63, "double",       X_INV,                                       0,0,0,0},
#endif
  {  64, "width",        X_INT,  &WIDTH,           1, 65536, o_width ,       },
  {  65, "height",       X_INT,  &HEIGHT,          1, 65536, o_height,       },
  {  66, "interp",       X_FIX,  &use_half_interp, TRUE ,                 0,0},
  {  66, "nointerp",     X_FIX,  &use_half_interp, FALSE,                 0,0},
  {  67, "skipline",     X_FIX,  &use_interlace,   -1,                    0,0},
  {  67, "noskipline",   X_FIX,  &use_interlace,   0,                     0,0},
  {  67, "interlace",    X_FIX,  &use_interlace,   1,                     0,0},
  {  67, "nointerlace",  X_FIX,  &use_interlace,   0,                     0,0},
  {  68, "dga",          X_FIX,  &use_fullscreen,  TRUE ,                 0,0},
  {  68, "fullscreen",   X_FIX,  &use_fullscreen,  TRUE ,                 0,0},
  {  68, "nodga",        X_FIX,  &use_fullscreen,  FALSE,                 0,0},
  {  68, "window",       X_FIX,  &use_fullscreen,  FALSE,                 0,0},
  {  69, "hide_mouse",   X_FIX,  &hide_mouse,      TRUE ,                 0,0},
  {  69, "show_mouse",   X_FIX,  &hide_mouse,      FALSE,                 0,0},
  {  70, "grab_mouse",   X_FIX,  &grab_mouse,      TRUE ,                 0,0},
  {  70, "ungrab_mouse", X_FIX,  &grab_mouse,      FALSE,                 0,0},
  {  71, "status",       X_FIX,  &show_status,     TRUE ,                 0,0},
  {  71, "nostatus",     X_FIX,  &show_status,     FALSE,                 0,0},
  {  72, "status_fg",    X_INT,  &status_fg,       0, 0xffffff,             0},
  {  73, "status_bg",    X_INT,  &status_bg,       0, 0xffffff,             0},

  /*  91��120: �������ꥪ�ץ���� */

  {  91, "tenkey",       X_FIX,  &tenkey_emu,      TRUE ,                 0,0},
  {  91, "notenkey",     X_FIX,  &tenkey_emu,      FALSE,                 0,0},
  {  92, "cursor",       X_FIX,  &cursor_key_mode, 1,                     0,0},
  {  92, "nocursor",     X_FIX,  &cursor_key_mode, 0,                     0,0},
  {  93, "numlock",      X_FIX,  &numlock_emu,     TRUE ,                 0,0},
  {  93, "nonumlock",    X_FIX,  &numlock_emu,     FALSE,                 0,0},
  {  94, "f1",           X_FN,   &function_f[ 1],                       0,0,0},
  {  95, "f2",           X_FN,   &function_f[ 2],                       0,0,0},
  {  96, "f3",           X_FN,   &function_f[ 3],                       0,0,0},
  {  97, "f4",           X_FN,   &function_f[ 4],                       0,0,0},
  {  98, "f5",           X_FN,   &function_f[ 5],                       0,0,0},
  {  99, "f6",           X_FN,   &function_f[ 6],                       0,0,0},
  { 100, "f7",           X_FN,   &function_f[ 7],                       0,0,0},
  { 101, "f8",           X_FN,   &function_f[ 8],                       0,0,0},
  { 102, "f9",           X_FN,   &function_f[ 9],                       0,0,0},
  { 103, "f10",          X_FN,   &function_f[10],                       0,0,0},
  { 104, "f11",          X_FN,   &function_f[11],                       0,0,0},
  { 105, "f12",          X_FN,   &function_f[12],                       0,0,0},
  { 106, "romaji",       X_INT,  &romaji_type,     0, 2,                    0},
  { 107, "menukey",      X_NOP,  0,                0, 0, o_menukey ,         },
  { 108, "kanjikey",     X_NOP,  0,                0, 0, o_kanjikey,         },
  { 109, "joyswap",      X_FIX,  &joy_swap_button, TRUE,                  0,0},
  { 110, "joyassign",    X_INT,  &tmp_arg,         0, 4, o_joyassign,        },

  /* 121��140: ��˥塼���ꥪ�ץ���� */

  { 121, "menu",         X_NOP,  0,                0, 0, o_menu,             },
  { 122, "english",      X_FIX,  &menu_lang,       LANG_ENGLISH,          0,0},
  { 122, "japanese",     X_FIX,  &menu_lang,       LANG_JAPAN  ,          0,0},
  { 123, "sjis",         X_FIX,  &file_coding,     1,                     0,0},
  { 123, "euc",          X_FIX,  &file_coding,     0,                     0,0},
  { 124, "bmp",          X_FIX,  &snapshot_format, SNAPSHOT_FMT_BMP,      0,0},
  { 124, "ppm",          X_FIX,  &snapshot_format, SNAPSHOT_FMT_PPM,      0,0},
  { 124, "raw",          X_FIX,  &snapshot_format, SNAPSHOT_FMT_RAW,      0,0},
  { 125, "swapdrv",      X_FIX,  &menu_swapdrv,    TRUE,                  0,0},
  { 125, "noswapdrv"  ,  X_FIX,  &menu_swapdrv,    FALSE,                 0,0},
  {   0, "button2menu",  X_INV,                                       0,0,0,0},
  {   0, "nobutton2menu",X_INV,                                       0,0,0,0},

  /* 141��170: �����ƥ����ꥪ�ץ���� */

  { 141, "romdir",       X_NOP,  &tmp_arg,         0, 0, o_romdir,           },
  { 142, "diskdir",      X_NOP,  &tmp_arg,         0, 0, o_diskdir,          },
  { 143, "tapedir",      X_NOP,  &tmp_arg,         0, 0, o_tapedir,          },
  { 144, "snapdir",      X_NOP,  &tmp_arg,         0, 0, o_snapdir,          },
  { 145, "statedir",     X_NOP,  &tmp_arg,         0, 0, o_statedir,         },
  { 146, "noconfig",     X_FIX,  &load_config,     FALSE,                 0,0},
  { 147, "compatrom",    X_STR,  &file_compatrom,                       0,0,0},
  { 148, "resume",       X_NOP,  0,                0, 0, o_resume,           },
  { 149, "resumefile",   X_NOP,  &tmp_arg,         0, 0, o_resumefile,       },
  { 150, "resumeforce",  X_NOP,  &tmp_arg,         0, 0, o_resumeforce,      },
  { 151, "record",       X_STR,  &file_rec,                             0,0,0},
  { 152, "playback",     X_STR,  &file_pb,                              0,0,0},
  { 153, "use_joy",      X_FIX,  &use_joydevice,   TRUE ,                 0,0},
  { 153, "nouse_joy",    X_FIX,  &use_joydevice,   FALSE,                 0,0},
  { 154, "focus",        X_FIX,  &need_focus,      TRUE ,                 0,0},
  { 154, "nofocus",      X_FIX,  &need_focus,      FALSE,                 0,0},
  { 155, "sleep",        X_FIX,  &wait_by_sleep,   TRUE ,                 0,0},
  { 155, "nosleep",      X_FIX,  &wait_by_sleep,   FALSE,                 0,0},
  { 156, "sleepparm",    X_INT,  &wait_sleep_min_us, 0, 1000000,            0},
  {   0, "logo",         X_INV,                                       0,0,0,0},
  {   0, "nologo",       X_INV,                                       0,0,0,0},

  /* 171��200: �ǥХå��ѥ��ץ���� */

  { 171, "help",         X_NOP,  0,                0, 0, o_help,             },
  { 172, "verbose",      X_INT,  &verbose_level,   0x00, 0xff,              0},
  { 173, "timestop",     X_FIX,  &calendar_stop,   TRUE,                  0,0},
  { 174, "vsync",        X_DBL,  &vsync_freq_hz,   10.0, 240.0,             0},
  { 175, "soundclock",   X_DBL,  &sound_clock_mhz, 0.001, 65536.0,          0},
  { 177, "subload",      X_INT,  &sub_load_rate,   0, 65536,                0},
  { 178, "nofont",       X_FIX,  &use_built_in_font,TRUE,                 0,0},
  { 178, "font",         X_FIX,  &use_built_in_font,FALSE,                0,0},
  {   0, "load",         X_INV,  &tmp_arg,                              0,0,0},
  { 179, "diskimage",    X_STR,  &tmp_boot_file,   0, 0, o_diskimage,        },

#ifdef  USE_MONITOR
  { 197, "debug",        X_FIX,  &debug_mode,      TRUE,                  0,0},
  { 198, "monitor",      X_FIX,  &debug_mode,      TRUE, 0, o_monitor,       },
  { 199, "fdcdebug",     X_FIX,  &fdc_debug_mode,  TRUE ,                 0,0},
#else
  {   0, "debug",        X_INV,                                       0,0,0,0},
  {   0, "monitor",      X_INV,                                       0,0,0,0},
  {   0, "fdcdebug",     X_INV,                                       0,0,0,0},
#endif

  /* 201��255: �����ƥ��¸���ץ���� */

  {   0, "cmap",         X_INV,  &tmp_arg,                              0,0,0},
  {   0, "shm",          X_INV,                                       0,0,0,0},
  {   0, "noshm",        X_INV,                                       0,0,0,0},
  {   0, "xdnd",         X_INV,                                       0,0,0,0},
  {   0, "noxdnd",       X_INV,                                       0,0,0,0},

  { 201, "hwsurface",    X_FIX,  &use_hwsurface,   TRUE,                  0,0},
  { 201, "swsurface",    X_FIX,  &use_hwsurface,   FALSE,                 0,0},
  { 202, "doublebuf",    X_FIX,  &use_doublebuf,   TRUE,                  0,0},
  { 202, "nodoublebuf",  X_FIX,  &use_doublebuf,   FALSE,                 0,0},
  { 203, "menucursor",   X_FIX,  &use_swcursor,    TRUE,                  0,0},
  { 203, "nomenucursor", X_FIX,  &use_swcursor,    FALSE,                 0,0},

  {   0, "streamspace",  X_INV,  &tmp_arg,                              0,0,0},
  {   0, "ss",           X_INV,  &tmp_arg,                              0,0,0},



#if     ( USE_SOUND != 1 )              /* ������ɤʤ����Υ��ץ���� (̵��) */

  {   0, "sound",        X_INV,                                       0,0,0,0},
  {   0, "nosound",      X_INV,                                       0,0,0,0},
  {   0, "snd",          X_INV,                                       0,0,0,0},
  {   0, "nosnd",        X_INV,                                       0,0,0,0},
  {   0, "samples",      X_INV,                                       0,0,0,0},
  {   0, "nosamples",    X_INV,                                       0,0,0,0},
  {   0, "sam",          X_INV,                                       0,0,0,0},
  {   0, "nosam",        X_INV,                                       0,0,0,0},
  {   0, "fakesound",    X_INV,  &tmp_arg,                              0,0,0},
  {   0, "fsnd",         X_INV,  &tmp_arg,                              0,0,0},
  {   0, "samplefreq",   X_INV,  &tmp_arg,                              0,0,0},
  {   0, "sf",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "bufsize",      X_INV,  &tmp_arg,                              0,0,0},
  {   0, "bs",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "volume",       X_INV,  &tmp_arg,                              0,0,0},
  {   0, "v",            X_INV,  &tmp_arg,                              0,0,0},
  {   0, "audiodevice",  X_INV,  &tmp_arg,                              0,0,0},
  {   0, "ad",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "mixerdevice",  X_INV,  &tmp_arg,                              0,0,0},
  {   0, "md",           X_INV,  &tmp_arg,                              0,0,0},

  {   0, "fmvol",        X_INV,  &tmp_arg,                              0,0,0},
  {   0, "fv",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "psgvol",       X_INV,  &tmp_arg,                              0,0,0},
  {   0, "pv",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "beepvol",      X_INV,  &tmp_arg,                              0,0,0},
  {   0, "bv",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "rhythmvol",    X_INV,  &tmp_arg,                              0,0,0},
  {   0, "rv",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "adpcmvol",     X_INV,  &tmp_arg,                              0,0,0},
  {   0, "av",           X_INV,  &tmp_arg,                              0,0,0},
  {   0, "sdlbufsize",   X_INV,  &tmp_arg,                              0,0,0},
  {   0, "sdlbufnum",    X_INV,  &tmp_arg,                              0,0,0},
  {   0, "close",        X_INV,                                       0,0,0,0},
  {   0, "noclose",      X_INV,                                       0,0,0,0},
  {   0, "fmgen",        X_INV,                                       0,0,0,0},
  {   0, "nofmgen",      X_INV,                                       0,0,0,0},
  {   0, "streamspace",  X_INV,  &tmp_arg,                              0,0,0},
  {   0, "ss",           X_INV,  &tmp_arg,                              0,0,0},
				/* �ۤ��ˤ⤢�뤱�ɡ�����ʾ���ϤǤ��ʤ��� */
#else
#if	( USE_FMGEN != 1 )              /* FMGEN �ʤ����Υ��ץ���� (̵��) */
  {   0, "fmgen",        X_INV,                                       0,0,0,0},
  {   0, "nofmgen",      X_INV,                                       0,0,0,0},
#endif
#endif
};



/*--------------------------------------------------------------------------
 * 2�Ĥ�Ϣ³�������� opt1, opt2 ����������������������ο� (1��2) ���֤���
 *  (�������۾�(�ͤ��ϰϳ��ʤ�)�ǽ������ʤ��ä�����Ʊ����)
 *
 * ̤�Τΰ����ξ�硢���������� 0 ���֤���
 * �����Ƿ�³��ǽ�ʰ۾郎ȯ���������ϡ� -1 ���֤���
 *--------------------------------------------------------------------------*/

static	int	check_option( char *opt1, char *opt2, int priority )
{
  int	i,  ret_val = 1;
  int	ignore, applied;
  char	*end;


  if( opt1==NULL )     return 0;
  if( opt1[0] != '-' ) return -1;


  /* ���ץ����ʸ����ˤ˹��פ����Τ�õ���ޤ��礦 */

  for( i=0; i<COUNTOF(option_table); i++ ){
    if( strcmp( &opt1[1], option_table[i].name )==0 ) break;
  }

  if( i==COUNTOF(option_table) ){

    /* ���Ĥ���ʤ���С�MAME �Υ��ץ���󤫤�õ���ޤ� */

    i = xmame_check_option( opt1, opt2, priority );
    return i;

  }else{

    /* ���Ĥ���н������������ǽ��ʽ����� 'ͥ���٤��⤤' �� 'Ʊ��' ���Τ� */

    if( priority < opt_prioroty[ option_table[i].group ] ){
      ignore = TRUE;
    }else{
      ignore = FALSE;
    }
    applied = FALSE;


    /* ���ץ����Υ������̤˽������ޤ� */

    switch( option_table[i].type ){

    case X_FIX:			/* �ʤ�:   *var = (int)val1 [���]           */
      {
	if( ignore == FALSE ){
	  *((int*)option_table[i].var) = (int)option_table[i].val1;
	  applied = TRUE;
	}
      }
      break;

    case X_INT:			/* int:    *var = argv  [�ϰ� var1��val2]    */
      {
	int low, high, work;

	if( opt2 ){
	  ret_val ++;
	  if( ignore == FALSE ){
	    low  = (int)option_table[i].val1;
	    high = (int)option_table[i].val2;
	    work = strtol( opt2, &end, 0 );

	    if( *end=='\0' && low <= work && work <= high ){
	      *((int*)option_table[i].var) = work;
	      applied = TRUE;
	    }else{
	      fprintf( stderr, "error: invalid value %s %s\n", opt1, opt2 );
	    }
	  }
	}else{
	  fprintf( stderr, "error: %s requires an argument\n", opt1 );
	}
      }
      break;

    case X_DBL:			/* double: *var = argv  [�ϰ� var1��val2]    */
      {
	double low, high, work;

	if( opt2 ){
	  ret_val ++;
	  if( ignore == FALSE ){
	    low  = (double)option_table[i].val1;
	    high = (double)option_table[i].val2;
	    work = strtod( opt2, &end );

	    if( *end=='\0' && low <= work && work <= high ){
	      *((double*)option_table[i].var) = work;
	      applied = TRUE;
	    }else{
	      fprintf( stderr, "error: invalid value %s %s\n", opt1, opt2 );
	    }
	  }
	}else{
	  fprintf( stderr, "error: %s requires an argument\n", opt1 );
	}
      }
      break;

    case X_STR:			/* ʸ����: strcpy( var, argv );              */
      {
	char *work;

	if( opt2 ){
	  ret_val ++;
	  if( ignore == FALSE ){
	    work = (char*)malloc( strlen(opt2)+1 );
	    if( work == NULL ){
	      fprintf( stderr, "error: malloc failed for %s\n", opt1 );
	      return -1;
	    }else{
	      strcpy( work, opt2 );
	      if( *(char **)option_table[i].var ){
		free( *(char **)option_table[i].var );
	      }
	      *(char **)option_table[i].var = work;
	      applied = TRUE;
	    }
	  }
	}else{
	  fprintf( stderr, "error: %s requires an argument\n", opt1 );
	}
      }
      break;

    case X_FN:			/* FN��ǽ: *var = ��ǽ			     */
      {
	int work=0;
	int k;

	if( opt2 ){
	  ret_val ++;
	  if( ignore == FALSE ){
	    for( k=1; k < COUNTOF(fn_index); k++ ){
	      if( my_strcmp( opt2, fn_index[k].str ) == 0 ){
		work = fn_index[k].num;
		break;
	      }
	    }
	    if( k==COUNTOF(fn_index) ){
	      fprintf( stderr, "error: invalid function %s %s\n", opt1, opt2 );
	    }else{
	      *((int*)option_table[i].var) = work;
	      applied = TRUE;
	    }
	  }
	}else{
	  fprintf( stderr, "error: %s requires an argument\n", opt1 );
	}
      }
      break;

    case X_NOP:			/* ̵����:                                   */
      if( option_table[i].var ){
	if( opt2 ){
	  ret_val ++;
	  applied = TRUE;
	}else{
	  fprintf( stderr, "error: %s requires an argument\n", opt1 );
	}
      }else{
	  applied = TRUE;
      }
      break;

    case X_INV:			/* ̵��:                                     */
      if( option_table[i].var && opt2 ){
	ret_val ++;
	fprintf( stderr, "error: invalid option %s %s\n", opt1, opt2 );
      }else{
	fprintf( stderr, "error: invalid option %s\n", opt1 );
      }
      applied = FALSE;
      break;

    default:
      break;
    }


    /* ������θƤӽФ��ؿ�������С������ƤӤޤ� */

    if( option_table[i].func && applied ){
      if( (option_table[i].func)( opt2 ) != 0 ){
	fprintf( stderr, "error: invalid value %s %s\n", opt1, opt2 );
      }
    }


    /* ͥ�����٤�񤭴����Ƥ���� */

    opt_prioroty[ option_table[i].group ] = priority;

    return ret_val;
  }
}





/*--------------------------------------------------------------------------
 * ��ư���Υ��ץ�������Ϥ��롣
 *	����ͤϡ������ 0���۾�� �� 0
 *
 *	-help ���ץ�����Ĥ���ȡ�����Ū�˽�λ���롣
 *--------------------------------------------------------------------------*/
static	int	get_option( int argc, char *argv[], int priority )
{
  int	i;
  char	*end;
  int	img, disk_count;

  disk_count = 0;

	/* ������1�ĤŤĽ�˲��� */

  for( i=1; i<argc; ){

    if( *argv[i] != '-' ){	/* '-' �ʳ��ǻϤޤ륪�ץ����ϡ��ե�����̾ */

      switch( disk_count ){
      case 0:
      case 1:
	if( strlen(argv[i]) >= QUASI88_MAX_FILENAME ){
	  fprintf( stderr, "error: image file name \"%s\" is too long\n",
		   argv[i] );
	  break;
	}

	boot_file[ disk_count ] = argv[i];

	if( i+1 < argc ){				/* ���ΰ����������  */
	  img = strtol( argv[i+1], &end, 0 );		/* ���ͤ��Ѵ���OK�ʤ�*/
	  if( *end=='\0' ){				/* ����ϥ��᡼���ֹ�*/
	    if( img<1 || img>MAX_NR_IMAGE ){
	      fprintf( stderr, "error: invalid image value %d\n", img );
	      img = 1;
	    }
	    boot_image[ disk_count ] = img;

	    if( i+2 < argc ){				/* ����˰����������*/
	      img = strtol( argv[i+2], &end, 0 );	/* ���ͤ��Ѵ���OK�ʤ�*/
	      if( *end=='\0' ){				/* ����⥤�᡼���ֹ�*/
		if( img<1 || img>MAX_NR_IMAGE ){
		  fprintf( stderr, "error: invalid image value %d\n", img );
		  img = 1;
		}
		if( disk_count+1 >= NR_DRIVE ){
		  fprintf( stderr, "error: too many image number\n" );
		  img = 1;
		}
		boot_file[ disk_count+1 ] = argv[i];
		boot_image[ disk_count+1 ] = img;
		disk_count ++;
		i ++;
	      }

	    }
	    i ++;
	  }
	}
	disk_count ++;
	break;

      default:			/* 3�İʾ�ե�������ꤷ����硢̵�� */

	fprintf( stderr, "warning: too many image file\n" );
	break;
      }

      i ++;

    }else{			/* '-' �ǻϤޤ�����ϡ����ץ���� */

      int j  = check_option( argv[i], (i+1<argc)? argv[i+1] :NULL, priority );
      if( j < 0 ){			/* ��̿Ū���顼�ʤ顢���ϼ��� */
	return -1;
      }
      if( j==0 ){			/* ̤�ΤΥ��ץ����ϡ������å� */
	fprintf( stderr, "error: unknown option %s\n", argv[i] );
	j = 1;
      }

      i += j;

    }
  }

  return 0;
}






/*--------------------------------------------------------------------------
 * �Ķ��ե�����Υ��ץ�������Ϥ��롣
 *	����ͤϡ������ 0���۾�� �� 0
 *
 *	-help ���ץ�����Ĥ���ȡ�����Ū�˽�λ���롣
 *--------------------------------------------------------------------------*/

/* �Ķ��ե�����1�Ԥ�����κ���ʸ���� */
#define	MAX_RCFILE_LINE	(256)


static	int	get_config_file( FILE *fp, int priority )
{
  int  line_cnt;
  char line[ MAX_RCFILE_LINE ];
  char parm[ MAX_RCFILE_LINE ];


		/* ����ե������1�ԤŤĲ��� */

  line_cnt = 0;

  while( fgets( line, MAX_RCFILE_LINE, fp ) ){
    char *p = &line[0];
    char *q = &parm[0];
    int   argcnt = 0;			/* �ѥ�᡼����1�Ԥ����ꡢ*/
    char *argval[2];			/* ����ǡ�2�Ĥޤ�        */
    int   result;
    int   esc = FALSE;			/* ���������ץ������󥹽����� */

    line_cnt++;
    *q = '\0';

    while(1){
      while(1){
	if     ( *p=='#' || *p=='\n' || *p=='\0' ) goto BREAK;
	else if( *p==' ' || *p=='\t' ) p++;
	else{
	  if( argcnt==2 ){ argcnt++;              goto BREAK; }
	  else           { argval[argcnt++] = q;  break;      }
	}
      }
      while(1){
	if      (*p=='\n' || *p=='\0')         { *q++ = '\0'; goto BREAK; }
	else if (*p=='#'              && !esc ){ *q++ = '\0'; goto BREAK; }
	else if((*p==' ' || *p=='\t') && !esc ){ *q++ = '\0'; p++; break; }
	else if (*p==CONFIG_ESC       && !esc ){         p++; esc=TRUE;   }
	else                                   { *q++ = *p++; esc=FALSE;  }
      }
    }
  BREAK:;


	/* �ѥ�᡼�����ʤ���м��ιԤء�����в��Ͻ��� */

    if      ( argcnt==0 ){			/* �ѥ�᡼���ʤ�    */
      continue;

    }else if( argcnt>2 ){			/* �ѥ�᡼��3�İʾ� */
      fprintf( stderr, "warning: too many argument in line %d\n", line_cnt );

    }else{					/* �ѥ�᡼��1��2��  */
      result = check_option( argval[0], (argcnt==2) ? argval[1] : NULL,
			     priority );

      if( result != argcnt ){		/* ���顼���� ���顼�Ԥ�ɽ�� */
	fprintf( stderr, "warning: error in line %d (ignored)\n", line_cnt );
      }
    }

  }

  return 0;
}















/***********************************************************************
 * �����ν���
 *	���顼ȯ���ʤɤǽ�����³�ԤǤ��ʤ���硢�����֤���
 ************************************************************************/

int	config_init( int argc, char *argv[] )
{
  FILE	*fp = NULL;
  int	same, step;
  char	*fname;

  command = argv[0];

  boot_file[0]  = boot_file[1]  = NULL;
  boot_image[0] = boot_image[1] = 0;


	/* ��ư���Υ��ץ�������� */

  if( get_option( argc, argv, 2 ) != 0 ) return FALSE;


  verbose_proc	= verbose_level & 0x01;

  if( resume_flag ){
    boot_file[0]  = boot_file[1]  = NULL;
  }


	/* ��ư���˻��ꤵ�줿���ǥ��������᡼���ե����뤬���뤫�����å� */

  if( boot_file[0] == boot_file[1] ) same = TRUE;
  else                               same = FALSE;

  if( boot_file[0] ){				/* DRIVE 1 �ե��������� */
    fname = alloc_diskname( boot_file[0] );
    if( fname == NULL ){
      printf( "\n" );
      printf( "[[[ %-26s ]]]\n", "Open failed" );
      printf( "[[[   drive 1: %-15s ]]]\n" "\n", boot_file[0] );
    }
    boot_file[0] = fname;
  }

  if( same ){					/* DRIVE 1/2 Ʊ���ե������ */

    boot_file[1] = boot_file[0];

  }else{
    if( boot_file[1] ){				/* DRIVE 2 �ե��������� */
      fname = alloc_diskname( boot_file[1] );
      if( fname == NULL ){
	printf( "\n" );
	printf( "[[[ %-26s ]]]\n", "Open failed" );
	printf( "[[[   drive 2: %-15s ]]]\n" "\n", boot_file[1] );
      }
      boot_file[1] = fname;
    }
  }


		/* ����ե��������			*/
		/*	step 0 : ��������ե���������	*/
		/*	step 1 : ��������ե���������	*/

  for( step=0; step<2; step ++ ){

    FILE *fp;
    char *dummy;

    if( load_config ){

      if( step == 0 ){
			/* ��������ե�����Υե�����̾ */
	fname = alloc_global_cfgname();
	dummy = "Global Config File";

      }else{
			/* �ɥ饤�֤ޤ��ϥơ��פΥ��᡼���򸵤ˤ����ե�����̾*/
	if     ( boot_file[0] )        fname = boot_file[0];
	else if( boot_file[1] )        fname = boot_file[1];
	else if( file_tape[CLOAD][0] ) fname = file_tape[CLOAD];
	else break;

	fname = alloc_local_cfgname( fname );
	dummy = "Local Config File";
      }

      if( fname ) fp = fopen( fname, "r" );
      else        fp = NULL;

      if( verbose_proc ){
	if( fp ){ printf( "\"%s\" read and initialize\n", fname ); }
	else    { printf( "\"%s\" open failed\n", (fname) ? fname : dummy ); }
      }
      if( fname ) free( fname );

      if( fp ){
	int result = get_config_file( fp, 1 );
	fclose( fp );
	if( result != 0 ) return FALSE;
      }

      verbose_proc = verbose_level & 0x01;
    }
  }


	/* �ǥ��������᡼��̾�򥻥å� */

  memset( file_disk[0], 0, QUASI88_MAX_FILENAME );
  memset( file_disk[1], 0, QUASI88_MAX_FILENAME );
  if( boot_file[0] ) strcpy( file_disk[0], boot_file[0] );
  if( boot_file[1] ) strcpy( file_disk[1], boot_file[1] );
  image_disk[0] = boot_image[0] -1;	/* ���᡼���ֹ�� 1������(0���ˤ���) */
  image_disk[1] = boot_image[1] -1;
  readonly_disk[0] = menu_readonly;	/* �꡼�ɥ���꡼�ϡ�ξ�ɥ饤�ֶ��� */
  readonly_disk[1] = menu_readonly;



	/* �ߴ�ROM������ˡ��ե����뤬���뤫�����å� */

  if( file_compatrom ){
    fname = alloc_romname( file_compatrom );
    if( fname ){
      file_compatrom = fname;
    }
  }



	/* �Ƽ�ǥ��쥯�ȥ��ɽ�� (�ǥХå���) */

  if( verbose_proc ){
    const char *d;
    d = osd_dir_cwd ( );  printf( "cwd  directory = %s\n", d ? d : "(undef)" );
    d = osd_dir_rom ( );  printf( "rom  directory = %s\n", d ? d : "(undef)" );
    d = osd_dir_disk( );  printf( "disk directory = %s\n", d ? d : "(undef)" );
    d = osd_dir_tape( );  printf( "tape directory = %s\n", d ? d : "(undef)" );
    d = osd_dir_snap( );  printf( "snap directory = %s\n", d ? d : "(undef)" );
    d = osd_dir_state();  printf( "stat directory = %s\n", d ? d : "(undef)" );
    d = osd_dir_gcfg( );  printf( "gcfg directory = %s\n", d ? d : "(undef)" );
    d = osd_dir_lcfg( );  printf( "lcfg directory = %s\n", d ? d : "(undef)" );
  }

  return TRUE;
}
