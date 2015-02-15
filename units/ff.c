
#include <string.h>
#include <stdio.h>

#include "integer.h"
#include "ffconf.h"

#ifdef  _FATFS
#error "looks like ff.h was included somehow.."
#endif

#define _FATFS 0

#define FF_STR_LEN 64

/* File function return code (FRESULT) */
typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive cannot work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */
	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_SHARE */
	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;

/*--------------------------------------------------------------*/
/* Flags and offset address                                     */


/* File access control and file status flags (FIL.flag) */

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00

#if !_FS_READONLY
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#endif

/* Type of path name strings on FatFs API */
#if _LFN_UNICODE			/* Unicode string */
#if !_USE_LFN
#error _LFN_UNICODE must be 0 in non-LFN cfg.
#endif
#ifndef _INC_TCHAR
typedef WCHAR TCHAR;
#define _T(x) L ## x
#define _TEXT(x) L ## x
#endif

#else						/* ANSI/OEM string */
#ifndef _INC_TCHAR
typedef char TCHAR;
#define _T(x) x
#define _TEXT(x) x
#endif

#endif

typedef struct {
  uint8_t  mount_path[FF_STR_LEN];
  uint32_t is_mounted:1;
} FATFS;

typedef struct {
  uint8_t fpath[FF_STR_LEN<<1];
  FILE *fp;
  BYTE mode;
  uint32_t is_active:1;
} FIL;

typedef struct {
} DIR;

typedef struct {
} FILINFO;

static FATFS *_fs = NULL;

FRESULT f_mount (
	FATFS* fs,
	const TCHAR* path,
	BYTE opt
)
{
  if (NULL == fs) { /* unmount */
    _fs->mount_path[0] = 0;
    _fs->is_mounted = 0;
    return FR_OK;
  }
  /* mount */
  strncat(fs->mount_path, path, FF_STR_LEN);
  fs->is_mounted = 1;
  _fs = fs;
  return FR_OK;
}

FRESULT f_open (
	FIL* fp,
	const TCHAR* path,
	BYTE mode
)
{
  assert(NULL != _fs);
  sprintf(fp->fpath, "%s", path);

  /* */
  if (fp->is_active) {
    fclose(fp->fp);
    fp->is_active = 0;
  }

  /* */
  fp->fp = fopen(fp->fpath, (((FA_READ|FA_WRITE)&mode)==(FA_READ|FA_WRITE)) ? "a+" : (FA_READ&mode) ? "r":"a");
  if (NULL == fp->fp) {
    return FR_DISK_ERR;
  }
  fp->is_active = 1;
  fp->mode = mode;

  return FR_OK;
}

FRESULT f_read (
	FIL* fp,
	void* buff,
	UINT btr,
	UINT* br
)
{
  uint32_t size;
  
  size = fread(buff, 1, btr, fp->fp);
  *br = size;
  return (0 == size) ? FR_DISK_ERR : FR_OK;
}

#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_write (
	FIL* fp,			/* Pointer to the file object */
	const void *buff,	/* Pointer to the data to be written */
	UINT btw,			/* Number of bytes to write */
	UINT* bw			/* Pointer to number of bytes written */
)
{
  uint32_t size;
  
  size = fwrite(buff, 1, btw, fp->fp);
  *bw = size;
  return (0 == size) ? FR_DISK_ERR : FR_OK;
}


FRESULT f_sync (
	FIL* fp
)
{
  fflush(fp->fp);
}

#endif /* !_FS_READONLY */


FRESULT f_close (
	FIL *fp
)
{
  fclose(fp->fp);
  fp->is_active = 0;
}


#if _FS_RPATH >= 1
#if _VOLUMES >= 2
FRESULT f_chdrive (
	const TCHAR* path
)
{
  assert(0);
  return FR_DISK_ERR;
}
#endif


FRESULT f_chdir (
	const TCHAR* path
)
{
  assert(0);
  return FR_DISK_ERR;
}


#if _FS_RPATH >= 2
FRESULT f_getcwd (
	TCHAR* buff,	/* Pointer to the directory path */
	UINT len		/* Size of path */
)
{
  assert(0);
  return FR_DISK_ERR;
}
#endif /* _FS_RPATH >= 2 */
#endif /* _FS_RPATH >= 1 */



#if _FS_MINIMIZE <= 2
/*-----------------------------------------------------------------------*/
/* Seek File R/W Pointer                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_lseek (
	FIL* fp,		/* Pointer to the file object */
	DWORD ofs		/* File pointer from top of file */
)
{
  fseek(fp->fp, ofs, SEEK_SET);
  return FR_OK;
}

UINT f_eof(FIL *fp)
{
  return feof(fp->fp);
}

FRESULT f_error(FIL* fp)
{
  return ferror(fp->fp);
}

DWORD f_tell(FIL *fp)
{
  return ftell(fp->fp);
}

DWORD f_size(FIL *fp)
{
  DWORD size, temp;

  temp = ftell(fp->fp);
  fseek(fp->fp, 0L, SEEK_END);
  size = ftell(fp->fp);
  fseek(fp->fp, temp, SEEK_SET);

  return size;
}

#if _FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Create a Directory Object                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_opendir (
	DIR* dp,			/* Pointer to directory object to create */
	const TCHAR* path	/* Pointer to the directory path */
)
{
  assert(0);
  return FR_DISK_ERR;
}


FRESULT f_closedir (
	DIR *dp		/* Pointer to the directory object to be closed */
)
{
  assert(0);
  return FR_DISK_ERR;
}




/*-----------------------------------------------------------------------*/
/* Read Directory Entries in Sequence                                    */
/*-----------------------------------------------------------------------*/

