#ifndef	FILENAME_H_INCLUDED
#define	FILENAME_H_INCLUDED


#ifndef	QUASI88_FMAC
#define	QUASI88_FMAC
#endif


/* 設定ファイル名 */

#ifndef	CONFIG_SUFFIX
#define	CONFIG_SUFFIX		".prefs"
#endif

#ifndef	CONFIG_FILENAME
#define	CONFIG_FILENAME		"quasi88"
#endif

/* サスペンドのファイル名 */

#ifndef	STATE_SUFFIX
#define	STATE_SUFFIX		".sta"
#endif

#ifndef	STATE_FILENAME
#define	STATE_FILENAME		"quasi88"
#endif

/* スナップショットのファイル名(ベース) */

#ifndef	SNAPSHOT_FILENAME
#define	SNAPSHOT_FILENAME	"save"
#endif



/* パス・ファイル名の最大長さ ? */
#define	OSD_MAX_FILENAME	(255)


#endif	/* FILENAME_H_INCLUDED */
