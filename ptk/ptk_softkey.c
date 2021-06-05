#include <pspgu.h>
#include <pspctrl.h>

typedef unsigned char byte;

#include "keyboard.h"
#include "ptk_video.h"

#define BUF_WIDTH  (512)
#define BUF_HEIGHT (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

#include "ptk_menu.h"
#include "ptk_softkey.h"
#include "keytbl_sw.h"

static unsigned short __attribute__((aligned(16))) kbdtex[BUF_WIDTH * SCR_HEIGHT];

extern unsigned char swkbd_bmp[];

int kbd_blend = 0;
int kbd_sw = 0;
int kbd_cur = 0;
int kbd_click = 0;
int kbd_blend_sw = 0;
int kbd_tblsize;
int kbd_relcode_index = 0;

int kbd_btn_cro = 0;
int kbd_btn_tri = 0;
int kbd_btn_squ = 0;

int kbd_chg_cro = 0;
int kbd_chg_tri = 0;
int kbd_chg_squ = 0;



#define KBD_OK 0x80
#define KBD_LOCK 0x01
#define KBD_UNLOCK 0x02

int kbd_lock_data[16];
int kbd_lock_state[16];

#define KBD_BL_DELTA (0x101010)

void video_kbdcopy(unsigned char *data,int x,int y,int w,int h)
{
	int i,j;
	int clr;
	for(j=y; j < y+h; j++)
	{
		for(i=x; i < x+w; i++)
		{
			clr = MAKE_RGB(data[0],data[1],data[2]);
			kbdtex[( j * BUF_WIDTH ) + i]=clr;
			data+=3;
		}
	}
}

void reset_swkbd(void)
{
	int i;
	for(i=0; i < 16; i++)
	{
		kbd_lock_data[i] = 0;
		kbd_lock_state[i] =KBD_UNLOCK | KBD_OK;
	}
}

int is_lock_swkbd(int code)
{
	int i;
	
	for(i=0; i < 16; i++)
	{
		if ( (kbd_lock_data[i] == code) )
		{
			if ( (kbd_lock_state[i] & KBD_LOCK) )
				return 1;
		}
	}
	return 0;
}

int change_swkbd(int code)
{
	int i;
	
	for(i=0; i < 16; i++)
	{
		if ( (kbd_lock_data[i] == code) )
		{
			if ( (kbd_lock_state[i] & KBD_LOCK) )
				kbd_lock_state[i] = KBD_UNLOCK;
			else
				kbd_lock_state[i] = KBD_LOCK;

			return 1;
		}
	}

	for(i=0; i < 16; i++)
	{
		if ((kbd_lock_state[i] & KBD_UNLOCK)
			&& (kbd_lock_state[i] & KBD_OK))
		{
			kbd_lock_state[i] = KBD_LOCK;
			kbd_lock_data[i]  = code;
			return 1;
		}
	}
	return -1;
}

int set_ok_swkbd(int code)
{
	int i;
	for(i=0; i < 16; i++)
	{
		if (kbd_lock_data[i] == code)
		{
			kbd_lock_state[i] |= KBD_OK;
			return 0;
		}
	}
	return -1;
}

int need_lock_swkbd(void)
{
	int i;

	for(i=0; i < 16; i++)
	{
		if ((kbd_lock_state[i] & (KBD_LOCK | KBD_OK)) == KBD_LOCK)
		{
			return kbd_lock_data[i];
		}
	}
	return -1;
}

int need_unlock_swkbd(void)
{
	int i;

	for(i=0; i < 16; i++)
	{
		if ((kbd_lock_state[i] & (KBD_UNLOCK | KBD_OK)) == KBD_UNLOCK)
		{
			return kbd_lock_data[i];
		}
	}
	return -1;
}

int change_curcode_swkbd(void)
{
	if (kbd_cur >= 0 && kbd_cur < kbd_tblsize)
	{
		return change_swkbd(swkey_tbl[kbd_cur].scancode);
	}
	return -1;
}


int getkey_swkbd(void)
{
	return kbd_cur;
}

void setkey_swkbd(int val)
{
	if (val < kbd_tblsize)
		kbd_cur = val;
	else
		kbd_cur = 0;
}

char *getcurname_swkbd(void)
{
	if (kbd_cur < kbd_tblsize) 
		return swkey_tbl[kbd_cur].name;
	return "";
}

char *getname_swkbd(int val)
{
	if (val < kbd_tblsize) 
		return swkey_tbl[val].name;
	return "";
}

