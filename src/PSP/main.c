/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <pspkernel.h>
#include <pspdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"

#include "file-op.h"
#include "getconf.h"

#include "device.h"
#include "snddrv.h"	/* xmame_system_init(), xmame_system_term() */
#include "suspend.h"
#include "menu.h"
#include "mame-quasi88.h"

#include "ptk_menu.h"
#include "ptk_softkey.h"
#include "ptk_popup.h"
#include "ptk_font.h"
#include "ptk_sound.h"
#include "ptk_config.h"
#include "ptk_ext.h"

PSP_MODULE_INFO("NEOQ88PSP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int exitCallback(int arg1, int arg2, void *common)
{
	ext_back_cpufreq();
	sceKernelExitGame();
	return 0;
}

int callbackThread(SceSize args, void *argp) {
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (void*) exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

int setupCallbacks(void) {
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}

#define printf	pspDebugScreenPrintf
#define gotoxy(x,y)  pspDebugScreenSetXY((x),(y))

char eboot_dir[512];

void set_eboot_dir(char *path)
{
	int i;

	strcpy(eboot_dir,path);
	
	for(i=strlen(eboot_dir); i > 0; i--)
	{
		if (eboot_dir[i]=='/')
		{
			eboot_dir[i]=0; break;
		}
	}
}

int	main( int argc, char *argv[] )
{
  pspDebugScreenInit();
  setupCallbacks();

  use_fmgen = 0;

  memset( file_disk[0], 0, QUASI88_MAX_FILENAME );
  memset( file_disk[1], 0, QUASI88_MAX_FILENAME );
  
  set_eboot_dir(argv[0]);
  
  font_init();
  init_swkbd();
  pmenu_init(eboot_dir);
  pop_init();
  psound_init();
  pconfig_init("quasi88p.cfg");
  
  pconfig_load();
  
  if (pmenu_drive0_img >= 0)
  {
	strcpy(file_disk[0],pmenu_disk0);
	image_disk[0] = pmenu_drive0_img;
  }

  if (pmenu_drive1_img >= 0)
  {
	strcpy(file_disk[1],pmenu_disk1);
	image_disk[1] = pmenu_drive1_img;
  }
    
  pmenu_set_status(pmenu_menu_start);
  
  ext_save_deffreq();
  ext_set_cpufreq();
  

//  strcpy(file_disk[0],"disk.d88");
//  strcpy(file_disk[1],"disk.d88");
//  image_disk[0] = 1;	/* ���᡼���ֹ�� 1������(0���ˤ���) */
//  image_disk[1] = 2;	/* ���᡼���ֹ�� 1������(0���ˤ���) */
  
  readonly_disk[0] = menu_readonly;	/* �꡼�ɥ���꡼�ϡ�ξ�ɥ饤�ֶ��� */
  readonly_disk[1] = menu_readonly;

  psp_system_init(); // graphics

  xmame_system_init();		/* XMAME������ɴ�Ϣ����� (��������������!) */

  if( osd_environment() ){		/* �Ķ�����	*/

      quasi88();			/* PC-8801 ���ߥ�졼����� */
  }

  xmame_system_term();		/* XMAME������ɴ�Ϣ����� */

  psp_system_term();

  

  return 0;
}


/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

/*	¾�ξ��󤹤٤Ƥ����� or �����֤��줿��˸ƤӽФ���롣
 *	ɬ�פ˱����ơ������ƥ��ͭ�ξ�����ղä��Ƥ⤤�����ȡ�
 */

int	stateload_system( void )
{
  return TRUE;
}
int	statesave_system( void )
{
  return TRUE;
}



/***********************************************************************
 * ��˥塼���̤�ɽ�����롢�����ƥ��ͭ��å�����
 ************************************************************************/

static int about_lang;
static int about_line;

/*	���� japanese �����ʤ顢���ܸ�ɽ�����׵�
 *	���� ���������� 0��2��-1�ʤ����ʤ�
 */
int	about_msg_init( int japanese )
{
  about_lang = ( japanese ) ? 1 : 0;
  about_line = 0;

  return -1;		/* ʸ�������ɻ���ʤ� */
}

/*	��å�������ʸ������֤�
 *	����ʾ��å�������̵�����ϡ�NULL
 */
#ifdef	USE_SOUND

const char *about_msg( void )
{
  static const char *about_en[] =
  {
#ifdef	USE_SOUND
    "(Caution) Volume control is not supported.",
#endif
  };


  static const char *about_jp[] =
  {
#ifdef	USE_SOUND
    "(���) �����ѹ���ǽ�ϥ��ݡ��Ȥ���Ƥ��ޤ��� ",
#endif
  };


  if( about_lang ){
    if( about_line >= COUNTOF(about_jp) ) return NULL;
    else                                  return about_jp[ about_line ++ ];
  }else{
    if( about_line >= COUNTOF(about_en) ) return NULL;
    else                                  return about_en[ about_line ++ ];
  }
}

#else
const char *about_msg( void ){ return NULL; }
#endif
