/************************************************************************/
/*									*/
/* ������ɤν��� (�������顢XMAME�δؿ���ƤӽФ�)			*/
/*									*/
/*	���٤ơ�XMAME �ν�����Ԥʤ��ؿ�				*/
/*	QUASI88 �ϡ������δؿ����̤��ơ�XMAME�Υ�����ɽ�����Ԥʤ�	*/
/*									*/
/*	�ե������Ⱦ�ϡ�MAME ���ƤӽФ� osd_xxx �ؿ������Ҥ��줤�Ƥ롣	*/
/*	���� osd_xxx �� XMAME �Υ��������¤���ƺ������줿��		*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mame-quasi88.h"

#define  SNDDRV_WORK_DEFINE
#include "snddrv-psp.h"
#include "ptk_sound.h"
#include "ptk_popup.h"

#ifdef	USE_SOUND

#include "fm.h"			/* YM2203TimerOver()	*/

/*----------------------------------------------------------------------*/
/* ���ץ����ϡ����������椹�� */

int	attenuation = 0;	/* �ܥ�塼�� -32��0 [db] ����̤���ݡ��� */
/*int	sample_rate = 22050;	   sampling rate [Hz]	*/
				/* �����ѿ���������Ѥ� */

int	fmvol     = 100;	/* level of FM    (0-100)[%] */
int	psgvol    =  20;	/* level of PSG   (0-100)[%] */
int	beepvol   =  60;	/* level of BEEP  (0-100)[%] */
int	rhythmvol = 100;	/* level of RHYTHM(0-100)[%] depend on fmvol */
int	adpcmvol  = 100;	/* level of ADPCM (0-100)[%] depend on fmvol */
int	close_device = FALSE;	/* close sound device at menu mode */

/*----------------------------------------------------------------------*/

T_XMAME_FUNC	*xmame_func      = &xmame_func_nosound;


int		use_sound = TRUE;	/* 0:not use / 1 use sound	*/


/****************************************************************/
/* XMAME �Υ�����ɤ���Ѥ��뤿��ν��������λ�ؿ�		*/
/****************************************************************/
int	xmame_system_init( void )
{
  sample_rate = 22050;

  return 0;	/*OSD_OK;*/
}
void	xmame_system_term( void )
{
}


/****************************************************************/
/* XMAME �Υ��ץ�������Ϥ��뤿��δؿ�			*/
/* -1 ������­��ʤ������ϰϳ�					*/
/* 0  �������ץ����ʤ�					*/
/* 1  ����1�Ľ��� (ͥ���٤ˤ�����������ޤ�)			*/
/* 2  ����2�Ľ��� (ͥ���٤ˤ�����������ޤ�)			*/
/****************************************************************/
enum {
  OPT_SOUND, OPT_NOSOUND,
  OPT_VOLUME, OPT_FMVOL, OPT_PSGVOL, OPT_BEEPVOL,
  OPT_RHYTHMVOL, OPT_ADPCMVOL,
  OPT_SAMPLEFREQ, OPT_SDLBUFSIZE, OPT_SDLBUFNUM,
  OPT_CLOSE, OPT_NOCLOSE, OPT_FMGEN, OPT_NOFMGEN,
  EndofOPT
};

static struct {
  int		num;
  const	char	*str;
} opt_index[] =
{
  { OPT_SOUND,     "sound",     },
  { OPT_NOSOUND,   "nosound",   },
  { OPT_VOLUME,    "volume",    },
  { OPT_FMVOL,     "fmvol",     },
  { OPT_PSGVOL,    "psgvol",    },
  { OPT_BEEPVOL,   "beepvol",   },
  { OPT_RHYTHMVOL, "rhythmvol", },
  { OPT_ADPCMVOL,  "adpcmvol",  },
  { OPT_SAMPLEFREQ,"samplefreq",},
  { OPT_SDLBUFSIZE,"sdlbufsize",},
  { OPT_SDLBUFNUM, "sdlbufnum",	},
  { OPT_CLOSE,     "close",     },
  { OPT_NOCLOSE,   "noclose",   },
  { OPT_FMGEN,     "fmgen",     },
  { OPT_NOFMGEN,   "nofmgen",   },
};

