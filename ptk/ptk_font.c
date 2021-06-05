#include <pspctrl.h>
#include <pspgu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ptk_video.h"

#define BUF_WIDTH  (512)
#define BUF_HEIGHT (128)

typedef unsigned char bit8;

#include "font.h"

static unsigned short __attribute__((aligned(16))) texfont[BUF_WIDTH * BUF_HEIGHT];

static int font_blendsw = 1;
static int font_clr_fg = 0x7fff;
static int font_clr_bg = 0x0000;

static void font_decode(int x,int y,int code)
{
	int i,j;
	char data;
	
	for(j=0; j < 8; j++)
	{
		data = built_in_font_ANK[(code*8)+j];

		for(i=x; i < x+8; i++)
		{
			if (data & 0x80)
				texfont[((y+j) * BUF_WIDTH) + i] = font_clr_fg;
			else	
				texfont[((y+j) * BUF_WIDTH) + i] = font_clr_bg;
			data <<= 1;
		}
	}
}

void font_init(void)
{
	int i;
	
	for(i = 0; i < 256; i++)
	{
		font_decode((i&0x3f)<<3,i>>3,i);
	}
}

void font_set_blend(int sw)
{
	font_blendsw = sw;
}

void font_blend(void)
{
	if (!font_blendsw)
		video_blend(0);
	else 
	{
		video_blend(1);
		sceGuBlendFunc(GU_MAX, GU_SRC_COLOR, GU_FIX,0,0);
	}
}


void font_putchar(int x,int y,int code)
{
	video_start();
	font_blend();
	video_settex(texfont);

	video_blit_norm((code&0x3f)<<3,code >> 3,8,8,x,y);
	video_end();
}

void font_puts(int x,int y,char *str)
{
	unsigned char code;
	video_start();
	font_blend();
	video_settex(texfont);

	while(*str)
	{
		code = *(str++);
		video_blit_norm((code&0x3f)<<3,code >> 3,8,8,x,y);
		x+=8;
	}
	video_end();
}

void font_printf(int x,int y,char *format,...)
{
	va_list args;

	char buf[256];

	va_start(args,format);
	vsprintf(buf,format,args);
	va_end(args);
	font_puts(x,y,buf);
} 


