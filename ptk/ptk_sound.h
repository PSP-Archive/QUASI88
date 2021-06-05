#ifndef __PTK_SOUND_H_
#define __PTK_SOUND_H_


#define SNDBUF_SIZE_MAX 0x2000
#define SNDBUF_SIZE (sound_buf_size)
#define SNDBUF_SIZE_HALF (SNDBUF_SIZE/2)
#define SNDBUF_SIZE_MASK (SNDBUF_SIZE-1)

short *psound_get_buffer(void);
void   psound_init(void);
void   psound_switch(int value);
void   psound_setfps(int frames);
void   psound_setfreq(int freq);
void   psound_set_stereo(int value);

int    psound_gettotal(void);

unsigned  psound_get_read_pos(void);
unsigned  psound_needsamples(unsigned len);

extern unsigned sound_buf_size;
extern int psound_flag_stereo;


#endif
