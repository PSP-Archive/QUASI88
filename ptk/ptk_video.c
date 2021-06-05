
// pspvideo

#include <pspgu.h>
#include <pspdisplay.h>

#include "ptk_video.h"

static unsigned int __attribute__((aligned(16))) list[262144];
static unsigned short __attribute__((aligned(16))) pixels[BUF_WIDTH * SCR_HEIGHT];
static unsigned short __attribute__((aligned(16))) boxtex[16 * 16];

typedef struct
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
} t_vertex;

#define SLICE 32

void video_blit(int x,int y,int w,int h,int dx,int dy)
{
	int width;
	int start,end;
	
	for(start = x,end = x+w; start < end; start += SLICE,dx+=SLICE)
	{
		if (start + SLICE < end) 
			width = SLICE;
		else
			width = end - start;
			
		t_vertex *vert= (t_vertex*)sceGuGetMemory(2 * sizeof(t_vertex));
	
		vert[0].u = start; vert[0].v = y;
		vert[0].color = 0;
		vert[0].x = dx; vert[0].y = dy; vert[0].z = 0;
		
		vert[1].u = start + width; vert[1].v = y+h;
		vert[1].color = 0;
		vert[1].x = dx + width; vert[1].y = dy+h; vert[1].z = 0;

//		sceGuDrawArray(GU_SPRITES,GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vert);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vert);
	}
}

// dx= 22 ,dw = 435 , actual width = 430 = 43*10
void video_blit_full(int x,int y,int w,int h,int dx,int dy,int dw,int dh)
{
	int vwidth,vstep;
	int vend;
	
	int width;
	int vram_width;
	int start,end;
	
	vstep = dw / (w / SLICE); // 43
	vend = dx + dw;
	
	for(start = x,end = x+w; start < end; start += SLICE,dx += vstep)
	{
		if (dx + vstep < vend)
			vwidth = vstep;
		else
			vwidth = vend - dx;
		
		if (start + SLICE < end) 
			width = SLICE;
		else
			width = end - start;
			
		t_vertex *vert= (t_vertex*)sceGuGetMemory(2 * sizeof(t_vertex));
	
		vert[0].u = start; vert[0].v = y;
		vert[0].color = 0;
		vert[0].x = dx; vert[0].y = dy; vert[0].z = 0;
		
		vert[1].u = start + width; vert[1].v = y+h;
		vert[1].color = 0;
		vert[1].x = dx + vwidth; vert[1].y = dy+dh; vert[1].z = 0;

//		sceGuDrawArray(GU_SPRITES,GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vert);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vert);
	}
}


void video_blit_norm(int x,int y,int w,int h,int dx,int dy)
{
	
	t_vertex *vert= (t_vertex*)sceGuGetMemory(2 * sizeof(t_vertex));
	
	vert[0].u = x; vert[0].v = y;
	vert[0].color = 0;
	vert[0].x = dx; vert[0].y = dy; vert[0].z = 0;
		
	vert[1].u = x+w; vert[1].v = y+h;
	vert[1].color = 0;
	vert[1].x = dx+ w; vert[1].y = dy+h; vert[1].z = 0;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vert);
}

void video_blit_clr(int x,int y,int w,int h,int dx,int dy,int clr)
{
	
	t_vertex *vert= (t_vertex*)sceGuGetMemory(2 * sizeof(t_vertex));
	
	vert[0].u = x; vert[0].v = y;
	vert[0].color = clr;
	vert[0].x = dx; vert[0].y = dy; vert[0].z = 0;
		
	vert[1].u = x+w; vert[1].v = y+h;
	vert[1].color = clr;
	vert[1].x = dx+ w; vert[1].y = dy+h; vert[1].z = 0;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_5551|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vert);
}

void video_init(void)
{
	sceGuInit();

	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_5551,(void*)0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)RGB_WORD_VRAMSIZE,BUF_WIDTH);
	sceGuDepthBuffer((void*)RGB_WORD_VRAMSIZE+RGB_WORD_VRAMSIZE,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);

	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);
}

