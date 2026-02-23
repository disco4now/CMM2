/***********************************************************************************************************************
MMBasic

OtherDisplays.c

Does the basic LCD display commands and drawing in MMBasic.

Copyright 2018 Peter Mather.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "array_utility.h"
#include "defaultfont.h"
extern SPI_HandleTypeDef SD_SPI;
int displayoffset=0;
#define timeoutvalue 20000000
int transmitOK(int timeout, int updown);
const uint8_t BIT_4low[16] = {
0,0,1,2,3,4,4,5,6,7,8,8,9,10,11,12  //4-bit
};
const uint8_t BIT_4medium[16] = {
0,1,2,2,3,4,5,6,7,8,9,10,11,12,13,14
};
const uint8_t BIT_4high[16] = {
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
};
//
const uint8_t BIT_5low[32] = {
		0,0,1,2,3,3,4,5,6,6,7,8,9,10,10,11,12,13,
		13,14,15,16,17,17,18,19,20,20,21,22,23,24
};
const uint8_t BIT_5medium[32] = {
		0,0,1,2,3,4,5,6,7,8,9,9,10,11,12,13,14,15,
		16,17,18,18,19,20,21,22,23,24,25,26,27,28
};
const uint8_t BIT_5high[32] = {
0,1,2,3,4,5,6,7,
8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,
24,25,26,27,28,29,30,31
};
//
const uint8_t BIT_6low[64] = {
		0,0,1,2,3,3,4,5,6,6,7,8,9,9,10,11,12,12,13,14,15,16,
		16,17,18,19,19,20,21,22,22,23,24,25,25,26,27,28,28,
		29,30,31,32,32,33,34,35,35,36,37,38,38,39,40,41,41,
		42,43,44,44,45,46,47,48
};
const uint8_t BIT_6medium[64] = {
		0,0,0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14,
		16,16,18,18,18,18,20,20,22,22,24,24,26,26,28,28,30,30,
		32,32,34,34,36,36,36,36,38,38,40,40,42,42,44,44,46,46,48,48,50,50,52,52,54,54,56,56
};
const uint8_t BIT_6high[64] = {
		0,1,2,3,4,5,6,7,
		8,9,10,11,12,13,14,15,
		16,17,18,19,20,21,22,23,
		24,25,26,27,28,29,30,31,
		32,33,34,35,36,37,38,39,
		40,41,42,43,44,45,46,47,
		48,49,50,51,52,53,54,55,
		56,57,58,59,60,61,62,63
};

const uint8_t RBIT_4low[16]={
0,21,42,63,85,106,127,148,170,191,212,233,255,255,255,255
};
const uint8_t RBIT_5low[32]={
0,10,21,31,42,53,63,74,85,95,106,116,127,138,148,159,170,180,
191,201,212,223,233,244,255,255,255,255,255,255,255,255
};
const uint8_t RBIT_6low[64]={
0,5,10,15,21,26,31,37,42,47,53,58,63,69,74,79,85,90,95,100,
106,111,116,122,127,132,138,143,148,154,159,164,170,175,180,
185,191,196,201,207,212,217,223,228,233,239,244,249,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255
};
const uint8_t RBIT_4medium[16]={
0,18,36,54,72,91,109,127,145,163,182,200,218,236,255,255

};
const uint8_t RBIT_5medium[32]={
0,9,18,27,36,45,54,63,72,81,91,100,109,118,127,136,145,154,163,
173,182,191,200,209,218,227,236,245,255,255,255,255
};
const uint8_t RBIT_6medium[64]={
0,4,9,13,18,22,27,31,36,40,45,50,54,59,63,68,72,77,81,86,91,
95,100,104,109,113,118,122,127,132,136,141,145,150,154,159,163,
168,173,177,182,186,191,195,200,204,209,214,218,223,227,232,236,
241,245,250,255,255,255,255,255,255,255,255
};
const uint8_t RBIT_4high[16]={
0,17,34,51,68,85,102,119,136,153,170,187,204,221,238,255
};
const uint8_t RBIT_5high[32]={
0,8,16,24,32,41,49,57,65,74,82,90,98,106,115,123,131,139,148,
156,164,172,180,189,197,205,213,222,230,238,246,255
};
const uint8_t RBIT_6high[64]={
0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,
72,76,80,85,89,93,97,101,105,109,113,117,121,125,129,
133,137,141,145,149,153,157,161,165,170,174,178,182,186,
190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,255
};
/* transfer state */
char *BIT_4 = (char *)BIT_4low;
char *BIT_5 = (char *)BIT_5low;
char *BIT_6 = (char *)BIT_6low;
char *RBIT_4 = (char *)RBIT_4low;
char *RBIT_5 = (char *)RBIT_5low;
char *RBIT_6 = (char *)RBIT_6low;