// false = no release code
int is_release_swkbd(void)
{
	if (kbd_relcode_index < 0)
		return 0;
	return 1;
}

// -1 = doesn't have release key code
int getrelcode_swkbd(void)
{
	int code = swkey_tbl[kbd_relcode_index].scancode;
	kbd_relcode_index = -1;
	return code;
}

int is_change_swkbd(void)
{
	if (kbd_relcode_index < 0)
		return 0;

	if (kbd_relcode_index == kbd_cur && kbd_click)
		return 0;
	
	return 1;
}

// -1 = error or still have a release key code
int getcode_swkbd(void)
{
	if (kbd_relcode_index >= 0)
		return -1;
	
	if (kbd_cur < kbd_tblsize)
	{
		kbd_relcode_index = kbd_cur;
		return swkey_tbl[kbd_cur].scancode;
	}
	return -1;
}

int is_press_swkbd(void)
{
	return kbd_click;
}

int is_press_squ_swkbd(void)
{
	return kbd_btn_squ;
}

int is_press_tri_swkbd(void)
{
	return kbd_btn_tri;
}

int is_press_cro_swkbd(void)
{
	return kbd_btn_cro;
}

int is_change_squ_swkbd(void)
{
	return kbd_chg_squ;
}

int is_change_tri_swkbd(void)
{
	return kbd_chg_tri;
}

int is_change_cro_swkbd(void)
{
	return kbd_chg_cro;
}

int read_squ_swkbd(void)
{
	kbd_chg_squ = 0;
	return kbd_btn_squ;
}

int read_tri_swkbd(void)
{
	kbd_chg_tri = 0;
	return kbd_btn_tri;
}

int read_cro_swkbd(void)
{
	kbd_chg_cro = 0;
	return kbd_btn_cro;
}

void change_tri_swkbd(int sw)
{
	kbd_chg_tri = 1;
	kbd_btn_tri = sw;
}

void change_cro_swkbd(int sw)
{
	kbd_chg_cro = 1;
	kbd_btn_cro = sw;
}

void change_squ_swkbd(int sw)
{
	kbd_chg_squ = 1;
	kbd_btn_squ = sw;
}



int status_swkbd(void)
{
	return kbd_sw;
}

int sw_swkbd(int sw)
{
	kbd_sw = sw;
	return kbd_sw;
}

void set_blend_swkbd(int sw)
{
	kbd_blend_sw = sw;
}
int get_blend_swkbd(void)
{
	return kbd_blend_sw;
}

int getkey_code_swkbd(int keycode)
{
	int i;
	for(i=0; swkey_tbl[i].name; i++)
	{
		if (swkey_tbl[i].scancode == keycode)
			return i; 
	}
	return 0;
}

void init_swkbd(void)
{
	kbd_cur = 0;
	kbd_blend = 0;
	kbd_sw = 0;
	kbd_click = 0;
	kbd_blend_sw = 1;
	kbd_relcode_index = -1;
	
	kbd_tblsize = sizeof(swkey_tbl)/sizeof(t_swkey);

	reset_swkbd();

	video_box_tex16(kbdtex,0,100,64,32,MAKE_RGB(0x1f,0x1f,0x00));
	video_box_tex16(kbdtex,0,132,64,32,MAKE_RGB(0x1f,0x00,0x00));

	video_kbdcopy(swkbd_bmp,0,0,400,100);	

}

#define RGB_BLEND_HALF (0x808080)
#define RGB_BLEND_MAX  (0xffffff)

void disp_swkbd(void)
{
	int kbd_x = (SCR_WIDTH-400)/2;
	int cx,cy,cw,ch;
	int i,j;
	
	cx = swkey_tbl[kbd_cur].x;
	cy = swkey_tbl[kbd_cur].y;
	cw = swkey_tbl[kbd_cur].w;
	ch = swkey_tbl[kbd_cur].h;
	
	if (kbd_sw) 
	{
		if (kbd_blend + KBD_BL_DELTA < RGB_BLEND_HALF)
			kbd_blend += KBD_BL_DELTA;
		else
			kbd_blend = RGB_BLEND_HALF;
	}
	else
	{
		if (kbd_blend - KBD_BL_DELTA > 0x00)
			kbd_blend -= KBD_BL_DELTA;
		else
			kbd_blend = 0x00;
	}
	if (kbd_blend > 0)
	{
		video_start();
		video_settex(kbdtex);
		if (kbd_blend_sw)
		{
			video_blend(1);
			sceGuBlendFunc(GU_ADD, GU_FIX,GU_FIX,kbd_blend,RGB_BLEND_MAX-kbd_blend);
		} else
			video_blend(0);
		
		video_blit(0,0,400,100,kbd_x,120);
		video_blit(0,kbd_click ? 132 : 100,cw,ch,kbd_x+cx,120+cy);
		for(i=0; i < 16; i++)
		{
			j = kbd_lock_data[i];
			if (kbd_lock_state[i] & KBD_LOCK)
			{
				j = getkey_code_swkbd(j);
				cx = swkey_tbl[j].x;
				cy = swkey_tbl[j].y;
				cw = swkey_tbl[j].w;
				ch = swkey_tbl[j].h;
				video_blit(0,132,cw,ch,kbd_x+cx,120+cy);
			}
		}

		video_blend(0);
		video_end();
	}
}



