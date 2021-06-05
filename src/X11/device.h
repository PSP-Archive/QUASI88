#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <X11/Xlib.h>
#include <X11/Xutil.h>


extern	Display	*display;
extern	Window	window;
extern	Atom	atom_WM_close_type;
extern	Atom	atom_WM_close_data;



extern	int	SCREEN_DX;		/* ������ɥ�����ȡ�		*/
extern	int	SCREEN_DY;		/* ���̥��ꥢ����ȤΥ��ե��å�	*/

extern	int	status_fg;		/* ���ơ��������ʿ�		*/
extern	int	status_bg;		/* ���ơ������طʿ�		*/


extern	int	colormap_type;		/* ���顼�ޥåפΥ�����	0/1/2	*/
#ifdef MITSHM
extern	int	use_SHM;		/* MIT-SHM ����Ѥ��뤫�ɤ���	*/
#endif

extern	int	mouse_rel_move;		/* �ޥ������а�ư�̸��Τ����뤫	*/
extern	int	get_focus;		/* ���ߡ��ե����������꤫�ɤ���	*/

extern	int	use_xdnd;		/* XDnD ���б����뤫�ɤ���	*/
extern	int	use_joydevice;		/* ���祤���ƥ��å��ǥХ����򳫤�? */


void	x11_system_init( void );
void	x11_system_term( void );


void	xdnd_initialize( void );
void	xdnd_start( void );
void	xdnd_receive_drag( XClientMessageEvent *E );
void	xdnd_receive_drop( XSelectionEvent *E );


#endif	/* DEVICE_H_INCLUDED */
