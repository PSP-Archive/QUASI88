#ifndef __PTKSOFTKEY_H_
#define __PTKSOFTKEY_H_

#define KBD_ANA_LEFT 1
#define KBD_ANA_RIGHT 2
#define KBD_ANA_UP 4
#define KBD_ANA_DOWN 8

void  init_swkbd(void);
void  disp_swkbd(void);
int   status_swkbd(void);
int  sw_swkbd(int sw);

int is_press_swkbd(void);
int is_release_swkbd(void);
int is_change_swkbd(void);

int is_press_squ_swkbd(void);
int is_press_tri_swkbd(void);
int is_press_cro_swkbd(void);

int is_change_squ_swkbd(void);
int is_change_tri_swkbd(void);
int is_change_cro_swkbd(void);

int read_squ_swkbd(void);
int read_tri_swkbd(void);
int read_cro_swkbd(void);

int state_akey_swkbd(void);
int current_akey_swkbd(void);
void set_stat_akey_swkbd(int mask);
void input_akey_swkbd(int x,int y);



int change_swkbd(int code);
int set_ok_swkbd(int code);
int need_lock_swkbd(void);
int need_unlock_swkbd(void);
int change_curcode_swkbd(void);
int is_lock_swkbd(int code);


void  input_swkbd(int pad);
int   getkey_swkbd(void);
void  setkey_swkbd(int val);
char *getcurname_swkbd(void);
char *getname_swkbd(int val);

int  getrelcode_swkbd(void);
int  getcode_swkbd(void);
int  getkey_code_swkbd(int keycode);
void set_blend_swkbd(int sw);
int  get_blend_swkbd(void);


#endif
