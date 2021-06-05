#ifndef _PTKVIDEO_H_
#define _PTKVIDEO_H_

#define RGB_5551MAX  (0x1f)
#define RGB_5551HALF (0x10)
#define RGB_CURMAX RGB_5551MAX
#define RGB_CURHALF RGB_5551HALF

#define RGB_WORD_VRAMSIZE 0x44000
#define RGB_LONG_VRAMSIZE 0x88000

#define BUF_WIDTH  (512)
#define BUF_HEIGHT (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

#define MAKE_RGB32(r,g,b) ((r)&0xff)|(((g)&0xff)<<8)|(((b)&0xff)<<16)
#define MAKE_RGB(r,g,b) ((r)&0x1f)|(((g)&0x1f)<<5)|(((b)&0x1f)<<10)

#define MAKE_CURRGB(r,g,b) MAKE_RGB((r),(g),(b))

void video_init(void);
void video_pixfill(void);
void video_pixclear(void);


void video_clear(void);
void video_box(int x,int y,int w,int h,int clr);
void video_boxclr(int clr);
void video_putimage(void);
void video_putimage_full(int x,int y,int w,int h);
void video_putimage_truewide(int x,int y,int w,int h);
void *video_getteximage(void);


void video_blit(int x,int y,int w,int h,int dx,int dy);
void video_blit_norm(int x,int y,int w,int h,int dx,int dy);
void video_blit_clr(int x,int y,int w,int h,int dx,int dy,int clr);
void video_start(void);
void video_on(void);
void video_blend(int on);
void video_blend_mix(int color);
void video_tex(int on);
void video_settex(void *tex);
void video_end(void);
void video_box_tex16(unsigned short *texbuf,int x,int y,int w,int h,int clr);
void video_swapbuffer(int wait);

#endif
