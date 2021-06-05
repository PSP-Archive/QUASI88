#ifndef	EVENT_H_INCLUDED
#define	EVENT_H_INCLUDED


/***********************************************************************
 * ���٥�Ƚ��� (�����ƥ��¸)
 ************************************************************************/

/******************************************************************************
 *
 * void event_handle_init( void )
 *	QUASI88 ��ư���˸ƤӽФ���롣
 *	�����ƥ��ͭ�Υ����Х���ǥ��󥰽����ʤɤϡ������Ǥ�äƤ�����
 *
 * void event_handle( void )
 *	��1/60���� (���ߥ�졼���� VSYNC��) �˸ƤӽФ���롣
 *	�����ǡ����ƤΥ��٥�Ƚ�����Ԥ������ߥ�졼���ؤ����Ϥ򹹿����롣
 *
 *	o ����������������
 *	  �����줿�����˱����ơ��ʲ��δؿ���ƤӽФ���
 *		������: pc88_key_press( KEY88_XXX, TRUE );
 *		������: pc88_key_press( KEY88_XXX, FALSE );
 *
 *	  ���λ��Ρ������줿������ KEY88_XXX ���б���Ǥ�� (�����¸) ������
 *	  �������б��򤷤Ƥ��ʤ� (������ƤƤ��ʤ�) KEY88_XXX �ˤĤ��Ƥϡ�
 *	  QUASI88 �������ԲĤȤʤ롣
 *
 *	  �äˡ�KEY_SPACE �� KEY88_TILDE �������ƤƤ��ʤ��ä���硢��˥塼
 *	  �⡼�ɤ� �����˳������� ASCIIʸ�������Ϥ�����ʤ��ʤ�Τ���ա�
 *
 *	o �ޥ����ܥ��󲡲���������
 *	  �����줿�ܥ���˱����ơ��ʲ��δؿ���ƤӽФ���
 *		������: pc88_mouse_press( KEY88_MOUSE_XXX, TRUE );
 *		������: pc88_mouse_press( KEY88_MOUSE_XXX, FALSE );
 *
 *	o ���祤���ƥ��å�������������
 *	  �����줿�ܥ���˱����ơ��ʲ��δؿ���ƤӽФ���
 *		������: pc88_pad_press( KEY88_PAD_XXX, TRUE );
 *		������: pc88_pad_press( KEY88_PAD_XXX, FALSE );
 *
 *	o �ޥ�����ư�� (���к�ɸ / ���к�ɸ)
 *	  ��ư�� x,y (�ʤ�����ư�� dx,xy) �˱����ơ��ʲ��δؿ���ƤӽФ���
 *		��ư��: pc88_mouse_moved_abs(  x,  y );
 *		��ư��: pc88_mouse_moved_rel( dx, dy );
 *
 *	  ���к�ɸ�ξ�硢���̥������� 640x400 �Ȥ��������ͤ򥻥åȤ��뤳�ȡ�
 *	  �Ĥޤꡢ�ܡ��������ϰ����Ƥ��� (�ͤ��ϰϤϡ�640x400��Ķ���Ƥ��)
 *
 *	o �ե����������꡿�ʤ���
 *	  ������ɥ����ե��������򼺤ä����˥ݡ����⡼�ɤ����ܤ��������ʤ顢
 *	  �ʲ��δؿ���ƤӽФ������ܤ��������ʤ��ʤ顢�ƤӽФ�ɬ�פʤ���
 *			pc88_focus_in() / pc88_focus_out()
 *
 *	o ������λ��
 *	  ������ɥ�������Ū���Ĥ���줿��硢�ʲ��δؿ���ƤӽФ���
 *			pc88_quit()
 *
 * int  numlock_on( void )
 * void numlock_off( void )
 *	���եȥ����� NumLock ��ͭ���ˤ��롿̵���ˤ���ݤˡ��ƤӽФ���롣
 *	�����ƥ�Υ�������ˤ˹�碌�ơ������Х���ǥ��󥰤��ѹ�����Ф褤��
 *	�ѹ��������ʤ����Ǥ��ʤ��ʤ�С� numlock_on() ������ͤ� �� �ˤ��롣
 *
 * void	init_mouse_position( int *x, int *y )
 *	���ߤΥޥ��������к�ɸ�򥻥åȤ��롣
 *	���к�ɸ�γ�ǰ���ʤ����ϡ�Ǥ�դ��ͤ򥻥åȤ��Ƥ�褤��
 *	���åȤ����ɸ�ͤϡ����̥������� 640x400 �Ȥ��������ͤ򥻥åȤ��뤳�ȡ�
 *	�Ĥޤꡢ�ܡ��������ϰ����Ƥ��� (�ͤ��ϰϤϡ�640x400��Ķ���Ƥ��)
 *	���δؿ��ϡ��⡼�ɤ��ڤ��ؤ����˸ƤӽФ���롣
 *
 * void	event_init( void )
 *	���ߥ�졼�ȥ⡼�ɡ���˥塼�⡼�ɡ��ݡ����⡼�ɡ���˥����⡼�ɤ�
 *	���ϻ��˸ƤӽФ���롣
 *	���ߥ�졼�ȥ⡼�ɰʳ��Ǥϥ���֤�������Ȥ����ޥ�����ɽ������ɽ����
 *	�ڤ��ؤ���Ȥ��ġġ�
 *
 *****************************************************************************/


		/**************************************/
		/*   �����ƥ��¸���̤��Ѱդ���ؿ�   */
		/**************************************/

