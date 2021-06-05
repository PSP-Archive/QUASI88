#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

extern	int	menu_lang;			/* ��˥塼�θ���           */
extern	int	menu_readonly;			/* �ǥ������������������ */
						/* ������֤� ReadOnly ?    */
extern	int	menu_swapdrv;			/* �ɥ饤�֤�ɽ�����       */


extern	int	file_coding;			/* �ե�����̾�δ���������   */
extern	int	filename_synchronize;		/* �ե�����̾��ƱĴ������   */


	/* ��˥塼�⡼�� */

void	menu_main( void );



/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
int		about_msg_init( int japanese );
const char	*about_msg( void );



/*----------------------------------------------------------------------
 * ���٥�Ƚ������н�
 *----------------------------------------------------------------------*/
void	q8tk_event_key_on( int code );
void	q8tk_event_key_off( int code );
void	q8tk_event_mouse_on( int code );
void	q8tk_event_mouse_off( int code );
void	q8tk_event_mouse_moved( int x, int y );
void	q8tk_event_quit( void );




#endif	/* MENU_H_INCLUDED */
