#ifndef WAIT_H_INCLUDED
#define WAIT_H_INCLUDED

/************************************************************************/
/* ��������Ĵ�����ѿ�							*/
/************************************************************************/

extern	int	wait_rate;			/* ��������Ĵ�� ��Ψ    [%]  */
extern	int	wait_by_sleep;			/* ��������Ĵ���� sleep ���� */

extern	long	wait_sleep_min_us;		/* �Ĥ� idle���֤����� us�ʲ���
						   ���ϡ� sleep �������Ԥġ�
						   (MAX 1�� = 1,000,000us) */



/************************************************************************/
/* ��������Ĵ���Ѵؿ�							*/
/************************************************************************/
int	wait_vsync_init( void );
void	wait_vsync_term( void );

void	wait_vsync_reset( void );
void	wait_vsync( void );

void	wait_menu( void );


#endif	/* WAIT_H_INCLUDED */
