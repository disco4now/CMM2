/***************************************************************************

CMM2 MMBasic
Draw.c

Does the basic LCD display commands and drawing in MMBasic.


Copyright 2011-2026 Geoff Graham, Peter Mather and Gerry Allardice.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

------------------------------------------------------------------------------
  * In addition the software components from STMicroelectronics are provided
  * subject to the license as detailed below:
------------------------------------------------------------------------------
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
*******************************************************************************/


#include <float.h>

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "dma2d.h"
#include "ltdc.h"
#include "rotate.h"
#include "resize.h"
#include "array_utility.h"
#include "bmp.h"
#define hLtdcHandler hltdc
#define hDma2dHandler hdma2d
extern LTDC_HandleTypeDef hltdc;
extern const uint32_t L8_CLUT_LOW[];
extern const uint32_t L8_CLUT_MEDIUM[];
extern const uint32_t L8_CLUT_HIGH[];
extern JPEG_ConfTypeDef       JPEG_Info;
uint8_t *FontBuffer;
extern uint8_t convert_8bit(uint32_t c);
extern uint16_t convert_12bit(uint32_t c);
extern uint16_t convert_16bit(uint32_t c);
extern uint8_t *linebuff;
extern int ReadPixelFast(int x, int y);
extern void DrawPixelFast(int x, int y, int c);
void DrawPolygon(int n, short *xcoord, short *ycoord, int face);
void CalcLine(int x1, int y1, int x2, int y2, short *xmin, short *xmax);
int autocursor=0;
extern const int colours[16];
extern void Merge(uint32_t fadd1, uint32_t fadd2, uint32_t tadd, int colour);
void polygon(char *p, int close);
/***************************************************************************/
// define the fonts


    #include "font1.h"
    #include "Misc_12x20_LE.h"
    #include "Hom_16x24_LE.h"
    #include "Fnt_10x16.h"
    #include "Inconsola.h"
    #include "ArialNumFontPlus.h"
    #include "Font_8x6.h"

    unsigned char *FontTable[FONT_TABLE_SIZE] = {   (unsigned char *)font1,
                                                    (unsigned char *)Misc_12x20_LE,
                                                    (unsigned char *)Hom_16x24_LE,
                                                    (unsigned char *)Fnt_10x16,
                                                    (unsigned char *)Inconsola,
                                                    (unsigned char *)ArialNumFontPlus,
													(unsigned char *)F_6x8_LE,
													(unsigned char *)NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,

                                                };

const uint8_t cursor8[]={
		255,0,0,0,0,0,0,0,0,0,0,0,0,
		255,255,0,0,0,0,0,0,0,0,0,0,0,
		255,0,255,0,0,0,0,0,0,0,0,0,0,
		255,0,0,255,0,0,0,0,0,0,0,0,0,
		255,0,0,0,255,0,0,0,0,0,0,0,0,
		255,0,0,0,0,255,0,0,0,0,0,0,0,
		255,0,0,0,0,0,255,0,0,0,0,0,0,
		255,0,0,0,0,0,0,255,0,0,0,0,0,
		255,0,0,0,0,0,0,0,255,0,0,0,0,
		255,0,0,0,0,0,0,0,0,255,0,0,0,
		255,0,0,0,0,0,0,0,0,0,255,0,0,
		255,0,0,0,0,0,0,0,0,0,0,255,0,
		255,0,0,0,0,0,0,255,255,255,255,255,255,
		255,0,0,0,255,0,0,255,0,0,0,0,0,
		255,0,0,255,255,0,0,255,0,0,0,0,0,
		255,0,255,0,0,255,0,0,255,0,0,0,0,
		255,255,0,0,0,255,0,0,255,0,0,0,0,
		0,0,0,0,0,0,255,0,0,255,0,0,0,
		0,0,0,0,0,0,255,255,255,255,0,0,0
};
const uint16_t cursor16[]={
		0xFFFF,0,0,0,0,0,0,0,0,0,0,0,0,
		0xFFFF,0xFFFF,0,0,0,0,0,0,0,0,0,0,0,
		0xFFFF,0,0xFFFF,0,0,0,0,0,0,0,0,0,0,
		0xFFFF,0,0,0xFFFF,0,0,0,0,0,0,0,0,0,
		0xFFFF,0,0,0,0xFFFF,0,0,0,0,0,0,0,0,
		0xFFFF,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0xFFFF,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,
		0xFFFF,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,
		0xFFFF,0,0,0,0,0,0,0,0xFFFF,0,0,0,0,
		0xFFFF,0,0,0,0,0,0,0,0,0xFFFF,0,0,0,
		0xFFFF,0,0,0,0,0,0,0,0,0,0xFFFF,0,0,
		0xFFFF,0,0,0,0,0,0,0,0,0,0,0xFFFF,0,
		0xFFFF,0,0,0,0,0,0,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
		0xFFFF,0,0,0,0xFFFF,0,0,0xFFFF,0,0,0,0,0,
		0xFFFF,0,0,0xFFFF,0xFFFF,0,0,0xFFFF,0,0,0,0,0,
		0xFFFF,0,0xFFFF,0,0,0xFFFF,0,0,0xFFFF,0,0,0,0,
		0xFFFF,0xFFFF,0,0,0,0xFFFF,0,0,0xFFFF,0,0,0,0,
		0,0,0,0,0,0,0xFFFF,0,0,0xFFFF,0,0,0,
		0,0,0,0,0,0,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0,0,0
};
const uint32_t cursor32[]={
		0xFFFEFEFE,0,0,0,0,0,0,0,0,0,0,0,0,
		0xFFFEFEFE,0xFFFEFEFE,0,0,0,0,0,0,0,0,0,0,0,
		0xFFFEFEFE,0,0xFFFEFEFE,0,0,0,0,0,0,0,0,0,0,
		0xFFFEFEFE,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,0,0,
		0xFFFEFEFE,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,0,
		0xFFFEFEFE,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0xFFFEFEFE,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,
		0xFFFEFEFE,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,
		0xFFFEFEFE,0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,
		0xFFFEFEFE,0,0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,
		0xFFFEFEFE,0,0,0,0,0,0,0,0,0,0xFFFEFEFE,0,0,
		0xFFFEFEFE,0,0,0,0,0,0,0,0,0,0,0xFFFEFEFE,0,
		0xFFFEFEFE,0,0,0,0,0,0,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,
		0xFFFEFEFE,0,0,0,0xFFFEFEFE,0,0,0xFFFEFEFE,0,0,0,0,0,
		0xFFFEFEFE,0,0,0xFFFEFEFE,0xFFFEFEFE,0,0,0xFFFEFEFE,0,0,0,0,0,
		0xFFFEFEFE,0,0xFFFEFEFE,0,0,0xFFFEFEFE,0,0,0xFFFEFEFE,0,0,0,0,
		0xFFFEFEFE,0xFFFEFEFE,0,0,0,0xFFFEFEFE,0,0,0xFFFEFEFE,0,0,0,0,
		0,0,0,0,0,0,0xFFFEFEFE,0,0,0xFFFEFEFE,0,0,0,
		0,0,0,0,0,0,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0,0,0
};
const uint8_t cross8[]={
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
};
const uint16_t cross16[]={
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFF,0,0,0,0,0,0,0,
};
const uint32_t cross32[]={
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,0xFFFEFEFE,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0xFFFEFEFE,0,0,0,0,0,0,0,
};

int wcursor=13;
int hcursor=19;
int xoffcursor=0;
int yoffcursor=0;
/***************************************************************************/

volatile int gui_font;
int gui_fcolour;
int gui_bcolour;
int PrintPixelMode=0;
volatile int CurrentX, CurrentY;                                             // the current default position for the next char to be written
// the MMBasic programming characteristics of the display

// Helper macros for strided array access (used for struct member arrays)
// Added to support STRUCTENABLED
#define STRIDE_FLOAT(ptr, idx, stride) (*(MMFLOAT *)((char *)(ptr) + (idx) * (stride)))
#define STRIDE_INT(ptr, idx, stride) (*(long long int *)((char *)(ptr) + (idx) * (stride)))

// Maximum number of vertices for polygon fill operations
//#define MAX_POLYGON_VERTICES 256

// Magic number to indicate sprite position is not in use
#define SPRITE_POS_INACTIVE 10000

// Magic number to indicate buffer is a triangle buffer (not rectangular)
#define TRIANGLE_BUFFER_MARKER 9999

// pointers to the drawing primitives
void (*DrawPixel)(int x1, int y1, int c) = (void (*)(int , int , int ))DisplayNotSet;
void (*DrawRectangle)(int x1, int y1, int x2, int y2, int c) = (void (*)(int , int , int , int , int ))DisplayNotSet;
void (*DrawBitmap)(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap) = (void (*)(int , int , int , int , int , int , int , unsigned char *))DisplayNotSet;
void (*ScrollLCD) (int lines, int blank) = (void (*)(int, int ))DisplayNotSet;
void (*DrawBuffer)(int x1, int y1, int x2, int y2, char *c, int skip) = (void (*)(int , int , int , int , char * , int))DisplayNotSet;
void (*DrawBufferFast)(int x1, int y1, int x2, int y2, char *c) = (void (*)(int , int , int , int , char * ))DisplayNotSet;
void (*ReadBuffer)(int x1, int y1, int x2, int y2, char *c) = (void (*)(int , int , int , int , char * ))DisplayNotSet;
void (*ReadBufferFast)(int x1, int y1, int x2, int y2, char *c) = (void (*)(int , int , int , int , char * ))DisplayNotSet;
void (*ScrollBufferV)(int lines, int blank) = (void (*)(int , int ))DisplayNotSet;
void (*ScrollBufferH)(int lines) = (void (*)(int ))DisplayNotSet;
void (*BlitShowBuffer)(int bnbr, int x1, int y1, int mode) = (void (*)(int , int , int , int ))DisplayNotSet;
void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill);
void ClearTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int ints_per_line, uint32_t *br);
void DrawFilledCircle(int x, int y, int radius, int r, int fill, int ints_per_line, uint32_t *br, MMFLOAT aspect, MMFLOAT aspect2);
void DrawLine(int x1, int y1, int x2, int y2, int w, int c) ;
void floodFillScanline(int x, int y);
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define RoundUptoPage512(a)     ((((uint64_t)a) + (uint64_t)(512*1024 - 1)) & (uint64_t)(~(512*1024 - 1)))// round up to the nearest whole integer
MMFLOAT pidiv2=PI_VALUE/2.0, kpar = 0.5522847498 ;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
extern volatile uint8_t pagesetdone;
volatile int gui_font_width, gui_font_height;
volatile int CursorTimer;               // used to time the flashing cursor
int lastx=0,lasty=0;
int AutoLineWrap=true;
struct D3D *struct3d[MAX3D+1]={NULL};
volatile int deferredcopy=0;
uint32_t deferredfadd=0, deferredtadd=0, deferredtransparent=0;
extern void ScrollBuff8H(int pixels);
extern void ScrollBuff16H(int pixels);
typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char trans;
} rgb_t;

typedef struct {
    TFLOAT  xpos;       // current position and heading
    TFLOAT  ypos;       // (uses floating-point numbers for
    TFLOAT  heading;    //  increased accuracy)

    rgb_t  pen_color;   // current pen color
    rgb_t  fill_color;  // current fill color
    bool   pendown;     // currently drawing?
    bool   filled;      // currently filling?
} fill_t;
fill_t main_fill;
fill_t backup_fill;
int    main_fill_poly_vertex_count = 0;       // polygon vertex count
TFLOAT *main_fill_polyX=NULL; // polygon vertex x-coords
TFLOAT *main_fill_polyY=NULL; // polygon vertex y-coords
int xcursor=0;
int ycursor=0;
int cursoron=0;
int cursorenable=0;
char *cursorsave=NULL;
int cursor=0;
uint8_t *loadcursordata=NULL;
int wlcursor;
int hlcursor;
int xloffcursor;
int yloffcursor;
int cursorcolour32=0xFFFEFEFE;
int cursorcolour16=0xFFFF;
int cursorcolour8=0xFF;
s_camera camera[MAXCAM+1];
char *nostackp;
uint32_t *newstack;
int filloldcolour, ConvertedColour, fillmaxH, fillmaxW;
int HRes,VRes;
uint32_t ReadPageAddressExternal, WritePageAddressExternal;
volatile int mouseupdated=0;
/****************************************************************************************************

 MMBasic commands and functions

****************************************************************************************************/
void loadcursor(char *p){
	int fnbr, width, height=0, xoffset, yoffset, lc, i;
	char *q, *fname;
	short *qq;
	uint32_t *qqq;
	char buff[256];
	getargs(&p, 1,",");
	wlcursor=0;
	hlcursor=0;
	xloffcursor=0;
	yloffcursor=0;
	fnbr = FindFreeFileNbr();
	fname = getCstring(argv[0]);
	if(strchr(fname, '.') == NULL) strcat(p, ".CUR");
	if(!BasicFileOpen(fname, fnbr, FA_READ)) error("File not found");
	MMgetline(fnbr, (char *)buff);							    // get the input line
	while(buff[0]==39)MMgetline(fnbr, (char *)buff);
	sscanf((char *)buff, "%d,%d,%d,%d", &width,  &height, &xoffset, &yoffset);
	loadcursordata = GetMemory(width * height * (VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
	q=(char *)loadcursordata;
	qq=(short *)loadcursordata;
	qqq=(uint32_t *)loadcursordata;
	lc=height;
	while(lc--){
		MMgetline(fnbr, (char *)buff);									    // get the input line
		while(buff[0]==39)MMgetline(fnbr, (char *)buff);
		if(strlen(buff)<width)mymemset(&buff[strlen(buff)],32,width-strlen(buff));
		if(VideoColour==8){
			for(i=0;i<width;i++){
				if(buff[i]==' ')*q++=0;
				else if(buff[i]=='0')*q++=convert_8bit(NOTBLACK);
				else if(buff[i]=='1')*q++=convert_8bit(BLUE);
				else if(buff[i]=='2')*q++=convert_8bit(GREEN);
				else if(buff[i]=='3')*q++=convert_8bit(CYAN);
				else if(buff[i]=='4')*q++=convert_8bit(RED);
				else if(buff[i]=='5')*q++=convert_8bit(MAGENTA);
				else if(buff[i]=='6')*q++=convert_8bit(YELLOW);
				else if(buff[i]=='7')*q++=convert_8bit(WHITE);
				else if(buff[i]=='8')*q++=convert_8bit(LITEGRAY);
				else if(buff[i]=='9')*q++=convert_8bit(GRAY);
				else if(buff[i]=='A' || buff[i]=='a')*q++=convert_8bit(ORANGE);
				else if(buff[i]=='B' || buff[i]=='b')*q++=convert_8bit(PINK);
				else if(buff[i]=='C' || buff[i]=='c')*q++=convert_8bit(GOLD);
				else if(buff[i]=='D' || buff[i]=='d')*q++=convert_8bit(SALMON);
				else if(buff[i]=='E' || buff[i]=='e')*q++=convert_8bit(BEIGE);
				else if(buff[i]=='F' || buff[i]=='f')*q++=convert_8bit(BROWN);
				else *q++=0;
			}
		}
		if(VideoColour==12){
			for(i=0;i<width;i++){
				if(buff[i]==' ')*qq++=0;
				else if(buff[i]=='0')*qq++=0xF000;
				else if(buff[i]=='1')*qq++=convert_12bit(BLUE)|0xF000;
				else if(buff[i]=='2')*qq++=convert_12bit(GREEN)|0xF000;
				else if(buff[i]=='3')*qq++=convert_12bit(CYAN)|0xF000;
				else if(buff[i]=='4')*qq++=convert_12bit(RED)|0xF000;
				else if(buff[i]=='5')*qq++=convert_12bit(MAGENTA)|0xF000;
				else if(buff[i]=='6')*qq++=convert_12bit(YELLOW)|0xF000;
				else if(buff[i]=='7')*qq++=convert_12bit(WHITE)|0xF000;
				else if(buff[i]=='8')*qq++=convert_12bit(LITEGRAY)|0xF000;
				else if(buff[i]=='9')*qq++=convert_12bit(GRAY)|0xF000;
				else if(buff[i]=='A' || buff[i]=='a')*qq++=convert_12bit(ORANGE)|0xF000;
				else if(buff[i]=='B' || buff[i]=='b')*qq++=convert_12bit(PINK)|0xF000;
				else if(buff[i]=='C' || buff[i]=='c')*qq++=convert_12bit(GOLD)|0xF000;
				else if(buff[i]=='D' || buff[i]=='d')*qq++=convert_12bit(SALMON)|0xF000;
				else if(buff[i]=='E' || buff[i]=='e')*qq++=convert_12bit(BEIGE)|0xF000;
				else if(buff[i]=='F' || buff[i]=='f')*qq++=convert_12bit(BROWN)|0xF000;
				else *qq++=0;
			}
		}
		if(VideoColour==16){
			for(i=0;i<width;i++){
				if(buff[i]==' ')*qq++=0;
				else if(buff[i]=='0')*qq++=convert_16bit(NOTBLACK);
				else if(buff[i]=='1')*qq++=convert_16bit(BLUE);
				else if(buff[i]=='2')*qq++=convert_16bit(GREEN);
				else if(buff[i]=='3')*qq++=convert_16bit(CYAN);
				else if(buff[i]=='4')*qq++=convert_16bit(RED);
				else if(buff[i]=='5')*qq++=convert_16bit(MAGENTA);
				else if(buff[i]=='6')*qq++=convert_16bit(YELLOW);
				else if(buff[i]=='7')*qq++=convert_16bit(WHITE);
				else if(buff[i]=='8')*qq++=convert_16bit(LITEGRAY);
				else if(buff[i]=='9')*qq++=convert_16bit(GRAY);
				else if(buff[i]=='A' || buff[i]=='a')*qq++=convert_16bit(ORANGE);
				else if(buff[i]=='B' || buff[i]=='b')*qq++=convert_16bit(PINK);
				else if(buff[i]=='C' || buff[i]=='c')*qq++=convert_16bit(GOLD);
				else if(buff[i]=='D' || buff[i]=='d')*qq++=convert_16bit(SALMON);
				else if(buff[i]=='E' || buff[i]=='e')*qq++=convert_16bit(BEIGE);
				else if(buff[i]=='F' || buff[i]=='f')*qq++=convert_16bit(BROWN);
				else *qq++=0;
			}
		}
		if(VideoColour==32){
			for(i=0;i<width;i++){
				if(buff[i]==' ')*qqq++=0;
				else if(buff[i]=='0')*qqq++=NOTBLACK;
				else if(buff[i]=='1')*qqq++=BLUE;
				else if(buff[i]=='2')*qqq++=GREEN;
				else if(buff[i]=='3')*qqq++=CYAN;
				else if(buff[i]=='4')*qqq++=RED;
				else if(buff[i]=='5')*qqq++=MAGENTA;
				else if(buff[i]=='6')*qqq++=YELLOW;
				else if(buff[i]=='7')*qqq++=WHITE;
				else if(buff[i]=='8')*qqq++=LITEGRAY;
				else if(buff[i]=='9')*qqq++=convert_16bit(GRAY);
				else if(buff[i]=='A' || buff[i]=='a')*qqq++=ORANGE;
				else if(buff[i]=='B' || buff[i]=='b')*qqq++=PINK;
				else if(buff[i]=='C' || buff[i]=='c')*qqq++=GOLD;
				else if(buff[i]=='D' || buff[i]=='d')*qqq++=SALMON;
				else if(buff[i]=='E' || buff[i]=='e')*qqq++=BEIGE;
				else if(buff[i]=='F' || buff[i]=='f')*qqq++=BROWN;
				else *qqq++=0;
			}
		}
	}
	FileClose(fnbr);
	wlcursor=width;
	hlcursor=height;
	xloffcursor=xoffset;
	yloffcursor=yoffset;
	return;
}

void hidecursor(int override){
	if(cursoron==0 ||  (WritePage != (VideoColour==12 ? 1 : 0) && !override))return;
    uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
    ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
    DrawBufferFast(xcursor,ycursor,xcursor+wcursor-1,ycursor+hcursor-1,cursorsave);
    cursoron=0;
    ReadPage=readsave;
    WritePage=writesave;
}
void closecursor(void){
	if(!cursorenable)return;
	hidecursor(1);
	xcursor=0;
	ycursor=0;
	cursorenable=0;
	cursor=0;
	FreeMemorySafe((void *)&cursorsave);
	cursorsave=NULL;
}
void showcursor(int override, int x, int y){
	if(cursorenable==0  || (WritePage != (VideoColour==12 ? 1 : 0) && !override))return;
	char *modify=GetInternalMemory(wcursor * hcursor * (VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
    uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
    ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
	int bytecount=wcursor * hcursor * PageTable[WritePage].nbytes;
    ReadBufferFast(x, y, x+wcursor-1, y+hcursor-1,cursorsave);
    mycopy(modify,cursorsave, bytecount);
    xcursor=x;
    ycursor=y;
    if(VideoColour==8){
    	char *p=modify;
    	char *q=cursorsave;
    	char *r=(cursor == 0 ? (char *)cursor8 : (cursor==1 ? (char *)cross8 : (char *)loadcursordata));
    	if(cursor<2)*r = *r & cursorcolour8;
    	while(bytecount--){
    	    if(*r){
    	    	if(cursor<2)*p=cursorcolour8;
    	    	else *p=*r;
    	    }
    	    else *p=*q;
    	    p++;
    	    r++;
    	    q++;
    	}
    } else if(VideoColour<=16){
    	short *p=(short *)modify;
    	short *q=(short *)cursorsave;
    	short *r=(cursor == 0 ? (short *)cursor16 : (cursor==1 ? (short *)cross16 : (short *)loadcursordata));
    	if(cursor<2)*r = *r & cursorcolour16;
    	while(bytecount-=2){
    	    if(*r){
    	    	if(cursor<2)*p = cursorcolour16;
    	    	else *p=*r;
    	    }
    	    else *p=*q;
    	    p++;
    	    r++;
    	    q++;
    	}
    } else {
    	uint32_t *p=(uint32_t *)modify;
    	uint32_t *q=(uint32_t *)cursorsave;
    	uint32_t *r=(cursor == 0 ? (uint32_t *)cursor32 : (cursor==1 ? (uint32_t *)cross32 : (uint32_t *)loadcursordata));
    	if(cursor<2)*r = *r & cursorcolour32;
    	while(bytecount-=4){
    	    if(*r){
    	    	if(cursor<2)*p = cursorcolour32;
    	    	else *p=*r;
    	    }
    	    else *p=*q;
    	    p++;
    	    r++;
    	    q++;
    	}
    }
    DrawBufferFast(xcursor,ycursor,xcursor+wcursor-1,ycursor+hcursor-1,modify);
    cursoron=1;
    ReadPage=readsave;
    WritePage=writesave;
    FreeMemorySafe((void *)&modify);
}
void cmd_cursor(char *p){
	char *tp=NULL;
	int x,y,c;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
    tp = checkstring(p, "ON");
    if(tp) {  //CURSOR ON
    	int x=0,y=0;
    	getargs(&tp,7,",");
    	if(cursorenable)return;
		cursorcolour32=0xFFFEFEFE;
		cursorcolour16=0xFFFF;
		cursorcolour8=0xFF;
    	wcursor=13;
    	hcursor=19;
    	xoffcursor=0;
    	yoffcursor=0;
    	cursor=0;
    	//cursorsave=GetInternalMemory(wcursor * hcursor * (VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
    	if(argc){
    		if(argc==3)error("Argument count");
    		if(*argv[0])cursor=(getint(argv[0],0,(loadcursordata==NULL? 1 : 2)));
    		if(argc>=5){
				x=getint(argv[2],0,maxW-1);
				y=getint(argv[4],0,maxH-1);
    		}
    		if(argc==7){
    			c=getColour(argv[6], 0);
    			if(VideoColour==32){
    				cursorcolour32=c;
    			} else if(VideoColour==16){
    	            uint16_t red, green, blue;
    	        	red=BIT_5[((c & 0xFF0000)>>19)]<<11;
    	            green=BIT_6[((c & 0xFF00)>>10)]<<5;
    	            blue=BIT_5[((c & 0xFF)>>3)];
    	            cursorcolour16=red|green|blue;
    	        } else if(VideoColour==12){
    	            uint16_t red, green, blue, trans;
    	        	red=BIT_4[((c & 0xFF0000)>>20)]<<8;
    	            green=BIT_4[((c & 0xFF00)>>12)]<<4;
    	            blue=BIT_4[((c & 0xFF)>>4)];
    	            trans=((c & 0xF000000)>>12);
    	            cursorcolour16=red|green|blue|trans;
    	        } else {
    	        	cursorcolour8 = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
    	        }
    		}
    	}
    	if(cursor==1){
    		wcursor=15;
    		hcursor=15;
    		xoffcursor=7;
    		yoffcursor=7;
    	} else if(cursor==2){
    		wcursor=wlcursor;
    		hcursor=hlcursor;
    		xoffcursor=xloffcursor;
    		yoffcursor=yloffcursor;
    	}
    	cursorsave=GetInternalMemory(wcursor * hcursor * (VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
    	cursorenable=1;
    	showcursor(1, x-xoffcursor,y-yoffcursor);
    	return;
    }

    tp = checkstring(p, "COLOUR");
    if(tp) {
    	if(!cursorenable)return;
		c=getColour(tp, 0);
		if(VideoColour==32){
			cursorcolour32=c;
		} else if(VideoColour==16){
            uint16_t red, green, blue;
        	red=BIT_5[((c & 0xFF0000)>>19)]<<11;
            green=BIT_6[((c & 0xFF00)>>10)]<<5;
            blue=BIT_5[((c & 0xFF)>>3)];
            cursorcolour16=red|green|blue;
        } else if(VideoColour==12){
            uint16_t red, green, blue, trans;
        	red=BIT_4[((c & 0xFF0000)>>20)]<<8;
            green=BIT_4[((c & 0xFF00)>>12)]<<4;
            blue=BIT_4[((c & 0xFF)>>4)];
            trans=((c & 0xF000000)>>12);
            cursorcolour16=red|green|blue|trans;
        } else {
        	cursorcolour8 = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
        }
    	hidecursor(1);
    	showcursor(1, xcursor, ycursor);
    	return;
    }
    tp = checkstring(p, "OFF");
    if(tp) {
    	closecursor();
    	return;
    }
    tp = checkstring(p, "HIDE");
    if(tp) {
    	if(!cursorenable)return;
    	hidecursor(1);
    	return;
    }
    tp = checkstring(p, "LOAD");
    if(tp){
    	if(cursorenable)error("Cursor On");
    	loadcursor(tp);
    	return;
    }
    tp = checkstring(p, "SHOW");
    if(tp) {
    	if(!cursorenable)error("Cursor Off");
    	if(cursoron)return;
    	showcursor(1, xcursor, ycursor);
    	return;
    }
    tp = checkstring(p, "LINK MOUSE");
    if(tp) {
    	if(!cursorenable)error("Cursor Off");
    	if(autocursor!=0)error("Already linked");
    	if(GUIactive)error("Mouse in use for GUI");
    	if(!(mouse3 || mouse2 || mouse1 || mouse0 ))error("Not open");
    	autocursor=1;
    	return;
    }
    tp = checkstring(p, "UNLINK MOUSE");
    if(tp) {
    	if(autocursor==0)error("Not Linked");
    	autocursor=0;
    	return;
    }
    getargs(&p,3,",");
    if(!cursorenable)error("Cursor Off");
    x=getint(argv[0],0,maxW-1);
    y=getint(argv[2],0,maxH-1);
    if(x-xoffcursor==xcursor && y-yoffcursor==ycursor &&cursoron)return;
    if(cursoron){
    	hidecursor(1);
    	showcursor(1, x-xoffcursor,y-yoffcursor);
    } else {
        xcursor=x-xoffcursor;
        ycursor=y-yoffcursor;
    }
}
void fill_set_pen_color(int red, int green, int blue, int trans)
{
    main_fill.pen_color.red = red;
    main_fill.pen_color.green = green;
    main_fill.pen_color.blue = blue;
    main_fill.pen_color.trans = trans;
}

void fill_set_fill_color(int red, int green, int blue, int trans)
{
    main_fill.fill_color.red = red;
    main_fill.fill_color.green = green;
    main_fill.fill_color.blue = blue;
    main_fill.fill_color.trans = trans;
}
static void fill_begin_fill()
{
    main_fill.filled = true;
    main_fill_poly_vertex_count = 0;
}

static void fill_end_fill(int count, int ystart, int yend)
{
    // based on public-domain fill algorithm in C by Darel Rex Finley, 2007
    //   from http://alienryderflex.com/polygon_fill/

    TFLOAT *nodeX=GetInternalMemory(count * sizeof(TFLOAT));     // x-coords of polygon intercepts
    int nodes;                              // size of nodeX
    int y, i, j;                         // current pixel and loop indices
    TFLOAT temp;                            // temporary variable for sorting
    int f=(main_fill.fill_color.trans<<24) | (main_fill.fill_color.red<<16) | (main_fill.fill_color.green<<8) | main_fill.fill_color.blue;
    int c=(main_fill.pen_color.trans<<24) | (main_fill.pen_color.red<<16) | (main_fill.pen_color.green<<8) | main_fill.pen_color.blue;
    int xstart, xend;
	int red, green, blue, trans, Colour;
	if(VideoColour==32){
		Colour=f;
	} else if(VideoColour<=8){
		Colour = (((main_fill.fill_color.red) & 0b11100000)) | (((main_fill.fill_color.green) & 0b11100000)>>3) | ((main_fill.fill_color.blue)>>6);
	} else {
		if(VideoColour==16){
			red=BIT_5[(main_fill.fill_color.red>>3)]<<11;
			green=BIT_6[(main_fill.fill_color.green>>2)]<<5;
			blue=BIT_5[(main_fill.fill_color.blue>>3)];
			Colour=red|green|blue;
		} else {
			trans=((main_fill.fill_color.trans)<<12);
			red=BIT_4[(main_fill.fill_color.red>>4)]<<8;
			green=BIT_4[(main_fill.fill_color.green>>4)]<<4;
			blue=BIT_4[(main_fill.fill_color.blue>>4)];
			Colour=red|green|blue|trans;
		}
	}
    //  loop through the rows of the image

    for (y = ystart; y < yend; y++) {

        //  build a list of polygon intercepts on the current line
        nodes = 0;
        j = main_fill_poly_vertex_count-1;
        for (i = 0; i < main_fill_poly_vertex_count; i++) {
            if ((main_fill_polyY[i] <  (TFLOAT)y &&
                 main_fill_polyY[j] >= (TFLOAT)y) ||
                (main_fill_polyY[j] <  (TFLOAT)y &&
                 main_fill_polyY[i] >= (TFLOAT)y)) {

                // intercept found; record it
                nodeX[nodes++] = (main_fill_polyX[i] +
                        ((TFLOAT)y - main_fill_polyY[i]) /
                        (main_fill_polyY[j] - main_fill_polyY[i]) *
                        (main_fill_polyX[j] - main_fill_polyX[i]));
            }
            j = i;
        }

        //  sort the nodes via simple insertion sort
        for (i = 1; i < nodes; i++) {
            temp = nodeX[i];
            for (j = i; j > 0 && temp < nodeX[j-1]; j--) {
                nodeX[j] = nodeX[j-1];
            }
            nodeX[j] = temp;
        }

        //  fill the pixels between node pairs
        for (i = 0; i < nodes; i += 2) {
        	xstart=(int)tfloor(nodeX[i])+1;
        	xend=(int)tceil(nodeX[i+1])-1;
        	DrawHLineFast(xstart,y,xend,Colour);
        }
    }

    main_fill.filled = false;

    // redraw polygon (filling is imperfect and can occasionally occlude sides)
    for (i = 0; i < main_fill_poly_vertex_count; i++) {
        int x0 = (int)(main_fill_polyX[i]);
        int y0 = (int)(main_fill_polyY[i]);
        int x1 = (int)(main_fill_polyX[(i+1) %
            main_fill_poly_vertex_count]);
        int y1 = (int)(main_fill_polyY[(i+1) %
            main_fill_poly_vertex_count]);
        DrawLine(x0, y0, x1, y1, 1, c);
    }
    FreeMemory(nodeX);
}
static void fill_fast_fill(int count, int ystart, int yend)
// this version only works on convex shapes
{
    int f=(main_fill.fill_color.trans<<24) | (main_fill.fill_color.red<<16) | (main_fill.fill_color.green<<8) | main_fill.fill_color.blue;
    int c=(main_fill.pen_color.trans<<24) | (main_fill.pen_color.red<<16) | (main_fill.pen_color.green<<8) | main_fill.pen_color.blue;
	int i, y, red, green, blue, trans, Colour;
	if(VideoColour==32){
		Colour=f;
	} else if(VideoColour<=8){
		Colour = (((main_fill.fill_color.red) & 0b11100000)) | (((main_fill.fill_color.green) & 0b11100000)>>3) | ((main_fill.fill_color.blue)>>6);
	} else {
		if(VideoColour==16){
			red=BIT_5[(main_fill.fill_color.red>>3)]<<11;
			green=BIT_6[(main_fill.fill_color.green>>2)]<<5;
			blue=BIT_5[(main_fill.fill_color.blue>>3)];
			Colour=red|green|blue;
		} else {
			trans=((main_fill.fill_color.trans)<<12);
			red=BIT_4[(main_fill.fill_color.red>>4)]<<8;
			green=BIT_4[(main_fill.fill_color.green>>4)]<<4;
			blue=BIT_4[(main_fill.fill_color.blue>>4)];
			Colour=red|green|blue|trans;
		}
	}
	short *xmin=(short *)linebuff;
	short *xmax=(short *)(linebuff+2160); //max number of lines is 1080
	for(y=ystart; y<=yend; y++){
		if(y>=0 && y<1080){
			xmin[y]=32767;
			xmax[y]=-1;
		}
	}

    for (i = 0; i < main_fill_poly_vertex_count; i++) {
        int x0 = (int)(main_fill_polyX[i]);
        int y0 = (int)(main_fill_polyY[i]);
        int x1 = (int)(main_fill_polyX[(i+1) %
            main_fill_poly_vertex_count]);
        int y1 = (int)(main_fill_polyY[(i+1) %
            main_fill_poly_vertex_count]);
        CalcLine(x0, y0, x1, y1, xmin, xmax);
    }
	for(y=ystart;y<=yend;y++){
		if(y>=0 && y<1080)DrawHLineFast(xmin[y], y, xmax[y], Colour);
	}
	if(f!=c){
	    for (i = 0; i < main_fill_poly_vertex_count; i++) {
	        int x0 = (int)(main_fill_polyX[i]);
	        int y0 = (int)(main_fill_polyY[i]);
	        int x1 = (int)(main_fill_polyX[(i+1) %
	            main_fill_poly_vertex_count]);
	        int y1 = (int)(main_fill_polyY[(i+1) %
	            main_fill_poly_vertex_count]);
	        DrawLine(x0, y0, x1, y1, 1, c);
	    }
	}
}

int colourmap[8];

int getColour(char *c, int minus){
	int colour;
	if(CMM1){
		colour = getint(c,(minus ? -1: 0),7);
		if(colour>=0)colour=colourmap[colour];
	} else colour=getint(c,(minus ? -1: 0),0xFFFFFFF);
	return colour;

}

void PageCopy(uint32_t fadd, uint32_t tadd, int transparent){
	uint8_t writesave=WritePage, readsave=ReadPage;
	transparent = (transparent<<2);
	int copymode=PageTable[fadd].expand | ((PageTable[tadd].expand)<<1) | transparent;
    docopy=mycopy;
	if(VideoColour==8 && (PageTable[fadd].xmax & 3) == 0 ){
		docopy=zcopy;
    } else if(VideoColour <=16 && (PageTable[fadd].xmax & 1) == 0){
		docopy=zcopy;
    } else if(VideoColour==32) docopy=zcopy;
	WritePage=tadd;
	ReadPage=fadd;
	switch(copymode){
	case 0:
	case 3:
		{
			int n=(PageTable[fadd].xmax*PageTable[fadd].ymax*PageTable[fadd].nbytes);
			if(PageTable[fadd].expand)n*=2;
			uint32_t *d=(uint32_t *)GetPageAddress(tadd);
			uint32_t *s=(uint32_t *)GetPageAddress(fadd);
			mycopy(d,s,n);
			routinechecks(1);
		}
		break;
	case 1:
	case 5:
		MoveBufferContract( 0, 0, 0, 0, PageTable[fadd].xmax, PageTable[fadd].ymax, transparent);
		break;
	case 2:
	case 6:
		MoveBufferExpand( 0, 0, 0, 0, PageTable[fadd].xmax, PageTable[fadd].ymax, transparent);
		break;
	case 4:
		MoveBufferNormal( 0, 0, 0, 0, PageTable[fadd].xmax, PageTable[fadd].ymax, transparent);
		break;
	case 7:
		MoveBufferDup( 0, 0, 0, 0, PageTable[fadd].xmax, PageTable[fadd].ymax, transparent);
	}
	docopy=mycopy;
	WritePage=writesave;
	ReadPage=readsave;
}
void cmd_framebuffer(void){
	char *p;
    p = checkstring(cmdline, "WINDOW");
    if(p) {
    	int tempwrite;
    	getargs(&p,7,",");
		uint8_t oldwrite=WritePage;
		if(PageTable[WPN].address==NULL)error("World not created");
    	int32_t fadd=getint(argv[4],0,LastPage);
    	tempwrite=fadd;
    	int x=getint(argv[0],0,PageTable[WPN].xmax-PageTable[tempwrite].xmax);
    	int y=getint(argv[2],0,PageTable[WPN].ymax-PageTable[tempwrite].ymax);
        if(argc==7){
    		char *p=argv[6];
    		if (toupper(*p)!='I'){
    			if(toupper(*p)=='B'){
    				pagesetdone=0;
    				while(!pagesetdone){
    					CheckAbort();
    				}
    			} else error("Syntax");
    		}
        }
        WritePage=fadd;
    	ReadPage=WPN;
    	WindowFrame( x, y);
    	WritePage=ReadPage=oldwrite;
    	return;
    }
    p = checkstring(cmdline, "WRITE");
    if(p) {
		if(PageTable[WPN].address==NULL)error("Framebuffer not created");
		WritePage=ReadPage=WPN;
    	HRes=PageTable[WritePage].xmax;
    	VRes=PageTable[WritePage].xmax;
    	WritePageAddressExternal=ReadPageAddressExternal=(uint32_t)PageTable[WritePage].address;
		return;
    }
    p = checkstring(cmdline, "CLOSE");
    if(p) {
		if(PageTable[WPN].address==NULL)error("Framebuffer not created");
		if(WritePage==WPN)error("Framebuffer is set for write");
		FreeMemorySafe((void *)&PageTable[BPN].address);
		FreeMemorySafe((void *)&PageTable[WPN].address);
    	PageTable[BPN].address=NULL;
    	PageTable[BPN].expand=0;
    	PageTable[BPN].nbytes=0;
    	PageTable[WPN].address=NULL;
    	PageTable[WPN].expand=0;
    	PageTable[WPN].nbytes=0;
    	return;
    }
    p = checkstring(cmdline, "BACKUP");
    if(p) {
		if(PageTable[WPN].address==NULL)error("Framebuffer not created");
    	if(PageTable[BPN].address==NULL){
        	if(PageTable[WPN].xmax*PageTable[WPN].ymax*PageTable[BPN].nbytes>FreeSpaceOnHeap()-32768)error("Not enough memory for framebuffer backup");
    		PageTable[BPN].address=GetMemory(PageTable[WPN].xmax*PageTable[WPN].ymax*PageTable[WPN].nbytes);
        	PageTable[BPN].address=(uint8_t *)PageTable[BPN].address;
        	PageTable[BPN].expand=0;
        	PageTable[BPN].nbytes=(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4));
        	PageTable[BPN].xmax=PageTable[WPN].xmax;
        	PageTable[BPN].ymax=PageTable[WPN].ymax;
        	PageTable[BPN].size=PageTable[WPN].size;
    	}
		int n=(PageTable[WPN].xmax*PageTable[WPN].ymax*PageTable[BPN].nbytes)>>2;
		_Z10copy_wordsPKmPmm((uint32_t *)PageTable[WPN].address,(uint32_t *)PageTable[BPN].address,n);
    	return;
    }
    p = checkstring(cmdline, "RESTORE");
    if(p) {
		getargs(&p,7,",");
		if(PageTable[WPN].address==NULL)error("Framebuffer not created");
		if(PageTable[BPN].address==NULL)error("Framebuffer backup not created");
 		if(argc){
 			uint8_t oldwrite=WritePage;
 			int w=getint(argv[4],1,PageTable[WPN].xmax);
 			int h=getint(argv[6],1,PageTable[WPN].ymax);
 	    	int x=getint(argv[0],0,PageTable[WPN].xmax-w);
 	    	int y=getint(argv[2],0,PageTable[WPN].ymax-h);
 	    	ReadPage=BPN;
 	    	WritePage=WPN;
 	        docopy=mycopy;
 	        MoveBufferNormal( x, y, x, y, w, h, 0);
 	    	ReadPage = WritePage = oldwrite;
 		} else {
 	 		int n=(PageTable[WPN].xmax*PageTable[WPN].ymax*PageTable[BPN].nbytes)>>2;
 			_Z10copy_wordsPKmPmm((uint32_t *)PageTable[BPN].address,(uint32_t *)PageTable[WPN].address,n);
 		}

    	return;
    }
    p = checkstring(cmdline, "CREATE");
    if(p) {
    	getargs(&p,3,",");
        PageTable[WPN].xmax=getint(argv[0],PageTable[WritePage].xmax,(G1Hardware ? 1600: 4096));
    	PageTable[WPN].ymax=getint(argv[2],PageTable[WritePage].ymax,(G1Hardware ? 1200: 2048));
    	FreeMemorySafe((void *)&PageTable[WPN].address);
    	PageTable[WPN].nbytes=(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4));
    	if(PageTable[WPN].xmax*PageTable[WPN].ymax*PageTable[WPN].nbytes>FreeSpaceOnHeap()-32768)error("Not enough memory for framebuffer");
    	PageTable[WPN].address=GetMemory(PageTable[WPN].xmax*PageTable[WPN].ymax*PageTable[WPN].nbytes);
    	PageTable[WPN].address=(uint8_t *)PageTable[WPN].address;
    	PageTable[WPN].expand=0;
    	PageTable[WPN].size= PageTable[WPN].xmax*PageTable[WPN].ymax*PageTable[WPN].nbytes;
    	return;
    }
    error("Syntax");
}

// this function positions the cursor within a PRINT command
void fun_at(void) {
    char buf[27];
	getargs(&ep, 5, ",");
	if(commandfunction(cmdtoken) != cmd_print) error("Invalid function");
//	if((argc == 3 || argc == 5)) error("Incorrect number of arguments");
//	AutoLineWrap = false;
	lastx = CurrentX = getinteger(argv[0]);
	if(argc>=3  && *argv[2])lasty = CurrentY = getinteger(argv[2]);
	if(argc == 5) {
	    PrintPixelMode = getinteger(argv[4]);
    	if(PrintPixelMode < 0 || PrintPixelMode > 7) {
        	PrintPixelMode = 0;
        	error("Number out of bounds");
        }
    } else
	    PrintPixelMode = 0;

    // BJR: VT100 set cursor location: <esc>[y;xf
    //      where x and y are ASCII string integers.
    //      Assumes overall font size of 6x12 pixels (480/80 x 432/36), including gaps between characters and lines

    sprintf(buf, "\033[%d;%df", (int)CurrentY/(FontTable[gui_font >> 4][1] * (gui_font & 0b1111))+1, (int)CurrentX/(FontTable[gui_font >> 4][0] * (gui_font & 0b1111)));
    SerUSBPutS(buf);								                // send it to the USB
	if(PrintPixelMode==2 || PrintPixelMode==5)SerUSBPutS("\033[7m");
    targ=T_STR;
    sret = "\0";                                                    // normally pointing sret to a string in flash is illegal
}
void Free3DMemory(int i){
	FreeMemorySafe((void *)&struct3d[i]->q_vertices);//array of original vertices
	FreeMemorySafe((void *)&struct3d[i]->r_vertices); //array of rotated vertices
	FreeMemorySafe((void *)&struct3d[i]->q_centroids);//array of original vertices
	FreeMemorySafe((void *)&struct3d[i]->r_centroids); //array of rotated vertices
	FreeMemorySafe((void *)&struct3d[i]->facecount); //number of vertices for each face
	FreeMemorySafe((void *)&struct3d[i]->facestart); //index into the face_x_vert table of the start of a given face
	FreeMemorySafe((void *)&struct3d[i]->fill); //fill colours
	FreeMemorySafe((void *)&struct3d[i]->line); //line colours
	FreeMemorySafe((void *)&struct3d[i]->colours);
	FreeMemorySafe((void *)&struct3d[i]->face_x_vert); //list of vertices for each face
	FreeMemorySafe((void *)&struct3d[i]->dots);
	FreeMemorySafe((void *)&struct3d[i]->depth);
	FreeMemorySafe((void *)&struct3d[i]->depthindex);
	FreeMemorySafe((void *)&struct3d[i]->normals);
	FreeMemorySafe((void *)&struct3d[i]->flags);
	FreeMemorySafe((void *)&struct3d[i]);
}
void closeall3d(void){
    int i;
    for(i = 0; i < MAX3D; i++) {
    	if(struct3d[i]!=NULL){
    		Free3DMemory(i);
    	}
    }
    for(i=1; i<4;i++){
    	camera[i].viewplane=-32767;
    }
}
void T_Mult(FLOAT3D *q1, FLOAT3D *q2, FLOAT3D *n){
    FLOAT3D a1=q1[0],a2=q2[0],b1=q1[1],b2=q2[1],c1=q1[2],c2=q2[2],d1=q1[3],d2=q2[3];
    n[0]=a1*a2-b1*b2-c1*c2-d1*d2;
    n[1]=a1*b2+b1*a2+c1*d2-d1*c2;
    n[2]=a1*c2-b1*d2+c1*a2+d1*b2;
    n[3]=a1*d2+b1*c2-c1*b2+d1*a2;
    n[4]=q1[4]*q2[4];
}

void T_Invert(FLOAT3D *q, FLOAT3D *n){
    n[0]=q[0];
    n[1]=-q[1];
    n[2]=-q[2];
    n[3]=-q[3];
    n[4]=q[4];
}

void depthsort(FLOAT3D *farray, int n, int *index){
    int i, j = n, s = 1;
    int t;
    FLOAT3D f;
	while (s) {
		s = 0;
		for (i = 1; i < j; i++) {
			if (farray[i] > farray[i - 1]) {
				f = farray[i];
				farray[i] = farray[i - 1];
				farray[i - 1] = f;
				s = 1;
				if(index!=NULL){
					t=index[i-1];
					index[i-1]=index[i];
					index[i]=t;
				}
			}
		}
		j--;
	}
}
void q_rotate(s_quaternion *in, s_quaternion rotate, s_quaternion *out){
//	PFlt(in->x);PFltComma(in->y);PFltComma(in->z);PFltComma(in->m);PRet();
	s_quaternion temp, qtemp;
	T_Mult((FLOAT3D *)&rotate, (FLOAT3D *)in, (FLOAT3D *)&temp);
	T_Invert((FLOAT3D *)&rotate, (FLOAT3D *)&qtemp);
	T_Mult((FLOAT3D *)&temp, (FLOAT3D *)&qtemp, (FLOAT3D *)out);
//	PFlt(out->x);PFltComma(out->y);PFltComma(out->z);PFltComma(out->m);PRet();
}
void normalise(s_vector *v){
	FLOAT3D n = sqrt3d((v->x) * (v->x) + (v->y) * (v->y) + (v->z) * (v->z) );
	v->x /= n;
	v->y /= n;
	v->z /= n;
}
void display3d(int n, FLOAT3D x, FLOAT3D y, FLOAT3D z, int clear, int nonormals, int depthmode){
	s_vector ray, lighting={0};
	s_vector p1, p2, p3, U, V;
	FLOAT3D x1, y1, z1, tmp;
	FLOAT3D at, bt, ct, t, /*A=0, B=0, */C=1, D=-camera[struct3d[n]->camera].viewplane;
	int maxH=PageTable[WritePage].ymax;
	int maxW=PageTable[WritePage].xmax;
	int vp, v, f, sortindex, csave=0, fsave=0;
	if(struct3d[n]->vmax>4){ //needed for polygon fill
		main_fill_polyX=(TFLOAT  *)GetInternalMemory(struct3d[n]->tot_face_x_vert * sizeof(TFLOAT));
		main_fill_polyY=(TFLOAT  *)GetInternalMemory(struct3d[n]->tot_face_x_vert * sizeof(TFLOAT));
	}
	if(struct3d[n]->xmin!=32767 && clear)DrawRectangle(struct3d[n]->xmin,struct3d[n]->ymin,struct3d[n]->xmax,struct3d[n]->ymax,0);
	struct3d[n]->xmin=32767;
	struct3d[n]->ymin=32767;
	struct3d[n]->xmax=-32767;
	struct3d[n]->ymax=-32767;
	short xcoord[MAX_POLYGON_VERTICES],ycoord[MAX_POLYGON_VERTICES];
	struct3d[n]->distance=0.0;
	for(f=0;f<struct3d[n]->nf;f++){
// calculate the surface normals for each face
		vp=struct3d[n]->facestart[f];
		p1.x=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].x * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].m + x;
		p1.y=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].y  *struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].m + y;
		p1.z=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].z * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].m + z;
		p2.x=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].x * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].m + x;
		p2.y=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].y * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].m + y;
		p2.z=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].z * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].m + z;
		p3.x=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].x * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].m + x;
		p3.y=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].y * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].m + y;
		p3.z=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].z * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].m + z;
		U.x=p2.x-p1.x;  U.y=p2.y-p1.y;  U.z=p2.z-p1.z;
		V.x=p3.x-p1.x;  V.y=p3.y-p1.y;  V.z=p3.z-p1.z;
		struct3d[n]->normals[f].x=U.y * V.z - U.z * V.y;
		struct3d[n]->normals[f].y=U.z * V.x - U.x * V.z;
		struct3d[n]->normals[f].z=U.x * V.y - U.y * V.x;
		normalise(&struct3d[n]->normals[f]);
		ray.x=p1.x - camera[struct3d[n]->camera].x;
		ray.y=p1.y - camera[struct3d[n]->camera].y;
		ray.z=p1.z - camera[struct3d[n]->camera].z;
		normalise(&ray);
		lighting.x=p1.x - struct3d[n]->light.x;
		lighting.y=p1.y - struct3d[n]->light.y;
		lighting.z=p1.z - struct3d[n]->light.z;
		normalise(&lighting);
		struct3d[n]->dots[f] = ray.x * struct3d[n]->normals[f].x + ray.y * struct3d[n]->normals[f].y + ray.z * struct3d[n]->normals[f].z;
        if (depthmode == 0)
        {
            tmp = struct3d[n]->r_centroids[f].m;
            struct3d[n]->depth[f] = sqrt3d(
                (struct3d[n]->r_centroids[f].z * tmp + z - camera[struct3d[n]->camera].z) *
                    (struct3d[n]->r_centroids[f].z * tmp + z - camera[struct3d[n]->camera].z) +
                (struct3d[n]->r_centroids[f].y * tmp + y - camera[struct3d[n]->camera].y) *
                    (struct3d[n]->r_centroids[f].y * tmp + y - camera[struct3d[n]->camera].y) +
                (struct3d[n]->r_centroids[f].x * tmp + x - camera[struct3d[n]->camera].x) *
                    (struct3d[n]->r_centroids[f].x * tmp + x - camera[struct3d[n]->camera].x));
            struct3d[n]->depthindex[f] = f;
            struct3d[n]->distance += struct3d[n]->depth[f];
        }
        else
        {
            FLOAT3D max_depth = -32767.0;
            for (v = 0; v < struct3d[n]->facecount[f]; v++)
            {
                tmp = struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp + v]].m;
                FLOAT3D vertex_depth = sqrt3d(
                    (struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp + v]].z * tmp + z - camera[struct3d[n]->camera].z) *
                        (struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp + v]].z * tmp + z - camera[struct3d[n]->camera].z) +
                    (struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp + v]].y * tmp + y - camera[struct3d[n]->camera].y) *
                        (struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp + v]].y * tmp + y - camera[struct3d[n]->camera].y) +
                    (struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp + v]].x * tmp + x - camera[struct3d[n]->camera].x) *
                        (struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp + v]].x * tmp + x - camera[struct3d[n]->camera].x));
                if (vertex_depth > max_depth)
                {
                    max_depth = vertex_depth;
                }
            }
            struct3d[n]->depth[f] = max_depth;
            struct3d[n]->depthindex[f] = f;
            struct3d[n]->distance += struct3d[n]->depth[f];
        }
	}
	struct3d[n]->distance/=f;
	// sort the distances from the faces to the camera
	depthsort(struct3d[n]->depth, struct3d[n]->nf, struct3d[n]->depthindex);
	// display the forward facing faces in the order of the furthest away first
	for(f=0;f<struct3d[n]->nf;f++){
		sortindex=struct3d[n]->depthindex[f];
		vp=struct3d[n]->facestart[sortindex];
		if(struct3d[n]->flags[sortindex] & 4)struct3d[n]->dots[sortindex]=-struct3d[n]->dots[sortindex];
		if(nonormals || struct3d[n]->dots[sortindex]<0){
			for(v=0;v<struct3d[n]->facecount[sortindex];v++){
				x1=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+v]].x * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+v]].m + x;
				y1=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+v]].y * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+v]].m + y;
				z1=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+v]].z * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+v]].m + z;