#define	ERR_MES1( fmt )						\
		do{						\
		  fprintf( stderr, "error : %s\n", fmt );	\
		}while(0)

#define	ERR_MES2( fmt, arg )					\
		do{						\
		  fprintf( stderr, "error : " );		\
		  fprintf( stderr, fmt, arg );			\
		  fprintf( stderr, "\n" );			\
		}while(0)

#define	CONV_TO_INT(opt2)	strtol( opt2, &conv_end, 0 )
#define CONV_ERR()		(*conv_end!='\0')
#define CONV_OK()		(*conv_end=='\0')

#define	SET_OPT2_NUM_TO_WORK( work, low, high, mes_overflow, mes_no_opt )\
		do{							\
		  if( opt2 ){						\
		    long tmp;						\
		    ret_val ++;						\
		    tmp = CONV_TO_INT(opt2);				\
		    if( CONV_ERR() || tmp < low || high < tmp ){	\
		      ERR_MES1( mes_overflow );				\
		      return -1;					\
		    }							\
		    work = tmp;						\
		  }else{						\
		    ERR_MES1( mes_no_opt );				\
		    return -1;						\
		  }							\
		}while(0)


int	xmame_check_option( char *opt1, char *opt2, int priority )
{
  int	j,  ret_val = 1;
  char *conv_end;


  if( opt1==NULL )     return -1;
  if( opt1[0] != '-' ) return -1;


  for( j=0; j<COUNTOF(opt_index); j++ ){
    if( strcmp( &opt1[1], opt_index[j].str )==0 ) break;
  }

  if( j==COUNTOF(opt_index) ){

    /* ERR_MES2( "%s ... Wrong option!", opt1 ); */
    return 0;

  }else{

    switch( opt_index[j].num ){
    case OPT_SOUND:	use_sound = 1;			break;
    case OPT_NOSOUND:	use_sound = 0;			break;

    case OPT_VOLUME:
      SET_OPT2_NUM_TO_WORK( attenuation, -32, 0,
			   "-volume <x> ... x = (soft)-32..0(loud)",
			   "-volume ... No volume level supplied" );
      break;
    case OPT_FMVOL:
      SET_OPT2_NUM_TO_WORK( fmvol, 0, 100,
			   "-fmvol <level> ... level = 0..100",
			   "-fmvol ... No FM volume level supplied" );
      break;
    case OPT_PSGVOL:
      SET_OPT2_NUM_TO_WORK( psgvol, 0, 100,
			   "-psgvol <level> ... level = 0..100",
			   "-psgvol ... No PSG volume level supplied" );
      break;
    case OPT_BEEPVOL:
      SET_OPT2_NUM_TO_WORK( beepvol, 0, 100,
			   "-beepvol <level> ... level = 0..100",
			   "-beepvol ... No BEEP volume level supplied" );
      break;
    case OPT_RHYTHMVOL:
      SET_OPT2_NUM_TO_WORK( rhythmvol, 0, 200,
			   "-rhythmvol <level> ... level = 0..200",
			   "-rhythmvol ... No RHYTHM volume level supplied" );
      break;
    case OPT_ADPCMVOL:
      SET_OPT2_NUM_TO_WORK( adpcmvol, 0, 100,
			   "-adpcmvol <level> ... level = 0..100",
			   "-adpcmvol ... No ADPCM volume level supplied" );
      break;

    case OPT_SAMPLEFREQ:
      SET_OPT2_NUM_TO_WORK( sample_rate, 8000, 48000,
			   "-samplefreq <rate> ... rate = 8000..48000",
			   "-samplefreq ... No frequency rate supplied" );
      break;
	  
#if 0

    case OPT_SDLBUFSIZE:
      SET_OPT2_NUM_TO_WORK( SDL_BUFFERSIZE, 32, 65536,
			   "-sdlbufsize <size> ... size is 32..65536( recommended 'power of 2' )",
			   "-sdlbufsize ... No size supplied");
#if 0	/* �ͤ�2���Ѿ褫�ɤ����ϥ����å����ʤ� */
      {
	int power;			/* 2���Ѿ�ˤʤäƤ��뤫������å� */
	for( power=0; (1<<power) < SDL_BUFFERSIZE; power++ );
	if( (1<<power) != SDL_BUFFERSIZE ){
	  ERR_MES1( "-sdlbufsize <size> ... size is 32..65536( power of 2 )" );
	  return -1;
	}
      }
#endif
      break;

    case OPT_SDLBUFNUM:
      SET_OPT2_NUM_TO_WORK( SDL_BUFFERNUM, 1, 16,
			   "-sdlbufnum <size> ... size is 1..16",
			   "-sdlbufnum ... No size supplied");
      break;
#endif

    case OPT_CLOSE:	close_device = 1;		break;
    case OPT_NOCLOSE:	close_device = 0;		break;

    case OPT_FMGEN:	use_fmgen = 1;			break;
    case OPT_NOFMGEN:	use_fmgen = 0;			break;

    default:
      ERR_MES1( "Internal Error !" );
      return -1;
    }
  }

  return ret_val;
}



