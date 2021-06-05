/***********
 PSP POPUP
***********/

#include <pspgu.h>
#include <pspctrl.h>
#include <pspdebug.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "ptk_video.h"
#include "ptk_font.h"

#define POP_BLEND (0x808080)

int pop_oldinput = 0;
int pop_repcnt = 15;
int pop_sw = 0;
int pop_cnt = 0;
int pop_debug = 0;

char pop_msg[256];
char pop_dbgmsg[256];

void pop_init(void)
{
	pop_debug = 0;
	pop_cnt = 0;
	pop_oldinput = 0;
	pop_repcnt = 15;
	pop_sw = 0;
	pop_dbgmsg[0] = pop_msg[0] = 0;
}

void pop_switch(int sw)
{
	pop_sw = sw;
}

int pop_status(void)
{
	return pop_sw;
} 

void pop_click(void)
{
	pop_sw = 0;
}

void pop_debug_force(void)
{
	font_puts(0,SCR_HEIGHT-32,pop_dbgmsg);
	video_swapbuffer(0);
}

void pop_disp(void)
{
	if (pop_debug)
	{
		font_puts(0,SCR_HEIGHT-32,pop_dbgmsg);
	}

	if (!pop_sw)
		return;
		
	if (pop_cnt > 0)
		pop_cnt--;
	else
		pop_sw = 0;

	video_start();
	video_tex(0);
	video_blend(1);
	video_blend_mix(POP_BLEND);
	video_blit_clr(0,0,200,24,100,100,MAKE_RGB(0x00,0x00,0x00));
	video_tex(1);
	video_end();
	
	font_puts(108,108,pop_msg);
}

void pop_about(char *msg)
{
	strcpy(pop_msg,msg);
	pop_sw = 1;
	pop_cnt = 120;
}

void pop_about_form(char *msg,...)
{
	va_list args;

	char buf[512];

	va_start(args,msg);
	vsprintf(buf,msg,args);
	va_end(args);
	pop_about(buf);
} 

void pop_debugf(char *msg,...)
{
	va_list args;

	char buf[512];

	va_start(args,msg);
	vsprintf(buf,msg,args);
	va_end(args);

	strcpy(pop_dbgmsg,buf);
	pop_debug = 1;
}

void pop_debug_sw(int val)
{
	pop_debug = val;
} 


void pop_input(int pad)
{
	int mask;

	mask = pop_oldinput ^ pad;
	if (!mask && pop_repcnt)
	{
		if (pop_repcnt > 0) pop_repcnt--;
		return;
	}
	if (pop_oldinput)
		pop_repcnt = 5;
	else
		pop_repcnt = 15;
			
	pop_oldinput = pad;
	
	if (mask)
		pad&=mask;
	
	if (pop_sw)
	{
		if (pad & PSP_CTRL_SQUARE)
			pop_click();
	}
}