// We now have the coordinates in real space so project them
				at=x1-camera[struct3d[n]->camera].x;
				bt=y1-camera[struct3d[n]->camera].y;
				ct=z1-camera[struct3d[n]->camera].z;
				t=-(/*A * x1 + B * y1*/ + C * z1 + D)/(/*A * at + B * bt + */C *ct);
				xcoord[v]=x1+round3d(at*t)+(maxW>>1)-camera[struct3d[n]->camera].x-camera[struct3d[n]->camera].panx;
				if(optiony)ycoord[v]=round3d(y1+bt*t);
				else ycoord[v]=maxH-round3d(y1+bt*t)-1;
				ycoord[v]-=(maxH>>1)-camera[struct3d[n]->camera].y-camera[struct3d[n]->camera].pany;
				if(clear){
					if(xcoord[v]>struct3d[n]->xmax)struct3d[n]->xmax=xcoord[v];
					if(xcoord[v]<struct3d[n]->xmin)struct3d[n]->xmin=xcoord[v];
					if(ycoord[v]>struct3d[n]->ymax)struct3d[n]->ymax=ycoord[v];
					if(ycoord[v]<struct3d[n]->ymin)struct3d[n]->ymin=ycoord[v];
				}
			}
			if((struct3d[n]->flags[sortindex] & 1) == 0) {
				if(struct3d[n]->flags[sortindex] & 10) {
					fsave=struct3d[n]->fill[sortindex];
					csave=struct3d[n]->line[sortindex];
					if(struct3d[n]->flags[sortindex] & 2)struct3d[n]->fill[sortindex]=0xFF0000;
					if(struct3d[n]->flags[sortindex] & 8){
						FLOAT3D lightratio=fabs3d(lighting.x * struct3d[n]->normals[sortindex].x + lighting.y * struct3d[n]->normals[sortindex].y + lighting.z * struct3d[n]->normals[sortindex].z);
						lightratio=(lightratio*struct3d[n]->ambient)+struct3d[n]->ambient;
						int red=(struct3d[n]->fill[sortindex] & 0xFF0000)>>16;
						int green=(struct3d[n]->fill[sortindex] & 0xFF00)>>8;
						int blue=(struct3d[n]->fill[sortindex] & 0xFF);
						int trans=(struct3d[n]->fill[sortindex] & 0xF000000);
						red=(round3d)((FLOAT3D)red*lightratio);
						green=(round3d)((FLOAT3D)green*lightratio);
						blue=(round3d)((FLOAT3D)blue*lightratio);
						struct3d[n]->fill[sortindex]=trans | (red<<16) | (green<<8) | blue;
						red=(struct3d[n]->line[sortindex] & 0xFF0000)>>16;
						green=(struct3d[n]->line[sortindex] & 0xFF00)>>8;
						blue=(struct3d[n]->line[sortindex] & 0xFF);
						trans=(struct3d[n]->line[sortindex] & 0xF000000);
						red=(round3d)((FLOAT3D)red*lightratio);
						green=(round3d)((FLOAT3D)green*lightratio);
						blue=(round3d)((FLOAT3D)blue*lightratio);
						struct3d[n]->line[sortindex]=trans | (red<<16) | (green<<8) | blue;
					}
				}
				DrawPolygon(n, xcoord, ycoord, sortindex);
				if(struct3d[n]->flags[sortindex] & 10){
					struct3d[n]->fill[sortindex]=fsave;
					struct3d[n]->line[sortindex]=csave;
				}
			}
		}
	}
	// Save information about how it was displayed for DRAW3D function and RESTORE command
	struct3d[n]->current.x=x;
	struct3d[n]->current.y=y;
	struct3d[n]->current.z=z;
	struct3d[n]->nonormals=nonormals;
	struct3d[n]->depthmode=depthmode;

	if(struct3d[n]->vmax>4){ //needed for polygon fill
		FreeMemory(main_fill_polyX);
		FreeMemory(main_fill_polyY);
	}

}
void diagnose3d(int n, FLOAT3D x, FLOAT3D y, FLOAT3D z, int sort){
	s_vector ray, normals;
	s_vector p1, p2, p3, U, V;
	FLOAT3D tmp;
	int vp, f, sortindex;
	for(f=0;f<struct3d[n]->nf;f++){
// calculate the surface normals for each face
		vp=struct3d[n]->facestart[f];
		p1.x=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].x * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].m + x;
		p1.y=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].y  *struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].m + y;
		p1.z=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].z * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+1]].m + z;
		p2.x=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].x * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].m + x;
		p2.y=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].y * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].m + y;
		p2.z=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].z * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp+2]].m + z;
		p3.x=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].x * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].m + x;
		p3.y=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].y * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].m + y;
		p3.z=struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].z * struct3d[n]->r_vertices[struct3d[n]->face_x_vert[vp]].m + z;
		U.x=p2.x-p1.x;  U.y=p2.y-p1.y;  U.z=p2.z-p1.z;
		V.x=p3.x-p1.x;  V.y=p3.y-p1.y;  V.z=p3.z-p1.z;
		normals.x=U.y * V.z - U.z * V.y;
		normals.y=U.z * V.x - U.x * V.z;
		normals.z=U.x * V.y - U.y * V.x;
		normalise(&normals);
		ray.x=p1.x - camera[struct3d[n]->camera].x;
		ray.y=p1.y - camera[struct3d[n]->camera].y;
		ray.z=p1.z/*  -camera[struct3d[n]->camera].z*/;
		normalise(&ray);
		struct3d[n]->dots[f] = ray.x * normals.x + ray.y * normals.y + ray.z * normals.z;
		tmp=struct3d[n]->r_centroids[f].m;
		struct3d[n]->depth[f]=sqrt3d(
				(struct3d[n]->r_centroids[f].z * tmp + z - camera[struct3d[n]->camera].z) *
				(struct3d[n]->r_centroids[f].z * tmp + z - camera[struct3d[n]->camera].z) +
				(struct3d[n]->r_centroids[f].y * tmp + y - camera[struct3d[n]->camera].y) *
				(struct3d[n]->r_centroids[f].y * tmp + y - camera[struct3d[n]->camera].y) +
				(struct3d[n]->r_centroids[f].x * tmp + x - camera[struct3d[n]->camera].x) *
				(struct3d[n]->r_centroids[f].x * tmp + x - camera[struct3d[n]->camera].x)
				);
		struct3d[n]->depthindex[f]=f;
	}
	// sort the dot products
	depthsort(struct3d[n]->depth, struct3d[n]->nf, struct3d[n]->depthindex);
	// display the forward facing faces in the order of the furthest away first
	for(f=0;f<struct3d[n]->nf;f++){
		if(sort)sortindex=struct3d[n]->depthindex[f];
		else sortindex=f;
		vp=struct3d[n]->facestart[sortindex];
		MMPrintString("Face ");PInt(sortindex);
		MMPrintString(" at distance ");PFlt(struct3d[n]->depth[f]);
		MMPrintString(" dot product is ");PFlt(struct3d[n]->dots[sortindex]);
		MMPrintString(" so the face is ");MMPrintString(struct3d[n]->dots[sortindex]>0 ? "Hidden" : "Showing");PRet();
	}
}
void cmd_3D(void){
	char *p;
	if((p=checkstring(cmdline, "CREATE"))) {
	   // parameters are
		// 3D object number (1 to MAX3D
		// # of vertices = nv
		// # of faces = nf
		// vertex structure (nv)
		// face array (face number, vertex number)
		// colours array
		// edge colour index array [nf]
		// fill colour index array [nf]
		// centroid structure [nf]
		// normals structure [nf]
		MMFLOAT *vertex;
		TFLOAT tmp;
		long long int *faces, *facecount, *facecountindex, *colours, *linecolour=NULL, *fillcolour=NULL;
		getargs(&p,19,",");
		if(argc<17)error("Argument count");
		int c, colourcount=0, vp, v, f, fc=0, n=getint(argv[0],1,MAX3D);
		if(struct3d[n]!=NULL)error("Object already exists");
		int nv=getinteger(argv[2]);
		if(nv<3)error("3D object must have a minimum of 3 vertices");
		int nf=getinteger(argv[4]);
		if(nf<1)error("3D object must have a minimum of 1 face");
		int cam=getint(argv[6],1,MAXCAM);
		vertex = (MMFLOAT *)findvar(argv[8], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
		if((uint32_t)vertex!=(uint32_t)vartbl[VarIndex].val.s)error("Vertex array must be a 2D floating point array");
		if(vartbl[VarIndex].type & T_NBR) {
			if(vartbl[VarIndex].dims[2] != 0) error("Vertex array must be a 2D floating point array");
			if(vartbl[VarIndex].dims[0] - OptionBase!= 2) {		// Not an array
				error("Vertex array must have 3 elements in first dimension");
			}
			if(vartbl[VarIndex].dims[1] - OptionBase < nv-1) {		// Not an array
				error("Vertex array too small");
			}
		} else error("Vertex array must be a 2D floating point array");

		facecount = (long long int *)findvar(argv[10], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
		if((uint32_t)facecount!=(uint32_t)vartbl[VarIndex].val.s)error("Vertex count array must be a 1D integer array");
		if(vartbl[VarIndex].type & T_INT) {
			if(vartbl[VarIndex].dims[1] != 0) error("Vertex count array must be a 1D integer array");
			if(vartbl[VarIndex].dims[0] - OptionBase< nf-1) {		// Not an array
				error("Vertex count array too small");
			}
		} else error("Vertex count array must be a 1D integer array");
		facecountindex=facecount;
		for(f=0;f<nf;f++)fc += (*facecountindex++);

		faces = (long long int *)findvar(argv[12], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
		if((uint32_t)faces!=(uint32_t)vartbl[VarIndex].val.s)error("Face/vertex array must be a 1D integer array");
		if(vartbl[VarIndex].type & T_INT) {
			if(vartbl[VarIndex].dims[1] != 0) error("Face/vertex array must be a 1D integer array");
			if(vartbl[VarIndex].dims[0] - OptionBase< fc-1) {		// Not an array
				error("Face/vertex array too small");
			}
		} else error("Face/vertex array must be a 1D integer array");

		colours = (long long int *)findvar(argv[14], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
		if((uint32_t)colours!=(uint32_t)vartbl[VarIndex].val.s)error("Colour array must be a 1D integer array");
		if(vartbl[VarIndex].type & T_INT) {
			if(vartbl[VarIndex].dims[1] != 0) error("Colour array must be a 1D integer array");
			colourcount=vartbl[VarIndex].dims[0] - OptionBase + 1;
		} else error("Colour array must be a 1D integer array");


		if(argc>=17 && *argv[16]){
			linecolour = (long long int *)findvar(argv[16], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
			if((uint32_t)linecolour!=(uint32_t)vartbl[VarIndex].val.s)error("Line colour array must be a 1D integer array");
			if(vartbl[VarIndex].type & T_INT) {
				if(vartbl[VarIndex].dims[1] != 0) error("Line colour  array must be a 1D integer array");
				if(vartbl[VarIndex].dims[0] - OptionBase< nf-1) {		// Not an array
					error("Line colour  array too small");
				}
			} else error("Line colour must be a 1D integer array");
		}

		if(argc==19){
			fillcolour = (long long int *)findvar(argv[18], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
			if((uint32_t)fillcolour!=(uint32_t)vartbl[VarIndex].val.s)error("Fill colour array must be a 1D integer array");
			if(vartbl[VarIndex].type & T_INT) {
				if(vartbl[VarIndex].dims[1] != 0) error("Fill colour array must be a 1D integer array");
				if(vartbl[VarIndex].dims[0] - OptionBase< nf-1) {		// Not an array
					error("Fill colour array too small");
				}
			} else error("Fill colour must be a 1D integer array");
		}
		// The data look valid so now create the object in memory
		struct3d[n]=GetInternalMemory(sizeof(struct D3D));
		struct3d[n]->nf=nf;
		struct3d[n]->nv=nv;
		struct3d[n]->current.x=-32767;
		struct3d[n]->current.y=-32767;
		struct3d[n]->current.z=-32767;
		struct3d[n]->xmin=32767;
		struct3d[n]->ymin=32767;
		struct3d[n]->xmax=-32767;
		struct3d[n]->ymax=-32767;
		struct3d[n]->camera=cam;
		struct3d[n]->q_vertices=NULL;//array of original vertices
		struct3d[n]->r_vertices=NULL; //array of rotated vertices
		struct3d[n]->q_centroids=NULL;//array of original vertices
		struct3d[n]->r_centroids=NULL; //array of rotated vertices
		struct3d[n]->facecount=NULL; //number of vertices for each face
		struct3d[n]->facestart=NULL; //index into the face_x_vert table of the start of a given face
		struct3d[n]->fill=NULL; //fill colours
		struct3d[n]->line=NULL; //line colours
		struct3d[n]->colours=NULL;
		struct3d[n]->face_x_vert=NULL; //list of vertices for each face
		struct3d[n]->light.x=0;
		struct3d[n]->light.y=0;
		struct3d[n]->light.z=0;
		struct3d[n]->ambient=0;
		struct3d[n]->nonormals=0;
		struct3d[n]->depthmode=0;
		// load up things that have one entry per vertex
		struct3d[n]->q_vertices=GetMemory(struct3d[n]->nv * sizeof(struct t_quaternion));
		struct3d[n]->r_vertices=GetMemory(struct3d[n]->nv * sizeof(struct t_quaternion));
		for(v=0;v<struct3d[n]->nv;v++){
			FLOAT3D m=0.0;
			struct3d[n]->q_vertices[v].x=(FLOAT3D)(*vertex++);
			m+=struct3d[n]->q_vertices[v].x*struct3d[n]->q_vertices[v].x;
			struct3d[n]->q_vertices[v].y=*vertex++;
			m+=struct3d[n]->q_vertices[v].y*struct3d[n]->q_vertices[v].y;
			struct3d[n]->q_vertices[v].z=*vertex++;
			m+=struct3d[n]->q_vertices[v].z*struct3d[n]->q_vertices[v].z;
			if(m){
				m=sqrt(m);
				struct3d[n]->q_vertices[v].x=struct3d[n]->q_vertices[v].x/m;
				struct3d[n]->q_vertices[v].y=struct3d[n]->q_vertices[v].y/m;
				struct3d[n]->q_vertices[v].z=struct3d[n]->q_vertices[v].z/m;
				struct3d[n]->q_vertices[v].w=0.0;
				struct3d[n]->q_vertices[v].m=m;
			} else {
				struct3d[n]->q_vertices[v].x=0;
				struct3d[n]->q_vertices[v].y=0;
				struct3d[n]->q_vertices[v].z=0;
				struct3d[n]->q_vertices[v].w=0.0;
				struct3d[n]->q_vertices[v].m=1.0;
			}
			mycpy(&struct3d[n]->r_vertices[v],&struct3d[n]->q_vertices[v], sizeof(s_quaternion));
		}
		struct3d[n]->tot_face_x_vert=0;
		//load up things that have one entry per face
		struct3d[n]->vmax=0;
		struct3d[n]->facecount=GetMemory(struct3d[n]->nf * sizeof(uint16_t));
		struct3d[n]->facestart=GetMemory(struct3d[n]->nf * sizeof(uint16_t));
		struct3d[n]->fill=GetMemory(struct3d[n]->nf * sizeof(uint32_t));
		struct3d[n]->line=GetMemory(struct3d[n]->nf * sizeof(uint32_t));
		struct3d[n]->r_centroids=GetMemory(struct3d[n]->nf * sizeof(struct t_quaternion));
		struct3d[n]->q_centroids=GetMemory(struct3d[n]->nf * sizeof(struct t_quaternion));
		struct3d[n]->dots=GetMemory(struct3d[n]->nf * sizeof(MMFLOAT));
		struct3d[n]->depth=GetMemory(struct3d[n]->nf * sizeof(MMFLOAT));
		struct3d[n]->flags=GetMemory(struct3d[n]->nf * sizeof(uint8_t));
		struct3d[n]->depthindex=GetMemory(struct3d[n]->nf * sizeof(int));
		struct3d[n]->normals=GetMemory(struct3d[n]->nf * sizeof(struct SVD));
		for(f=0;f<struct3d[n]->nf;f++){
			struct3d[n]->facecount[f]=*facecount++;
			if(struct3d[n]->facecount[f]<3){
				Free3DMemory(n);
				error("Vertex count less than 3 for face %",f+OptionBase);
			}
			if(struct3d[n]->facecount[f]>struct3d[n]->vmax)struct3d[n]->vmax=struct3d[n]->facecount[f];
			struct3d[n]->facestart[f]=struct3d[n]->tot_face_x_vert;
			struct3d[n]->tot_face_x_vert+=struct3d[n]->facecount[f];
		}
		// load up the array that holds all the face vertex information
		struct3d[n]->face_x_vert=GetMemory(struct3d[n]->tot_face_x_vert * sizeof(uint16_t)); // allocate memory for the list of vertices per face
		struct3d[n]->colours=GetMemory(colourcount * sizeof(uint32_t));
		for(c=0; c<colourcount;c++){
			struct3d[n]->colours[c]=(uint32_t)*colours++;
		}
		for(f=0;f<struct3d[n]->tot_face_x_vert;f++){
			struct3d[n]->face_x_vert[f]=*faces++;
		}
		for(f=0;f<struct3d[n]->nf;f++){
			if(linecolour!=NULL){
				int index=(*linecolour++) - OptionBase;
				if(index>=colourcount || index<0){
					Free3DMemory(n);
					error("Edge colour Index %",index);
				}
				struct3d[n]->line[f]=struct3d[n]->colours[index];
			} else struct3d[n]->line[f]=gui_fcolour;
			if(fillcolour!=NULL){
				int index=(*fillcolour++) - OptionBase;
				if(index>=colourcount || index<0){
					Free3DMemory(n);
					error("Fill colour Index %",index);
				}
				struct3d[n]->fill[f]=struct3d[n]->colours[index];
			} else struct3d[n]->fill[f]=0xFFFFFFFF;
			FLOAT3D x=0, y=0, z=0, scale;
			vp=struct3d[n]->facestart[f];
// calculate the centroids of each face

			for(v=0;v<struct3d[n]->facecount[f];v++){
				tmp=struct3d[n]->q_vertices[struct3d[n]->face_x_vert[vp+v]].m;
				x+=struct3d[n]->q_vertices[struct3d[n]->face_x_vert[vp+v]].x*tmp;
				y+=struct3d[n]->q_vertices[struct3d[n]->face_x_vert[vp+v]].y*tmp;
				z+=struct3d[n]->q_vertices[struct3d[n]->face_x_vert[vp+v]].z*tmp;
			}
			x/=(FLOAT3D)struct3d[n]->facecount[f];
			y/=(FLOAT3D)struct3d[n]->facecount[f];
			z/=(FLOAT3D)struct3d[n]->facecount[f];
			struct3d[n]->q_centroids[f].x=x;
			struct3d[n]->q_centroids[f].y=y;
			struct3d[n]->q_centroids[f].z=z;
			scale=sqrt(struct3d[n]->q_centroids[f].x*struct3d[n]->q_centroids[f].x +
					struct3d[n]->q_centroids[f].y*struct3d[n]->q_centroids[f].y +
					struct3d[n]->q_centroids[f].z*struct3d[n]->q_centroids[f].z);
			struct3d[n]->q_centroids[f].x/=scale;
			struct3d[n]->q_centroids[f].y/=scale;
			struct3d[n]->q_centroids[f].z/=scale;
			struct3d[n]->q_centroids[f].m=scale;
			struct3d[n]->q_centroids[f].w=0;
			mycpy(&struct3d[n]->r_centroids[f],&struct3d[n]->q_centroids[f], sizeof(s_quaternion));
			}
		return;
	} else if((p=checkstring(cmdline, "DIAGNOSE"))) {
		getargs(&p,9,",");
		if(argc<7)error("Argument count");
		int n=getint(argv[0],1,MAX3D);
		int x=getint(argv[2],-32766,32766);
		int y=getint(argv[4],-32766,32766);
		int z=getinteger(argv[6]);
		int sort=1;
		if(argc==9)sort=getint(argv[8],0,1);
		if(struct3d[n]==NULL)error("Object % does not exist",n);
		if(camera[struct3d[n]->camera].viewplane==-32767)error("Camera position not defined");
		diagnose3d(n, x, y, z, sort);
		return;
	} else if((p=checkstring(cmdline, "LIGHT"))) {
		getargs(&p,9,",");
		if(argc!=9)error("Argument count");
		int n=getint(argv[0],1,MAX3D);
		struct3d[n]->light.x=getint(argv[2],-32766,32766);
		struct3d[n]->light.y=getint(argv[4],-32766,32766);
		struct3d[n]->light.z=getint(argv[6],-32766,32766);
		struct3d[n]->ambient=(FLOAT3D)(getint(argv[8],0,100))/100.0;
		return;
	} else if((p=checkstring(cmdline, "SHOW"))) {
		getargs(&p,11,",");
		if(argc<7)error("Argument count");
		int n=getint(argv[0],1,MAX3D);
		int x=getint(argv[2],-32766,32766);
		int y=getint(argv[4],-32766,32766);
		int z=getinteger(argv[6]);
		int nonormals=0;
        int depthmode = 0;
        if (argc >= 9 && *argv[8])
            nonormals = getint(argv[8], 0, 1);
        if (argc == 11)
            depthmode = getint(argv[10], 0, 1);
		if(struct3d[n]==NULL)error("Object % does not exist",n);
		if(camera[struct3d[n]->camera].viewplane==-32767)error("Camera position not defined");
		display3d(n, x, y, z, 1, nonormals, depthmode);
		return;
	} else if((p=checkstring(cmdline, "SET FLAGS"))) {
		int i, face, nbr;
		getargs(&p, ((MAX_ARG_COUNT-1) * 2) - 1, ",");
		if((argc & 0b11) != 0b11) error("Invalid syntax");
		int n=getint(argv[0],1,MAX3D);
		int flag=getint(argv[2],0,255);
	    // step over the equals sign and get the value for the assignment
	    for(i = 4; i < argc; i += 4) {
	        face = getinteger(argv[i]);
	        nbr = getinteger(argv[i + 2]);

	        if(nbr <= 0 || nbr>struct3d[n]->nf-face) error("Invalid argument");

	        while(--nbr>=0) {
	        	struct3d[n]->flags[face+nbr]=flag;
	        }
	    }
	} else if((p=checkstring(cmdline, "ROTATE"))) {
		void *ptr1 = NULL;
		int i, n, v, f;
		s_quaternion q1;
		MMFLOAT *q=NULL;
		getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
		if((argc & 0x01 || argc<3) == 0) error("Argument count");
		ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
		if(vartbl[VarIndex].type & T_NBR) {
			if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
			if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
				error("Argument 1 must be a 5 element floating point array");
			}
			if(vartbl[VarIndex].dims[0] - OptionBase!=4)error("Argument 1 must be a 5 element floating point array");
			q = (MMFLOAT *)ptr1;
			if((uint32_t)ptr1!=(uint32_t)vartbl[VarIndex].val.s)error("Syntax");
		} else error("Argument 1 must be a 5 element floating point array");
		q1.w=(FLOAT3D)(*q++);
		q1.x=(FLOAT3D)(*q++);
		q1.y=(FLOAT3D)(*q++);
		q1.z=(FLOAT3D)(*q++);
		q1.m=(FLOAT3D)(*q);
		for(i = 2; i < argc; i += 2) {
			n=getint(argv[i],1,MAX3D);
			if(struct3d[n]==NULL)error("Object % does not exist",n);
			for(v=0;v<struct3d[n]->nv;v++){
				q_rotate(&struct3d[n]->q_vertices[v],q1,&struct3d[n]->r_vertices[v]);
			}
			for(f=0;f<struct3d[n]->nf;f++){
				q_rotate(&struct3d[n]->q_centroids[f],q1,&struct3d[n]->r_centroids[f]);
			}
		}
		return;
	} else if((p=checkstring(cmdline, "HIDE ALL"))) {
		for(int i=1;i<=MAX3D;i++){
			if(struct3d[i]!=NULL && struct3d[i]->xmin!=32767){
				DrawRectangle(struct3d[i]->xmin,struct3d[i]->ymin,struct3d[i]->xmax,struct3d[i]->ymax,0);
				struct3d[i]->xmin=32767;
				struct3d[i]->ymin=32767;
				struct3d[i]->xmax=-32767;
				struct3d[i]->ymax=-32767;
			}
		}
		return;
	} else if((p=checkstring(cmdline, "RESET"))) {
		int i, n;
		int v, f;
		getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
		if((argc & 0x01 || argc<3) == 0) error("Argument count");
		for(i = 0; i < argc; i += 2) {
			n=getint(argv[i],1,MAX3D);
			for(v=0;v<struct3d[n]->nv;v++){
				mycpy(&struct3d[n]->q_vertices[v],&struct3d[n]->r_vertices[v], sizeof(s_quaternion));
			}
			for(f=0;f<struct3d[n]->nf;f++){
				mycpy(&struct3d[n]->q_centroids[f],&struct3d[n]->r_centroids[f], sizeof(s_quaternion));
			}
		}
		return;
	} else if((p=checkstring(cmdline, "HIDE"))) {
		int i, n;
		getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
		if((argc & 0x01 || argc<3) == 0) error("Argument count");
		for(i = 0; i < argc; i += 2) {
			n=getint(argv[i],1,MAX3D);
			if(struct3d[n]==NULL)error("Object % does not exist",n);
			if(struct3d[n]->xmin==32767)return;
			DrawRectangle(struct3d[n]->xmin,struct3d[n]->ymin,struct3d[n]->xmax,struct3d[n]->ymax,0);
			struct3d[n]->xmin=32767;
			struct3d[n]->ymin=32767;
			struct3d[n]->xmax=-32767;
			struct3d[n]->ymax=-32767;
		}
		return;
	} else if((p=checkstring(cmdline, "RESTORE"))) {
		int i, n;
		getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
		if((argc & 0x01 || argc<3) == 0) error("Argument count");
		for(i = 0; i < argc; i += 2) {
			n=getint(argv[i],1,MAX3D);
			if(struct3d[n]==NULL)error("Object % does not exist",n);
			if(struct3d[n]->xmin!=32767)error("Object % is not hidden",n);
			display3d(n, struct3d[n]->current.x, struct3d[n]->current.y, struct3d[n]->current.z, 1, struct3d[n]->nonormals, struct3d[n]->depthmode);
		}
		return;
	} else if((p=checkstring(cmdline, "WRITE"))) {
		getargs(&p,11,",");
		if(argc<7)error("Argument count");
		int n=getint(argv[0],1,MAX3D);
		int x=getint(argv[2],-32766,32766);
		int y=getint(argv[4],-32766,32766);
		int z=getinteger(argv[6]);
		int nonormals=0;
        int depthmode = 0;
        if (argc >= 9 && *argv[8])
            nonormals = getint(argv[8], 0, 1);
        if (argc == 11)
            depthmode = getint(argv[10], 0, 1);
		if(struct3d[n]==NULL)error("Object % does not exist",n);
		if(camera[struct3d[n]->camera].viewplane==-32767)error("Camera position not defined");
		display3d(n, x, y, z, 0, nonormals, depthmode);
		return;
	} else if((p=checkstring(cmdline, "CLOSE ALL"))) {
		closeall3d();
		return;
	} else if((p=checkstring(cmdline, "CLOSE"))) {
		int i, n;
		getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
		if((argc & 0x01 || argc<3) == 0) error("Argument count");
		for(i = 0; i < argc; i += 2) {
			n=getint(argv[i],1,MAX3D);
			if(struct3d[n]==NULL)error("Object % does not exist",n);
			if(struct3d[n]->xmin!=32767)DrawRectangle(struct3d[n]->xmin,struct3d[n]->ymin,struct3d[n]->xmax,struct3d[n]->ymax,0);
			Free3DMemory(n);
		}
		return;
	} else if((p=checkstring(cmdline, "CAMERA"))) {
		getargs(&p,11,",");
		if(argc<3)error("Argument count");
		int n=getint(argv[0],1,MAXCAM);
		camera[n].viewplane=getnumber(argv[2]);
		camera[n].x=(FLOAT3D)0;
		camera[n].y=(FLOAT3D)0;
		camera[n].panx=(FLOAT3D)0;
		camera[n].pany=(FLOAT3D)0;
		camera[n].z=0.0;
		if(argc>=5 && *argv[4])	camera[n].x=getnumber(argv[4]);
		if(camera[n].x > 32766 || camera[n].x < -32766 )error("Valid is -32766 to 32766");
		if(argc>=7 && *argv[6])	camera[n].y=getnumber(argv[6]);
		if(camera[n].y > 32766 || camera[n].x < -32766 )error("Valid is -32766 to 32766");
		if(argc>=9 && *argv[8])	camera[n].panx=getint(argv[8],-32766-camera[n].x,32766-camera[n].x);
		if(argc==11 )camera[n].pany=getint(argv[10],-32766-camera[n].y,32766-camera[n].y);
		return;
	} else {
		error("Syntax");
	}
}
void fun_3D(void){
	char *p;
	if((p=checkstring(ep, "XMIN"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
    	fret=struct3d[n]->xmin;
	} else if((p=checkstring(ep, "XMAX"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
    	fret=struct3d[n]->xmax;
	} else if((p=checkstring(ep, "YMIN"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
    	fret=struct3d[n]->ymin;
	} else if((p=checkstring(ep, "YMAX"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
    	fret=struct3d[n]->ymax;
	} else if((p=checkstring(ep, "X"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
    	fret=struct3d[n]->current.x;
	} else if((p=checkstring(ep, "Y"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
		fret=struct3d[n]->current.y;
	} else if((p=checkstring(ep, "DISTANCE"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
    	fret=struct3d[n]->distance;
	} else if((p=checkstring(ep, "Z"))) {
    	getargs(&p,1,",");
    	int n=getint(argv[0],1,MAX3D);
		if(struct3d[n]==NULL)error("Object does not exist");
    	fret=struct3d[n]->current.z;
	} else error("Syntax");
	targ=T_NBR;
}
void cmd_image(void){
	char *p;
	if((p=checkstring(cmdline, "ROTATE_FAST"))) {
    	int dontcopyblack=0;
    	getargs(&p,17,",");
    	if(argc<13)error("Argument count");
    	int cursorhidden=0;
        if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    		hidecursor(0);
    		cursorhidden=1;
        }
        if(argc>=15 && *argv[14]){
        	if(checkstring(argv[14], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[14],0,LastPage);
        	}
        }
        int maxWR=PageTable[ReadPage].xmax;
    	int maxHR=PageTable[ReadPage].ymax;
    	int maxH=PageTable[WritePage].ymax;
        int maxW=PageTable[WritePage].xmax;
    	int Scale=(PageTable[WritePage].expand ? 2 : 1);
		uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    	uint32_t ox=getint(argv[0],0,maxWR-1);
    	uint32_t oy=getint(argv[2],0,maxHR-1);
    	uint32_t width=getint(argv[4],1,maxWR-ox);
    	uint32_t height=getint(argv[6],1,maxHR-oy);
    	uint32_t nx=getint(argv[8],0,maxW-1);
    	uint32_t ny=getint(argv[10],0,maxH-1);
    	float angle=getnumber(argv[12]) * (float) Pi / 180;
        if(argc==17)dontcopyblack=getint(argv[16],0,1);
        int sinma = (int)  (tsin(-angle) * (float)65536.0);
        int cosma = (int)(tcos(-angle) * (float)65536.0);
		float hwidth = width/2;
		float hheight = height/2;
		if((width & 1) == 0)hwidth-=0.5;
		if((height & 1) == 0)hheight-=0.5;
		int c, x, y, xs, ys, xx, yy;
		float xt, yt;
		int multiplier=(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4));
		short *buff;
		char *bbuff;
        if(width*height*multiplier<LBUFFSIZE){
			buff=(short *)linebuff;
        } else {
        	buff=GetTempMemory(width * height * multiplier);
        }
		bbuff=(char*)buff;
		ReadBufferFast(ox,oy,ox+width-1,oy+height-1, (char *)bbuff);
		for(x = 0; x < width; x++) {
			xt = x - hwidth;
			for(y = 0; y < height; y++) {
				yt = y - hheight;
				xs = ((int)(cosma * xt - sinma * yt)>>16)+width/2;
				ys = ((int)(sinma * xt + cosma * yt)>>16)+height/2;
//				PInt(xs);PIntComma(ys);PRet();
				if(xs >= 0 && xs < width && ys >= 0 && ys < height) {
					if(VideoColour==8){
						c=bbuff[xs+ys*width];
						if(dontcopyblack==0 || c){
							yy=y+ny;
							xx=x+nx;
							if(optiony)yy=maxH-1-yy;
							if(xx<maxW && yy<maxH && xx>=0 && yy>=0){
								if(Scale==1){
									*(uint8_t *)((yy * maxW + xx) + wpa)=(uint8_t)c;
								} else {
									*(uint8_t *)((yy * 2 * maxW + xx) +wpa)=(uint8_t)c;
									*(uint8_t *)(((yy * 2 + 1) * maxW + xx) + wpa)=(uint8_t)c;
								}
							}
						}
					} else if(VideoColour<=16) {
						c=buff[xs+ys*width];
						if(dontcopyblack==0 || c){
							yy=y+ny;
							xx=x+nx;
							if(optiony)yy=maxH-1-yy;
							if(xx<maxW && yy<maxH && xx>=0 && yy>=0){
								if(Scale==1){
									*(uint16_t *)((yy * maxW + xx) * 2 + wpa)=(uint16_t)c;
								} else {
									*(uint16_t *)((yy * 2 * maxW + xx) * 2 + wpa)=(uint16_t)c;
									*(uint16_t *)(((yy * 2 + 1) * maxW + xx) * 2 + wpa)=(uint16_t)c;
								}
							}
						}
					} else {
						c=buff[xs+ys*width];
						if(dontcopyblack==0 || c){
							yy=y+ny;
							xx=x+nx;
							if(optiony)yy=maxH-1-yy;
							if(xx<maxW && yy<maxH && xx>=0 && yy>=0){
								if(Scale==1){
									*(uint32_t *)((yy * maxW + xx) * 4 + wpa)=(uint32_t)c;
								} else {
									*(uint32_t *)((yy * 2 * maxW + xx) * 4 + wpa)=(uint32_t)c;
									*(uint32_t *)(((yy * 2 + 1) * maxW + xx) * 4 + wpa)=(uint32_t)c;
								}
							}
						}
					}
				}
			}
		}
        ReadPage=WritePage;
        if(cursorhidden)showcursor(0, xcursor,ycursor);
    	return;
    }
	if((p=checkstring(cmdline, "ROTATE"))) {
    	static uint8_t ***temp2=NULL;
    	int dontcopyblack=0;
    	getargs(&p,17,",");
    	if(argc<13)error("Argument count");
        int cursorhidden=0;
        if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    		hidecursor(0);
    		cursorhidden=1;
        }
        if(argc>=15 && *argv[14]){
        	if(checkstring(argv[14], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[14],0,LastPage);
        	}
        }
        int maxWR=PageTable[ReadPage].xmax;
    	int maxHR=PageTable[ReadPage].ymax;
    	int maxH=PageTable[WritePage].ymax;
        int maxW=PageTable[WritePage].xmax;
    	uint32_t x=getint(argv[0],0,maxWR-1);
    	uint32_t y=getint(argv[2],0,maxHR-1);
    	uint32_t w=getint(argv[4],1,maxWR-x);
    	uint32_t h=getint(argv[6],1,maxHR-y);
    	uint32_t nx=getint(argv[8],0,maxW-1);
    	uint32_t ny=getint(argv[10],0,maxH-1);
    	uint8_t*** output = alloc3df(3, h, w);
    	float angle=-getnumber(argv[12]);
        if(argc==17)dontcopyblack=getint(argv[16],0,1);
    	copytofloat(output, x, y, w, h);
    	temp2=rotate (output, h, w, angle);
    	dealloc3df (output, 3, h, w);
    	copyfromfloat(temp2, nx, ny, w, h, dontcopyblack);
    	dealloc3df (temp2, 3, h, w);
        ReadPage=WritePage;
        if(cursorhidden)showcursor(0, xcursor,ycursor);
    	return;
    }
	if((p=checkstring(cmdline, "WARP_H"))) {
    	getargs(&p,23,",");
    	if(argc<19)error("Argument count");
        int cursorhidden=0;
    	int dontcopyblack=0;
        if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    		hidecursor(0);
    		cursorhidden=1;
        }
        if(argc>=21 && *argv[20]){
        	if(checkstring(argv[20], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[20],0,LastPage);
        	}
        }
        int maxWR=PageTable[ReadPage].xmax;
    	int maxHR=PageTable[ReadPage].ymax;
    	int maxH=PageTable[WritePage].ymax;
        int maxW=PageTable[WritePage].xmax;
    	int Scale=(PageTable[WritePage].expand ? 2 : 1);
		uint32_t wpa=(uint32_t)PageTable[WritePage].address;
        int xx, yy, ys, ye, ww, c, xp, yp;
        float ratiox, ratioy;
		int px, py, yshift;
    	//get details of the source rectangle
        int32_t x=getint(argv[0],0,maxWR-1);
    	int32_t y=getint(argv[2],0,maxHR-1);
    	int32_t w1=getint(argv[4],1,maxWR-x);
    	int32_t h1=getint(argv[6],1,maxHR-y);
    	// get details of the trapezoid
    	int32_t nx=getint(argv[8],0,maxW-1);
    	int32_t ny=getint(argv[10],0,maxH-1);
    	int32_t nh=getint(argv[12],1,maxH-ny);
    	int32_t nx2=getint(argv[14],nx+1,maxW-1);
    	int32_t ny2=getint(argv[16],0,maxH-1);
    	int32_t nh2=getint(argv[18],1,maxH-ny2);
		float x_ratio = ((float)w1/(float)(nx2-nx+1));
		float y_ratio = ((float)h1/(float)(nh2)) ;
    	// read in the rectangle
        if(argc==23)dontcopyblack=getint(argv[22],0,1);
		int multiplier=(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4));
		short *buff;
		char *bbuff;
        if(w1*h1*multiplier<LBUFFSIZE){
			buff=(short *)linebuff;
        } else {
        	buff=GetTempMemory(w1 * h1 * multiplier);
        }
		bbuff=(char*)buff;
		ReadBufferFast(x,y,x+w1-1,y+h1-1, (char *)bbuff);
		ww=nx2-nx;
		if(VideoColour==8){
			for (xx=0;xx<ww;xx++) {
				ratiox=(float)xx/(float)(ww-1);
				ys=ny-(int)(ratiox*(float)(ny-ny2));
				ye=ys + nh - (int)(ratiox*(float)(nh-nh2));
				ratioy=(float)(nh2)/(float)(ye-ys);
				px = (float)(xx)*ratioy*x_ratio;
				yshift=(int)(y_ratio*ratioy*(float)65536.0);
				routinechecks(1);
				for(yy=0;yy<ye-ys;yy++){
					py = (yy*yshift)>>16;
					c=bbuff[px+py*w1];
					if(dontcopyblack==0 || c){
						yp=yy+ys;
						xp=xx+nx;
						if(optiony)yy=maxH-1-yy;
						if(xp<maxW && yp<maxH && xp>=0 && yp>=0){
							if(Scale==1){
								*(uint8_t *)((yp * maxW + xp) + wpa)=(uint8_t)c;
							} else {
								*(uint8_t *)((yp * 2 * maxW + xp) + wpa)=(uint8_t)c;
								*(uint8_t *)(((yp * 2 + 1) * maxW + xp) + wpa)=(uint8_t)c;
							}
						}
					}
				}
			}
		} else if(VideoColour<=16) {
			for (xx=0;xx<ww;xx++) {
				ratiox=(float)xx/(float)(ww-1);
				ys=ny-(int)(ratiox*(float)(ny-ny2));
				ye=ys + nh - (int)(ratiox*(float)(nh-nh2));
				ratioy=(float)(nh2)/(float)(ye-ys);
				px = (float)(xx)*ratioy*x_ratio;
				yshift=(int)(y_ratio*ratioy*(float)65536.0);
				routinechecks(1);
				for(yy=0;yy<ye-ys;yy++){
					py = (yy*yshift)>>16;
					c=buff[px+py*w1];
					if(dontcopyblack==0 || c){
						yp=yy+ys;
						xp=xx+nx;
						if(optiony)yy=maxH-1-yy;
						if(xp<maxW && yp<maxH && xp>=0 && yp>=0){
							if(Scale==1){
								*(uint16_t *)((yp * maxW + xp) * 2 + wpa)=(uint16_t)c;
							} else {
								*(uint16_t *)((yp * 2 * maxW + xp) * 2 + wpa)=(uint16_t)c;
								*(uint16_t *)(((yp * 2 + 1) * maxW + xp) * 2 + wpa)=(uint16_t)c;
							}
						}
					}
				}
			}
		} else {
			for (xx=0;xx<ww;xx++) {
				ratiox=(float)xx/(float)(ww-1);
				ys=ny-(int)(ratiox*(float)(ny-ny2));
				ye=ys + nh - (int)(ratiox*(float)(nh-nh2));
				ratioy=(float)(nh2)/(float)(ye-ys);
				px = (float)(xx)*ratioy*x_ratio;
				yshift=(int)(y_ratio*ratioy*(float)65536.0);
				routinechecks(1);
				for(yy=0;yy<ye-ys;yy++){
					py = (yy*yshift)>>16;
					c=buff[px+py*w1];
					if(dontcopyblack==0 || c){
						yp=yy+ys;
						xp=xx+nx;
						if(optiony)yy=maxH-1-yy;
						if(xp<maxW && yp<maxH && xp>=0 && yp>=0){
							if(Scale==1){
								*(uint32_t *)((yp * maxW + xp) * 4 + wpa)=(uint32_t)c;
							} else {
								*(uint32_t *)((yp * 2 * maxW + xp) * 4 + wpa)=(uint32_t)c;
								*(uint32_t *)(((yp * 2 + 1) * maxW + xp) * 4 + wpa)=(uint32_t)c;
							}
						}
					}
				}
			}
		}
		// tidy up
        ReadPage=WritePage;
        if(cursorhidden)showcursor(0, xcursor,ycursor);
        return;
	}

	if((p=checkstring(cmdline, "WARP_V"))) {
    	getargs(&p,23,",");
       	if(argc<19)error("Argument count");
        int cursorhidden=0;
    	int dontcopyblack=0;
        if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    		hidecursor(0);
    		cursorhidden=1;
        }
        if(argc>=21 && *argv[20]){
        	if(checkstring(argv[20], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[20],0,LastPage);
        	}
        }
        int maxWR=PageTable[ReadPage].xmax;
    	int maxHR=PageTable[ReadPage].ymax;
    	int maxH=PageTable[WritePage].ymax;
        int maxW=PageTable[WritePage].xmax;
    	int Scale=(PageTable[WritePage].expand ? 2 : 1);
		uint32_t wpa=(uint32_t)PageTable[WritePage].address;
        int xx, yy, xs, xe, wh, c, xp, yp;
        float ratiox, ratioy;
		int px, py, xshift;
    	//get details of the source rectangle
        int32_t x=getint(argv[0],0,maxWR-1);
    	int32_t y=getint(argv[2],0,maxHR-1);
    	int32_t w1=getint(argv[4],1,maxWR-x);
    	int32_t h1=getint(argv[6],1,maxHR-y);
    	// get details of the trapezoid
    	int32_t nx=getint(argv[8],0,maxW-1);
    	int32_t ny=getint(argv[10],0,maxH-1);
    	int32_t nw=getint(argv[12],1,maxW-nx);
    	int32_t nx2=getint(argv[14],0,maxW-1);
    	int32_t ny2=getint(argv[16],ny+1,maxH-1);
    	int32_t nw2=getint(argv[18],1,maxW-nx2);
		float x_ratio = ((float)w1/(float)nw2) ;
		float y_ratio = ((float)h1/(float)(ny2-ny+1)) ;
    	// read in the rectangle
        if(argc==23)dontcopyblack=getint(argv[22],0,1);
		int multiplier=(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4));
		short *buff;
		char *bbuff;
        if(w1*h1*multiplier<LBUFFSIZE){
			buff=(short *)linebuff;
        } else {
        	buff=GetTempMemory(w1 * h1 * multiplier);
        }
		bbuff=(char*)buff;
		ReadBufferFast(x,y,x+w1-1,y+h1-1, (char *)bbuff);
		wh=ny2-ny;
		if(VideoColour==8){
			for (yy=0;yy<wh;yy++) {
				ratioy=(float)yy/(float)(wh-1);
				xs=nx-(int)(ratioy*(float)(nx-nx2));
				xe=xs + nw - (int)(ratioy*(float)(nw-nw2));
				ratiox=(float)(nw2)/(float)(xe-xs);
				py = (float)(yy)*ratiox*y_ratio;
				xshift=(int)(x_ratio*ratiox*(float)65536.0);
				routinechecks(1);
				for(xx=0;xx<xe-xs;xx++){
					px = (xx*xshift)>>16;
					c=bbuff[px+py*w1];
					if(dontcopyblack==0 || c){
						yp=yy+ny;
						xp=xx+xs;
						if(optiony)yy=maxH-1-yy;
						if(xp<maxW && yp<maxH && xp>=0 && yp>=0){
							if(Scale==1){
								*(uint8_t *)((yp * maxW + xp) + wpa)=(uint8_t)c;
							} else {
								*(uint8_t *)((yp * 2 * maxW + xp) + wpa)=(uint8_t)c;
								*(uint8_t *)(((yp * 2 + 1) * maxW + xp) + wpa)=(uint8_t)c;
							}
						}
					}
				}
			}
		} else if(VideoColour<=16) {
			for (yy=0;yy<wh;yy++) {
				ratioy=(float)yy/(float)(wh-1);
				xs=nx-(int)(ratioy*(float)(nx-nx2));
				xe=xs + nw - (int)(ratioy*(float)(nw-nw2));
				ratiox=(float)(nw2)/(float)(xe-xs);
				py = (float)(yy)*ratiox*y_ratio;
				xshift=(int)(x_ratio*ratiox*(float)65536.0);
				routinechecks(1);
				for(xx=0;xx<xe-xs;xx++){
					px = (xx*xshift)>>16;
					c=buff[px+py*w1];
					if(dontcopyblack==0 || c){
						yp=yy+ny;
						xp=xx+xs;
						if(optiony)yy=maxH-1-yy;
						if(xp<maxW && yp<maxH && xp>=0 && yp>=0){
							if(Scale==1){
								*(uint16_t *)((yp * maxW + xp) * 2 + wpa)=(uint16_t)c;
							} else {
								*(uint16_t *)((yp * 2 * maxW + xp) * 2 + wpa)=(uint16_t)c;
								*(uint16_t *)(((yp * 2 + 1) * maxW + xp) * 2 + wpa)=(uint16_t)c;
							}
						}
					}
				}
			}
		} else {
			for (yy=0;yy<wh;yy++) {
				ratioy=(float)yy/(float)(wh-1);
				xs=nx-(int)(ratioy*(float)(nx-nx2));
				xe=xs + nw - (int)(ratioy*(float)(nw-nw2));
				ratiox=(float)(nw2)/(float)(xe-xs);
				py = (float)(yy)*ratiox*y_ratio;
				xshift=(int)(x_ratio*ratiox*(float)65536.0);
				routinechecks(1);
				for(xx=0;xx<xe-xs;xx++){
					px = (xx*xshift)>>16;
					c=buff[px+py*w1];
					if(dontcopyblack==0 || c){
						yp=yy+ny;
						xp=xx+xs;
						if(optiony)yy=maxH-1-yy;
						if(xp<maxW && yp<maxH && xp>=0 && yp>=0){
							if(Scale==1){
								*(uint32_t *)((yp * maxW + xp) * 4 + wpa)=(uint32_t)c;
							} else {
								*(uint32_t *)((yp * 2 * maxW + xp) * 4 + wpa)=(uint32_t)c;
								*(uint32_t *)(((yp * 2 + 1) * maxW + xp) * 4 + wpa)=(uint32_t)c;
							}
						}
					}
				}
			}
		}
		// tidy up
        ReadPage=WritePage;
        if(cursorhidden)showcursor(0, xcursor,ycursor);
    	return;
    }
    if((p=checkstring(cmdline, "RESIZE_FAST"))) {
    	getargs(&p,19,",");
       	if(argc<15)error("Argument count");
        int cursorhidden=0;
        if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    		hidecursor(0);
    		cursorhidden=1;
        }
    	int dontcopyblack=0;
        if(argc>=17 && *argv[16]){
        	if(checkstring(argv[16], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[16],0,LastPage);
        	}
        }
        int maxWR=PageTable[ReadPage].xmax;
    	int maxHR=PageTable[ReadPage].ymax;
    	int maxH=PageTable[WritePage].ymax;
        int maxW=PageTable[WritePage].xmax;
    	uint32_t x=getint(argv[0],0,maxWR-1);
    	uint32_t y=getint(argv[2],0,maxHR-1);
    	uint32_t w1=getint(argv[4],1,maxWR-x);
    	uint32_t h1=getint(argv[6],1,maxHR-y);
    	uint32_t nx=getint(argv[8],0,maxW-1);
    	uint32_t ny=getint(argv[10],0,maxH-1);
    	uint32_t w2=getint(argv[12],1,maxW-nx);
    	uint32_t h2=getint(argv[14],1,maxH-ny);
        if(argc==19)dontcopyblack=(getint(argv[18],0,1)<<2);
		short *buff;
		char *bbuff;
		int c, multiplier=(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4));
		int px, py, yy;
		if(w1==w2 && h1==h2 && ReadPage!=WritePage){
	    	int copymode=PageTable[ReadPage].expand | (PageTable[WritePage].expand<<1);
	    	switch(copymode){
	    	case 0:
	    		MoveBufferNormal( x, y, nx, ny, w1, h1, dontcopyblack);
	    		break;
	    	case 1:
	        	MoveBufferContract( x, y, nx, ny, w1, h1, dontcopyblack);
	    		break;
	    	case 2:
	        	MoveBufferExpand( x, y, nx, ny, w1, h1, dontcopyblack);
	    		break;
	    	case 3:
	        	MoveBufferDup( x, y, nx, ny, w1, h1, dontcopyblack);
	    	}
		} else if(w1==w2 && ReadPage!=WritePage){
    		float y_ratio = ((float)h1/(float)h2) ;
	    	int copymode=PageTable[ReadPage].expand | (PageTable[WritePage].expand<<1);
	    	switch(copymode){
	    	case 0:
				for (yy=0;yy<h2;yy++) {
					py = (yy*y_ratio)+y;
					MoveBufferNormal( x, py, nx, ny+yy, w1, 1, dontcopyblack);
				}
	    		break;
	    	case 1:
				for (yy=0;yy<h2;yy++) {
					py = (yy*y_ratio)+y;
					MoveBufferContract( x, py, nx, ny+yy, w1, 1, dontcopyblack);
				}
	    		break;
	    	case 2:
				for (yy=0;yy<h2;yy++) {
					py = (yy*y_ratio)+y;
					MoveBufferExpand( x, py, nx, ny+yy, w1, 1, dontcopyblack);
				}
	    		break;
	    	case 3:
				for (yy=0;yy<h2;yy++) {
					py = (yy*y_ratio)+y;
					MoveBufferDup( x, py, nx, ny+yy, w1, 1, dontcopyblack);
				}
	    	}
        } else {
    		int x_ratio = (int)((float)w1/(float)w2*(float)65536.0) ;
    		int y_ratio = (int)((float)h1/(float)h2*(float)65536.0) ;
    		int Scale=(PageTable[WritePage].expand ? 2 : 1);
    		uint32_t wpa=(uint32_t)PageTable[WritePage].address;
			if(w1*h1*multiplier<LBUFFSIZE){
				buff=(short *)linebuff;
			} else {
				buff=GetTempMemory(w1 * h1 * multiplier);
			}
			bbuff=(char*)buff;
			ReadBufferFast(x,y,x+w1-1,y+h1-1, (char *)bbuff);
			if(VideoColour==8){
				if(Scale==1){
					uint8_t *start1=(uint8_t *)((ny * maxW + nx) + wpa);
					uint8_t *cout1;
					for (int i=0;i<h2;i++) {
						cout1=start1;
						py = (i*y_ratio)>>16 ;
						for (int j=0;j<w2;j++) {
							px = (j*x_ratio)>>16 ;
							c=bbuff[(int)(px+py*w1)];
							if(dontcopyblack==0 || c){
								*cout1=c;
							}
							cout1++;
						}
						start1+=maxW;
					}
				} else {
					uint8_t *start1=(uint8_t *)((ny * 2 * maxW + nx) + wpa);
					uint8_t *start2=(uint8_t *)(((ny * 2 + 1) * maxW + nx) + wpa);
					uint8_t *cout1,*cout2;
					for (int i=0;i<h2;i++) {
						py = (i*y_ratio)>>16 ;
						cout1=start1;
						cout2=start2;
						for (int j=0;j<w2;j++) {
							px = (j*x_ratio)>>16 ;
							c=bbuff[(int)(px+py*w1)];
							if(dontcopyblack==0 || c){
								*cout1=c;
								*cout2=c;
							}
							cout1++;
							cout2++;
						}
					start1+=(maxW<<1);
					start2+=(maxW<<1);
					}
				}
			} else if(VideoColour<=16) {
				if(Scale==1){
					uint16_t *start1=(uint16_t *)((ny * maxW + nx) * 2 + wpa);
					uint16_t *cout1;
					for (int i=0;i<h2;i++) {
						cout1=start1;
						py = (i*y_ratio)>>16 ;
						for (int j=0;j<w2;j++) {
							px = (j*x_ratio)>>16 ;
							c=buff[(int)(px+py*w1)];
							if(dontcopyblack==0 || c){
								*cout1=c;
							}
							cout1++;
						}
						start1+=maxW;
					}
				} else {
					uint16_t *start1=(uint16_t *)((ny * 2 * maxW + nx) * 2 + wpa);
					uint16_t *start2=(uint16_t *)(((ny * 2 + 1) * maxW + nx) * 2 + wpa);
					uint16_t *cout1,*cout2;
					for (int i=0;i<h2;i++) {
						py = (i*y_ratio)>>16 ;
						cout1=start1;
						cout2=start2;
						for (int j=0;j<w2;j++) {
							px = (j*x_ratio)>>16 ;
							c=buff[(int)(px+py*w1)];
							if(dontcopyblack==0 || c){
								*cout1=c;
								*cout2=c;
							}
							cout1++;
							cout2++;
						}
					start1+=(maxW<<1);
					start2+=(maxW<<1);
					}
				}
			} else{
				if(Scale==1){
					uint32_t *start1=(uint32_t *)((ny * maxW + nx) * 4 + wpa);
					uint32_t *cout1;
					for (int i=0;i<h2;i++) {
						cout1=start1;
						py = (i*y_ratio)>>16 ;
						for (int j=0;j<w2;j++) {
							px = (j*x_ratio)>>16 ;
							c=buff[(int)(px+py*w1)];
							if(dontcopyblack==0 || c){
								*cout1=c;
							}
							cout1++;
						}
						start1+=maxW;
					}
				} else {
					uint32_t *start1=(uint32_t *)((ny * 2 * maxW + nx) * 4 + wpa);
					uint32_t *start2=(uint32_t *)(((ny * 2 + 1) * maxW + nx) * 4 + wpa);
					uint32_t *cout1,*cout2;
					for (int i=0;i<h2;i++) {
						py = (i*y_ratio)>>16 ;
						cout1=start1;
						cout2=start2;
						for (int j=0;j<w2;j++) {
							px = (j*x_ratio)>>16 ;
							c=buff[(int)(px+py*w1)];
							if(dontcopyblack==0 || c){
								*cout1=c;
								*cout2=c;
							}
							cout1++;
							cout2++;
						}
					start1+=(maxW<<1);
					start2+=(maxW<<1);
					}
				}
			}
        }
        ReadPage=WritePage;
        if(cursorhidden)showcursor(0, xcursor,ycursor);
        return;
    }

    if((p=checkstring(cmdline, "RESIZE"))) {
    	int dontcopyblack=0;
    	static uint8_t ***temp2=NULL;
    	getargs(&p,19,",");
       	if(argc<15)error("Argument count");
        int cursorhidden=0;
        if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    		hidecursor(0);
    		cursorhidden=1;
        }
        if(argc>=17 && *argv[16]){
        	if(checkstring(argv[16], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[16],0,LastPage);
        	}
        }
        int maxWR=PageTable[ReadPage].xmax;
    	int maxHR=PageTable[ReadPage].ymax;
    	int maxH=PageTable[WritePage].ymax;
        int maxW=PageTable[WritePage].xmax;
    	uint32_t x=getint(argv[0],0,maxWR-1);
    	uint32_t y=getint(argv[2],0,maxHR-1);
    	uint32_t w=getint(argv[4],1,maxWR-x);
    	uint32_t h=getint(argv[6],1,maxHR-y);
    	uint32_t nx=getint(argv[8],0,maxW-1);
    	uint32_t ny=getint(argv[10],0,maxH-1);
    	uint32_t nw=getint(argv[12],1,maxW-nx);
    	uint32_t nh=getint(argv[14],1,maxH-ny);
        if(argc==19)dontcopyblack=getint(argv[18],0,1);
    	uint8_t*** output = alloc3df(3, h, w);
    	copytofloat(output, x, y, w, h);
    	temp2=resize(output, h, w, nh, nw);
    	dealloc3df (output, 3, h, w);
    	copyfromfloat(temp2, nx, ny, nw, nh, dontcopyblack);
    	dealloc3df (temp2, 3, nh, nw);
        ReadPage=WritePage;
        if(cursorhidden)showcursor(0, xcursor,ycursor);
        return;
    }
    error("Syntax");
}
void getcoord(char *p, int *x, int *y) {
	char *tp, *ttp;
	char b[STRINGSIZE];
	char savechar;
	tp = getclosebracket(p);
	savechar=*tp;
	*tp = 0;														// remove the closing brackets
	strcpy(b, p);													// copy the coordinates to the temp buffer
	*tp = savechar;														// put back the closing bracket
	ttp = b+1;
	// kludge (todo: fix this)
	{
		getargs(&ttp, 3, ",");										// this is a macro and must be the first executable stmt in a block
		if(argc != 3) error("Invalid Syntax");
		*x = getinteger(argv[0]);
		*y = getinteger(argv[2]);
	}
}

void fun_getscanline(void){
	iret=((int)(hltdc.Instance->CPSR & 0xFFFF)- (int)PageTable[0].startactive);
	if(iret<0)iret+=((int)PageTable[0].maxlines+1);
	iret/=(PageTable[0].expand ? 2 : 1);
	if(iret>PageTable[0].maxlines || iret<0)iret=0;
	targ=T_INT;
}

#define RoundUptoPage(a)     ((((uint64_t)a) + (uint64_t)(128*1024 - 1)) & (uint64_t)(~(128*1024 - 1)))// round up to the nearest whole integer

uint32_t GetPageAddress(int page){
	return (uint32_t)PageTable[page].address;
}
void fun_clut(void){
	int cl=getint(ep,0,255);
	targ=T_INT;
	iret=((cl & 0b11100000)<<16) | ((cl & 0b00011100)<<11) | ((cl & 0b00000011)<<6);
}
void cmd_clut(void){
	char *p;
    if((p=checkstring(cmdline, "SET"))) {
		pagesetdone=0;
		while(!pagesetdone)CheckAbort();
		HAL_LTDC_ConfigCLUT(&hltdc, CLUT, 256, 0);
    } else if((p=checkstring(cmdline, "RESET"))){
    	reset_CLUT();
    } else if((p=checkstring(cmdline, "MAXIMITE"))){
    	if(VideoColour!=8)error("Set 8-bit mode to use compatibility");
    	CLUT[48]=0;
    	CLUT[49]=0xFF;
    	CLUT[50]=0xFF00;
    	CLUT[51]=0xFFFF;
    	CLUT[52]=0xFF0000;
    	CLUT[53]=0xFF00FF;
    	CLUT[54]=0xFFFF00;
    	CLUT[55]=0xFFFFFF;
    	CLUT[0]=0x0;
    	CLUT[1]=0xFF;
    	CLUT[2]=0xFF00;
    	CLUT[3]=0xFFFF;
    	CLUT[4]=0xFF0000;
    	CLUT[5]=0xFF00FF;
    	CLUT[6]=0xFFFF00;
    	CLUT[7]=0xFFFFFF;
    	CLUT[32]=0;
    	pagesetdone=0;
    	while(!pagesetdone)CheckAbort();
    	HAL_LTDC_ConfigCLUT(&hltdc, CLUT, 256, 0);
    } else {
    	int cl = getint(cmdline,0,255);
		while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
		if(!*cmdline) error("Invalid syntax");
		++cmdline;
		if(!*cmdline) error("Invalid syntax");
		int col=getColour(cmdline,0);
		if(!(VideoColour==8))error("8-bit video mode only");
		CLUT[cl]=col;
    }
}
void cmd_page(void){
	char *p;
	int cursorhidden=0;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
    static int Display_Off=0;
    p = checkstring(cmdline, "DISPLAY");
    if(p) {
    	int LayerIDX=LTDC_LAYER_1;
    	uint32_t Address;
    	getargs(&p,3,",");
    	if(argc==3)LayerIDX=getint(argv[2],LTDC_LAYER_1,(VideoColour==12 ? LTDC_LAYER_2 : LTDC_LAYER_1));
		int32_t fadd=getint(argv[0],-1,LastPage);
		if(fadd==-1 && !CurrentLinePtr) error("Only valid in a program");
		if(fadd==-1 && !Display_Off){
			GPIO_InitTypeDef GPIO_InitStruct = {0};
			Display_Off=1;
		    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
		    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		    GPIO_InitStruct.Pull = GPIO_NOPULL;
		    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
			return;
		}
		if(fadd!=-1 && Display_Off){
			GPIO_InitTypeDef GPIO_InitStruct = {0};
			Display_Off=0;
		    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
		    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		    GPIO_InitStruct.Pull = GPIO_NOPULL;
		    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
		    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
		}
		if((VideoMode==3 || VideoMode==5 || VideoMode==6 || VideoMode==7 || VideoMode==12 || VideoMode==13) && fadd>(VideoColour==12 ? 2: 1))error(VideoColour==12 ? "Display page limited to 0, 1 and 2 in this mode": "Display page limited to 0 and 1 in this mode");
    	Address=(uint32_t)PageTable[fadd].address;
    	if(Address>=0x24000000 &&  Address < 0x24000000+512*1024-PageTable[0].size){
			pagesetdone=0;
			while(!pagesetdone){
				CheckAbort();
			}
        	HAL_LTDC_SetAddress(&hltdc,  Address, LayerIDX);
        	return;
    	}
    	if(Address>=0xD0000000 &&  Address < 0xD0000000+(G1Hardware ? 0x300000: 0x780000)-PageTable[0].size){
    		ShareNeeded=RoundUptoPage512(Address+PageTable[fadd].size);
    		if(ShareNeeded>0xD0300000)error("Display page address > &HD0300000 - use a lower page number");
    		MPU_Config_nCacheable(0);
			pagesetdone=0;
			while(!pagesetdone){
				CheckAbort();
			}
    		HAL_LTDC_SetAddress(&hltdc,  Address, LayerIDX);
        	return;
    	}
    	error("Address");
    	return;
    }
    p = checkstring(cmdline, "WRITE");
    if(p) {
    	if(checkstring(p, "FRAMEBUFFER")){
    		if(PageTable[WPN].address==NULL)error("Framebuffer not created");
    		WritePage=ReadPage=WPN;
    	} else {
			int32_t fadd=getint(p,0,LastPage);
			WritePage=ReadPage=fadd;
    	}
    	HRes=PageTable[WritePage].xmax;
    	VRes=PageTable[WritePage].xmax;
    	WritePageAddressExternal=ReadPageAddressExternal=(uint32_t)PageTable[WritePage].address;
    	return;
    }
    p = checkstring(cmdline, "RESIZE");
    if(p) {
    	getargs(&p,5,",");
    	int maxHZ=PageTable[0].ymax;
        int maxWZ=PageTable[0].xmax;
    	uint32_t page=getint(argv[0],(VideoColour==12 ? 2 : 1),LastPage);
    	int x=getint(argv[2],1,maxWZ);
    	int y=getint(argv[4],1,maxHZ);
    	PageTable[page].xmax=x;
    	PageTable[page].ymax=y;
    	return;
    }
    p = checkstring(cmdline, "MERGE");
    if(p) {
    	getargs(&p,7,",");
    	if(!(argc==7 || argc==5))error("Argument count");
    	uint32_t fadd1=getint(argv[0],(VideoColour==12 ? 2 : 1),LastPage);
    	uint32_t fadd2=getint(argv[2],(VideoColour==12 ? 2 : 1),LastPage);
    	uint32_t tadd=getint(argv[4],0,LastPage);
    	int colour=0;
    	if(argc==7)colour=getColour(argv[6], 0);
		int newfadd1=fadd1,newfadd2=fadd2,newtadd=tadd;
    	if(!(PageTable[fadd1].xmax==PageTable[tadd].xmax && PageTable[fadd2].xmax==PageTable[tadd].xmax &&
    			PageTable[fadd1].ymax==PageTable[tadd].ymax && PageTable[fadd2].ymax==PageTable[tadd].ymax))error("Page size mismatch");
		if(cursoron && (tadd==(VideoColour==12 ? 1 : 0) || fadd1==(VideoColour==12 ? 1 : 0) || fadd2==(VideoColour==12 ? 1 : 0))){
			uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
			ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
			hidecursor(0);
			cursorhidden=1;
			ReadPage=readsave;
			WritePage=writesave;
		}
		if(PageTable[tadd].expand){
			PageTable[TPN].address=GetTempMemory(PageTable[fadd1].size);
			PageTable[TPN].expand=0;
			PageTable[TPN].size=PageTable[fadd1].size;
			PageTable[TPN].xmax=PageTable[fadd1].xmax;
			PageTable[TPN].ymax=PageTable[fadd1].ymax;
			PageTable[TPN].nbytes=PageTable[fadd1].nbytes;
			newtadd=TPN;
		}
		if(PageTable[fadd1].expand){
			PageTable[TPN1].address=GetTempMemory(PageTable[fadd1].size);
			PageTable[TPN1].expand=0;
			PageTable[TPN1].size=PageTable[fadd1].size;
			PageTable[TPN1].xmax=PageTable[fadd1].xmax;
			PageTable[TPN1].ymax=PageTable[fadd1].ymax;
			PageTable[TPN1].nbytes=PageTable[fadd1].nbytes;
			newfadd1=TPN1;
			PageCopy(fadd1,TPN1,0);
		}
		if(PageTable[fadd2].expand){
			PageTable[TPN2].address=GetTempMemory(PageTable[fadd2].size);
			PageTable[TPN2].expand=0;
			PageTable[TPN2].size=PageTable[fadd2].size;
			PageTable[TPN2].xmax=PageTable[fadd2].xmax;
			PageTable[TPN2].ymax=PageTable[fadd2].ymax;
			PageTable[TPN2].nbytes=PageTable[fadd2].nbytes;
			newfadd2=TPN2;
			PageCopy(fadd2,TPN2,0);
		}
		Merge(newfadd1,newfadd2,newtadd,colour);
		if(PageTable[tadd].expand){
			PageCopy(TPN,tadd,0);
		}

		if(cursorhidden){
			uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
			ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
			showcursor(0, xcursor, ycursor);
			ReadPage=readsave;
			WritePage=writesave;
		}
       	return;
    }
    p = checkstring(cmdline, "SCROLL");
    if(p) {
    	int x, y, blank=-2,m,n;
        char *current=NULL;
    	uint32_t savepage=WritePage;
    	getargs(&p,7,",");
		int multiplier=(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4));
    	if(argc<5)error("Syntax");
    	x=getint(argv[2],-maxW/2-1,maxW/2);
    	y=getint(argv[4],-maxH/2-1,maxH/2);
    	if(x==0 && y==0)return;
    	m=((maxW*(y>0?y:-y))*multiplier);
    	n=((maxH*(x>0?x:-x))*multiplier);
    	if(n>m)m=n;
    	if(argc==7)blank=getColour(argv[6], 1);
		if(blank==-2)current=GetTempMemory(m);
		ReadPage=WritePage=getint(argv[0],0,LastPage);
    	if(cursoron && ReadPage==(VideoColour==12 ? 1 : 0)){
    		hidecursor(0);
    		cursorhidden=1;
    	}
        if(x>0){
            if(blank==-2)ReadBufferFast(maxW-x,0,maxW-1,maxH-1,current);
            ScrollBufferH(x);
            if(blank==-2)DrawBufferFast(0,0,x-1,maxH-1,current);
            else if(blank!=-1)DrawRectangle(0, 0, x-1, maxH-1,  blank) ;
        } else if(x<0){
            x=-x;
            if(blank==-2)ReadBufferFast(0,0,x-1,maxH-1,current);
            ScrollBufferH(-x);
            if(blank==-2)DrawBufferFast(maxW-x,0,maxW-1,maxH-1, current);
            else if(blank!=-1)DrawRectangle(maxW-x,0,maxW-1,maxH-1, blank);
		}
		if(y>0){
	        if(blank==-2)ReadBufferFast(0,0,maxW-1,y-1,current);
	        ScrollBufferV(y, 0);
	        if(blank==-2)DrawBufferFast(0,maxH-y,maxW-1,maxH-1,current);
	        else if(blank!=-1)DrawRectangle(0,maxH-y,maxW-1,maxH-1, blank) ;
	    } else if(y<0){
	        y=-y;
	        if(blank==-2)ReadBufferFast(0,maxH-y,maxW-1,maxH-1,current);
	        ScrollBufferV(-y, 0 );
	        if(blank==-2)DrawBufferFast(0,0,maxW-1,y-1,current);
	        else if(blank!=-1)DrawRectangle(0,0,maxW-1,y-1, blank) ;
	    }
		ReadPage=WritePage=savepage;
    	if(cursorhidden)showcursor(0, xcursor, ycursor);
    	return;
    }
    p = checkstring(cmdline, "COPY");
    if(p) {
    	int transparent=0;
    	char ss[3];														// this will be used to split up the argument line
    	ss[0] = tokenTO;
    	ss[1]=',';
    	ss[2] = 0;
    	getargs(&p,7,ss);
       	if(argc<3)error("Argument count");
    	int32_t fadd=getint(argv[0],0,LastPage);
    	int32_t tadd=getint(argv[2],0,LastPage);
    	if(!(PageTable[fadd].xmax==PageTable[tadd].xmax && PageTable[fadd].ymax==PageTable[tadd].ymax))error("Page size mismatch - use BLIT");
    	if(argc>=5 && *argv[4]){
    		char *p=argv[4];
    		if (toupper(*p)!='I'){
    			if(toupper(*p)=='B'){
    				pagesetdone=0;
    				while(!pagesetdone){
    					CheckAbort();
    				}
    			} else if (toupper(*p)=='D'){
    				deferredcopy=1;
    				pagesetdone=0;
    				deferredfadd=fadd;
    				deferredtadd=tadd;
    			} else error("Syntax");
    		}
    	}
    	if(argc==7){
    		char *p=argv[6];
    		if (toupper(*p)=='T' || *p=='1')transparent=1;
    		if(deferredcopy)deferredtransparent=1;
    	}
    	if(!deferredcopy){
        	if(cursoron && (tadd==(VideoColour==12 ? 1 : 0) || fadd==(VideoColour==12 ? 1 : 0))){
        	    uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
        	    ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
        		hidecursor(0);
        		cursorhidden=1;
        	    ReadPage=readsave;
        	    WritePage=writesave;
        	}
    		PageCopy(fadd,tadd,transparent);
			if(cursorhidden){
				uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
				ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
				showcursor(0, xcursor, ycursor);
				ReadPage=readsave;
				WritePage=writesave;
			}
    	}
    	return;
    }
    p = checkstring(cmdline, "STITCH");
    if(p) {
    	getargs(&p,7,",");
    	if(argc!=7)error("Argument count");
    	uint32_t fadd1=getint(argv[0],(VideoColour==12 ? 2 : 1),LastPage);
    	uint32_t fadd2=getint(argv[2],(VideoColour==12 ? 2 : 1),LastPage);
    	uint32_t tadd=getint(argv[4],0,LastPage);
		int newfadd1=fadd1,newfadd2=fadd2,newtadd=tadd;
    	if(!(PageTable[fadd1].xmax==PageTable[tadd].xmax && PageTable[fadd2].xmax==PageTable[tadd].xmax &&
    			PageTable[fadd1].ymax==PageTable[tadd].ymax && PageTable[fadd2].ymax==PageTable[tadd].ymax))error("Page size mismatch");
		if(cursoron && (tadd==(VideoColour==12 ? 1 : 0) || fadd1==(VideoColour==12 ? 1 : 0) || fadd2==(VideoColour==12 ? 1 : 0))){
			uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
			ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
			hidecursor(0);
			cursorhidden=1;
			ReadPage=readsave;
			WritePage=writesave;
		}
    	int offset=getint(argv[6],0,maxW);
		if(PageTable[tadd].expand){
			PageTable[TPN].address=GetTempMemory(PageTable[fadd1].size);
			PageTable[TPN].expand=0;
			PageTable[TPN].size=PageTable[fadd1].size;
			PageTable[TPN].xmax=PageTable[fadd1].xmax;
			PageTable[TPN].ymax=PageTable[fadd1].ymax;
			PageTable[TPN].nbytes=PageTable[fadd1].nbytes;
			newtadd=TPN;
		}
		if(PageTable[fadd1].expand){
			PageTable[TPN1].address=GetTempMemory(PageTable[fadd1].size);
			PageTable[TPN1].expand=0;
			PageTable[TPN1].size=PageTable[fadd1].size;
			PageTable[TPN1].xmax=PageTable[fadd1].xmax;
			PageTable[TPN1].ymax=PageTable[fadd1].ymax;
			PageTable[TPN1].nbytes=PageTable[fadd1].nbytes;
			newfadd1=TPN1;
			PageCopy(fadd1,TPN1,0);
		}
		if(PageTable[fadd2].expand){
			PageTable[TPN2].address=GetTempMemory(PageTable[fadd2].size);
			PageTable[TPN2].expand=0;
			PageTable[TPN2].size=PageTable[fadd2].size;
			PageTable[TPN2].xmax=PageTable[fadd2].xmax;
			PageTable[TPN2].ymax=PageTable[fadd2].ymax;
			PageTable[TPN2].nbytes=PageTable[fadd2].nbytes;
			newfadd2=TPN2;
			PageCopy(fadd2,TPN2,0);
		}
		Stitch(newfadd1,newfadd2,newtadd,offset);
		if(PageTable[tadd].expand){
			PageCopy(TPN,tadd,0);
		}

		if(cursorhidden){
			uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
			ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
			showcursor(0, xcursor, ycursor);
			ReadPage=readsave;
			WritePage=writesave;
		}
       	return;
    }
    p = checkstring(cmdline, "AND_PIXELS");
    if(p) {
			getargs(&p,5,",");
			if(argc!=5)error("Argument count");
			uint32_t fadd1=getint(argv[0],0,LastPage);
			uint32_t fadd2=getint(argv[2],0,LastPage);
			uint32_t tadd=getint(argv[4],0,LastPage);
			int newfadd1=fadd1,newfadd2=fadd2,newtadd=tadd;
	    	if(!(PageTable[fadd1].xmax==PageTable[tadd].xmax && PageTable[fadd2].xmax==PageTable[tadd].xmax &&
	    			PageTable[fadd1].ymax==PageTable[tadd].ymax && PageTable[fadd2].ymax==PageTable[tadd].ymax))error("Page size mismatch");
			if(cursoron && (tadd==(VideoColour==12 ? 1 : 0) || fadd1==(VideoColour==12 ? 1 : 0) || fadd2==(VideoColour==12 ? 1 : 0))){
				uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
				ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
				hidecursor(0);
				cursorhidden=1;
				ReadPage=readsave;
				WritePage=writesave;
			}
			if(PageTable[tadd].expand){
				PageTable[TPN].address=GetTempMemory(PageTable[fadd1].size);
				PageTable[TPN].expand=0;
				PageTable[TPN].size=PageTable[fadd1].size;
				PageTable[TPN].xmax=PageTable[fadd1].xmax;
				PageTable[TPN].ymax=PageTable[fadd1].ymax;
				PageTable[TPN].nbytes=PageTable[fadd1].nbytes;
				newtadd=TPN;
			}
			if(PageTable[fadd1].expand){
				PageTable[TPN1].address=GetTempMemory(PageTable[fadd1].size);
				PageTable[TPN1].expand=0;
				PageTable[TPN1].size=PageTable[fadd1].size;
				PageTable[TPN1].xmax=PageTable[fadd1].xmax;
				PageTable[TPN1].ymax=PageTable[fadd1].ymax;
				PageTable[TPN1].nbytes=PageTable[fadd1].nbytes;
				newfadd1=TPN1;
				PageCopy(fadd1,TPN1,0);
			}
			if(PageTable[fadd2].expand){
				PageTable[TPN2].address=GetTempMemory(PageTable[fadd2].size);
				PageTable[TPN2].expand=0;
				PageTable[TPN2].size=PageTable[fadd2].size;
				PageTable[TPN2].xmax=PageTable[fadd2].xmax;
				PageTable[TPN2].ymax=PageTable[fadd2].ymax;
				PageTable[TPN2].nbytes=PageTable[fadd2].nbytes;
				newfadd2=TPN2;
				PageCopy(fadd2,TPN2,0);
			}
			PageAnd(newfadd1,newfadd2,newtadd);
			if(PageTable[tadd].expand){
				PageCopy(TPN,tadd,0);
			}
			if(cursorhidden){
				uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
				ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
				showcursor(0, xcursor, ycursor);
				ReadPage=readsave;
				WritePage=writesave;
			}
      	 return;
         }
    p = checkstring(cmdline, "OR_PIXELS");
    if(p) {
			getargs(&p,5,",");
			if(argc!=5)error("Argument count");
			uint32_t fadd1=getint(argv[0],0,LastPage);
			uint32_t fadd2=getint(argv[2],0,LastPage);
			uint32_t tadd=getint(argv[4],0,LastPage);
			int newfadd1=fadd1,newfadd2=fadd2,newtadd=tadd;
	    	if(!(PageTable[fadd1].xmax==PageTable[tadd].xmax && PageTable[fadd2].xmax==PageTable[tadd].xmax &&
	    			PageTable[fadd1].ymax==PageTable[tadd].ymax && PageTable[fadd2].ymax==PageTable[tadd].ymax))error("Page size mismatch");
			if(cursoron && (tadd==(VideoColour==12 ? 1 : 0) || fadd1==(VideoColour==12 ? 1 : 0) || fadd2==(VideoColour==12 ? 1 : 0))){
				uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
				ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
				hidecursor(0);
				cursorhidden=1;
				ReadPage=readsave;
				WritePage=writesave;
			}
			if(PageTable[tadd].expand){
				PageTable[TPN].address=GetTempMemory(PageTable[fadd1].size);
				PageTable[TPN].expand=0;
				PageTable[TPN].size=PageTable[fadd1].size;
				PageTable[TPN].xmax=PageTable[fadd1].xmax;
				PageTable[TPN].ymax=PageTable[fadd1].ymax;
				PageTable[TPN].nbytes=PageTable[fadd1].nbytes;
				newtadd=TPN;
			}
			if(PageTable[fadd1].expand){
				PageTable[TPN1].address=GetTempMemory(PageTable[fadd1].size);
				PageTable[TPN1].expand=0;
				PageTable[TPN1].size=PageTable[fadd1].size;
				PageTable[TPN1].xmax=PageTable[fadd1].xmax;
				PageTable[TPN1].ymax=PageTable[fadd1].ymax;
				PageTable[TPN1].nbytes=PageTable[fadd1].nbytes;
				newfadd1=TPN1;
				PageCopy(fadd1,TPN1,0);
			}
			if(PageTable[fadd2].expand){
				PageTable[TPN2].address=GetTempMemory(PageTable[fadd2].size);
				PageTable[TPN2].expand=0;
				PageTable[TPN2].size=PageTable[fadd2].size;
				PageTable[TPN2].xmax=PageTable[fadd2].xmax;
				PageTable[TPN2].ymax=PageTable[fadd2].ymax;
				PageTable[TPN2].nbytes=PageTable[fadd2].nbytes;
				newfadd2=TPN2;
				PageCopy(fadd2,TPN2,0);
			}
			PageOr(newfadd1,newfadd2,newtadd);
			if(PageTable[tadd].expand){
				PageCopy(TPN,tadd,0);
			}
			if(cursorhidden){
				uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
				ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
				showcursor(0, xcursor, ycursor);
				ReadPage=readsave;
				WritePage=writesave;
			}
      	 return;
         }
    p = checkstring(cmdline, "XOR_PIXELS");
    if(p) {
			getargs(&p,5,",");
			if(argc!=5)error("Argument count");
			uint32_t fadd1=getint(argv[0],0,LastPage);
			uint32_t fadd2=getint(argv[2],0,LastPage);
			uint32_t tadd=getint(argv[4],0,LastPage);
			int newfadd1=fadd1,newfadd2=fadd2,newtadd=tadd;
	    	if(!(PageTable[fadd1].xmax==PageTable[tadd].xmax && PageTable[fadd2].xmax==PageTable[tadd].xmax &&
	    			PageTable[fadd1].ymax==PageTable[tadd].ymax && PageTable[fadd2].ymax==PageTable[tadd].ymax))error("Page size mismatch");
			if(cursoron && (tadd==(VideoColour==12 ? 1 : 0) || fadd1==(VideoColour==12 ? 1 : 0) || fadd2==(VideoColour==12 ? 1 : 0))){
				uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
				ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
				hidecursor(0);
				cursorhidden=1;
				ReadPage=readsave;
				WritePage=writesave;
			}
			if(PageTable[tadd].expand){
				PageTable[TPN].address=GetTempMemory(PageTable[fadd1].size);
				PageTable[TPN].expand=0;
				PageTable[TPN].size=PageTable[fadd1].size;
				PageTable[TPN].xmax=PageTable[fadd1].xmax;
				PageTable[TPN].ymax=PageTable[fadd1].ymax;
				PageTable[TPN].nbytes=PageTable[fadd1].nbytes;
				newtadd=TPN;
			}
			if(PageTable[fadd1].expand){
				PageTable[TPN1].address=GetTempMemory(PageTable[fadd1].size);
				PageTable[TPN1].expand=0;
				PageTable[TPN1].size=PageTable[fadd1].size;
				PageTable[TPN1].xmax=PageTable[fadd1].xmax;
				PageTable[TPN1].ymax=PageTable[fadd1].ymax;
				PageTable[TPN1].nbytes=PageTable[fadd1].nbytes;
				newfadd1=TPN1;
				PageCopy(fadd1,TPN1,0);
			}
			if(PageTable[fadd2].expand){
				PageTable[TPN2].address=GetTempMemory(PageTable[fadd2].size);
				PageTable[TPN2].expand=0;
				PageTable[TPN2].size=PageTable[fadd2].size;
				PageTable[TPN2].xmax=PageTable[fadd2].xmax;
				PageTable[TPN2].ymax=PageTable[fadd2].ymax;
				PageTable[TPN2].nbytes=PageTable[fadd2].nbytes;
				newfadd2=TPN2;
				PageCopy(fadd2,TPN2,0);
			}
			PageXor(newfadd1,newfadd2,newtadd);
			if(PageTable[tadd].expand){
				PageCopy(TPN,tadd,0);
			}
			if(cursorhidden){
				uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
				ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
				showcursor(0, xcursor, ycursor);
				ReadPage=readsave;
				WritePage=writesave;
			}
      	 return;
         }
    error("Syntax");

}



// get and decode the justify$ string used in TEXT and GUI CAPTION
// the values are returned via pointers
int GetJustification(char *p, int *jh, int *jv, int *jo) {
    switch(toupper(*p++)) {
        case 'L':   *jh = JUSTIFY_LEFT; break;
        case 'C':   *jh = JUSTIFY_CENTER; break;
        case 'R':   *jh = JUSTIFY_RIGHT; break;
        case  0 :   return true;
        default:    p--;
    }
    skipspace(p);
    switch(toupper(*p++)) {
        case 'T':   *jv = JUSTIFY_TOP; break;
        case 'M':   *jv = JUSTIFY_MIDDLE; break;
        case 'B':   *jv = JUSTIFY_BOTTOM; break;
        case  0 :   return true;
        default:    p--;
    }
    skipspace(p);
    switch(toupper(*p++)) {
        case 'N':   *jo = ORIENT_NORMAL; break;                     // normal
        case 'V':   *jo = ORIENT_VERT; break;                       // vertical text (top to bottom)
        case 'I':   *jo = ORIENT_INVERTED; break;                   // inverted
        case 'U':   *jo = ORIENT_CCW90DEG; break;                   // rotated CCW 90 degrees
        case 'D':   *jo = ORIENT_CW90DEG; break;                    // rotated CW 90 degrees
        case  0 :   return true;
        default:    return false;
    }
    return *p == 0;
}

void cmd_text(void) {
    int x, y, font, scale, fc, bc;
    char *s;
    int jh = 0, jv = 0, jo = 0;

    getargs(&cmdline, 17, ",");                                     // this is a macro and must be the first executable stmt
    if(!(argc & 1) || argc < 5) error("Argument count");
    x = getinteger(argv[0]);
    y = getinteger(argv[2]);
    s = getCstring(argv[4]);

    if(argc > 5 && *argv[6])
        if(!GetJustification(argv[6], &jh, &jv, &jo))
            if(!GetJustification(getCstring(argv[6]), &jh, &jv, &jo))
                error("Justification");;

    font = (gui_font >> 4) + 1; scale = (gui_font & 0b1111); fc = gui_fcolour; bc = gui_bcolour;        // the defaults
    if(argc > 7 && *argv[8]) {
        if(*argv[8] == '#') argv[8]++;
        font = getint(argv[8], 1, FONT_TABLE_SIZE);
    }
    if(FontTable[font - 1] == NULL) error("Invalid font #%", font);
	if(argc > 9 && *argv[10]) scale = getint(argv[10], 1, 15);
	if(argc > 11 && *argv[12]) fc = getColour(argv[12], 0);
	if(argc ==15) bc = getColour(argv[14], 1);
    GUIPrintString(x, y, ((font - 1) << 4) | scale, jh, jv, jo, fc, bc, s);
}
// Used by structures  STRUCTENABLED
//void  getargaddress (volatile char *p, long long int **ip, MMFLOAT **fp, volatile int *n){
void getargaddress(volatile char *p, long long int **ip, MMFLOAT **fp, int *n, int *stride){
    char *ptr=NULL;
    *fp=NULL;
    *ip=NULL;
    if (stride)
        *stride = sizeof(MMFLOAT); // Default stride for normal arrays (8 bytes)
    char pp[STRINGSIZE]={0};
    strcpy(pp,(char *)p);
    if(!isnamestart(pp[0])){ //found a literal
        *n=1;
        return;
    }
    ptr = findvar(pp, V_FIND | V_EMPTY_OK | V_NOFIND_NULL);
    if(ptr && vartbl[VarIndex].type & (T_NBR | T_INT)) {
        if(vartbl[VarIndex].dims[0] <= 0){ //simple variable
            *n=1;
            return;
        } else { // array or array element
            if(*n == 0)*n=vartbl[VarIndex].dims[0] + 1 - OptionBase;
            else *n = (vartbl[VarIndex].dims[0] + 1 - OptionBase)< *n ? (vartbl[VarIndex].dims[0] + 1 - OptionBase) : *n;
            skipspace(p);
            do {
                p++;
            } while(isnamechar(*p));
            if(*p == '!') p++;
            if(*p == '(') {
                p++;
                skipspace(p);
                if(*p != ')') { //array element
                    *n=1;
                    return;
                }
            }
        }
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].type & T_NBR)
          *fp = (MMFLOAT*)ptr;
        else
          *ip = (long long int *)ptr;
    }
#ifdef STRUCTENABLED
    // Check if this is a struct member access (g_StructMemberType set by findvar)
    else if (ptr && (vartbl[VarIndex].type & T_STRUCT) && g_StructMemberType != 0)
    {
        // Caller must handle stride for struct member arrays
        if (stride == NULL)
            StandardError(47);

        // Check if this is an array element access (e.g., boxes(i%).x) vs whole array (boxes().x)
        // We need to check if there's an index expression in the parentheses
        volatile char *pcheck = p;
        skipspace(pcheck);
        // Skip past variable name
        while (isnamechar(*pcheck))
            pcheck++;
        if (*pcheck == '!' || *pcheck == '%' || *pcheck == '$')
            pcheck++;
        skipspace(pcheck);
        if (*pcheck == '(')
        {
            pcheck++;
            skipspace(pcheck);
            if (*pcheck != ')')
            {
                // There's an index expression - this is a single element access
                *n = 1;
                return;
            }
        }

        // This is a struct array with member access like points().x
        int struct_type = (int)vartbl[VarIndex].size;
        int struct_size = g_structtbl[struct_type]->total_size;

        // Get member type from g_StructMemberType (set by findvar/ResolveStructMember)
        int member_type = g_StructMemberType;

        // Check member type is numeric
        if (!(member_type & (T_NBR | T_INT)))
        {
            *n = 1; // Not a numeric member, treat as single value
            return;
        }

        // Calculate number of elements from array dimensions
        if (*n == 0)
            *n = vartbl[VarIndex].dims[0] + 1 - OptionBase;
        else
            *n = (vartbl[VarIndex].dims[0] + 1 - OptionBase) < *n ? (vartbl[VarIndex].dims[0] + 1 - OptionBase) : *n;

        // Check for 2D arrays (not supported)
        if (vartbl[VarIndex].dims[1] != 0)
            StandardError(6);

        // Set stride to structure size
        *stride = struct_size;

        // Set the appropriate pointer based on member type
        if (member_type & T_NBR)
            *fp = (MMFLOAT *)ptr;
        else
            *ip = (long long int *)ptr;
    }
#endif
    else
    {
    	*n=1; //may be a function call
    }
}

/*  Original before structures
void  getargaddress (volatile char *p, long long int **ip, MMFLOAT **fp, volatile int *n){
    char *ptr=NULL;
    *fp=NULL;
    *ip=NULL;
    char pp[STRINGSIZE]={0};
    strcpy(pp,(char *)p);
    if(!isnamestart(pp[0])){ //found a literal
        *n=1;
        return;
    }
    ptr = findvar(pp, V_FIND | V_EMPTY_OK | V_NOFIND_NULL);
    if(ptr && vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[0] <= 0){ //simple variable
            *n=1;
            return;
        } else { // array or array element
            if(*n == 0)*n=vartbl[VarIndex].dims[0] + 1 - OptionBase;
            else *n = (vartbl[VarIndex].dims[0] + 1 - OptionBase)< *n ? (vartbl[VarIndex].dims[0] + 1 - OptionBase) : *n;
            skipspace(p);
            do {
                p++;
            } while(isnamechar(*p));
            if(*p == '!') p++;
            if(*p == '(') {
                p++;
                skipspace(p);
                if(*p != ')') { //array element
                    *n=1;
                    return;
                }
            }
        }
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        *fp = (MMFLOAT*)ptr;
    } else if(ptr && vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[0] <= 0){
            *n=1;
            return;
        } else {
            if(*n == 0)*n=vartbl[VarIndex].dims[0] + 1 - OptionBase;
            else *n = (vartbl[VarIndex].dims[0] + 1 - OptionBase)< *n ? (vartbl[VarIndex].dims[0] + 1 - OptionBase) : *n;
            skipspace(p);
            do {
                p++;
            } while(isnamechar(*p));
            if(*p == '%') p++;
            if(*p == '(') {
                p++;
                skipspace(p);
                if(*p != ')') { //array element
                    *n=1;
                   return;
                }
            }
        }
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        *ip = (long long int *)ptr;
    } else {
    	*n=1; //may be a function call
    }
}
*/

// This from Picomite uses less memory but is half the speed.
// Used bt cmd_fill   ????

// Define your screen bounds here
#define SCREEN_WIDTH HRes
#define SCREEN_HEIGHT VRes

// Stack structure for flood fill with dynamic block allocation
#define BLOCK_SIZE_ENTRIES 256

#define STACK_BLOCK_SIZE 256 // Entries per block

typedef struct
{
    int x;
    int y;
} Point;

typedef struct StackBlock
{
    Point data[STACK_BLOCK_SIZE];
    struct StackBlock *next;
    struct StackBlock *prev;
} StackBlock;

typedef struct
{
    StackBlock *first_block;
    StackBlock *current_block;
    int current_ptr;   // Index within current block (0 to STACK_BLOCK_SIZE-1)
    int total_entries; // Total number of entries across all blocks
} FloodStack;

// Stack operations with dynamic block allocation
static bool init_stack(FloodStack *s)
{
    s->first_block = (StackBlock *)GetMemory(sizeof(StackBlock));
    if (s->first_block == NULL)
    {
        return false;
    }
    s->first_block->next = NULL;
    s->first_block->prev = NULL;
    s->current_block = s->first_block;
    s->current_ptr = 0;
    s->total_entries = 0;
    return true;
}

static void free_stack(FloodStack *s)
{
    if (s->first_block == NULL)
    {
        return;
    }
    StackBlock *block = s->first_block;
    while (block != NULL)
    {
        StackBlock *next = block->next;
        void *ptr = (void *)block;
        FreeMemorySafe(&ptr);
        block = next;
    }
    s->first_block = NULL;
    s->current_block = NULL;
}

static bool push(FloodStack *s, int x, int y)
{
    // Check if current block is full
    if (s->current_ptr >= STACK_BLOCK_SIZE)
    {
        // Allocate a new block
        StackBlock *new_block = (StackBlock *)GetMemory(sizeof(StackBlock));
        if (new_block == NULL)
        {
            return false; // Out of memory
        }

        // Link the new block
        new_block->next = NULL;
        new_block->prev = s->current_block;
        s->current_block->next = new_block;

        // Move to the new block
        s->current_block = new_block;
        s->current_ptr = 0;
    }

    // Add entry to current block
    s->current_block->data[s->current_ptr].x = x;
    s->current_block->data[s->current_ptr].y = y;
    s->current_ptr++;
    s->total_entries++;
    return true;
}

static bool pop(FloodStack *s, int *x, int *y)
{
    // Check if stack is empty
    if (s->total_entries == 0)
    {
        return false;
    }

    // If current block is empty, move to previous block
    if (s->current_ptr == 0)
    {
        // Must have a previous block if total_entries > 0
        if (s->current_block->prev == NULL)
        {
            // This should never happen if total_entries is correct
            return false;
        }

        // Move to previous block
        s->current_block = s->current_block->prev;
        s->current_ptr = STACK_BLOCK_SIZE;
    }

    // Pop from current block
    s->current_ptr--;
    *x = s->current_block->data[s->current_ptr].x;
    *y = s->current_block->data[s->current_ptr].y;
    s->total_entries--;
    return true;
}

// Read a scanline segment efficiently
static void read_scanline(int x_start, int x_end, int y, unsigned char *buffer)
{
    ReadBuffer(x_start, y, x_end, y, (char *) buffer);
}

// Get color from buffer
// ReadBuffer returns pixels in B,G,R order (little endian RGB888)
static inline uint32_t get_color_from_buffer(unsigned char *buffer, int index)
{
	if(VideoColour==12){
		int idx = index * 4;
		uint32_t b = buffer[idx];
		uint32_t g = buffer[idx + 1];
	    uint32_t r = buffer[idx + 2];
	    uint32_t t = buffer[idx + 3];
	    return (t << 24 )|( r << 16) | (g << 8) | b;
	}else{
	   int idx = index * 3;
       uint32_t b = buffer[idx];
       uint32_t g = buffer[idx + 1];
       uint32_t r = buffer[idx + 2];
       return (r << 16) | (g << 8) | b;
   }
}

// Set color in buffer
// DrawBuffer expects pixels in B,G,R order (little endian RGB888)
static inline void set_color_in_buffer(unsigned char *buffer, int index, uint32_t color)
{
	if(VideoColour==32 ){

		int idx = index * 4;
        buffer[idx] = color & 0xFF;             // B
        buffer[idx + 1] = (color >> 8) & 0xFF;  // G
        buffer[idx + 2] = (color >> 16) & 0xFF; // R
        buffer[idx + 3] = (0xFF >> 24) & 0xFF;  // t always 0xFF

	}else if(VideoColour==12){

        int idx = index * 4;
        buffer[idx] = color & 0xFF;             // B
        buffer[idx + 1] = (color >> 8) & 0xFF;  // G
        buffer[idx + 2] = (color >> 16) & 0xFF; // R
        buffer[idx + 3] = (color >> 24) & 0xFF; // t

    }else{

    	int idx = index * 3;
        buffer[idx] = color & 0xFF;             // B
        buffer[idx + 1] = (color >> 8) & 0xFF;  // G
        buffer[idx + 2] = (color >> 16) & 0xFF; // R

    }
}
// Scanline flood fill algorithm with block reading
// Supports two modes:
// 1. Replace mode: if boundary_colour == -1, replace all pixels matching color at (x,y) with internal_colour
// 2. Boundary mode: if boundary_colour != -1, fill with internal_colour up to boundary_colour
void floodfill(int x, int y, int internal_colour, int boundary_colour)
{
	fillmaxH=PageTable[WritePage].ymax;
	fillmaxW=PageTable[WritePage].xmax;

	// Determine which mode we're in
    bool boundary_mode = (boundary_colour != -1);

    // internal_colour must always be valid (not -1)
    if (internal_colour == -1)
    {
        return; // Invalid: must specify fill color
    }
    uint32_t c_new = (uint32_t)internal_colour;
    //uint32_t c_new = (uint32_t)internal_colour & 0xFFFFFF;
   // PIntH(c_new);MMPrintString(" NEW");PRet();

    // Bounds check
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    {
        return;
    }

    // Initialize dynamic stack
    FloodStack stack;
    if (!init_stack(&stack))
    {
        return; // Memory allocation failed
    }
    char numbytes=3;
    if(VideoColour==12 || VideoColour==32)numbytes=4;

    // Allocate buffer for scanline reads (one full row)
    unsigned char *line_buffer = (unsigned char *)GetMemory(SCREEN_WIDTH * numbytes);
    if (line_buffer == NULL)
    {
        free_stack(&stack);
        return; // Memory allocation failed
    }

    // Allocate persistent buffers for checking adjacent lines
    unsigned char *above_buffer = (unsigned char *)GetMemory(SCREEN_WIDTH * numbytes);
    unsigned char *below_buffer = (unsigned char *)GetMemory(SCREEN_WIDTH * numbytes);
    if (above_buffer == NULL || below_buffer == NULL)
    {
        FreeMemorySafe((void **)&above_buffer);
        FreeMemorySafe((void **)&below_buffer);
        FreeMemorySafe((void **)&line_buffer);
        free_stack(&stack);
        return; // Memory allocation failed
    }

    // Allocate a "filled" bitmap to track which pixels we've already processed
    int bitmap_size = (SCREEN_WIDTH * SCREEN_HEIGHT + 7) / 8;
    unsigned char *filled_bitmap = (unsigned char *)GetMemory(bitmap_size);
    if (filled_bitmap == NULL)
    {
        FreeMemorySafe((void **)&below_buffer);
        FreeMemorySafe((void **)&above_buffer);
        FreeMemorySafe((void **)&line_buffer);
        free_stack(&stack);
        return; // Memory allocation failed
    }

    // Read the initial scanline to get origin color
    read_scanline(0, SCREEN_WIDTH - 1, y, line_buffer);
    uint32_t c_origin = get_color_from_buffer(line_buffer, x);

    // Mode-specific validation
    if (boundary_mode)
    {
        // In boundary mode, if starting pixel is the boundary color, nothing to do
        uint32_t c_boundary = (uint32_t)boundary_colour;
        if (c_origin == c_boundary)
        {
            FreeMemorySafe((void **)&filled_bitmap);
            FreeMemorySafe((void **)&below_buffer);
            FreeMemorySafe((void **)&above_buffer);
            FreeMemorySafe((void **)&line_buffer);
            free_stack(&stack);
            return;
        }
    }
    else
    {
        // In replace mode, if starting pixel is already the target color, nothing to do
        if (c_origin == c_new)
        {
           // MMPrintString("Already Same in Replace Mode \r\n");
        	FreeMemorySafe((void **)&filled_bitmap);
            FreeMemorySafe((void **)&below_buffer);
            FreeMemorySafe((void **)&above_buffer);
            FreeMemorySafe((void **)&line_buffer);
            free_stack(&stack);
            return;
        }
    }

    // Push initial point
    if (!push(&stack, x, y))
    {
        FreeMemorySafe((void **)&filled_bitmap);
        FreeMemorySafe((void **)&below_buffer);
        FreeMemorySafe((void **)&above_buffer);
        FreeMemorySafe((void **)&line_buffer);
        free_stack(&stack);
        return;
    }

    int current_y = -1; // Track which line is currently buffered

    while (pop(&stack, &x, &y))
    {
        // Read the scanline if we don't have it buffered
        if (y != current_y)
        {
            read_scanline(0, SCREEN_WIDTH - 1, y, line_buffer);
            current_y = y;
        }

        // Calculate bitmap position
        int bit_pos = y * SCREEN_WIDTH + x;
        int byte_idx = bit_pos / 8;
        int bit_idx = bit_pos % 8;

        // Check if we've already processed this pixel
        if (filled_bitmap[byte_idx] & (1 << bit_idx))
        {
            continue;
        }

        // Check if this pixel should be filled based on mode
        uint32_t pixel_color = get_color_from_buffer(line_buffer, x);
        bool should_fill;

        if (boundary_mode)
        {
            // Boundary mode: fill if pixel is NOT the boundary color
            uint32_t c_boundary = (uint32_t)boundary_colour;
            should_fill = (pixel_color != c_boundary);
        }
        else
        {
            // Replace mode: only fill if pixel matches origin color
            should_fill = (pixel_color == c_origin);
        }

        if (!should_fill)
        {
            continue;
        }

        // Find leftmost pixel in this row
        int x1 = x;
        while (x1 > 0)
        {
            uint32_t left_color = get_color_from_buffer(line_buffer, x1 - 1);
            bool can_extend;

            if (boundary_mode)
            {
                uint32_t c_boundary = (uint32_t)boundary_colour;
                can_extend = (left_color != c_boundary);
            }
            else
            {
                can_extend = (left_color == c_origin);
            }

            if (!can_extend)
                break;
            x1--;
        }

        // Find rightmost pixel in this row
        int x2 = x;
        while (x2 < SCREEN_WIDTH - 1)
        {
            uint32_t right_color = get_color_from_buffer(line_buffer, x2 + 1);
            bool can_extend;

            if (boundary_mode)
            {
                uint32_t c_boundary = (uint32_t)boundary_colour;
                can_extend = (right_color != c_boundary);
            }
            else
            {
                can_extend = (right_color == c_origin);
            }

            if (!can_extend)
                break;
            x2++;
        }

        // Fill the scanline
        int span_width = x2 - x1 + 1;
        unsigned char *draw_buffer = (unsigned char *)GetMemory(span_width * 3);
        if (draw_buffer != NULL)
        {
            // Prepare the buffer with new color in B,G,R order
            for (int i = 0; i < span_width; i++)
            {
                draw_buffer[i * 3] = c_new & 0xFF;
                draw_buffer[i * 3 + 1] = (c_new >> 8) & 0xFF;
                draw_buffer[i * 3 + 2] = (c_new >> 16) & 0xFF;
            }

            // Write the entire span at once
            DrawBuffer(x1, y, x2, y, (char*) draw_buffer,0);
            FreeMemorySafe((void **)&draw_buffer);

            // Mark all pixels in the span as filled in the bitmap
            for (int i = x1; i <= x2; i++)
            {
                int pos = y * SCREEN_WIDTH + i;
                int b_idx = pos / 8;
                int bit = pos % 8;
                filled_bitmap[b_idx] |= (1 << bit);

                // Also update line buffer
                set_color_in_buffer(line_buffer, i, c_new);
            }
        }

        // Check pixels above and below, adding spans to stack
        bool span_above = false;
        bool span_below = false;

        // Read line above if needed
        if (y > 0)
        {
            read_scanline(0, SCREEN_WIDTH - 1, y - 1, above_buffer);

            for (int i = x1; i <= x2; i++)
            {
                // Check bitmap first
                int pos = (y - 1) * SCREEN_WIDTH + i;
                int b_idx = pos / 8;
                int bit = pos % 8;

                if (!(filled_bitmap[b_idx] & (1 << bit)))
                {
                    uint32_t c = get_color_from_buffer(above_buffer, i);
                    bool can_fill;

                    if (boundary_mode)
                    {
                        uint32_t c_boundary = (uint32_t)boundary_colour;
                        can_fill = (c != c_boundary);
                    }
                    else
                    {
                        can_fill = (c == c_origin);
                    }

                    if (can_fill)
                    {
                        if (!span_above)
                        {
                            if (!push(&stack, i, y - 1))
                            {
                                FreeMemorySafe((void **)&filled_bitmap);
                                FreeMemorySafe((void **)&below_buffer);
                                FreeMemorySafe((void **)&above_buffer);
                                FreeMemorySafe((void **)&line_buffer);
                                free_stack(&stack);
                                return;
                            }
                            span_above = true;
                        }
                    }
                    else
                    {
                        span_above = false;
                    }
                }
                else
                {
                    span_above = false;
                }
            }
        }

        // Read line below if needed
        if (y < SCREEN_HEIGHT - 1)
        {
            read_scanline(0, SCREEN_WIDTH - 1, y + 1, below_buffer);

            for (int i = x1; i <= x2; i++)
            {
                // Check bitmap first
                int pos = (y + 1) * SCREEN_WIDTH + i;
                int b_idx = pos / 8;
                int bit = pos % 8;

                if (!(filled_bitmap[b_idx] & (1 << bit)))
                {
                    uint32_t c = get_color_from_buffer(below_buffer, i);
                    bool can_fill;

                    if (boundary_mode)
                    {
                        uint32_t c_boundary = (uint32_t)boundary_colour;
                        can_fill = (c != c_boundary);
                    }
                    else
                    {
                        can_fill = (c == c_origin);
                    }

                    if (can_fill)
                    {
                        if (!span_below)
                        {
                            if (!push(&stack, i, y + 1))
                            {
                                FreeMemorySafe((void **)&filled_bitmap);
                                FreeMemorySafe((void **)&below_buffer);
                                FreeMemorySafe((void **)&above_buffer);
                                FreeMemorySafe((void **)&line_buffer);
                                free_stack(&stack);
                                return;
                            }
                            span_below = true;
                        }
                    }
                    else
                    {
                        span_below = false;
                    }
                }
                else
                {
                    span_below = false;
                }
            }
        }
        routinechecks(1);
    }

    // Clean up heap memory
    FreeMemorySafe((void **)&filled_bitmap);
    FreeMemorySafe((void **)&below_buffer);
    FreeMemorySafe((void **)&above_buffer);
    FreeMemorySafe((void **)&line_buffer);
    free_stack(&stack);
}

void cmd_fill(void)
{
	fillmaxH=PageTable[WritePage].ymax;
    fillmaxW=PageTable[WritePage].xmax;
    //PIntComma(fillmaxW);PIntComma(SCREEN_WIDTH);PIntComma(fillmaxH);PIntComma(SCREEN_HEIGHT);PRet();
	uint32_t c = -1, b = -1;
	//int red, blue, green, trans;
    getcsargs(&cmdline, 7);
    if ((void *)ReadBuffer == (void *)DisplayNotSet)
        StandardError(11);
    //if (!(Option.DISPLAY_TYPE))
    //    error("No display");
    if (!(argc == 5 || argc == 7))SyntaxError();

	int x = getint(argv[0],0,fillmaxW);
	int y = getint(argv[2],0,fillmaxH);
    // Set colour to match format in ReadBuffer and Drawbuffer
	if(VideoColour==32){
	   c = getColour((char *)argv[4], 0);
       c=(c & 0xFFFFFF) ;//| 0xFF000000;
	} else if(VideoColour<=8){
		c = getColour((char *)argv[4], 0);
		c=c & 0xE0E0E0;   //RGB333   RGB332 ???
	} else {
		if(VideoColour==16){
			c = getColour((char *)argv[4], 0);
			c=c & 0xFFFFFF;  // RGB888 no transparency
		} else { //VideoColour12 ?
			c = getColour((char *)argv[4], 15);
			c=c & 0xFFFFFFF;  // RGB888 plus transparency
		}
	}

    if (argc == 7) {
    	//b = (uint32_t)getColour((char *)argv[6], 0);
    	//b = getColour((char *)argv[6], 0);
        // Set colour to match format in ReadBuffer and Drawbuffer
    	if(VideoColour==32){
    	   b = getColour((char *)argv[6], 0);
    	   b=(b & 0xFFFFFF);// | 0xFF000000;
    	} else if(VideoColour<=8){
    		//c = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
    		b = getColour((char *)argv[6], 0);
    		b=b & 0xE0E0E0;   //RGB333   RGB332 ???
    	} else {
    		if(VideoColour==16){
    			b = getColour((char *)argv[6], 0);
    			b=b & 0xFFFFFF;  // RGB888 no transparency
    		} else { //VideoColour12 ?
    			b = getColour((char *)argv[6],15);
    			b=(b & 0xFFFFFF) | 0xF000000;  // RGB888 plus transparency
    		}
    	}

    }
    // Call with replace mode (boundary_colour = -1)
    floodfill(x, y, c, b);
}

//End of Picomite version



void floodFillScanline(int x, int y)
{
  if(filloldcolour == ConvertedColour) return;
  if(ReadPixelFast(x,y) != filloldcolour) return;

  int x1, xe, xs;

  //draw current scanline from start position to the right
  x1 = x;
  xe = -1;
  while(x1 < fillmaxW && ReadPixelFast(x1, y) == filloldcolour)
  {
	xe=x1;
    x1++;
  }
  if(xe != -1){
	  DrawHLineFast(x,y,xe,ConvertedColour);
  }
  //draw current scanline from start position to the left
  x1 = x - 1;
  xs = -1;
  while(x1 >= 0 && ReadPixelFast(x1, y) == filloldcolour)
  {
	xs = x1;
    x1--;
  }
  if(xs!=-1){
	  DrawHLineFast(xs,y,x-1,ConvertedColour);
  }

  //test for new scanlines above
  x1 = x;
  if(xe!=-1){
	  while(x1 <= xe)
	  {
		if(y > 0 && ReadPixelFast(x1, (y - 1)) == filloldcolour)
		{
		  floodFillScanline(x1, y - 1);
		}
		x1++;
	  }
  }
  x1 = x - 1;
  if(xs!=-1){
	  while(x1 >= xs )
	  {
		if(y > 0 && ReadPixelFast(x1, (y - 1)) == filloldcolour)
		{
		  floodFillScanline( x1, y - 1);
		}
		x1--;
	  }
  }

  //test for new scanlines below
  x1 = x;
  if(xe!=-1){
	  while(x1 <= xe)
	  {
		if(y < fillmaxH - 1 && ReadPixelFast(x1, (y + 1)) == filloldcolour)
		{
		  floodFillScanline( x1, y + 1);
		}
		x1++;
	  }
  }
  x1 = x - 1;
  if(xs!=-1){
	  while(x1 >= xs)
	  {
		if(y < fillmaxH - 1 && ReadPixelFast(x1, (y + 1)) == filloldcolour)
		{
		  floodFillScanline(x1, y + 1);
		}
		x1--;
	  }
  }
  routinechecks(1);
}

void cmd_pixel(void) {
	if((nostackp=checkstring(cmdline, "FILL"))){
		fillmaxH=PageTable[WritePage].ymax;
	    fillmaxW=PageTable[WritePage].xmax;
 		uint32_t oldstack = __get_MSP();
 		//uint32_t c=-1,b=-1;
		__set_MSP((uint32_t)MinHeap());
		{
			int x1, y1, f, red, blue, green, trans;
			getargs(&nostackp,5,",");
			x1 = getint(argv[0],0,fillmaxW);
			y1 = getint(argv[2],0,fillmaxH);
			f = getColour(argv[4], 0);
			if(VideoColour==32){
				ConvertedColour=f;
			} else if(VideoColour<=8){
				ConvertedColour = ((f & 0b111000000000000000000000)>>16) | ((f & 0b1110000000000000)>>11) | ((f & 0b11000000)>>6);
			} else {
				if(VideoColour==16){
					red=BIT_5[((f & 0xFF0000)>>19)]<<11;
					green=BIT_6[((f & 0xFF00)>>10)]<<5;
					blue=BIT_5[((f & 0xFF)>>3)];
					ConvertedColour=red|green|blue;
				} else {
					red=BIT_4[((f & 0xFF0000)>>20)]<<8;
					green=BIT_4[((f & 0xFF00)>>12)]<<4;
					blue=BIT_4[((f & 0xFF)>>4)];
					trans=((f & 0xF000000)>>12);
					ConvertedColour=red|green|blue|trans;
				}
			}
			filloldcolour=ReadPixelFast(x1, y1);
			floodFillScanline(x1, y1);
			//floodfill(x, y, c, b);
		}
	__set_MSP(oldstack);
	return;
	}
	if(CMM1){
		int x, y, value;
		getcoord(cmdline, &x, &y);
		cmdline = getclosebracket(cmdline) + 1;
		while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
		if(!*cmdline) error("Invalid syntax");
		++cmdline;
		if(!*cmdline) error("Invalid syntax");
		value = getColour(cmdline,0);
		DrawPixel(x, y, value);
		lastx = x; lasty = y;
	} else {
	    int x1, y1, c=0, n=0 ,i, nc=0;
	    int x1stride = sizeof(MMFLOAT), y1stride = sizeof(MMFLOAT), cstride = sizeof(MMFLOAT);  //STRUCTURE
	    long long int *x1ptr, *y1ptr, *cptr;
	    MMFLOAT *x1fptr, *y1fptr, *cfptr;
	    getargs(&cmdline, 5,",");
	    if(!(argc == 3 || argc == 5)) error("Argument count");
	    //getargaddress(argv[0], &x1ptr, &x1fptr, &n,NULL);
	    getargaddress(argv[0], &x1ptr, &x1fptr, &n, &x1stride);
	    if(n != 1){
	    	getargaddress(argv[2], &y1ptr, &y1fptr, &n, &y1stride);
	    	//getargaddress(argv[2], &y1ptr, &y1fptr, &n,NULL);
	    }
	    if(n==1){ //just a single point
	        c = gui_fcolour;                                    // setup the defaults
	        x1 = getinteger(argv[0]);
	        y1 = getinteger(argv[2]);
	        if(argc == 5) c = getColour(argv[4], 0);
	        DrawPixel(x1, y1, c);
	    } else {
	        c = gui_fcolour;                                        // setup the defaults
	        if(argc == 5){
	            //getargaddress(argv[4], &cptr, &cfptr, &nc,NULL);
	            getargaddress(argv[4], &cptr, &cfptr, &nc, &cstride);
	            if(nc == 1) c = getColour(argv[4], 0);
	            else if(nc>1) {
	                if(nc < n) n=nc; //adjust the dimensionality
	                for(i=0;i<nc;i++){
	                	if(CMM1){
	                		c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
	                		if(c < 0 || c > 7) error("% is invalid (valid is % to %)", (int)c, 0, 7);
	                	} else {
	                		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
	                		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
	                		if(c < 0 || c > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)c, 0, 0xFFFFFFF);
	                	}
	                }
	            }
	        }
	        for(i=0;i<n;i++){
	            //x1 = (x1fptr == NULL ? x1ptr[i] : (int)x1fptr[i]);
	            //y1 = (y1fptr == NULL ? y1ptr[i] : (int)y1fptr[i]);
                x1 = (x1fptr == NULL ? STRIDE_INT(x1ptr, i, x1stride) : (int)STRIDE_FLOAT(x1fptr, i, x1stride));
                y1 = (y1fptr == NULL ? STRIDE_INT(y1ptr, i, y1stride) : (int)STRIDE_FLOAT(y1fptr, i, y1stride));
	            if(nc > 1) {
	            	if(CMM1){
	            		c = (cfptr == NULL ? colourmap[cptr[i]] : colourmap[(int)cfptr[i]]);
	            	} else {
	            		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
	            		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
	            	}
	            }
	            DrawPixel(x1, y1, c);
	        }
	    }
	}
}

// Fast arc drawing using direct scanline rendering
// Draws arc segments directly without intermediate bitmap buffer

static inline int normalize_angle(int angle)
{
    angle %= 360;
    return angle < 0 ? angle + 360 : angle;
}

static inline int point_in_arc_sector(int px, int py, int cx, int cy, int start_deg, int end_deg)
{
    // Calculate angle of point relative to center
    int dx = px - cx;
    int dy = py - cy;

    if (dx == 0 && dy == 0)
        return 1;

    // Original uses: 0° = up, 90° = right, 180° = down, 270° = left (clockwise from top)
    // Standard atan2 gives: 0° = right, 90° = up, 180° = left, 270° = down (counter-clockwise from right)
    // Convert: angle = 90 - atan2_angle, which is equivalent to atan2(dx, -dy)
    float angle = atan2f(dx, -dy) * 57.29577951f; // 180/PI
    if (angle < 0)
        angle += 360.0f;

    int angle_deg = (int)angle;

    // Handle wrap-around
    if (end_deg < start_deg)
        end_deg += 360;
    if (angle_deg < start_deg)
        angle_deg += 360;

    return (angle_deg >= start_deg && angle_deg <= end_deg);
}

void cmd_arc(void)
{
    int x, y, r1, r2, c;
    int rad1, rad2;

    getcsargs(&cmdline, 13);
    if (!(argc == 11 || argc == 13))error("Argument count");
 //       StandardError(2);
 //   CheckDisplay();

    x = getinteger(argv[0]);
    y = getinteger(argv[2]);
    r1 = getinteger(argv[4]);

    if (*argv[6])
        r2 = getinteger(argv[6]);
    else
    {
        r2 = r1;
        r1--;
    }

    if (r2 < r1)
        error("Inner radius < outer");

    rad1 = getnumber(argv[8]);
    rad2 = getnumber(argv[10]);
    //CMM2 addition
   // if(optiony){
   // 	rad1+=180.0;
   // 	rad2+=180.0;
   // }

    // Normalize angles to 0-359
    rad1 = normalize_angle(rad1);
    rad2 = normalize_angle(rad2);

    if (rad1 == rad2)
        error("Radials");

    if (argc == 13)
        //c = getint(argv[12], 0, WHITE);
        c = getColour(argv[12], 0);
    else
        c = gui_fcolour;

    // Ensure rad2 > rad1 for sweep direction
    if (rad2 < rad1)
        rad2 += 360;

   // int save_refresh = Option.Refresh;
   // Option.Refresh = 0;
    // Draw arc using scanline method
    // Iterate through bounding box
    int min_y = y - r2;
    int max_y = y + r2;
    for (int scan_y = min_y; scan_y <= max_y; scan_y++)
    {
        int dy = scan_y - y;
        int dy2 = dy * dy;

        // Calculate x range for outer circle at this y
        int dx_outer = (int)sqrtf(r2 * r2 - dy2);
        int dx_inner = (r1 * r1 > dy2) ? (int)sqrtf(r1 * r1 - dy2) : 0;

        // Check left and right segments
        for (int side = 0; side < 2; side++)
        {
            int x_start, x_end;

            if (side == 0)
            {
                // Left side: from -dx_outer to -dx_inner
                x_start = x - dx_outer;
                x_end = x - dx_inner;
            }
            else
            {
                // Right side: from dx_inner to dx_outer
                x_start = x + dx_inner;
                x_end = x + dx_outer;
            }

            // Find the actual segment within the arc
            int segment_start = -1;
            int segment_end = -1;

            for (int scan_x = x_start; scan_x <= x_end; scan_x++)
            {
                if (point_in_arc_sector(scan_x, scan_y, x, y, rad1, rad2))
                {
                    if (segment_start == -1)
                        segment_start = scan_x;
                    segment_end = scan_x;
                }
                else if (segment_start != -1)
                {
                    // Draw completed segment
                    DrawRectangle(segment_start, scan_y, segment_end, scan_y, c);
                    segment_start = -1;
                }
                routinechecks(1);
            }

            // Draw any remaining segment
            if (segment_start != -1)
            {
                DrawRectangle(segment_start, scan_y, segment_end, scan_y, c);
            }
            routinechecks(1);
        }
        routinechecks(1);
    }
}

/*
void pointcalc(int angle, int x, int y, int r2, int *x0, int * y0){
	float c1,s1;
	int quad;
	angle %=360;
	switch(angle){
	case 0:
		*x0=x;
		*y0=y-r2;
		break;
	case 45:
		*x0=x+r2+1;
		*y0=y-r2;
		break;
	case 90:
		*x0=x+r2+1;
		*y0=y;
		break;
	case 135:
		*x0=x+r2+1;
		*y0=y+r2;
		break;
	case 180:
		*x0=x;
		*y0=y+r2;
		break;
	case 225:
		*x0=x-r2;
		*y0=y+r2;
		break;
	case 270:
		*x0=x-r2;
		*y0=y;
		break;
	case 315:
		*x0=x-r2;
		*y0=y-r2;
		break;
	default:
		c1=cos(Rad(angle));
		s1=sin(Rad(angle));
		quad = (angle / 45) % 8;
		switch(quad){
		case 0:
			*y0=y-r2;
			*x0=x+s1*r2/c1;
			break;
		case 1:
 		  *x0=x+r2+1;
 		  *y0=y-c1*r2/s1;
 		  break;
		case 2:
 		  *x0=x+r2+1;
 		  *y0=y-c1*r2/s1;
 		  break;
		case 3:
 		  *y0=y+r2;
 		  *x0=x-s1*r2/c1;
 		  break;
		case 4:
 		  *y0=y+r2;
 		  *x0=x-s1*r2/c1;
 		  break;
		case 5:
 		  *x0=x-r2;
 		  *y0=y+c1*r2/s1;
 		  break;
		case 6:
			*x0=x-r2;
			*y0=y+c1*r2/s1;
			break;
		case 7:
			*y0=y-r2;
			*x0=x+s1*r2/c1;
			break;
		}
	}
}
void cmd_arc(void){
	// Parameters are:
	// X coordinate of centre of arc
	// Y coordinate of centre of arc
	// inner radius of arc
	// outer radius of arc - omit it 1 pixel wide
	// start radial of arc in degrees
	// end radial of arc in degrees
	// Colour of arc
	int x, y, r1, r2, c ,i ,j, k, xs=-1, xi=0, m;
	int rad1, rad2, rad3, rstart, quadr;
	int x0, y0, x1, y1, x2, y2, xr, yr;
	getargs(&cmdline, 13,",");
    if(!(argc == 11 || argc == 13)) error("Argument count");
    x = getinteger(argv[0]);
    y = getinteger(argv[2]);
    r1 = getinteger(argv[4]);
    if(*argv[6])r2 = getinteger(argv[6]);
    else {
    	r2=r1;
    	r1--;
    }
    if(r2 < r1)error("Inner radius < outer");
    rad1 = getnumber(argv[8]);
    rad2 = getnumber(argv[10]);
    if(optiony){
    	rad1+=180.0;
    	rad2+=180.0;
    }
    while(rad1<0.0)rad1+=360.0;
    while(rad2<0.0)rad2+=360.0;
	if(rad1==rad2)error("Radials");
     if(argc == 13)
        c = getColour(argv[12], 0);
    else
        c = gui_fcolour;
    while(rad2<rad1)rad2+=360;
    rad3=rad1+360;
    rstart=rad2;
    int quad1 = (rad1 / 45) % 8;
    x2=x;y2=y;
    int ints_per_line=RoundUptoInt((r2*2)+1)/32;
    uint32_t *br=(uint32_t *)GetTempMemory(((ints_per_line+1)*((r2*2)+1))*4);
    DrawFilledCircle(x, y, r2, r2, 1, ints_per_line, br, 1.0, 1.0);
    DrawFilledCircle(x, y, r1, r2, 0, ints_per_line, br, 1.0, 1.0);
    while(rstart<rad3){
		pointcalc(rstart, x, y, r2, &x0, &y0);
   		quadr = (rstart / 45) % 8;
    	if(quadr==quad1 && rad3-rstart<45){
    		pointcalc(rad3, x, y, r2, &x1, &y1);
    		ClearTriangle(x0-x+r2, y0-y+r2, x1-x+r2, y1-y+r2, x2-x+r2, y2-y+r2, ints_per_line, br);
    		rstart=rad3;
    	} else {
    		rstart +=45;
    		rstart -= (rstart % 45);
    		pointcalc(rstart, x, y, r2, &xr, &yr);
    		ClearTriangle(x0-x+r2, y0-y+r2, xr-x+r2, yr-y+r2, x2-x+r2, y2-y+r2, ints_per_line, br);
    	}
    	routinechecks(1);
    }
 	for(j=0;j<r2*2+1;j++){
 		for(i=0;i<ints_per_line;i++){
 			k=br[i+j*ints_per_line];
 			for(m=0;m<32;m++){
 				if(xs==-1 && (k & 0x80000000)){
 					xs=m;
 					xi=i;
 				}
 				if(xs!=-1 && !(k & 0x80000000)){
					DrawRectangle(x-r2+xs+xi*32, y-r2+j, x-r2+m+i*32, y-r2+j, c);
 					xs=-1;
 				}
 				k<<=1;
 			}
 		}
		if(xs!=-1){
			DrawRectangle(x-r2+xs+xi*32, y-r2+j, x-r2+m+i*32, y-r2+j, c);
			xs=-1;
		}
    	routinechecks(1);
	}
// 	FreeMemorySafe((void *)&br);
}
*/

//BEZIER from Picomite 6.01.00
// Fixed-point configuration: 10.22 format (10 bits integer, 22 bits fraction)
#define FP_SHIFT 24
#define FP_ONE (1 << FP_SHIFT)
#define FP_HALF (1 << (FP_SHIFT - 1))

// Convert integer to fixed-point
#define INT_TO_FP(x) ((int32_t)(x) << FP_SHIFT)

// Convert fixed-point to integer (with rounding)
#define FP_TO_INT(x) (((x) + FP_HALF) >> FP_SHIFT)

// Fixed-point multiplication
#define FP_MUL(a, b) (((int64_t)(a) * (int64_t)(b)) >> FP_SHIFT)

// Binomial coefficient calculation (unchanged)
static int binomial(int n, int k)
{
    if (k > n)
        return 0;
    if (k == 0 || k == n)
        return 1;
    int result = 1;
    for (int i = 0; i < k; i++)
    {
        result *= (n - i);
        result /= (i + 1);
    }
    return result;
}

// Fast integer square root for bounding box diagonal
static int isqrt(int n)
{
    if (n < 2)
        return n;

    int x = n;
    int y = (x + 1) / 2;

    while (y < x)
    {
        x = y;
        y = (x + n / x) / 2;
    }

    return x;
}

// Function to plot n-point Bezier curve using fixed-point arithmetic
void PlotBezier(int n, int c, int64_t *x, int64_t *y)
{
    int32_t t_fp, one_minus_t_fp;
    int64_t x_val, y_val;
    int prev_x, prev_y;

    // Calculate bounding box to estimate curve length
    int min_x = x[0], max_x = x[0];
    int min_y = y[0], max_y = y[0];
    for (int i = 1; i < n; i++)
    {
        if (x[i] < min_x)
            min_x = x[i];
        if (x[i] > max_x)
            max_x = x[i];
        if (y[i] < min_y)
            min_y = y[i];
        if (y[i] > max_y)
            max_y = y[i];
    }

    // Estimate steps based on bounding box diagonal
    int bbox_width = max_x - min_x;
    int bbox_height = max_y - min_y;
    int bbox_diag = isqrt(bbox_width * bbox_width + bbox_height * bbox_height);
    int num_steps = bbox_diag * 3;

    // Clamp to reasonable range
    if (num_steps < 10)
        num_steps = 10;
    if (num_steps > 2000)
        num_steps = 2000;

    // Pre-calculate binomial coefficients
    int binom_coeffs[16]; // Assuming max 16 control points
    for (int i = 0; i < n; i++)
    {
        binom_coeffs[i] = binomial(n - 1, i);
    }

    // Calculate and draw first point
    prev_x = x[0];
    prev_y = y[0];
    DrawPixel(prev_x, prev_y, c);

    int line_start_x = prev_x;
    int line_start_y = prev_y;
    int line_dx = 0;
    int line_dy = 0;
    int line_len = 0;

    // Pre-allocate arrays for power calculations (incremental approach)
    int32_t t_powers[16];   // t^i in fixed-point
    int32_t omt_powers[16]; // (1-t)^i in fixed-point

    // Plot the Bezier curve using line segments
    for (int step = 1; step <= num_steps; step++)
    {
        // Calculate t in fixed-point: t = step / num_steps
        t_fp = ((int64_t)step << FP_SHIFT) / num_steps;
        one_minus_t_fp = FP_ONE - t_fp;

        // Calculate powers incrementally using previous values
        // t^0 = 1, t^1 = t, t^2 = t * t^1, etc.
        t_powers[0] = FP_ONE;
        omt_powers[0] = FP_ONE;

        for (int i = 1; i < n; i++)
        {
            t_powers[i] = FP_MUL(t_powers[i - 1], t_fp);
            omt_powers[i] = FP_MUL(omt_powers[i - 1], one_minus_t_fp);
        }

        x_val = 0;
        y_val = 0;

        // Calculate point using Bezier formula with fixed-point
        for (int i = 0; i < n; i++)
        {
            // basis = binomial(n-1,i) * (1-t)^(n-1-i) * t^i
            // Note: binomial coefficient is integer, powers are fixed-point
            int32_t basis_fp = FP_MUL(omt_powers[n - 1 - i], t_powers[i]);

            // Multiply by binomial coefficient (integer)
            basis_fp = basis_fp * binom_coeffs[i];

            // Accumulate: basis * coordinate
            x_val += (int64_t)basis_fp * x[i];
            y_val += (int64_t)basis_fp * y[i];
        }

        // Convert back to integer coordinates
        // Need to shift by FP_SHIFT since basis_fp is already in fixed-point
        int curr_x = (int)((x_val + FP_HALF) >> FP_SHIFT);
        int curr_y = (int)((y_val + FP_HALF) >> FP_SHIFT);

        // Only process if coordinate changed
        // Only process if coordinate changed
        if (curr_x != prev_x || curr_y != prev_y)
        {
            int dx = curr_x - prev_x;
            int dy = curr_y - prev_y;
            // If jump > 1 pixel, bridge it immediately
            if (dx > 1 || dx < -1 || dy > 1 || dy < -1)
            {
                if (line_len > 1)
                {
                    DrawLine(line_start_x, line_start_y, prev_x, prev_y, 1, c);
                }
                else if (line_len == 1)
                {
                    DrawPixel(prev_x, prev_y, c);
                }
                DrawLine(prev_x, prev_y, curr_x, curr_y, 1, c);
                line_len = 0; // reset run
            }
            else
            {

                // Check if this continues the current straight line
                if (line_len > 0 && dx == line_dx && dy == line_dy)
                {
                    line_len++;
                }
                else
                {
                    // Direction changed - draw accumulated line if any
                    if (line_len > 1)
                    {
                        DrawLine(line_start_x, line_start_y, prev_x, prev_y, 1, c);
                    }
                    else if (line_len == 1)
                    {
                        DrawPixel(prev_x, prev_y, c);
                    }

                    // Start new line segment
                    line_start_x = prev_x;
                    line_start_y = prev_y;
                    line_dx = dx;
                    line_dy = dy;
                    line_len = 1;
                }
            }
            prev_x = curr_x;
            prev_y = curr_y;
        }
    }

    // Draw final accumulated line segment
    if (line_len > 1)
    {
        DrawLine(line_start_x, line_start_y, prev_x, prev_y, 1, c);
    }
    else if (line_len == 1)
    {
        DrawPixel(prev_x, prev_y, c);
    }
}

void cmd_bezier(void)
{
    int64_t *x = NULL;
    int64_t *y = NULL;
    int countx, county;
    getcsargs(&cmdline, 7);
    if (argc < 3)
        SyntaxError();
    countx = parseintegerarray(argv[0], &x, 1, 1, NULL, false,NULL);
    county = parseintegerarray(argv[2], &y, 2, 1, NULL, false,NULL);
    if (countx != county)
        StandardError(16);
    int n = countx;
    if (argc >= 5 && *argv[4])
        n = getint(argv[4], 2, countx);
    int colour = WHITE;
    if (argc == 7)
        colour = getColour((char *)argv[6], 0);
    PlotBezier(n, colour, x, y);
}


//OLD BEIZER
/*
void DrawBuffered(int xti, int yti, int c, int complete){
	static unsigned char pos=0;
	static unsigned char movex, movey, movec;
	static short xtilast[8];
	static short ytilast[8];
	static int clast[8];
	xtilast[pos]=xti;
	ytilast[pos]=yti;
	clast[pos]=c;
	if(complete==1){
		if(pos==1){
			DrawPixel(xtilast[0],ytilast[0],clast[0]);
		} else {
			DrawLine(xtilast[0],ytilast[0],xtilast[pos-1],ytilast[pos-1],1,clast[0]);
		}
		pos=0;
	} else {
		if(pos==0){
			movex = movey = movec = 1;
			pos+=1;
		} else {
			if(xti==xtilast[0] && abs(yti-ytilast[pos-1])==1)movex=0;else movex=1;
			if(yti==ytilast[0] && abs(xti-xtilast[pos-1])==1)movey=0;else movey=1;
			if(c==clast[0])movec=0;else movec=1;
			if(movec==0 && (movex==0 || movey==0) && pos<6) pos+=1;
			else {
				if(pos==1){
					DrawPixel(xtilast[0],ytilast[0],clast[0]);
				} else {
					DrawLine(xtilast[0],ytilast[0],xtilast[pos-1],ytilast[pos-1],1,clast[0]);
				}
				movex = movey = movec = 1;
				xtilast[0]=xti;
				ytilast[0]=yti;
				clast[0]=c;
				pos=1;
			}
		}
	}
}

void bezier(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, int c){
    float tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8,t=0.0,xt=x0,yt=y0;
    int i, xti,yti,xtlast=-1, ytlast=-1;
    for (i=0; i<500; i++)
    {
    	tmp = 1.0 - t;
    	tmp3 = t * t;
    	tmp4 = tmp * tmp;
    	tmp1 = tmp3 * t;
    	tmp2 = tmp4 * tmp;
    	tmp5 = 3.0 * t;
    	tmp6 = 3.0 *  tmp3;
    	tmp7 = tmp5 * tmp4;
    	tmp8 = tmp6 * tmp;
    	xti=(int)xt;
    	yti=(int)yt;
    	xt =    ((((tmp2 * x0) + (tmp7 * x1)) + (tmp8 * x2)) + (tmp1 * x3));
    	yt =    ((((tmp2 * y0) + (tmp7 * y1)) +	(tmp8 * y2)) +(tmp1 * y3));
    	if((xti!=xtlast) || (yti!=ytlast)) {
    		DrawBuffered(xti, yti, c, 0);
    		xtlast=xti;
    		ytlast=yti;
    	}
    	t+=0.002;
    	routinechecks(1);
    }
	DrawBuffered(0, 0, 0, 1);
}

void cmd_bezier(void) {
	// X coordinate of start point
	// Y coordinate of start point
	// X coordinate of first control point
	// Y coordinate of first control point
	// X coordinate of second control point
	// Y coordinate of second control point
	// X coordinate of end point
	// Y coordinate of end point
	// Colour of curve
		int maxH=PageTable[WritePage].ymax;
	    int maxW=PageTable[WritePage].xmax;
	int xx0, yy0, xx1, yy1, xx2, yy2, xx3, yy3, c;
    getargs(&cmdline, 17,",");
    if(!(argc==15 || argc==17))error("Argument count");
    xx0 = getint(argv[0],0,maxW-1);
    yy0 = getint(argv[2],0,maxH-1);
    xx1 = getinteger(argv[4]);
    yy1 = getinteger(argv[6]);
    xx2 = getinteger(argv[8]);
    yy2 = getinteger(argv[10]);
    xx3 = getint(argv[12],0,maxW-1);
    yy3 = getint(argv[14],0,maxH-1);
    c = gui_fcolour;                                    // setup the defaults
    if(argc > 15 && *argv[16]) c = getint(argv[16], 0, WHITE);
    bezier(xx0, yy0, xx1, yy1, xx2, yy2, xx3, yy3, c);
}

*/

void polygon(char *p, int close){
	int xcount=0;
	long long int *xptr=NULL, *yptr=NULL,xptr2=0, yptr2=0, *polycount=NULL, *cptr=NULL, *fptr=NULL;
	MMFLOAT *polycountf=NULL, *cfptr=NULL, *ffptr=NULL, *xfptr=NULL, *yfptr=NULL, xfptr2=0, yfptr2=0;
	int i, f=0, c, xtot=0, ymax=0, ymin=1000000, idx = 0;
    //volatile int n=0, nx=0, ny=0, nc=0, nf=0;
    int n=0, nx=0, ny=0, nc=0, nf=0;
    int xstride = sizeof(MMFLOAT), ystride = sizeof(MMFLOAT);
    getargs(&p, 9,",");
//	int maxH=PageTable[WritePage].ymax;
//    int maxW=PageTable[WritePage].xmax;
    getargaddress(argv[0], &polycount, &polycountf, &n,NULL);
    if(n==1){
    	xcount = xtot = getinteger(argv[0]);
    	if((xcount<3 || xcount>9999) && xcount!=0)error("Invalid number of vertices");
        getargaddress(argv[2], &xptr, &xfptr, &nx,&xstride);
        if(xcount==0){
            xcount = xtot = nx;
        }
        if(nx<xtot)error("X Dimensions %", nx);
        getargaddress(argv[4], &yptr, &yfptr, &ny,&ystride);
        if(ny<xtot)error("Y Dimensions %", ny);
        if(xptr)
        	xptr2 = STRIDE_INT(xptr, 0, xstride);
        	//xptr2=*xptr;
        else
        	xfptr2 = STRIDE_FLOAT(xfptr, 0, xstride);
        	//xfptr2=*xfptr;
        if(yptr)
        	yptr2 = STRIDE_INT(yptr, 0, ystride);
        	//yptr2=*yptr;
        else
        	yfptr2 = STRIDE_FLOAT(yfptr, 0, ystride);
        	//yfptr2=*yfptr;
        c = gui_fcolour;                                    // setup the defaults
        if(argc > 5 && *argv[6]) c = getint(argv[6], 0, WHITE);
        if(argc > 7 && *argv[8]){
        	main_fill_polyX=(TFLOAT  *)GetTempMemory(xtot * sizeof(TFLOAT));
        	main_fill_polyY=(TFLOAT  *)GetTempMemory(xtot * sizeof(TFLOAT));
        	f = getint(argv[8], 0, WHITE);
    		fill_set_fill_color((f>>16) & 0xFF, (f>>8) & 0xFF , f & 0xFF, (f>>24)&0xF);
        	fill_set_pen_color((c>>16) & 0xFF, (c>>8) & 0xFF , c & 0xFF, (c>>24)&0xF);
    //	    main_field_width = maxW;
    //	    main_field_height = maxH;
        	fill_begin_fill();
        }
        idx = 0;
        for(i=0;i<xcount-1;i++){
          	if(argc > 7){
                  //main_fill_polyX[main_fill_poly_vertex_count] = (xfptr==NULL ? (TFLOAT )*xptr++ : (TFLOAT )*xfptr++) ;
                  //main_fill_polyY[main_fill_poly_vertex_count] = (yfptr==NULL ? (TFLOAT )*yptr++ : (TFLOAT )*yfptr++) ;
                 main_fill_polyX[main_fill_poly_vertex_count] = (xfptr == NULL ? (TFLOAT)STRIDE_INT(xptr, idx, xstride) : (TFLOAT)STRIDE_FLOAT(xfptr, idx, xstride));
                 main_fill_polyY[main_fill_poly_vertex_count] = (yfptr == NULL ? (TFLOAT)STRIDE_INT(yptr, idx, ystride) : (TFLOAT)STRIDE_FLOAT(yfptr, idx, ystride));
                 idx++;
                 if(main_fill_polyY[main_fill_poly_vertex_count]>ymax)
                	 ymax=main_fill_polyY[main_fill_poly_vertex_count];
                 if(main_fill_polyY[main_fill_poly_vertex_count]<ymin)
                	 ymin=main_fill_polyY[main_fill_poly_vertex_count];
                 main_fill_poly_vertex_count++;
          	} else {
          		//int x1=(xfptr==NULL ? *xptr++ : (int)*xfptr++);
          		//int x2=(xfptr==NULL ? *xptr : (int)*xfptr);
          		//int y1=(yfptr==NULL ? *yptr++ : (int)*yfptr++);
          		//int y2=(yfptr==NULL ? *yptr : (int)*yfptr);
                int x1 = (xfptr == NULL ? STRIDE_INT(xptr, idx, xstride) : (int)STRIDE_FLOAT(xfptr, idx, xstride));
                int x2 = (xfptr == NULL ? STRIDE_INT(xptr, idx + 1, xstride) : (int)STRIDE_FLOAT(xfptr, idx + 1, xstride));
                int y1 = (yfptr == NULL ? STRIDE_INT(yptr, idx, ystride) : (int)STRIDE_FLOAT(yfptr, idx, ystride));
                int y2 = (yfptr == NULL ? STRIDE_INT(yptr, idx + 1, ystride) : (int)STRIDE_FLOAT(yfptr, idx + 1, ystride));
                idx++;
           		DrawLine(x1,y1,x2,y2, 1, c);
           	}
        }
        if(argc > 7){
            //main_fill_polyX[main_fill_poly_vertex_count] = (xfptr==NULL ? (TFLOAT )*xptr++ : (TFLOAT )*xfptr++) ;
            //main_fill_polyY[main_fill_poly_vertex_count] = (yfptr==NULL ? (TFLOAT )*yptr++ : (TFLOAT )*yfptr++) ;
            main_fill_polyX[main_fill_poly_vertex_count] = (xfptr == NULL ? (TFLOAT)STRIDE_INT(xptr, idx, xstride) : (TFLOAT)STRIDE_FLOAT(xfptr, idx, xstride));
            main_fill_polyY[main_fill_poly_vertex_count] = (yfptr == NULL ? (TFLOAT)STRIDE_INT(yptr, idx, ystride) : (TFLOAT)STRIDE_FLOAT(yfptr, idx, ystride));
            if(main_fill_polyY[main_fill_poly_vertex_count]>ymax)
            	ymax=main_fill_polyY[main_fill_poly_vertex_count];
            if(main_fill_polyY[main_fill_poly_vertex_count]<ymin)
            	ymin=main_fill_polyY[main_fill_poly_vertex_count];
            if(main_fill_polyY[main_fill_poly_vertex_count]!=main_fill_polyY[0] || main_fill_polyX[main_fill_poly_vertex_count] != main_fill_polyX[0]){
                	main_fill_poly_vertex_count++;
                	main_fill_polyX[main_fill_poly_vertex_count]=main_fill_polyX[0];
                	main_fill_polyY[main_fill_poly_vertex_count]=main_fill_polyY[0];
            }
            main_fill_poly_vertex_count++;
        	if(main_fill_poly_vertex_count>5){
        		fill_end_fill(xcount,ymin,ymax);
        	} else if(main_fill_poly_vertex_count==5){
				DrawTriangle(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[1],main_fill_polyY[1],main_fill_polyX[2],main_fill_polyY[2],f,f);
				DrawTriangle(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[2],main_fill_polyY[2],main_fill_polyX[3],main_fill_polyY[3],f,f);
				if(f!=c){
					DrawLine(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[1],main_fill_polyY[1],1,c);
					DrawLine(main_fill_polyX[1],main_fill_polyY[1],main_fill_polyX[2],main_fill_polyY[2],1,c);
					DrawLine(main_fill_polyX[2],main_fill_polyY[2],main_fill_polyX[3],main_fill_polyY[3],1,c);
					DrawLine(main_fill_polyX[3],main_fill_polyY[3],main_fill_polyX[4],main_fill_polyY[4],1,c);
				}
			} else {
				DrawTriangle(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[1],main_fill_polyY[1],main_fill_polyX[2],main_fill_polyY[2],c,f);
        	}
       } else if(close){
    		//int x1=(xfptr==NULL ? *xptr : (int)*xfptr);
    		int x1 = (xfptr == NULL ? STRIDE_INT(xptr, idx, xstride) : (int)STRIDE_FLOAT(xfptr, idx, xstride));
    		int x2=(xfptr==NULL ? xptr2 : (int)xfptr2);
    		//int y1=(yfptr==NULL ? *yptr : (int)*yfptr);
    		int y1 = (yfptr == NULL ? STRIDE_INT(yptr, idx, ystride) : (int)STRIDE_FLOAT(yfptr, idx, ystride));
    		int y2=(yfptr==NULL ? yptr2 : (int)yfptr2);
    		DrawLine(x1,y1,x2,y2, 1, c);
        }
    } else {
    	int *cc=GetTempMemory(n*sizeof(int)); //array for foreground colours
    	int *ff=GetTempMemory(n*sizeof(int)); //array for background colours
    	int xstart ,j, xmax=0;
    	for(i=0;i<n;i++){
    		if((polycountf == NULL ? polycount[i] : (int)polycountf[i])>xmax)xmax=(polycountf == NULL ? polycount[i] : (int)polycountf[i]);
    		if(!(polycountf == NULL ? polycount[i] : (int)polycountf[i]))break;
    		xtot+=(polycountf == NULL ? polycount[i] : (int)polycountf[i]);
    		if((polycountf == NULL ? polycount[i] : (int)polycountf[i])<3 || (polycountf == NULL ? polycount[i] : (int)polycountf[i])>9999)error("Invalid number of vertices, polygon %",i);
    	}
    	n=i;
    	getargaddress(argv[2], &xptr, &xfptr, &nx, &xstride);
        //getargaddress(argv[2], &xptr, &xfptr, &nx,NULL);
        if(nx<xtot)error("X Dimensions %", nx);
        getargaddress(argv[4], &yptr, &yfptr, &ny, &ystride);
        //getargaddress(argv[4], &yptr, &yfptr, &ny,NULL);
        if(ny<xtot)error("Y Dimensions %", ny);
    	main_fill_polyX=(TFLOAT  *)GetTempMemory(xmax * sizeof(TFLOAT));
    	main_fill_polyY=(TFLOAT  *)GetTempMemory(xmax * sizeof(TFLOAT));
		if(argc > 5 && *argv[6]){ //foreground colour specified
			getargaddress(argv[6], &cptr, &cfptr, &nc,NULL);
			if(nc == 1) for(i=0;i<n;i++)cc[i] = getint(argv[6], 0, WHITE);
			else {
				if(nc < n) error("Foreground colour Dimensions");
				for(i=0;i<n;i++){
                	if(CMM1){
                		cc[i] = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(cc[i] < 0 || cc[i] > 7) error("% is invalid (valid is % to %)", (int)cc[i], 0, 7);
                	} else {
                		cc[i] = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(cc[i] < 0 || cc[i] > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)cc[i], 0, 0xFFFFFFF);
                	}
				}
			}
		} else for(i=0;i<n;i++)cc[i] = gui_fcolour;
		if(argc > 7){ //background colour specified
			getargaddress(argv[8], &fptr, &ffptr, &nf,NULL);
			if(nf == 1) for(i=0;i<n;i++) ff[i] = getint(argv[8], 0, WHITE);
			else {
				if(nf < n) error("Background colour Dimensions");
				for(i=0;i<n;i++){
                	if(CMM1){
                		ff[i] = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(ff[i] < 0 || ff[i] > 7) error("% is invalid (valid is % to %)", (int)ff[i], 0, 7);
                	} else {
                		ff[i] = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(ff[i] < 0 || ff[i] > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)ff[i], 0, 0xFFFFFFF);
                	}
				}
			}
		}
    	xstart=0;
    	idx = 0;
    	for(i=0;i<n;i++){
            if(xptr)
            	xptr2 = STRIDE_INT(xptr, idx, xstride);
            else
            	xfptr2 = STRIDE_FLOAT(xfptr, idx, xstride);
            if(yptr)
            	yptr2 = STRIDE_INT(yptr, idx, ystride);
            else
            	yfptr2 = STRIDE_FLOAT(yfptr, idx, ystride);
            ymax=0;
    		ymin=1000000;
    		main_fill_poly_vertex_count=0;
        	xcount = (int)(polycountf == NULL ? polycount[i] : (int)polycountf[i]);
            if(argc > 7 && *argv[8]){
            	fill_set_pen_color((cc[i]>>16) & 0xFF, (cc[i]>>8) & 0xFF , cc[i] & 0xFF, (cc[i]>>24)&0xF);
        		fill_set_fill_color((ff[i]>>16) & 0xFF, (ff[i]>>8) & 0xFF , ff[i] & 0xFF, (ff[i]>>24)&0xF);
   //     	    main_field_width = maxW;
   //     	    main_field_height = maxH;
            	fill_begin_fill();
            }
           for(j=xstart;j<xstart+xcount-1;j++){
            	if(argc > 7){
                    //main_fill_polyX[main_fill_poly_vertex_count] = (xfptr==NULL ? (TFLOAT )*xptr++ : (TFLOAT )*xfptr++) ;
                   // main_fill_polyY[main_fill_poly_vertex_count] = (yfptr==NULL ? (TFLOAT )*yptr++ : (TFLOAT )*yfptr++) ;
                    main_fill_polyX[main_fill_poly_vertex_count] = (xfptr == NULL ? (TFLOAT)STRIDE_INT(xptr, idx, xstride) : (TFLOAT)STRIDE_FLOAT(xfptr, idx, xstride));
                    main_fill_polyY[main_fill_poly_vertex_count] = (yfptr == NULL ? (TFLOAT)STRIDE_INT(yptr, idx, ystride) : (TFLOAT)STRIDE_FLOAT(yfptr, idx, ystride));
                    idx++;
                    if(main_fill_polyY[main_fill_poly_vertex_count]>ymax)
                    	ymax=main_fill_polyY[main_fill_poly_vertex_count];
                    if(main_fill_polyY[main_fill_poly_vertex_count]<ymin)
                    	ymin=main_fill_polyY[main_fill_poly_vertex_count];
                    main_fill_poly_vertex_count++;
            	} else {
            		//int x1=(xfptr==NULL ? *xptr++ : (int)*xfptr++);
            		//int x2=(xfptr==NULL ? *xptr : (int)*xfptr);
            		//int y1=(yfptr==NULL ? *yptr++ : (int)*yfptr++);
            		//int y2=(yfptr==NULL ? *yptr : (int)*yfptr);
                    int x1 = (xfptr == NULL ? STRIDE_INT(xptr, idx, xstride) : (int)STRIDE_FLOAT(xfptr, idx, xstride));
                    int x2 = (xfptr == NULL ? STRIDE_INT(xptr, idx + 1, xstride) : (int)STRIDE_FLOAT(xfptr, idx + 1, xstride));
                    int y1 = (yfptr == NULL ? STRIDE_INT(yptr, idx, ystride) : (int)STRIDE_FLOAT(yfptr, idx, ystride));
                    int y2 = (yfptr == NULL ? STRIDE_INT(yptr, idx + 1, ystride) : (int)STRIDE_FLOAT(yfptr, idx + 1, ystride));
                    idx++;
            		DrawLine(x1,y1,x2,y2, 1, cc[i]);
            	}
            }
            if(argc > 7){
                //main_fill_polyX[main_fill_poly_vertex_count] = (xfptr==NULL ? (TFLOAT )*xptr++ : (TFLOAT )*xfptr++) ;
                //main_fill_polyY[main_fill_poly_vertex_count] = (yfptr==NULL ? (TFLOAT )*yptr++ : (TFLOAT )*yfptr++) ;
                main_fill_polyX[main_fill_poly_vertex_count] = (xfptr == NULL ? (TFLOAT)STRIDE_INT(xptr, idx, xstride) : (TFLOAT)STRIDE_FLOAT(xfptr, idx, xstride));
                main_fill_polyY[main_fill_poly_vertex_count] = (yfptr == NULL ? (TFLOAT)STRIDE_INT(yptr, idx, ystride) : (TFLOAT)STRIDE_FLOAT(yfptr, idx, ystride));
                idx++;
                if(main_fill_polyY[main_fill_poly_vertex_count]>ymax)
                	ymax=main_fill_polyY[main_fill_poly_vertex_count];
                if(main_fill_polyY[main_fill_poly_vertex_count]<ymin)
                	ymin=main_fill_polyY[main_fill_poly_vertex_count];
                if(main_fill_polyY[main_fill_poly_vertex_count]!=main_fill_polyY[0] || main_fill_polyX[main_fill_poly_vertex_count] != main_fill_polyX[0]){
                    	main_fill_poly_vertex_count++;
                    	main_fill_polyX[main_fill_poly_vertex_count]=main_fill_polyX[0];
                    	main_fill_polyY[main_fill_poly_vertex_count]=main_fill_polyY[0];
                }
                main_fill_poly_vertex_count++;
            	if(main_fill_poly_vertex_count>5){
            		fill_end_fill(xcount,ymin,ymax);
            	} else if(main_fill_poly_vertex_count==5){
    				DrawTriangle(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[1],main_fill_polyY[1],main_fill_polyX[2],main_fill_polyY[2],ff[i],ff[i]);
    				DrawTriangle(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[2],main_fill_polyY[2],main_fill_polyX[3],main_fill_polyY[3],ff[i],ff[i]);
    				if(ff[i]!=cc[i]){
    					DrawLine(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[1],main_fill_polyY[1],1,cc[i]);
    					DrawLine(main_fill_polyX[1],main_fill_polyY[1],main_fill_polyX[2],main_fill_polyY[2],1,cc[i]);
    					DrawLine(main_fill_polyX[2],main_fill_polyY[2],main_fill_polyX[3],main_fill_polyY[3],1,cc[i]);
    					DrawLine(main_fill_polyX[3],main_fill_polyY[3],main_fill_polyX[4],main_fill_polyY[4],1,cc[i]);
    				}
    			} else {
    				DrawTriangle(main_fill_polyX[0],main_fill_polyY[0],main_fill_polyX[1],main_fill_polyY[1],main_fill_polyX[2],main_fill_polyY[2],cc[i],ff[i]);
            	}
            } else {
        		//int x1=(xfptr==NULL ? *xptr : (int)*xfptr);
            	int x1 = (xfptr == NULL ? STRIDE_INT(xptr, idx, xstride) : (int)STRIDE_FLOAT(xfptr, idx, xstride));
        		int x2=(xfptr==NULL ? xptr2 : (int)xfptr2);
        		//int y1=(yfptr==NULL ? *yptr : (int)*yfptr);
        		int y1 = (yfptr == NULL ? STRIDE_INT(yptr, idx, ystride) : (int)STRIDE_FLOAT(yfptr, idx, ystride));
        		int y2=(yfptr==NULL ? yptr2 : (int)yfptr2);
        		DrawLine(x1,y1,x2,y2, 1, cc[i]);
        		idx++;
            	//if(xfptr!=NULL)xfptr++;
            	//else xptr++;
            	//if(yfptr!=NULL)yfptr++;
            	//else yptr++;
            }

            xstart+=xcount;
    	}
    }
}
void cmd_polygon(void){
    polygon((char *)cmdline,1);
}

void cmd_circle(void) {
	if(CMM1){
		int x, y, radius, colour, fill;
		float aspect;
		getargs(&cmdline, 9, ",");
		if(argc%2 == 0 || argc < 3) error("Invalid syntax");
		if(*argv[0] != '(') error("Expected opening bracket");
		if(toupper(*argv[argc - 1]) == 'F') {
	    	argc -= 2;
	    	fill = true;
	    } else fill = false;
		getcoord(argv[0] , &x, &y);
		radius = getinteger(argv[2]);
		if(radius == 0) return;                                         //nothing to draw
		if(radius < 1) error("Invalid argument");
		if(argc > 3 && *argv[4])colour = getColour(argv[4],0);
		else colour = gui_fcolour;

		if(argc > 5 && *argv[6])
		    aspect = getnumber(argv[6]);
		else
		    aspect = 1;

		DrawCircle(x, y, radius, (fill ? 0:1), colour, (fill ? colour : -1), aspect);
		lastx = x; lasty = y;
	} else {
		int x, y, r, w=0, c=0, f=0, n=0 ,i, nc=0, nw=0, nf=0, na=0;
        int xstride = sizeof(MMFLOAT), ystride = sizeof(MMFLOAT), rstride = sizeof(MMFLOAT);
        int wstride = sizeof(MMFLOAT), astride = sizeof(MMFLOAT), cstride = sizeof(MMFLOAT), fstride = sizeof(MMFLOAT);
		MMFLOAT a;
		long long int *xptr, *yptr, *rptr, *fptr, *wptr, *cptr, *aptr;
		MMFLOAT *xfptr, *yfptr, *rfptr, *ffptr, *wfptr, *cfptr, *afptr;
		getargs(&cmdline, 13,",");
		if(!(argc & 1) || argc < 5) error("Argument count");
		//getargaddress(argv[0], &xptr, &xfptr, &n,NULL);
		getargaddress(argv[0], &xptr, &xfptr, &n, &xstride);
		if(n != 1) {
			getargaddress(argv[2], &yptr, &yfptr, &n,&ystride);
			getargaddress(argv[4], &rptr, &rfptr, &n,&rstride);
		}
		if(n==1){
			w = 1; c = gui_fcolour; f = -1; a = 1;                          // setup the defaults
			x = getinteger(argv[0]);
			y = getinteger(argv[2]);
			r = getinteger(argv[4]);
			if(argc > 5 && *argv[6]) w = getint(argv[6], 0, 100);
			if(argc > 7 && *argv[8]) a = getnumber(argv[8]);
			if(argc > 9 && *argv[10]) c = getColour(argv[10], 0);
			if(argc > 11) f = getColour(argv[12], 1);
			DrawCircle(x, y, r, w, c, f, a);
		} else {
			w = 1; c = gui_fcolour; f = -1; a = 1;                          // setup the defaults
			if(argc > 5 && *argv[6]) {
				//getargaddress(argv[6], &wptr, &wfptr, &nw,NULL);
				getargaddress(argv[6], &wptr, &wfptr, &nw, &wstride);
				if(nw == 1) w = getint(argv[6], 0, 100);
				else if(nw>1) {
					if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
					for(i=0;i<nw;i++){
						w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
						//w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
						if(w < 0 || w > 100) error("% is invalid (valid is % to %)", (int)w, 0, 100);
					}
				}
			}
			if(argc > 7 && *argv[8]){
				getargaddress(argv[8], &aptr, &afptr, &na, &astride);
				//getargaddress(argv[8], &aptr, &afptr, &na,NULL);
				if(na == 1) a = getnumber(argv[8]);
				if(na > 1 && na < n) n=na; //adjust the dimensionality
			}
			if(argc > 9 && *argv[10]){
				getargaddress(argv[10], &cptr, &cfptr, &nc, &cstride);
				//getargaddress(argv[10], &cptr, &cfptr, &nc,NULL);
				if(nc == 1) c = getint(argv[10], 0, WHITE);
				else if(nc>1) {
					if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
					for(i=0;i<nc;i++){
	                	if(CMM1){
	                		c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
	                		if(c < 0 || c > 7) error("% is invalid (valid is % to %)", (int)c, 0, 7);
	                	} else {
	                		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
	                		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
	                		if(c < 0 || c > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)c, 0, 0xFFFFFFF);
	                	}
					}
				}
			}
			if(argc > 11){
				//getargaddress(argv[12], &fptr, &ffptr, &nf,NULL);
				getargaddress(argv[12], &fptr, &ffptr, &nf, &fstride);
				if(nf == 1) f = getint(argv[12], -1, WHITE);
				else if(nf>1) {
					if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
					for(i=0;i<nf;i++){
	                	if(CMM1){
	                		f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
	                		if(f < 0 || f > 7) error("% is invalid (valid is % to %)", (int)f, 0, 7);
	                	} else {
	                		f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
	                		//f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
	                		if(f < 0 || f > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)f, 0, 0xFFFFFFF);
	                	}
					}
				}
			}
			for(i=0;i<n;i++){
				//x = (xfptr==NULL ? xptr[i] : (int)xfptr[i]);
				//y = (yfptr==NULL ? yptr[i] : (int)yfptr[i]);
				//r = (rfptr==NULL ? rptr[i] : (int)rfptr[i])-1;
                x = (xfptr == NULL ? STRIDE_INT(xptr, i, xstride) : (int)STRIDE_FLOAT(xfptr, i, xstride));
                y = (yfptr == NULL ? STRIDE_INT(yptr, i, ystride) : (int)STRIDE_FLOAT(yfptr, i, ystride));
                r = (rfptr == NULL ? STRIDE_INT(rptr, i, rstride) : (int)STRIDE_FLOAT(rfptr, i, rstride)) - 1;
				if(nw > 1)
					w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
					//w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
				if(nc > 1)
					c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
					//c = (cfptr==NULL ? cptr[i] : (int)cfptr[i]);
				if(nf > 1)
					f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
					//f = (ffptr==NULL ? fptr[i] : (int)ffptr[i]);
				if(na > 1)
					a = (afptr == NULL ? (MMFLOAT)STRIDE_INT(aptr, i, astride) : STRIDE_FLOAT(afptr, i, astride));
					//a = (afptr==NULL ? (MMFLOAT)aptr[i] : afptr[i]);
				DrawCircle(x, y, r, w, c, f, a);
			}
		}
	}
}
static int xb0,xb1,yb0,yb1;
void GetPixel(int x, int y, int *r, int *g, int *b){
    union colourmap
    {
        char rgbbytes[4];
        unsigned int rgb;
    } c;
    ReadBuffer(x,y,x,y,(char *)&c.rgb);
    *r=c.rgbbytes[2];
    *g=c.rgbbytes[1];
    *b=c.rgbbytes[0];
}

void drawAAPixel( int x , int y , MMFLOAT alpha, uint32_t c){
    int bgR, bgG, bgB;

    // Get the current background color of the pixel
    GetPixel(x, y, &bgR, &bgG, &bgB);
    union colourmap
    {
        unsigned char rgbbytes[4];
        unsigned int rgb;
    } col;
	col.rgb=c;
	col.rgbbytes[0]= (unsigned char)((MMFLOAT)col.rgbbytes[0]*alpha);
	col.rgbbytes[0]+=(unsigned char)((MMFLOAT)bgB*(1.0-alpha));
	col.rgbbytes[1]= (unsigned char)((MMFLOAT)col.rgbbytes[1]*alpha);
	col.rgbbytes[1]+=(unsigned char)((MMFLOAT)bgG*(1.0-alpha));
	col.rgbbytes[2]= (unsigned char)((MMFLOAT)col.rgbbytes[2]*alpha);
	col.rgbbytes[2]+=(unsigned char)((MMFLOAT)bgR*(1.0-alpha));
 	if(((x>=xb0 && x<=xb1) && (y>=yb0 && y<=yb1)))DrawPixel(x,y,col.rgb);
}

void drawAALine(MMFLOAT x0 , MMFLOAT y0 , MMFLOAT x1 , MMFLOAT y1, uint32_t c, int w)
{

// Ensure positive integer values for width
	if(w < 1) w = 1;

// If drawing a dot, the call drawDot function
//if Math.abs(y1 - y0) < 1.0 && Math.abs(x1 - x0) < 1.0
//  #drawDot (x0 + x1) / 2, (y0 + y1) / 2
//  return
    xb0=x0;xb1=x1;yb0=y0;yb1=y1;
    if(xb1<xb0)swap(xb1,xb0);
    if(yb1<yb0)swap(yb1,yb0);

// steep means that m > 1
	int steep = abs(y1 - y0) > abs(x1 - x0) ;
// swap the co-ordinates if slope > 1 or we
// draw backwards
	if (steep)
	{
		swap(x0 , y0);
		swap(x1 , y1);
	}
	if (x0 > x1)
	{
		swap(x0 ,x1);
		swap(y0 ,y1);
	}
	//compute the slope
	MMFLOAT dx = x1-x0;
	MMFLOAT dy = y1-y0;
	MMFLOAT gradient = dy/dx;
	if (dx == 0.0)
		gradient = 1;

//rotate w
	w = w * sqrt(1 + (gradient * gradient));

// Handle first endpoint
	MMFLOAT xend = round(x0);
	MMFLOAT yend = y0 - (w - 1) * 0.5 + gradient * (xend - x0);
	MMFLOAT xgap = 1 - (x0 + 0.5 - xend);
	MMFLOAT xpxl1 = xend; //this will be used in the main loop
	MMFLOAT ypxl1 = floor(yend);
	MMFLOAT fpart = yend - floor(yend);
	MMFLOAT rfpart = 1 - fpart;

	if(steep){
	  drawAAPixel(ypxl1    , xpxl1, rfpart * xgap, c);
	  for(int i=1;i<=w;i++) drawAAPixel(ypxl1 + i, xpxl1, 1, c);
	  drawAAPixel(ypxl1 + w, xpxl1,  fpart * xgap, c);
	} else {
	  drawAAPixel(xpxl1, ypxl1    , rfpart * xgap, c);
	  for(int i=1; i<=w; i++) drawAAPixel(xpxl1, ypxl1 + i, 1, c);
	  drawAAPixel(xpxl1, ypxl1 + w,  fpart * xgap, c);
	}
	MMFLOAT intery = yend + gradient; // first y-intersection for the main loop

// Handle second endpoint
	xend = round(x1);
	yend = y1 - (w - 1) * 0.5 + gradient * (xend - x1);
	xgap = 1 - (x1 + 0.5 - xend);
	MMFLOAT xpxl2 = xend; // this will be used in the main loop
	MMFLOAT ypxl2 = floor(yend);
	fpart = yend - floor(yend);
	rfpart = 1 - fpart;

	if(steep){
		drawAAPixel(ypxl2    , xpxl2, rfpart * xgap, c);
		for(int i=1;i<=w;i++) drawAAPixel(ypxl2 + i, xpxl2, 1, c);
		drawAAPixel(ypxl2 + w, xpxl2,  fpart * xgap, c);
	} else {
		drawAAPixel(xpxl2, ypxl2    , rfpart * xgap, c);
		for(int i=1; i<=w; i++) drawAAPixel(xpxl2, ypxl2 + i, 1, c);
		drawAAPixel(xpxl2, ypxl2 + w,  fpart * xgap, c);
	}
// main loop
	if(steep){
		for(int x=xpxl1 + 1; x<=xpxl2; x++){
			fpart = intery - floor(intery);
			rfpart = 1 - fpart;
			MMFLOAT y = floor(intery);
			drawAAPixel(y    , x, rfpart, c);
			for(int i=1;i<w;i++) drawAAPixel(y + i, x, 1, c);
			drawAAPixel(y + w, x,  fpart, c);
			intery = intery + gradient;
		}
	} else {
		for(int x=xpxl1 + 1; x<=xpxl2; x++){
			fpart = intery - floor(intery);
			rfpart = 1 - fpart;
			MMFLOAT y = floor(intery);
			drawAAPixel(x, y    , rfpart, c);
			for(int i=1;i<w;i++) drawAAPixel(x, y + i, 1, c);
			drawAAPixel(x, y + w,  fpart, c);
			intery = intery + gradient;
		}
	}
}
void cmd_line(void) {
	char *p;
	if(CMM1){
		int x1, y1, x2, y2, colour, box, fill;
		getargs(&cmdline, 5, ",");

		// check if it is actually a LINE INPUT command
		if(argc < 1) error("Invalid syntax");
		x1 = lastx; y1 = lasty; colour = gui_fcolour; box = false; fill = false;	// set the defaults for optional components
		p = argv[0];
		if(tokenfunction(*p) != op_subtract) {
			// the start point is specified - get the coordinates and step over to where the minus token should be
			if(*p != '(') error("Expected opening bracket");
			getcoord(p , &x1, &y1);
			p = getclosebracket(p) + 1;
			skipspace(p);
		}
		if(tokenfunction(*p) != op_subtract) error("Invalid syntax");
		p++;
		skipspace(p);
		if(*p != '(') error("Expected opening bracket");
		getcoord(p , &x2, &y2);
		if(argc > 1 && *argv[2]){
			colour = getColour(argv[2],0);
		}
		if(argc == 5) {
			box = (strchr(argv[4], 'b') != NULL || strchr(argv[4], 'B') != NULL);
			fill = (strchr(argv[4], 'f') != NULL || strchr(argv[4], 'F') != NULL);
		}
		if(box)
			DrawBox(x1, y1, x2, y2, 1, colour, (fill ? colour : -1));						// draw a box
		else
			DrawLine(x1, y1, x2, y2, 1, colour);								// or just a line

		lastx = x2; lasty = y2;											// save in case the user wants the last value
	} else {
		int x1, y1, x2, y2, w=0, c=0, n=0 ,i, nc=0, nw=0;
        if((p=checkstring(cmdline,"PLOT"))){
            long long int *y1ptr;
            MMFLOAT *y1fptr;
            int xs=0,xinc=1;
            int ys=0,yinc=1;
            int y1stride = sizeof(MMFLOAT);
            getargs(&p, 13,",");
            //getargaddress(argv[0], &y1ptr, &y1fptr, &n,NULL);
            getargaddress(argv[0], &y1ptr, &y1fptr, &n, &y1stride);
            if(n==1)error("Argument 1 is not an array");
            nc=n;
            if(argc>=3 && *argv[2])nc=getint(argv[2],1,HRes-1);
            if(nc>n)nc=n;
            if(argc>=5 && *argv[4])xs=getint(argv[4],0,HRes-1);
            if(argc>=7 && *argv[6])xinc=getint(argv[6],1,HRes-1);
            if(argc>=9 && *argv[8])ys=getint(argv[8],OptionBase,n-2+OptionBase);
            if(argc>=11 && *argv[10])yinc=getint(argv[10],1,n-1);
            c = gui_fcolour;  w = 1;                                        // setup the defaults
            if(argc == 13) c = getint(argv[12], 0, WHITE);
            int y=ys-OptionBase;
            for(i=0;i<(nc-1);i++){
              if(y>=nc)break;
              if(y+yinc>=nc)break;
              x1 = xs+i*xinc;
             // y1 = (y1fptr==NULL ? y1ptr[y] : (int)y1fptr[y]);
              y1 = (y1fptr == NULL ? STRIDE_INT(y1ptr, y, y1stride) : (int)STRIDE_FLOAT(y1fptr, y, y1stride));
              if(y1<0)y1=0;
              if(y1>=VRes)y1=VRes-1;
              x2 = xs+(i+1)*xinc;
             // y2 = (y1fptr==NULL ? y1ptr[y+yinc] : (int)y1fptr[y+yinc]);
              y2 = (y1fptr == NULL ? STRIDE_INT(y1ptr, y + yinc, y1stride) : (int)STRIDE_FLOAT(y1fptr, y + yinc, y1stride));
              if(x1>=HRes)break; //can only get worse so stop now
              if(x2>=HRes)x2=HRes-1;
              if(y2<0)y2=0;
              if(y2>=VRes)y2=VRes-1;
              DrawLine(x1, y1, x2, y2, w, c);
              y+=yinc;
            }
		} else if((p=checkstring(cmdline,"GRAPH"))){
            char *pp=GetTempMemory(STRINGSIZE);
            strcpy((char *)pp,(char *)p);
            memmove(&pp[2],pp,strlen((char *)p)+1);
            pp[0]='0';
            pp[1]=',';
            polygon(pp,0);
            return;
        } else if((p=checkstring(cmdline,"AA"))){
			MMFLOAT x1, y1, x2, y2;
			getargs(&p, 11,",");
			c = gui_fcolour;  ;  w = 1;                                         // setup the defaults
			x1 = getnumber(argv[0]);
			y1 = getnumber(argv[2]);
			x2 = getnumber(argv[4]);
			y2 = getnumber(argv[6]);
			if(argc > 7 && *argv[8]){
				w = getint(argv[8], 1, 100);
			}
			if(argc ==11) c = getColour(argv[10], 0);
			drawAALine(x1, y1, x2, y2, c, w);
			return;
		} else {
			long long int *x1ptr, *y1ptr, *x2ptr, *y2ptr, *wptr, *cptr;
			MMFLOAT *x1fptr, *y1fptr, *x2fptr, *y2fptr, *wfptr, *cfptr;
            int x1stride = sizeof(MMFLOAT), y1stride = sizeof(MMFLOAT), x2stride = sizeof(MMFLOAT), y2stride = sizeof(MMFLOAT);
            int wstride = sizeof(MMFLOAT), cstride = sizeof(MMFLOAT);
			getargs(&cmdline, 11,",");
			if(!(argc & 1) || argc < 7) error("Argument count");
			getargaddress(argv[0], &x1ptr, &x1fptr, &n, &x1stride);
			//getargaddress(argv[0], &x1ptr, &x1fptr, &n,NULL);
			if(n != 1) {
                getargaddress(argv[2], &y1ptr, &y1fptr, &n, &y1stride);
                getargaddress(argv[4], &x2ptr, &x2fptr, &n, &x2stride);
                getargaddress(argv[6], &y2ptr, &y2fptr, &n, &y2stride);
			}
			if(n==1){
				c = gui_fcolour;  w = 1;                                        // setup the defaults
				x1 = getinteger(argv[0]);
				y1 = getinteger(argv[2]);
				x2 = getinteger(argv[4]);
				y2 = getinteger(argv[6]);
				if(argc > 7 && *argv[8]){
					w = getint(argv[8], -100, 100);
					if(w==0)return;
				}
				if(argc == 11) c = getColour(argv[10], 0);
				DrawLine(x1, y1, x2, y2, w, c);
			} else {
				c = gui_fcolour;  w = 1;                                        // setup the defaults
				if(argc > 7 && *argv[8]){
					getargaddress(argv[8], &wptr, &wfptr, &nw, &wstride);
					//getargaddress(argv[8], &wptr, &wfptr, &nw,NULL);
					if(nw == 1) w = getint(argv[8], -100, 100);
					else if(nw>1) {
						if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
						for(i=0;i<nw;i++){
							w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
							//w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
							if(w < -100 || w > 100) error("% is invalid (valid is % to %)", (int)w, -100, 100);
						}
					}
				}
				if(argc == 11){
					getargaddress(argv[10], &cptr, &cfptr, &nc, &cstride);
					//getargaddress(argv[10], &cptr, &cfptr, &nc,NULL);
					if(nc == 1) c = getColour(argv[10], 0);
					else if(nc>1) {
						if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
						for(i=0;i<nc;i++){
							if(CMM1){
								c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
								if(c < 0 || c > 7) error("% is invalid (valid is % to %)", (int)c, 0, 7);
							} else {
								c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
								//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
								if(c < 0 || c > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)c, 0, 0xFFFFFFF);
							}
						}
					}
				}
				for(i=0;i<n;i++){
					//x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
					//y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
					//x2 = (x2fptr==NULL ? x2ptr[i] : (int)x2fptr[i]);
					//y2 = (y2fptr==NULL ? y2ptr[i] : (int)y2fptr[i]);
	                x1 = (x1fptr == NULL ? STRIDE_INT(x1ptr, i, x1stride) : (int)STRIDE_FLOAT(x1fptr, i, x1stride));
	                y1 = (y1fptr == NULL ? STRIDE_INT(y1ptr, i, y1stride) : (int)STRIDE_FLOAT(y1fptr, i, y1stride));
	                x2 = (x2fptr == NULL ? STRIDE_INT(x2ptr, i, x2stride) : (int)STRIDE_FLOAT(x2fptr, i, x2stride));
	                y2 = (y2fptr == NULL ? STRIDE_INT(y2ptr, i, y2stride) : (int)STRIDE_FLOAT(y2fptr, i, y2stride));
					if(nw > 1)
						w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
						//w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
					if(nc > 1)
						c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
						//c = (cfptr==NULL ? cptr[i] : (int)cfptr[i]);
					if(w)
						DrawLine(x1, y1, x2, y2, w, c);
				}
			}
		}
	}
}