void __attribute__((section(".extra"))) DrawBitmap32(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
void __attribute__((section(".extra"))) DrawBuffer32(int x1, int y1, int x2, int y2, char* p, int skip);
void __attribute__((section(".extra"))) ReadBuffer32(int x1, int y1, int x2, int y2, char* p);
void __attribute__((section(".extra"))) DrawRectangle32(int x1, int y1, int x2, int y2, int c);
void __attribute__((section(".extra"))) DrawBitmap16(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
void __attribute__((section(".extra"))) DrawBuffer16(int x1, int y1, int x2, int y2, char* p, int skip);
void __attribute__((section(".extra"))) ReadBuffer16(int x1, int y1, int x2, int y2, char* p);
void __attribute__((section(".extra"))) DrawRectangle16(int x1, int y1, int x2, int y2, int c);
void __attribute__((section(".extra"))) DrawBitmap8(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
void __attribute__((section(".extra"))) DrawBuffer8(int x1, int y1, int x2, int y2, char* p, int skip);
void __attribute__((section(".extra"))) ReadBuffer8(int x1, int y1, int x2, int y2, char* p);
void __attribute__((section(".extra"))) DrawRectangle8(int x1, int y1, int x2, int y2, int c);
//void __attribute__((section(".extra"))) MoveBuffer8(int x1, int y1, int x2, int y2, int w, int h, int flip);
//void __attribute__((section(".extra"))) MoveBuffer16(int x1, int y1, int x2, int y2, int w, int h, int flip);
void (*docopy) (void *d, const void *s, int n) = mycopy;
//void __attribute__((section(".extra"))) setmode(int mode, int colour, int bc);
extern volatile uint64_t uSecTimer;
extern TIM_HandleTypeDef htim16;
uint8_t *linebuff=(uint8_t *)LBUFFSTART;

void __attribute__((section(".extra"))) InitDisplayOther(void){
	docopy=mycopy;
	setmode(-Option.mode,DEFCOLOUR,0,1);
    PromptFont = gui_font;
    PromptFC = gui_fcolour = Option.DefaultFC;
    PromptBC = gui_bcolour = Option.DefaultBC;
    if(Option.colourmap==2){
    	BIT_4 = (char *)BIT_4high;
    	BIT_5 = (char *)BIT_5high;
    	BIT_6 = (char *)BIT_6high;
    	RBIT_4 = (char *)RBIT_4high;
    	RBIT_5 = (char *)RBIT_5high;
    	RBIT_6 = (char *)RBIT_6high;
    } else if(Option.colourmap==1){
    	BIT_4 = (char *)BIT_4medium;
    	BIT_5 = (char *)BIT_5medium;
    	BIT_6 = (char *)BIT_6medium;
    	RBIT_4 = (char *)RBIT_4medium;
    	RBIT_5 = (char *)RBIT_5medium;
    	RBIT_6 = (char *)RBIT_6medium;
    } else {
    	BIT_4 = (char *)BIT_4low;
    	BIT_5 = (char *)BIT_5low;
    	BIT_6 = (char *)BIT_6low;
    	RBIT_4 = (char *)RBIT_4low;
    	RBIT_5 = (char *)RBIT_5low;
    	RBIT_6 = (char *)RBIT_6low;
   }
    ResetDisplay();
    ClearScreen(gui_bcolour);
}


/****************************************************************************************************
 ****************************************************************************************************

 Basic drawing primitives
 all drawing on the LCD is done using either one of these two functions

 ****************************************************************************************************
****************************************************************************************************/
void __attribute__((section(".extra"))) copytofloat(uint8_t ***output, int xs, int ys, int w, int h){
    union colourmap
    {
        char rgbbytes[4];
        unsigned int rgb;
    } c;
	int x,y;
	for(x=xs;x<xs+w;x++){
		for(y=ys;y<ys+h;y++){
		    ReadBuffer(x,y,x,y,(char *)&c.rgb);
		    output[0][y-ys][x-xs]=c.rgbbytes[0];
		    output[1][y-ys][x-xs]=c.rgbbytes[1];
		    output[2][y-ys][x-xs]=c.rgbbytes[2];
		}
	}
}
void __attribute__((section(".extra"))) copyfromfloat(uint8_t ***input, int xs, int ys, int w, int h, int dontcopyblack){
    union colourmap
    {
        char rgbbytes[4];
        unsigned int rgb;
    } c;
	int x,y;
	for(x=xs;x<xs+w;x++){
		for(y=ys;y<ys+h;y++){
		    c.rgbbytes[0]=(uint8_t)input[0][y-ys][x-xs];
		    c.rgbbytes[1]=(uint8_t)input[1][y-ys][x-xs];
		    c.rgbbytes[2]=(uint8_t)input[2][y-ys][x-xs];
		    c.rgbbytes[3]=0;
		    if(c.rgb)c.rgbbytes[3]=0x0f;
		    if(c.rgb || dontcopyblack==0)DrawPixel(x,y,c.rgb);
		}
	}
}

void __attribute__((section(".extra"))) PageAnd(uint32_t fadd1, uint32_t fadd2, uint32_t taddt){
	uint32_t wpa=(uint32_t)PageTable[taddt].address;
	uint32_t rpa1=(uint32_t)PageTable[fadd1].address;
	uint32_t rpa2=(uint32_t)PageTable[fadd2].address;
    int maxW=PageTable[WritePage].xmax;
	int h=PageTable[taddt].ymax;
    int x,y;
	uint64_t *s1, *s2, *d ;
    if(VideoColour<=8){
        for(y=0;y<h;y++){
    		s1=(uint64_t *)((y * maxW ) + rpa1);
    		s2=(uint64_t *)((y * maxW ) + rpa2);
    		d=(uint64_t *)((y * maxW ) + wpa);
    		for(x=0;x<(maxW>>3);x++)*d++ = ((*s1++) & (*s2++));
        }
    } else if(VideoColour<=16){
         for(y=0;y<h;y++){
            for(y=0;y<h;y++){
        		s1=(uint64_t *)((y * maxW * 2) + rpa1);
        		s2=(uint64_t *)((y * maxW *2 ) + rpa2);
        		d=(uint64_t *)((y * maxW  *2 ) + wpa);
        		for(x=0;x<(maxW>>2);x++)*d++ = ((*s1++) & (*s2++));
            }
        }
    } else {
        for(y=0;y<h;y++){
           for(y=0;y<h;y++){
       		s1=(uint64_t *)((y * maxW * 4) + rpa1);
       		s2=(uint64_t *)((y * maxW *4 ) + rpa2);
       		d=(uint64_t *)((y * maxW  *4 ) + wpa);
       		for(x=0;x<(maxW>>1);x++)*d++ = ((*s1++) & (*s2++));
           }
       }
    }
}

void __attribute__((section(".extra"))) PageOr(uint32_t fadd1, uint32_t fadd2, uint32_t taddt){
	uint32_t wpa=(uint32_t)PageTable[taddt].address;
	uint32_t rpa1=(uint32_t)PageTable[fadd1].address;
	uint32_t rpa2=(uint32_t)PageTable[fadd2].address;
    int maxW=PageTable[WritePage].xmax;
	int h=PageTable[taddt].ymax;
    int x,y;
	uint64_t *s1, *s2, *d ;
    if(VideoColour<=8){
        for(y=0;y<h;y++){
    		s1=(uint64_t *)((y * maxW ) + rpa1);
    		s2=(uint64_t *)((y * maxW ) + rpa2);
    		d=(uint64_t *)((y * maxW ) + wpa);
    		for(x=0;x<(maxW>>3);x++)*d++ = ((*s1++) | (*s2++));
        }
    } else if(VideoColour<=16){
         for(y=0;y<h;y++){
            for(y=0;y<h;y++){
        		s1=(uint64_t *)((y * maxW * 2) + rpa1);
        		s2=(uint64_t *)((y * maxW *2 ) + rpa2);
        		d=(uint64_t *)((y * maxW  *2 ) + wpa);
        		for(x=0;x<(maxW>>2);x++)*d++ = ((*s1++) | (*s2++));
            }
        }
    } else {
        for(y=0;y<h;y++){
           for(y=0;y<h;y++){
       		s1=(uint64_t *)((y * maxW * 4) + rpa1);
       		s2=(uint64_t *)((y * maxW *4 ) + rpa2);
       		d=(uint64_t *)((y * maxW  *4 ) + wpa);
       		for(x=0;x<(maxW>>1);x++)*d++ = ((*s1++) | (*s2++));
           }
       }

    }
}

void __attribute__((section(".extra"))) PageXor(uint32_t fadd1, uint32_t fadd2, uint32_t taddt){
	uint32_t wpa=(uint32_t)PageTable[taddt].address;
	uint32_t rpa1=(uint32_t)PageTable[fadd1].address;
	uint32_t rpa2=(uint32_t)PageTable[fadd2].address;
    int maxW=PageTable[WritePage].xmax;
	int h=PageTable[taddt].ymax;
    int x,y;
	uint64_t *s1, *s2, *d ;
    if(VideoColour<=8){
        for(y=0;y<h;y++){
    		s1=(uint64_t *)((y * maxW ) + rpa1);
    		s2=(uint64_t *)((y * maxW ) + rpa2);
    		d=(uint64_t *)((y * maxW ) + wpa);
    		for(x=0;x<(maxW>>3);x++)*d++ = ((*s1++) ^ (*s2++));
        }
    } else if(VideoColour<=16){
         for(y=0;y<h;y++){
            for(y=0;y<h;y++){
        		s1=(uint64_t *)((y * maxW * 2) + rpa1);
        		s2=(uint64_t *)((y * maxW *2 ) + rpa2);
        		d=(uint64_t *)((y * maxW  *2 ) + wpa);
        		for(x=0;x<(maxW>>2);x++)*d++ = ((*s1++) ^ (*s2++));
            }
        }
    } else {
        for(y=0;y<h;y++){
           for(y=0;y<h;y++){
       		s1=(uint64_t *)((y * maxW * 4) + rpa1);
       		s2=(uint64_t *)((y * maxW *4 ) + rpa2);
       		d=(uint64_t *)((y * maxW  *4 ) + wpa);
       		for(x=0;x<(maxW>>1);x++)*d++ = ((*s1++) ^ (*s2++));
           }
       }
    }
}
void __attribute__((section(".extra"))) Merge(uint32_t fadd1, uint32_t fadd2, uint32_t tadd, int c){
	uint32_t wpa=(uint32_t)PageTable[tadd].address;
	uint32_t rpa1=(uint32_t)PageTable[fadd1].address;
	uint32_t rpa2=(uint32_t)PageTable[fadd2].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
    int x,y;
    unsigned int sc;
    if(VideoColour<=8){
        uint8_t *s, *d;
    	sc = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
        for(y=0;y<maxH;y++){
    		s=(uint8_t *)((y * maxW ) + rpa1);
    		d=(uint8_t *)((y * maxW ) + wpa);
    		mycopy(d,s,maxW); //Copy the first page to the target
    		s=(uint8_t *)((y * maxW ) + rpa2);
    		d=(uint8_t *)((y * maxW ) + wpa);
    		for(x=0;x<maxW;x++){
    			if(*s!=sc)*d=*s;
    			d++;
    			s++;
    		}
        }
    } else if(VideoColour<=16){
    	uint16_t red, green, blue, trans;
    	if(VideoColour==16){
    		red=BIT_5[((c & 0xFF0000)>>19)]<<11;
    		green=BIT_6[((c & 0xFF00)>>10)]<<5;
    		blue=BIT_5[((c & 0xFF)>>3)];
    		sc=red|green|blue;
    	} else {
    		red=BIT_4[((c & 0xFF0000)>>20)]<<8;
    		green=BIT_4[((c & 0xFF00)>>12)]<<4;
    		blue=BIT_4[((c & 0xFF)>>4)];
    		trans=((c & 0xF000000)>>12);
    		sc=red|green|blue|trans;
    	}
        uint16_t *s, *d;
		for(y=0;y<maxH;y++){
			s=(uint16_t *)(y * maxW *2  + rpa1);
			d=(uint16_t *)((y * maxW *2) + wpa);
			mycopy(d,s,(maxW)<<1);
			s=(uint16_t *)((y * maxW *2) + rpa2);
			d=(uint16_t *)(y * maxW *2  + wpa);
    		for(x=0;x<maxW;x++){
    			if(*s!=sc)*d=*s;
    			d++;
    			s++;
    		}
		}
    } else {
    	sc=c;
        uint32_t *s, *d;
		for(y=0;y<maxH;y++){
			s=(uint32_t *)(y * maxW *4 + rpa1);
			d=(uint32_t *)((y * maxW *4) + wpa);
			mycopy(d,s,(maxW)<<2);
			s=(uint32_t *)((y * maxW *4) + rpa2);
			d=(uint32_t *)(y * maxW *4 + wpa);
    		for(x=0;x<maxW;x++){
    			if(*s!=sc)*d=*s;
    			d++;
    			s++;
    		}
		}
    }
}

void __attribute__((section(".extra"))) Stitch(uint32_t fadd1, uint32_t fadd2, uint32_t tadd, int offset){
	uint32_t wpa=(uint32_t)PageTable[tadd].address;
	uint32_t rpa1=(uint32_t)PageTable[fadd1].address;
	uint32_t rpa2=(uint32_t)PageTable[fadd2].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
    int y;
    if(VideoColour<=8){
        uint8_t *s, *d;
        for(y=0;y<maxH;y++){
    		s=(uint8_t *)((y * maxW +offset) + rpa1);
    		d=(uint8_t *)((y * maxW ) + wpa);
    		mycopy(d,s,maxW-offset);
    		s=(uint8_t *)((y * maxW ) + rpa2);
    		d=(uint8_t *)((y * maxW  +maxW - offset) + wpa);
    		mycopy(d,s,offset);
        }
    } else if(VideoColour<=16){
        uint16_t *s, *d;
		for(y=0;y<maxH;y++){
			s=(uint16_t *)(y * maxW *2 +( offset *2) + rpa1);
			d=(uint16_t *)((y * maxW *2) + wpa);
			mycopy(d,s,(maxW-offset)<<1);
			s=(uint16_t *)((y * maxW *2) + rpa2);
			d=(uint16_t *)(y * maxW *2 + (maxW*2) - (offset*2) + wpa);
			mycopy(d,s,offset<<1);
		}
    } else {
        uint32_t *s, *d;
		for(y=0;y<maxH;y++){
			s=(uint32_t *)(y * maxW *4 +( offset *4) + rpa1);
			d=(uint32_t *)((y * maxW *4) + wpa);
			mycopy(d,s,(maxW-offset)<<2);
			s=(uint32_t *)((y * maxW *4) + rpa2);
			d=(uint32_t *)(y * maxW *4 + (maxW*4) - (offset*4) + wpa);
			mycopy(d,s,offset<<2);
		}
    }
}
void __attribute__((section(".extra"))) WindowFrame(int x, int y){
	//PageTable[ReadPage].address points to the start of the framebuffer
	//PageTable[WritePage].address points to the start of the page to be written
	//maxW is the width of the target page
	//maxH is the height of the target page
	//PageTable[WPN].xmax is the width of the framebuffer
	//PageTable[WPN].ymax is the height of the framebuffer
	//ModeScale is undefined for this operation so define it locally
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	uint32_t rpa=(uint32_t)PageTable[WPN].address;
	uint32_t hrw = (uint32_t)PageTable[WritePage].xmax;
	uint32_t hrr = (uint32_t)PageTable[WPN].xmax;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	if(optiony)y=PageTable[WPN].ymax-1-y;
	uint32_t yp;
	if(VideoColour==8){ //first deal with 8 bit colour modes
		uint8_t *s=(uint8_t *)((y * hrr + x) + rpa); //pointer to the start of the source data
		uint8_t *d=(uint8_t *)wpa;//pointer to the start of the destination data
		if(PageTable[WritePage].expand){ //now deal with duplicated line modes
			for(yp=0; yp<maxH; yp++){
          		mycopy(d,s,hrw);
          		d+=hrw;
          		mycopy(d,s,hrw);
          		d+=hrw;
          		s+=hrr;
			}
		} else { //single line modes
			for(yp=0; yp<maxH; yp++){
          		mycopy(d,s,maxW);
          		d+=hrw;
          		s+=hrr;
			}
		}
	} else if(VideoColour<=16){
		uint8_t *s=(uint8_t *)((y * hrr + x) * 2 + rpa); //pointer to the start of the source data
		uint8_t *d=(uint8_t *)wpa;//pointer to the start of the destination data
		if(PageTable[WritePage].expand){ //now deal with duplicated line modes
			for(yp=0; yp<maxH; yp++){
          		mycopy(d,s,(hrw<<1));
          		d+=(hrw<<1);
          		mycopy(d,s,(hrw<<1));
          		d+=(hrw<<1);
          		s+=(hrr<<1);
			}
		} else { //single line modes
			for(yp=0; yp<maxH; yp++){
          		mycopy(d,s,(hrw<<1));
          		d+=(hrw<<1);
          		s+=(hrr<<1);
			}
		}
	} else {
		uint8_t *s=(uint8_t *)((y * hrr + x) * 4 + rpa); //pointer to the start of the source data
		uint8_t *d=(uint8_t *)wpa;//pointer to the start of the destination data
		if(PageTable[WritePage].expand){ //now deal with duplicated line modes
			for(yp=0; yp<maxH; yp++){
          		mycopy(d,s,(hrw<<2));
          		d+=(hrw<<2);
          		mycopy(d,s,(hrw<<2));
          		d+=(hrw<<2);
          		s+=(hrr<<2);
			}
		} else { //single line modes
			for(yp=0; yp<maxH; yp++){
          		mycopy(d,s,(hrw<<2));
          		d+=(hrw<<2);
          		s+=(hrr<<2);
			}
		}
	}
}
void __attribute__((section(".extra"))) zerocopy8(uint8_t *d, uint8_t *s, int n, int flip){ //copy skipping black
	int i;
	if(flip){
		uint8_t swap;
		uint8_t *ss;
		uint8_t *buff=linebuff;
		mycpy(buff,s,n);
	    for (i = 0; i < (n / 2); i++) {
	    	swap = buff[n - 1 - i];
	    	buff[n - 1 - i] = buff[i];
	    	buff [i] = swap;
	    }
	    ss=buff;
		for(i=0;i<n;i++){
			if(*ss)*d++=*ss++;
			else {
				ss++;
				d++;
			}
		}
	} else {
		for(i=0;i<n;i++){
			if(*s)*d++=*s++;
			else {
				s++;
				d++;
			}
		}
	}
}
void __attribute__((section(".extra"))) zerocopy8dup(uint8_t *d, uint8_t *s, int n, int flip){ //copy skipping black
	int i;
    int maxW=PageTable[WritePage].xmax;
	uint8_t *dup=d+maxW;
	if(flip){
		uint8_t swap;
		uint8_t *ss;
		uint8_t *buff=linebuff;
		mycpy(buff,s,n);
	    for (i = 0; i < (n / 2); i++) {
	    	swap = buff[n - 1 - i];
	    	buff[n - 1 - i] = buff[i];
	    	buff [i] = swap;
	    }
	    ss=buff;
		for(i=0;i<n;i++){
			if(*ss){
				*d++=*ss;
				*dup++=*ss++;
			}
			else {
				ss++;
				d++;
				dup++;
			}
		}
	} else {
		for(i=0;i<n;i++){
			if(*s){
				*d++=*s;
				*dup++=*s++;
			}
			else {
				s++;
				d++;
				dup++;
			}
		}
	}
}


void __attribute__((section(".extra"))) zerocopy16(uint16_t *d, uint16_t *s, int n, int flip){ //copy skipping black
	int i;
	if(flip){
		uint16_t swap;
		uint8_t *buff=linebuff;
		uint16_t *lbuff=(uint16_t *)buff;
		mycpy(buff,(uint8_t *)s,n<<1);
	    for (i = 0; i < (n / 2); i++) {
	    	swap = lbuff[n - 1 - i];
	    	lbuff[n - 1 - i] = lbuff[i];
	    	lbuff [i] = swap;
	    }
	    lbuff=(uint16_t *)buff;
		for(i=0;i<n;i++){
			if(*lbuff)*d++=*lbuff++;
			else {
				lbuff++;
				d++;
			}
		}
	} else {
		for(i=0;i<n;i++){
			if(*s)*d++=*s++;
			else {
				s++;
				d++;
			}
		}
	}
}
void __attribute__((section(".extra"))) zerocopy16dup(uint16_t *d, uint16_t *s, int n, int flip){ //copy skipping black
	int i;
	uint16_t *dup=d;
    int maxW=PageTable[WritePage].xmax;
	dup=dup+maxW;
	if(flip){
		uint16_t swap;
		uint8_t *buff=linebuff;
		uint16_t *lbuff=(uint16_t *)buff;
		mycpy(buff,(uint8_t *)s,n<<1);
	    for (i = 0; i < (n / 2); i++) {
	    	swap = lbuff[n - 1 - i];
	    	lbuff[n - 1 - i] = lbuff[i];
	    	lbuff [i] = swap;
	    }
	    lbuff=(uint16_t *)buff;
		for(i=0;i<n;i++){
			if(*lbuff){
				*d++=*lbuff;
				*dup++=*lbuff++;
			}
			else {
				lbuff++;
				d++;
				dup++;
			}
		}
	} else {
		for(i=0;i<n;i++){
			if(*s){
				*d++=*s;
				*dup++=*s++;
			}
			else {
				s++;
				d++;
				dup++;
			}
		}
	}
}
void __attribute__((section(".extra"))) zerocopy32(uint32_t *d, uint32_t *s, int n, int flip){ //copy skipping black
	int i;
	if(flip){
		uint32_t swap;
		uint8_t *buff=linebuff;
		uint32_t *lbuff=(uint32_t *)buff;
		mycpy(buff,(uint8_t *)s,n<<2);
	    for (i = 0; i < (n / 2); i++) {
	    	swap = lbuff[n - 1 - i];
	    	lbuff[n - 1 - i] = lbuff[i];
	    	lbuff [i] = swap;
	    }
	    lbuff=(uint32_t *)buff;
		for(i=0;i<n;i++){
			if(*lbuff)*d++=*lbuff++;
			else {
				lbuff++;
				d++;
			}
		}
	} else {
		for(i=0;i<n;i++){
			if(*s)*d++=*s++;
			else {
				s++;
				d++;
			}
		}
	}

}
void __attribute__((section(".extra"))) zerocopy32dup(uint32_t *d, uint32_t *s, int n, int flip){ //copy skipping black
	int i;
	uint32_t *dup=d;
    int maxW=PageTable[WritePage].xmax;
	dup=dup+maxW;
	if(flip){
		uint32_t swap;
		uint8_t *buff=linebuff;
		uint32_t *lbuff=(uint32_t *)buff;
		mycpy(buff,(uint8_t *)s,n<<2);
	    for (i = 0; i < (n / 2); i++) {
	    	swap = lbuff[n - 1 - i];
	    	lbuff[n - 1 - i] = lbuff[i];
	    	lbuff [i] = swap;
	    }
	    lbuff=(uint32_t *)buff;
		for(i=0;i<n;i++){
			if(*lbuff){
				*d++=*lbuff;
				*dup++=*lbuff++;
			}
			else {
				lbuff++;
				d++;
				dup++;
			}
		}
	} else {
		for(i=0;i<n;i++){
			if(*s){
				*d++=*s;
				*dup++=*s++;
			}
			else {
				s++;
				d++;
				dup++;
			}
		}
	}

}

void __attribute__((section(".extra"))) swapcopy8(uint8_t *d, uint8_t *s, int n){ //copy skipping black
	int i;
	for (i = 0; i < (n / 2); i++) {
		d[i] = s[n - 1 - i];
		d[n - 1 - i] = s[i];
	}
}

void __attribute__((section(".extra"))) swapcopy16(uint16_t *d, uint16_t *s, int n){ //copy skipping black
	int i;
	for (i = 0; i < (n / 2); i++) {
		d[i] = s[n - 1 - i];
		d[n - 1 - i] = s[i];
	}
}
void __attribute__((section(".extra"))) swapcopy32(uint32_t *d, uint32_t *s, int n){ //copy skipping black
	int i;
	for (i = 0; i < (n / 2); i++) {
		d[i] = s[n - 1 - i];
		d[n - 1 - i] = s[i];
	}
}
/*
 * FLIP parameter
0 - normal display (default if omitted)
1 - mirrored left to right
2 - mirrored top to bottom
3 - rotated 180 degrees (= 1+2)
4 - transparent normal display (default if omitted)
5 - transparent mirrored left to right
6 - transparent mirrored top to bottom
7 - transparent rotated 180 degrees (= 1+2)
*/
void __attribute__((section(".extra"))) MoveBufferNormal(int x1, int y1, int x2, int y2, int w, int h, int flip){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
	uint32_t hrw = (uint32_t)PageTable[WritePage].xmax;
	uint32_t hrr = (uint32_t)PageTable[ReadPage].xmax;
 	if(optiony)y1=PageTable[ReadPage].ymax-y1-h;
	if(optiony)y2=PageTable[WritePage].ymax-y2-h;
	int yp, cursorhidden=0;
    if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    	uint8_t writesave=WritePage;
    	WritePage=ReadPage;
    	cursorhidden=1;
    	hidecursor(0);
    	WritePage=writesave;
    }
    //Check if we need to process overlapping areas
    if(ReadPage==WritePage && !(x1 + w <= x2 || x1 >= x2+w || y1 + h <= y2 || y1 >= y2+h)){
    	char *buff=GetMemory(w*h*(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
    	ReadBufferFast(x1,y1,x1+w-1,y1+h-1,buff);
    	if(flip==0){
    		DrawBufferFast(x2,y2,x2+w-1,y2+h-1,buff);
    	} else if(flip==1){
    		if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy8(d,s,w);
    				s+=w;
    				d+=hrw;
    			}
    		} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
    				s+=(w<<1);
    				d+=(hrw<<1);
    			}
    		} else {
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
    				s+=(w<<2);
    				d+=(hrw<<2);
    			}
    		}
		} else if(flip==2){
    		if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
    			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				docopy(d,s,w);
    				s+=w;
    				d-=hrw;
    			}
    		} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
    			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				docopy(d,s,(w<<1));
    				s+=(w<<1);
    				d-=(hrw<<1);
    			}
    		} else {
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
    			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				docopy(d,s,(w<<2));
    				s+=(w<<2);
    				d-=(hrw<<2);
    			}
    		}
		} else if(flip==3){
			if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
    				swapcopy8(d,s,w);
    				mycpy(d,s,w);
    				s+=w;
					d-=hrw;
				}
			} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
    				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
    				mycpy(d,s,w<<1);
					s+=(w<<1);
					d-=(hrw<<1);
				}
			} else {
    			uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
    				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
    				mycpy(d,s,w<<2);
					s+=(w<<2);
					d-=(hrw<<2);
				}
			}
		} else if(flip==4 || flip==5){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy8(d,s,w, flip & 1);
					s+=w;
					d+=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy16((uint16_t *)d,(uint16_t *)s,w, flip & 1);
					s+=(w<<1);
					d+=(hrw<<1);
				}
			} else{
				uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy32((uint32_t *)d,(uint32_t *)s,w, flip & 1);
					s+=(w<<2);
					d+=(hrw<<2);
				}
			}
		} else if(flip==6 || flip==7){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy8(d,s,w, flip & 1);
					s+=w;
					d-=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy16((uint16_t *)d,(uint16_t *)s,w, flip & 1);
					s+=(w<<1);
					d-=(hrw<<1);
				}
			} else{
				uint8_t *s=(uint8_t *)buff;//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy32((uint32_t *)d,(uint32_t *)s,w, flip & 1);
					s+=(w<<2);
					d-=(hrw<<2);
				}
			}
		}
    	FreeMemorySafe((void **)&buff);
    	return;
    } else {
		if(flip==0){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) + x1 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,w);
					s+=hrr;
					d+=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 * 2 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<1));
					s+=(hrr<<1);
					d+=(hrw<<1);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 4 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<2));
					s+=(hrr<<2);
					d+=(hrw<<2);
				}
			}
		} else if(flip==1){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) + x1 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy8(d,s,w);
					s+=hrr;
					d+=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 * 2 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy16((uint16_t *)d,(uint16_t *)s,w);
					s+=(hrr<<1);
					d+=(hrw<<1);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 4 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy32((uint32_t *)d,(uint32_t *)s,w);
					s+=(hrr<<2);
					d+=(hrw<<2);
				}
			}
		} else if(flip==2){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) + x1 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,w);
					s+=hrr;
					d-=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 * 2 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<1));
					s+=(hrr<<1);
					d-=(hrw<<1);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 4 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<2));
					s+=(hrr<<2);
					d-=(hrw<<2);
				}
			}
		} else if(flip==3){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) + x1 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy8(d,s,w);
					s+=hrr;
					d-=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 * 2 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy16((uint16_t *)d,(uint16_t *)s,w);
					s+=(hrr<<1);
					d-=(hrw<<1);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 4 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy32((uint32_t *)d,(uint32_t *)s,w);
					s+=(hrr<<2);
					d-=(hrw<<2);
				}
			}
		} else if(flip==4 || flip==5){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr)  + x1 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy8(d,s,w, flip & 1);
					s+=hrr;
					d+=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 * 2 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy16((uint16_t *)d,(uint16_t *)s,w, flip & 1);
					s+=(hrr<<1);
					d+=(hrw<<1);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 4 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy32((uint32_t *)d,(uint32_t *)s,w, flip & 1);
					s+=(hrr<<2);
					d+=(hrw<<2);
				}
			}
		} else if(flip==6 || flip==7){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) + x1 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy8(d,s,w, flip & 1);
					s+=hrr;
					d-=hrw;
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 * 2 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy16((uint16_t *)d,(uint16_t *)s,w, flip & 1);
					s+=(hrr<<1);
					d-=(hrw<<1);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 4 + rpa);//pointer to the start of the source data
				uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy32((uint32_t *)d,(uint32_t *)s,w, flip & 1);
					s+=(hrr<<2);
					d-=(hrw<<2);
				}
			}
		}
    }
	if(cursorhidden){
		uint8_t writesave=WritePage;
		WritePage=ReadPage;
		cursorhidden=1;
		showcursor(0, xcursor,ycursor);
		WritePage=writesave;
	}
}


