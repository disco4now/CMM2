/***************************************************************************
CMM2 MMBasic
MiscSTM32.c
Handles the a few miscellaneous functions.

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


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "ltdc.h"
extern void CallCFuncT5(void);                                      // this is implemented in CFunction.c
extern unsigned int CFuncT5;                                        // we should call the CFunction T5 interrupt function if this is non zero
extern void ConfigSDCard(char *p);
volatile MMFLOAT VCC=3.3;
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
extern RTC_HandleTypeDef hrtc;
char *FrameInterrupt;
volatile int Framecomplete,DoSound=0;
const uint16_t nunaddr=0xA4;
const uint16_t mouseaddr=82;
extern I2C_HandleTypeDef hi2c1, hi2c2, hi2c4;
extern unsigned int I2C_Timeout, I2C2_Timeout, I2C3_Timeout;
extern unsigned int I2C1_enabled,  I2C2_enabled, I2C3_enabled;									// I2C enable marker
extern void rtcChangeSet(void);
char *nun1Interruptc=NULL;
char *nun2Interruptc=NULL;
char *nun3Interruptc=NULL;
char *nun1Interruptz=NULL;
char *nun2Interruptz=NULL;
char *nun3Interruptz=NULL;
volatile int nun1foundz=0,nun2foundz=0,nun3foundz=0;
volatile int nun1foundc=0,nun2foundc=0,nun3foundc=0;
char *mouse1Interruptc=NULL;
char *mouse2Interruptc=NULL;
char *mouse3Interruptc=NULL;
char *mouse1Interruptz=NULL;
char *mouse2Interruptz=NULL;
char *mouse3Interruptz=NULL;
char *mouse1Interruptu=NULL;
char *mouse2Interruptu=NULL;
char *mouse3Interruptu=NULL;
volatile int mouse1foundz=0,mouse2foundz=0,mouse3foundz=0;
volatile int mouse1foundc=0,mouse2foundc=0,mouse3foundc=0;
volatile int mouse1leftup=0,mouse2leftup=0,mouse3leftup=0;
volatile uint32_t nuntype[4]={0};
///////////////////////////////////////////////////////////////////////////////////////////////
// constants and functions used in the OPTION LIST command
const char *CaseList[] = {"", "LOWER", "UPPER"};
const char *KBrdList[] = {"", "UK", "US", "DE", "FR", "ES", "BE", "IT" };
const unsigned char nuninit[2]={0xF0,0x55};
const unsigned char nuninit2[2]={0xFB,0x0};
const unsigned char readcontroller[1]={0};
unsigned char mouseinit[10];
const unsigned char nuncalib[1]={0x20};
extern volatile unsigned int SleepTimer;
const unsigned char nunid[1]={0xFC};
int VideoMode, VideoColour, VideoBackground;
volatile int nun1=0, nun2=0, nun3=0;
volatile int classic1=0, classic2=0, classic3=0;
volatile int mouse1=0, mouse2=0, mouse3=0;
uint8_t nunbuff[10];
uint8_t mousebuff[10];
extern UART_HandleTypeDef huart1;
volatile struct s_nunstruct nunstruct[4];
volatile struct s_nunstruct mousestruct[4];
extern int Overclock;

uint32_t swap32(uint32_t in)
{
  in = __builtin_bswap32(in);
  return in;
}
void SRet(void){
    SerUSBPutS("\r\n");
}

void SInt(int64_t n) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    SerUSBPutS(s);
}

void SIntComma(int64_t n) {
	SerUSBPutS(", "); SInt(n);
}

void SIntH(unsigned long long int n) {
    char s[20];
    IntToStr(s, (int64_t)n, 16);
    SerUSBPutS(s);
}
void SIntHC(unsigned long long int n) {
	SerUSBPutS(", "); SIntH(n);
}

void SFlt(MMFLOAT flt){
	   char s[20];
	   FloatToStr(s, flt, 4,4, ' ');
	   SerUSBPutS(s);
}
void SFltComma(MMFLOAT n) {
	SerUSBPutS(", "); SFlt(n);
}

void PRet(void){
    MMPrintString("\r\n");
}

void PO(char *s,int m) {
	if(m==1)MMPrintString("OPTION ");
	else if(m==2)MMPrintString("DEFAULT ");
	else if(m==3)MMPrintString("CURRENT ");
	MMPrintString(s); MMPrintString(" ");
}

void PInt(int64_t n) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    MMPrintString(s);
}

void PIntComma(int64_t n) {
    MMPrintString(", "); PInt(n);
}

void PO2Str(char *s1, const char *s2, int m) {
    PO(s1,m); MMPrintString((char *)s2); MMPrintString("\r\n");
}


void PO2Int(char *s1, int64_t n) {
    PO(s1,1); PInt(n); MMPrintString("\r\n");
}

void PO3Int(char *s1, int64_t n1, int64_t n2) {
    PO(s1,1); PInt(n1); MMPrintString(",");PInt(n2); MMPrintString("\r\n");
}
void PIntH(unsigned long long int n) {
    char s[20];
    IntToStr(s, (int64_t)n, 16);
    MMPrintString(s);
}
void PIntHC(unsigned long long int n) {
    MMPrintString(", "); PIntH(n);
}

void PFlt(MMFLOAT flt){
	   char s[20];
	   FloatToStr(s, flt, 4,4, ' ');
	    MMPrintString(s);
}
void PFltComma(MMFLOAT n) {
    MMPrintString(", "); PFlt(n);
}
///////////////////////////////////////////////////////////////////////////////////////////////

void printoptions(void){
	LoadOptions();

	MMPrintString((G1Hardware ? (Overclock==0 ? MES_SIGNON504 : ((HAL_GetREVID()==0x1003 || G1Hardware>=2) ? MES_SIGNON400:MES_SIGNON480)) : (Overclock==0 ? MES_SIGNON504G2 : (HAL_GetREVID()==0x1003 ? MES_SIGNON400G2:MES_SIGNON480G2))));
	MMPrintString( "MMBasic " VERSION "\r\n" );

	if(Option.mode==8) 	PO2Str("Default mode 8","640x480",3);
    else if(Option.mode==9)	PO2Str("Default mode 9","1024x768",3);
    else if(Option.mode==10)	PO2Str("Default mode 10","848x480",3);
    else if(Option.mode==11)	PO2Str("Default mode 11","1280x720",3);
    else if(Option.mode==12)	PO2Str("Default mode 12","960x540",3);
    else if(Option.mode==14)	PO2Str("Default mode 14","960x540",3);
    else if(Option.mode==15)	PO2Str("Default mode 15","1280x1024",3);
    else if(Option.mode==16)	PO2Str("Default mode 16","1920x1080",3);
    else if(Option.mode==18)	PO2Str("Default mode 18","1024x600",3);
	if(VideoMode==1) 	PO2Str("VGA mode 800x600",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==2) 	PO2Str("VGA mode 640x400",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==3) 	PO2Str("VGA mode 320x200",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==4) 	PO2Str("VGA mode 480x432",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==5) 	PO2Str("VGA mode 240x216",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==6) 	PO2Str("VGA mode 256x240",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==7) 	PO2Str("VGA mode 320x240",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==8) 	PO2Str("VGA mode 640x480",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==9) 	PO2Str("VGA mode 1024x768",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==10) 	PO2Str("VGA mode 848x480",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==11) 	PO2Str("VGA mode 1280x720",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==12) 	PO2Str("VGA mode 960x540",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==13) 	PO2Str("VGA mode 400x300",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==14) 	PO2Str("VGA mode 960x540",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==15) 	PO2Str("VGA mode 1280x1024",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==16) 	PO2Str("VGA mode 1920x1080",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==17) 	PO2Str("VGA mode 1384x240",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
    else if(VideoMode==18) 	PO2Str("VGA mode 1024x600",(VideoColour==8 ? "RGB332" : (VideoColour==12 ? "ARGB444" : (VideoColour==16 ? "RGB565" : "ARGB8888"))),3);
//    if(Option.Height != 24 || Option.Width != 80){
    	char buff[30]={0};
    	sprintf(buff,"CURRENT DISPLAY %d,%d\r\n",Option.Height, Option.Width);
    	MMPrintString(buff);
//    }

    if(Option.Autorun == true) PO2Str("AUTORUN", "ON",1);
    if(Option.Baudrate != CONSOLE_BAUDRATE) PO2Int("BAUDRATE", Option.Baudrate);
    if(Option.Invert == true) PO2Str("CONSOLE", "INVERT",1);
    if(Option.Invert == 2) PO2Str("CONSOLE", "AUTO",1);
    if(!Option.colourmode) PO2Str("COLOURCODE", "OFF",1);
    if(Option.colourmode==-1) PO2Str("COLOURCODE", "REVERSE",1);
    if(Option.Listcase != CONFIG_TITLE) PO2Str("CASE", CaseList[(int)Option.Listcase],1);
    if(Option.Tab != 2){
    	char buff[20]={0};
    	sprintf(buff,"OPTION TAB %d\r\n",Option.Tab);
    	MMPrintString(buff);
    }
    if(Option.sleep){
    	char buff[20]={0};
    	sprintf(buff,"OPTION SLEEP %d\r\n",Option.sleep);
    	MMPrintString(buff);
    }
    if(Option.rtcdrive!=2){
    	if(Option.rtcdrive==0)PO2Str("RTC DRIVE", "LOW",1);
    	if(Option.rtcdrive==1)PO2Str("RTC DRIVE", "MEDIUMLOW",1);
    	if(Option.rtcdrive==3)PO2Str("RTC DRIVE", "HIGH",1);
    }
    if(Option.RTCinstalled)PO2Str("DS3231", "ON",1);
    if(Option.profile)PO2Str("PROFILING", "ON",1);
    if(OptionConsole == 0) PO2Str("CONSOLE","OFF",1);
    else if(OptionConsole == 1) PO2Str("CONSOLE","SERIAL",1);
    else if(OptionConsole == 2) PO2Str("CONSOLE","SCREEN",1);
    if(!Option.SerialPullup) PO2Str("SERIAL PULLUPS", "OFF",1);
    if(Option.USBKeyboard != NO_KEYBOARD){
    	PO("USBKEYBOARD",1);
    	MMPrintString((char *)KBrdList[(int)Option.USBKeyboard]);
    	if(Option.noLED)PIntComma(Option.noLED);
        MMPrintString("\r\n");

    }
    if(Option.CPUmode & 2)PO2Str("BASELINE", "ON",1);
    if(Option.CPUmode & 0x10)PO2Str("OVERCLOCK", "ON",1);
    if(Option.Mouse>=0)PO3Int("MOUSE",Option.Mouse, Option.Sensitivity);
    if(Option.ConsolePort!=3)PO2Int("CONSOLE PORT COM",Option.ConsolePort);
    if(Option.editfont == 3) PO2Str("EDIT FONT","VERY LARGE",1);
    if(Option.editfont == 7) PO2Str("EDIT FONT","SMALL",1);
    if(Option.editfont == 4) PO2Str("EDIT FONT","MEDIUM",1);
    if(Option.editfont == 2) PO2Str("EDIT FONT","LARGE",1);
    if(Option.ProgramStartCode<0)PO2Str("RAM","ON",1);
    else if(Option.ProgramStartCode>PROGRAMSTARTCODE)PO2Int("FLASH",Option.ProgramStartCode);
    if(Option.SDspeed==1)PO2Str("SD TIMING","FAST",1);
    if(Option.RTC_Calibrate){
    	char buff[30]={0};
    	sprintf(buff,"OPTION RTC CALIBRATE %d\r\n",Option.RTC_Calibrate);
    	MMPrintString(buff);
    }
    if(!(Option.RepeatStart==600 && Option.RepeatRate==150)){
    	char buff[40]={0};
    	sprintf(buff,"OPTION KEYBOARD REPEAT %d,%d\r\n",Option.RepeatStart, Option.RepeatRate);
    	MMPrintString(buff);
    }
    if(strlen((char *)Option.path))PO2Str("SEARCH PATH",(char *)Option.path,1);

    if(strlen((char *)Option.F11Key)){
    	char cc[64];
    	strcpy(cc,(char *)Option.F11Key);
    	if(cc[strlen(cc)-2]=='\r' && cc[strlen(cc)-2]=='\r'){
    		cc[strlen(cc)-2]=0;
    		strcat(cc,"<crlf>");
    	}

    	PO2Str("F11",(char *)cc,1);
    }
    if(strlen((char *)Option.F12Key)){
    	char cc[64];
    	strcpy(cc,(char *)Option.F12Key);
    	if(cc[strlen(cc)-2]=='\r' && cc[strlen(cc)-2]=='\r'){
    		cc[strlen(cc)-2]=0;
    		strcat(cc,"<crlf>");
    	}

    	PO2Str("F12",(char *)cc,1);
    }
    if(strlen((char *)Option.F15Key)){
    	char cc[64];
    	strcpy(cc,(char *)Option.F15Key);
    	if(cc[strlen(cc)-2]=='\r' && cc[strlen(cc)-2]=='\r'){
    		cc[strlen(cc)-2]=0;
    		strcat(cc,"<crlf>");
    	}

    	PO2Str("F15",(char *)cc,1);
    }

    if(strlen((char *)Option.F16Key)){
    	char cc[64];
    	strcpy(cc,(char *)Option.F16Key);
    	if(cc[strlen(cc)-2]=='\r' && cc[strlen(cc)-2]=='\r'){
    		cc[strlen(cc)-2]=0;
    		strcat(cc,"<crlf>");
    	}

    	PO2Str("F16",(char *)cc,1);
    }

    if(strlen((char *)Option.F19Key)){
    	char cc[64];
    	strcpy(cc,(char *)Option.F19Key);
    	if(cc[strlen(cc)-2]=='\r' && cc[strlen(cc)-2]=='\r'){
    		cc[strlen(cc)-2]=0;
    		strcat(cc,"<crlf>");
    	}

    	PO2Str("F19",(char *)cc,1);
    }
    if(strlen((char *)Option.F20Key)){
    	char cc[64];
    	strcpy(cc,(char *)Option.F20Key);
    	if(cc[strlen(cc)-2]=='\r' && cc[strlen(cc)-2]=='\r'){
    		cc[strlen(cc)-2]=0;
    		strcat(cc,"<crlf>");
    	}

    	PO2Str("F20",(char *)cc,1);
    }

    if(Option.MaxCtrls)PO2Int("MAXCTRLS", Option.MaxCtrls);
    return;

}
void nunproc(int chan){
	static int lastc[4]={0},lastz[4]={0};
	nunstruct[chan].x=nunbuff[0];
	nunstruct[chan].y=nunbuff[1];
	nunstruct[chan].ax=nunbuff[2]<<2;
	nunstruct[chan].ay=nunbuff[3]<<2;
	nunstruct[chan].az=nunbuff[4]<<2;
	nunstruct[chan].Z=(~(nunbuff[5] & 1)) & 1;
	nunstruct[chan].C=(~((nunbuff[5] & 2)>>1)) & 1;
	nunstruct[chan].ax += ((nunbuff[5]>>2) & 3);
	nunstruct[chan].ay += ((nunbuff[5]>>4) & 3);
	nunstruct[chan].az += ((nunbuff[5]>>6) & 3);
	if(lastc[chan]==0 && nunstruct[chan].C){
		lastc[chan]=1;
		if(chan==1)nun1foundc=1;
		if(chan==2)nun2foundc=1;
		if(chan==3)nun3foundc=1;
	}
	if(lastz[chan]==0 && nunstruct[chan].Z){
		lastz[chan]=1;
		if(chan==1)nun1foundz=1;
		if(chan==2)nun2foundz=1;
		if(chan==3)nun3foundz=1;
	}
	if(nunstruct[chan].C==0)lastc[chan]=0;
	if(nunstruct[chan].Z==0)lastz[chan]=0;
}
void classicproc(int chan){
//	int ax; //classic left x
//	int ay; //classic left y
//	int az; //classic centre
//	int Z;  //classic right x
//	int C;  //classic right y
//	int L;  //classic left analog
//	int R;  //classic right analog
//	unsigned short x0; //classic buttons
	static unsigned short buttonlast[4]={0};
	unsigned short inttest=(((nunbuff[4]>>1) | (nunbuff[5]<<7)) ^ 0b111111111111111) & nunstruct[chan].x1;
	nunstruct[chan].classic[0]=nunbuff[0];
	nunstruct[chan].classic[1]=nunbuff[1];
	nunstruct[chan].classic[2]=nunbuff[2];
	nunstruct[chan].classic[3]=nunbuff[3];
	nunstruct[chan].classic[4]=nunbuff[4];
	nunstruct[chan].classic[5]=nunbuff[5];
	if(inttest>buttonlast[chan]){
		if(chan==1)nun1foundz=1;
		if(chan==2)nun2foundz=1;
		if(chan==3)nun3foundz=1;
	}
	buttonlast[chan]=inttest;
	nunstruct[chan].ax=(nunbuff[0] & 0b111111)<<2;
	nunstruct[chan].ay=(nunbuff[1] & 0b111111)<<2;
	nunstruct[chan].Z=(((nunbuff[2] & 0b10000000)>>7) |
			((nunbuff[1] & 0b11000000)>>5) |
			((nunbuff[0] & 0b11000000)>>3))<<3;
	nunstruct[chan].C=(nunbuff[2] & 0b11111)<<3;
	nunstruct[chan].R=((nunbuff[3] & 0b00011111))<<3;
	nunstruct[chan].L=(((nunbuff[3] & 0b11100000)>>5) |
			((nunbuff[2] & 0b01100000)>>2))<<3;
	nunstruct[chan].x0=((nunbuff[4]>>1) | (nunbuff[5]<<7)) ^ 0b111111111111111;
}
void mouseproc(int chan){
	static int lastxclick=0,lastyclick=0,lastc[4]={0},lastz[4]={0};
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	int testx, testy;
	testx=(mousebuff[0]<<8) | mousebuff[1];
	testy=(mousebuff[2]<<8) | mousebuff[3];
	TOUCH_DOWN=mousebuff[4];
    if(mousestruct[chan].type>1000) mousestruct[chan].R=0;
	if(testx>=0 && testx<maxW && testy>=0 && testy<maxH){
		mousestruct[chan].ax=testx;
		mousestruct[chan].ay=testy;
		mousestruct[chan].Z=mousebuff[4];
		mousestruct[chan].C=mousebuff[5];
		mousestruct[chan].L=mousebuff[6];
		if(mousebuff[9]==0xFF)mousestruct[chan].az++;
		else if (mousebuff[9]==0x01)mousestruct[chan].az--;
		if(lastc[chan]==0 && mousestruct[chan].C){
			lastc[chan]=1;
			if(chan==1)mouse1foundc=1;
			if(chan==2)mouse2foundc=1;
			if(chan==3)mouse3foundc=1;
		}
		if(lastz[chan]==0 && mousestruct[chan].Z){
			int xmove=abs(lastxclick-mousestruct[chan].ax);
			int ymove=abs(lastyclick-mousestruct[chan].ay);
			lastz[chan]=1;
			if(mousestruct[chan].type>=500 || mousestruct[chan].type<100 || xmove>10 || ymove>10) mousestruct[chan].type=0;
			else {
				mousestruct[chan].R=1;
				mousestruct[chan].type=500 ;
			}
			if(chan==1)mouse1foundz=1;
			if(chan==2)mouse2foundz=1;
			if(chan==3)mouse3foundz=1;
			lastxclick=mousestruct[chan].ax;
			lastyclick=mousestruct[chan].ay;
		}
		if(lastz[chan]==1 && !mousestruct[chan].Z){
			if(chan==1)mouse1leftup=1;
			if(chan==2)mouse2leftup=1;
			if(chan==3)mouse3leftup=1;
		}
		if(mousestruct[chan].C==0)lastc[chan]=0;
		if(mousestruct[chan].Z==0)lastz[chan]=0;
	}
	mouseupdated=1;
}

void cmd_nunchuck(char *cmd){
	int nchan=3;
	char * tp;
	uint32_t id=0;
	tp = checkstring(cmd, "OPEN");
	if(tp) {
		getargs(&tp,5,",");
		if(argc>=1 && *argv[0])nchan=getint(argv[0],1,3);
		if(nchan==1){
			if(nun1 || classic1) error("I2C already OPEN");
			mymemset((void *)&nunstruct[nchan],0,sizeof(struct s_nunstruct));
			if(!mouse1){
				i2c_enable(100);
				I2C_Timeout = 1000;
			}
			int retry=5;
			mT4IntEnable(0);
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr, (uint8_t *)nuninit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
			HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr, (uint8_t *)nuninit2, 2, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr,  (uint8_t *)nunid, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, nunaddr, (uint8_t *)&id, 4, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
			nunstruct[nchan].type=swap32(id);
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr,  (uint8_t *)nuncalib, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, nunaddr, (uint8_t *)nunstruct[nchan].calib, 16, 1000);
			nunstruct[nchan].x0=((unsigned short)(nunstruct[nchan].calib[0])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>2) & 3);
			nunstruct[nchan].y0=((unsigned short)(nunstruct[nchan].calib[1])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>4) & 3);
			nunstruct[nchan].z0=((unsigned short)(nunstruct[nchan].calib[2])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>6) & 3);
			nunstruct[nchan].x1=((unsigned short)(nunstruct[nchan].calib[4])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>2) & 3);
			nunstruct[nchan].y1=((unsigned short)(nunstruct[nchan].calib[5])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>4) & 3);
			nunstruct[nchan].z1=((unsigned short)(nunstruct[nchan].calib[6])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>6) & 3);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			retry=5;
			nunbuff[0]=0;
			while((nunbuff[0]==0 || nunbuff[0]==255) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
				MM_Delay(16);
				mymemset(nunbuff,0,6);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, nunaddr, nunbuff, 6, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Nunchuk not connected");}
				MM_Delay(16);
			}
			if(nunbuff[0]==0 || nunbuff[0]==255){
				nun1=0;
				i2c_disable();
				mT4IntEnable(1); error("Nunchuk not responding");
			}
			if(argc>=3 && *argv[2]){
				nun1Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc==5){
				nun1Interruptc = GetIntAddress(argv[4]);					// get the interrupt location
	            InterruptUsed = true;
			}
			HAL_NVIC_SetPriority(I2C1_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
			nun1=1;
		} else if(nchan==2){
			if(nun2 || classic2) error("I2C already OPEN");
			mymemset((void *)&nunstruct[nchan],0,sizeof(struct s_nunstruct));
			if(!mouse2){
				i2c2_enable(100);
				I2C2_Timeout = 1000;
			}
			int retry=5;
			mT4IntEnable(0);
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr, (uint8_t *)nuninit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
			HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr, (uint8_t *)nuninit2, 2, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr,  (uint8_t *)nunid, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c4, nunaddr, (uint8_t *)&id, 4, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
			nunstruct[nchan].type=swap32(id);
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr,  (uint8_t *)nuncalib, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c4, nunaddr, (uint8_t *)nunstruct[nchan].calib, 16, 1000);
			nunstruct[nchan].x0=((unsigned short)(nunstruct[nchan].calib[0])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>2) & 3);
			nunstruct[nchan].y0=((unsigned short)(nunstruct[nchan].calib[1])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>4) & 3);
			nunstruct[nchan].z0=((unsigned short)(nunstruct[nchan].calib[2])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>6) & 3);
			nunstruct[nchan].x1=((unsigned short)(nunstruct[nchan].calib[4])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>2) & 3);
			nunstruct[nchan].y1=((unsigned short)(nunstruct[nchan].calib[5])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>4) & 3);
			nunstruct[nchan].z1=((unsigned short)(nunstruct[nchan].calib[6])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>6) & 3);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			retry=5;
			nunbuff[0]=0;
			while((nunbuff[0]==0 || nunbuff[0]==255) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
				MM_Delay(16);
				mymemset(nunbuff,0,6);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c4, nunaddr, nunbuff, 6, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Nunchuk not connected");}
				MM_Delay(16);
			}
			if(nunbuff[0]==0 || nunbuff[0]==255){
				nun2=0;
				i2c2_disable();
				mT4IntEnable(1);error("Nunchuk not responding");
			}
			if(argc>=3 && *argv[2]){
				nun2Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc==5){
				nun2Interruptc = GetIntAddress(argv[4]);					// get the interrupt location
	            InterruptUsed = true;
			}
			HAL_NVIC_SetPriority(I2C4_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
			nun2=1;
		} else { //nchan==3
			if(nun3 || classic3) error("I2C already OPEN");
			mymemset((void *)&nunstruct[nchan],0,sizeof(struct s_nunstruct));
			if(!mouse3){
				i2c3_enable(100);
				I2C3_Timeout = 1000;
			}
			int retry=5;
			mT4IntEnable(0);
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr, (uint8_t *)nuninit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
			HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr, (uint8_t *)nuninit2, 2, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr,  (uint8_t *)nunid, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, nunaddr, (uint8_t *)&id, 4, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
			nunstruct[nchan].type=swap32(id);
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr,  (uint8_t *)nuncalib, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, nunaddr, (uint8_t *)nunstruct[nchan].calib, 16, 1000);
			nunstruct[nchan].x0=((unsigned short)(nunstruct[nchan].calib[0])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>2) & 3);
			nunstruct[nchan].y0=((unsigned short)(nunstruct[nchan].calib[1])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>4) & 3);
			nunstruct[nchan].z0=((unsigned short)(nunstruct[nchan].calib[2])<<2) | (((unsigned short)(nunstruct[nchan].calib[3])>>6) & 3);
			nunstruct[nchan].x1=((unsigned short)(nunstruct[nchan].calib[4])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>2) & 3);
			nunstruct[nchan].y1=((unsigned short)(nunstruct[nchan].calib[5])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>4) & 3);
			nunstruct[nchan].z1=((unsigned short)(nunstruct[nchan].calib[6])<<2) | (((unsigned short)(nunstruct[nchan].calib[7])>>6) & 3);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
			MM_Delay(5);
			retry=5;
			nunbuff[0]=0;
			while((nunbuff[0]==0 || nunbuff[0]==255) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
				MM_Delay(16);
				mymemset(nunbuff,0,6);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, nunaddr, nunbuff, 6, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Nunchuk not connected");}
				MM_Delay(16);
			}
			if(nunbuff[0]==0 || nunbuff[0]==255){
				nun3=0;
				i2c3_disable();
				mT4IntEnable(1);error("Nunchuk not responding");
			}
			if(argc>=3 && *argv[2]){
				nun3Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc==5){
				nun3Interruptc = GetIntAddress(argv[4]);					// get the interrupt location
	            InterruptUsed = true;
			}
			HAL_NVIC_SetPriority(I2C2_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
			nun3=1;
		}
		mT4IntEnable(1);
		if(nchan==1){
			while(nun1==1){};
		}
		else if(nchan==2){
			while(nun2==1){};
		}
		else if(nchan==3){
			while(nun3==1){};
		}
		return;
	}
	tp = checkstring(cmd, "CLOSE");
	if(tp) {
		getargs(&tp,1,",");
		if(argc==1)nchan=getint(argv[0],1,3);
		if(nchan==1){
			if(!nun1)error("Not open");
			if(!(mouse1))i2c_disable();
			nun1=0;
			nun1Interruptc=NULL;
			nun1Interruptz=NULL;
		} else if(nchan==2){
			if(!nun2)error("Not open");
			if(!(mouse2))i2c2_disable();
			nun2=0;
			nun2Interruptc=NULL;
			nun2Interruptz=NULL;
		} else {
			if(!nun3)error("Not open");
			if(!(mouse3))i2c3_disable();
			nun3=0;
			nun3Interruptc=NULL;
			nun3Interruptz=NULL;
		}
		return;
	}
	error("Syntax");
}

void cmd_classic(char *cmd){
	int nchan=3;
	char * tp;
	uint32_t id=0;
	tp = checkstring(cmd, "OPEN");
	if(tp) {
		getargs(&tp,5,",");
		if(argc>=1 && *argv[0])nchan=getint(argv[0],1,3);
		if(nchan==1){
			if(nun1 || classic1 ) error("I2C already OPEN");
			mymemset((void *)&nunstruct[nchan],0,sizeof(struct s_nunstruct));
			if(!mouse1){
				i2c_enable(100);
				I2C_Timeout = 1000;
			}
			int retry=5;
			mT4IntEnable(0);
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr, (uint8_t *)nuninit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Classic not connected");}
			HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr, (uint8_t *)nuninit2, 2, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Classic not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr,  (uint8_t *)nunid, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Classic not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, nunaddr, (uint8_t *)&id, 4, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Classic not connected");}
			nunstruct[nchan].type=swap32(id);
			MM_Delay(5);
			retry=5;
			nunbuff[0]=0;
			while((nunbuff[0]==0 || nunbuff[0]==255) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)nunaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Classic not connected");}
				MM_Delay(16);
				mymemset(nunbuff,0,6);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, nunaddr, nunbuff, 6, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Classic not connected");}
				MM_Delay(16);
			}
			if(nunbuff[0]==0 || nunbuff[0]==255){
				classic1=0;
				i2c_disable();
				mT4IntEnable(1);error("Classic not responding");
			}
			if(argc>=3 && *argv[2]){
				nun1Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
	            nunstruct[nchan].x1=0b111111111111111;
	            if(argc==5)nunstruct[nchan].x1=getint(argv[4],0,0b111111111111111);
			}
			HAL_NVIC_SetPriority(I2C1_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
			classic1=1;
		} else if(nchan==2){
			if(nun2 || classic2) error("I2C already OPEN");
			mymemset((void *)&nunstruct[nchan],0,sizeof(struct s_nunstruct));
			if(!mouse2){
				i2c2_enable(100);
				I2C2_Timeout = 1000;
			}
			int retry=5;
			mT4IntEnable(0);
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr, (uint8_t *)nuninit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Classic not connected");}
			HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr, (uint8_t *)nuninit2, 2, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Classic not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr,  (uint8_t *)nunid, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Classic not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c4, nunaddr, (uint8_t *)&id, 4, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Classic not connected");}
			nunstruct[nchan].type=swap32(id);
			MM_Delay(5);
			retry=5;
			nunbuff[0]=0;
			while((nunbuff[0]==0 || nunbuff[0]==255) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)nunaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Classic not connected");}
				MM_Delay(16);
				mymemset(nunbuff,0,6);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c4, nunaddr, nunbuff, 6, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Classic not connected");}
				MM_Delay(16);
			}
			if(nunbuff[0]==0 || nunbuff[0]==255){
				classic2=0;
				i2c2_disable();
				mT4IntEnable(1);error("Classic not responding");
			}
			if(argc>=3 && *argv[2]){
				nun2Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
	            nunstruct[nchan].x1=0b111111111111111;
	            if(argc==5)nunstruct[nchan].x1=getint(argv[4],0,0b111111111111111);
			}
			HAL_NVIC_SetPriority(I2C4_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
			classic2=1;
		} else { //nchan==3
			if(nun3 || classic3) error("I2C already OPEN");
			mymemset((void *)&nunstruct[nchan],0,sizeof(struct s_nunstruct));
			if(!mouse3){
				i2c3_enable(100);
				I2C3_Timeout = 1000;
			}
			int retry=5;
			mT4IntEnable(0);
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr, (uint8_t *)nuninit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Classic not connected");}
			HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr, (uint8_t *)nuninit2, 2, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Classic not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr,  (uint8_t *)nunid, 1, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Classic not connected");}
			MM_Delay(5);
			mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, nunaddr, (uint8_t *)&id, 4, 1000);
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Classic not connected");}
			nunstruct[nchan].type=swap32(id);
			MM_Delay(5);
			retry=5;
			nunbuff[0]=0;
			while((nunbuff[0]==0 || nunbuff[0]==255) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)nunaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Classic not connected");}
				MM_Delay(16);
				mymemset(nunbuff,0,6);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, nunaddr, nunbuff, 6, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Classic not connected");}
				MM_Delay(16);
			}
			if(nunbuff[0]==0 || nunbuff[0]==255){
				classic3=0;
				i2c3_disable();
				mT4IntEnable(1);error("Classic not responding");
			}
			if(argc>=3 && *argv[2]){
				nun3Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
	            nunstruct[nchan].x1=0b111111111111111;
	            if(argc==5)nunstruct[nchan].x1=getint(argv[4],0,0b111111111111111);
			}
			HAL_NVIC_SetPriority(I2C2_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
			classic3=1;
		}
		mT4IntEnable(1);
		if(nchan==1){
			while(classic1==1){};
		}
		else if(nchan==2){
			while(classic2==1){};
		}
		else if(nchan==3){
			while(classic3==1){};
		}
		return;
	}
	tp = checkstring(cmd, "CLOSE");
	if(tp) {
		getargs(&tp,1,",");
		if(argc==1)nchan=getint(argv[0],1,3);
		if(nchan==1){
			if(!classic1)error("Not open");
			if(!(mouse1))i2c_disable();
			classic1=0;
			nun1Interruptc=NULL;
			nun1Interruptz=NULL;
		} else if(nchan==2){
			if(!classic2)error("Not open");
			if(!(mouse2))i2c2_disable();
			classic2=0;
			nun2Interruptc=NULL;
			nun2Interruptz=NULL;
		} else {
			if(!classic3)error("Not open");
			if(!(mouse3))i2c3_disable();
			classic3=0;
			nun3Interruptc=NULL;
			nun3Interruptz=NULL;
		}
		return;
	}
	error("Syntax");
}

void cmd_mouse(char *cmd){
	int nchan=2, sensitivity=0;
	char * tp;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	tp = checkstring(cmd, "OPEN");
	if(tp) {
		getargs(&tp,9,",");
		if(argc>=1 && *argv[0])nchan=getint(argv[0],0,3);
		if(nchan==0){
	    	if(ExtCurrentConfig[INT3PIN] == EXT_PER_IN || ExtCurrentConfig[INT3PIN] == EXT_CNT_IN || ExtCurrentConfig[INT3PIN] == EXT_FREQ_IN ) error("H/W interrupt in use for pin-15 (COUNT 3)");
			CheckPin(MOUSE_CLOCK, CP_CHECKALL);
			CheckPin(MOUSE_DATA, CP_CHECKALL);
		    ExtCfg(MOUSE_CLOCK, EXT_DIG_IN, 0);ExtCfg(MOUSE_CLOCK, EXT_COM_RESERVED, 0);
		    ExtCfg(MOUSE_DATA, EXT_DIG_IN, 0);ExtCfg(MOUSE_DATA, EXT_COM_RESERVED, 0);
			if(argc>=3 && *argv[2]){
				mouse0Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=5 && *argv[4]){
				mouse0Interruptc = GetIntAddress(argv[4]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=7 && *argv[6])sensitivity=getint(argv[6],0,8);
			if(argc>=9 && *argv[8]){
				mouse0Interruptu = GetIntAddress(argv[8]);					// get the interrupt location
	            InterruptUsed = true;
			}
			initMouse0(sensitivity);
		} else if(nchan==1){
			if(mouse1) error("I2C1 already OPEN");
			mymemset((struct s_nunstruct *)&mousestruct[nchan],0,sizeof(struct s_nunstruct));
			if(!(nun1 || classic1)){
				i2c_enable(100);
				I2C_Timeout = 1000;
			}
			if(argc>=7 && *argv[6])sensitivity=getint(argv[6],0,10);
			int retry=1;
			mT4IntEnable(0);
			mouseinit[0]=20;
			mouseinit[1]=maxW>>8;
			mouseinit[2]=maxW & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
			mouseinit[0]=22;
			mouseinit[1]=maxH>>8;
			mouseinit[2]=maxH & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
			mouseinit[0]=24;
			mouseinit[1]=maxW>>9;
			mouseinit[2]=(maxW>>1) & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
			mouseinit[0]=26;
			mouseinit[1]=maxH>>9;
			mouseinit[2]=(maxH>>1) & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
			mouseinit[0]=28;
			if(sensitivity)mouseinit[1]=sensitivity;
			else mouseinit[1]=(maxW>>7); //scale speed to video resolution
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
			retry=5;
			while((abs(((nunbuff[0]<<8) | nunbuff[1]) - (maxW>>1)) > 50) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)mouseaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ i2c_disable(); error("Mouse not connected"); }
				MM_Delay(16);
				mymemset(nunbuff,0,7);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, mouseaddr, mousebuff, 10, 1000);
				if(mmI2Cvalue){ i2c_disable(); error("Mouse not connected"); }
				MM_Delay(16);
			}
			if(abs(((mousebuff[0]<<8) | mousebuff[1]) - (maxW>>1)) > 50){
				mouse1=0;
				i2c_disable();
				mT4IntEnable(1);error("Mouse not responding");
			}
			if(argc>=3 && *argv[2]){
				mouse1Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=5 && *argv[4]){
				mouse1Interruptc = GetIntAddress(argv[4]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=9 && *argv[8]){
				mouse1Interruptu = GetIntAddress(argv[8]);					// get the interrupt location
	            InterruptUsed = true;
			}
			HAL_NVIC_SetPriority(I2C1_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
			mouse1=1;
		} else if(nchan==2){
			if(mouse2) error("I2C2 already OPEN");
			mymemset((struct s_nunstruct *)&mousestruct[nchan],0,sizeof(struct s_nunstruct));
			if(!(nun2 || classic2)){
				i2c2_enable(100);
				I2C2_Timeout = 1000;
			}
			if(argc>=7 && *argv[6])sensitivity=getint(argv[6],0,10);
			int retry=1;
			mT4IntEnable(0);
			mouseinit[0]=20;
			mouseinit[1]=maxW>>8;
			mouseinit[2]=maxW & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Mouse not connected"); }
			mouseinit[0]=22;
			mouseinit[1]=maxH>>8;
			mouseinit[2]=maxH & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Mouse not connected"); }
			mouseinit[0]=24;
			mouseinit[1]=maxW>>9;
			mouseinit[2]=(maxW>>1) & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Mouse not connected"); }
			mouseinit[0]=26;
			mouseinit[1]=maxH>>9;
			mouseinit[2]=(maxH>>1) & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Mouse not connected"); }
			mouseinit[0]=28;
			if(sensitivity)mouseinit[1]=sensitivity;
			else mouseinit[1]=(maxW>>7); //scale speed to video resolution
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c2_disable(); error("Mouse not connected"); }
			retry=5;
			while((abs(((nunbuff[0]<<8) | nunbuff[1]) - (maxW>>1)) > 50) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)mouseaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
				MM_Delay(16);
				mymemset(nunbuff,0,7);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c4, mouseaddr, mousebuff, 10, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
				MM_Delay(16);
			}
			if(abs(((mousebuff[0]<<8) | mousebuff[1]) - (maxW>>1)) > 50){
				i2c2_disable();
				mT4IntEnable(1);
				error("Mouse not responding");
			}
			if(argc>=3 && *argv[2]){
				mouse2Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=5 && *argv[4]){
				mouse2Interruptc = GetIntAddress(argv[4]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=9 && *argv[8]){
				mouse2Interruptu = GetIntAddress(argv[8]);					// get the interrupt location
	            InterruptUsed = true;
			}
			HAL_NVIC_SetPriority(I2C4_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
			mouse2=1;
		} else { //nchan==3
			if(mouse3) error("I2C3 already OPEN");
			mymemset((struct s_nunstruct *)&mousestruct[nchan],0,sizeof(struct s_nunstruct));
			if(!(nun3 || classic3)){
				i2c3_enable(100);
				I2C3_Timeout = 1000;
			}
			if(argc>=7 && *argv[6])sensitivity=getint(argv[6],0,10);
			int retry=1;
			mT4IntEnable(0);
			mouseinit[0]=20;
			mouseinit[1]=maxW>>8;
			mouseinit[2]=maxW & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Mouse not connected"); }
			mouseinit[0]=22;
			mouseinit[1]=maxH>>8;
			mouseinit[2]=maxH & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Mouse not connected"); }
			mouseinit[0]=24;
			mouseinit[1]=maxW>>9;
			mouseinit[2]=(maxW>>1) & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Mouse not connected"); }
			mouseinit[0]=26;
			mouseinit[1]=maxH>>9;
			mouseinit[2]=(maxH>>1) & 0xFF;
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 3, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ i2c3_disable(); error("Mouse not connected"); }
			mouseinit[0]=28;
			if(sensitivity)mouseinit[1]=sensitivity;
			else mouseinit[1]=(maxW>>7); //scale speed to video resolution
			while((mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)mouseaddr, (uint8_t *)mouseinit, 2, 1000))  && retry--){
				MM_Delay(5);
			}
			if(mmI2Cvalue){ mT4IntEnable(1); i2c3_disable(); error("Mouse not connected"); }
			retry=5;
			while((abs(((nunbuff[0]<<8) | nunbuff[1]) - (maxW>>1)) > 50) && retry--){
				mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)mouseaddr,  (uint8_t *)readcontroller, 1, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
				MM_Delay(16);
				mymemset(nunbuff,0,7);
				mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, mouseaddr, mousebuff, 10, 1000);
				if(mmI2Cvalue){ mT4IntEnable(1); i2c_disable(); error("Mouse not connected"); }
				MM_Delay(16);
			}
			if(abs(((mousebuff[0]<<8) | mousebuff[1]) - (maxW>>1)) > 50){
				mouse3=0;
				i2c3_disable();
				mT4IntEnable(1);
				error("Mouse not responding");
			}
			if(argc>=3 && *argv[2]){
				mouse3Interruptz = GetIntAddress(argv[2]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=5 && *argv[4]){
				mouse3Interruptc = GetIntAddress(argv[4]);					// get the interrupt location
	            InterruptUsed = true;
			}
			if(argc>=9 && *argv[8]){
				mouse3Interruptu = GetIntAddress(argv[8]);					// get the interrupt location
	            InterruptUsed = true;
			}
			HAL_NVIC_SetPriority(I2C2_EV_IRQn,0,0);
			HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
			mouse3=1;
		}
		mT4IntEnable(1);
		if(nchan==1){
			while(mouse1==1){};
		}
		else if(nchan==2){
			while(mouse2==1){};
		}
		else if(nchan==3){
			while(mouse3==1){};
		}
		return;
	}
	tp = checkstring(cmd, "CLOSE");
	if(tp) {
		getargs(&tp,1,",");
		if(argc==1)nchan=getint(argv[0],0,3);
		if(nchan==0){
			mouse0close();
		} else if(nchan==1){
			if(!mouse1)return;
			if(!(nun1 || classic1))i2c_disable();
			mouse1=0;
			mouse1Interruptc=NULL;
			mouse1Interruptz=NULL;
		} else if(nchan==2){
			if(!mouse2)return;
			if(!(nun2 || classic2))i2c2_disable();
			mouse2=0;
			mouse2Interruptc=NULL;
			mouse2Interruptz=NULL;
		} else {
			if(!mouse3)return;
			if(!(nun3 || classic3))i2c3_disable();
			mouse3=0;
			mouse3Interruptc=NULL;
			mouse3Interruptz=NULL;
		}
		return;
	}
	error("Syntax");
}

void cmd_Controller(void){
	char *tp;
	tp=checkstring(cmdline, "NUNCHUK");
	if(tp){
		cmd_nunchuck(tp);
		return;
	}
	tp=checkstring(cmdline, "MOUSE");
	if(tp){
		cmd_mouse(tp);
		return;
	}
	tp=checkstring(cmdline, "CLASSIC");
	if(tp){
		cmd_classic(tp);
		return;
	}
	error("Syntax");
}
void fun_classic(void){
	//	int ax; //classic left x
	//	int ay; //classic left y
	//	int az; //classic centre
	//	int Z;  //classic right x
	//	int C;  //classic right y
	//	int L;  //classic left analog
	//	int R;  //classic right analog
	//	unsigned short x0; //classic buttons
	int chan=3;
	getargs(&ep,3,",");
	if(argc==3)chan=getint(argv[2],1,3);
	if((chan==3 && !classic3) || (chan==2 && !classic2) || (chan==1 && !classic1))error("Not open");
	if(checkstring(argv[0], "LX"))iret=nunstruct[chan].ax;
	else if(checkstring(argv[0], "LY"))iret=nunstruct[chan].ay;
	else if(checkstring(argv[0], "RX"))iret=nunstruct[chan].Z;
	else if(checkstring(argv[0], "RY"))iret=nunstruct[chan].C;
	else if(checkstring(argv[0], "L"))iret=nunstruct[chan].L;
	else if(checkstring(argv[0], "R"))iret=nunstruct[chan].R;
	else if(checkstring(argv[0], "B"))iret=nunstruct[chan].x0;
	else if(checkstring(argv[0], "T"))iret=nunstruct[chan].type;
	else iret=0;
	targ=T_INT;
}
void fun_nunchuck(void){
	int chan=3;
	char *p;
	getargs(&ep,3,",");
	if(argc==3)chan=getint(argv[2],1,3);
	if((chan==3 && !nun3) || (chan==2 && !nun2) || (chan==1 && !nun1))error("Not open");
	p=argv[0];
	if(toupper(*p)=='A'){
		p++;
		if(p[1]==0){
			if(toupper(*p)=='X')iret=nunstruct[chan].ax;
			else if(toupper(*p)=='Y')iret=nunstruct[chan].ay;
			else if(toupper(*p)=='Z')iret=nunstruct[chan].az;
			else error("Syntax");
		} else {
			if(p[1]=='0'){
				if(toupper(*p)=='X')iret=nunstruct[chan].x0;
				else if(toupper(*p)=='Y')iret=nunstruct[chan].y0;
				else if(toupper(*p)=='Z')iret=nunstruct[chan].z0;
				else error("Syntax");
			} else if(p[1]=='1'){
				if(toupper(*p)=='X')iret=nunstruct[chan].x1;
				else if(toupper(*p)=='Y')iret=nunstruct[chan].y1;
				else if(toupper(*p)=='Z')iret=nunstruct[chan].z1;
				else error("Syntax");
			} else error("Syntax");
		}
	} else if(toupper(*p)=='J'){
		p++;
		if(p[1]==0){
			if(toupper(*p)=='X')iret=nunstruct[chan].x;
			else if(toupper(*p)=='Y')iret=nunstruct[chan].y;
		} else {
			if(toupper(*p)=='X'){
				p++;
				if(toupper(*p)=='L'){
					iret=nunstruct[chan].calib[9];
				} else if(toupper(*p)=='C'){
					iret=nunstruct[chan].calib[10];
				} else if(toupper(*p)=='R'){
					iret=nunstruct[chan].calib[8];
				} else error("Syntax");
			} else if(toupper(*p)=='Y'){
				p++;
				if(toupper(*p)=='T'){
					iret=nunstruct[chan].calib[11];
				} else if(toupper(*p)=='C'){
					iret=nunstruct[chan].calib[13];
				} else if(toupper(*p)=='B'){
					iret=nunstruct[chan].calib[12];
				} else error("Syntax");
			} else error("Syntax");
		}
	} else {
		if(toupper(*p)=='Z')iret=nunstruct[chan].Z;
		else if(toupper(*p)=='C')iret=nunstruct[chan].C;
		else if(toupper(*p)=='T')iret=nunstruct[chan].type;
		else error("Syntax");
	}
	targ=T_INT;
}

void cmd_mode(void){
	int mode,colour,bc=0;
    getargs(&cmdline, 7, ",");
    if(!argc)error("Syntax");
    mode = getint(argv[0], 1, MAX_MODES);
    if(!(G1Hardware==0 && HAL_GetREVID()!=0x1003) && mode==16)error("G2 resolution only");
	colour=8;
	if(argc > 1 && *argv[2]){
		if(getinteger(argv[2])==16)colour=16;
		else if(getinteger(argv[2])==32)colour=32;
		else if(getinteger(argv[2])==12)colour=12;
		else if(getinteger(argv[2])==8)colour=8;
		else error("Colour depth must be 8, 12, 16, or 32");
	}
	if(colour==32 && G1Hardware)error("32-bit mode not available on this H/W");
	if((colour==12 || colour==32) && mode==9) error("12 and 32-bit colour not available for mode 9");
	if((colour==12 || colour==32) && mode==11)error("12 and 32-bit colour not available for mode 11");
	if((colour==12 || colour==32) && mode==12)error("12 and 32-bit colour not available for mode 12");
	if((colour!=8) && mode==15)error("12, 16, and 32-bit colour not available for mode 15");
	if((colour!=8) && mode==16)error("12, 16, and 32-bit colour not available for mode 16");
   	if(colour!=8 && CMM1)error("Display must be in 8-bit mode for legacy use");

	if(argc > 3 && argv[4]){
		bc=getColour(argv[4],0);
	}
	if(argc > 5 && argv[6]){
        FrameInterrupt = GetIntAddress(argv[6]);					// get the interrupt location
        InterruptUsed = true;
	} else FrameInterrupt=NULL;
	setmode(mode,colour,bc,0);
}

void MIPS16 OtherOptions(void) {
	char *tp, *ttp;

	tp = checkstring(cmdline, "RESET");
	if(tp) {
	    if(CurrentLinePtr) error("Invalid in a program");
        ResetAllOptions();
        SaveOptions(0);
		goto saveandreset;
	}

	tp = checkstring(cmdline, "USBKEYBOARD");
	if(tp) {
	    getargs(&tp, 3, ",");
	    if(CurrentLinePtr) error("Invalid in a program");
	    if((argc <1) || argc>3) error("Argument count");
    	if(CurrentLinePtr) error("Invalid in a program");
        if(checkstring(argv[0], "DISABLE"))	{
        	Option.USBKeyboard = NO_KEYBOARD;
        	Option.noLED=0;
        }
        else {
            if(checkstring(argv[0], "US"))	Option.USBKeyboard = CONFIG_US;
            else if(checkstring(argv[0], "UK"))	Option.USBKeyboard = CONFIG_UK;
            else if(checkstring(argv[0], "DE"))	Option.USBKeyboard = CONFIG_DE;
            else if(checkstring(argv[0], "FR"))	Option.USBKeyboard = CONFIG_FR;
            else if(checkstring(argv[0], "ES"))	Option.USBKeyboard = CONFIG_ES;
            else if(checkstring(argv[0], "BE"))	Option.USBKeyboard = CONFIG_BE;
            else error("Layout not supported");
        }
        if(argc==3)Option.noLED=getint(argv[2],0,1);
        else Option.noLED=0;
        goto saveandreset;
	}

    tp = checkstring(cmdline, "SERIAL PULLUP");
	if(tp) {
	    if(CurrentLinePtr) error("Invalid in a program");
        if(checkstring(tp, "DISABLE"))
            Option.SerialPullup = 0;
        else if(checkstring(tp, "ENABLE"))
            Option.SerialPullup = 1;
        else error("Invalid Command");
        goto saveandreset;
	}

    tp = checkstring(cmdline, "MAXCTRLS");
	if(tp) {
	    getargs(&tp, 3, ",");
		Option.MaxCtrls=getint(argv[0],0,2000);
        goto saveandreset;
	}

    tp = checkstring(cmdline, "HORIZONTAL OFFSET");
	if(tp) {
	    getargs(&tp, 3, ",");
	    int mode=getint(argv[0],1,MAX_MODES);
		Option.offsets[mode]=getint(argv[2],-128,127);
        goto saveandreset;
	}
	tp = checkstring(cmdline, "KEYBOARD REPEAT");
	if(tp) {
		getargs(&tp,3,",");
		Option.RepeatStart=getint(argv[0],100,2000);
		Option.RepeatRate=getint(argv[2],25,2000);
		SaveOptions(1);
		return;
	}

    tp = checkstring(cmdline, "SLEEP");
	if(tp) {
		getargs(&tp,1,",");
		Option.sleep=getint(argv[0],0,255);
		SleepTimer=Option.sleep*1000*60;
		SaveOptions(1);
		return;
	}

	tp = checkstring(cmdline, "SD TIMING");
	if(tp) {
	    if(CurrentLinePtr) error("Invalid in a program");
        if(checkstring(tp, "NORMAL"))
            Option.SDspeed = 0;
        else if(checkstring(tp, "FAST"))
            Option.SDspeed = 1;
        else error("Invalid Command");
        goto saveandreset;
	}
    tp = checkstring(cmdline, "RAM");
	if(tp) {
	    if(CurrentLinePtr) error("Invalid in a program");
		if(Option.profile && G1Hardware)error("Not available when profiling is enabled");
            Option.ProgramStartCode = (G1Hardware ? -2 : -1);
            goto saveandreset;
	}
    tp = checkstring(cmdline, "FLASH");
	if(tp) {
	    if(CurrentLinePtr) error("Invalid in a program");
		int offset=0;
		getargs(&tp,1,",");
		if(argc==1)offset=getint(argv[0],PROGRAMSTARTCODE,6);
		Option.ProgramStartCode = offset;
		goto saveandreset;
	}

    tp = checkstring(cmdline, "MOUSE");
	if(tp) {
	    if(CurrentLinePtr) error("Invalid in a program");
		if(checkstring(tp, "OFF")){
			Option.Mouse=-1;
			Option.Sensitivity = 0;
		}
		else {
			Option.Sensitivity = 0;
			getargs(&tp,3,",");
			Option.Mouse = getint(argv[0],0,3);
			if(argc==3)Option.Sensitivity = getint(argv[2],0,(Option.Mouse==0 ? 8 : 10));
		}
		goto saveandreset;
	}

	tp=checkstring(cmdline, "RTC CALIBRATE");
	if(tp){
		Option.RTC_Calibrate=getint(tp,-511,512);
		int up=RTC_SMOOTHCALIB_PLUSPULSES_RESET;
		int calibrate= -Option.RTC_Calibrate;
		if(Option.RTC_Calibrate>0){
			up=RTC_SMOOTHCALIB_PLUSPULSES_SET;
			calibrate=512-Option.RTC_Calibrate;
		}
		HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, up, calibrate);
		rtcChangeSet();
		SaveOptions(1);
		return;
	}

	tp=checkstring(cmdline, "RTC DRIVE");
	if(tp){
	    if(CurrentLinePtr) error("Invalid in a program");
		Option.rtcdrive=(char)getint(tp,0,3);
		rtcChangeSet();
        goto saveandreset;
	}
	tp=checkstring(cmdline, "DS3231");
	if(tp) {
		if(G1Hardware)error("DS3231 not available on this H/W");
 		if(checkstring(tp, "ON")) {
            Option.RTCinstalled = 1;
        } else if(checkstring(tp, "OFF")) {
        	Option.RTCinstalled = 0;
        } else error("Invalid Command");
        goto saveandreset;

	}
	tp=checkstring(cmdline, "PROFILING");
	if(tp) {
		if(Option.ProgramStartCode<0 && G1Hardware)error("Not available when running in RAM");
 		if(checkstring(tp, "ON")) {
            Option.profile = 1;
        } else if(checkstring(tp, "OFF")) {
        	Option.profile = 0;
        } else error("Invalid Command");
        goto saveandreset;

	}
	tp=checkstring(cmdline, "VGA OUTPUT");
	if(tp){
	    if(CurrentLinePtr) error("Invalid in a program");
        if(checkstring(tp, "HIGH"))
            Option.colourmap=2;
        else if(checkstring(tp, "MEDIUM"))
        	Option.colourmap=1;
        else if(checkstring(tp, "LOW"))
        	Option.colourmap=0;
        else error("Invalid Command");
        goto saveandreset;
	}
	tp=checkstring(cmdline, "USB POLLING");
	if(tp){
	    if(CurrentLinePtr) error("Invalid in a program");
		Option.USBPolling=getint(tp,1,64);
		SaveOptions(1);
		return;
	}
	tp=checkstring(cmdline, "EDIT FONT");
	if(tp){
	    if(CurrentLinePtr) error("Invalid in a program");
        if(checkstring(tp, "LARGE"))
            Option.editfont=2;
        else if(checkstring(tp, "MEDIUM"))
        	Option.editfont=4;
        else if(checkstring(tp, "SMALL"))
        	Option.editfont=7;
        else if(checkstring(tp, "VERY LARGE"))
        	Option.editfont=3;
        else if(checkstring(tp, "NORMAL"))
        	Option.editfont=1;
        else error("Invalid Command");
		SaveOptions(1);
		return;
	}
	tp = checkstring(cmdline, "OVERCLOCK");
	if(tp) {
		if(checkstring(tp, "ON")) {
			Option.CPUmode = 0x10;
		} else if(checkstring(tp, "OFF")) {
			Option.CPUmode = 0;
		} else error("Invalid Command");
		goto saveandreset;
	}

	tp = checkstring(cmdline, "BASELINE");
	if(tp) {
		if(checkstring(tp, "ON")) {
			Option.CPUmode = 0x2;
		} else if(checkstring(tp, "OFF")) {
			Option.CPUmode = 0;
		} else error("Invalid Command");
		goto saveandreset;
	}

	tp = checkstring(cmdline, "ERROR");
	if(tp) {
 		if(checkstring(tp, "CONTINUE")) {
            OptionFileErrorAbort = false;
            return;
        }
		if(checkstring(tp, "ABORT")) {
            OptionFileErrorAbort = true;
            return;
        }
	}
	tp = checkstring(cmdline, "DEFAULT MODE");
	if(tp) {
	    if(CurrentLinePtr) error("Invalid in a program");
 		if(checkstring(tp, "8")) {
            Option.mode=8;
            Option.editfont=1;
        } else if(checkstring(tp, "1")) {
            Option.mode=1;
            Option.editfont=1;
        } else if(checkstring(tp, "8")) {
            Option.mode=9;
            Option.editfont=1;
        } else if(checkstring(tp, "9")) {
            Option.mode=9;
            Option.editfont=4;
        } else if(checkstring(tp, "10")) {
            Option.mode=10;
            Option.editfont=1;
        } else if(checkstring(tp, "11")) {
            Option.mode=11;
            Option.editfont=4;
        } else if(checkstring(tp, "12")) {
            Option.mode=12;
            Option.editfont=1;
        } else if(checkstring(tp, "14")) {
            Option.mode=14;
            Option.editfont=1;
        } else if(checkstring(tp, "15")) {
            Option.mode=15;
            Option.editfont=1;
        } else if(checkstring(tp, "16") && G1Hardware==0) {
            Option.mode=16;
            Option.editfont=3;
        } else if(checkstring(tp, "18")) {
            Option.mode=18;
            Option.editfont=4;
        } else error("Invalid mode");
        goto saveandreset;
	}
    tp = checkstring(cmdline, "CONSOLE");
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
    if(tp) {
        if((ttp = checkstring(tp, "BOTH"))) {
            Option.Height = maxH/gui_font_height;
            Option.Width = maxW/gui_font_width;
            skipspace(ttp);
            Option.DefaultFC = WHITE;
            Option.DefaultBC = BLACK;
             if(!(*ttp == 0 || *ttp == '\'')) {
                getargs(&ttp, 5, ",");                              // this is a macro and must be the first executable stmt in a block
                if(argc > 0) {
                    if(*argv[0] == '#') argv[0]++;                  // skip the hash if used
                    SetFont(((getint(argv[0], 1, FONT_BUILTIN_NBR) - 1) << 4) | 1);
                    Option.DefaultFont = gui_font;
                }
                if(argc > 2) Option.DefaultFC = getint(argv[2], BLACK, WHITE);
                if(argc > 4) Option.DefaultBC = getint(argv[4], BLACK, WHITE);
                if(Option.DefaultFC == Option.DefaultBC) error("Same colours");
            } //else
            if(com3)error("Console port in use");
         	if(!(OptionConsole & 1))start_console();
            OptionConsole = 3;
            Option.colourmode = true;
            PromptFont = Option.DefaultFont;
            PromptFC = Option.DefaultFC;
            PromptBC = Option.DefaultBC;
            HAL_NVIC_EnableIRQ(USART1_IRQn);
        } else if(checkstring(tp, "SCREEN")) {
        	if(OptionConsole & 1)stop_console();
        	OptionConsole = 2;
        } else if((ttp = checkstring(tp, "PORT"))) {
            getargs(&ttp, 1, ",");                              // this is a macro and must be the first executable stmt in a block
        	Option.ConsolePort=getint(argv[0],1,3);
            goto saveandreset;
        } else if(checkstring(tp, "SERIAL")) {
            if(com3)error("Console port in use");
        	if(!(OptionConsole & 1))start_console();
        	OptionConsole = 1;
            HAL_NVIC_EnableIRQ(USART1_IRQn);
        } else if(checkstring(tp, "SAVE")) {
        	Option.Console=OptionConsole;
            goto saveandreset;
        } else error("Syntax");
 		return;
    }
	tp = checkstring(cmdline, "LIST");
    if(tp) {
    	LoadOptions();
    	printoptions();
    	return;
    }


	error("Unrecognised option");

saveandreset:
    // used for options that require the cpu to be reset
    SaveOptions(0);
    _excep_code = RESTART_NOAUTORUN;                            // otherwise do an automatic reset
	while(ConsoleTxBufTail != ConsoleTxBufHead);
	MM_Delay(5);
    SoftReset();                                                // this will restart the processor
}