void video_blend(int on)
{
	if (on)
		sceGuEnable(GU_BLEND);
	else
		sceGuDisable(GU_BLEND);
}

void video_blend_mix(int color)
{
	sceGuBlendFunc(GU_ADD, GU_FIX,GU_FIX,color,0xFFFFFF - color);
}


void video_tex(int on)
{
	if (on)
		sceGuEnable(GU_TEXTURE_2D);
	else
		sceGuDisable(GU_TEXTURE_2D);
}

void video_on(void)
{
	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
}

void video_start(void)
{
		sceGuStart(GU_DIRECT,list);
}

void video_clear(void)
{
		video_start();
		video_tex(0);
		video_blend(0);
		video_blit_clr(0,0,SCR_WIDTH,SCR_HEIGHT,0,0,0x0000);
		video_tex(1);
		video_end();
}

void video_side_clear(void)
{
		video_start();
		video_tex(0);
		video_blend(0);
		video_blit_clr(0,0,23,SCR_HEIGHT,0,0,0x0000);
		video_blit_clr(0,0,30,SCR_HEIGHT,450,0,0x0000); // actual W = 430
		video_tex(1);
		video_end();
}

void video_settex(void *tex)
{
		sceGuTexMode(GU_PSM_5551,0,0,0);
		sceGuTexImage(0,512,512,512,tex);
		sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
		sceGuTexFilter(GU_NEAREST,GU_NEAREST);
}

void video_setboxtex()
{
		sceGuTexMode(GU_PSM_5551,0,0,0);
		sceGuTexImage(0,16,16,16,boxtex);
		sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
		sceGuTexFilter(GU_NEAREST,GU_NEAREST);
}

void video_end(void)
{
		sceGuFinish();
		sceGuSync(0,0);
}

void video_pixfill(void)
{
	int i;
	for(i=0; i < BUF_WIDTH * SCR_HEIGHT; i++)
		pixels[i]=i;
}

void video_pixclear(void)
{
	int i;
	for(i=0; i < BUF_WIDTH * SCR_HEIGHT; i++)
		pixels[i]=0;
}

void video_box(int x,int y,int w,int h,int clr)
{
	int i,j;
	for(j=y; j < y+h; j++)
	{
		for(i=x; i < x+w; i++)
			pixels[( j * BUF_WIDTH ) + i]=clr;
	}
}
void video_box_tex16(unsigned short *texbuf,int x,int y,int w,int h,int clr)
{
	int i,j;
	for(j=y; j < y+h; j++)
	{
		for(i=x; i < x+w; i++)
			texbuf[( j * BUF_WIDTH ) + i]=clr;
	}
}

void video_boxclr(int clr)
{
	int i;
	for(i=0; i < 16*16; i++)
	{
		boxtex[i] = clr;
	}
}

int make_clr(int r,int g,int b)
{
	r&=0x1f; g &= 0x1f; b &= 0x1f;
	return (b<<10)|(g<<5)|r;
}

void *video_getteximage(void)
{
	return pixels;
}

void video_putimage_full(int x,int y,int w,int h)
{
		video_start();
		video_tex(1);
		video_settex(pixels);
		video_blend(0);
		video_blit_full(x,y,w,h,0,0,SCR_WIDTH,SCR_HEIGHT);
		video_end();
}

void video_putimage_truewide(int x,int y,int w,int h)
{
// 435.2=435 x 272
		video_start();
		video_tex(1);
		video_settex(pixels);
		video_blend(0);
		video_blit_full(x,y,w,h,22,0,435,SCR_HEIGHT);
		video_end();
}



void video_putimage(void)
{
		video_start();
		video_tex(1);
		video_settex(pixels);
		video_blend(0);
		video_blit(0,0,SCR_WIDTH,SCR_HEIGHT,0,0);
		video_end();
}

void video_swapbuffer(int wait)
{
		unsigned char *fb0;
		fb0 = sceGuSwapBuffers();
		pspDebugScreenSetOffset((int)fb0);

		if (wait)
			sceDisplayWaitVblankStart();
}