void __attribute__((section(".extra"))) MoveBufferDup(int x1, int y1, int x2, int y2, int w, int h, int flip){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
	uint32_t hrw = (uint32_t)PageTable[WritePage].xmax;
	uint32_t hrr = (uint32_t)PageTable[ReadPage].xmax;
	int maxH=PageTable[WritePage].ymax;
	int yp,cursorhidden=0;
	if(optiony)y1=maxH-y1-h;
	if(optiony)y2=maxH-y2-h;
    if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    	uint8_t writesave=WritePage;
    	WritePage=ReadPage;
    	cursorhidden=1;
    	hidecursor(0);
    	WritePage=writesave;
    }
    //Check if we need to process overlapping areas
    if(ReadPage==WritePage && !(x1 + w <= x2 || x1 >= x2+w || y1 + h <= y2 || y1 >= y2+h)){
    	char *buff=GetMemory(w*h*(VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
    	ReadBufferFast(x1,y1,x1+w-1,y1+h-1,buff);
    	if(flip==0){
    		DrawBufferFast(x2,y2,x2+w-1,y2+h-1,buff);
    	} else if(flip==1){
    		if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy8(d,s,w);
    				d+=hrw;
    				swapcopy8(d,s,w);
    				d+=hrw;
    				s+=w;
    			}
    		} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
    				d+=(hrw<<1);
    				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
    				d+=(hrw<<1);
    				s+=(w<<1);
    			}
    		} else {
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 4 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
    				d+=(hrw<<2);
    				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
    				d+=(hrw<<2);
    				s+=(w<<2);
    			}
    		}
    	} else if(flip==2){
    		if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff + w * (h-1); //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				docopy(d,s,w);
    				d+=hrw;
    				docopy(d,s,w);
    				d+=hrw;
    				s-=w;
    			}
    		} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff + w * 2 * (h-1); //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				docopy(d,s,(w<<1));
    				d+=(hrw<<1);
    				docopy(d,s,(w<<1));
    				d+=(hrw<<1);
    				s-=(w<<1);
    			}
    		} else {
    			uint8_t *s=(uint8_t *)buff + w * 4 * (h-1); //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				docopy(d,s,(w<<2));
    				d+=(hrw<<2);
    				docopy(d,s,(w<<2));
    				d+=(hrw<<2);
    				s-=(w<<2);
    			}
    		}
    	} else if(flip==3){
    		if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy8(d,s,w);
    				d+=hrw;
    				swapcopy8(d,s,w);
    				d+=hrw;
    				s-=w;
    			}
    		} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
    				d+=(hrw<<1);
    				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
    				d+=(hrw<<1);
    				s-=(w<<1);
    			}
    		} else {
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
    				d+=(hrw<<2);
    				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
    				d+=(hrw<<2);
    				s-=(w<<2);
    			}
    		}
    	} else if(flip==4 || flip==5){
    		if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				zerocopy8dup(d,s,w, flip & 1);
    				d+=(hrw<<1);
    				s+=w;
    			}
    		} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * 2 * hrw) * 2  + x2 * 2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				zerocopy16dup((uint16_t *)d,(uint16_t *)s,w, flip & 1);
    				d+=(hrw<<2);
    				s+=(w<<1);
    			}
    		} else {
    			uint8_t *s=(uint8_t *)buff; //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * 4 * hrw) * 2  + x2 * 4 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				zerocopy32dup((uint32_t *)d,(uint32_t *)s,w, flip & 1);
    				d+=(hrw<<3);
    				s+=(w<<2);
    			}
     		}
    	} else if(flip==6 || flip==7){
    		if(VideoColour==8){ //first deal with 8 bit colour modes
    			uint8_t *s=(uint8_t *)buff + w * (h-1); //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				zerocopy8dup(d,s,w, flip & 1);
    				d+=(hrw<<1);
    				s-=w;
    			}
    		} else if(VideoColour<=16){
    			uint8_t *s=(uint8_t *)buff + w * 2 * (h-1); //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				zerocopy16dup((uint16_t *)d,(uint16_t *)s,w, flip & 1);
    				d+=(hrw<<2);
    				s-=(w<<1);
    			}
    		} else {
    			uint8_t *s=(uint8_t *)buff + w * 4 * (h-1); //pointer to the start of the source data
    			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
    			for(yp=0; yp<h; yp++){
    				zerocopy32dup((uint32_t *)d,(uint32_t *)s,w, flip & 1);
    				d+=(hrw<<3);
    				s-=(w<<2);
    			}
    		}
    	}
    	FreeMemorySafe((void **)&buff);
    	return;
    } else {
   	if(flip==0){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2 + x1 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 2 + x2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,w);
					d+=hrw;
					docopy(d,s,w);
					d+=hrw;
					s+=(hrr<<1);
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<1));
					d+=(hrw<<1);
					docopy(d,s,(w<<1));
					d+=(hrw<<1);
					s+=(hrr<<2);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<2));
					d+=(hrw<<2);
					docopy(d,s,(w<<2));
					d+=(hrw<<2);
					s+=(hrr<<3);
				}
			}
		} else if(flip==1){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2 + x1 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy8(d,s,w);
					d+=hrw;
					swapcopy8(d,s,w);
					d+=hrw;
					s+=(hrr<<1);
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy16((uint16_t *)d,(uint16_t *)s,w);
					d+=(hrw<<1);
					swapcopy16((uint16_t *)d,(uint16_t *)s,w);
					d+=(hrw<<1);
					s+=(hrr<<2);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy32((uint32_t *)d,(uint32_t *)s,w);
					d+=(hrw<<2);
					swapcopy32((uint32_t *)d,(uint32_t *)s,w);
					d+=(hrw<<2);
					s+=(hrr<<3);
				}
			}
		} else if(flip==2){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,w);
					d+=hrw;
					docopy(d,s,w);
					d+=hrw;
					s-=(hrr<<1);
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) * 2 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<1));
					d+=(hrw<<1);
					docopy(d,s,(w<<1));
					d+=(hrw<<1);
					s-=(hrr<<2);
				}
			} else {
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) * 4 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					docopy(d,s,(w<<2));
					d+=(hrw<<2);
					docopy(d,s,(w<<2));
					d+=(hrw<<2);
					s-=(hrr<<3);
				}
			}
		} else if(flip==3){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy8(d,s,w);
					d+=hrw;
					swapcopy8(d,s,w);
					d+=hrw;
					s-=(hrr<<1);
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) * 2 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy16((uint16_t *)d,(uint16_t *)s,w);
					d+=(hrw<<1);
					swapcopy16((uint16_t *)d,(uint16_t *)s,w);
					d+=(hrw<<1);
					s-=(hrr<<2);
				}
			} else {
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) * 4 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					swapcopy32((uint32_t *)d,(uint32_t *)s,w);
					d+=(hrw<<2);
					swapcopy32((uint32_t *)d,(uint32_t *)s,w);
					d+=(hrw<<2);
					s-=(hrr<<3);
				}
			}
		} else if(flip==4 || flip==5){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 + rpa);//pointer to the start of the destination data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy8dup(d,s,w, flip & 1);
					d+=(hrw<<1);
					s+=(hrr<<1);
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)((y1 * 2 * hrw) * 2  + x1 * 2 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * 2 * hrw) * 2  + x2 * 2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy16dup((uint16_t *)d,(uint16_t *)s,w, flip & 1);
					d+=(hrw<<2);
					s+=(hrr<<2);
				}
			} else {
				uint8_t *s=(uint8_t *)((y1 * 2 * hrw) * 2  + x1 * 4 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * 2 * hrw) * 2  + x2 * 4 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy32dup((uint32_t *)d,(uint32_t *)s,w, flip & 1);
					d+=(hrw<<3);
					s+=(hrr<<3);
				}
			}
		} else if(flip==6 || flip==7){
			if(VideoColour==8){ //first deal with 8 bit colour modes
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy8dup(d,s,w, flip & 1);
					d+=(hrw<<1);
					s-=(hrr<<1);
				}
			} else if(VideoColour<=16){
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) * 2 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy16dup((uint16_t *)d,(uint16_t *)s,w, flip & 1);
					d+=(hrw<<2);
					s-=(hrr<<2);
				}
			} else {
				uint8_t *s=(uint8_t *)(((y1 + h - 1) * 2 * hrw + x1) * 4 + rpa); //pointer to the start of the source data
				uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
				for(yp=0; yp<h; yp++){
					zerocopy32dup((uint32_t *)d,(uint32_t *)s,w, flip & 1);
					d+=(hrw<<3);
					s-=(hrr<<3);
				}
			}
		}
    }
	if(cursorhidden){
		uint8_t writesave=WritePage;
		WritePage=ReadPage;
		cursorhidden=1;
		showcursor(0, xcursor,ycursor);
		WritePage=writesave;
	}
}

