
/***********************************************************************************************************************
MMBasic

Hardware_Includes.h

Defines the hardware aspects for PIC32-Generic MMBasic.

Copyright 2011 - 2021 Geoff Graham.  All Rights Reserved.
Copyright 2016 - 2021 Peter Mather.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.
- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "configuration.h"
#include "gifdec.h"
#include "IOPorts.h"
#include "Timers.h"
#include "SPI-LCD.h"
#include "ff.h"
#include "dma2d.h"
#include "ltdc.h"
	#include "usbh_core.h"
	#include "usbh_hid.h"
	#include "usbh_hid_parser.h"
	#include "usb_host.h"
    // global variables
    extern int MMCharPos;
    extern char *InterruptReturn;
    extern char IgnorePIN;
    extern char WatchdogSet;
    extern char oc1, oc2, oc3, oc4, oc5;
    extern char canopen,canmode;
    extern uint8_t OptionConsole;
    extern volatile MMFLOAT VCC;
    extern int PromptFont, PromptFC, PromptBC;                          // the font and colours selected at the prompt;
    extern void TM_USART2_ReceiveHandler(uint8_t c);
    extern int ShortScroll;
    extern volatile unsigned int WDTimer;                               // used for the watchdog timer
    extern volatile unsigned int ScrewUpTimer;                               // used for the screwup timer
//    extern TM_USART_t TM_USART2;
//    extern TM_USART_t TM_USART3;
    extern int BasicRunning;
	#define PROG_FLASH_SIZE 0x80000
    // console related I/O
    int MMInkey(void);
    int MMgetchar(int update);
    char MMputchar(char c);
    extern void CheckAbort(void) ;
//    extern void TM_USART_INT_InsertToBuffer(TM_USART_t* u, uint8_t c);
    int kbhitConsole(void);
    void putConsole(int c);
    void SoftReset(void);
    extern void MM_Delay(int n);
    extern void SerUSBPutS(char *s);
    extern void SerUSBPutC(char c);
    extern void MX470PutS(char *s, int fc, int bc);
    extern void SaveProgramToFlash(char *pm, int msg, char *fname, int size);
    extern uint32_t SaveProgramToMemory(char *pm, int msg, char *fname);
	extern int VideoMode, VideoColour, VideoBackground;
    int getConsole(void);
    void initSerialConsole(void);
    extern int pattern_matching (	/* 0:not matched, 1:matched */
    	const TCHAR* pat,	/* Matching pattern */
    	const TCHAR* nam,	/* String to be tested */
    	int skip,			/* Number of pre-skip chars (number of ?s) */
    	int inf				/* Infinite search (* specified) */
    );
    // Use the core timer.  The maximum delay is 4 seconds
    void shortpause(unsigned int ticks);
    extern void varnamecopy(char *out, char *in);
    extern void mycpy(void *out, const void *in, int n);
    extern void myset(void *out, uint32_t in, int n);
    extern void mymemset(void *out, uint32_t in, int n);
    extern void mycopy(void *out, const void *in, int n);
    extern void zcopy(void *out, const void *in, int n);
    extern void _Z10copy_wordsPKmPmm(uint32_t *s, uint32_t *d, int n);
    extern uint64_t *clearpage(void *out);
    extern void cleardims(void *vardim);
    extern void clearvar(void *var);
    // used to control the processor reset
    extern unsigned int _excep_dummy;//  __attribute__ ((persistent)); // for some reason persistent does not work on the first variable
    extern unsigned int _excep_code;//  __attribute__ ((persistent));  // if there was an exception this is the exception code
    extern unsigned int _excep_addr;//  __attribute__ ((persistent));  // and this is the address
    extern void PRet(void);
    extern void PInt(int64_t n);
    extern void PO(char *s,int m);
    extern void PIntComma(int64_t n);
    extern void PO2Str(char *s1, const char *s2, int m);
    extern void PO2Int(char *s1, int64_t n);
    extern void PO3Int(char *s1, int64_t n1, int64_t n2);
    extern void PIntH(unsigned long long int n);
    extern void PIntHC(unsigned long long int n);
    extern void PFlt(MMFLOAT flt);
    extern void PFltComma(MMFLOAT n);
    extern void SRet(void);
    extern void SInt(int64_t n);
    extern void SIntComma(int64_t n);
    extern void SIntH(unsigned long long int n);
    extern void SIntHC(unsigned long long int n);
    extern void SFlt(MMFLOAT flt);
    extern void SFltComma(MMFLOAT n);
    extern void PPinName(int n);
    extern void PPinNameComma(int n);
    extern void MPU_Config_nCacheable(int jpg);
    extern int HRes,VRes;
    extern uint32_t ReadPageAddressExternal, WritePageAddressExternal;
    extern int hcursor;
    extern int wcursor;
	extern int xoffcursor;
    extern int yoffcursor;
    extern char *cursorsave;
    extern int G1Hardware;
    extern void (*docopy)(void *d, const void *s, int n);
    extern uint8_t DS3231_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len);
    extern void DS3231_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len);
    extern volatile int mouse0;
	extern volatile int nun1, nun2, nun3;
	extern volatile int classic1, classic2, classic3;
	extern volatile int mouse1, mouse2, mouse3;
    #ifdef __DEBUG
        void dump(char *p, int nbr);
    #endif
    extern void routinechecks(int all);
