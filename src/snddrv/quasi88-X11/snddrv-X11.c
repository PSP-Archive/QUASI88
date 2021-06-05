/************************************************************************/
/*									*/
/* ������ɤν��� (�������顢XMAME�δؿ���ƤӽФ�)			*/
/*									*/
/*	���٤ơ�XMAME �ν�����Ԥʤ��ؿ�				*/
/*	QUASI88 �ϡ������δؿ����̤��ơ�XMAME�Υ�����ɽ�����Ԥʤ�	*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mame-quasi88.h"

#define  SNDDRV_WORK_DEFINE
#include "snddrv-X11.h"



#ifdef	USE_SOUND

#include "sysdep_mixer.h"	/* sysdep_mixer_init()	*/
#include "fm.h"			/* YM2203TimerOver()	*/

/*----------------------------------------------------------------------*/
/* rc_struct ��¤�Τˤ�ꡢ���ץ��������椹�� */

static struct rc_struct *rc;

extern	struct rc_option sound_opts[];


/*----------------------------------------------------------------------*/



T_XMAME_FUNC	*xmame_func      = &xmame_func_nosound;


int		use_sound = TRUE;	/* 0:not use / 1 use sound	*/


/****************************************************************/
/* XMAME �Υ�����ɤ���Ѥ��뤿��ν��������λ�ؿ�		*/
/****************************************************************/
int	xmame_system_init( void )
{
   /* create the rc object */
   if (!(rc = rc_create()))
      return OSD_NOT_OK;

   if(sysdep_dsp_init(rc, NULL))
      return OSD_NOT_OK;

   if(sysdep_mixer_init(rc, NULL))
      return OSD_NOT_OK;

   if(rc_register(rc, sound_opts))
      return OSD_NOT_OK;

   return OSD_OK;
}

void	xmame_system_term( void )
{
   if(rc)
   {
      sysdep_mixer_exit();
      sysdep_dsp_exit();
      rc_destroy(rc);
   }
}


/****************************************************************/
/* XMAME �Υ��ץ�������Ϥ��뤿��δؿ�			*/
/* -1 ������­��ʤ������ϰϳ�					*/
/* 0  �������ץ����ʤ�					*/
/* 1  ����1�Ľ��� (ͥ���٤ˤ�����������ޤ�)			*/
/* 2  ����2�Ľ��� (ͥ���٤ˤ�����������ޤ�)			*/
/****************************************************************/
int	xmame_check_option( char *arg1, char *arg2, int priority )
{
  return rc_quasi88( rc, arg1, arg2, priority );
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
	   );

  rc_print_help(rc, stdout);
}





/****************************************************************/
/* XMAME �Υ�����ɤγ���					*/
/* XMAME �Υ�����ɤι���					*/
/* XMAME �Υ�����ɤν�λ					*/
/* XMAME �Υ�����ɤ�����					*/
/* XMAME �Υ�����ɤκƳ�					*/
/* XMAME �Υ�����ɤΥꥻ�å�					*/
/****************************************************************/
int	xmame_sound_start( void )
{
  if(verbose_proc) printf("Starting sound server: .....");

  if( sound_start()==0 ){

    xmame_func_set(use_sound);

    if(verbose_proc) printf("...OK\n");
    sound_reset();

  }else{

    if(verbose_proc) printf("...FAILED, Can't use sound\n");

  }
  return 1;
}

void	xmame_sound_update( void )
{
  sound_update();
}

void	xmame_sound_stop( void )
{
  sound_stop();
}

void	xmame_sound_suspend( void )
{
  if( close_device )
    osd_sound_enable(0);
}
void	xmame_sound_resume( void )
{
  if( close_device )
    osd_sound_enable(1);
}
void	xmame_sound_reset( void )
{
  sound_reset();
}

int	xmame_sound_is_enable( void )
{
  if( sound_stream ) return TRUE;
  else               return FALSE;
}

/*
  ��mame/xmame (0.71) �� ������ɽ��Ϥ�ή���

  src/mame.c
	int updatescreen(void)		VSYNC �Υ����ߥ󥰤ˤƸƤӽФ����
	{
	    sound_update();				[1]��
	    :
	    :�����ǡ�������
	    :
	    update_video_and_audio();			[A]��
	    :
	}

  src/sndintrf.c
	void sound_update(void)		    --- [1]
	{
	    :
	    mixer_sh_update():				[2]��
	    :
	    osd_update_audio_stream(..);		[3]��
	}

  src/sound/mixer.c
	void mixer_sh_update(void)	    --- [2]
	{
	    :VSYNC�ֳֻ���ʬ��PCM�ǡ��������� 
	}

  src/unix/sound.c
	int osd_update_audio_stream(..)	    --- [3]
	{
	    sound_stream_write(..)			[4]��
	}

  src/unix/sysdep/sound_stream.c
	void sound_stream_write(..)	    --- [4]
	{
	    PCM�ǡ�����FIFO�ˤ���Ƥ�����
	    3�� (VSYNC 3��ʬ��PCM�ǡ���) �ޤǤ�����뤬��
	    FIFO �����դΤȤ��ϡ�PCM�ǡ����ϼΤƤ���(�Υ���ȯ��?)
	}

  src/mame.c
	void update_video_and_audio(void)   === [A]
	{
	    :
	    :�ѥ�åȾ��󹹿�
	    :
	    artwork_update_video_and_audio(..);		[B]��
	    :
	}

  src/artwork.c
	void artwork_update_video_and_audio(..) === [B]
	{
	    :
	    osd_update_video_and_audio(..);		[C]��
	    :
	}

  src/unix/video.c
	void osd_update_video_and_audio(..) === [C]
	{
	    :
	    :UI����
	    :�ե졼�ॹ���å�Ƚ�� & ��������Ĵ��(usleep)
	    :
	    sound_stream_update(..);			[D]��
	    :
	    :��������
	    :�ѥ�åȹ���
	    :FPS����
	    :LED����
	    :���祤���ƥ��å�����
	}

  src/unix/sysdep/sound_stream.c
	void sound_stream_update(..)	    === [D]
	{
	    FIFO�ˤ��ޤäƤ���ǡ�����DSP�˽��ϡ�
	    DSP�˽��ϤǤ���¤�Υǡ�������Ϥ���Τǡ�
	    FIFO�����ˤʤäƤ�DSP�ˤޤ����ϤǤ���Ȥ��ϡ�
	    ���ߡ��ǡ���(?)����Ϥ���(�Υ���ȯ��?)
	}

*/

void	xmame_update_video_and_audio( void )
{
   if (sound_stream && sound_enabled)
      sound_stream_update(sound_stream);
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