void __attribute__((section(".extra"))) MoveBufferExpand(int x1, int y1, int x2, int y2, int w, int h, int flip){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
	uint32_t hrw = (uint32_t)PageTable[WritePage].xmax;
	uint32_t hrr = (uint32_t)PageTable[ReadPage].xmax;
	int yp,cursorhidden=0;
	if(optiony)y1=PageTable[ReadPage].ymax-y1-h;
	if(optiony)y2=PageTable[WritePage].ymax-y2-h;
    if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    	uint8_t readsave=ReadPage;
    	ReadPage=WritePage;
    	cursorhidden=1;
    	hidecursor(0);
    	ReadPage=readsave;
    }
	if(flip==0){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,w);
				d+=hrw;
				docopy(d,s,w);
				d+=hrw;
				s+=hrr;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) * 2 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<1));
				d+=(hrw<<1);
				docopy(d,s,(w<<1));
				d+=(hrw<<1);
				s+=(hrr<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) * 4 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<2));
				d+=(hrw<<2);
				docopy(d,s,(w<<2));
				d+=(hrw<<2);
				s+=(hrr<<2);
			}
		}
	} else if(flip==1){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy8(d,s,w);
				d+=hrw;
				swapcopy8(d,s,w);
				d+=hrw;
				s+=hrr;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) * 2 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
				d+=(hrw<<1);
				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
				d+=(hrw<<1);
				s+=(hrr<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) * 4 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
				d+=(hrw<<2);
				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
				d+=(hrw<<2);
				s+=(hrr<<2);
			}
		}
	} else if(flip==2){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
			*d+=hrw;
			for(yp=0; yp<h; yp++){
				docopy(d,s,w);
				d+=hrw;
				docopy(d,s,w);
				d+=hrw;
				s-=hrr;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) * 2 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
			*d+=hrw;
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<1));
				d+=(hrw<<1);
				docopy(d,s,(w<<1));
				d+=(hrw<<1);
				s-=(hrr<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) * 4 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
			*d+=hrw;
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<2));
				d+=(hrw<<2);
				docopy(d,s,(w<<2));
				d+=(hrw<<2);
				s-=(hrr<<2);
			}
		}
	} else if(flip==3){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy8(d,s,w);
				d+=hrw;
				swapcopy8(d,s,w);
				d+=hrw;
				s-=hrr;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) * 2 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
				d+=(hrw<<1);
				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
				d+=(hrw<<1);
				s-=(hrr<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) * 4 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
				d+=(hrw<<2);
				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
				d+=(hrw<<2);
				s-=(hrr<<2);
			}
		}
	} else if(flip==4 || flip==5){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy8dup(d,s,w, flip & 1);
				d+=hrw;
				d+=hrw;
				s+=hrr;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) * 2 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy16dup((uint16_t *)d,(uint16_t *)s,w, flip & 1);
				d+=(hrw<<1);
				d+=(hrw<<1);
				s+=(hrr<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr + x1) * 4 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy32dup((uint32_t *)d,(uint32_t *)s,w, flip & 1);
				d+=(hrw<<2);
				d+=(hrw<<2);
				s+=(hrr<<2);
			}
		}
	} else if(flip==6 || flip==7){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 2  + x2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy8dup(d,s,w, flip & 1);
				d+=hrw;
				d+=hrw;
				s-=hrr;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) * 2 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 4  + x2 * 2 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy16dup((uint16_t *)d,(uint16_t *)s,w, flip & 1);
				d+=(hrw<<1);
				d+=(hrw<<1);
				s-=(hrr<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)(((y1 + h - 1) * hrr + x1) * 4 + rpa); //pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw) * 8  + x2 * 4 + wpa);//pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy32dup((uint32_t *)d,(uint32_t *)s,w, flip & 1);
				d+=(hrw<<2);
				d+=(hrw<<2);
				s-=(hrr<<2);
			}
		}
	}

	if(cursorhidden){
    	uint8_t readsave=ReadPage;
    	ReadPage=WritePage;
    	cursorhidden=1;
		showcursor(0, xcursor,ycursor);
    	ReadPage=readsave;
	}
}
void __attribute__((section(".extra"))) MoveBufferContract(int x1, int y1, int x2, int y2, int w, int h, int flip){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
	uint32_t hrw = (uint32_t)PageTable[WritePage].xmax;
	uint32_t hrr = (uint32_t)PageTable[ReadPage].xmax;
	int yp, cursorhidden=0;
    if(cursoron && (ReadPage==(VideoColour == 12 ? 1 : 0 ) || WritePage==(VideoColour == 12 ? 1 : 0 ))) {
    	uint8_t writesave=WritePage;
    	WritePage=ReadPage;
    	cursorhidden=1;
    	hidecursor(0);
    	WritePage=writesave;
    }
	if(flip==0){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,w);
				s+=(hrr<<1);
				d+=hrw;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<1));
				s+=(hrr<<2);
				d+=(hrw<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<2));
				s+=(hrr<<3);
				d+=(hrw<<2);
			}
		}
	} else if(flip==1){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy8(d,s,w);
				s+=(hrr<<1);
				d+=hrw;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
				s+=(hrr<<2);
				d+=(hrw<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
				s+=(hrr<<3);
				d+=(hrw<<2);
			}
		}
	} else if(flip==2){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,w);
				s+=(hrr<<1);
				d-=hrw;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<1));
				s+=(hrr<<2);
				d-=(hrw<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				docopy(d,s,(w<<2));
				s+=(hrr<<3);
				d-=(hrw<<2);
			}
		}
	} else if(flip==3){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy8(d,s,w);
				s+=(hrr<<1);
				d-=hrw;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy16((uint16_t *)d,(uint16_t *)s,w);
				s+=(hrr<<2);
				d-=(hrw<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				swapcopy32((uint32_t *)d,(uint32_t *)s,w);
				s+=(hrr<<3);
				d-=(hrw<<2);
			}
		}
	} else if(flip==4 || flip==5){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy8(d,s,w, flip & 1);
				s+=(hrr<<1);
				d+=hrw;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy16((uint16_t *)d,(uint16_t *)s,w, flip & 1);
				s+=(hrr<<2);
				d+=(hrw<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)((y2 * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy32((uint32_t *)d,(uint32_t *)s,w, flip & 1);
				s+=(hrr<<3);
				d+=(hrw<<2);
			}
		}
	} else if(flip==6 || flip==7){
		if(VideoColour==8){ //first deal with 8 bit colour modes
			uint8_t *s=(uint8_t *)((y1 * hrr) * 2  + x1 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy8(d,s,w, flip & 1);
				s+=(hrr<<1);
				d-=hrw;
			}
		} else if(VideoColour<=16){
			uint8_t *s=(uint8_t *)((y1 * hrr) * 4  + x1 * 2 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 2 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy16((uint16_t *)d,(uint16_t *)s,w, flip & 1);
				s+=(hrr<<2);
				d-=(hrw<<1);
			}
		} else {
			uint8_t *s=(uint8_t *)((y1 * hrr) * 8  + x1 * 4 + rpa);//pointer to the start of the source data
			uint8_t *d=(uint8_t *)(((y2 + h - 1) * hrw + x2) * 4 + wpa); //pointer to the start of the destination data
			for(yp=0; yp<h; yp++){
				zerocopy32((uint32_t *)d,(uint32_t *)s,w, flip & 1);
				s+=(hrr<<3);
				d-=(hrw<<2);
			}
		}
	}
	if(cursorhidden){
		uint8_t writesave=WritePage;
		WritePage=ReadPage;
		cursorhidden=1;
		showcursor(0, xcursor,ycursor);
		WritePage=writesave;
	}
}

void __attribute__((section(".extra"))) ScrollBuff32V(int lines, int blank){
	uint32_t *s,*d;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int y, yy,cursorhidden=0;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	int n=(maxW * Scale  * 4);
    if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
    	cursorhidden=1;
    	hidecursor(0);
    }
	if(lines>0){
		for(y=0;y<maxH-lines;y++){
			yy=y+lines;
			d=(uint32_t *)((y * maxW * Scale) * 4 + wpa);
			s=(uint32_t *)((yy * maxW * Scale) * 4 + wpa);
			mycopy(d,s,n);
		}
        if(blank){
        	if(ShortScroll)DrawRectangle(0, maxH-lines-gui_font_height, maxW - 1, maxH -gui_font_height - 1, gui_bcolour);
        	else DrawRectangle(0, maxH-lines, maxW - 1, maxH - 1, gui_bcolour); // erase the line to be scrolled off
        }
    } else if(lines<0){
    	lines=-lines;
    	for(y=maxH-1;y>=lines;y--){
			yy=y-lines;
			d=(uint32_t *)((y * maxW * Scale) * 4 + wpa);
			s=(uint32_t *)((yy * maxW * Scale) * 4 + wpa);
			mycopy(d,s,n);
		}
    	if(blank)DrawRectangle(0, 0, maxW - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
    }
    if(cursorhidden)showcursor(0, xcursor, ycursor);
}