int kbd_oldinput = 0;
int kbd_repcnt = 15;

int kbd_cur_akey = 0;

int kbd_chg_akey = 0;

int kbd_stat_akey = 0;

int state_akey_swkbd(void)
{
	return kbd_chg_akey;
}

int current_akey_swkbd(void)
{
	return kbd_cur_akey;
}

void set_stat_akey_swkbd(int mask)
{
	kbd_stat_akey = (kbd_stat_akey & (~mask)) | (kbd_cur_akey & mask);
}

void input_akey_swkbd(int x,int y)
{
	int kbd_akey;
	
	kbd_akey = 0;
	
	if (x > 64 && x < 192)
		kbd_akey &= (KBD_ANA_UP | KBD_ANA_DOWN);
	else 
	{
		if (x < 64)
			kbd_akey |= KBD_ANA_LEFT;
		if (x > 192)
			kbd_akey |= KBD_ANA_RIGHT;
	}

	if (y > 64 && y < 192)
		kbd_akey &= (KBD_ANA_LEFT | KBD_ANA_RIGHT);
	else 
	{
		if (y < 64)
			kbd_akey |= KBD_ANA_UP;
		if (y > 192)
			kbd_akey |= KBD_ANA_DOWN;
	}
	
	kbd_chg_akey = kbd_akey ^ kbd_stat_akey;
	kbd_cur_akey = kbd_akey;
}


void input_swkbd(int pad)
{
	int mask;
	int orgpad;
	
	orgpad = pad;
	mask = kbd_oldinput ^ pad;
	if (!mask && kbd_repcnt)
	{
		if (kbd_repcnt > 0) kbd_repcnt--;
		return;
	}
	if (kbd_oldinput)
		kbd_repcnt = 5;
	else
		kbd_repcnt = 15;
			
		
	kbd_oldinput = pad;
	
	if (mask)
		pad&=mask;
	
	if (kbd_sw)
	{
		
		if (pad & PSP_CTRL_LEFT)
		{
			if (swkey_tbl[kbd_cur].l >= 0)
				kbd_cur = swkey_tbl[kbd_cur].l;
		}
		if (pad & PSP_CTRL_RIGHT)
		{
			if (swkey_tbl[kbd_cur].r >= 0)
				kbd_cur = swkey_tbl[kbd_cur].r;
		}
		if (pad & PSP_CTRL_UP)
		{
			if (swkey_tbl[kbd_cur].u >= 0)
				kbd_cur = swkey_tbl[kbd_cur].u;
		}
		if (pad & PSP_CTRL_DOWN)
		{
			if (swkey_tbl[kbd_cur].d >= 0)
				kbd_cur = swkey_tbl[kbd_cur].d;
		}
		
		if (pad & PSP_CTRL_CIRCLE)
			kbd_click = 1;
		else
			kbd_click = 0;
		
		if (pad & PSP_CTRL_CROSS)
			change_cro_swkbd(1);
		else
			change_cro_swkbd(0);
		
		if (pad & PSP_CTRL_SQUARE)
			change_squ_swkbd(1);
		else
			change_squ_swkbd(0);
			
		if (orgpad & PSP_CTRL_TRIANGLE)
			change_tri_swkbd(1);
		else
			change_tri_swkbd(0);
	}

	if (!pmenu_rev_menusw && (pad & PSP_CTRL_LTRIGGER))
			kbd_sw ^= 1;
	if (pmenu_rev_menusw && (pad & PSP_CTRL_RTRIGGER))
			kbd_sw ^= 1;

}