void cmd_box(void) {
    int x1, y1, h, w=0, c=0, f=0,  n=0 ,i, nc=0, nw=0, nf=0,hmod,wmod;
    int nwidth = 0, nheight = 0, width = 0, height = 0;
    int x1stride = sizeof(MMFLOAT), y1stride = sizeof(MMFLOAT), wistride = sizeof(MMFLOAT), hstride = sizeof(MMFLOAT);
    int wstride = sizeof(MMFLOAT), cstride = sizeof(MMFLOAT), fstride = sizeof(MMFLOAT);
    long long int *x1ptr, *y1ptr, *wiptr, *hptr, *wptr, *cptr, *fptr;
    MMFLOAT *x1fptr, *y1fptr, *wifptr, *hfptr, *wfptr, *cfptr, *ffptr;
    char *tp;
	if((tp = checkstring(cmdline, "AND_PIXELS"))) {
		uint64_t ConvertedColour;
		int f, red, blue, green, trans, size;
		char *p;
		uint64_t *q;
		uint64_t map;
		getargs(&tp,11,",");
		if(argc < 9) error("Syntax");
		x1 = getinteger(argv[0]);
		y1 = getinteger(argv[2]);
		w = getinteger(argv[4]);
		h = getinteger(argv[6]);
		f = ConvertedColour = getColour(argv[8], 0);
		if(VideoColour==32){
			map=(ConvertedColour<<32) | ConvertedColour;
		} else if(VideoColour<=8){
			ConvertedColour = ((f & 0b111000000000000000000000)>>16) | ((f & 0b1110000000000000)>>11) | ((f & 0b11000000)>>6);
			map=(ConvertedColour<<56) | (ConvertedColour<<48) | (ConvertedColour<<40) | (ConvertedColour<<32) | (ConvertedColour<<24) | (ConvertedColour<<16) | (ConvertedColour<<8) | ConvertedColour;
		} else {
			if(VideoColour==16){
				red=BIT_5[((f & 0xFF0000)>>19)]<<11;
				green=BIT_6[((f & 0xFF00)>>10)]<<5;
				blue=BIT_5[((f & 0xFF)>>3)];
				ConvertedColour=red|green|blue;
			} else {
				red=BIT_4[((f & 0xFF0000)>>20)]<<8;
				green=BIT_4[((f & 0xFF00)>>12)]<<4;
				blue=BIT_4[((f & 0xFF)>>4)];
				trans=((f & 0xF000000)>>12);
				ConvertedColour=red|green|blue|trans;
			}
			map=(ConvertedColour<<48) | (ConvertedColour<<32) | (ConvertedColour<<16) | ConvertedColour;
		}
		if(argc>=10){
			if(checkstring(argv[10], "FRAMEBUFFER")){
				ReadPage=WPN;
			} else {
				ReadPage=getint(argv[10],0,LastPage);
			}
		}
		int readlimx=PageTable[ReadPage].xmax;
		int readlimy=PageTable[ReadPage].ymax;
		if(w < 1 || h < 1) {
			ReadPage=WritePage;
			return;
		}
		if(x1 < 0) {
			w += x1;
			x1 = 0;
		}
		if(y1 < 0) {
			h += y1;
			y1 = 0;
		}
		if(x1 + w > readlimx) {
			w = readlimx - x1;
		}
		if(y1 + h > readlimy) {
			h = readlimy - y1;
		}
		if(w < 1 || h < 1 || x1 < 0 || x1 + w > readlimx || y1 < 0 || y1 + h > readlimy) {
			ReadPage=WritePage;
			return;
		}
		size = w * h * PageTable[ReadPage].nbytes + 8;
		p=GetMemory(size);
		q=(uint64_t *)p;
		ReadBufferFast(x1,y1,x1+w-1,y1+h-1,p);
		size>>=3;
		while(size--){
			*q &=map;
			q++;
		}
		DrawBufferFast(x1,y1,x1+w-1,y1+h-1,p);
		ReadPage=WritePage;
		FreeMemorySafe((void *)&p);
		return;
	} else if((tp = checkstring(cmdline, "OR_PIXELS"))) {
		uint64_t ConvertedColour;
		int f, red, blue, green, trans, size;
		char *p;
		uint64_t *q;
		uint64_t map;
		getargs(&tp,11,",");
		if(argc < 9) error("Syntax");
		x1 = getinteger(argv[0]);
		y1 = getinteger(argv[2]);
		w = getinteger(argv[4]);
		h = getinteger(argv[6]);
		f = ConvertedColour = getColour(argv[8], 0);
		if(VideoColour==32){
			map=(ConvertedColour<<32) | ConvertedColour;
		} else if(VideoColour<=8){
			ConvertedColour = ((f & 0b111000000000000000000000)>>16) | ((f & 0b1110000000000000)>>11) | ((f & 0b11000000)>>6);
			map=(ConvertedColour<<56) | (ConvertedColour<<48) | (ConvertedColour<<40) | (ConvertedColour<<32) | (ConvertedColour<<24) | (ConvertedColour<<16) | (ConvertedColour<<8) | ConvertedColour;
		} else {
			if(VideoColour==16){
				red=BIT_5[((f & 0xFF0000)>>19)]<<11;
				green=BIT_6[((f & 0xFF00)>>10)]<<5;
				blue=BIT_5[((f & 0xFF)>>3)];
				ConvertedColour=red|green|blue;
			} else {
				red=BIT_4[((f & 0xFF0000)>>20)]<<8;
				green=BIT_4[((f & 0xFF00)>>12)]<<4;
				blue=BIT_4[((f & 0xFF)>>4)];
				trans=((f & 0xF000000)>>12);
				ConvertedColour=red|green|blue|trans;
			}
			map=(ConvertedColour<<48) | (ConvertedColour<<32) | (ConvertedColour<<16) | ConvertedColour;
		}
		if(argc>=10){
			if(checkstring(argv[10], "FRAMEBUFFER")){
				ReadPage=WPN;
			} else {
				ReadPage=getint(argv[10],0,LastPage);
			}
		}
		int readlimx=PageTable[ReadPage].xmax;
		int readlimy=PageTable[ReadPage].ymax;
		if(w < 1 || h < 1) {
			ReadPage=WritePage;
			return;
		}
		if(x1 < 0) {
			w += x1;
			x1 = 0;
		}
		if(y1 < 0) {
			h += y1;
			y1 = 0;
		}
		if(x1 + w > readlimx) {
			w = readlimx - x1;
		}
		if(y1 + h > readlimy) {
			h = readlimy - y1;
		}
		if(w < 1 || h < 1 || x1 < 0 || x1 + w > readlimx || y1 < 0 || y1 + h > readlimy) {
			ReadPage=WritePage;
			return;
		}
		size = w * h * PageTable[ReadPage].nbytes + 8;
		p=GetMemory(size);
		q=(uint64_t *)p;
		ReadBufferFast(x1,y1,x1+w-1,y1+h-1,p);
		size>>=3;
		while(size--){
			*q |=map;
			q++;
		}
		DrawBufferFast(x1,y1,x1+w-1,y1+h-1,p);
		ReadPage=WritePage;
		FreeMemorySafe((void *)&p);
		return;
	} else if((tp = checkstring(cmdline, "XOR_PIXELS"))) {
		uint64_t ConvertedColour;
		int f, red, blue, green, trans, size;
		char *p;
		uint64_t *q;
		uint64_t map;
		getargs(&tp,11,",");
		if(argc < 9) error("Syntax");
		x1 = getinteger(argv[0]);
		y1 = getinteger(argv[2]);
		w = getinteger(argv[4]);
		h = getinteger(argv[6]);
		f = ConvertedColour = getColour(argv[8], 0);
		if(VideoColour==32){
			map=(ConvertedColour<<32) | ConvertedColour;
		} else if(VideoColour<=8){
			ConvertedColour = ((f & 0b111000000000000000000000)>>16) | ((f & 0b1110000000000000)>>11) | ((f & 0b11000000)>>6);
			map=(ConvertedColour<<56) | (ConvertedColour<<48) | (ConvertedColour<<40) | (ConvertedColour<<32) | (ConvertedColour<<24) | (ConvertedColour<<16) | (ConvertedColour<<8) | ConvertedColour;
		} else {
			if(VideoColour==16){
				red=BIT_5[((f & 0xFF0000)>>19)]<<11;
				green=BIT_6[((f & 0xFF00)>>10)]<<5;
				blue=BIT_5[((f & 0xFF)>>3)];
				ConvertedColour=red|green|blue;
			} else {
				red=BIT_4[((f & 0xFF0000)>>20)]<<8;
				green=BIT_4[((f & 0xFF00)>>12)]<<4;
				blue=BIT_4[((f & 0xFF)>>4)];
				trans=((f & 0xF000000)>>12);
				ConvertedColour=red|green|blue|trans;
			}
			map=(ConvertedColour<<48) | (ConvertedColour<<32) | (ConvertedColour<<16) | ConvertedColour;
		}
		if(argc>=10){
			if(checkstring(argv[10], "FRAMEBUFFER")){
				ReadPage=WPN;
			} else {
				ReadPage=getint(argv[10],0,LastPage);
			}
		}
		int readlimx=PageTable[ReadPage].xmax;
		int readlimy=PageTable[ReadPage].ymax;
		if(w < 1 || h < 1) {
			ReadPage=WritePage;
			return;
		}
		if(x1 < 0) {
			w += x1;
			x1 = 0;
		}
		if(y1 < 0) {
			h += y1;
			y1 = 0;
		}
		if(x1 + w > readlimx) {
			w = readlimx - x1;
		}
		if(y1 + h > readlimy) {
			h = readlimy - y1;
		}
		if(w < 1 || h < 1 || x1 < 0 || x1 + w > readlimx || y1 < 0 || y1 + h > readlimy) {
			ReadPage=WritePage;
			return;
		}
		size = w * h * PageTable[ReadPage].nbytes + 8;
		p=GetMemory(size);
		q=(uint64_t *)p;
		ReadBufferFast(x1,y1,x1+w-1,y1+h-1,p);
		size>>=3;
		while(size--){
			*q ^=map;
			q++;
		}
		DrawBufferFast(x1,y1,x1+w-1,y1+h-1,p);
		ReadPage=WritePage;
		FreeMemorySafe((void *)&p);
		return;
	}
    getcsargs(&cmdline, 13);
    if (!(argc & 1) || argc < 7)
        StandardError(2);
    getargaddress(argv[0], &x1ptr, &x1fptr, &n, &x1stride);
    if (n != 1)
    {
        getargaddress(argv[2], &y1ptr, &y1fptr, &n, &y1stride);
    }
    if (n == 1)
    {
        c = gui_fcolour;
        w = 1;
        f = -1; // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        width = getinteger(argv[4]);
        height = getinteger(argv[6]);
        wmod = (width > 0 ? -1 : 1);
        hmod = (height > 0 ? -1 : 1);
        if (argc > 7 && *argv[8])
            w = getint(argv[8], 0, 100);
        if (argc > 9 && *argv[10])
            c = getint(argv[10], 0, WHITE);
        if (argc == 13)
            f = getint(argv[12], -1, WHITE);
        if (width != 0 && height != 0)
            DrawBox(x1, y1, x1 + width + wmod, y1 + height + hmod, w, c, f);
    }
    else
    {
        getargaddress(argv[4], &wiptr, &wifptr, &nwidth, &wistride);
        if (nwidth == 1)
            width = getint(argv[4], 1, HRes);
        else if (nwidth > 1)
        {
            if (nwidth > 1 && nwidth < n)
                n = nwidth; // adjust the dimensionality
            for (i = 0; i < nwidth; i++)
            {
                width = (wifptr == NULL ? STRIDE_INT(wiptr, i, wistride) : (int)STRIDE_FLOAT(wifptr, i, wistride));
                if (width < 1 || width > HRes)
                    error("Width % is invalid (valid is % to %)", (int)width, 1, HRes);
            }
        }
        getargaddress(argv[6], &hptr, &hfptr, &nheight, &hstride);
        if (nheight == 1)
            height = getint(argv[6], 1, VRes);
        else if (nheight > 1)
        {
            if (nheight > 1 && nheight < n)
                n = nheight; // adjust the dimensionality
            for (i = 0; i < nheight; i++)
            {
                height = (hfptr == NULL ? STRIDE_INT(hptr, i, hstride) : (int)STRIDE_FLOAT(hfptr, i, hstride));
                if (height < 1 || height > VRes)
                    error("Height % is invalid (valid is % to %)", (int)height, 1, VRes);
            }
        }
        c = gui_fcolour;
        w = 1; // setup the defaults
        if (argc > 7 && *argv[8])
        {
            getargaddress(argv[8], &wptr, &wfptr, &nw, &wstride);
            if (nw == 1)
                w = getint(argv[8], 0, 100);
            else if (nw > 1)
            {
                if (nw > 1 && nw < n)
                    n = nw; // adjust the dimensionality
                for (i = 0; i < nw; i++)
                {
                    w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
                    if (w < 0 || w > 100)
                        StandardErrorParam3(26, (int)w, 0, 100);
                }
            }
        }
        if (argc > 9 && *argv[10])
        {
            getargaddress(argv[10], &cptr, &cfptr, &nc, &cstride);
            if (nc == 1)
                c = getint(argv[10], 0, WHITE);
            else if (nc > 1)
            {
                if (nc > 1 && nc < n)
                    n = nc; // adjust the dimensionality
                for (i = 0; i < nc; i++)
                {
                    c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
                    if (c < 0 || c > WHITE)
                        StandardErrorParam3(26, (int)c, 0, WHITE);
                }
            }
        }
        if (argc == 13)
        {
            getargaddress(argv[12], &fptr, &ffptr, &nf, &fstride);
            if (nf == 1)
                f = getint(argv[12], 0, WHITE);
            else if (nf > 1)
            {
                if (nf > 1 && nf < n)
                    n = nf; // adjust the dimensionality
                for (i = 0; i < nf; i++)
                {
                    f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
                    if (f < -1 || f > WHITE)
                        StandardErrorParam3(26, (int)f, -1, WHITE);
                }
            }
        }
        for (i = 0; i < n; i++)
        {
            x1 = (x1fptr == NULL ? STRIDE_INT(x1ptr, i, x1stride) : (int)STRIDE_FLOAT(x1fptr, i, x1stride));
            y1 = (y1fptr == NULL ? STRIDE_INT(y1ptr, i, y1stride) : (int)STRIDE_FLOAT(y1fptr, i, y1stride));
            if (nwidth > 1)
                width = (wifptr == NULL ? STRIDE_INT(wiptr, i, wistride) : (int)STRIDE_FLOAT(wifptr, i, wistride));
            if (nheight > 1)
                height = (hfptr == NULL ? STRIDE_INT(hptr, i, hstride) : (int)STRIDE_FLOAT(hfptr, i, hstride));
            wmod = (width > 0 ? -1 : 1);
            hmod = (height > 0 ? -1 : 1);
            if (nw > 1)
                w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
            if (nc > 1)
                c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
            if (nf > 1)
                f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
            if (width != 0 && height != 0)
                DrawBox(x1, y1, x1 + width + wmod, y1 + height + hmod, w, c, f);
        }
    }
 }
