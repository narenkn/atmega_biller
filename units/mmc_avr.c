/*-----------------------------------------------------------------------*/
/* MMCv3/SDv1/SDv2 (in SPI mode) control module  (C)ChaN, 2010           */
/*-----------------------------------------------------------------------*/

#include <avr/io.h>
#include "diskio.h"


/* Port controls  (Platform dependent) */
#define CS_LOW()	PORTB &= ~1			/* CS=low */
#define	CS_HIGH()	PORTB |= 1			/* CS=high */
#define SOCKINS		(!(PINB & 0x10))	/* Card detected.   yes:true, no:false, default:true */
#define SOCKWP		(PINB & 0x20)		/* Write protected. yes:true, no:false, default:false */
#define	FCLK_SLOW()	SPCR = 0x52		/* Set slow clock (F_CPU / 64) */
#define	FCLK_FAST()	SPCR = 0x50		/* Set fast clock (F_CPU / 2) */


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */


static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
BYTE Timer1, Timer2;	/* 100Hz decrement timer */

static
BYTE CardType;			/* Card type flags */

/* Implement a fake card
   128 KB Card
   cluster size of 512
   # clusters = 128K / 512 = 256
 */
#define SD_SIZE (1<<17)
#define SD_SECTOR_SIZE (1<<9)
#define SD_NUM_SECTORS (1<<8)

static uint8_t SD_MEM[SD_NUM_SECTORS][SD_SECTOR_SIZE];

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
  Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
  return Stat;
}


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	if (pdrv) return STA_NOINIT;	/* Supports only single drive */
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..128) */
)
{
  //  printf("Read to sector:%d, count:%d\n", sector, count);
  for (; count>0; count--) {
    memcpy(buff, SD_MEM[sector], SD_SECTOR_SIZE);
  }
  return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..128) */
)
{
  //  printf("Write to sector:%d, count:%d\n", sector, count);
  for (; count>0; count--) {
    memcpy(SD_MEM[sector], buff, SD_SECTOR_SIZE);
  }
  return count ? RES_ERROR : RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  DRESULT res;
  BYTE n, csd[16], *ptr = buff;
  DWORD *dp, st, ed, csize;


  /* Implements only drive 0 */
  if (pdrv) return RES_PARERR;

  res = RES_ERROR;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd) {
  case CTRL_SYNC :		/* Make sure that no pending write process. Do not remove this or written sector might not left updated. */
    res = RES_OK;
    break;

  case GET_SECTOR_SIZE :
    *(DWORD*)buff = SD_SECTOR_SIZE;
    res = RES_OK;
    break;

  case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
    *(DWORD*)buff = SD_NUM_SECTORS;
    res = RES_OK;
    break;

  case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
    *(DWORD*)buff = 8;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif

