#ifndef _PTKEXT_H_
#define _PTKEXT_H_

void  ext_quit(void);
void  ext_reset(void);
void  ext_save_setting(void);
void  ext_save_state(void);
void  ext_load_state(void);

int   ext_getdiskimgs(int drv);
char *ext_getimgname(int drv,int img);
void  ext_setdrvimage(int drv,int img);
int   ext_opendisk(int drv,char *name);

int   ext_get_curimage(int drv);

void ext_get_sysval(void);
void ext_set_sysval(void);

void ext_save_deffreq(void);
void ext_set_cpufreq(void);
void ext_back_cpufreq(void);
int  ext_get_cpufreq(void);

extern int ext_boot_dipsw;
extern int ext_boot_from_rom;
extern int ext_boot_basic;
extern int ext_boot_clock_4mhz;
extern int ext_set_version;
extern unsigned char ext_baudrate_sw;
extern int ext_sound_board;
extern int ext_sound_buf_size;

extern int ext_cpuclock;
extern int ext_s1mode;

#endif