#ifdef OLDBOX
	//BOX command
    getargs(&cmdline, 13,",");
    if(!(argc & 1) || argc < 7) error("Argument count");
    //getargaddress(argv[0], &x1ptr, &x1fptr, &n, &x1stride);
    getargaddress(argv[0], &x1ptr, &x1fptr, &n,NULL);
    if(n != 1) {
    	//getargaddress(argv[2], &y1ptr, &y1fptr, &n, &y1stride);
        getargaddress(argv[2], &y1ptr, &y1fptr, &n,NULL);
        getargaddress(argv[4], &wiptr, &wifptr, &n,NULL);
        getargaddress(argv[6], &hptr, &hfptr, &n,NULL);
    }
    if(n == 1){
        c = gui_fcolour; w = 1; f = -1;                                 // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        wi = getinteger(argv[4]) ;
        h = getinteger(argv[6]) ;
        wmod=(wi > 0 ? -1 : 1);
        hmod=(h > 0 ? -1 : 1);
        if(argc > 7 && *argv[8]) w = getint(argv[8], 0, 100);
        if(argc > 9 && *argv[10]) c = getColour(argv[10], 0);
        if(argc == 13) f = getColour(argv[12], 0);
        if(wi != 0 && h != 0) DrawBox(x1, y1, x1 + wi + wmod, y1 + h + hmod, w, c, f);
    } else {
        c = gui_fcolour;  w = 1;                                        // setup the defaults
        if(argc > 7 && *argv[8]){
            getargaddress(argv[8], &wptr, &wfptr, &nw,NULL);
            if(nw == 1) w = getint(argv[8], 0, 100);
            else if(nw>1) {
                if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
                for(i=0;i<nw;i++){
                    w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if(w < 0 || w > 100) error("% is invalid (valid is % to %)", (int)w, 0, 100);
                }
            }
        }
        if(argc > 9 && *argv[10]) {
            getargaddress(argv[10], &cptr, &cfptr, &nc,NULL);
            if(nc == 1) c = getColour(argv[10], 0);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                	if(CMM1){
                		c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(c < 0 || c > 7) error("% is invalid (valid is % to %)", (int)c, 0, 7);
                	} else {
                		c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(c < 0 || c > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)c, 0, 0xFFFFFFF);
                	}
                }
            }
        }
        if(argc == 13){
            getargaddress(argv[12], &fptr, &ffptr, &nf,NULL);
            if(nf == 1) f = getColour(argv[12], 0);
            else if(nf>1) {
                if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
                for(i=0;i<nf;i++){
                	if(CMM1){
                		f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(f < 0 || f > 7) error("% is invalid (valid is % to %)", (int)f, 0, 7);
                	} else {
                		f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(f < 0 || f > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)f, 0, 0xFFFFFFF);
                	}
                }
            }
        }
        for(i=0;i<n;i++){
            x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
            wi = (wifptr==NULL ? wiptr[i] : (int)wifptr[i]);
            h =  (hfptr==NULL ? hptr[i] : (int)hfptr[i]);
            wmod=(wi > 0 ? -1 : 1);
            hmod=(h > 0 ? -1 : 1);
            if(nw > 1) w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
            if(nc > 1) {
            	if(CMM1){
            		c = (cfptr == NULL ? colourmap[cptr[i]] : colourmap[(int)cfptr[i]]);
            	} else {
            		c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
            	}
            }
            if(nf > 1) {
            	if(CMM1){
            		f = (ffptr == NULL ? colourmap[fptr[i]] : colourmap[(int)ffptr[i]]);
            	} else {
            		f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
            	}
            }
            if(wi != 0 && h != 0) DrawBox(x1, y1, x1 + wi + wmod, y1 + h + hmod, w, c, f);
        }
    }
}
#endif


