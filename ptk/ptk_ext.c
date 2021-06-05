// pspext.c

#include <pspkernel.h>
#include <pspgu.h>
#include <psppower.h>

#include <string.h>

#include "quasi88.h"
#include "suspend.h"
#include "pc88main.h"
#include "pc88sub.h"
#include "soundbd.h"
#include "memory.h"
#include "keyboard.h"
#include "drive.h"
#include "image.h"
#include "emu.h"
#include "snddrv.h"

#include "event.h"

#include "ptk_popup.h"
#include "ptk_ext.h"
#include "ptk_config.h"
#include "ptk_sound.h"

int ext_boot_dipsw;
int ext_boot_from_rom;
int ext_boot_basic;
int ext_boot_clock_4mhz;
int ext_set_version;
unsigned char ext_baudrate_sw;
int ext_sound_board;

int ext_sound_buf_size;
int ext_s1mode = 0;

int ext_cpuclock = 0;
static int ext_old_cpuclock = 0;


char *ext_dummy[]=
{
 "RED",
 "GREEN",
 "BLUE",
 "BROWN",
 "SILVER",
 "GOLD",
 NULL
};

void ext_quit(void)
{
	pc88main_term();
	pc88sub_term();
	disk_eject(0);
	disk_eject(1);
	
	ext_back_cpufreq();
	
	sceKernelExitGame();
//	pop_about("QUIT");
}

void ext_get_sysval(void)
{
	ext_boot_dipsw		= boot_dipsw;
	ext_boot_from_rom	= boot_from_rom;
	ext_boot_basic		= boot_basic;
	ext_boot_clock_4mhz = boot_clock_4mhz;
	ext_set_version		= set_version;
	ext_baudrate_sw		= baudrate_sw;
	ext_sound_board		= sound_board;
	ext_sound_buf_size  = sound_buf_size;
}

void ext_set_sysval(void)
{
	boot_dipsw		= ext_boot_dipsw;
	boot_from_rom	= ext_boot_from_rom;
	boot_basic		= ext_boot_basic;
	boot_clock_4mhz = ext_boot_clock_4mhz;
	set_version		= ext_set_version;
	baudrate_sw		= ext_baudrate_sw;
	sound_board		= ext_sound_board;
	sound_buf_size  = ext_sound_buf_size;
	
}

void ext_save_deffreq(void)
{
	ext_old_cpuclock = scePowerGetCpuClockFrequency();
}

void ext_set_cpufreq(void)
{
	if (ext_cpuclock)
	{
		switch(ext_cpuclock)
		{
			case 222:
				scePowerSetClockFrequency(222, 222, 111);
			break;
			case 333:
				scePowerSetClockFrequency(333, 333, 166);
			break;
			case 300:
				scePowerSetClockFrequency(300, 300, 150);
			break;
			case 266:
				scePowerSetClockFrequency(266, 266, 133);
			break;
		}
	}
}

int  ext_get_cpufreq(void)
{
	return scePowerGetCpuClockFrequency();
}

void ext_back_cpufreq(void)
{
	if (ext_old_cpuclock)
	{
		ext_cpuclock = ext_old_cpuclock;
		ext_set_cpufreq();
	}
}

void ext_reset(void)
{
  int cur_sndbd = sound_board;

  ext_set_sysval();
  
  quasi88_reset();

  if( cur_sndbd != sound_board )
  {
    if( memory_allocate_additional() == FALSE ){
      pop_about("FATAL ERROR!!!");
	  return;
    }
    xmame_sound_resume();		/* 中断したサウンドを復帰後に */
    xmame_sound_stop();			/* サウンドを停止させる。     */
    xmame_sound_start();		/* そして、サウンド再初期化   */
  }

  set_emu_mode( GO );

//  pop_about("RESET");
}

void ext_save_setting(void)
{
  ext_set_sysval();
  if (pconfig_save())
	pop_about("FAILED TO SAVE");
else
	pop_about("SAVE SETTING");
}

void ext_save_state(void)
{
  if (statesave())
	pop_about("SAVE STATE");
else
	pop_about("FAILD TO SAVE STATE");
}

void ext_load_state(void)
{
  if (stateload_check())
  {
	  if( quasi88_stateload() )
		  pop_about("LOAD STATE");
	  else
		  pop_about("FAILD TO SAVE STATE");
  }
	else
  {
	pop_about("NO STATE FILE");
  }
}


void ext_setdip_disk(void)
{
	if (disk_image_exist(0) || disk_image_exist(1))
		ext_boot_from_rom = 0;
	else
		ext_boot_from_rom = 1;
}

int ext_opendisk(int drv,char *name)
{
    if( quasi88_disk_insert( drv,name, 0, 0 ) == FALSE)
	{
		pop_about("ERROR : DISK OPEN ERROR");
		return -1;
    }

//	pop_debugf("0:%08x 1:%08x 0rd:%d",drive[0].fp,drive[1].fp,drive[0].read_only);

	disk_change_image( drv, 0 );
	
	ext_setdip_disk();
	
	return 0;
}

int ext_getdiskimgs(int drv)
{
	if (disk_image_exist(drv))
		return disk_image_num(drv)+1;
	return 0;
//	return 5;
}

char *ext_getimgname(int drv,int img)
{
	if (!img)
	return "EJECT";
   else
    {
		if (drive[drv].empty || img < 0 || img > drive[drv].image_nr)
			return "";
		else
			return drive[drv].image[img-1].name;
    }
//	return ext_dummy[img];
}

int ext_get_curimage(int drv)
{
  if (drive[drv].empty)
	return -1;

  return drive[drv].selected_image;
}

void ext_setdrvimage(int drv,int img)
{
	if (!img)
	{
		pop_about("EJECT IMAGE");
		quasi88_disk_eject( drv );
	}
	else
	{				/* img>=0 なら イメージ番号 */
		drive_unset_empty( drv );
		disk_change_image( drv, img-1 );
	}
	ext_setdip_disk();
}


