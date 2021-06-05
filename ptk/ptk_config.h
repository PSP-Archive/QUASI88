#ifndef _PTK_CONFIG_H_
#define _PTK_CONFIG_H_

void pconfig_init(char *filename);
int  pconfig_load_file(char *filename);
int  pconfig_save_file(char *filename);
int  pconfig_save(void);
int  pconfig_load(void);

#endif