void cmd_rbox(void) {
    int x1, y1, wi, h, w=0, c=0, f=0,  r=0, n=0 ,i, nc=0, nw=0, nf=0,hmod,wmod;
    int x1stride = sizeof(MMFLOAT), y1stride = sizeof(MMFLOAT), wistride = sizeof(MMFLOAT), hstride = sizeof(MMFLOAT);
    int wstride = sizeof(MMFLOAT), cstride = sizeof(MMFLOAT), fstride = sizeof(MMFLOAT);
    long long int *x1ptr, *y1ptr, *wiptr, *hptr, *wptr, *cptr, *fptr;
    MMFLOAT *x1fptr, *y1fptr, *wifptr, *hfptr, *wfptr, *cfptr, *ffptr;
    getargs(&cmdline, 13,",");
    if(!(argc & 1) || argc < 7) error("Argument count");
    getargaddress(argv[0], &x1ptr, &x1fptr, &n, &x1stride);
    //getargaddress(argv[0], &x1ptr, &x1fptr, &n,NULL);
    if(n != 1) {
        getargaddress(argv[2], &y1ptr, &y1fptr, &n, &y1stride);
        getargaddress(argv[4], &wiptr, &wifptr, &n, &wistride);
        getargaddress(argv[6], &hptr, &hfptr, &n, &hstride);
    }
    if(n == 1){
        c = gui_fcolour; w = 1; f = -1; r = 10;                         // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        w = getinteger(argv[4]);
        h = getinteger(argv[6]);
        wmod=(w > 0 ? -1 : 1);
        hmod=(h > 0 ? -1 : 1);
        if(argc > 7 && *argv[8]) r = getint(argv[8], 0, 100);
        if(argc > 9 && *argv[10]) c = getColour(argv[10], 0);
        if(argc == 13) f = getColour(argv[12], 0);
        if(w != 0 && h != 0) DrawRBox(x1, y1, x1 + w + wmod, y1 + h + hmod, r, c, f);
    } else {
        c = gui_fcolour;  w = 1;                                        // setup the defaults
        if(argc > 7 && *argv[8]){
        	getargaddress(argv[8], &wptr, &wfptr, &nw, &wstride);
           // getargaddress(argv[8], &wptr, &wfptr, &nw,NULL);
            if(nw == 1) w = getint(argv[8], 0, 100);
            else if(nw>1) {
                if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
                for(i=0;i<nw;i++){
                	w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
                   // w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if(w < 0 || w > 100) error("% is invalid (valid is % to %)", (int)w, 0, 100);
                }
            }
        }
        if(argc > 9 && *argv[10]) {
        	getargaddress(argv[10], &cptr, &cfptr, &nc, &cstride);
            //getargaddress(argv[10], &cptr, &cfptr, &nc,NULL);
            if(nc == 1) c = getColour(argv[10], 0);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                	if(CMM1){
                		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
                		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(c < 0 || c > 7) error("% is invalid (valid is % to %)", (int)c, 0, 7);
                	} else {
                		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
                		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(c < 0 || c > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)c, 0, 0xFFFFFFF);
                	}
                }
            }
        }
        if(argc == 13){
        	getargaddress(argv[12], &fptr, &ffptr, &nf, &fstride);
            //getargaddress(argv[12], &fptr, &ffptr, &nf,NULL);
            if(nf == 1) f = getColour(argv[12], 0);
            else if(nf>1) {
                if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
                for(i=0;i<nf;i++){
                	if(CMM1){
                		f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
                		//f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(f < 0 || f > 7) error("% is invalid (valid is % to %)", (int)f, 0, 7);
                	} else {
                		f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
                		//f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(f < 0 || f > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)f, 0, 0xFFFFFFF);
                	}
                }
            }
        }
        for(i=0;i<n;i++){
           // x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
           // y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
           // wi = (wifptr==NULL ? wiptr[i] : (int)wifptr[i]);
           // h =  (hfptr==NULL ? hptr[i] : (int)hfptr[i]);
            x1 = (x1fptr == NULL ? STRIDE_INT(x1ptr, i, x1stride) : (int)STRIDE_FLOAT(x1fptr, i, x1stride));
            y1 = (y1fptr == NULL ? STRIDE_INT(y1ptr, i, y1stride) : (int)STRIDE_FLOAT(y1fptr, i, y1stride));
            wi = (wifptr == NULL ? STRIDE_INT(wiptr, i, wistride) : (int)STRIDE_FLOAT(wifptr, i, wistride));
            h = (hfptr == NULL ? STRIDE_INT(hptr, i, hstride) : (int)STRIDE_FLOAT(hfptr, i, hstride));
            wmod=(wi > 0 ? -1 : 1);
            hmod=(h > 0 ? -1 : 1);
            if(nw > 1)
            	w = (wfptr == NULL ? STRIDE_INT(wptr, i, wstride) : (int)STRIDE_FLOAT(wfptr, i, wstride));
            	//w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
            if(nc > 1) {
            	if(CMM1){
            		c = (cfptr == NULL ? colourmap[STRIDE_INT(cptr, i, cstride)] : colourmap[(int)STRIDE_FLOAT(cfptr, i, cstride)]);
            		//c = (cfptr == NULL ? colourmap[cptr[i]] : colourmap[(int)cfptr[i]]);
            	} else {
            		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
            		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
            	}
            }
            if(nf > 1) {
            	if(CMM1){
            		f = (ffptr == NULL ? colourmap[STRIDE_INT(fptr, i, fstride)] : colourmap[(int)STRIDE_FLOAT(ffptr, i, fstride)]);
            		//f = (ffptr == NULL ? colourmap[fptr[i]] : colourmap[(int)ffptr[i]]);
            	} else {
            		f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
            		//f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
            	}
            }
            if(wi != 0 && h != 0) DrawRBox(x1, y1, x1 + wi + wmod, y1 + h + hmod, w, c, f);
        }
    }
}