void __attribute__((section(".extra"))) ScrollBuff16V(int lines, int blank){
	uint32_t *s,*d;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int y, yy,cursorhidden=0;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	int n=(maxW * Scale  * 2);
    if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
    	cursorhidden=1;
    	hidecursor(0);
    }
	if(lines>0){
		for(y=0;y<maxH-lines;y++){
			yy=y+lines;
			d=(uint32_t *)((y * maxW * Scale) * 2 + wpa);
			s=(uint32_t *)((yy * maxW * Scale) * 2 + wpa);
			mycopy(d,s,n);
		}
        if(blank){
        	if(ShortScroll)DrawRectangle(0, maxH-lines-gui_font_height, maxW - 1, maxH -gui_font_height - 1, gui_bcolour);
        	else DrawRectangle(0, maxH-lines, maxW - 1, maxH - 1, gui_bcolour); // erase the line to be scrolled off
        }
    } else if(lines<0){
    	lines=-lines;
    	for(y=maxH-1;y>=lines;y--){
			yy=y-lines;
			d=(uint32_t *)((y * maxW * Scale) * 2 + wpa);
			s=(uint32_t *)((yy * maxW * Scale) * 2 + wpa);
			mycopy(d,s,n);
		}
    	if(blank)DrawRectangle(0, 0, maxW - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
    }
    if(cursorhidden)showcursor(0, xcursor, ycursor);
}

void __attribute__((section(".extra"))) ScrollBuff8V(int lines, int blank){
    uint32_t *s,*d;
    int y, yy;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
    int n=(maxW * Scale),cursorhidden=0;
    if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
    	cursorhidden=1;
    	hidecursor(0);
    }
    if(lines>0){
        for(y=0;y<maxH-lines-(ShortScroll?gui_font_height:0);y++){
            yy=y+lines;
            d=(uint32_t *)((y * maxW * Scale) + wpa);
            s=(uint32_t *)((yy * maxW * Scale) + wpa);
            mycopy(d,s,n);
        }
        if(blank){
        	if(ShortScroll)DrawRectangle(0, maxH-lines-gui_font_height, maxW - 1, maxH -gui_font_height - 1, gui_bcolour);
        	else DrawRectangle(0, maxH-lines, maxW - 1, maxH - 1, gui_bcolour); // erase the line to be scrolled off
        }
    } else if(lines<0){
        lines=-lines;
        for(y=maxH-1;y>=lines;y--){
            yy=y-lines;
            d=(uint32_t *)((y * maxW * Scale) + wpa);
            s=(uint32_t *)((yy * maxW * Scale) + wpa);
            mycopy(d,s,n);
        }
        if(blank)DrawRectangle(0, 0, maxW - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
    }
    if(cursorhidden)showcursor(0, xcursor, ycursor);
}

void __attribute__((section(".extra"))) ScrollBuff32H(int pixels){
	if(!pixels)return;
	uint32_t *s,*d;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	int y, lmaxH=maxH*Scale,cursorhidden=0;
    if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
    	cursorhidden=1;
    	hidecursor(0);
    }
	if(pixels>0){
		for(y=0;y<lmaxH;y++){
			d=(uint32_t *)((y * maxW * 4 ) + wpa);
			s=(uint32_t *)((y * maxW * 4 ) + wpa);
			d+=pixels;
			mycopy(linebuff,s,(maxW-pixels)<<2);
			mycopy(d,linebuff,(maxW-pixels)<<2);
		}
	} else if(pixels<0){
		pixels=-pixels;
		for(y=0;y<lmaxH;y++){
			d=(uint32_t *)((y * maxW * 4 ) + wpa);
			s=(uint32_t *)((y * maxW * 4 ) + wpa);
			s+=pixels;
			mycopy(linebuff,s,(maxW-pixels)<<2);
			mycopy(d,linebuff,(maxW-pixels)<<2);
		}
	}
    if(cursorhidden)showcursor(0, xcursor, ycursor);

}
void __attribute__((section(".extra"))) ScrollBuff16H(int pixels){
	if(!pixels)return;
	uint16_t *s,*d;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	int y, lmaxH=maxH*Scale,cursorhidden=0;
    if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
    	cursorhidden=1;
    	hidecursor(0);
    }
	if(pixels>0){
		for(y=0;y<lmaxH;y++){
			d=(uint16_t *)((y * maxW * 2 ) + wpa);
			s=(uint16_t *)((y * maxW * 2 ) + wpa);
			d+=pixels;
			mycopy(linebuff,s,(maxW-pixels)<<1);
			mycopy(d,linebuff,(maxW-pixels)<<1);
		}
	} else if(pixels<0){
		pixels=-pixels;
		for(y=0;y<lmaxH;y++){
			d=(uint16_t *)((y * maxW * 2 ) + wpa);
			s=(uint16_t *)((y * maxW * 2 ) + wpa);
//			s+=pixels;
//			mycopy(linebuff,s,(maxW-pixels)<<1);
			mycopy(linebuff,s,maxW<<1);
			mycopy(d,linebuff+(pixels<<1),(maxW-pixels)<<1);
		}
	}
    if(cursorhidden)showcursor(0, xcursor, ycursor);
}

void __attribute__((section(".extra"))) ScrollBuff8H(int pixels){
	if(!pixels)return;
	uint8_t *s,*d;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	int y, lmaxH=maxH*Scale,cursorhidden=0;
    if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
    	cursorhidden=1;
    	hidecursor(0);
    }
	if(pixels>0){
		for(y=0;y<lmaxH;y++){
			d=(uint8_t *)((y * maxW) + wpa);
			s=(uint8_t *)((y * maxW) + wpa);
			d+=pixels;
			mycopy(linebuff,s,maxW-pixels);
			mycopy(d,linebuff,maxW-pixels);
		}
	} else if(pixels<0){
		pixels=-pixels;
		for(y=0;y<lmaxH;y++){
			d=(uint8_t *)((y * maxW) + wpa);
			s=(uint8_t *)((y * maxW) + wpa);
//			s+=pixels;
//			mycopy(linebuff,s,maxW-pixels);
			mycopy(linebuff,s,maxW);
			mycopy(d,linebuff+pixels,maxW-pixels);
		}
	}
    if(cursorhidden)showcursor(0, xcursor, ycursor);
}

void __attribute__((section(".extra"))) DrawPixel32(int x, int y, int c){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	if(optiony)y=maxH-1-y;
	if(x<0 || x>=maxW || y<0 || y>=maxH) return;
	if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
		if(xcursor<=x && xcursor+wcursor>x && ycursor<=y && ycursor+hcursor>y){
			int offset=(y-ycursor)*wcursor + x-xcursor;
			if(VideoColour==8 && cursor8[offset]!=0)return;
			if(VideoColour==16 && cursor16[offset]!=0)return;
		}
	}
	uint32_t *s, *s1;
	if(PageTable[WritePage].expand==0){
		s=(uint32_t *)((y * maxW + x) * 4 + wpa);
		*s=c;
	} else {
		y*=2;
		s=(uint32_t *)((y * maxW + x) * 4 + wpa);
		s1=(uint32_t *)(((y+1) * maxW + x) * 4 + wpa);
		*s=c;
		*s1=c;
	}
}

void __attribute__((section(".extra"))) DrawPixel16(int x, int y, int c){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	if(optiony)y=maxH-1-y;
	if(x<0 || x>=maxW || y<0 || y>=maxH) return;
	if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
		if(xcursor<=x && xcursor+wcursor>x && ycursor<=y && ycursor+hcursor>y){
			int offset=(y-ycursor)*wcursor + x-xcursor;
			if(VideoColour==8 && cursor8[offset]!=0)return;
			if(VideoColour==16 && cursor16[offset]!=0)return;
		}
	}
	uint16_t *s, *s1, sc;
	uint16_t red, green, blue, trans;
	if(VideoColour==16){
		red=BIT_5[((c & 0xFF0000)>>19)]<<11;
		green=BIT_6[((c & 0xFF00)>>10)]<<5;
		blue=BIT_5[((c & 0xFF)>>3)];
		sc=red|green|blue;
	} else {
		red=BIT_4[((c & 0xFF0000)>>20)]<<8;
		green=BIT_4[((c & 0xFF00)>>12)]<<4;
		blue=BIT_4[((c & 0xFF)>>4)];
		trans=((c & 0xF000000)>>12);
		sc=red|green|blue|trans;
	}
	if(PageTable[WritePage].expand==0){
		s=(uint16_t *)((y * maxW + x) * 2 + wpa);
		*s=sc;
	} else {
		y*=2;
		s=(uint16_t *)((y * maxW + x) * 2 + wpa);
		s1=(uint16_t *)(((y+1) * maxW + x) * 2 + wpa);
		*s=sc;
		*s1=sc;
	}
}
void __attribute__((section(".extra"))) DrawPixel8(int x, int y, int c){
	uint8_t *s, *s1, sc;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	if(optiony)y=maxH-1-y;
	if(x<0 || x>=maxW || y<0 || y>=maxH) return;
	if(cursoron && WritePage == (VideoColour==12 ? 1 : 0)){
		if(xcursor<=x && xcursor+wcursor>x && ycursor<=y && ycursor+hcursor>y){
			int offset=(y-ycursor)*wcursor + x-xcursor;
			if(VideoColour==8 && cursor8[offset]!=0)return;
			if(VideoColour==16 && cursor16[offset]!=0)return;
		}
	}
	sc = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
	if(PageTable[WritePage].expand==0){
		s=(uint8_t *)((y * maxW + x) + wpa);
		*s=sc;
	} else {
		y*=2;
		s=(uint8_t *)((y * maxW + x) + wpa);
		s1=(uint8_t *)(((y+1) * maxW + x) + wpa);
		*s=sc;
		*s1=sc;
	}
}

void __attribute__((section(".extra"))) DrawPixelExternal(int x, int y, int c){
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	if(optiony)y=maxH-1-y;
	if(x<0 || x>=maxW || y<0 || y>=maxH) return;
	if(cursoron && WritePage==(VideoColour==12 ? 1 : 0)){
		if(xcursor<=x && xcursor+wcursor>x && ycursor<=y && ycursor+hcursor>y){
			int offset=(y-ycursor)*wcursor + x-xcursor;
			if(VideoColour==8 && cursor8[offset]!=0)return;
			if(VideoColour==16 && cursor16[offset]!=0)return;
		}
	}
    if(VideoColour<=8){
    	uint8_t *s, *s1, sc;
        sc = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
        if(PageTable[WritePage].expand==0){
        	s=(uint8_t *)((y * maxW + x) + wpa);
            *s=sc;
        } else {
        	y*=2;
           	s=(uint8_t *)((y * maxW + x) + wpa);
           	s1=(uint8_t *)(((y+1) * maxW + x) + wpa);
        	*s=sc;
        	*s1=sc;
        }
    } else if(VideoColour<=16){
    	uint16_t *s, *s1, sc;
        uint16_t red, green, blue, trans;
        if(VideoColour==16){
        	red=BIT_5[((c & 0xFF0000)>>19)]<<11;
            green=BIT_6[((c & 0xFF00)>>10)]<<5;
            blue=BIT_5[((c & 0xFF)>>3)];
            sc=red|green|blue;
        } else {
        	red=BIT_4[((c & 0xFF0000)>>20)]<<8;
            green=BIT_4[((c & 0xFF00)>>12)]<<4;
            blue=BIT_4[((c & 0xFF)>>4)];
            trans=((c & 0xF000000)>>12);
            sc=red|green|blue|trans;
        }
        if(PageTable[WritePage].expand==0){
        	s=(uint16_t *)((y * maxW + x) * 2 + wpa);
        	*s=sc;
        } else {
        	y*=2;
        	s=(uint16_t *)((y * maxW + x) * 2 + wpa);
        	s1=(uint16_t *)(((y+1) * maxW + x) * 2 + wpa);
        	*s=sc;
        	*s1=sc;
        }
    } else {

    }
}

void __attribute__((section(".extra"))) DrawRectangle32(int x1, int y1, int x2, int y2, int c){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int y, t;
    uint32_t *s;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    if((x1<0 && x2<0) || (y1<0 && y2<0) || (x1>=maxW && x2>=maxW) || (y1>=maxH && y2>=maxH))return;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) x1 = 0;
    if(x1 >= maxW) x1 = maxW - 1;
    if(x2 < 0) x2 = 0;
    if(x2 >= maxW) x2 = maxW - 1;
    if(y1 < 0) y1 = 0;
    if(y1 >= maxH) y1 = maxH - 1;
    if(y2 < 0) y2 = 0;
    if(y2 >= maxH) y2 = maxH - 1;
    int cursorhidden=0;
    if(cursoron && WritePage==(VideoColour == 12 ? 1 : 0 ) )
			if( !(xcursor + wcursor < x1 ||
				xcursor > x2 ||
				ycursor + hcursor < y1 ||
				ycursor > y2)){

			hidecursor(0);
			cursorhidden=1;
    	}
    if(PageTable[WritePage].expand==0){
    	for(y=y1;y<=y2;y++){
    		s=(uint32_t *)((y * maxW + x1) * 4 + wpa);
    		myset(s,c,(x2-x1+1)<<2);
    	}
    } else {
    	uint32_t *s1;
    	for(y=y1*2;y<=y2*2;y+=2){
    		s=(uint32_t *)((y * maxW + x1) * 4 + wpa);
    		s1=(uint32_t *)(((y+1) * maxW + x1) * 4 + wpa);
    		myset(s,c,(x2-x1+1)<<2);
    		myset(s1,c,(x2-x1+1)<<2);
    	}
    }
    if(cursorhidden)showcursor(0, xcursor,ycursor);
}