/*----------------------------------------------------------------------
 * event_handle() �� 1/60��˸ƤӽФ����
 *----------------------------------------------------------------------*/
void	event_handle_init( void );
void	event_handle( void );


/*----------------------------------------------------------------------
 * ���եȥ����� NumLock ͭ��/̵�����ν���
 *----------------------------------------------------------------------*/
int	numlock_on( void );
void	numlock_off( void );


/*----------------------------------------------------------------------
 * ���ߤΥޥ��������к�ɸ���֤���
 *----------------------------------------------------------------------*/
void	init_mouse_position( int *x, int *y );


/*----------------------------------------------------------------------
 * �ƥ⡼�ɤγ��ϻ��Υ��٥�Ƚ����ƽ����
 *----------------------------------------------------------------------*/
void	event_init( void );




		/******************************************/
		/* �嵭�δؿ���������ƤӽФ��Ƥۤ����ؿ� */
		/******************************************/

/*----------------------------------------------------------------------
 * ���٥�Ƚ������н�
 *----------------------------------------------------------------------*/
void	pc88_key  ( int code, int on_flag );
void	pc88_mouse( int code, int on_flag );
void	pc88_pad  ( int code, int on_flag );
void	pc88_mouse_move ( int x, int y, int abs_flag );

void	pc88_focus_in( void );
void	pc88_focus_out( void );
void	pc88_quit( void );

#define	pc88_key_pressed( code )	pc88_key  ( code, TRUE  )
#define	pc88_key_released( code )	pc88_key  ( code, FALSE )
#define	pc88_mouse_pressed( code )	pc88_mouse( code, TRUE  )
#define	pc88_mouse_released( code )	pc88_mouse( code, FALSE )
#define	pc88_pad_pressed( code )	pc88_pad  ( code, TRUE  )
#define	pc88_pad_released( code )	pc88_pad  ( code, FALSE )
#define	pc88_mouse_moved_abs( x, y )	pc88_mouse_move( x, y, TRUE )
#define	pc88_mouse_moved_rel( x, y )	pc88_mouse_move( x, y, FALSE )



/* �ʲ��ϡ������¸��档�ƤӽФ��˾�郎����Ǥ���� */

void	quasi88_change_screen( void );
void	quasi88_snapshot( void );
void	quasi88_status( void );
void	quasi88_menu( void );
void	quasi88_pause( void );
void	quasi88_framerate_up( void );
void	quasi88_framerate_down( void );
void	quasi88_volume_up( void );
void	quasi88_volume_down( void );
void	quasi88_wait_up( void );
void	quasi88_wait_down( void );
void	quasi88_wait_none( void );
void	quasi88_drv1_image_empty( void );
void	quasi88_drv2_image_empty( void );
void	quasi88_drv1_image_next( void );
void	quasi88_drv1_image_prev( void );
void	quasi88_drv2_image_next( void );
void	quasi88_drv2_image_prev( void );

int	quasi88_disk_insert_and_reset( const char *filename, int ro );
int	quasi88_disk_insert_all( const char *filename, int ro );
int	quasi88_disk_insert( int drv, const char *filename, int image, int ro);
int	quasi88_disk_insert_A_to_B( int src_drv, int dst_drv, int dst_img );
void	quasi88_disk_eject_all( void );
void	quasi88_disk_eject( int drv );
int	quasi88_load_tape_insert( const char *filename );
int	quasi88_load_tape_rewind( void );
void	quasi88_load_tape_eject( void );
int	quasi88_save_tape_insert( const char *filename );
void	quasi88_save_tape_eject( void );

int	quasi88_serial_in_connect( const char *filename );
void	quasi88_serial_in_remove( void );
int	quasi88_serial_out_connect( const char *filename );
void	quasi88_serial_out_remove( void );
int	quasi88_printer_connect( const char *filename );
void	quasi88_printer_remove( void );



#endif	/* EVENT_H_INCLUDED */
