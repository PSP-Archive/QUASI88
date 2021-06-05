#ifndef _PTKFONT_H_
#define _PTKFONT_H_

void font_init(void);
void font_putchar(int x,int y,int code);
void font_puts(int x,int y,char *str);
void font_printf(int x,int y,char *format,...);

#endif

