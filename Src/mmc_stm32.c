/*
 * mmc_stm32.c
 *
 *  Created on: 30 Jul 2018
 *      Author: Peter Mather
 */


/*------------------------------------------------------------------------/
/  MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include "Hardware_Includes.h"
#include "SPI-LCD.h"
#include "diskio.h"
//#include "integer.h"
int SPISpeed=0xFF;
#define CD	MDD_SDSPI_CardDetectState()	/* Card detected   (yes:true, no:false, default:true) */
#define WP	MDD_SDSPI_WriteProtectState()		/* Write protected (yes:true, no:false, default:false) */
/* SPI bit rate controls */
static int SD_SPI_SPEED=SD_SLOW_SPI_SPEED;
#define	FCLK_SLOW()		{SD_SPI_SPEED=SD_SLOW_SPI_SPEED;SPISpeedSet(SD_SPI_SPEED);}	/* Set slow clock (100k-400k) */
#define	FCLK_FAST()		{SD_SPI_SPEED=SD_FAST_SPI_SPEED;SPISpeedSet(SD_SPI_SPEED);}/* Set fast clock (depends on the CSD) */
extern SPI_HandleTypeDef SD_SPI;
extern volatile BYTE SDCardStat;
extern void RtcGetTime(void);
extern volatile int diskcheckrate;
const uint8_t high[512]={[0 ... 511]=0xFF};
int CurrentSPISpeed=NONE_SPI_SPEED;
BYTE (*xchg_byte) (BYTE data_out);
void (*xmit_byte_multi) (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
);
void (*rcvr_byte_multi) (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
);
/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
#define ACMD41 (41|0x80)	/* SEND_OP_COND (SDC) */
#define CMD6   (6)			/* SEND_IF_COND */
#define CMD8   (8)			/* SEND_IF_COND */
#define CMD9   (9)			/* SEND_CSD */
#define CMD10  (10)			/* SEND_CID */
#define CMD12  (12)			/* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)	/* SD_STATUS (SDC) */
#define CMD16  (16)			/* SET_BLOCKLEN */
#define CMD17  (17)			/* READ_SINGLE_BLOCK */
#define CMD18  (18)			/* READ_MULTIPLE_BLOCK */
#define CMD23  (23)			/* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)			/* WRITE_BLOCK */
#define CMD25  (25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)			/* SEND_OP_COND (ACMD) */
#define CMD55  (55)			/* APP_CMD */
#define CMD58  (58)			/* READ_OCR */

unsigned char CRC7(const unsigned char message[], const unsigned int length) {
  const unsigned char poly = 0b10001001;
  unsigned char crc = 0;
  for (unsigned i = 0; i < length; i++) {
     crc ^= message[i];
     for (int j = 0; j < 8; j++) {
      // crc = crc & 0x1 ? (crc >> 1) ^ poly : crc >> 1;
      crc = (crc & 0x80u) ? ((crc << 1) ^ (poly << 1)) : (crc << 1);
    }
  }
  //return crc;
  return crc >> 1;
}

static
UINT CardType;
BYTE MDD_SDSPI_CardDetectState(void){
            return (SD_CD_GPIO_Port->IDR & SD_CD_Pin? 0: 1);
}
BYTE MDD_SDSPI_WriteProtectState(void)
{
	return (SD_WP_GPIO_Port->IDR & SD_WP_Pin? 1: 0);
}

