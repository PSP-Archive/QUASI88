#include <pspgu.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>

#include <string.h>

#include "quasi88.h"
#include "keyboard.h"
#include "emu.h"
#include "fdc.h"
#include "memory.h"
#include "pc88main.h"
#include "intr.h"
#include "screen.h"

#include "ptk_menu.h"
#include "ptk_softkey.h"
#include "ptk_video.h"
#include "ptk_font.h"
#include "ptk_ext.h"
#include "ptk_popup.h"

#ifdef USE_FMGEN
#include "mame-quasi88.h"
#endif

#define MENU_BLEND (0x808080)

enum
{
MENU_NONE,
MENU_RADIO,
MENU_DBL_RADIO,
MENU_CHECK,
MENU_FILE,
MENU_VALUE,
MENU_JUMP,
MENU_BACK,
MENU_EXIT,
MENU_GETKEY,
MENU_ABOUT,
MENU_IMAGE_SEL,
MENU_CMDEXE,
MENU_DISP_DOUBLE,
MENU_DISP_FLOAT,
MENU_DISP_INT,
MENU_DISP_STRING

} MENU_ITEMTYPE;

enum
{
MCMD_NONE,
MCMD_CREATE_DISK,
MCMD_RESET,
MCMD_QUIT,
MCMD_SAVESETTING,
MCMD_SAVESTATE,
MCMD_LOADSTATE,
MCMD_SETCLOCK
} MENU_CMDTYPE;

char pmenu_curdir[512];
char pmenu_disk0[512];
char pmenu_disk1[512];

char pmenu_disk0_file[128];
char pmenu_disk1_file[128];

char pmenu_drive0_imgname[32];
char pmenu_drive1_imgname[32];


char *pmenu_dskpath[]={(char *)&pmenu_disk0,(char *)&pmenu_disk1};
char *pmenu_dskfile[]={(char *)&pmenu_disk0_file,(char *)&pmenu_disk1_file};
char *pmenu_imgname[]={(char *)&pmenu_drive0_imgname,(char *)&pmenu_drive1_imgname};


char *pmenu_curstr;
int  pmenu_drive0_img = 0;
int  pmenu_drive1_img = 0;

int pmenu_kctbl[PMENU_KCTBL];
int pmenu_kc_aflag = 0;


int pmenu_video_current = 0;
int pmenu_type_fs	 = 0;
int pmenu_fullscreen = 0;
int pmenu_showfps	 = 0;
int pmenu_bgrun		 = 0;
int pmenu_menu_start = 1;

int pmenu_curmenu_idx = 0;

int pmenu_reset_start = 1;

int pmenu_rev_menusw  = 0;

int pmenu_cpuclk_int = 3993600;
int pmenu_sndclk_int = 3993600;

typedef struct st_menucont
{
	char *name;
	int   cmd;
	void *ptr;
	int   value;
} t_menucont;

enum
{
 PMENU_IDX_MAIN,
 PMENU_IDX_CLOCK,
 PMENU_IDX_BASIC,
 PMENU_IDX_DISK,
 PMENU_IDX_DIP,
 PMENU_IDX_SOUND,
 PMENU_IDX_SUBCPU,
 PMENU_IDX_KEYCONF,
 PMENU_IDX_QUIT,
 PMENU_IDX_FPS,
 PMENU_IDX_PSPMENU,
 PMENU_IDX_PC88MENU,
 PMENU_IDX_DEBUG,
 PMENU_IDX_SNDBUF,
 PMENU_IDX_OVERCLOCK,
 PMENU_IDX_DUMMY
} PMENU_IDXES;

typedef struct st_menu
{
	char *title;
	int idx_menu;
	t_menucont cont[];
} t_menu;

