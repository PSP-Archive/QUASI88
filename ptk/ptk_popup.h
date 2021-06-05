#ifndef _PTKPOPUP_H_
#define _PTKPOPUP_H_

void pop_init();
void pop_switch(int sw);
int pop_status(void);
void pop_click(void);
void pop_disp(void);
void pop_about(char *msg);
void pop_about_form(char *msg,...);

void pop_input(int pad);

void pop_debugf(char *msg,...);
void pop_debug_sw(int val);
void pop_debug_force(void);



#endif