/****************************************************************/
/* XMAME �Υ��ץ�����ɽ�����뤿��δؿ�			*/
/****************************************************************/
void	xmame_show_option( void )
{
  fprintf( stdout, 
	   "\n"
	   "==========================================\n"
	   "== SOUND OPTIONS ( dependent on xmame ) ==\n"
	   "==                    [ xmame  0.71.1 ] ==\n"
	   "==========================================\n"
   "    -sound/-nosound      - Enable/disable sound (if available) [-sound]\n"
   "    -fmvol <level>       - Set FM     level to <level> %%, (0 - 100) [100]\n"
   "    -psgvol <level>      - Set PSG    level to <level> %%, (0 - 100) [20]\n"
   "    -beepvol <level>     - Set BEEP   level to <level> %%, (0 - 100) [60]\n"
   "    -samplefreq <rate>   - Set the playback sample-frequency/rate [22050]\n"
   "    -sdlbufsize <size>   - buffer size of sound stream (power of 2) [2048]\n"
   "    -sdlbufnum <size>    - Number of frames of sound to buffer [4]\n"
   "    -close/-noclose      - Close/don't close sound device in MENU mode [-close]\n"
	   );
}




/****************************************************************/
/* XMAME �Υ�����ɤγ���					*/
/* XMAME �Υ�����ɤι���					*/
/* XMAME �Υ�����ɤν�λ					*/
/* XMAME �Υ�����ɤ�����					*/
/* XMAME �Υ�����ɤκƳ�					*/
/* XMAME �Υ�����ɤΥꥻ�å�					*/
/****************************************************************/
static	int	sound_stream = FALSE;
static short *snd_buffer;
static int snd_bufpos = 0;

int	xmame_sound_start( void )
{
  int i;
  xmame_func_set(0);

  snd_buffer = psound_get_buffer();
  snd_bufpos = 0;

  if( use_sound ){ sound_enabled = TRUE;  sound_stream = TRUE; }
  else           { sound_enabled = FALSE; sound_stream = FALSE;  return 1; }


  if(verbose_proc) printf("Starting sound server: .....");

  if( sound_start()==0 ){

    sound_enabled = TRUE;
    xmame_func_set(sound_enabled);
    sound_reset();
    osd_sound_enable(1);

  }else{

    sound_enabled = FALSE;
    xmame_func_set(sound_enabled);

  }

  if(verbose_proc)
    printf("%s\n", (sound_enabled) ? "...OK" : "...FAILED, Can't use sound");

  return 1;
}

void	xmame_sound_update( void )
{
  if( sound_enabled )
    sound_update();
}

void	xmame_sound_stop( void )
{
  if( sound_enabled )
    sound_stop();
}