void __attribute__((section(".extra"))) DrawRectangle16(int x1, int y1, int x2, int y2, int c){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int y, t;
    uint16_t *s, sc;
    uint32_t bulkcolour;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    if((x1<0 && x2<0) || (y1<0 && y2<0) || (x1>=maxW && x2>=maxW) || (y1>=maxH && y2>=maxH))return;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) x1 = 0;
    if(x1 >= maxW) x1 = maxW - 1;
    if(x2 < 0) x2 = 0;
    if(x2 >= maxW) x2 = maxW - 1;
    if(y1 < 0) y1 = 0;
    if(y1 >= maxH) y1 = maxH - 1;
    if(y2 < 0) y2 = 0;
    if(y2 >= maxH) y2 = maxH - 1;
    int cursorhidden=0;
    if(cursoron && WritePage==(VideoColour == 12 ? 1 : 0 ) )
			if( !(xcursor + wcursor < x1 ||
				xcursor > x2 ||
				ycursor + hcursor < y1 ||
				ycursor > y2)){

			hidecursor(0);
			cursorhidden=1;
    	}
    // convert the colours to 565 format
    uint16_t red, green, blue, trans;
    if(VideoColour==16){
    	red=BIT_5[((c & 0xFF0000)>>19)]<<11;
        green=BIT_6[((c & 0xFF00)>>10)]<<5;
        blue=BIT_5[((c & 0xFF)>>3)];
        sc=red|green|blue;
    } else {
    	red=BIT_4[((c & 0xFF0000)>>20)]<<8;
        green=BIT_4[((c & 0xFF00)>>12)]<<4;
        blue=BIT_4[((c & 0xFF)>>4)];
        trans=((c & 0xF000000)>>12);
        sc=red|green|blue|trans;
    }
    bulkcolour=sc | (sc<<16);
    if(PageTable[WritePage].expand==0){
    	for(y=y1;y<=y2;y++){
    		s=(uint16_t *)((y * maxW + x1) * 2 + wpa);
    		myset(s,bulkcolour,(x2-x1+1)<<1);
    	}
    } else {
    	uint16_t *s1;
    	for(y=y1*2;y<=y2*2;y+=2){
    		s=(uint16_t *)((y * maxW + x1) * 2 + wpa);
    		s1=(uint16_t *)(((y+1) * maxW + x1) * 2 + wpa);
    		myset(s,bulkcolour,(x2-x1+1)<<1);
    		myset(s1,bulkcolour,(x2-x1+1)<<1);
    	}
    }
    if(cursorhidden)showcursor(0, xcursor,ycursor);
}
void __attribute__((section(".extra"))) DrawRectangle8(int x1, int y1, int x2, int y2, int c){
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
    int y, t;
    uint8_t *s, *s1, sc;
    uint32_t bulkcolour;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    if((x1<0 && x2<0) || (y1<0 && y2<0) || (x1>=maxW && x2>=maxW) || (y1>=maxH && y2>=maxH))return;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) x1 = 0;
    if(x1 >= maxW) x1 = maxW - 1;
    if(x2 < 0) x2 = 0;
    if(x2 >= maxW) x2 = maxW - 1;
    if(y1 < 0) y1 = 0;
    if(y1 >= maxH) y1 = maxH - 1;
    if(y2 < 0) y2 = 0;
    if(y2 >= maxH) y2 = maxH - 1;
    int cursorhidden=0;
    if(cursoron && WritePage==(VideoColour == 12 ? 1 : 0 ) )
		if( !(xcursor + wcursor < x1 ||
			xcursor > x2 ||
			ycursor + hcursor < y1 ||
			ycursor > y2)){
		hidecursor(0);
		cursorhidden=1;
    	}
    // convert the colours to 332 format
    sc = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
    bulkcolour=sc | (sc<<8) | (sc<<16) | (sc <<24);
    if(PageTable[WritePage].expand==0){
    	for(y=y1;y<=y2;y++){
    	    s=(uint8_t *)((y * maxW + x1) + wpa);
    		myset(s,bulkcolour,x2-x1+1);
    	}
    } else {
    	for(y=y1*2;y<=y2*2;y+=2){
       		s=(uint8_t *)((y * maxW + x1) + wpa);
       		s1=(uint8_t *)(((y+1) * maxW + x1) + wpa);
    		myset(s,bulkcolour,x2-x1+1);
    		myset(s1,bulkcolour,x2-x1+1);
    	}
    }
    if(cursorhidden)showcursor(0, xcursor,ycursor);
}

void __attribute__((section(".extra"))) ReadBuffer32(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
	uint32_t *s,l;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
    int maxW=PageTable[ReadPage].xmax;
	int maxH=PageTable[ReadPage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
	int Scale=(PageTable[ReadPage].expand ? 2 : 1);
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
	for(y=y1*Scale;y<=y2*Scale;y+=Scale){
	   routinechecks(1);
	   s=(uint32_t *)((y * maxW + x1) * 4 + rpa);
	   for(x=x1;x<=x2;x++){
			if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
				l=*s++;
				*p++ = l & 0xFF;
				*p++ = (l>>8)& 0xFF;
				*p++ = (l>>16)& 0xFF;
			} else {
				s+=4;
				p+=3;
			}
		}
	}
}

void __attribute__((section(".extra"))) ReadBuffer16(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
	uint16_t *s,l;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
    int maxW=PageTable[ReadPage].xmax;
	int maxH=PageTable[ReadPage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
	int Scale=(PageTable[ReadPage].expand ? 2 : 1);
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(VideoColour==16){
        for(y=y1*Scale;y<=y2*Scale;y+=Scale){
        	routinechecks(1);
           s=(uint16_t *)((y * maxW + x1) * 2 + rpa);
           for(x=x1;x<=x2;x++){
            	if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
            		l=*s++;
            		*p++ = RBIT_5[(l & 0x1F)];
            		*p++ = RBIT_6[(l & 0x7E0) >>5];
            		*p++ = RBIT_5[(l & 0xF800) >>11];
            	} else {
            		s+=2;
            		p+=3;
            	}
            }
        }
    } else {
        for(y=y1*Scale;y<=y2*Scale;y+=Scale){
        	routinechecks(1);
           s=(uint16_t *)((y * maxW + x1) * 2 + rpa);
           for(x=x1;x<=x2;x++){
            	if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
            		l=*s++;
            		*p++ = RBIT_4[(l & 0xF)];
            		*p++ = RBIT_4[((l & 0xF0)>>4)];
            		*p++ = RBIT_4[(l & 0xF00)>>8];
            		*p++ = (l & 0xF000)>>12;
            	} else {
            		s+=2;
            		p+=4;
            	}
            }
        }
    }
 }

void __attribute__((section(".extra"))) ReadBuffer8(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y,div;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
    unsigned char sc, *s;
    int maxW=PageTable[ReadPage].xmax;
	int maxH=PageTable[ReadPage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
	if(Option.colourmap==0)div=192;
	else if(Option.colourmap==1)div=224;
	else div=255;
	int Scale=(PageTable[ReadPage].expand ? 2 : 1);
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    for(y=y1*Scale;y<=y2*Scale;y+=Scale){
    	routinechecks(1);
        s=(uint8_t *)(y * maxW + x1 + rpa);
        for(x=x1;x<=x2;x++){
        	if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
        		sc=*s++;
        		*p++ = (CLUT[sc] & 0xFF)*255/div;
        		*p++ = ((CLUT[sc]>>8) & 0xFF)*255/div;
        		*p++ = ((CLUT[sc]>>16) & 0xFF)*255/div;
        	} else {
        		s++;
        		p+=3;
        	}
        }
    }
}

void __attribute__((section(".extra"))) ReadBufferFast32(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
    // make sure the coordinates are kept within the display area
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
    int maxW=PageTable[ReadPage].xmax;
	int maxH=PageTable[ReadPage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
	int Scale=(PageTable[ReadPage].expand ? 2 : 1);
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
	uint32_t *s, *pp=(uint32_t *)p;
	for(y=y1*Scale;y<=y2*Scale;y+=Scale){
		routinechecks(1);
		s=(uint32_t *)((y * maxW + x1) * 4 + rpa);
		for(x=x1;x<=x2;x++){
			if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
				*pp++ = *s++;
			} else {
				s++;
				pp++;
			}
		}
	}
}

void __attribute__((section(".extra"))) ReadBufferFast16(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
    // make sure the coordinates are kept within the display area
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
    int maxW=PageTable[ReadPage].xmax;
	int maxH=PageTable[ReadPage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
	int Scale=(PageTable[ReadPage].expand ? 2 : 1);
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if((x1 % 2)==0 && (x2 % 2)==1){
        uint32_t *s, *pp=(uint32_t *)p;
    	for(y=y1*Scale;y<=y2*Scale;y+=Scale){
        	routinechecks(1);
    		s=(uint32_t *)((y * maxW + x1) * 2 + rpa);
    		for(x=x1;x<=x2;x+=2){
    			if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
    				*pp++ = *s++;
    			} else {
    				s++;
    				pp++;
    			}
    		}
    	}
    } else {
        uint16_t *s, *pp=(uint16_t *)p;
    	for(y=y1*Scale;y<=y2*Scale;y+=Scale){
        	routinechecks(1);
    		s=(uint16_t *)((y * maxW + x1) * 2 + rpa);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
    				*pp++ = *s++;
    			} else {
    				s++;
    				pp++;
    			}
    		}
    	}
    }
}
void __attribute__((section(".extra"))) ReadBufferFast8(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
    int maxW=PageTable[ReadPage].xmax;
	int maxH=PageTable[ReadPage].ymax;
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
	int Scale=(PageTable[ReadPage].expand ? 2 : 1);
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if((x1 % 2)==0 && (x2 % 2)==1){
        uint16_t *s, *pp=(uint16_t *)p;
    	for(y=y1*Scale;y<=y2*Scale;y+=Scale){
        	routinechecks(1);
    		s=(uint16_t *)((y * maxW + x1) + rpa);
    		for(x=x1;x<=x2;x+=2){
    			if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
    				*pp++ = *s++;
    			} else {
    				s++;
    				pp++;
    			}
    		}
    	}
    } else {
        unsigned char *s;
    	for(y=y1*Scale;y<=y2*Scale;y+=Scale){
    		s=(uint8_t *)(y * maxW + x1 + rpa);
        	routinechecks(1);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH*Scale){
    				*p++ = *s++;
    			} else {
    				s++;
    				p++;
    			}
    		}
    	}
    }
}
int ReadPixelFast(int x, int y) {
    int maxW=PageTable[ReadPage].xmax;
	int maxH=PageTable[ReadPage].ymax;
	uint32_t rpa=(uint32_t)PageTable[ReadPage].address;
	if(optiony)y=maxH-1-y;
	if(x>=maxW || y>=maxH)return 0;
	int Scale=(PageTable[ReadPage].expand ? 2 : 1);
	if(VideoColour==8)	return (int)*(char *)((y * Scale * maxW + x) + rpa);
	else if(VideoColour<=16) return (int)*(uint16_t *)((y * Scale * maxW + x) *2 + rpa);
	else return (int)*(uint32_t *)((y * Scale * maxW + x) *4 + rpa);
 }

void __attribute__((section(".extra"))) DrawBuffer32(int x1, int y1, int x2, int y2, char* p, int skip) {
    int x, y, t;
    uint32_t *sc;
    union colourmap
    {
    char rgbbytes[4];
    uint32_t rgb;
    } c;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    t=0;
    int cursorhidden=0;
    if(cursoron)
		if( !(xcursor + wcursor < x1 ||
			xcursor > x2 ||
			ycursor + hcursor < y1 ||
			ycursor > y2)){
		hidecursor(0);
		cursorhidden=1;
    	}
    if(Scale==1){
    	for(y=y1;y<=y2;y++){
        	routinechecks(1);
    		sc=(uint32_t *)((y * maxW + x1) * 4 + wpa);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH){
    				if(skip & 2){
        				c.rgbbytes[3]=0xFF; //assume solid colour
        				c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[0]=*p++;
        				if(skip & 1)c.rgbbytes[3]=*p++; //ARGB8888 so set transparency
    				} else {
        				c.rgbbytes[3]=0;
        				c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[2]=*p++;
        				if(skip & 1)p++;
    				}
    			    *sc=c.rgb;
    			} else {
    				p+=(skip & 1) ? 4 : 3;
    			}
    		    sc++;
    		}
    	}
    } else {
    	uint32_t *s1;
    	for(y=y1*2;y<=y2*2;y+=2){
        	routinechecks(1);
    		sc=(uint32_t *)((y * maxW + x1) * 4 + wpa);
    		s1=(uint32_t *)(((y+1) * maxW + x1) * 4 + wpa);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    				if(skip & 2){
        				c.rgbbytes[3]=0xFF;
        				c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[0]=*p++;
        				if(skip & 1)c.rgbbytes[3]=*p++; //ARGB8888 so set transparency
    				} else {
        				c.rgbbytes[3]=0;
        				c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[2]=*p++;
        				if(skip & 1)p++;
    				}
					*sc=c.rgb;
					*s1=*sc;
    			} else {
    				p+=(skip & 1) ? 4 : 3;
    			}
				sc++;
				s1++;
    		}
    	}
    }
    if(cursorhidden)showcursor(0, xcursor,ycursor);

}