// these three functions were written by Peter Mather (matherp on the Back Shed forum)
// read the contents of a pIXEL out of screen memory
//PIXEL( x, y [,page_number])
void fun_pixel(void) {
    union colourmap
    {
        char rgbbytes[4];
        unsigned int rgb;
    } c;
    int x,y;
	getargs(&ep, 5,",");
	if(!(argc == 3 || argc==5)) error("Argument count");
    x = getinteger(argv[0]);
    y = getinteger(argv[2]);
    if(argc==5){
    	if(checkstring(argv[4], "FRAMEBUFFER")){
    		ReadPage=WPN;
    	} else {
			ReadPage=getint(argv[4],0,LastPage);
    	}
    }
    ReadBuffer(x,y,x,y,(char *)&c.rgb);
    ReadPage=WritePage;
   // if(VideoColour==8 || VideoColour==16)iret = c.rgb & 0xFFFFFF;
   // else if(VideoColour==12) iret = c.rgb & 0xFFFFFFF;
    if(VideoColour==12) iret = c.rgb & 0xFFFFFFF;
    else iret = c.rgb & 0xFFFFFF;
   // else iret = c.rgb & 0xFFFFFFFF;  //VideoColour==32  RGB888 and transaperncy 0-255
    targ = T_INT;
}