t_menu pmenu_quit=
{
	"DO YOU REALLY WANT TO QUIT?",PMENU_IDX_QUIT,
	{
	{"QUIT",MENU_CMDEXE,NULL,MCMD_QUIT},
	{"BACK",MENU_BACK,NULL,PMENU_IDX_MAIN},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_disk=
{
	"DISK MENU",PMENU_IDX_DISK,
	{
	{"DRIVE 1",MENU_FILE,&pmenu_disk0,0},
	{"DRIVE 1 IMAGE",MENU_IMAGE_SEL,NULL,0},
	{"DRIVE 2",MENU_FILE,&pmenu_disk1,1},
	{"DRIVE 2 IMAGE",MENU_IMAGE_SEL,NULL,1},
	{"BACK",MENU_BACK,NULL,PMENU_IDX_MAIN},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_debug=
{
	"DEBUG MENU",PMENU_IDX_DEBUG,
	{
	{"CPU CLK",MENU_DISP_DOUBLE,&cpu_clock_mhz,0},
	{"SND CLK",MENU_DISP_DOUBLE,&sound_clock_mhz,0},
	{"BACK",MENU_JUMP,NULL,PMENU_IDX_MAIN},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_pspmenu=
{
	"PSP MENU",PMENU_IDX_PSPMENU,
	{
	{"BG RUN"				,MENU_CHECK,&pmenu_bgrun,0},
	{"SHOW FPS"				,MENU_CHECK,&pmenu_showfps,0},
	{"NORMAL SCREEN"		,MENU_RADIO,&pmenu_type_fs,0},
	{"FULL SCREEN"			,MENU_RADIO,&pmenu_type_fs,1},
	{"TRUE WIDE"			,MENU_RADIO,&pmenu_type_fs,2},
	{"MENU AT START"		,MENU_CHECK,&pmenu_menu_start,0},
	{"RESET WITH EXIT MENU"	,MENU_CHECK,&pmenu_reset_start,0},
	{"REVERSE MENU SW"		,MENU_CHECK,&pmenu_rev_menusw,0},

	{"CPU OVERCLOCK"		,MENU_JUMP,NULL,PMENU_IDX_OVERCLOCK},
	{"SOUND BUFFER"			,MENU_JUMP,NULL,PMENU_IDX_SNDBUF},

//	{"DEBUG MODE"			,MENU_JUMP,NULL,PMENU_IDX_DEBUG},

	{"BACK"					,MENU_BACK,NULL,PMENU_IDX_MAIN},
	{NULL,0,NULL,0}
	}
};


t_menu pmenu_sndbuf=
{
	"BUFFER SIZE AFTER RESET",PMENU_IDX_SNDBUF,
	{
	{"SMALL" ,MENU_RADIO,&ext_sound_buf_size,0x800},
	{"MIDDLE",MENU_RADIO,&ext_sound_buf_size,0x1000},
	{"LARGE" ,MENU_RADIO,&ext_sound_buf_size,0x2000},
	{"BACK"  ,MENU_BACK,NULL,PMENU_IDX_PSPMENU},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_overclock=
{
	"CPU OVERCLOCK",PMENU_IDX_OVERCLOCK,
	{
	{"222MHz",MENU_RADIO,&ext_cpuclock,222},
	{"266MHz",MENU_RADIO,&ext_cpuclock,266},
	{"300MHz",MENU_RADIO,&ext_cpuclock,300},
	{"333MHz",MENU_RADIO,&ext_cpuclock,333},
	{"APPLY" ,MENU_CMDEXE,NULL,MCMD_SETCLOCK},
	{"BACK"  ,MENU_BACK,NULL,PMENU_IDX_PSPMENU},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_dip=
{
	"DIP MENU",PMENU_IDX_DIP,
	{
	{"BACK",MENU_BACK,NULL,PMENU_IDX_PC88MENU},
	{NULL,0,NULL,0}
	}
};



t_menu pmenu_sound=
{
	"SOUND BOARD",PMENU_IDX_SOUND,
	{
	{"SOUND 4MHz" ,MENU_RADIO,&pmenu_sndclk_int,3993600},
	{"SOUND 8MHz" ,MENU_RADIO,&pmenu_sndclk_int,3993600*2},
	{"OPN"        ,MENU_RADIO,&ext_sound_board,0},
	{"OPNA(SBII)" ,MENU_RADIO,&ext_sound_board,1},
	
#ifdef USE_FMGEN
	{"USE FMGEN"  ,MENU_CHECK,&use_fmgen,0},
#endif
	{"BACK",MENU_BACK,NULL,PMENU_IDX_PC88MENU},
	{NULL,0,NULL,0}
	}
};
t_menu pmenu_subcpu=
{
	"SUBCPU",PMENU_IDX_SUBCPU,
	{
	{"ONLY SUB WHEN DISK PROCESSING",MENU_RADIO,&cpu_timing,0},
	{"DUAL CPU WHEN DISK PROCESSING",MENU_RADIO,&cpu_timing,1},
	{"ALWAYS"   ,MENU_RADIO,&cpu_timing,2},
	{"FDC WAIT" ,MENU_CHECK,&fdc_wait,0},
	{"HS BASIC" ,MENU_CHECK,&highspeed_mode,0},
	{"MEM WAIT" ,MENU_CHECK,&memory_wait,0},
	{"BACK"     ,MENU_BACK,NULL,PMENU_IDX_PC88MENU},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_fps=
{
	"FRAME SKIP",PMENU_IDX_FPS,
	{
	{"USE AUTOSKIP",MENU_CHECK,&use_auto_skip,0},
	{"60fps" ,MENU_RADIO,&frameskip_rate,1},
	{"30fps" ,MENU_RADIO,&frameskip_rate,2},
	{"20fps" ,MENU_RADIO,&frameskip_rate,3},
	{"15fps" ,MENU_RADIO,&frameskip_rate,4},
	{"12fps" ,MENU_RADIO,&frameskip_rate,5},
	{"10fps" ,MENU_RADIO,&frameskip_rate,6},
	{"6fps"  ,MENU_RADIO,&frameskip_rate,7},
	{"5fps"  ,MENU_RADIO,&frameskip_rate,8},
	{"BACK"  ,MENU_BACK,NULL,PMENU_IDX_PC88MENU},
	{NULL,0,NULL,0}
	}
};


t_menu pmenu_basic=
{
	"BASIC MODE",PMENU_IDX_BASIC,
	{
	{"AUTO"    ,MENU_RADIO,&ext_boot_basic,-1},
	{"N88 V2"  ,MENU_RADIO,&ext_boot_basic,3},
	{"N88 V1H" ,MENU_RADIO,&ext_boot_basic,2},
	{"N88 V1S" ,MENU_RADIO,&ext_boot_basic,1},
	{"N"       ,MENU_RADIO,&ext_boot_basic,0},
	{"BACK"    ,MENU_BACK,NULL,PMENU_IDX_PC88MENU},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_clock=
{
	"CPU CLOCK",PMENU_IDX_CLOCK,
	{
	{"CPU 4MHz",MENU_RADIO,&pmenu_cpuclk_int,3993600},
	{"CPU 8MHz",MENU_RADIO,&pmenu_cpuclk_int,3993600*2},
	{"SW 4MHz" ,MENU_RADIO,&ext_boot_clock_4mhz,1},
	{"SW 8MHz" ,MENU_RADIO,&ext_boot_clock_4mhz,0},
	{"BACK",MENU_BACK,NULL,PMENU_IDX_PC88MENU},
	{NULL,0,NULL,0}
	}
};


t_menu pmenu_pc88menu=
{
	"PC88 MENU",PMENU_IDX_PC88MENU,
	{
//	{"DIP",MENU_JUMP,NULL,PMENU_IDX_DUMMY},
	{"CPU CLOCK" ,MENU_JUMP,NULL,PMENU_IDX_CLOCK},
	{"SUBCPU"    ,MENU_JUMP,NULL,PMENU_IDX_SUBCPU},
	{"BASIC MODE",MENU_JUMP,NULL,PMENU_IDX_BASIC},
	{"SOUND"     ,MENU_JUMP,NULL,PMENU_IDX_SOUND},
	{"FRAMESKIP" ,MENU_JUMP,NULL,PMENU_IDX_FPS},
	{"BACK"      ,MENU_BACK,NULL,PMENU_IDX_MAIN},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_keyconf=
{
	"KEY CONFIG",PMENU_IDX_KEYCONF,
	{
	{"USE ANALOG",MENU_CHECK,&pmenu_kc_aflag,0},
	{"UP",MENU_GETKEY    ,&pmenu_kctbl[PMENU_UP],0},
	{"DOWN",MENU_GETKEY  ,&pmenu_kctbl[PMENU_DW],0},
	{"LEFT",MENU_GETKEY  ,&pmenu_kctbl[PMENU_LE],0},
	{"RIGHT",MENU_GETKEY ,&pmenu_kctbl[PMENU_RI],0},

	{"CIRCLE",MENU_GETKEY  ,&pmenu_kctbl[PMENU_A],0},
	{"CROSS",MENU_GETKEY   ,&pmenu_kctbl[PMENU_B],0},
	{"SQUARE",MENU_GETKEY  ,&pmenu_kctbl[PMENU_C],0},
	{"TRIANGLE",MENU_GETKEY,&pmenu_kctbl[PMENU_D],0},

	{"START",MENU_GETKEY   ,&pmenu_kctbl[PMENU_E],0},
	{"SELECT",MENU_GETKEY  ,&pmenu_kctbl[PMENU_F],0},

	{"A-UP",MENU_GETKEY    ,&pmenu_kctbl[PMENU_AUP],0},
	{"A-DOWN",MENU_GETKEY  ,&pmenu_kctbl[PMENU_ADW],0},
	{"A-LEFT",MENU_GETKEY  ,&pmenu_kctbl[PMENU_ALE],0},
	{"A-RIGHT",MENU_GETKEY ,&pmenu_kctbl[PMENU_ARI],0},

	{"BACK",MENU_BACK,NULL,PMENU_IDX_MAIN},
	{NULL,0,NULL,0}
	}
};

t_menu pmenu_main=
{
	"MAIN MENU",PMENU_IDX_MAIN,
	{
	{"DISK",MENU_JUMP,NULL,PMENU_IDX_DISK},
	{"PC88 MENU",MENU_JUMP,NULL,PMENU_IDX_PC88MENU},
	{"KEY CONFIG",MENU_JUMP,NULL,PMENU_IDX_KEYCONF},
	{"PSP SETTING",MENU_JUMP,NULL,PMENU_IDX_PSPMENU},
	{"SAVE STATE",MENU_CMDEXE,NULL,MCMD_SAVESTATE},
	{"LOAD STATE",MENU_CMDEXE,NULL,MCMD_LOADSTATE},
	{"SAVE SETTING",MENU_CMDEXE,NULL,MCMD_SAVESETTING},
	{"RESET",MENU_CMDEXE,NULL,MCMD_RESET},
	{"ABOUT",MENU_ABOUT,NULL,0},
	{"QUIT",MENU_JUMP,NULL,PMENU_IDX_QUIT},
	{"EXIT MENU",MENU_EXIT,NULL,0},
	{NULL,0,NULL,0}
	}
};


int pmenu_sw;
int pmenu_cy;

t_menu *pmenus[]=
{
 &pmenu_debug,
 &pmenu_pc88menu,
 &pmenu_main,
 &pmenu_clock,
 &pmenu_basic,
 &pmenu_disk,
 &pmenu_dip,
 &pmenu_sound,
 &pmenu_subcpu,
 &pmenu_keyconf,
 &pmenu_quit,
 &pmenu_fps,
 &pmenu_pspmenu,
 &pmenu_sndbuf,
 &pmenu_overclock,
 NULL
};
t_menu *pmenu_current;

int pmenu_old_index;
int pmenu_old_cy;

int pmenu_index;
int pmenu_len;
int pmenu_lastmenu;
int *pmenu_getkey_code;

int pmenu_curdrive;

int pmenu_mode;

enum
{
MENU_MODE_NORMAL,
MENU_MODE_FILE,
MENU_MODE_GETKEY,
MENU_MODE_IMAGESEL,
MENU_MODE_ABOUT
} MENU_MODETYPE;

char *pmenu_about[]=
{ "QUASI88-0.6.1 for PSP",
  "",
  "Sound output is ported from",
  "xmame-0.71.1",
  "",
  "(c) Nicola Salmoria,",
  "The MAME Team and xmame project",
#ifdef USE_FMGEN
  "",
  "FM Sound Generator",
  "with OPN/OPM interface",
  "Copyright (C) by cisc 1998, 2003.",
#endif
  NULL
};

void pmenu_dirup(void)
{
	int i;
	for(i=strlen(pmenu_curdir); i>0 && pmenu_curdir[i]!='/'; i--);

	if (pmenu_curdir[i]=='/') 
		pmenu_curdir[i]=0;
}

void pmenu_dirdown(char *dir)
{
	strcat(pmenu_curdir,"/");
	strcat(pmenu_curdir,dir);
}

int pmenu_menusize(t_menucont *ptr)
{
	int i;
	for(i=0; ptr[i].name; i++);
	return i;
}

t_menu *pmenu_getmenu_idx(int idx)
{
	int i;
	for( i = 0; pmenus[i]; i++)
	{
		if (pmenus[i]->idx_menu == idx)
			return pmenus[i];
	}
	return pmenus[0]; // the index is not found ...
}

void pmenu_save_menupos(void)
{
	pmenu_old_index = pmenu_index;
	pmenu_old_cy = pmenu_cy;
	pmenu_lastmenu = pmenu_curmenu_idx;
}

void pmenu_setmenu_ex(int menu,int index,int cy)
{
	pmenu_curstr = NULL;

	pmenu_save_menupos();

	pmenu_curmenu_idx = menu;
	pmenu_current = pmenu_getmenu_idx(menu);
	pmenu_len = pmenu_menusize(pmenu_current->cont);


	pmenu_cy = cy;
	pmenu_index = index;


}


void pmenu_setmenu(int menu)
{
	pmenu_setmenu_ex(menu,0,0);
}

void pmenu_init(const char *dir)
{
	pmenu_sw = 0;
	pmenu_disk0[0] = 0;
	pmenu_disk1[0] = 0;
	pmenu_disk0_file[0] = 0;
	pmenu_disk1_file[0] = 0;
	
	pmenu_kctbl[PMENU_UP] = KEY88_KP_8;
	pmenu_kctbl[PMENU_DW] = KEY88_KP_2;
	pmenu_kctbl[PMENU_LE] = KEY88_KP_4;
	pmenu_kctbl[PMENU_RI] = KEY88_KP_6;

	pmenu_kctbl[PMENU_AUP] = KEY88_KP_8;
	pmenu_kctbl[PMENU_ADW] = KEY88_KP_2;
	pmenu_kctbl[PMENU_ALE] = KEY88_KP_4;
	pmenu_kctbl[PMENU_ARI] = KEY88_KP_6;
	
	pmenu_kctbl[PMENU_A] = 	KEY88_SPACE;
	pmenu_kctbl[PMENU_B] =  KEY88_ESC;
	pmenu_kctbl[PMENU_C] =  KEY88_RETURN;
	pmenu_kctbl[PMENU_D] =  KEY88_INVALID;
	pmenu_kctbl[PMENU_E] =  KEY88_INVALID;
	pmenu_kctbl[PMENU_F] =  KEY88_INVALID;
	
	pmenu_drive0_img = 0;
	pmenu_drive1_img = 0;
	
	pmenu_old_cy = pmenu_old_index = 0;
	
	pmenu_mode = MENU_MODE_NORMAL;
	pmenu_getkey_code = NULL;
	
	pmenu_setmenu(PMENU_IDX_MAIN);
	
	strcpy(pmenu_curdir,dir);
}

/////////////
// disk
/////////////
 
void pmenu_make_diskname(char *dst,char *src)
{
	char *p,*oldp;
	
	oldp = p = src;
	while(p && *p)
	{
		oldp = p; p = strchr(p+1,'/');
	}
	
	if (*oldp == '/') 
		oldp++;

	strcpy(dst,oldp);
}

void pmenu_set_current_image_name(int drv)
{
	int curimg;
	
	curimg = ext_get_curimage(drv);
	if (!drv)
		pmenu_drive0_img = drv;
	else
		pmenu_drive1_img = drv;
	
	if (curimg < 0)
		*pmenu_imgname[drv] = 0;
	else
		strcpy(pmenu_imgname[drv],ext_getimgname(drv,curimg+1));
}

void pmenu_set_image(int drv,int img)
{
	if (!img)
	{
		*pmenu_dskpath[drv] = 
		*pmenu_dskfile[drv] = 
		*pmenu_imgname[drv] = 0;
	} else
	{
		strcpy(pmenu_imgname[drv],ext_getimgname(drv,img));
	}
}


#define MENU_ORG_X 60
#define MENU_ORG_Y 10
#define MENU_SIZE (16)

void pmenu_disp_list(void)
{
	int i,j;
	int x,y;
	t_menucont *ptr;
	
	ptr = pmenu_current->cont;
	x = MENU_ORG_X+8;

	font_printf(x,6*8,"-- %s --",pmenu_current->title);
		
	for(y=MENU_ORG_Y,i=pmenu_index; i < pmenu_len && i<pmenu_index+MENU_SIZE; i++,y++)
	{
		switch(ptr[i].cmd)
		{
			case MENU_DISP_INT:
						font_printf(x,y*8,"%s : %d",ptr[i].name,*(int *)ptr[i].ptr);
			break;
			case MENU_DISP_DOUBLE:
						font_printf(x,y*8,"%s : %f",ptr[i].name,*(double *)ptr[i].ptr);
			break;
			case MENU_DISP_FLOAT:
						font_printf(x,y*8,"%s : %f",ptr[i].name,*(float *)ptr[i].ptr);
			break;
			case MENU_DISP_STRING:
						font_printf(x,y*8,"%s : %s",ptr[i].name,ptr[i].ptr);
			break;


			case MENU_GETKEY:
				j = getkey_code_swkbd(*(int *)ptr[i].ptr);
				if (j < 0)
						font_printf(x,y*8,"%s : NONE",ptr[i].name);
				else
						font_printf(x,y*8,"%-10s : %s",ptr[i].name,getname_swkbd(j));
			break;
			case MENU_CHECK:
				if (*(int *)ptr[i].ptr)
						font_printf(x,y*8,"[x] %-10s ",ptr[i].name);
					else
						font_printf(x,y*8,"[ ] %-10s ",ptr[i].name);
			break;
			case MENU_RADIO:
				if (*(int *)ptr[i].ptr == ptr[i].value)
						font_printf(x,y*8,"[x] %-10s",ptr[i].name);
					else
						font_printf(x,y*8,"[ ] %-10s",ptr[i].name);
				break;
			case MENU_DBL_RADIO:
				if (*(double *)ptr[i].ptr == (double)ptr[i].value/1000000)
						font_printf(x,y*8,"[x] %-10s",ptr[i].name);
					else
						font_printf(x,y*8,"[ ] %-10s",ptr[i].name);
				break;
			case MENU_FILE:
				font_printf(x,y*8,"%-10s : %s",ptr[i].name,pmenu_dskfile[ptr[i].value]);
			break;
			case MENU_IMAGE_SEL:
				font_printf(x,y*8,"%-10s : %s",ptr[i].name,pmenu_imgname[ptr[i].value]);
			break;
			case MENU_JUMP:
				font_printf(x,y*8,"%s...",ptr[i].name);
			break;

			default:
				font_printf(x,y*8,"%s  ",ptr[i].name);
				break;
		}
	}
	
//	font_printf(0,24,"cpu : %d",ext_cpuclock);
}

void pmenu_disp_cur(void)
{
	if (pmenu_sw)
		font_printf(MENU_ORG_X,(MENU_ORG_Y+pmenu_cy)*8,">");
}


SceIoDirent pmenu_dent[64];
int pmenu_dent_pos = 0;
int pmenu_dent_len = 0;
int pmenu_dent_flag = 0;



void pmenu_read_dir(void)
{
	int fd;
	int idx,cnt;
	int flag;
	SceIoDirent dent;

	idx = 0;
	cnt = 0;
	flag = 1;
	
	fd = sceIoDopen(pmenu_curdir);
	
	if (fd >= 0)
	{
		while(cnt < pmenu_dent_pos && (flag = sceIoDread(fd,&dent) > 0))  
			cnt++;
		
		if (flag > 0) 
		{
			while(idx < 64 && (flag = sceIoDread(fd,&pmenu_dent[idx++])) > 0);
		}
		sceIoDclose(fd);
	}
	pmenu_dent_len = idx-1;
	pmenu_dent_flag = flag;
}

void pmenu_reset_dir(void)
{
	pmenu_cy = 0;
	pmenu_index = 0;
	pmenu_dent_pos = 0;
	pmenu_read_dir();
	pmenu_len = pmenu_dent_len;
}

void pmenu_restore_swkbd(void)
{
	set_blend_swkbd(1);
	sw_swkbd(0);
}

void pmenu_back(void)
{
	switch(pmenu_mode)
	{
		case MENU_MODE_GETKEY:
			pmenu_restore_swkbd();
		break;
	}
	pmenu_mode = MENU_MODE_NORMAL;
	pmenu_setmenu_ex(pmenu_lastmenu,pmenu_old_index,pmenu_old_cy);
	pmenu_old_cy = pmenu_old_index = 0;
	pmenu_lastmenu = PMENU_IDX_MAIN;
}

void pmenu_reset_getkey(void)
{
	int i = 0;
	if (pmenu_getkey_code)
		i = getkey_code_swkbd(*pmenu_getkey_code);
	
	if (i >= 0)
		setkey_swkbd(i);
		
	set_blend_swkbd(0);
	sw_swkbd(1);
}

void pmenu_reset_image(void)
{
	pmenu_cy = 0;
	pmenu_index = 0;
	pmenu_len = ext_getdiskimgs(pmenu_curdrive);
}

void pmenu_exec_cmd(int value)
{
	switch(value)
	{
		case MCMD_QUIT:
			ext_quit();
		break;
		case MCMD_RESET:
			ext_reset();
			if (pmenu_reset_start)			
				pmenu_sw = 0;
		break;
		case MCMD_SAVESETTING:
			ext_save_setting();
		break;
		case MCMD_SAVESTATE:
			ext_save_state();
		break;
		case MCMD_LOADSTATE:
			ext_load_state();
		break;
		case MCMD_SETCLOCK:
			ext_set_cpufreq();
			pop_about_form("SET CLOCK : %d",ext_get_cpufreq());
		break;
	}
}

void pmenu_select_menu(void)
{
	int idx;
	t_menucont *ptr;
	
	idx = pmenu_cy + pmenu_index;
	ptr = pmenu_current->cont;
	
	switch(ptr[idx].cmd)
	{
		case MENU_CMDEXE:
			pmenu_exec_cmd(ptr[idx].value);
		break;
		case MENU_CHECK:
			*(int *)ptr[idx].ptr = !*(int *)ptr[idx].ptr; 
		break;
		case MENU_RADIO:
			*(int *)ptr[idx].ptr = ptr[idx].value; 
		break;
		case MENU_DBL_RADIO:
			*(double *)ptr[idx].ptr = (double)ptr[idx].value/1000000; 
		break;
		case MENU_JUMP:
			pmenu_setmenu(ptr[idx].value);
		break;
		case MENU_BACK:
			pmenu_back();
		break;
		case MENU_FILE:
			pmenu_curstr = (char *)ptr[idx].ptr;
			pmenu_mode = MENU_MODE_FILE;
			pmenu_curdrive = ptr[idx].value;
			pmenu_save_menupos();
			pmenu_reset_dir();
		break;
		case MENU_IMAGE_SEL:
			pmenu_mode = MENU_MODE_IMAGESEL;
			pmenu_curdrive = ptr[idx].value;
			pmenu_save_menupos();
			pmenu_reset_image();
		break;
		case MENU_GETKEY:
			pmenu_mode = MENU_MODE_GETKEY;
			pmenu_getkey_code = (int *)ptr[idx].ptr;
			pmenu_save_menupos();
			pmenu_reset_getkey();
		break;
		case MENU_ABOUT:
			pmenu_save_menupos();
			pmenu_mode = MENU_MODE_ABOUT;
		break;
		case MENU_EXIT:
			pmenu_set_status(0);
		break;
	}
}

void pmenu_select_disk(void)
{
	int idx;
	
	idx = pmenu_cy + pmenu_index;
	if (pmenu_dent[idx].d_stat.st_mode & FIO_S_IFDIR)
	{
			if (strcmp(pmenu_dent[idx].d_name,".")) // if not curdir
			{
				
				if (!strcmp(pmenu_dent[idx].d_name,".."))
					pmenu_dirup();
				else
					pmenu_dirdown(pmenu_dent[idx].d_name);
			}

			pmenu_reset_dir();
	} else
	{
		strcpy(pmenu_curstr,pmenu_curdir);
		strcat(pmenu_curstr,"/");
		strcat(pmenu_curstr,pmenu_dent[idx].d_name);
		
		ext_opendisk(pmenu_curdrive,pmenu_curstr);
		
		pmenu_make_diskname(pmenu_dskfile[pmenu_curdrive],pmenu_curstr);

		pmenu_set_current_image_name(pmenu_curdrive);
		
		pmenu_back();
	}
}

void pmenu_select_key(void)
{
	if (pmenu_getkey_code)
		*pmenu_getkey_code = getcode_swkbd();
	pmenu_back();
}

void pmenu_select_about(void)
{
	pmenu_back();
}

void pmenu_select_image(void)
{
	int idx;
	idx = pmenu_cy + pmenu_index;
	
	if (pmenu_curdrive > 0)
	{
		pmenu_drive1_img = idx-1;
	} else
		pmenu_drive0_img = idx-1;
	
	ext_setdrvimage(pmenu_curdrive,idx);

	pmenu_set_image(pmenu_curdrive,idx);

	

	pmenu_back();
}


void pmenu_select(void)
{
	switch(pmenu_mode)
	{
	case MENU_MODE_ABOUT:
		pmenu_select_about();
	break;
	case MENU_MODE_FILE:
		pmenu_select_disk();
	break;
	case MENU_MODE_IMAGESEL:
		pmenu_select_image();
	break;
	case MENU_MODE_GETKEY:
		pmenu_select_key();
	break;
	default:
		pmenu_select_menu();
	}
}

void pmenu_disp_getkey(void)
{
	int x;
	
	x = MENU_ORG_X+8;

	font_printf(x,6*8,"-- KEY CONFIG --");
}


void pmenu_disp_file(void)
{
	int i;
	int x,y;
	
	x = MENU_ORG_X+8;

	font_printf(x,6*8,"-- SELECT A FILE --");
		
	for(y=MENU_ORG_Y,i=pmenu_index; i < pmenu_len && i<pmenu_index+MENU_SIZE; i++,y++)
	{
		font_printf(x,y*8,"%s",pmenu_dent[i].d_name);
	}
}

void pmenu_disp_about(void)
{
	int i;
	int x,y;
	
	x = MENU_ORG_X+8;

	font_printf(x,6*8,"-- ABOUT QUASI88 for PSP --");
		
	for(y=MENU_ORG_Y,i=0; pmenu_about[i]; i++,y++)
	{
		font_printf(x,y*8,"%s",pmenu_about[i]);
	}
}

void pmenu_disp_image(void)
{
	int i;
	int x,y;
	
	x = MENU_ORG_X+8;

	font_printf(x,6*8,"-- SELECT AN IMAGE --");
		
	for(y=MENU_ORG_Y,i=pmenu_index; i < pmenu_len && i<pmenu_index+MENU_SIZE; i++,y++)
	{
		font_printf(x,y*8,"%d : %s",i,ext_getimgname(pmenu_curdrive,i));
	}
}

void pmenu_disp(void)
{
	
	if (!pmenu_sw)
		return;
		
	video_start();
	video_tex(0);
	video_blend(1);
	video_blend_mix(MENU_BLEND);
	video_blit_clr(0,0,300,200,60,40,MAKE_RGB(0x00,0x00,0x07));
	video_tex(1);
	video_end();

	switch(pmenu_mode)
	{
		case MENU_MODE_ABOUT:
			pmenu_disp_about();
		break;
		case MENU_MODE_GETKEY:
			pmenu_disp_getkey();
			disp_swkbd();
		break;
		case MENU_MODE_FILE:
			pmenu_disp_file();
			pmenu_disp_cur();
		break;
		case MENU_MODE_IMAGESEL:
			pmenu_disp_image();
			pmenu_disp_cur();
		break;
		default:
			pmenu_disp_list();
			pmenu_disp_cur();
		break;
	}
}


int pmenu_oldinput = 0;
int pmenu_repcnt = 15;

// set current values in the emulator to menu values

void pmenu_start_menu(void)
{
	ext_get_sysval();

	pmenu_make_diskname(pmenu_disk0_file,pmenu_disk0);
	pmenu_make_diskname(pmenu_disk1_file,pmenu_disk1);

	pmenu_drive0_img = ext_get_curimage(0);
	pmenu_drive1_img = ext_get_curimage(1);
	
	pmenu_drive0_imgname[0] =
	pmenu_drive1_imgname[0] = 0;
	
	if (pmenu_drive0_img >= 0)
		strcpy(pmenu_drive0_imgname,ext_getimgname(0,pmenu_drive0_img+1));
	
	if (pmenu_drive1_img >= 0)
		strcpy(pmenu_drive1_imgname,ext_getimgname(1,pmenu_drive1_img+1));
	
}

void pmenu_exit_menu(void)
{
	cpu_clock_mhz = (double)pmenu_cpuclk_int / 1000000;
	sound_clock_mhz = (double)pmenu_sndclk_int / 1000000;
}

int pmenu_status(void)
{
	return pmenu_sw;
} 

void pmenu_set_status(int val)
{
	pmenu_sw = val;
	if (pmenu_sw) 
		pmenu_start_menu();
	else
		pmenu_exit_menu();
} 

void pmenu_set_disk(int drv,char *path,int img)
{
	ext_opendisk(drv,path);
	ext_setdrvimage(drv,img + 1);
	pmenu_make_diskname(pmenu_dskfile[drv],path);
	pmenu_set_current_image_name(drv);
}

void pmenu_input(int pad)
{
	int mask;

	mask = pmenu_oldinput ^ pad;
	if (!mask && pmenu_repcnt)
	{
		if (pmenu_repcnt > 0) pmenu_repcnt--;
		return;
	}
	if (pmenu_oldinput)
		pmenu_repcnt = 5;
	else
		pmenu_repcnt = 15;
			
		
	pmenu_oldinput = pad;
	
	if (mask)
		pad&=mask;
	
	if (pmenu_sw)
	{
		if (pad & PSP_CTRL_UP)
		{
			if (pmenu_cy > 0)
			{
				pmenu_cy--;
			}
			else
			{
				if (pmenu_index > 0)
				pmenu_index--;

			}
		}
		if (pad & PSP_CTRL_DOWN)
		{
			if (pmenu_cy < pmenu_len-1 && pmenu_cy < (MENU_SIZE-1))
			{
					pmenu_cy++;
			} else
			{
				if (pmenu_index < (pmenu_len-1-pmenu_cy))
					pmenu_index++;
			}
		}
		if (pad & PSP_CTRL_CIRCLE)
			pmenu_select();
		if (pad & PSP_CTRL_SQUARE)
			pmenu_back();

		if (pad & PSP_CTRL_SELECT)
			pop_about("SELECT");
			
	}

	if (!pmenu_rev_menusw && pad & PSP_CTRL_RTRIGGER)
			pmenu_set_status(pmenu_sw ^ 1);
	if (pmenu_rev_menusw && pad & PSP_CTRL_LTRIGGER)
			pmenu_set_status(pmenu_sw ^ 1);
}