void __attribute__((section(".extra"))) DrawBuffer16(int x1, int y1, int x2, int y2, char* p, int skip) {
    int x, y, t;
    uint16_t *sc;
    union colourmap
    {
    char rgbbytes[4];
    uint32_t rgb;
    } c;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    uint16_t red, green, blue, trans, colour;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    t=0;
    int deftrans=skip & 0xF0;
    int cursorhidden=0;
    if(cursoron)
		if( !(xcursor + wcursor < x1 ||
			xcursor > x2 ||
			ycursor + hcursor < y1 ||
			ycursor > y2)){
		hidecursor(0);
		cursorhidden=1;
    	}
    if(Scale==1){
    	for(y=y1;y<=y2;y++){
        	routinechecks(1);
    		sc=(uint16_t *)((y * maxW + x1) * 2 + wpa);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH){
    				if(skip & 2){
        				c.rgbbytes[3]=0xF0; //assume solid colour
        				c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[0]=*p++;
        				if(skip & 1)c.rgbbytes[3]=*p++; //ARGB8888 so set transparency
    				} else {
        				c.rgbbytes[3]=0;
        				c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[2]=*p++;
        				if(skip & 1)p++;
    				}
    // convert the colours to 565 format)
    			    if(VideoColour==16){
    			    	red=BIT_5[((c.rgb & 0xFF0000)>>19)]<<11;
    			        green=BIT_6[((c.rgb & 0xFF00)>>10)]<<5;
    			        blue=BIT_5[((c.rgb & 0xFF)>>3)];
    			        trans=c.rgbbytes[3];
    			        if(trans<deftrans)trans=0;
    			        else trans = 0xF0;
    			        if(trans || (skip & 4)==0)*sc=red|green|blue; //output if not transparent and transparency enabled
    			    } else {
    			    	red=BIT_4[((c.rgb & 0xFF0000)>>20)]<<8;
    			        green=BIT_4[((c.rgb & 0xFF00)>>12)]<<4;
    			        blue=BIT_4[((c.rgb & 0xFF)>>4)];
    			        trans=c.rgbbytes[3];
    			        if(trans<deftrans)trans=0;
    			        else trans = 0xF0;
    			        colour=red|green|blue|((skip & 4)==0 ? 0xF000 : trans<<8);
    			        if(trans)*sc=colour;
    			    }
    			} else {
    				p+=(skip & 1) ? 4 : 3;
    			}
    		    sc++;
    		}
    	}
    } else {
    	uint16_t *s1;
    	for(y=y1*2;y<=y2*2;y+=2){
        	routinechecks(1);
    		sc=(uint16_t *)((y * maxW + x1) * 2 + wpa);
    		s1=(uint16_t *)(((y+1) * maxW + x1) * 2 + wpa);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    				if(skip & 2){
        				c.rgbbytes[3]=0xF0;
        				c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[0]=*p++;
        				if(skip & 1)c.rgbbytes[3]=*p++; //ARGB8888 so set transparency
    				} else {
        				c.rgbbytes[3]=0;
        				c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[2]=*p++;
        				if(skip & 1)p++;
    				}
    // convert the colours to 565 format
    			    if(VideoColour==16){
    			    	red=BIT_5[((c.rgb & 0xFF0000)>>19)]<<11;
    			        green=BIT_6[((c.rgb & 0xFF00)>>10)]<<5;
    			        blue=BIT_5[((c.rgb & 0xFF)>>3)];
    			        trans=c.rgbbytes[3];
    			        if(trans<deftrans)trans=0;
    			        else trans = 0xF0;
    			        if(trans || (skip & 4)==0){
    			        	*sc=red|green|blue; //output if not
    		   				*s1=*sc;
    			        }
    			    } else {
    			    	red=BIT_4[((c.rgb & 0xFF0000)>>20)]<<8;
    			        green=BIT_4[((c.rgb & 0xFF00)>>12)]<<4;
    			        blue=BIT_4[((c.rgb & 0xFF)>>4)];
    			        trans=c.rgbbytes[3];
    			        if(trans<deftrans)trans=0;
    			        else trans =0xF0;
    			        colour=red|green|blue|((skip & 4)==0 ? 0xF000 : trans<<8);
    			        if(trans){
    			        	*sc=colour;
    		   				*s1=*sc;
    			        }
    			    }
    			} else {
    				p+=(skip & 1) ? 4 : 3;
    			}
				sc++;
				s1++;
    		}
    	}
    }
    if(cursorhidden)showcursor(0, xcursor,ycursor);
}
void __attribute__((section(".extra"))) DrawBuffer8(int x1, int y1, int x2, int y2, char* p, int skip) {
    int x, y, t;
    unsigned char *s;
    union colourmap
    {
    char rgbbytes[4];
    uint16_t argb[2];
    uint32_t rgb;
    } c;
    c.rgb=0;
    int trans;
    int deftrans=skip & 0xF0;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
    // make sure the coordinates are kept within the display area
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    t=0;
    int cursorhidden=0;
    if(cursoron)
		if( !(xcursor + wcursor < x1 ||
			xcursor > x2 ||
			ycursor + hcursor < y1 ||
			ycursor > y2)){
		hidecursor(0);
		cursorhidden=1;
    	}
    if(Scale==1){
    	for(y=y1;y<=y2;y++){
        	routinechecks(1);
    		s=(uint8_t *)(y * maxW + x1 + wpa);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH){
    				if(skip & 2){
        				c.rgbbytes[3]=0;
        				c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[0]=*p++;
        				if(skip & 1)c.rgbbytes[3]=(*p++) & 0xF0; //ARGB8888 so set transparency
    				} else if(skip & 8) {
    					c.rgbbytes[0]=*p++;
    					c.rgbbytes[1]=*p++;
    				} else {
        				c.rgbbytes[3]=0;
        				c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[2]=*p++;
        				if(skip & 1)p++;
    				}
    				if(skip & 8){
    					*s++ = ((c.argb[0] & 0b1110000000000000) >> 8) |
    							((c.argb[0] & 0b11100000000) >> 6) |
								((c.argb[0] & 0b11000) >> 3);
    				} else {
    				// convert the colours to 332 format
    			        trans=c.rgbbytes[3];
    			        if(trans<deftrans)trans=0;
    			        else trans = 0xF0;
    					if(trans || (skip & 4)==0) *s = (c.rgbbytes[2] & 0b11100000) | ((c.rgbbytes[1]  & 0b11100000)>>3) | ((c.rgbbytes[0] & 0b11000000)>>6) ;
    					s++;
    				}
     			} else {
    				s++;
    				p+=(skip & 8) ? 2 : ((skip & 1) ? 4 : 3);
    			}
    		}
    	}
    } else {
    	uint8_t *s1;
    	for(y=y1*2;y<=y2*2;y+=2){
        	routinechecks(1);
    		s=(uint8_t *)(y * maxW + x1 + wpa);
    		s1=(uint8_t *)((y+1) * maxW + x1 + wpa);
    		for(x=x1;x<=x2;x++){
    			if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    				if(skip & 2){
        				c.rgbbytes[3]=0;
        				c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[0]=*p++;
        				if(skip & 1)c.rgbbytes[3]=(*p++) & 0xF0; //ARGB8888 so set transparency
    				} else if(skip & 8) {
    					c.rgbbytes[0]=*p++;
    					c.rgbbytes[1]=*p++;
    				} else {
        				c.rgbbytes[3]=0;
        				c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        				c.rgbbytes[1]=*p++;
        				c.rgbbytes[2]=*p++;
        				if(skip & 1)p++;
    				}
    				if(skip & 8){
    					*s = *s1 =((c.argb[0] & 0b1110000000000000) >> 8) |
    							((c.argb[0] & 0b11100000000) >> 6) |
								((c.argb[0] & 0b11000) >> 3);
    					s++;
    					s1++;
    				} else {
    				// convert the colours to 332 format
    			        trans=c.rgbbytes[3];
    			        if(trans<deftrans)trans=0;
    			        else trans = 0xF0;
    					if(trans || (skip & 4)==0){
    						*s = (c.rgbbytes[2] & 0b11100000) | ((c.rgbbytes[1]  & 0b11100000)>>3) | ((c.rgbbytes[0] & 0b11000000)>>6) ;
    						*s1= *s;
    					}
    					s++;
    					s1++;
    				}
    			} else {
    				s++;
    				s1++;
    				p+=(skip & 8) ? 2 : ((skip & 1) ? 4 : 3);
    			}
    		}
    	}
    }
    if(cursorhidden)showcursor(0, xcursor,ycursor);
}

void __attribute__((section(".extra"))) DrawBufferFast32(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
    // make sure the coordinates are kept within the display area
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
	uint32_t *s, *pp=(uint32_t *)p;
	if(Scale==1){
		for(y=y1;y<=y2;y++){
			routinechecks(1);
			s=(uint32_t *)((y * maxW + x1) * 4 + wpa);
			for(x=x1;x<=x2;x++){
				if(x>=0 && x<maxW && y>=0 && y<maxH){
					*s++=*pp++; //this order swaps the bytes to match the .BMP file
				} else {
					s++;
					pp++;
				}
			}
		}
	} else {
		uint32_t *s1;
		for(y=y1*2;y<=y2*2;y+=2){
			routinechecks(1);
			s=(uint32_t *)((y * maxW + x1) * 4 + wpa);
			s1=(uint32_t *)(((y+1) * maxW + x1) * 4 + wpa);
			for(x=x1;x<=x2;x++){
				if(x>=0 && x<maxW && y>=0 && y<maxH*2){
					*s1++=*pp;
					*s++=*pp++; //this order swaps the bytes to match the .BMP file
				} else {
					s1++;
					s++;
					pp++;
				}
			}
		}
	}

}

void __attribute__((section(".extra"))) DrawBufferFast16(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
    // make sure the coordinates are kept within the display area
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if((x1 % 2)==0 && (x2 % 2)==1){
        uint32_t *sp, *ppp=(uint32_t *)p;
    	if(Scale==1){
    		for(y=y1;y<=y2;y++){
            	routinechecks(1);
    			sp=(uint32_t *)((y * maxW + x1) * 2 + wpa);
    			for(x=x1;x<=x2;x+=2){
    				if(x>=0 && x<maxW && y>=0 && y<maxH){
    					*sp++=*ppp++; //this order swaps the bytes to match the .BMP file
    				} else {
    					sp++;
    					ppp++;
    				}
    			}
    		}
    	} else {
    		uint32_t *s1p;
    		for(y=y1*2;y<=y2*2;y+=2){
            	routinechecks(1);
    			sp=(uint32_t *)((y * maxW + x1) * 2 + wpa);
    			s1p=(uint32_t *)(((y+1) * maxW + x1) * 2 + wpa);
    			for(x=x1;x<=x2;x+=2){
    				if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    					*s1p++=*ppp;
    					*sp++=*ppp++; //this order swaps the bytes to match the .BMP file
    				} else {
    					s1p++;
    					sp++;
    					ppp++;
    				}
    			}
    		}
    	}
    } else {
        unsigned short *s, *pp=(uint16_t *)p;
    	if(Scale==1){
    		for(y=y1;y<=y2;y++){
            	routinechecks(1);
    			s=(uint16_t *)((y * maxW + x1) * 2 + wpa);
    			for(x=x1;x<=x2;x++){
    				if(x>=0 && x<maxW && y>=0 && y<maxH){
    					*s++=*pp++; //this order swaps the bytes to match the .BMP file
    				} else {
    					s++;
    					pp++;
    				}
    			}
    		}
    	} else {
    		uint16_t *s1;
    		for(y=y1*2;y<=y2*2;y+=2){
            	routinechecks(1);
    			s=(uint16_t *)((y * maxW + x1) * 2 + wpa);
    			s1=(uint16_t *)(((y+1) * maxW + x1) * 2 + wpa);
    			for(x=x1;x<=x2;x++){
    				if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    					*s1++=*pp;
    					*s++=*pp++; //this order swaps the bytes to match the .BMP file
    				} else {
    					s1++;
    					s++;
    					pp++;
    				}
    			}
    		}
    	}
    }
}
void __attribute__((section(".extra"))) DrawBufferFast8(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
//    PInt(x1);PIntComma(y1);PIntComma(x2);PIntComma(y2);PRet();
    // make sure the coordinates are kept within the display area
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	int Scale=(PageTable[WritePage].expand ? 2 : 1);
	if(optiony)y1=maxH-1-y1;
	if(optiony)y2=maxH-1-y2;
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if((x1 % 2)==0 && (x2 % 2)==1){
        uint16_t *sp, *pp=(uint16_t *)p;
    	if(Scale==1){
    		for(y=y1;y<=y2;y++){
            	routinechecks(1);
    			sp=(uint16_t *)((y * maxW + x1) + wpa);
    			for(x=x1;x<=x2;x+=2){
    				if(x>=0 && x<maxW && y>=0 && y<maxH){
    					*sp++=*pp++;
    				} else {
    					sp++;
    					pp++;
    				}
    			}
    		}
    	} else {
    		uint16_t *s1p;
    		for(y=y1*2;y<=y2*2;y+=2){
            	routinechecks(1);
    			sp=(uint16_t *)((y * maxW + x1) + wpa);
    			s1p=(uint16_t *)(((y+1) * maxW + x1) + wpa);
    			for(x=x1;x<=x2;x+=2){
    				if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    					*sp++=*pp;
    					*s1p++=*pp++;
    				} else {
    					sp++;
    					s1p++;
    					pp++;
    				}
    			}
    		}
    	}
    } else {
        unsigned char *s;
    	if(Scale==1){
    		for(y=y1;y<=y2;y++){
            	routinechecks(1);
    			s=(uint8_t *)(y * maxW + x1 + wpa);
    			for(x=x1;x<=x2;x++){
    				if(x>=0 && x<maxW && y>=0 && y<maxH){
    					*s++=*p++;
    				} else {
    					s++;
    					p++;
    				}
    			}
    		}
    	} else {
    		uint8_t *s1;
     		for(y=y1*2;y<=y2*2;y+=2){
    			s=(uint8_t *)(y * maxW + x1 + wpa);
    			s1=(uint8_t *)((y+1) * maxW + x1 + wpa);
            	routinechecks(1);
    			for(x=x1;x<=x2;x++){
    				if(x>=0 && x<maxW && y>=0 && y<maxH*2){
    					*s++=*p;
    					*s1++=*p++;
    				} else {
    					s++;
    					s1++;
    					p++;
    				}
    			}
    		}
    	}
    }
}