void BitBangSendSPI(BYTE *buff, int cnt){
	int i, SPICount;
	BYTE SPIData;
    if(SD_SPI_SPEED==SD_SLOW_SPI_SPEED){
    	for(i=0;i<cnt;i++){
    		SPIData=buff[i];
    		for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock out the Address byte
    		{
    			if (SPIData & 0x80)                                 // Check for a 1
    				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
    			else
    				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin<<16;
    			uSec(10);
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin;
    			uSec(10);
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
    			SPIData <<= 1;                                      // Rotate to get the next bit
    		}  // and loop back to send the next bit
    	}
	} else {
    	for(i=0;i<cnt;i++){
    		SPIData=buff[i];
    		for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock out the Address byte
    		{
    			if (SPIData & 0x80)                                 // Check for a 1
    				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
    			else
    				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin<<16;
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin;
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
    			SPIData <<= 1;                                      // Rotate to get the next bit
    		}  // and loop back to send the next bit
    	}
	}
}
void BitBangReadSPI(BYTE *buff, int cnt){
	int i, SPICount;
	BYTE SPIData;
	SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
    if(SD_SPI_SPEED==SD_SLOW_SPI_SPEED){
    	for(i=0;i<cnt;i++){
    		SPIData = 0;
    		for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock in the data to be fread
    		{
    			SPIData <<=1;                                       // Rotate the data
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin;
    			uSec(10);
    			SPIData += (SD_MISO_GPIO_Port->IDR & SD_MISO_Pin? 1: 0);                       // Read the data bit
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
    			uSec(10);
    		}                                                     // and loop back
    		buff[i]=SPIData;
    	}
    } else {
    	for(i=0;i<cnt;i++){
    		SPIData = 0;
    		for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock in the data to be fread
    		{
    			SPIData <<=1;                                       // Rotate the data
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin;
    			SPIData += (SD_MISO_GPIO_Port->IDR & SD_MISO_Pin? 1: 0);                       // Read the data bit
    			SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
    		}                                                     // and loop back
    		buff[i]=SPIData;
    	}
    }
}

BYTE BitBangSwapSPI(BYTE data_out){
	BYTE data_in=0;
	int SPICount;
	if(SD_SPI_SPEED==SD_SLOW_SPI_SPEED){
		for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock in the data to be fread
		{
			if (data_out & 0x80)                                 // Check for a 1
				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
			else
				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin<<16;
	    	uSec(10);
	    	data_in <<=1;                                       // Rotate the data
	    	SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin;
	    	uSec(10);
	    	data_in += (SD_MISO_GPIO_Port->IDR & SD_MISO_Pin? 1: 0);                       // Read the data bit
	    	SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
	    	data_out <<= 1;
		}                                                     // and loop back
		return data_in;
	} else {
		for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock in the data to be fread
		{
			if (data_out & 0x80)                                 // Check for a 1
				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
			else
				SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin<<16;
	    	data_in <<=1;                                       // Rotate the data
	    	SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin;
	    	data_in += (SD_MISO_GPIO_Port->IDR & SD_MISO_Pin? 1: 0);                       // Read the data bit
	    	SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
	    	data_out <<= 1;
		}                                                     // and loop back
		return data_in;
	}
}
/*-----------------------------------------------------------------------*/
/* Transmit/Receive data to/from MMC via SPI  (Platform dependent)       */
/*-----------------------------------------------------------------------*/

/* Single byte SPI transfer */

BYTE xchg_spi (BYTE data_out)
{
    BYTE   clear;
    HAL_SPI_TransmitReceive(&SD_SPI,&data_out,&clear,1,500);
    return clear;                // return non-negative#
}
BYTE xchg_bitbang (BYTE data_out)
{
	return BitBangSwapSPI(data_out);
}

// Block SPI transfers


void xmit_spi_multi (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
)
{
	HAL_SPI_Transmit(&SD_SPI,(uint8_t *)buff,cnt,500);
	routinechecks(0);
}
void xmit_bitbang_multi (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
)
{
	BitBangSendSPI((BYTE *)buff,cnt);
	routinechecks(0);
}

