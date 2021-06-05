#include <stdio.h>
#include <string.h>
#include <ctype.h>

static char pconfig_cfgfile[512];

typedef struct
{
	int type;
	void *value;
	char *name;
} t_conf;

enum {
PCONF_INT,
PCONF_STR,
PCONF_DOUBLE,
PCONF_FLOAT,
PCONF_BYTE,
PCONF_UNSIGN
} PCONF_TYPE;

#ifdef TEST_CONFIG

#include "config_test.h"
#else
#include "quasi88.h"
#include "pc88main.h"
#include "intr.h"
#include "memory.h"
#include "screen.h"
#include "emu.h"
#include "soundbd.h"
#include "ptk_menu.h"
#include "ptk_sound.h"
#include "ptk_ext.h"
#include "mame-quasi88.h"

#include "fdc.h"


#define PROG_NAME "QUASI88 for PSP"

t_conf pconfig_param[]=
{
{PCONF_INT,&cpu_timing,"cpu_timing"},

{PCONF_DOUBLE,&vsync_freq_hz,"vsync_freq_hz"},
{PCONF_INT,&use_extram,"use_extram"},
{PCONF_INT,&use_jisho_rom,"use_jisho_rom"},
{PCONF_INT,&boot_basic,"boot_basic"},
{PCONF_INT,&boot_dipsw,"boot_dipsw"},
{PCONF_INT,&boot_from_rom,"boot_from_rom"},
{PCONF_INT,&boot_clock_4mhz,"boot_clock_4mhz"},
{PCONF_BYTE,&baudrate_sw,"baudrate_sw"},
{PCONF_INT,&verbose_level,"verbose_level"},
{PCONF_INT,&frameskip_rate,"frameskip_rate"},
{PCONF_INT,&use_auto_skip,"use_auto_skip"},

{PCONF_INT,&sound_board,"sound_board"},
{PCONF_INT,&pmenu_showfps,"pmenu_showfps"},
{PCONF_INT,&pmenu_bgrun,"pmenu_bgrun"},

{PCONF_INT,&pmenu_menu_start,"pmenu_menu_start"},
{PCONF_INT,&pmenu_drive0_img,"pmenu_drive0_img"},
{PCONF_INT,&pmenu_drive1_img,"pmenu_drive1_img"},
{PCONF_STR,&pmenu_disk0,"pmenu_disk0"},
{PCONF_STR,&pmenu_disk1,"pmenu_disk1"},

{PCONF_INT,&pmenu_type_fs,"pmenu_type_fs"},
{PCONF_UNSIGN,&sound_buf_size,"sound_buf_size"},
{PCONF_INT,&ext_cpuclock,"ext_cpuclock"},

{PCONF_INT,&pmenu_kc_aflag,"pmenu_kc_aflag"},
{PCONF_INT,&pmenu_kctbl[PMENU_UP],"pmenu_up"},
{PCONF_INT,&pmenu_kctbl[PMENU_DW],"pmenu_dw"},
{PCONF_INT,&pmenu_kctbl[PMENU_LE],"pmenu_le"},
{PCONF_INT,&pmenu_kctbl[PMENU_RI],"pmenu_ri"},
{PCONF_INT,&pmenu_kctbl[PMENU_A],"pmenu_a"},
{PCONF_INT,&pmenu_kctbl[PMENU_B],"pmenu_b"},
{PCONF_INT,&pmenu_kctbl[PMENU_C],"pmenu_c"},
{PCONF_INT,&pmenu_kctbl[PMENU_D],"pmenu_d"},
{PCONF_INT,&pmenu_kctbl[PMENU_E],"pmenu_e"},
{PCONF_INT,&pmenu_kctbl[PMENU_F],"pmenu_f"},

{PCONF_INT,&pmenu_kctbl[PMENU_AUP],"pmenu_aup"},
{PCONF_INT,&pmenu_kctbl[PMENU_ADW],"pmenu_adw"},
{PCONF_INT,&pmenu_kctbl[PMENU_ALE],"pmenu_ale"},
{PCONF_INT,&pmenu_kctbl[PMENU_ARI],"pmenu_ari"},

{PCONF_INT,&ext_sound_board,"ext_sound_board"},
{PCONF_INT,&ext_sound_buf_size,"ext_sound_buf_size"},
{PCONF_INT,&use_fmgen,"use_fmgen"},
{PCONF_INT,&pmenu_reset_start,"pmenu_reset_start"},


{PCONF_INT,&fdc_wait,"fdc_wait"},
{PCONF_INT,&highspeed_mode,"highspeed_mode"},
{PCONF_INT,&memory_wait,"memory_wait"},

{PCONF_INT,&pmenu_rev_menusw,"pmenu_rev_menusw"},


{PCONF_INT,&pmenu_cpuclk_int,"pmenu_cpuclk_int"},
{PCONF_INT,&pmenu_sndclk_int,"pmenu_sndclk_int"},



{0,NULL,NULL}
};