//    #define dp(...) {char s[140];sprintf(s,  __VA_ARGS__); MMPrintString(s); MMPrintString("\r\n");}
    #define db(i) {IntToStr(inpbuf, i, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define db2(i1, i2) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define db3(i1, i2, i3) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i3, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define ds(s) {MMPrintString(s); MMPrintString("\r\n");}
    #define ds2(s1, s2) {MMPrintString(s1); MMPrintString(s2); MMPrintString("\r\n");}
    #define ds3(s1, s2, s3) {MMPrintString(s1); MMPrintString(s2); MMPrintString(s3); MMPrintString("\r\n");}
    #define pp(i) { PinSetBit(i, TRISCLR); PinSetBit(i, ODCCLR); PinSetBit(i, LATCLR); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); }
    #define pp2(i) { PinSetBit(i, TRISCLR); PinSetBit(i, ODCCLR); PinSetBit(i, LATCLR); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); uSec(30); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); }
    #define pp3(i) { PinSetBit(i, TRISCLR); PinSetBit(i, ODCCLR); PinSetBit(i, LATCLR); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); uSec(30); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); uSec(30); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); }
#endif
#define VGA             1
#define SSD1963_4       2
#define SSD1963_5       3
#define SSD1963_5A      4
#define SSD1963_7       5
#define SSD1963_7A      6
#define SSD1963_8       7
#define SSD_PANEL_8     SSD1963_8    // anything less than or equal to SSD_PANEL is handled by the SSD 8-bit driver, anything more by the 16-bit or SPI driver

#define SSD1963_4_16    9
#define SSD1963_5_16    10
#define SSD1963_5A_16   11
#define SSD1963_7_16    12
#define SSD1963_7A_16   13
#define SSD1963_8_16    14
#define SSD_PANEL       SSD1963_8_16    // anything less than or equal to SSD_PANEL is handled by the SSD driver, anything more by the SPI driver

#define ILI9163         16
#define ST7735          17
#define SPI_PANEL       ST7735   // anything greater than SPI_PANEL is handled by otherdisplays

#define USER            19
#define ILI9481         20
#define ILI9341		    21

#define ILI9341_16      23
#define ILI9341_8       24
#define SSD1963_5_BUFF  25
#define SSD1963_7_BUFF  26
#define SSD1963_8_BUFF  27
#define SSD1963_5_640   28
#define SSD1963_7_640   29
#define SSD1963_8_640   30
#define SSD1963_5_8BIT  31
#define SSD1963_7_8BIT  32
#define SSD1963_8_8BIT  33
#define HDMI			34

