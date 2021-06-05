#ifndef _PSPMENU_H_
#define _PSPMENU_H_


#define PMENU_KCTBL 32

extern int pmenu_kctbl[PMENU_KCTBL];
extern int pmenu_kc_aflag;

enum {
PMENU_UP,
PMENU_DW,
PMENU_LE,
PMENU_RI,
PMENU_AUP,
PMENU_ADW,
PMENU_ALE,
PMENU_ARI,
PMENU_A,
PMENU_B,
PMENU_C,
PMENU_D,
PMENU_E,
PMENU_F,
PMENU_NONE
} PMENU_KEYDEF;


void pmenu_init(const char *dir);
void pmenu_disp(void);
void pmenu_input(int pad);
int  pmenu_status(void);
void pmenu_set_status(int val);
void pmenu_set_disk(int drv,char *path,int img);

extern int pmenu_showfps;
extern int pmenu_fullscreen;
extern int pmenu_type_fs;
extern int pmenu_video_current;

extern int pmenu_bgrun;


extern int pmenu_menu_start;
extern char pmenu_disk0[512];
extern char pmenu_disk1[512];

extern int  pmenu_drive0_img;
extern int  pmenu_drive1_img;

extern int pmenu_reset_start;
extern int pmenu_rev_menusw;

extern int pmenu_cpuclk_int;
extern int pmenu_sndclk_int;

#endif