#endif

static char *skip_space(char *ptr)
{
	while(*ptr && isspace(*ptr)) ptr++;
	return ptr;
}

static char *get_value(char *name,char *src)
{
	int quote;
		
	quote = 0;
	
	src = skip_space(src);
	
	while(*src)
	{
		if (*(src) == '"') 
		{
			quote = !quote;
			src++;
			continue;
		}
				
		if (!quote && isspace(*src))
		{
			*(name)=0;
			return src;		
		}
		else
		{
			*(name++)=*(src++);
		}
	}
	*(name)=0;
	return src;
}

void pconfig_init(char *filename)
{
	pconfig_cfgfile[0]=0;
	if (filename)
		strcpy(pconfig_cfgfile,filename);
}
 

int pconfig_load_file(char *filename)
{
	int  i;
	int val;
	FILE *fp;
	char buf[128];
	char name[128];
	char *ptr;
	float val_float;

	fp = fopen(filename,"r");
	if (!fp)
		return -1;

	do 
	{
		buf[0] = 0;
		fgets(buf,128,fp);
		ptr = strchr(buf,'#');
		if (ptr)
			*ptr = 0;
		
		ptr = get_value(name,buf);
		if (strlen(name) > 0)
		{
//			printf("Name = %s : ",name);
			for(i=0; pconfig_param[i].name; i++)
			{
				if (!strcmp(pconfig_param[i].name,name)) break;
			}

			if (!pconfig_param[i].name)
			{
//				printf("Unknown paramater : %s \n",name);
			}
			 else
			{
				get_value(name,ptr);
				switch (pconfig_param[i].type)
				{
					case PCONF_INT:
					sscanf(name,"%d",&val);
					*(int *)pconfig_param[i].value = val;
					break;
					case PCONF_STR:
					strcpy(pconfig_param[i].value,name);
					break;
					case PCONF_DOUBLE:
					sscanf(name,"%f",&val_float);
					*(double *)pconfig_param[i].value = val_float;
					break;
					case PCONF_FLOAT:
					sscanf(name,"%f",&val_float);
					*(float *)pconfig_param[i].value = val_float;
					break;
					case PCONF_BYTE:
					sscanf(name,"%d",&val);
					*(unsigned char *)pconfig_param[i].value = val;
					break;
					case PCONF_UNSIGN:
					sscanf(name,"%d",&val);
					*(unsigned *)pconfig_param[i].value = val;
					break;
				}
			}
		}
	}while(!feof(fp));
	
	fclose(fp);
	
	return 0;
}

int pconfig_save_file(char *filename)
{
	int  i;
	FILE *fp;


	fp = fopen(filename,"w");
	if (!fp)
		return -1;
		
	fprintf(fp,"#\n#\n# %s configuration\n#\n#\n\n",PROG_NAME);
	
	for(i=0; pconfig_param[i].name; i++)
	{
			switch(pconfig_param[i].type)
			{
			case PCONF_INT:
				fprintf(fp,"%s\t%d\n",pconfig_param[i].name,*(int *)pconfig_param[i].value);
			break;
			case PCONF_STR:
				fprintf(fp,"%s\t\"%s\"\n",pconfig_param[i].name,(char *)pconfig_param[i].value);
			break;
			case PCONF_DOUBLE:
				fprintf(fp,"%s\t%f\n",pconfig_param[i].name,*(double *)pconfig_param[i].value);
			break;
			case PCONF_FLOAT:
				fprintf(fp,"%s\t%f\n",pconfig_param[i].name,*(float *)pconfig_param[i].value);
			break;
			case PCONF_BYTE:
				fprintf(fp,"%s\t%d\n",pconfig_param[i].name,*(unsigned char *)pconfig_param[i].value);
			break;
			case PCONF_UNSIGN:
				fprintf(fp,"%s\t%d\n",pconfig_param[i].name,*(unsigned *)pconfig_param[i].value);
			break;

			}
	}
	
	fprintf(fp,"\n#\n#\n#\n");
	
	fclose(fp);	

	return 0;
}

int pconfig_save(void)
{
	return pconfig_save_file(pconfig_cfgfile);
}

int pconfig_load(void)
{
	return pconfig_load_file(pconfig_cfgfile);
}


#ifdef TEST_CONFIG

int main(int argc,char *argv[])
{
	printf("Config parser\n");
	pconfig_load_file("test.cfg");
	pconfig_save_file("testout.cfg");
	
	printf ("triangle = %d\n",tri_button);
	printf ("circle = %d\n",cir_button);
	printf ("cross = %d\n",cro_button);
	printf ("square = %d\n",squ_button);
	printf("dir = %s\n",dirpath);
	printf("cpumhz = %f\n",cpumhz);
	
	return 0;
}

#endif