void __attribute__((section(".extra"))) DrawPixelFast(int x, int y, int c){
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	if(optiony)y=maxH-1-y;
	if(x>=maxW || y>=maxH || x<0 || y<0)return;
	if(VideoColour==8){
		if(PageTable[WritePage].expand==0){
			*(uint8_t *)((y * maxW + x) + wpa)=(uint8_t)c;
		} else {
			*(uint8_t *)((y * 2 * maxW + x) + wpa)=(uint8_t)c;
			*(uint8_t *)(((y * 2 + 1) * maxW + x) + wpa)=(uint8_t)c;
		}
	} else if(VideoColour<=16) {
		if(PageTable[WritePage].expand==0){
			*(uint16_t *)((y * maxW + x) * 2 + wpa)=(uint16_t)c;
		} else {
			*(uint16_t *)((y * 2 * maxW + x) * 2 + wpa)=(uint16_t)c;
			*(uint16_t *)(((y * 2 + 1) * maxW + x) * 2 + wpa)=(uint16_t)c;
		}
	} else {
		if(PageTable[WritePage].expand==0){
			*(uint32_t *)((y * maxW + x) * 4 + wpa)=(uint32_t)c;
		} else {
			*(uint32_t *)((y * 2 * maxW + x) * 4 + wpa)=(uint32_t)c;
			*(uint32_t *)(((y * 2 + 1) * maxW + x) * 4 + wpa)=(uint32_t)c;
		}
	}
}
void __attribute__((section(".extra"))) DrawHLineFast(int x1, int y, int x2, int c){
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t wpa=(uint32_t)PageTable[WritePage].address;
	if(optiony)y=maxH-1-y;
	if(y<0 || y>= maxH || (x1<0 && x2<0) || (x1>=maxW && x2>=maxW))return;
	if (x1 > x2) {
		swap(x1, x2);
	}
	if(x1<0)x1=0;
	if(x2>=maxW)x2=maxW-1;
	if(VideoColour==8){
		int bulkcolour=c | (c<<8) | (c<<16) | (c <<24);
		if(PageTable[WritePage].expand==0){
			uint8_t *p=(uint8_t *)((y * maxW + x1) + wpa);
			myset(p,bulkcolour,x2-x1+1);
		} else {
			uint8_t *p1=(uint8_t *)((y * 2 * maxW + x1) + wpa);
			uint8_t *p2=(uint8_t *)(((y * 2 + 1) * maxW + x1) + wpa);
			myset(p1,bulkcolour,x2-x1+1);
			myset(p2,bulkcolour,x2-x1+1);
		}
	} else if(VideoColour<=16) {
	    int bulkcolour=c | (c<<16);
		if(PageTable[WritePage].expand==0){
			uint16_t *p=(uint16_t *)((y * maxW + x1) * 2 + wpa);
    		myset(p,bulkcolour,(x2-x1+1)<<1);
		} else {
			uint16_t *p1=(uint16_t *)((y * 2 * maxW + x1) * 2 + wpa);
			uint16_t *p2=(uint16_t *)(((y * 2 + 1) * maxW + x1) * 2 + wpa);
    		myset(p1,bulkcolour,(x2-x1+1)<<1);
    		myset(p2,bulkcolour,(x2-x1+1)<<1);
		}
	} else {
		if(PageTable[WritePage].expand==0){
			uint32_t *p=(uint32_t *)((y * maxW + x1) * 4 + wpa);
    		myset(p,c,(x2-x1+1)<<2);
		} else {
			uint32_t *p1=(uint32_t *)((y * 2 * maxW + x1) * 4 + wpa);
			uint32_t *p2=(uint32_t *)(((y * 2 + 1) * maxW + x1) * 4 + wpa);
    		myset(p1,c,(x2-x1+1)<<2);
    		myset(p2,c,(x2-x1+1)<<2);
		}
	}
}

void __attribute__((section(".extra"))) DrawBitmap32(int x1, int y1, int width, int height, int scale, int f, int b, unsigned char *bitmap){
    int i, j, k, m, t, x, y;
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
    // adjust when part of the bitmap is outside the displayable coordinates
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint32_t *wpa=(uint32_t *)PageTable[WritePage].address;
	if(optiony)y1=maxH-1-y1;
    if(x1>=maxW || y1>=maxH || x1+width*scale<0 || y1+height*scale<0)return;
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= maxW) XEnd = maxW - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= maxH) YEnd = maxH - 1;// the height of the bitmap will extend beyond the bottom margin
	if(PageTable[WritePage].expand==0){
		t = 0;
		for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
			for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
				y=vertCoord;
				if(vertCoord++ < 0) continue;                           // we are above the top of the screen
				if(vertCoord > maxH) {                                  // we have extended beyond the bottom of the screen
					return;
				}
				horizCoord = x1;
				for(k = 0; k < width; k++) {                            // step through each bit in a scan line
					for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
						x=horizCoord;
						if(horizCoord++ < 0) continue;                  // we have not reached the left margin
						if(horizCoord > maxW) continue;                 // we are beyond the right margin
							t= y * maxW + x;
							if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
								wpa[t]=f;
							} else {
								if(b != -1){
									wpa[t]=b;
								}
							}
						}
					}
				}
			}
	} else {
		int y1, t1;
		t1 = t = 0;
		for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
			for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
				y=vertCoord;
				if(vertCoord++ < 0) continue;                           // we are above the top of the screen
				if(vertCoord > maxH) {                                  // we have extended beyond the bottom of the screen
					return;
				}
				horizCoord = x1;
				for(k = 0; k < width; k++) {                            // step through each bit in a scan line
					for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
						x=horizCoord;
						if(horizCoord++ < 0) continue;                  // we have not reached the left margin
						if(horizCoord > maxW) continue;                 // we are beyond the right margin
							y1=y*2;
							t= y1 * maxW + x;
							t1= (y1+1) * maxW + x;
							if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
								wpa[t]=f;
								wpa[t1]=f;
							} else {
								if(b != -1){
									wpa[t]=b;
									wpa[t1]=b;
								}
							}
						}
					}
				}
			}
		}
}

//Print the bitmap of a char on the video output
//    x, y - the top left of the char
//    width, height - size of the char's bitmap
//    scale - how much to scale the bitmap
//	  fc, bc - foreground and background colour
//    bitmap - pointer to the bitmap
void __attribute__((section(".extra"))) DrawBitmap16(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){
    int i, j, k, m, t, x, y;
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
//    uint16_t *q=NULL, *q1=NULL;
    uint16_t f,b;
    // adjust when part of the bitmap is outside the displayable coordinates
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint16_t *wpa=(uint16_t *)PageTable[WritePage].address;
	if(optiony)y1=maxH-1-y1;
    if(x1>=maxW || y1>=maxH || x1+width*scale<0 || y1+height*scale<0)return;
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= maxW) XEnd = maxW - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= maxH) YEnd = maxH - 1;// the height of the bitmap will extend beyond the bottom margin
        uint16_t red, green, blue, trans;
        if(VideoColour==16){
        	red=BIT_5[((fc & 0xFF0000)>>19)]<<11;
            green=BIT_6[((fc & 0xFF00)>>10)]<<5;
            blue=BIT_5[((fc & 0xFF)>>3)];
            f=red|green|blue;
        	red=BIT_5[((bc & 0xFF0000)>>19)]<<11;
            green=BIT_6[((bc & 0xFF00)>>10)]<<5;
            blue=BIT_5[((bc & 0xFF)>>3)];
            b=red|green|blue;
        } else {
        	red=BIT_4[((fc & 0xFF0000)>>20)]<<8;
            green=BIT_4[((fc & 0xFF00)>>12)]<<4;
            blue=BIT_4[((fc & 0xFF)>>4)];
            trans=((fc & 0xF000000)>>12);
            f=red|green|blue|trans;
        	red=BIT_4[((bc & 0xFF0000)>>20)]<<8;
            green=BIT_4[((bc & 0xFF00)>>12)]<<4;
            blue=BIT_4[((bc & 0xFF)>>4)];
            trans=((bc & 0xF000000)>>12);
            b=red|green|blue|trans;
        }
        if(PageTable[WritePage].expand==0){
        	t = 0;
        	for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
        		for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
        			y=vertCoord;
        			if(vertCoord++ < 0) continue;                           // we are above the top of the screen
        			if(vertCoord > maxH) {                                  // we have extended beyond the bottom of the screen
        				return;
        			}
        			horizCoord = x1;
        			for(k = 0; k < width; k++) {                            // step through each bit in a scan line
        				for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
        					x=horizCoord;
        					if(horizCoord++ < 0) continue;                  // we have not reached the left margin
        					if(horizCoord > maxW) continue;                 // we are beyond the right margin
                        		t= y * maxW + x;
                        		if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        			wpa[t]=f;
                        		} else {
                        			if(bc != -1){
                        				wpa[t]=b;
                        			}
                        		}
        					}
        				}
        			}
        		}
        } else {
        	int y1, t1;
            t1 = t = 0;
            for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
                for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
                    y=vertCoord;
                    if(vertCoord++ < 0) continue;                           // we are above the top of the screen
                    if(vertCoord > maxH) {                                  // we have extended beyond the bottom of the screen
                        return;
                    }
                    horizCoord = x1;
                    for(k = 0; k < width; k++) {                            // step through each bit in a scan line
                        for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
                            x=horizCoord;
                            if(horizCoord++ < 0) continue;                  // we have not reached the left margin
                            if(horizCoord > maxW) continue;                 // we are beyond the right margin
                                y1=y*2;
                                t= y1 * maxW + x;
                                t1= (y1+1) * maxW + x;
                                if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                                	wpa[t]=f;
                                	wpa[t1]=f;
                                } else {
                                    if(bc != -1){
                                    	wpa[t]=b;
                                    	wpa[t1]=b;
                                    }
                                }
                            }
                        }
                    }
            	}
            }
}
void __attribute__((section(".extra"))) DrawBitmap8(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){
    int i, j, k, m, t, x, y;
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
    unsigned char f,b;
    // adjust when part of the bitmap is outside the displayable coordinates
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	uint8_t *wpa=(uint8_t *)PageTable[WritePage].address;
	if(optiony)y1=maxH-1-y1;
    if(x1>=maxW || y1>=maxH || x1+width*scale<0 || y1+height*scale<0)return;
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= maxW) XEnd = maxW - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= maxH) YEnd = maxH - 1;// the height of the bitmap will extend beyond the bottom margin
	f = ((fc & 0b111000000000000000000000)>>16) | ((fc & 0b1110000000000000)>>11) | ((fc & 0b11000000)>>6);
	b = ((bc & 0b111000000000000000000000)>>16) | ((bc & 0b1110000000000000)>>11) | ((bc & 0b11000000)>>6);
    // switch to SPI enhanced mode for the bulk transfer
    if(PageTable[WritePage].expand==0){
        t = 0;
    	for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
    		for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
    			y=vertCoord;
    			if(vertCoord++ < 0) continue;                           // we are above the top of the screen
    			if(vertCoord > maxH) {                                  // we have extended beyond the bottom of the screen
    				return;
    			}
    			horizCoord = x1;
    			for(k = 0; k < width; k++) {                            // step through each bit in a scan line
    				for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
    					x=horizCoord;
    					if(horizCoord++ < 0) continue;                  // we have not reached the left margin
    					if(horizCoord > maxW) continue;                 // we are beyond the right margin
                    		t= y * maxW + x;
                    		if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                    			wpa[t]=f;
                    		} else {
                    			if(bc != -1){
                    				wpa[t]=b;
                    			}
                    		}
    					}
    				}
    			}
    		}
    } else {
    	int y1, t1;
        t1 = t = 0;
        for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
            for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
                y=vertCoord;
                if(vertCoord++ < 0) continue;                           // we are above the top of the screen
                if(vertCoord > maxH) {                                  // we have extended beyond the bottom of the screen
                    return;
                }
                horizCoord = x1;
                for(k = 0; k < width; k++) {                            // step through each bit in a scan line
                    for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
                        x=horizCoord;
                        if(horizCoord++ < 0) continue;                  // we have not reached the left margin
                        if(horizCoord > maxW) continue;                 // we are beyond the right margin
                        y1=y*2;
                        t= y1 * maxW + x;
                        t1= (y1+1) * maxW + x;
                        if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        	wpa[t]=f;
                        	wpa[t1]=f;
                        } else {
                            if(bc != -1){
                            	wpa[t]=b;
                            	wpa[t1]=b;
                            }
                        }
                    }
                }
            }
    	}
    }
}

