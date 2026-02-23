/***********************************************************************************************************************
MMBasic

LCDDriver.h

Include file that contains the globals and defines for the LCD Driver.

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


extern void MIPS16 ConfigDisplaySPI(char *p);
extern void MIPS16 InitDisplaySPI(int InitOnly);
extern void MIPS16 SetAndReserve(int pin, int inp, int init, int type);
extern void DisplayNotSet(void);
extern void DrawRectangleSPI(int x1, int y1, int x2, int y2, int c);
extern void DrawBufferSPI(int x1, int y1, int x2, int y2, char* p);
extern void SpiCsLow(int pin, int speed);
extern void SPISpeedSet(int speed);
extern void DefineRegionSPI(int xstart, int ystart, int xend, int yend, int rw);
extern void SpiCsHigh(int pin);
extern void MIPS16 ResetController(void);
extern void ReadBufferSPI(int x1, int y1, int x2, int y2, char* p) ;
extern void spi_write_command(unsigned char data);
extern void spi_write_cd(unsigned char command, int data, ...);
extern void spi_write_data(unsigned char data);
extern void set_cs(void);
extern void DisplayPutC(char c);
extern void DisplayPutS(char *s);
extern void ShowCursor(int show);

#define ST7735_NOP              0x0
#define ST7735_SWRESET          0x01
#define ST7735_RDDID            0x04
#define ST7735_RDDST            0x09
#define ST7735_SLPIN            0x10
#define ST7735_SLPOUT           0x11
#define ST7735_PTLON            0x12
#define ST7735_NORON            0x13
#define ST7735_INVOFF           0x20
#define ST7735_INVON            0x21
#define ST7735_DISPOFF          0x28
#define ST7735_DISPON           0x29
#define ST7735_CASET            0x2A
#define ST7735_RASET            0x2B
#define ST7735_RAMWR            0x2C
#define ST7735_RAMRD            0x2E
#define ST7735_PTLAR            0x30
#define ST7735_MADCTL           0x36
#define ST7735_COLMOD           0x3A
#define ST7735_FRMCTR1          0xB1
#define ST7735_FRMCTR2          0xB2
#define ST7735_FRMCTR3          0xB3
#define ST7735_INVCTR           0xB4
#define ST7735_DISSET5          0xB6
#define ST7735_PWCTR1           0xC0
#define ST7735_PWCTR2           0xC1
#define ST7735_PWCTR3           0xC2
#define ST7735_PWCTR4           0xC3
#define ST7735_PWCTR5           0xC4
#define ST7735_VMCTR1           0xC5
#define ST7735_RDID1            0xDA
#define ST7735_RDID2            0xDB
#define ST7735_RDID3            0xDC
#define ST7735_RDID4            0xDD
#define ST7735_PWCTR6           0xFC
#define ST7735_GMCTRP1          0xE0
#define ST7735_GMCTRN1          0xE1
#define ST7735_Portrait         0xC0
#define ST7735_Portrait180      0
#define ST7735_Landscape        0xA0
#define ST7735_Landscape180     0x60

#define ILI9341_SOFTRESET       0x01
#define ILI9341_SLEEPIN         0x10
#define ILI9341_SLEEPOUT        0x11
#define ILI9341_NORMALDISP      0x13
#define ILI9341_INVERTOFF       0x20
#define ILI9341_INVERTON        0x21
#define ILI9341_GAMMASET        0x26
#define ILI9341_DISPLAYOFF      0x28
#define ILI9341_DISPLAYON       0x29
#define ILI9341_COLADDRSET      0x2A
#define ILI9341_PAGEADDRSET     0x2B
#define ILI9341_MEMORYWRITE     0x2C
#define ILI9341_RAMRD           0x2E
#define ILI9341_PIXELFORMAT     0x3A
#define ILI9341_FRAMECONTROL    0xB1
#define ILI9341_DISPLAYFUNC     0xB6
#define ILI9341_ENTRYMODE       0xB7
#define ILI9341_POWERCONTROL1   0xC0
#define ILI9341_POWERCONTROL2   0xC1
#define ILI9341_VCOMCONTROL1    0xC5
#define ILI9341_VCOMCONTROL2    0xC7
#define ILI9341_MEMCONTROL 	0x36
#define ILI9341_MADCTL_MY  	0x80
#define ILI9341_MADCTL_MX  	0x40
#define ILI9341_MADCTL_MV  	0x20
#define ILI9341_MADCTL_ML  	0x10
#define ILI9341_MADCTL_RGB 	0x00
#define ILI9341_MADCTL_BGR 	0x08
#define ILI9341_MADCTL_MH  	0x04

#define ILI9341_Portrait        ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR
#define ILI9341_Portrait180     ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR
#define ILI9341_Landscape       ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
#define ILI9341_Landscape180    ILI9341_MADCTL_MY | ILI9341_MADCTL_MX | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR

#define ILI9341_8_Portrait        ILI9341_MADCTL_BGR
#define ILI9341_8_Portrait180     ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR
#define ILI9341_8_Landscape       ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR| ILI9341_MADCTL_MX
#define ILI9341_8_Landscape180    ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
//
#define ILI9163_NOP             0x00//Non operation
#define ILI9163_SWRESET 	0x01//Soft Reset
#define ILI9163_SLPIN   	0x10//Sleep ON
#define ILI9163_SLPOUT  	0x11//Sleep OFF
#define ILI9163_PTLON   	0x12//Partial Mode ON
#define ILI9163_NORML   	0x13//Normal Display ON
#define ILI9163_DINVOF  	0x20//Display Inversion OFF
#define ILI9163_DINVON   	0x21//Display Inversion ON
#define ILI9163_GAMMASET 	0x26//Gamma Set (0x01[1],0x02[2],0x04[3],0x08[4])
#define ILI9163_DISPOFF 	0x28//Display OFF
#define ILI9163_DISPON  	0x29//Display ON
#define ILI9163_IDLEON  	0x39//Idle Mode ON
#define ILI9163_IDLEOF  	0x38//Idle Mode OFF
#define ILI9163_CLMADRS   	0x2A//Column Address Set
#define ILI9163_PGEADRS   	0x2B//Page Address Set
#define ILI9163_RAMWR   	0x2C//Memory Write
#define ILI9163_RAMRD   	0x2E//Memory Read
#define ILI9163_CLRSPACE   	0x2D//Color Space : 4K/65K/262K
#define ILI9163_PARTAREA	0x30//Partial Area
#define ILI9163_VSCLLDEF	0x33//Vertical Scroll Definition
#define ILI9163_TEFXLON		0x35//Tearing Effect Line ON
#define ILI9163_TEFXLOF		0x34//Tearing Effect Line OFF
#define ILI9163_MADCTL  	0x36//Memory Access Control
#define ILI9163_VSSTADRS	0x37//Vertical Scrolling Start address
#define ILI9163_PIXFMT  	0x3A//Interface Pixel Format
#define ILI9163_FRMCTR1 	0xB1//Frame Rate Control (In normal mode/Full colors)
#define ILI9163_FRMCTR2 	0xB2//Frame Rate Control(In Idle mode/8-colors)
#define ILI9163_FRMCTR3 	0xB3//Frame Rate Control(In Partial mode/full colors)
#define ILI9163_DINVCTR		0xB4//Display Inversion Control
#define ILI9163_RGBBLK		0xB5//RGB Interface Blanking Porch setting
#define ILI9163_DFUNCTR 	0xB6//Display Fuction set 5
#define ILI9163_SDRVDIR 	0xB7//Source Driver Direction Control
#define ILI9163_GDRVDIR 	0xB8//Gate Driver Direction Control
#define ILI9163_PWCTR1  	0xC0//Power_Control1
#define ILI9163_PWCTR2  	0xC1//Power_Control2
#define ILI9163_PWCTR3  	0xC2//Power_Control3
#define ILI9163_PWCTR4  	0xC3//Power_Control4
#define ILI9163_PWCTR5  	0xC4//Power_Control5
#define ILI9163_VCOMCTR1  	0xC5//VCOM_Control 1
#define ILI9163_VCOMCTR2  	0xC6//VCOM_Control 2
#define ILI9163_VCOMOFFS  	0xC7//VCOM Offset Control
#define ILI9163_PGAMMAC		0xE0//Positive Gamma Correction Setting
#define ILI9163_NGAMMAC		0xE1//Negative Gamma Correction Setting
#define ILI9163_GAMRSEL		0xF2//GAM_R_SEL
#define ILI9163_Portrait        0b00001000
#define ILI9163_Portrait180     0b11001000
#define ILI9163_Landscape       0b01101000
#define ILI9163_Landscape180    0b10101000