void rcvr_spi_multi (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
)
{
	HAL_SPI_TransmitReceive(&SD_SPI,(uint8_t *)high,buff,cnt,500);
	routinechecks(0);
}
void rcvr_bitbang_multi (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
)
{
	BitBangReadSPI(buff,cnt);
	routinechecks(0);
}


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void)
{
	BYTE d;

	Timer2 = 500;	/* Wait for ready in timeout of 500ms */
	do{
		d = xchg_byte(0xFF);
        uSec(5);
	} while ((d != 0xFF) && Timer2);

	return (d == 0xFF) ? 1 : 0;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
	HAL_GPIO_WritePin(SD_CE_GPIO_Port, SD_CE_Pin, GPIO_PIN_SET);
	xchg_byte(0xFF);		/* Dummy clock (force DO hi-z for multiple slave SPI) */
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait ready                                        */
/*-----------------------------------------------------------------------*/


int selectSD (void)	/* 1:Successful, 0:Timeout */
{
	HAL_GPIO_WritePin(SD_CE_GPIO_Port, SD_CE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SD_LED_GPIO_Port, SD_LED_Pin,GPIO_PIN_SET);
	SD_LED_Triggered=10;
    uSec(5);
    xchg_byte(0xFF);		/* Dummy clock (force DO enabled) */

	if (wait_ready()) return 1;	/* Wait for card ready */

	deselect();
	return 0;	/* Timeout */
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/


int rcvr_datablock (	/* 1:OK, 0:Failed */
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;


	Timer1 = 100;
	do {							/* Wait for data packet in timeout of 100ms */
		token = xchg_byte(0xFF);
	} while ((token == 0xFF) && Timer1);

	if(token != 0xFE) return 0;		/* If not valid data token, retutn with error */

	rcvr_byte_multi(buff, btr);		/* Receive the data block into buffer */
	xchg_byte(0xFF);					/* Discard CRC */
	xchg_byte(0xFF);
	diskcheckrate=1; //successful read so reset check
	return 1;						/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/



int xmit_datablock (	/* 1:OK, 0:Failed */
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data token */
)
{
	BYTE resp;


	if (!wait_ready()) return 0;

	xchg_byte(token);		/* Xmit a token */
	if (token != 0xFD) {	/* Not StopTran token */
		xmit_byte_multi(buff, 512);	/* Xmit the data block to the MMC */
		xchg_byte(0xFF);				/* CRC (Dummy) */
		xchg_byte(0xFF);
		resp = xchg_byte(0xFF);		/* Receive a data response */
		if ((resp & 0x1F) != 0x05)	/* If not accepted, return with error */
			return 0;
	}
	diskcheckrate=1; //successful write so reset check

	return 1;
}




/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/


BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;

	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready except to stop multiple block read */
	if (cmd != CMD12) {
		deselect();
		if (!selectSD()){
            return 0xFF;
        }
	}
	/* Send command packet */
	if(Option.SDspeed==0){
		xchg_byte(0x40 | cmd);			/* Start + Command index */
		xchg_byte((BYTE)(arg >> 24));	/* Argument[31..24] */
		xchg_byte((BYTE)(arg >> 16));	/* Argument[23..16] */
		xchg_byte((BYTE)(arg >> 8));		/* Argument[15..8] */
		xchg_byte((BYTE)arg);			/* Argument[7..0] */
		n = 0x01;						/* Dummy CRC + Stop */
		if (cmd == CMD0) n = 0x95;		/* Valid CRC for CMD0(0) + Stop */
		if (cmd == CMD8) n = 0x87;		/* Valid CRC for CMD8(0x1AA) + Stop */
		xchg_byte(n);
	} else {
		BYTE command[6];
		command[0]=(0x40 | cmd);			// Start + Command index
		command[1]=(arg >> 24);	// Argument[31..24]
		command[2]=(arg >> 16);	// Argument[23..16]
		command[3]=(arg >> 8);		// Argument[15..8]
		command[4]=(arg);			// Argument[7..0]
		command[5]=(CRC7(command, 5)<<1) | 1;
		HAL_SPI_Transmit(&SD_SPI,command,6,500);
	}
	/* Receive command response */
	if (cmd == CMD12) xchg_byte(0xFF);	/* Skip a stuff byte on stop to read */
	n = 100;							/* Wait for a valid response in timeout of 10 attempts */
	do
		res = xchg_byte(0xFF);
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	if (pdrv != 0) return STA_NOINIT;	/* Supports only single drive */

	return SDCardStat;
}

int CMD0send(void){
    char response,trys=100, responsetrys=10;
    do {
    	deselect();
    	HAL_GPIO_WritePin(SD_CE_GPIO_Port, SD_CE_Pin, GPIO_PIN_RESET);
    	HAL_GPIO_WritePin(SD_LED_GPIO_Port, SD_LED_Pin,GPIO_PIN_SET);
    	SD_LED_Triggered=10;
        trys--;
        xchg_byte(0x40);
        xchg_byte(0x0);
        xchg_byte(0x0);
        xchg_byte(0x0);
        xchg_byte(0x0);
        xchg_byte(0x95);
        do{
            response=xchg_byte(0xFF);
            responsetrys--;
        } while((responsetrys !=0) && (response !=1));
    } while((trys !=0) && (response !=1));
    return response;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	BYTE n, cmd, ty, ocr[4];


	if (pdrv != 0) return STA_NOINIT;	/* Supports only single drive */
	if (SDCardStat & STA_NODISK) return SDCardStat;	/* No card in the socket */
	FCLK_SLOW();
	deselect();							/* Initialize memory card interface */
	for (n = 10; n; n--) xchg_byte(0xFF);	/* 80 dummy clocks */
	ty = 0;

	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 1000;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2? */
//			MMPrintString("sdv2\r\n");
			for (n = 0; n < 4; n++) ocr[n] = xchg_byte(0xFF);			/* Get trailing return value of R7 resp */

			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 0x40000000));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {			/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = xchg_byte(0xFF);
					ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;	/* SDv2 */
				}
			}
		} else {							/* SDv1 or MMCv3 */
//			MMPrintString("sdv1\r\n");
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			while (Timer1 && send_cmd(cmd, 0));		/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set read/write block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	deselect();
/*
	int  k;
	send_cmd(CMD6,0x80FFFFF1);
	while((k=xchg_byte(0xFF)) & 0x80){}
	PInt(k);
    for (n=1; n<64; n++)
    	if(n % 16==0)PIntH(xchg_byte(0xFF));
		else if(n % 16==15){
			PIntHC(xchg_byte(0xFF));
			PRet();
		} else PIntHC(xchg_byte(0xFF));
    // and finally do a switch to high speed
    for (n=0; n<8; n++)
    	xchg_byte(0xFF);
    deselect();*/
	if (ty) {		/* Function succeded */
		SDCardStat &= ~STA_NOINIT;	/* Clear STA_NOINIT */
		FCLK_FAST();
	}

	return SDCardStat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	if (pdrv || !count) return RES_PARERR;
	if (SDCardStat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {		/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/


DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
	if (pdrv || !count) return RES_PARERR;
	if (SDCardStat & STA_NOINIT) return RES_NOTRDY;
	if (SDCardStat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {		/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}




/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/


DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	DWORD csz;

	if (pdrv) return RES_PARERR;
	if (SDCardStat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;
	switch (cmd) {
	case CTRL_SYNC :	/* Flush write-back cache, Wait for end of internal process */
		if (selectSD()) res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (WORD) */
		if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDv2? */
				csz = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(DWORD*)buff = csz << 10;
			} else {					/* SDv1 or MMCv3 */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csz = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = csz << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
		if (CardType & CT_SD2) {	/* SDv2? */
			if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
				xchg_byte(0xFF);
				if (rcvr_datablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) xchg_byte(0xFF);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDv1 or MMCv3 */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
				if (CardType & CT_SD1) {	/* SDv1 */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMCv3 */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
		*ptr = CardType;
		res = RES_OK;
		break;

	case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
		if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
			&& rcvr_datablock(buff, 16))
			res = RES_OK;
		break;

	case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
		if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
			&& rcvr_datablock(buff, 16))
			res = RES_OK;
		break;

	case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
		if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
			for (n = 0; n < 4; n++)
				*((BYTE*)buff+n) = xchg_byte(0xFF);
			res = RES_OK;
		}
		break;

	case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
		if ((CardType & CT_SD2) && send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
			xchg_byte(0xFF);
			if (rcvr_datablock(buff, 64))
				res = RES_OK;
		}
		break;

	case CTRL_POWER :	/* Power off */
		SDCardStat |= STA_NOINIT;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	deselect();

	return res;
}




/*-----------------------------------------------------------------------*/
/* Device Timer Driven Procedure                                         */
/*-----------------------------------------------------------------------*/
/* This function must be called by timer interrupt in period of 1ms      */

void disk_timerproc (void)
{
	BYTE s;
	UINT n;


	n = Timer1;					/* 1000Hz decrement timer with zero stopped */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;


	/* Update socket status */

	s = SDCardStat;

	if (WP) s |= STA_PROTECT;
	else	s &= ~STA_PROTECT;

	if (CD) s &= ~STA_NODISK;
	else	s |= (STA_NODISK | STA_NOINIT);

	SDCardStat = s;
}

DWORD get_fattime(void){
    DWORD i;
    RtcGetTime();
    i = ((year-1980) & 0x7F)<<25;
    i |= (month & 0xF)<<21;
    i |= (day & 0x1F)<<16;
    i |= (hour & 0x1F)<<11;
    i |= (minute & 0x3F)<<5;
    i |= (second/2 & 0x1F);
    return i;
}
void InitFileIO(void) {
	if(Option.SDspeed==0){
		  GPIO_InitTypeDef GPIO_InitStruct = {0};
		  GPIO_InitStruct.Pin = SD_MISO_Pin; //SD card MISO
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		  GPIO_InitStruct.Pull = GPIO_PULLUP;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		  HAL_GPIO_Init(SD_MISO_GPIO_Port, &GPIO_InitStruct);

		  SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
		  GPIO_InitStruct.Pin = SD_CLK_Pin; //SD card CLK
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		  HAL_GPIO_Init(SD_CLK_GPIO_Port, &GPIO_InitStruct);
		  SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;

		  SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
		  GPIO_InitStruct.Pin = SD_MOSI_Pin; //SD card MOSI
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		  HAL_GPIO_Init(SD_MOSI_GPIO_Port, &GPIO_InitStruct);
		  SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
	}
	CurrentSPISpeed=NONE_SPI_SPEED;
}

void SPISpeedSet(int speed){
if(Option.SDspeed){
    if(CurrentSPISpeed != speed){
        if(speed==SD_SLOW_SPI_SPEED){
    		CurrentSPISpeed=SD_SLOW_SPI_SPEED;
    		SD_SPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_256;
    	} else if(speed==SD_FAST_SPI_SPEED){
    		CurrentSPISpeed=SD_FAST_SPI_SPEED;
    		SD_SPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_2;
    	}
        WRITE_REG(SD_SPI.Instance->CFG1, (SD_SPI.Init.BaudRatePrescaler | SD_SPI.Init.CRCCalculation |
        		SD_SPI.Init.FifoThreshold     | SD_SPI.Init.DataSize));
    	}
	}
}
// set the chip select for the SPI to low (enabled)
// if the SPI is currently set to a different mode or baudrate this will change it accordingly
// also, it checks if the chip select pin needs to be changed
void SpiCsLow(int pin, int speed) {
	SPISpeedSet(speed);
    if(pin) PinSetBit(pin, LATCLR);                                 // set CS low
}


// set the chip select for SPI to high (disabled)
void SpiCsHigh(int pin) {
    if(pin) PinSetBit(pin, LATSET);                                 // set CS high
}



