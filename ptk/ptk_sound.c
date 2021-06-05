#include <pspkernel.h>
#include <pspaudiolib.h>
#include <pspaudio.h>
#include <psprtc.h>
#include <stdio.h>

#include "ptk_sound.h"

unsigned sound_buf_size = 0x2000;
unsigned sound_buf_readpos = 0;

int psound_flag_stereo = 0;
int psound_flag = 0;
int psound_fps = 0;
int psound_tps = 0;
int psound_totalcnt = 0;

u64 psound_lasttick;
u32 psound_tickperframe;

typedef struct 
{
 short l,r;
} t_sample;

short sound_buf_buffer[SNDBUF_SIZE_MAX];

void psound_callback(void* buf, unsigned int len, void *userdata) 
{
	int i,j;
	t_sample *sp = (t_sample *)buf;
	if (psound_flag)
	{
		if (psound_flag_stereo)
		{
			psound_totalcnt+=(len>>1);
			for(i=0; i < len; i++)
			{
				sp[i].l = 
				sound_buf_buffer[((i&0xfffe)+sound_buf_readpos) & SNDBUF_SIZE_MASK];

				sp[i].r = 
				sound_buf_buffer[( ((i&0xfffe)+1)+sound_buf_readpos) & SNDBUF_SIZE_MASK];
				
			}
			sound_buf_readpos = (sound_buf_readpos + len) & SNDBUF_SIZE_MASK;
		}
		else
		{
			psound_totalcnt+=(len>>1);
			for(i=0; i < len; i++)
			{
				sp[i].l = sp[i].r =
				sound_buf_buffer[((i>>1)+sound_buf_readpos) & SNDBUF_SIZE_MASK];
			}
			sound_buf_readpos = (sound_buf_readpos + (len>>1)) & SNDBUF_SIZE_MASK;
		}
	} else
	{
		for(i=0; i < len; i++)
			sp[i].l = sp[i].r = 0;
	}
}

void psound_reset_sync(void)
{
	sceRtcGetCurrentTick(&psound_lasttick);
	psound_tickperframe = sceRtcGetTickResolution() / 60;
}

void psound_clearpcm(void)
{
   int i;
   for(i=0; i < SNDBUF_SIZE_MAX; i++)
	sound_buf_buffer[i] = 0;
}

void psound_init(void)
{
  psound_clearpcm();
	
  sound_buf_readpos = SNDBUF_SIZE >> 1;
	
  pspAudioInit();
  pspAudioSetChannelCallback(0, psound_callback, NULL);
  psound_flag = 0;

  psound_reset_sync(); 
  psound_setfps(44100 / 60);
  psound_setfreq(44100);
}

unsigned psound_get_read_pos(void)
{
	return sound_buf_readpos;
}

short *psound_get_buffer(void)
{
	return sound_buf_buffer;
}

void psound_set_stereo(int value)
{
	psound_flag_stereo = value;
}

void psound_switch(int value)
{
	if (value)
	  psound_clearpcm();

	psound_flag = value;
}

void psound_setfps(int frames)
{
	psound_fps = frames;
}

void psound_setfreq(int freq)
{
	psound_tps = sceRtcGetTickResolution()/freq;
}

int psound_gettotal(void)
{
	return psound_totalcnt;
}

unsigned psound_needsamples(unsigned maxlen)
{
	u64 curtick;
	int i,rel;
	
	sceRtcGetCurrentTick(&curtick);

	rel = (curtick - psound_lasttick) / psound_tps;
	psound_lasttick = curtick;

	if (rel > 4096) 
	{
		rel = 4096;
	}
	if (maxlen > 0 && rel > maxlen) 
	{
		rel = maxlen;
	}
	return rel;
}