void cmd_triangle(void) {                                           // thanks to Peter Mather (matherp on the Back Shed forum)
    int x1, y1, x2, y2, x3, y3, c=0, f=0,  n=0,i, nc=0, nf=0;
    int x1stride = sizeof(MMFLOAT), y1stride = sizeof(MMFLOAT), x2stride = sizeof(MMFLOAT), y2stride = sizeof(MMFLOAT);
    int x3stride = sizeof(MMFLOAT), y3stride = sizeof(MMFLOAT), cstride = sizeof(MMFLOAT), fstride = sizeof(MMFLOAT);
    long long int *x3ptr, *y3ptr, *x1ptr, *y1ptr, *x2ptr, *y2ptr, *fptr, *cptr;
    MMFLOAT *x3fptr, *y3fptr, *x1fptr, *y1fptr, *x2fptr, *y2fptr, *ffptr, *cfptr;
    getargs(&cmdline, 15,",");
    if(!(argc & 1) || argc < 11) error("Argument count");
    getargaddress(argv[0], &x1ptr, &x1fptr, &n, &x1stride);
    //getargaddress(argv[0], &x1ptr, &x1fptr, &n,NULL);
    if(n != 1) {
    	int cn=n;
        getargaddress(argv[2], &y1ptr, &y1fptr, &n, &y1stride);
       // getargaddress(argv[2], &y1ptr, &y1fptr, &n,NULL);
        if(n<cn)cn=n;
        getargaddress(argv[4], &x2ptr, &x2fptr, &n, &x2stride);
        //getargaddress(argv[4], &x2ptr, &x2fptr, &n,NULL);
        if(n<cn)cn=n;
        getargaddress(argv[6], &y2ptr, &y2fptr, &n, &y2stride);
        //getargaddress(argv[6], &y2ptr, &y2fptr, &n,NULL);
        if(n<cn)cn=n;
        getargaddress(argv[8], &x3ptr, &x3fptr, &n, &x3stride);
        //getargaddress(argv[8], &x3ptr, &x3fptr, &n,NULL);
        if(n<cn)cn=n;
        getargaddress(argv[10], &y3ptr, &y3fptr, &n, &y3stride);
        //getargaddress(argv[10], &y3ptr, &y3fptr, &n,NULL);
        if(n<cn)cn=n;
        n=cn;
    }
    if(n == 1){
        c = gui_fcolour; f = -1;
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        x2 = getinteger(argv[4]);
        y2 = getinteger(argv[6]);
        x3 = getinteger(argv[8]);
        y3 = getinteger(argv[10]);
        if(argc >= 13 && *argv[12]) c = getColour(argv[12], 0);
        if(argc == 15) f = getColour(argv[14], 1);
        DrawTriangle(x1, y1, x2, y2, x3, y3, c, f);
    } else {
        c = gui_fcolour; f = -1;
        if(argc >= 13 && *argv[12]) {
        	getargaddress(argv[12], &cptr, &cfptr, &nc, &cstride);
           // getargaddress(argv[12], &cptr, &cfptr, &nc,NULL);
            if(nc == 1) c = getColour(argv[12], 0);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                	if(CMM1){
                		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
                		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(c < 0 || c > 7) error("% is invalid (valid is % to %)", (int)c, 0, 7);
                	} else {
                		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
                		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                		if(c < 0 || c > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)c, 0, 0xFFFFFFF);
                	}
                }
            }
        }
        if(argc == 15){
        	getargaddress(argv[14], &fptr, &ffptr, &nf, &fstride);
            //getargaddress(argv[14], &fptr, &ffptr, &nf,NULL);
            if(nf == 1) f = getColour(argv[14], 0);
            else if(nf>1) {
                if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
                for(i=0;i<nf;i++){
                	if(CMM1){
                		f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
                		//f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(f < 0 || f > 7) error("% is invalid (valid is % to %)", (int)f, 0, 7);
                	} else {
                		f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
                		//f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                		if(f < -1 || f > 0xFFFFFFF) error("% is invalid (valid is % to %)", (int)f, -1,0xFFFFFFF);
                	}
                }
            }
        }
        for(i=0;i<n;i++){
           // x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
           // y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
           // x2 = (x2fptr==NULL ? x2ptr[i] : (int)x2fptr[i]);
           // y2 = (y2fptr==NULL ? y2ptr[i] : (int)y2fptr[i]);
           // x3 = (x3fptr==NULL ? x3ptr[i] : (int)x3fptr[i]);
           // y3 = (y3fptr==NULL ? y3ptr[i] : (int)y3fptr[i]);
            x1 = (x1fptr == NULL ? STRIDE_INT(x1ptr, i, x1stride) : (int)STRIDE_FLOAT(x1fptr, i, x1stride));
            y1 = (y1fptr == NULL ? STRIDE_INT(y1ptr, i, y1stride) : (int)STRIDE_FLOAT(y1fptr, i, y1stride));
            x2 = (x2fptr == NULL ? STRIDE_INT(x2ptr, i, x2stride) : (int)STRIDE_FLOAT(x2fptr, i, x2stride));
            y2 = (y2fptr == NULL ? STRIDE_INT(y2ptr, i, y2stride) : (int)STRIDE_FLOAT(y2fptr, i, y2stride));
            x3 = (x3fptr == NULL ? STRIDE_INT(x3ptr, i, x3stride) : (int)STRIDE_FLOAT(x3fptr, i, x3stride));
            y3 = (y3fptr == NULL ? STRIDE_INT(y3ptr, i, y3stride) : (int)STRIDE_FLOAT(y3fptr, i, y3stride));
            if(x1==x2 && x1==x3 && y1==y2 && y1==y3 && x1==-1 && y1==-1)return;
            if(nc > 1) {
            	if(CMM1){
            		c = (cfptr == NULL ? colourmap[STRIDE_INT(cptr, i, cstride)] :  colourmap[(int)STRIDE_FLOAT(cfptr, i, cstride)]);
            		//c = (cfptr == NULL ? colourmap[cptr[i]] : colourmap[(int)cfptr[i]]);
            	} else {
            		c = (cfptr == NULL ? STRIDE_INT(cptr, i, cstride) : (int)STRIDE_FLOAT(cfptr, i, cstride));
            		//c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
            	}
            }
            if(nf > 1) {
            	if(CMM1){
            		f = (ffptr == NULL ? colourmap[STRIDE_INT(fptr, i, fstride)] : colourmap[(int)STRIDE_FLOAT(ffptr, i, fstride)]);
            		//f = (ffptr == NULL ? colourmap[fptr[i]] : colourmap[(int)ffptr[i]]);
            	} else {
            		f = (ffptr == NULL ? STRIDE_INT(fptr, i, fstride) : (int)STRIDE_FLOAT(ffptr, i, fstride));
            		//f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
            	}
            }
            DrawTriangle(x1, y1, x2, y2, x3, y3, c, f);
        }
    }
}
void cmd_cls(void) {
	getargs(&cmdline,5,",");
	if(argc>1){
        DrawPixel(getinteger(argv[0]), getinteger(argv[2]), (argc == 5 ? getColour(argv[4], 0): gui_fcolour));
	} else {
		skipspace(cmdline);
		CurrentX = CurrentY = 0;
		if(!(*cmdline == 0 || *cmdline == '\'')){
			uint32_t i=getColour(cmdline, 0);
			ClearScreen(i);
		} else ClearScreen(gui_bcolour);
		if(WritePage==0)SerUSBPutS("\0337\033[2J\033[H");									// vt100 clear screen and home cursor
	}
}



void fun_rgb(void) {
    getargs(&ep, 7, ",");
    if(argc == 5){
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255), getint(argv[4], 0, 255), 15);
    } else if(argc == 1) {
        if(checkstring(argv[0], "WHITE"))        iret = WHITE;
        else if(checkstring(argv[0], "BLACK"))   iret = BLACK;
        else if(checkstring(argv[0], "NOTBLACK"))   iret = NOTBLACK;
        else if(checkstring(argv[0], "BLUE"))    iret = BLUE;
        else if(checkstring(argv[0], "GREEN"))   iret = GREEN;
        else if(checkstring(argv[0], "CYAN"))    iret = CYAN;
        else if(checkstring(argv[0], "RED"))     iret = RED;
        else if(checkstring(argv[0], "MAGENTA")) iret = MAGENTA;
        else if(checkstring(argv[0], "YELLOW"))  iret = YELLOW;
        else if(checkstring(argv[0], "BROWN"))   iret = BROWN;
        else if(checkstring(argv[0], "GRAY"))    iret = GRAY;
        else if(checkstring(argv[0], "GREY"))    iret = GRAY;
        else if(checkstring(argv[0], "LIGHTGRAY"))    iret = LITEGRAY;
        else if(checkstring(argv[0], "LIGHTGREY"))    iret = LITEGRAY;
        else if(checkstring(argv[0], "ORANGE"))    iret = ORANGE;
        else if(checkstring(argv[0], "PINK"))    iret = PINK;
        else if(checkstring(argv[0], "GOLD"))    iret = GOLD;
        else if(checkstring(argv[0], "SALMON"))    iret = SALMON;
        else if(checkstring(argv[0], "LILAC"))    iret = LILAC;
        else if(checkstring(argv[0], "FUCHSIA"))    iret = FUCHSIA;
        else if(checkstring(argv[0], "RUST"))    iret = RUST;
        else if(checkstring(argv[0], "CERULEAN"))    iret = CERULEAN;
        else if(checkstring(argv[0], "MIDGREEN"))    iret = MIDGREEN;
        else if(checkstring(argv[0], "COBALT"))    iret = COBALT;
        else if(checkstring(argv[0], "MYRTLE"))    iret = MYRTLE;
        else if(checkstring(argv[0], "BEIGE"))    iret = BEIGE;
        else error("Invalid colour: $", argv[0]);
        if(VideoColour!=32)iret &= 0xFFFFFFF;
    } else if(argc == 3) {
    	if(VideoColour==8 || VideoColour==16)error("Transparency not valid for this mode");
        if(checkstring(argv[0], "WHITE"))        iret = WHITE;
        else if(checkstring(argv[0], "BLACK"))   iret = BLACK;
        else if(checkstring(argv[0], "NOTBLACK"))   iret = NOTBLACK;
        else if(checkstring(argv[0], "BLUE"))    iret = BLUE;
        else if(checkstring(argv[0], "GREEN"))   iret = GREEN;
        else if(checkstring(argv[0], "CYAN"))    iret = CYAN;
        else if(checkstring(argv[0], "RED"))     iret = RED;
        else if(checkstring(argv[0], "MAGENTA")) iret = MAGENTA;
        else if(checkstring(argv[0], "YELLOW"))  iret = YELLOW;
        else if(checkstring(argv[0], "BROWN"))   iret = BROWN;
        else if(checkstring(argv[0], "GRAY"))    iret = GRAY;
        else if(checkstring(argv[0], "GREY"))    iret = GRAY;
        else if(checkstring(argv[0], "LIGHTGREY"))    iret = LITEGRAY;
        else if(checkstring(argv[0], "LIGHTGRAY"))    iret = LITEGRAY;
        else if(checkstring(argv[0], "ORANGE"))    iret = ORANGE;
        else if(checkstring(argv[0], "PINK"))    iret = PINK;
        else if(checkstring(argv[0], "GOLD"))    iret = GOLD;
        else if(checkstring(argv[0], "SALMON"))    iret = SALMON;
        else if(checkstring(argv[0], "LILAC"))    iret = LILAC;
        else if(checkstring(argv[0], "FUCHSIA"))    iret = FUCHSIA;
        else if(checkstring(argv[0], "RUST"))    iret = RUST;
        else if(checkstring(argv[0], "CERULEAN"))    iret = CERULEAN;
        else if(checkstring(argv[0], "MIDGREEN"))    iret = MIDGREEN;
        else if(checkstring(argv[0], "COBALT"))    iret = COBALT;
        else if(checkstring(argv[0], "MYRTLE"))    iret = MYRTLE;
        else if(checkstring(argv[0], "BEIGE"))    iret = BEIGE;
        else error("Invalid colour: $", argv[0]);
        iret &= 0xFFFFFF;
        if(VideoColour==12)iret |= (getint(argv[2], 0, 15)<<24);
        if(VideoColour==32)iret |= (getint(argv[2], 0, 255)<<24);
    } else if(argc == 7) {
    	if(VideoColour==8 || VideoColour==16)error("Transparency not valid for this mode");
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255), getint(argv[4], 0, 255), (VideoColour==12 ? getint(argv[6], 0, 15) : getint(argv[6], 0, 255)));
    } else
        error("Syntax");
    targ = T_INT;
}



void fun_mmhres(void) {
    iret = PageTable[WritePage].xmax;
    targ = T_INT;
}



void fun_mmvres(void) {
    iret = PageTable[WritePage].ymax;
    targ = T_INT;
}



void cmd_font(void) {
    getargs(&cmdline, 3, ",");
	int maxH=PageTable[WritePage].ymax;
    if(argc < 1) error("Argument count");
    if((OptionConsole & 2) && !CurrentLinePtr && Option.showstatus) {                 // if we are at the command prompt on the LCD
    	ClearScreen(gui_bcolour);
    	CurrentX = CurrentY = 0;
    }
    if(*argv[0] == '#') ++argv[0];
    if(argc == 3)
        SetFont(((getint(argv[0], 1, FONT_TABLE_SIZE) - 1) << 4) | getint(argv[2], 1, 15));
    else
        SetFont(((getint(argv[0], 1, FONT_TABLE_SIZE) - 1) << 4) | 1);
    if((OptionConsole & 2) && !CurrentLinePtr) {                 // if we are at the command prompt on the LCD
        PromptFont = gui_font;
        if(CurrentY + gui_font_height >= maxH) {
            ScrollLCD(CurrentY + gui_font_height - maxH, 1);           // scroll up if the font change split the line over the bottom
            CurrentY -= (CurrentY + gui_font_height - maxH);
        }
    }
}



void cmd_colour(void) {
    getargs(&cmdline, 3, ",");
    if(argc < 1) error("Argument count");
    gui_fcolour = getColour(argv[0], 0);
    if(argc == 3)
        gui_bcolour = getColour(argv[2], 0);
    if(!CurrentLinePtr) {
        PromptFC = gui_fcolour;
        PromptBC = gui_bcolour;
    }
}


/*void fun_mmcharwidth(void) {
    iret = FontTable[gui_font >> 4][0] * (gui_font & 0b1111);
    targ = T_INT;
}


void fun_mmcharheight(void) {
    iret = FontTable[gui_font >> 4][1] * (gui_font & 0b1111);
    targ = T_INT;
}*/






/****************************************************************************************************

 General purpose drawing routines

****************************************************************************************************/


void ClearScreen(int c) {
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
	DrawRectangle(0, 0, maxW - 1, maxH - 1, c);
}


/**************************************************************************************************
Draw a line on a the video output
	x1, y1 - the start coordinate
	x2, y2 - the end coordinate
    w - the width of the line (ignored for diagional lines)
	c - the colour to use
***************************************************************************************************/
//#define abs( a)     (((a)> 0) ? (a) : -(a))
#define ABS(X) ((X)>0 ? (X) : (-(X)))

