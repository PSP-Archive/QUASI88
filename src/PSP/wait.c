/************************************************************************/
/*									*/
/* ウエイト調整用関数 (OS依存)						*/
/*									*/
/* 【関数】								*/
/*									*/
/* int  wait_vsync_init( void )		初期化 (起動時に呼び出される)	*/
/* void wait_vsync_term( void )		終了   (終了時に呼び出される)	*/
/*									*/
/* void	wait_vsync_reset( void )	計測リセット (設定変更時)	*/
/* void wait_vsync( void )		ウェイトする (設定間隔経過待ち)	*/
/*									*/
/* void wait_menu( void )		メニュー用ウェイト(1/60sec待つ)	*/
/*									*/
/************************************************************************/
#include <stdio.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"
#include "suspend.h"

#include "screen.h"	/* auto_skip... */

#include <pspgu.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <pspkernel.h>

/*
 * 自動フレームスキップ		( by floi, thanks ! )
 */

static	int	skip_counter = 0;		/* 連続何回スキップしたか */
static	int	skip_count_max = 15;		/* これ以上連続スキップしたら
						   一旦、強制的に描画する */


/*
 * ウェイト処理関数群
 */

static	int	wait_counter = 0;		/* 連続何回時間オーバーしたか*/
static	int	wait_count_max = 10;		/* これ以上連続オーバーしたら
						   一旦,時刻調整を初期化する */

/* 時刻調整は、us単位で行なう。でも変数の型が long なので 4295 秒で値が
   戻って(wrap)しまい、この時の 1フレームはタイミングが狂う。
   なので、可能ならば 64bit型(long long)にしてみよう。 */

#ifdef SDL_HAS_64BIT_TYPE
typedef	Sint64		T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* 次フレームの時刻 */
static	T_WAIT_TICK	delta_time;		/* 1 フレームの時間 */



/* ---- 現在時刻を取得する (usec単位) ---- */

#define	GET_TICK()	( (T_WAIT_TICK)SDL_GetTicks() * 1000 )


u64 wait_lasttick;
u32 wait_tickperframe;



/****************************************************************************
 * ウェイト処理初期化
 *****************************************************************************/
int	wait_vsync_init( void )
{
  skip_counter = 0;
  sceRtcGetCurrentTick(&wait_lasttick);
  wait_tickperframe = sceRtcGetTickResolution() / 60;

  return TRUE;
}



/****************************************************************************
 * ウェイト処理終了
 *****************************************************************************/
void	wait_vsync_term( void )
{
}



/****************************************************************************
 * ウェイト処理再初期化
 *****************************************************************************/
void	wait_vsync_reset( void )
{
}



/****************************************************************************
 * ウェイト処理
 *****************************************************************************/
void	wait_vsync( void )
{
  u64 curtick;
  
  sceRtcGetCurrentTick(&curtick);

  if (curtick < wait_lasttick + wait_tickperframe)
  {
	skip_counter = 0;
	while(curtick < wait_lasttick + wait_tickperframe)
	  sceRtcGetCurrentTick(&curtick);


	do_skip_draw = FALSE;
  } else
  {
    if (skip_counter < skip_count_max)
	{
		skip_counter++;
		do_skip_draw = TRUE;
	} else
	{
		skip_counter = 0;
		do_skip_draw = FALSE;
		already_skip_draw = FALSE;
		reset_frame_counter();	
	}
  }
  wait_lasttick = curtick;
  

}



/****************************************************************************
 * メニュー用のウェイト
 *	約 1/60 秒ほど待つ。精度は不要だが、可能なら必ず sleep させるべし
 *****************************************************************************/
void	wait_menu( void )
{
	sceDisplayWaitVblankStart();
}