void	xmame_sound_suspend( void )
{
   psound_switch(0);

  if( close_device )
    osd_sound_enable(0);
}
void	xmame_sound_resume( void )
{
   psound_switch(1);

  if( close_device )
    osd_sound_enable(1);
}
void	xmame_sound_reset( void )
{
  if( sound_enabled )
    sound_reset();
}

int	xmame_sound_is_enable( void )
{
  if( sound_stream ) return TRUE;
  else               return FALSE;
}

void	xmame_update_video_and_audio( void )
{
  /* nothing */
}


/****************************************************************/
/* ������ɥݡ�����������˸Ƥ�					*/
/****************************************************************/
byte	xmame_sound_in_data( void )
{
  if( xmame_func->sound_in_data ) return (xmame_func->sound_in_data)(0);
  else                            return 0xff;
}
byte	xmame_sound_in_status( void )
{
  if( xmame_func->sound_in_data ) return (xmame_func->sound_in_status)(0);
  else                            return 0;
}
void	xmame_sound_out_reg( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound_out_reg)(0,data);
}
void	xmame_sound_out_data( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound_out_data)(0,data);
}


byte	xmame_sound2_in_data( void )
{
  if( !use_sound ) return 0xff;
  else{
    if( sound_board==SOUND_I ) return 0xff;
    else                       return 0;
  }
}
byte	xmame_sound2_in_status( void )
{
  if( xmame_func->sound_in_data ) return (xmame_func->sound2_in_status)(0);
  else                            return 0xff;
}
void	xmame_sound2_out_reg( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound2_out_reg)(0,data);
}
void	xmame_sound2_out_data( byte data )
{
  if( xmame_func->sound_in_data ) (xmame_func->sound2_out_data)(0,data);
}


void	xmame_beep_out_data( byte data )
{
  if( xmame_func->beep_out_data ) (xmame_func->beep_out_data)(0,data);
}




/****************************************************************/
/* ������ɤΥ����ޡ������С��ե����˸Ƥ�			*/
/*	timer = 0 TimerAOver / 1 TimerBOver			*/
/****************************************************************/
void	xmame_sound_timer_over( int timer )
{
  if( xmame_func->sound_timer_over ) (xmame_func->sound_timer_over)(0, timer);
}




/****************************************************************/
/* �ܥ�塼�����						*/
/*	���ߤβ��̤��������					*/
/****************************************************************/
int	xmame_get_sound_volume( void )
{
  return osd_get_mastervolume();
}

/****************************************************************/
/* �ܥ�塼���ѹ�						*/
/*	�����ˡ����̤�Ϳ���롣���̤ϡ�-32��0 �ޤ�		*/
/****************************************************************/
void	xmame_set_sound_volume( int vol )
{
    if( vol > VOL_MAX ) vol = VOL_MAX;
    if( vol < VOL_MIN ) vol = VOL_MIN;
    osd_set_mastervolume( vol );
}



/****************************************************************/
/* �����ͥ��̥�٥����					*/
/*	�����ˡ������ͥ��Ϳ����				*/
/*	�����ͥ�ϡ�0==PSG�� 1==FM��2==BEEP			*/
/*	�� xmame �����ǤΥ����ͥ�				*/
/*	    ������ɥܡ���I	CH0��2  PSG			*/
/*				CH3	FM      		*/
/*				CH4	BEEP      		*/
/*	    ������ɥܡ���II	CH0��2  PSG			*/
/*				CH3	FM(L)			*/
/*				CH4	FM(R)			*/
/*				CH5	BEEP			*/
/****************************************************************/
int	xmame_get_mixer_volume( int ch )
{
  if( use_fmgen==0 ){
    if( sound_board==SOUND_I ){
      switch( ch ){
      case XMAME_MIXER_PSG:	ch = 0;		break;
      case XMAME_MIXER_FM:	ch = 3;		break;
      case XMAME_MIXER_BEEP:	ch = 4;		break;
      case XMAME_MIXER_RHYTHM:	return 0;
      case XMAME_MIXER_ADPCM:	return 0;
      default:			return 0;
      }
    }else{
      switch( ch ){
      case XMAME_MIXER_PSG:	ch = 0;		break;
      case XMAME_MIXER_FM:	ch = 3;		break;
      case XMAME_MIXER_BEEP:	ch = 5;		break;
      case XMAME_MIXER_RHYTHM:	return rhythmvol;
      case XMAME_MIXER_ADPCM:	return adpcmvol;
      default:			return 0;
      }
    }
  }else{
    switch( ch ){
    case XMAME_MIXER_FMPSG:	ch = 0;		break;
    case XMAME_MIXER_BEEP:	ch = 2;		break;
    default:			return 0;
    }
  }
  return mixer_get_mixing_level( ch );
}