void DrawLine(int x1, int y1, int x2, int y2, int w, int c) {
	int Colour;
	if(VideoColour==32){
		Colour=c;
	} else if(VideoColour<=8){
		Colour = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
	} else {
		if(VideoColour==16){
			Colour=
			(BIT_5[((c & 0xFF0000)>>19)]<<11) |
			(BIT_6[((c & 0xFF00)>>10)]<<5) |
			(BIT_5[((c & 0xFF)>>3)]);
		} else {
			Colour =
			(BIT_4[((c & 0xFF0000)>>20)]<<8) |
			(BIT_4[((c & 0xFF00)>>12)]<<4) |
			(BIT_4[((c & 0xFF)>>4)]) |
			((c & 0xF000000)>>12);
		}
	}

    if(y1 == y2 && w>0) {
    	for(int y=0;y<w;y++) DrawHLineFast(x1, y1+y, x2, Colour);                   // horiz line
        return;
    }
    if(x1 == x2 && w>0) {
    	if(w==1){
    		if(y2<y1)swap(y2,y1);
    		for(int y=y1;y<=y2;y++) DrawPixelFast(x1, y, Colour);
    	} else  DrawRectangle(x1, y1, x2 + w - 1, y2, c);                   // vert line
        return;
    }
	// uses a variant of Bresenham's line algorithm:
	//   https://en.wikipedia.org/wiki/Talk:Bresenham%27s_line_algorithm


    if(w==1 || w==-1){
        int  dx, dy, sx, sy, err, e2;
        dx = abs(x2 - x1); sx = x1 < x2 ? 1 : -1;
        dy = -abs(y2 - y1); sy = y1 < y2 ? 1 : -1;
        err = dx + dy;
        while(1) {
        	DrawPixelFast(x1, y1, Colour);
            e2 = 2 * err;
            if (e2 >= dy) {
                if (x1 == x2) break;
                err += dy; x1 += sx;
            }
            if (e2 <= dx) {
                if (y1 == y2) break;
                err += dx; y1 += sy;
            }
        }
    } else {
		float start,end;
		if(w<0){
			w=abs(w);
			start=-(w / 2.0f);
			end=w / 2.0f;
		} else {
			start=0.0f;
			end=w;
		}
		// Calculate the line direction and length
		float dx = x2 - x1;
		float dy = y2 - y1;
		float length = sqrtf(dx * dx + dy * dy);

		// Normalize direction vector
		float nx = dx / length;
		float ny = dy / length;

		// Calculate the perpendicular vector for width
		float px = -ny;
		float py = nx;

		// Half-width adjustment

		// Loop through every pixel inside the bounding rectangle of the line
		for (int i = 0; i <= length; i++) {
			float lineX = x1 + i * nx;
			float lineY = y1 + i * ny;

			for (float j = start; j <= end; j += 0.25f) { // Finer granularity
				float pixelX = lineX + j * px;
				float pixelY = lineY + j * py;

				DrawPixelFast(roundf(pixelX), roundf(pixelY), Colour);
			}
		}
    }
}

void CalcLine(int x1, int y1, int x2, int y2, short *xmin, short *xmax) {

    if(y1 == y2) {
    	if(y1<0)y1=0;
    	if(y1>=1080)y1=1079;
    	if(y2<0)y2=0;
    	if(y2>=1080)y2=1079;
		if(x1<xmin[y1])xmin[y1]=x1;
		if(x2<xmin[y1])xmin[y1]=x2;
		if(x1>xmax[y1])xmax[y1]=x1;
		if(x2>xmax[y1])xmax[y1]=x2;
        return;
    }
    if(x1 == x2) {
		if(y2<y1)swap(y2,y1);
    	if(y1<0)y1=0;
    	if(y1>=1080)y1=1079;
    	if(y2<0)y2=0;
    	if(y2>=1080)y2=1079;
		for(int y=y1;y<=y2;y++) {
			if(x1<xmin[y])xmin[y]=x1;
			if(x1>xmax[y])xmax[y]=x1;
		}
        return;
    }
	// uses a variant of Bresenham's line algorithm:
	//   https://en.wikipedia.org/wiki/Talk:Bresenham%27s_line_algorithm
	if (y1 > y2) {
		swap(y1, y2);
		swap(x1, x2);
	}
	if(y1<0)y1=0;
	if(y1>=1080)y1=1079;
	if(y2<0)y2=0;
	if(y2>=1080)y2=1079;
	int absX = ABS(x1-x2);          // absolute value of coordinate distances
	int absY = ABS(y1-y2);
	int offX = x2<x1 ? 1 : -1;      // line-drawing direction offsets
	int offY = y2<y1 ? 1 : -1;
	int x = x2;                     // incremental location
	int y = y2;
	int err;
	if(x<xmin[y])xmin[y]=x;
	if(x>xmax[y])xmax[y]=x;
	if (absX > absY) {

		// line is more horizontal; increment along x-axis
		err = absX / 2;
		while (x != x1) {
			err = err - absY;
			if (err < 0) {
				y   += offY;
				err += absX;
			}
			x += offX;
    		if(x<xmin[y])xmin[y]=x;
    		if(x>xmax[y])xmax[y]=x;
		}
	} else {

		// line is more vertical; increment along y-axis
		err = absY / 2;
		while (y != y1) {
			err = err - absX;
			if (err < 0) {
				x   += offX;
				err += absY;
			}
			y += offY;
    		if(x<xmin[y])xmin[y]=x;
    		if(x>xmax[y])xmax[y]=x;
		}
	}
}



/**********************************************************************************************
Draw a box
     x1, y1 - the start coordinate
     x2, y2 - the end coordinate
     w      - the width of the sides of the box (can be zero)
     c      - the colour to use for sides of the box
     fill   - the colour to fill the box (-1 for no fill)
***********************************************************************************************/
void DrawBox(int x1, int y1, int x2, int y2, int w, int c, int fill) {
    int t;

    // make sure the coordinates are in the right sequence
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(w > x2 - x1) w = x2 - x1;
    if(w > y2 - y1) w = y2 - y1;

    if(w > 0 && fill!=c) {
        w--;
        DrawRectangle(x1, y1, x2, y1 + w, c);                       // Draw the top horiz line
        DrawRectangle(x1, y2 - w, x2, y2, c);                       // Draw the bottom horiz line
        DrawRectangle(x1, y1, x1 + w, y2, c);                       // Draw the left vert line
        DrawRectangle(x2 - w, y1, x2, y2, c);                       // Draw the right vert line
        w++;
    }
    if(fill==c)w=0;
    if(fill >= 0) DrawRectangle(x1 + w, y1 + w, x2 - w, y2 - w, fill);
}



/**********************************************************************************************
Draw a box with rounded corners
     x1, y1 - the start coordinate
     x2, y2 - the end coordinate
     radius - the radius (in pixels) of the arc forming the corners
     c      - the colour to use for sides
     fill   - the colour to fill the box (-1 for no fill)
***********************************************************************************************/
void DrawRBox(int x1, int y1, int x2, int y2, int radius, int c, int fill) {
    int f, ddF_x, ddF_y, xx, yy;
    int t;

    // make sure the coordinates are in the right sequence
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }

    f = 1 - radius;
    ddF_x = 1;
    ddF_y = -2 * radius;
    xx = 0;
    yy = radius;

    while(xx < yy) {
        if(f >= 0) {
            yy-=1;
            ddF_y += 2;
            f += ddF_y;
        }
        xx+=1;
        ddF_x += 2;
        f += ddF_x  ;
        DrawPixel(x2 + xx - radius, y2 + yy - radius, c);           // Bottom Right Corner
        DrawPixel(x2 + yy - radius, y2 + xx - radius, c);           // ^^^
        DrawPixel(x1 - xx + radius, y2 + yy - radius, c);           // Bottom Left Corner
        DrawPixel(x1 - yy + radius, y2 + xx - radius, c);           // ^^^

        DrawPixel(x2 + xx - radius, y1 - yy + radius, c);           // Top Right Corner
        DrawPixel(x2 + yy - radius, y1 - xx + radius, c);           // ^^^
        DrawPixel(x1 - xx + radius, y1 - yy + radius, c);           // Top Left Corner
        DrawPixel(x1 - yy + radius, y1 - xx + radius, c);           // ^^^
        if(fill >= 0) {
            DrawLine(x2 + xx - radius - 1, y2 + yy - radius, x1 - xx + radius + 1, y2 + yy - radius, 1, fill);
            DrawLine(x2 + yy - radius - 1, y2 + xx - radius, x1 - yy + radius + 1, y2 + xx - radius, 1, fill);
            DrawLine(x2 + xx - radius - 1, y1 - yy + radius, x1 - xx + radius + 1, y1 - yy + radius, 1, fill);
            DrawLine(x2 + yy - radius - 1, y1 - xx + radius, x1 - yy + radius + 1, y1 - xx + radius, 1, fill);
        }
    }
    if(fill >= 0) DrawRectangle(x1 + 1, y1 + radius, x2 - 1, y2 - radius, fill);
    DrawRectangle(x1 + radius - 1, y1, x2 - radius + 1, y1, c);     // top side
    DrawRectangle(x1 + radius - 1, y2,  x2 - radius + 1, y2, c);    // botom side
    DrawRectangle(x1, y1 + radius, x1, y2 - radius, c);             // left side
    DrawRectangle(x2, y1 + radius, x2, y2 - radius, c);             // right side
}




/***********************************************************************************************
Draw a circle on the video output
	x, y - the center of the circle
	radius - the radius of the circle
    w - width of the line drawing the circle
	c - the colour to use for the circle
	fill - the colour to use for the fill or -1 if no fill
	aspect - the ration of the x and y axis (a MMFLOAT).  1.0 gives a prefect circle
***********************************************************************************************/
void DrawCircle(int x, int y, int radius, int w, int c, int fill, MMFLOAT aspect) {
   int a, b, P;
   int A, B;
   int asp;
   MMFLOAT aspect2;
   if(w>1){
	   if(fill>=0){ // thick border with filled centre
		   DrawCircle(x,y,radius,0,c,c,aspect);
		    aspect2=((aspect*(MMFLOAT)radius)-(MMFLOAT)w)/((MMFLOAT)(radius-w));
		   DrawCircle(x,y,radius-w,0,fill,fill,aspect2);
	   } else { //thick border with empty centre
		   	int r1=radius-w,r2=radius, xs=-1,xi=0, i,j,k,m, ll=radius;
		    if(aspect>1.0)ll=(int)((MMFLOAT)radius*aspect);
		    int ints_per_line=RoundUptoInt((ll*2)+1)/32;
		    uint32_t *br=(uint32_t *)GetTempMemory(((ints_per_line+1)*((r2*2)+1))*4);
		    DrawFilledCircle(x, y, r2, r2, 1, ints_per_line, br, aspect, aspect);
		    aspect2=((aspect*(MMFLOAT)r2)-(MMFLOAT)w)/((MMFLOAT)r1);
		    DrawFilledCircle(x, y, r1, r2, 0, ints_per_line, br, aspect, aspect2);
		    x=(int)((MMFLOAT)x+(MMFLOAT)r2*(1.0-aspect));
		 	for(j=0;j<r2*2+1;j++){
		 		for(i=0;i<ints_per_line;i++){
		 			k=br[i+j*ints_per_line];
		 			for(m=0;m<32;m++){
		 				if(xs==-1 && (k & 0x80000000)){
		 					xs=m;
		 					xi=i;
		 				}
		 				if(xs!=-1 && !(k & 0x80000000)){
							DrawRectangle(x-r2+xs+xi*32, y-r2+j, x-r2+m+i*32, y-r2+j, c);
		 					xs=-1;
		 				}
		 				k<<=1;
		 			}
		 		}
				if(xs!=-1){
					DrawRectangle(x-r2+xs+xi*32, y-r2+j, x-r2+m+i*32, y-r2+j, c);
					xs=-1;
				}
			}
	   }

   } else { //single thickness outline
	   int w1=w,r1=radius;
	   if(fill>=0){
		   while(w >= 0 && radius > 0) {
		       a = 0;
		       b = radius;
		       P = 1 - radius;
		       asp = aspect * (MMFLOAT)(1 << 10);

		       do {
		         A = (a * asp) >> 10;
		         B = (b * asp) >> 10;
		         if(fill >= 0 && w >= 0) {
		             DrawRectangle(x-A, y+b, x+A, y+b, fill);
		             DrawRectangle(x-A, y-b, x+A, y-b, fill);
		             DrawRectangle(x-B, y+a, x+B, y+a, fill);
		             DrawRectangle(x-B, y-a, x+B, y-a, fill);
		         }
		          if(P < 0)
		             P+= 3 + 2*a++;
		          else
		             P+= 5 + 2*(a++ - b--);

		        } while(a <= b);
		        w--;
		        radius--;
		   }
	   }
	   if(c!=fill){
		   w=w1; radius=r1;
		   while(w >= 0 && radius > 0) {
		       a = 0;
		       b = radius;
		       P = 1 - radius;
		       asp = aspect * (MMFLOAT)(1 << 10);
		       do {
		         A = (a * asp) >> 10;
		         B = (b * asp) >> 10;
		         if(w) {
		             DrawPixel(A+x, b+y, c);
		             DrawPixel(B+x, a+y, c);
		             DrawPixel(x-A, b+y, c);
		             DrawPixel(x-B, a+y, c);
		             DrawPixel(B+x, y-a, c);
		             DrawPixel(A+x, y-b, c);
		             DrawPixel(x-A, y-b, c);
		             DrawPixel(x-B, y-a, c);
		         }
		          if(P < 0)
		             P+= 3 + 2*a++;
		          else
		             P+= 5 + 2*(a++ - b--);

		        } while(a <= b);
		        w--;
		        radius--;
		   }
	   }
   }
}
#define RoundUptoInt(a)     (((a) + (32 - 1)) & (~(32 - 1)))// round up to the nearest whole integer

void hline(int x0, int x1, int y, int f, int ints_per_line, uint32_t *br) { //draw a horizontal line
    uint32_t w1, xx1, w0, xx0, x, xn, i;
    const uint32_t a[]={0xFFFFFFFF,0x7FFFFFFF,0x3FFFFFFF,0x1FFFFFFF,0xFFFFFFF,0x7FFFFFF,0x3FFFFFF,0x1FFFFFF,
                        0xFFFFFF,0x7FFFFF,0x3FFFFF,0x1FFFFF,0xFFFFF,0x7FFFF,0x3FFFF,0x1FFFF,
                        0xFFFF,0x7FFF,0x3FFF,0x1FFF,0xFFF,0x7FF,0x3FF,0x1FF,
                        0xFF,0x7F,0x3F,0x1F,0x0F,0x07,0x03,0x01};
    const uint32_t b[]={0x80000000,0xC0000000,0xe0000000,0xf0000000,0xf8000000,0xfc000000,0xfe000000,0xff000000,
                        0xff800000,0xffC00000,0xffe00000,0xfff00000,0xfff80000,0xfffc0000,0xfffe0000,0xffff0000,
                        0xffff8000,0xffffC000,0xffffe000,0xfffff000,0xfffff800,0xfffffc00,0xfffffe00,0xffffff00,
                        0xffffff80,0xffffffC0,0xffffffe0,0xfffffff0,0xfffffff8,0xfffffffc,0xfffffffe,0xffffffff};
    w0 = y * (ints_per_line);
    xx0 = 0;
    w1 = y * (ints_per_line) + x1/32;
    xx1 = (x1 & 0x1F);
    w0 = y * (ints_per_line) + x0/32;
    xx0 = (x0 & 0x1F);
    w1 = y * (ints_per_line) + x1/32;
    xx1 = (x1 & 0x1F);
    if(w1==w0){ //special case both inside same word
        x=(a[xx0] & b[xx1]);
        xn=~x;
        if(f)br[w0] |= x; else  br[w0] &= xn;                   // turn on the pixel
    } else {
        if(w1-w0>1){ //first deal with full words
            for(i=w0+1;i<w1;i++){
            // draw the pixel
            	br[i]=0;
                if(f)br[i] = 0xFFFFFFFF;          // turn on the pixels
            }
        }
        x=~a[xx0];
        br[w0] &= x;
        x=~x;
        if(f)br[w0] |= x;                         // turn on the pixel
        x=~b[xx1];
        br[w1] &= x;
        x=~x;
        if(f)br[w1] |= x;                         // turn on the pixel
    }
}

void DrawFilledCircle(int x, int y, int radius, int r, int fill, int ints_per_line, uint32_t *br, MMFLOAT aspect, MMFLOAT aspect2) {
   int a, b, P;
   int A, B, asp;
   	   x=(int)((MMFLOAT)r*aspect)+radius;
   	   y=r+radius;
       a = 0;
       b = radius;
       P = 1 - radius;
       asp = aspect2 * (MMFLOAT)(1 << 10);
       do {
	         A = (a * asp) >> 10;
	         B = (b * asp) >> 10;
	         hline(x-A-radius, x+A-radius,  y+b-radius, fill, ints_per_line, br);
	         hline(x-A-radius, x+A-radius,  y-b-radius, fill, ints_per_line, br);
	         hline(x-B-radius, x+B-radius,  y+a-radius, fill, ints_per_line, br);
	         hline(x-B-radius, x+B-radius,  y-a-radius, fill, ints_per_line, br);
	         if(P < 0)
	            P+= 3 + 2*a++;
	          else
	            P+= 5 + 2*(a++ - b--);

        } while(a <= b);
}
void ClearTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int ints_per_line, uint32_t *br) {
    if (x0 * (y1 - y2) + x1 * (y2 - y0) + x2 * (y0 - y1) == 0)return;

        long a, b, y, last;
        long  dx01,  dy01,  dx02,  dy02, dx12,  dy12,  sa, sb;

        if (y0 > y1) {
            swap(y0, y1);
            swap(x0, x1);
        }
        if (y1 > y2) {
            swap(y2, y1);
            swap(x2, x1);
        }
        if (y0 > y1) {
            swap(y0, y1);
            swap(x0, x1);
        }

            dx01 = x1 - x0;  dy01 = y1 - y0;  dx02 = x2 - x0;
            dy02 = y2 - y0; dx12 = x2 - x1;  dy12 = y2 - y1;
            sa = 0; sb = 0;
            if(y1 == y2) {
                last = y1;                                          //Include y1 scanline
            } else {
                last = y1 - 1;                                      // Skip it
            }
            for (y = y0; y <= last; y++){
                a = x0 + sa / dy01;
                b = x0 + sb / dy02;
                sa = sa + dx01;
                sb = sb + dx02;
                a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                if(a > b)swap(a, b);
                hline(a, b,  y, 0, ints_per_line, br);
            }
            sa = dx12 * (y - y1);
            sb = dx02 * ( y- y0);
            while (y <= y2){
                a = x1 + sa / dy12;
                b = x0 + sb / dy02;
                sa = sa + dx12;
                sb = sb + dx02;
                a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                if(a > b) swap(a, b);
                hline(a, b,  y, 0, ints_per_line, br);
                y = y + 1;
            }
}
/**********************************************************************************************
Draw a triangle
    Thanks to Peter Mather (matherp on the Back Shed forum)
     x0, y0 - the first corner
     x1, y1 - the second corner
     x2, y2 - the third corner
     c      - the colour to use for sides of the triangle
     fill   - the colour to fill the triangle (-1 for no fill)
***********************************************************************************************/
/*void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int f) {
	if (y0 > y1) {
		swap(y0, y1);
		swap(x0, x1);
	}
	if (y1 > y2) {
		swap(y2, y1);
		swap(x2, x1);
	}
	if (y0 > y1) {
		swap(y0, y1);
		swap(x0, x1);
	}
	if(x0 * (y1 - y2) +  x1 * (y2 - y0) +  x2 * (y0 - y1)==0){ // points are co-linear i.e zero area
		DrawLine(x0,y0,x2,y2,1,c);
	} else {
		if(f == -1){
			// draw only the outline
			DrawLine(x0, y0, x1, y1, 1, c);
			DrawLine(x1, y1, x2, y2, 1, c);
			DrawLine(x2, y2, x0, y0, 1, c);
		} else {
			int Colour;
			if(VideoColour==32){
				Colour=f;
			} else if(VideoColour<=8){
				Colour = ((f & 0b111000000000000000000000)>>16) | ((f & 0b1110000000000000)>>11) | ((f & 0b11000000)>>6);
			} else {
				if(VideoColour==16){
					Colour=
					(BIT_5[((f & 0xFF0000)>>19)]<<11) |
					(BIT_6[((f & 0xFF00)>>10)]<<5) |
					(BIT_5[((f & 0xFF)>>3)]);
				} else {
					Colour =
					(BIT_4[((f & 0xFF0000)>>20)]<<8) |
					(BIT_4[((f & 0xFF00)>>12)]<<4) |
					(BIT_4[((f & 0xFF)>>4)]) |
					((f & 0xF000000)>>12);
				}
			}


			//we are drawing a filled triangle which may also have an outline
			int  a, b, y, last;
            if(y1 == y2) {
                last = y1;                                          //Include y1 scanline
            } else {
                last = y1 - 1;                                      // Skip it
            }
            for (y = y0; y <= last; y++){
                a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                DrawHLineFast(a, y, b, Colour);
            }
            while (y <= y2){
                a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                DrawHLineFast(a, y, b, Colour);
                y = y + 1;
            }
            // we also need an outline but we do this last to overwrite the edge of the fill area
            if(c!=f){
				DrawLine(x0, y0, x1, y1, 1, c);
				DrawLine(x1, y1, x2, y2, 1, c);
				DrawLine(x2, y2, x0, y0, 1, c);
            }
        }
	}
}*/
void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int f) {
	if(x0 * (y1 - y2) +  x1 * (y2 - y0) +  x2 * (y0 - y1)==0){ // points are co-linear i.e zero area
		if (y0 > y1) {
			swap(y0, y1);
			swap(x0, x1);
		}
		if (y1 > y2) {
			swap(y2, y1);
			swap(x2, x1);
		}
		if (y0 > y1) {
			swap(y0, y1);
			swap(x0, x1);
		}
		DrawLine(x0,y0,x2,y2,1,c);
	} else {
		if(f == -1){
			// draw only the outline
			DrawLine(x0, y0, x1, y1, 1, c);
			DrawLine(x1, y1, x2, y2, 1, c);
			DrawLine(x2, y2, x0, y0, 1, c);
		} else {
			if (y0 > y1) {
				swap(y0, y1);
				swap(x0, x1);
			}
			if (y1 > y2) {
				swap(y2, y1);
				swap(x2, x1);
			}
			if (y0 > y1) {
				swap(y0, y1);
				swap(x0, x1);
			}
			short *xmin=(short *)linebuff;
			short *xmax=(short *)(linebuff+2160); //max number of lines is 1080

			int y;
			int Colour;
			if(VideoColour==32){
				Colour=f;
			} else if(VideoColour<=8){
				Colour = ((f & 0b111000000000000000000000)>>16) | ((f & 0b1110000000000000)>>11) | ((f & 0b11000000)>>6);
			} else {
				if(VideoColour==16){
					Colour=
					(BIT_5[((f & 0xFF0000)>>19)]<<11) |
					(BIT_6[((f & 0xFF00)>>10)]<<5) |
					(BIT_5[((f & 0xFF)>>3)]);
				} else {
					Colour =
					(BIT_4[((f & 0xFF0000)>>20)]<<8) |
					(BIT_4[((f & 0xFF00)>>12)]<<4) |
					(BIT_4[((f & 0xFF)>>4)]) |
					((f & 0xF000000)>>12);
				}
			}
			for(y=y0; y<=y2; y++){
				if(y>=0 && y<1080){
					xmin[y]=32767;
					xmax[y]=-1;
				}
			}
			CalcLine(x0, y0, x1, y1, xmin, xmax);
			CalcLine(x1, y1, x2, y2, xmin, xmax);
			CalcLine(x2, y2, x0, y0, xmin, xmax);
			for(y=y0;y<=y2;y++){
				if(y>=0 && y<1080)DrawHLineFast(xmin[y], y, xmax[y], Colour);
			}
            if(c!=f){
				DrawLine(x0, y0, x1, y1, 1, c);
				DrawLine(x1, y1, x2, y2, 1, c);
				DrawLine(x2, y2, x0, y0, 1, c);
            }
		}
	}

}
// draw a polygon from a 3D object
// parameters are the object and the face to draw
void DrawPolygon(int n, short *xcoord, short *ycoord, int face){
	int i, facecount=struct3d[n]->facecount[face];
    int c=struct3d[n]->line[face];
    int f=struct3d[n]->fill[face];
	// first deal with outline only
	if(struct3d[n]->fill[face]==0xFFFFFFFF){
	    for(i=0;i<facecount;i++){
       		if(i<facecount-1){
       			DrawLine(xcoord[i],ycoord[i],xcoord[i+1],ycoord[i+1],1,c);
       		} else {
       			DrawLine(xcoord[i],ycoord[i],xcoord[0],ycoord[0],1,c);
       		}
	    }
	} else {
		if(facecount==3){
			DrawTriangle(xcoord[0],ycoord[0],xcoord[1],ycoord[1],xcoord[2],ycoord[2],c,f);
		} else if(facecount==4){
			DrawTriangle(xcoord[0],ycoord[0],xcoord[1],ycoord[1],xcoord[2],ycoord[2],f,f);
			DrawTriangle(xcoord[0],ycoord[0],xcoord[2],ycoord[2],xcoord[3],ycoord[3],f,f);
			if(f!=c){
				DrawLine(xcoord[0],ycoord[0],xcoord[1],ycoord[1],1,c);
				DrawLine(xcoord[1],ycoord[1],xcoord[2],ycoord[2],1,c);
				DrawLine(xcoord[2],ycoord[2],xcoord[3],ycoord[3],1,c);
				DrawLine(xcoord[0],ycoord[0],xcoord[3],ycoord[3],1,c);
			}
		} else {
			int  ymax=-1000000, ymin=1000000;
        	fill_set_pen_color((c>>16) & 0xFF, (c>>8) & 0xFF , c & 0xFF, (c>>24)&0xF);
    		fill_set_fill_color((f>>16) & 0xFF, (f>>8) & 0xFF , f & 0xFF, (f>>24)&0xF);
			main_fill_poly_vertex_count=0;
			for(i=0;i<facecount;i++){
				main_fill_polyX[main_fill_poly_vertex_count] = (TFLOAT)xcoord[i];
				main_fill_polyY[main_fill_poly_vertex_count] = (TFLOAT)ycoord[i];
				if(main_fill_polyY[main_fill_poly_vertex_count]>ymax)ymax=main_fill_polyY[main_fill_poly_vertex_count];
				if(main_fill_polyY[main_fill_poly_vertex_count]<ymin)ymin=main_fill_polyY[main_fill_poly_vertex_count];
				main_fill_polyX[main_fill_poly_vertex_count] = (TFLOAT)xcoord[i];
				main_fill_poly_vertex_count++;
			 }
			 if(main_fill_polyY[main_fill_poly_vertex_count]!=main_fill_polyY[0] || main_fill_polyX[main_fill_poly_vertex_count] != main_fill_polyX[0]){
					main_fill_polyX[main_fill_poly_vertex_count]=main_fill_polyX[0];
					main_fill_polyY[main_fill_poly_vertex_count]=main_fill_polyY[0];
					main_fill_poly_vertex_count++;
			 }
			 fill_fast_fill(main_fill_poly_vertex_count-1,ymin,ymax);
		}
	}

}
/******************************************************************************************
 Print a char on the LCD display
 Any characters not in the font will print as a space.
 The char is printed at the current location defined by CurrentX and CurrentY
*****************************************************************************************/
void GUIPrintChar(int fnt, int fc, int bc, char c, int orientation) {
    unsigned char *p, *fp, *np = NULL;
    int BitNumber, BitPos, x, y, newx, newy, modx, mody, scale = fnt & 0b1111;
    int height, width;
    int cursorhidden=0;
    if(PrintPixelMode==1)bc=-1;
    if(PrintPixelMode==2){
    	int s=bc;
    	bc=fc;
    	fc=s;
    }
    if(PrintPixelMode==5){
    	fc=bc;
    	bc=-1;
    }
    // to get the +, - and = chars for font 6 we fudge them by scaling up font 1
    if((fnt & 0xf0) == 0x50 && (c == '-' || c == '+' || c == '=')) {
        fp = (unsigned char *)FontTable[0];
        scale = scale * 4;
    } else
        fp = (unsigned char *)FontTable[fnt >> 4];

    height = fp[1];
    width = fp[0];
    modx = mody = 0;
    if(orientation > ORIENT_VERT){
        np = GetTempMemory(width * height);
        if (orientation == ORIENT_INVERTED) {
            modx -= width * scale -1;
            mody -= height * scale -1;
        }
        else if (orientation == ORIENT_CCW90DEG) {
            mody -= width * scale;
        }
        else if (orientation == ORIENT_CW90DEG){
            modx -= height * scale -1;
        }
    }

    if(c >= fp[2] && c < fp[2] + fp[3]) {
        p = fp + 4 + (int)(((c - fp[2]) * height * width) / 8);

        if(orientation > ORIENT_VERT) {                             // non-standard orientation
            if (orientation == ORIENT_INVERTED) {
                for(y = 0; y < height; y++) {
                    newy = height - y - 1;
                    for(x=0; x < width; x++) {
                        newx = width - x - 1;
                        if((p[((y * width) + x)/8] >> (((height * width) - ((y * width) + x) - 1) %8)) & 1) {
                            BitNumber=((newy * width) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            } else if (orientation == ORIENT_CCW90DEG) {
                for(y = 0; y < height; y++) {
                    newx = y;
                    for(x = 0; x < width; x++) {
                        newy = width - x - 1;
                        if((p[((y * width) + x)/8] >> (((height * width) - ((y * width) + x) - 1) %8)) & 1) {
                            BitNumber=((newy * height) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            } else if (orientation == ORIENT_CW90DEG) {
                for(y = 0; y < height; y++) {
                    newx = height - y - 1;
                    for(x=0; x < width; x++) {
                        newy = x;
                        if((p[((y * width) + x)/8] >> (((height * width) - ((y * width) + x) - 1) %8)) & 1) {
                            BitNumber=((newy * height) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            }
        }  else {np = p;}

		if(cursoron)
			if( !(xcursor + wcursor < CurrentX + modx ||
				xcursor > CurrentX + modx + width * scale ||
				ycursor + hcursor < CurrentY + mody ||
				ycursor > CurrentY + mody + height * scale)){
				hidecursor(0);
				cursorhidden=1;
			}

        if(orientation < ORIENT_CCW90DEG) DrawBitmap(CurrentX + modx, CurrentY + mody, width, height, scale, fc, bc, np);
        else DrawBitmap(CurrentX + modx, CurrentY + mody, height, width, scale, fc, bc, np);
    } else {
        if(orientation < ORIENT_CCW90DEG) DrawRectangle(CurrentX + modx, CurrentY + mody, CurrentX + modx + (width * scale), CurrentY + mody + (height * scale), bc);
        else DrawRectangle(CurrentX + modx, CurrentY + mody, CurrentX + modx + (height * scale), CurrentY + mody + (width * scale), bc);
    }

    // to get the . and degree symbols for font 6 we draw a small circle
    if((fnt & 0xf0) == 0x50) {
        if(orientation > ORIENT_VERT) {
            if(orientation == ORIENT_INVERTED) {
                if(c == '.') DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + 7 * scale, 4 * scale, 0, fc, fc, 1.0);
                if(c == 0x60) DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + (height * scale)- 9 * scale, 6 * scale, 2 * scale, fc, -1, 1.0);
            } else if(orientation == ORIENT_CCW90DEG) {
                if(c == '.') DrawCircle(CurrentX + modx + (height * scale) - 7 * scale, CurrentY + mody + (width * scale)/2, 4 * scale, 0, fc, fc, 1.0);
                if(c == 0x60) DrawCircle(CurrentX + modx + 9 * scale, CurrentY + mody + (width * scale)/2, 6 * scale, 2 * scale, fc, -1, 1.0);
            } else if(orientation == ORIENT_CW90DEG) {
                if(c == '.') DrawCircle(CurrentX + modx + 7 * scale, CurrentY + mody + (width * scale)/2, 4 * scale, 0, fc, fc, 1.0);
                if(c == 0x60) DrawCircle(CurrentX + modx + (height * scale)- 9 * scale, CurrentY + mody + (width * scale)/2, 6 * scale, 2 * scale, fc, -1, 1.0);
            }

        } else {
            if(c == '.') DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + (height * scale) - 7 * scale, 4 * scale, 0, fc, fc, 1.0);
            if(c == 0x60) DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + 9 * scale, 6 * scale, 2 * scale, fc, -1, 1.0);
        }
    }

    if(orientation == ORIENT_NORMAL) CurrentX += width * scale;
    else if (orientation == ORIENT_VERT) CurrentY += height * scale;
    else if (orientation == ORIENT_INVERTED) CurrentX -= width * scale;
    else if (orientation == ORIENT_CCW90DEG) CurrentY -= width * scale;
    else if (orientation == ORIENT_CW90DEG ) CurrentY += width * scale;
    if(cursorhidden)showcursor(0, xcursor, ycursor);
}


/******************************************************************************************
 Print a string on the LCD display
 The string must be a C string (not an MMBasic string)
 Any characters not in the font will print as a space.
*****************************************************************************************/
void GUIPrintString(int x, int y, int fnt, int jh, int jv, int jo, int fc, int bc, char *str) {

	CurrentX = x;  CurrentY = y;
	char *newstr=NULL;
	if(optiony){
		newstr=GetTempStrMemory();
		if(jo==ORIENT_VERT || jo == ORIENT_CCW90DEG || jo == ORIENT_CW90DEG){
			int i=strlen(str)-1;
			int j=0;
			while(i>=0){
				newstr[i]=str[j];
				i--;
				j++;
			}
		} else strcpy(newstr,str);
	}
    if(jo == ORIENT_NORMAL && optiony==0) {
        if(jh == JUSTIFY_CENTER) CurrentX -= (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -= (strlen(str) * GetFontWidth(fnt));
        if(jv == JUSTIFY_MIDDLE) CurrentY -= GetFontHeight(fnt) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= GetFontHeight(fnt);
    }
    else if(jo == ORIENT_NORMAL && optiony) {
        if(jh == JUSTIFY_CENTER) CurrentX -= (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -= (strlen(str) * GetFontWidth(fnt));
        if(jv == JUSTIFY_MIDDLE) CurrentY += GetFontHeight(fnt) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY += GetFontHeight(fnt);
    }
    else if(jo == ORIENT_VERT && optiony==0) {
        if(jh == JUSTIFY_CENTER) CurrentX -= GetFontWidth(fnt)/ 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -= GetFontWidth(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY -= (strlen(str) * GetFontHeight(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= (strlen(str) * GetFontHeight(fnt));
    }
    else if(jo == ORIENT_VERT && optiony) {
       	CurrentY-=GetFontHeight(fnt) * (strlen(str)-1);
        if(jh == JUSTIFY_CENTER) CurrentX -= GetFontWidth(fnt)/ 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -= GetFontWidth(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY += (strlen(str) * GetFontHeight(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY += (strlen(str) * GetFontHeight(fnt));
    }
    else if(jo == ORIENT_INVERTED && optiony==0) {
         if(jh == JUSTIFY_CENTER) CurrentX += (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX += (strlen(str) * GetFontWidth(fnt));
        if(jv == JUSTIFY_MIDDLE) CurrentY += GetFontHeight(fnt) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY += GetFontHeight(fnt);
    }
    else if(jo == ORIENT_INVERTED && optiony) {
        if(jh == JUSTIFY_CENTER) CurrentX += (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX += (strlen(str) * GetFontWidth(fnt));
        if(jv == JUSTIFY_MIDDLE) CurrentY -= GetFontHeight(fnt) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= GetFontHeight(fnt);
    }
    else if(jo == ORIENT_CCW90DEG  && optiony==0) {
        if(jh == JUSTIFY_CENTER) CurrentX -=  GetFontHeight(fnt) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -=  GetFontHeight(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY += (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY += (strlen(str) * GetFontWidth(fnt));
    }
    else if(jo == ORIENT_CCW90DEG  && optiony) {
       	CurrentY+=GetFontWidth(fnt)*(strlen(str)+1);
        if(jh == JUSTIFY_CENTER) CurrentX -=  GetFontHeight(fnt) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -=  GetFontHeight(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY -= (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= (strlen(str) * GetFontWidth(fnt));
    }
    else if(jo == ORIENT_CW90DEG && optiony==0) {
        if(jh == JUSTIFY_CENTER) CurrentX += GetFontHeight(fnt) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX += GetFontHeight(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY -= (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= (strlen(str) * GetFontWidth(fnt));
    }
    else if(jo == ORIENT_CW90DEG && optiony) {
       	CurrentY-=GetFontWidth(fnt)*(strlen(str)-1);
        if(jh == JUSTIFY_CENTER) CurrentX += GetFontHeight(fnt) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX += GetFontHeight(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY += (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY += (strlen(str) * GetFontWidth(fnt));
    }
    if(optiony) while(*newstr) {
        if(*newstr == 0xff && Ctrl[InvokingCtrl].type == 10) {
            newstr++;
            GUIPrintChar(fnt, bc, fc, *newstr++, jo);
        } else
            GUIPrintChar(fnt, fc, bc, *newstr++, jo);
    }
    else while(*str) {
        if(*str == 0xff && Ctrl[InvokingCtrl].type == 10) {
            str++;
            GUIPrintChar(fnt, bc, fc, *str++, jo);
        } else
            GUIPrintChar(fnt, fc, bc, *str++, jo);
    }
}


/****************************************************************************************************

 General purpose routines

****************************************************************************************************/



int rgb(int r, int g, int b, int t) {
    return RGB(r, g, b, t);
}


int GetFontWidth(int fnt) {
    return FontTable[fnt >> 4][0] * (fnt & 0b1111);
}


int GetFontHeight(int fnt) {
    return FontTable[fnt >> 4][1] * (fnt & 0b1111);
}


void SetFont(int fnt) {
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
    if(FontTable[fnt >> 4] == NULL) error("Invalid font number #%", (fnt >> 4)+1);
	ShowCursor(false);
    gui_font_width = FontTable[fnt >> 4][0] * (fnt & 0b1111);
    gui_font_height = FontTable[fnt >> 4][1] * (fnt & 0b1111);
    if(((fnt >> 4)+1)==7)gui_font_height=10 * (fnt & 0b1111);
    if((OptionConsole & 2)) {
        Option.Height = maxH/gui_font_height;
        Option.Width = maxW/gui_font_width;
    }
    gui_font = fnt;
}


void ResetDisplay(void) {
//    if(!(OptionConsole & 2)) {
        SetFont(Option.DefaultFont);
        gui_fcolour = Option.DefaultFC;
        gui_bcolour = Option.DefaultBC;
        ResetGUI();
//    }
}

