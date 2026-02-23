/***********************************************************************************************************************
MMBasic

LCDDriver.c

This is the driver for SPI LCDs in MMBasic.
The core SPI LCD driver was written and developed by Peter Mather of the Back Shed Forum (http://www.thebackshed.com/forum/forum_topics.asp?FID=16)

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

#include <stdarg.h>
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"


void DefineRegionSPI(int xstart, int ystart, int xend, int yend, int rw);
void DrawBitmapSPI(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
extern SPI_HandleTypeDef SD_SPI;
extern int codecheck(char *line);
extern int codemap(char code, int pin);
#define SPIsend(a) {uint8_t b=a;HAL_SPI_Transmit(&SD_SPI,&b,1,500);}
#define SPIqueue(a) {HAL_SPI_Transmit(&SD_SPI,a,2,500);}
#define SPIsend2(a) {SPIsend(0);SPIsend(a);}
int visible = false;


// utility function for routines that want to reserve a pin for special I/O
// this ignores any previous settings and forces the pin to its new state
// pin is the pin number
// inp is true if an input or false if an output
// init is the value used to initialise the pin if it is an output (hi or lo)
// type is the final tag for the pin in ExtCurrentConfig[]
void MIPS16 SetAndReserve(int pin, int inp, int init, int type) {
    if(pin == 0) return;                                            // do nothing if not set
    GPIO_InitTypeDef GPIO_InitDef;
    if(inp) {
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
    } else {
        PinSetBit(pin, init ? LATSET : LATCLR);                     // set LAT
    	GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
    }
	GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
	GPIO_InitDef.Pin = PinDef[pin].bitnbr;
	GPIO_InitDef.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
    ExtCurrentConfig[pin] = type;
}
/**********************************************************************************************
 Control the cursor
 This does all the work of displaying, flashing and removing the cursor.

 It is called by FullScreenEditor() and MMgetchar()
 Both of these functions keep calling ShowCursor() with the argument true while they are in a
 loop waiting for a character - this function will then display and flash the cursor.   When
 a character is received these functions will call ShowCursor() with the argument false which
 is the signal to remove the cursor from the screen.

 Note that CursorTimer is incremented by the millisecond interrupt
***********************************************************************************************/
void ShowCursor(int show) {
    int newstate, fontheight=FontTable[gui_font >> 4][1] * (gui_font & 0b1111), fontwidth=FontTable[gui_font >> 4][0] * (gui_font & 0b1111);;

    if(!(OptionConsole & 2)) return;
	newstate = ((CursorTimer <= CURSOR_ON) && show);                // what should be the state of the cursor?
	if(visible == newstate) return;									// we can skip the rest if the cursor is already in the correct state
	visible = newstate;                                             // draw the cursor BELOW the font
    if(fontheight==12)DrawLine(CurrentX, CurrentY + fontheight-1, CurrentX + fontwidth, CurrentY + fontheight-1, 2, visible ? gui_fcolour : gui_bcolour);
    else if(fontheight==16 )DrawLine(CurrentX, CurrentY + fontheight-1, CurrentX + fontwidth, CurrentY + fontheight-1, 2, visible ? gui_fcolour : gui_bcolour);
    else if(gui_font_height==8)DrawLine(CurrentX, CurrentY + fontheight-1, CurrentX + fontwidth, CurrentY + fontheight-1, 1, visible ? gui_fcolour : gui_bcolour);
    else DrawLine(CurrentX, CurrentY + fontheight, CurrentX + fontwidth, CurrentY + fontheight, 2, visible ? gui_fcolour : gui_bcolour);
}



/******************************************************************************************
 Print a char on the LCD display (SSD1963 and in landscape only).  It handles control chars
 such as newline and will wrap at the end of the line and scroll the display if necessary.

 The char is printed at the current location defined by CurrentX and CurrentY
 *****************************************************************************************/
void DisplayPutC(char c) {

    if(!BasicRunning || !(OptionConsole & 2)) return;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
    // if it is printable and it is going to take us off the right hand end of the screen do a CRLF
    if(c >= FontTable[gui_font >> 4][2] && c < FontTable[gui_font >> 4][2] + FontTable[gui_font >> 4][3]) {
        if(CurrentX + gui_font_width > maxW) {
            DisplayPutC('\r');
            DisplayPutC('\n');
        }
    }

    // handle the standard control chars
    switch(c) {
        case '\b':  CurrentX -= gui_font_width;
                    if(CurrentX < 0) CurrentX = 0;
                    return;
        case '\r':  CurrentX = 0;
                    return;
        case '\n':  CurrentY += (optiony ? -gui_font_height: gui_font_height);
                    if(optiony==0 && (CurrentY + gui_font_height >= maxH-(ShortScroll?gui_font_height:0))){
                        if(!ShortScroll){
                            ScrollLCD(CurrentY + gui_font_height - maxH, 1);
                        	CurrentY -= (CurrentY + gui_font_height - maxH);
                        } else {
                        	ScrollLCD(gui_font_height,1);
                        	CurrentY -= (optiony ? -gui_font_height: gui_font_height);
                        }
                    } else if (optiony==1 && CurrentY<0){
                    	ScrollLCD(gui_font_height,0);
                    	CurrentY+=gui_font_height;
                    	DrawRectangle(0, 0, maxW - 1, gui_font_height, gui_bcolour); // erase the line to be scrolled off
                    }
                    return;
        case '\t':  do {
                        DisplayPutC(' ');
                    } while((CurrentX/gui_font_width) % Option.Tab);
                    return;
    }
    GUIPrintChar(gui_font, gui_fcolour, gui_bcolour, c, ORIENT_NORMAL);            // print it
}
/******************************************************************************************
 Print a string on the LCD display (SSD1963 and in landscape only).   It handles control
 chars such as newline and will wrap at the end of the line and scroll the display if
 necessary.

 The string is printed at the current location defined by CurrentX and CurrentY
*****************************************************************************************/
void DisplayPutS(char *s) {
    while(*s) DisplayPutC(*s++);
}

// set Chip Select for the LCD low
// this also checks the configuration of the SPI channel and if required reconfigures it to suit the LCD controller

/****************************************************************************************************
 ****************************************************************************************************

 Basic drawing primitives
 all drawing on the LCD is done using either one of these two functions

 ****************************************************************************************************
****************************************************************************************************/




// the default function for DrawRectangle() and DrawBitmap()
void DisplayNotSet(void) {
    error("Display not configured");
}

