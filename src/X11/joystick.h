/*
 *	Joystick �إå��ե�����
 */

#ifndef	JOYSTICK_H_INCLUDED
#define	JOYSTICK_H_INCLUDED

extern	int	enable_joystick;	/* ���祤���ƥ��å��λ��Ѳ��� */

void	joystick_init( void );
void	joystick_term( void );
void	joystick_event( void );


#endif	/* JOYSTICK_H_INCLUDED */
