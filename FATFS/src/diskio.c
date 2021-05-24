/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
#include "sdio_sd.h"		/* Example: Header file of existing MMC/SDC contorl module */

/* Definitions of physical drive number for each drive */
#define ATA		0	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */

#define SDIO_DRV 0 /* disk number = 0 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	SD_Error s;

	if (pdrv != SDIO_DRV)
	return STA_NOINIT;
	
  s = SD_GetCardInfo(&SDCardInfo);
	if (s != SD_OK)
	return STA_NOINIT;

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	SD_Error s = SD_OK;

	if(pdrv != SDIO_DRV)
	return STA_NOINIT; /* Only support disk 0 */

	s = SD_Init();
	if (s == SD_OK)
	return 0;
	
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	SD_Error s;

	if ((pdrv != SDIO_DRV) || !count)
	return RES_PARERR;

	if (count == 1) 
	{
		s = SD_ReadBlock((void*)buff, sector*512, 512);
	} 
	else 
	{
		s = SD_ReadMultiBlocks((void*)buff, sector*512, 512, count);
	}
	s = SD_WaitReadOperation();
	while(SD_GetStatus() != SD_TRANSFER_OK) {}

	return (s != SD_OK) ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	SD_Error s;

	if ((pdrv != SDIO_DRV) || !count)
		return RES_PARERR;

	if (count == 1) 
	{
		s = SD_WriteBlock((void*)buff, sector*512, 512);
	} 
	else 
	{
		s = SD_WriteMultiBlocks((void*)buff, sector*512, 512, count);
	}
	s = SD_WaitWriteOperation();
	while(SD_GetStatus() != SD_TRANSFER_OK) {}

	return (s != SD_OK) ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
DRESULT r = RES_OK;

	if (pdrv != SDIO_DRV)
		return RES_PARERR;

	switch (cmd) {
	case CTRL_SYNC:
		/* No synchronization to do since not buffering
		 * in this module */
		r = RES_OK;
		break;
	case GET_SECTOR_SIZE:
		*(WORD *)buff = 512;
		r = RES_OK;
		break;
	case GET_SECTOR_COUNT:
		*(WORD *)buff = SDCardInfo.CardCapacity/512;
		r = RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*(WORD *)buff = SDCardInfo.CardBlockSize;
		break;
	default:
		r = RES_OK;
		break;
	}
	return r;
}

DWORD get_fattime(void)
{
	/* bit 31..25 年 0-127 从1980算起
	 * bit 24..21 月 1-12
	 * bit 20..16 日 1-31
	 * bit 15..11 时 0-23
	 * bit 10..5  分 0-59
	 * bit 4..0   秒/2 0-29 */
	enum {
		YEAR = 2016-1980,
		MON = 6,
		DAY = 1,
		HOUR = 8,
		MINUTE = 0,
		SECOND = 0,
	};
	DWORD t = (YEAR << 25) | \
		  (MON << 21) | \
		  (DAY << 16) | \
		  (HOUR << 11) | \
		  (MINUTE << 5) | \
		  (SECOND/2);
	return t;
}