/****************************************************************/
/* �����ͥ��̥�٥��ѹ�					*/
/*	�����ˡ������ͥ�ȥ�٥��Ϳ����			*/
/*	��٥�ϡ�    0��100 �ޤ�				*/
/*	�����ͥ�ϡ�0==PSG�� 1==FM��2==BEEP			*/
/*				����ʳ��ϰ�����ɽ������	*/
/*	�� xmame �����ǤΥ����ͥ�				*/
/*	    ������ɥܡ���I	CH0��2  PSG			*/
/*				CH3	FM      		*/
/*				CH4	BEEP      		*/
/*	    ������ɥܡ���II	CH0��2  PSG			*/
/*				CH3	FM(L)			*/
/*				CH4	FM(R)			*/
/*				CH5	BEEP			*/
/*	    FMGEN�ξ��		CH0	PSG/FM(L)		*/
/*				CH2	PSG/FM(R)		*/
/*				CH2	BEEP      		*/
/****************************************************************/
void	xmame_set_mixer_volume( int ch, int level )
{
  if( use_fmgen ){
  switch( ch ){
  case XMAME_MIXER_FMPSG:
    if( level < FMPSGVOL_MIN ) level = FMPSGVOL_MIN;
    if( level > FMPSGVOL_MAX ) level = FMPSGVOL_MAX;
    mixer_set_mixing_level( 0, level );
    mixer_set_mixing_level( 1, level );
    break;
  case XMAME_MIXER_BEEP:
    if( level < BEEPVOL_MIN ) level = BEEPVOL_MIN;
    if( level > BEEPVOL_MAX ) level = BEEPVOL_MAX;
    mixer_set_mixing_level( 2, level );
    break;
  default:
    for( ch=0; ch<MIXER_MAX_CHANNELS ; ch++ ){
      const char *name = mixer_get_name(ch);
      if(name) printf( "%d[ch] %s\t:%d\n", ch,name,mixer_get_mixing_level(ch));
    }
    break;
  }
  return;
  }

  switch( ch ){
  case XMAME_MIXER_PSG:
    if( level < PSGVOL_MIN ) level = PSGVOL_MIN;
    if( level > PSGVOL_MAX ) level = PSGVOL_MAX;
    mixer_set_mixing_level( 0, level );
    mixer_set_mixing_level( 1, level );
    mixer_set_mixing_level( 2, level );
    break;
    
  case XMAME_MIXER_FM:
    if( level < FMVOL_MIN ) level = FMVOL_MIN;
    if( level > FMVOL_MAX ) level = FMVOL_MAX;
    if( sound_board==SOUND_I ){
      mixer_set_mixing_level( 3, level );
    }else{
      mixer_set_mixing_level( 3, level );
      mixer_set_mixing_level( 4, level );
    }
    break;
    
  case XMAME_MIXER_BEEP:
    if( level < BEEPVOL_MIN ) level = BEEPVOL_MIN;
    if( level > BEEPVOL_MAX ) level = BEEPVOL_MAX;
    if( sound_board==SOUND_I ){
      mixer_set_mixing_level( 4, level );
    }else{
      mixer_set_mixing_level( 5, level );
    }
    break;

  case XMAME_MIXER_RHYTHM:
    if( level < RHYTHMVOL_MIN ) level = RHYTHMVOL_MIN;
    if( level > RHYTHMVOL_MAX ) level = RHYTHMVOL_MAX;
    if( sound_board==SOUND_II ){
      rhythmvol = level;
    }
    break;

  case XMAME_MIXER_ADPCM:
    if( level < ADPCMVOL_MIN ) level = ADPCMVOL_MIN;
    if( level > ADPCMVOL_MAX ) level = ADPCMVOL_MAX;
    if( sound_board==SOUND_II ){
      adpcmvol = level;
    }
    break;

  default:
    for( ch=0; ch<MIXER_MAX_CHANNELS ; ch++ ){
      const char *name = mixer_get_name(ch);
      if(name) printf( "%d[ch] %s\t:%d\n", ch,name,mixer_get_mixing_level(ch));
    }
    break;
  }
}