#define LANDSCAPE       1
#define PORTRAIT        2
#define RLANDSCAPE      3
#define RPORTRAIT       4
#define TOUCH_NOT_CALIBRATED    -999999
#define RESET_COMMAND       9999                                // indicates that the reset was caused by the RESET command
#define WATCHDOG_TIMEOUT    9998                                // reset caused by the watchdog timer
#define PIN_RESTART         9997                                // reset caused by entering 0 at the PIN prompt
#define RESTART_NOAUTORUN   9996                                // reset required after changing the LCD or touch config
#define RESTART_BOOT0   	9995
#define SCREWUP_TIMEOUT    	9994                                // reset caused by the watchdog timer
#define RESTART_HEAP		9993								//reset caused by heap crash
#define SD_SLOW_SPI_SPEED 0
#define SD_FAST_SPI_SPEED 1
#define LCD_SPI_SPEED    2                                   // the speed of the SPI bus when talking to an SPI LCD display controller
#define TOUCH_SPI_SPEED 3
#define NONE_SPI_SPEED 4
#define TAB     	0x9
#define BKSP    	0x8
#define ENTER   	0xd
#define ESC     	0x1b

// the values returned by the function keys
#define F1      	0x91
#define F2      	0x92
#define F3      	0x93
#define F4      	0x94
#define F5      	0x95
#define F6      	0x96
#define F7      	0x97
#define F8      	0x98
#define F9      	0x99
#define F10     	0x9a
#define F11     	0x9b
#define F12     	0x9c

// the values returned by special control keys
#define UP			0x80
#define DOWN		0x81
#define LEFT		0x82
#define RIGHT		0x83
#define DOWNSEL     0xA1
#define RIGHTSEL    0xA3
#define INSERT		0x84
#define DEL			0x7f
#define HOME		0x86
#define END			0x87
#define PUP			0x88
#define PDOWN		0x89
#define NUM_ENT		ENTER
#define SLOCK		0x8c
#define ALT			0x8b
#define	SHIFT_TAB 	0x9F
#define SHIFT_DEL   0xa0
#define CTRLKEY(a) (a & 0x1f)
#define DISPLAY_CLS             1
#define REVERSE_VIDEO           3
#define CLEAR_TO_EOL            4
#define CLEAR_TO_EOS            5
#define SCROLL_DOWN             6
#define DRAW_LINE               7
#define CONFIG_TAB2		0b111
#define CONFIG_TAB4		0b001
#define CONFIG_TAB8		0b010
#define WPN 65   //Framebuffer page no.
#define BPN 64   //Framebuffer backup page no.
#define TPN 64   //temporary page no. for when duplicate lines need processing
#define TPN1 63   //temporary page no. for when duplicate lines need processing
#define TPN2 62   //temporary page no. for when duplicate lines need processing
#define MAXPAGES 60
#define IsxDigit(a) isxdigit((uint8_t)a)
#define IsDigit(a) isdigit((uint8_t)a)
#define IsAlpha(a) isalpha((uint8_t)a)
#define IsCntrl(a) iscntrl((uint8_t)a)
#define IsPrint(a) isprint((uint8_t)a)
#define IsAlnum(a) isalnum((uint8_t)a)
#include "Serial.h"
#include "FileIO.h"
#include "Memory.h"
#include "External.h"
#include "MM_Misc.h"
#include "MM_Custom.h"
#include "Onewire.h"
#include "I2C.h"
#include "SerialFileIO.h"
#include "PWM.h"
#include "SPI.h"
#include "CAN.h"
#include "Flash.h"
#include "Xmodem.h"
#include "Draw.h"
#include "MATHS.h"
#include "ff.h"
#include "diskio.h"
#include "audio.h"
#include "OtherDisplays.h"
#include "gps.h"
#include "keyboard.h"
#include "sprites.h"
#include "upng.h"
#include "turtle.h"
#include "gui.h"