FRESULT f_readdir (
	DIR* dp,			/* Pointer to the open directory object */
	FILINFO* fno		/* Pointer to file information to return */
)
{
  assert(0);
  return FR_DISK_ERR;
}



#if _FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/

FRESULT f_stat (
	const TCHAR* path,	/* Pointer to the file path */
	FILINFO* fno		/* Pointer to file information to return */
)
{
  assert(0);
  return FR_DISK_ERR;
}



#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/

FRESULT f_getfree (
	const TCHAR* path,	/* Path name of the logical drive number */
	DWORD* nclst,		/* Pointer to a variable to return number of free clusters */
	FATFS** fatfs		/* Pointer to return pointer to corresponding file system object */
)
{
  assert(0);
  return FR_DISK_ERR;
}

FRESULT f_truncate (
	FIL* fp		/* Pointer to the file object */
)
{
  assert(0);
  return FR_DISK_ERR;
}

FRESULT f_unlink (
	const TCHAR* path		/* Pointer to the file or directory path */
)
{
  uint8_t fpath[FF_STR_LEN<<2];
  
  fpath[0] = 0;
  sprintf(fpath, "rm -rf %s/%s", _fs->mount_path, path);

  return 0 == system(fpath) ? FR_OK : FR_DISK_ERR;
}

FRESULT f_mkdir (
	const TCHAR* path		/* Pointer to the directory path */
)
{
  uint8_t fpath[FF_STR_LEN<<2];
  
  fpath[0] = 0;
  sprintf(fpath, "mkdir %s/%s", _fs->mount_path, path);

  return 0 == system(fpath) ? FR_OK : FR_DISK_ERR;
}




/*-----------------------------------------------------------------------*/
/* Change Attribute                                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_chmod (
	const TCHAR* path,	/* Pointer to the file path */
	BYTE value,			/* Attribute bits */
	BYTE mask			/* Attribute mask to change */
)
{
  assert(0);
  return FR_DISK_ERR;
}

FRESULT f_utime (
	const TCHAR* path,	/* Pointer to the file/directory name */
	const FILINFO* fno	/* Pointer to the time stamp to be set */
)
{
  assert(0);
  return FR_DISK_ERR;
}

FRESULT f_rename (
	const TCHAR* path_old,	/* Pointer to the object to be renamed */
	const TCHAR* path_new	/* Pointer to the new name */
)
{
  assert(0);
  return FR_DISK_ERR;
}

#endif /* !_FS_READONLY */
#endif /* _FS_MINIMIZE == 0 */
#endif /* _FS_MINIMIZE <= 1 */
#endif /* _FS_MINIMIZE <= 2 */



#if _USE_LABEL
/*-----------------------------------------------------------------------*/
/* Get volume label                                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_getlabel (
	const TCHAR* path,	/* Path name of the logical drive number */
	TCHAR* label,		/* Pointer to a buffer to return the volume label */
	DWORD* sn			/* Pointer to a variable to return the volume serial number */
)
{
  assert(0);
  return FR_DISK_ERR;
}



#if !_FS_READONLY

FRESULT f_setlabel (
	const TCHAR* label	/* Pointer to the volume label to set */
)
{
  assert(0);
  return FR_DISK_ERR;
}

#endif /* !_FS_READONLY */
#endif /* _USE_LABEL */


#if _USE_FORWARD && _FS_TINY

FRESULT f_forward (
	FIL* fp, 						/* Pointer to the file object */
	UINT (*func)(const BYTE*,UINT),	/* Pointer to the streaming function */
	UINT btf,						/* Number of bytes to forward */
	UINT* bf						/* Pointer to number of bytes forwarded */
)
{
  assert(0);
  return FR_DISK_ERR;
}
#endif /* _USE_FORWARD */



#if _USE_MKFS && !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Create File System on the Drive                                       */
/*-----------------------------------------------------------------------*/
#define N_ROOTDIR	512		/* Number of root directory entries for FAT12/16 */
#define N_FATS		1		/* Number of FAT copies (1 or 2) */


FRESULT f_mkfs (
	const TCHAR* path,	/* Logical drive number */
	BYTE sfd,			/* Partitioning rule 0:FDISK, 1:SFD */
	UINT au				/* Allocation unit [bytes] */
)
{
  assert(0);
  return FR_DISK_ERR;
}



#if _MULTI_PARTITION

FRESULT f_fdisk (
	BYTE pdrv,			/* Physical drive number */
	const DWORD szt[],	/* Pointer to the size table for each partitions */
	void* work			/* Pointer to the working buffer */
)
{
  assert(0);
  return FR_DISK_ERR;
}


#endif /* _MULTI_PARTITION */
#endif /* _USE_MKFS && !_FS_READONLY */




#if _USE_STRFUNC
/*-----------------------------------------------------------------------*/
/* Get a string from the file                                            */
/*-----------------------------------------------------------------------*/

TCHAR* f_gets (
	TCHAR* buff,	/* Pointer to the string buffer to read */
	int len,		/* Size of string buffer (characters) */
	FIL* fp			/* Pointer to the file object */
)
{
  assert(0);
  return FR_DISK_ERR;
}



#if !_FS_READONLY
#include <stdarg.h>
/*-----------------------------------------------------------------------*/
/* Put a character to the file                                           */
/*-----------------------------------------------------------------------*/

int f_putc (
	TCHAR c,	/* A character to be output */
	FIL* fp		/* Pointer to the file object */
)
{
  assert(0);
  return FR_DISK_ERR;
}


int f_puts (
	const TCHAR* str,	/* Pointer to the string to be output */
	FIL* fp				/* Pointer to the file object */
)
{
  assert(0);
  return FR_DISK_ERR;
}

#define f_printf(FP, ...) fprintf(FP->fp, ...)

#endif /* !_FS_READONLY */
#endif /* _USE_STRFUNC */