/*==============================================================*/
/* osd ���������						*/
/*==============================================================*/

static float sound_bufsize = 3.0;
static int sound_samples_per_frame = 0;


void osd_set_mastervolume (int attenuation)
{
}


int osd_get_mastervolume (void)
{
  return 0;
}

/*
 * xmame-0.71.1/src/unix/sound.c
 */
int osd_start_audio_stream(int stereo)
{

   /* create sound_stream */
   if(sound_enabled)
   {
      /* calculate samples_per_frame */
      sound_samples_per_frame = (int)(Machine__sample_rate /
         Machine__drv__frames_per_second);	      /* mean 22050Hz/55.4Hz */
   }

   /* if sound is not enabled, set the samplerate of the core to 0 */

   if(!sound_enabled)
   {
      sound_samples_per_frame = 0;
   }

   psound_set_stereo(stereo);
   psound_switch(1);

   psound_setfps(sound_samples_per_frame);
   psound_setfreq(Machine__sample_rate);
   
   /* return the samples to play the first frame */
   return sound_samples_per_frame;
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
 
int osd_update_audio_stream(INT16 *buffer)
{
	unsigned i,j;
	unsigned space;
	unsigned samples;
			
	i = psound_get_read_pos();
	space = (i - snd_bufpos) & SNDBUF_SIZE_MASK;
	
	
	if (psound_flag_stereo)
		samples = sound_samples_per_frame<<1;
	else
		samples = sound_samples_per_frame;


	if (space < samples)
	{
		j = psound_gettotal();
		snd_bufpos += SNDBUF_SIZE_HALF;
	}
	for(i=0; i < samples; i++)
	{
		snd_buffer[(i + snd_bufpos) & SNDBUF_SIZE_MASK] = buffer[i];
	}
	snd_bufpos = (snd_bufpos + samples) & SNDBUF_SIZE_MASK;

	space = (psound_get_read_pos() - snd_bufpos) & SNDBUF_SIZE_MASK;

	if (psound_flag_stereo)
		sound_samples_per_frame = psound_needsamples(space>>1);
	else
		sound_samples_per_frame = psound_needsamples(space);
	
   /* return the samples to play this next frame */
   return sound_samples_per_frame;
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
void osd_stop_audio_stream(void)
{
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
void osd_sound_enable (int enable_it)
{
  if( ! sound_stream ) return;

   if (enable_it)
   {
      sound_enabled = 1;
      xmame_func_set(sound_enabled);
   }
   else
   {
      sound_enabled = 0;
      xmame_func_set(sound_enabled);
   }
}





#else		/* !USE_SOUND */

/*
 *  USE_SOUND ̤����ˤ�ؤ�餺���������Ȥ߹�������Υ�󥯥��顼����
 */

int	fmvol;
int	psgvol;
int	beepvol;
int	rhythmvol;
int	adpcmvol;

int osd_start_audio_stream(int stereo){return 0;}
void osd_stop_audio_stream(void){}
int osd_update_audio_stream(short *buffer){return 0;}

#endif
