/***************************************************************************

CMM2 MMBasic
mouse.c

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
#include "stm32h7xx_ll_gpio.h"
extern volatile struct s_nunstruct mousestruct[4];
volatile int mouse0=0;
int mouseID=0;
volatile int readreturn=-1;
volatile int PS2State, KCount, KParity, runmode=0;
char *mouse0Interruptc=NULL;
char *mouse0Interruptz=NULL;
char *mouse0Interruptu=NULL;
volatile unsigned char Code = 0;
volatile short mouse[4];
volatile unsigned int bno=0;
volatile unsigned char LastCode = 0;
volatile int mouse0foundz=0, mouse0foundc=0, mouse0leftup;
void setstream(void);
void mouse_init();
void sendCommand(int cmd);
int ReadReturn(int timeout);
// definition of the mouse PS/2 state machine
#define PS2START    0
#define PS2BIT      1
#define PS2PARITY   2
#define PS2STOP     3
#define PS2ERROR    9
#define MDATA 1
#define MCLK 0
#define HIGH 1
#define LOW 0
#define MouseTimeout 500
void MouseKBDIntEnable(int status){
    GPIO_InitTypeDef GPIO_InitDef;
    if(status){
        mouse0=1;
	    PinSetBit(MOUSE_CLOCK, TRISSET);                                         // same for data
	    PinSetBit(MOUSE_DATA, TRISSET);                                          // data low
        HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(EXTI3_IRQn);
        GPIO_InitDef.Pull = GPIO_PULLUP; //set as input with pull up
        GPIO_InitDef.Pin = PinDef[MOUSE_CLOCK].bitnbr;
        GPIO_InitDef.Mode = GPIO_MODE_IT_FALLING;
        GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(PinDef[MOUSE_CLOCK].sfr, &GPIO_InitDef);
	} else {
	    PinSetBit(MOUSE_CLOCK, TRISSET);                                         // same for data
	    PinSetBit(MOUSE_DATA, TRISSET);                                          // data low
		HAL_NVIC_DisableIRQ(EXTI3_IRQn);
	    PS2State = PS2START;
	}
}
void mousecheck(int n){
	if(MouseTimer < MouseTimeout){
		routinechecks(1);
		return;
	}
    MouseKBDIntEnable(0);      											// disable interrupt in case called from within CNInterrupt()
	runmode=0;
    mouse0=0;
    ExtCfg(MOUSE_CLOCK, EXT_NOT_CONFIG, 0);
    ExtCfg(MOUSE_DATA, EXT_NOT_CONFIG, 0);
    PS2State = PS2START;
    error("Mouse timeout % ",n);
}
/***************************************************************************************************
initMouse
Initialise the mouse routine.
****************************************************************************************************/
void initMouse0(int sensitivity) {
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	mouseID=0;
	runmode=0;
    MouseKBDIntEnable(0);      											// disable interrupt in case called from within CNInterrupt()
	// enable pullups on the clock and data lines.
	// This stops them from floating and generating random chars when no mouse is attached

    // reserve the mouse pins
	sendCommand(0xFF);                                              // Reset
	ReadReturn(500);
	sendCommand(0xF5);                                              // Turn off streaming
	ReadReturn(5);
	if(sensitivity){
		int scaling;
		if(sensitivity>4){
			scaling=1;
			sensitivity-=4;
		}
		sensitivity--;
		if(scaling){
		 	sendCommand(0xE7);                                              //
			ReadReturn(5);
		}
		sendCommand(0xE8);
		ReadReturn(5);
		sendCommand(sensitivity);
		ReadReturn(5);
	}
	sendCommand(0xF3);                                              //
	ReadReturn(5);
	sendCommand(200);                                              //
	ReadReturn(5);
	sendCommand(0xF3);                                              //
	ReadReturn(5);
	sendCommand(100);                                              //
	ReadReturn(5);
	sendCommand(0xF3);                                              //
	ReadReturn(5);
	sendCommand(80);                                              //
	ReadReturn(5);
	sendCommand(0xF2);                                              //
    mouseID=ReadReturn(10);
 	sendCommand(0xF3);                                              //
	ReadReturn(5);
	sendCommand(200);                                              //
	ReadReturn(5);
	sendCommand(0xF4);                                              // Turn on streaming
	ReadReturn(5);

    // setup Change Notification interrupt
    PS2State = PS2START;
	mymemset((struct s_nunstruct *)&mousestruct[0],0,sizeof(struct s_nunstruct));
    mousestruct[0].classic[0]=mouseID;
    mousestruct[0].type=0; //used for the double click timer
    mousestruct[0].ax=maxW/2;
    mousestruct[0].ay=maxH/2;
	runmode=1;
	Code = 0;
	bno=0;
	LastCode = 0;
	 __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
    MouseKBDIntEnable(1);       										// enable interrupt
}
void mouse0close(void){
	if(!mouse0)return;
	mouseID=0;
	runmode=0;
	sendCommand(0xFF);                                              // Turn off streaming
	ReadReturn(5);
    MouseKBDIntEnable(0);      											// disable interrupt in case called from within CNInterrupt()
	ExtCfg(MOUSE_CLOCK, EXT_NOT_CONFIG, 0);
    ExtCfg(MOUSE_DATA, EXT_NOT_CONFIG, 0);
    mouse0=0;
	runmode=0;
    mouse0Interruptz=NULL;
    mouse0Interruptc=NULL;
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
	mymemset((struct s_nunstruct *)&mousestruct[0],0,sizeof(struct s_nunstruct));
}
/***************************************************************************************************
sendCommand - Send a command to to mouse.
****************************************************************************************************/
void sendCommand(int cmd) {
    int i;
    char parity = 1;
    MouseKBDIntEnable(0);      											// disable interrupt in case called from within CNInterrupt()
    uSec(300);

    PinSetBit(MOUSE_CLOCK, LATCLR);                                         // same for data
    PinSetBit(MOUSE_DATA, LATCLR);                                          // data low
    PinSetBit(MOUSE_CLOCK, ODCSET);                                         // same for data

    uSec(300);
    PinSetBit(MOUSE_DATA, ODCSET);                                         // same for data

 	uSec(10);
    PinSetBit(MOUSE_CLOCK, TRISSET);                                         // same for data

 	uSec(5);
 	MouseTimer = 0;
 	while(PinRead(MOUSE_CLOCK)) mousecheck(1);             // wait for the mouse to pull the clock low
 	// send each bit including parity
 	for(i = 0; i < 8; i++) {
 		if(cmd & 1){
 			LL_GPIO_SetOutputPin(PinDef[MOUSE_DATA].sfr, PinDef[MOUSE_DATA].bitnbr);
 		}
 		else {
 			LL_GPIO_ResetOutputPin(PinDef[MOUSE_DATA].sfr, PinDef[MOUSE_DATA].bitnbr);
 		}
     	while(!PinRead(MOUSE_CLOCK)) mousecheck(2);          // wait for the mouse to bring the clock high
     	while(PinRead(MOUSE_CLOCK))  mousecheck(3);        // wait for clock low
        parity = parity ^ (cmd & 0x01);
     	cmd >>= 1;
    }
 	  if (parity) {
 		 LL_GPIO_SetOutputPin(PinDef[MOUSE_DATA].sfr, PinDef[MOUSE_DATA].bitnbr);
 	  }
 	  else {
 		 LL_GPIO_ResetOutputPin(PinDef[MOUSE_DATA].sfr, PinDef[MOUSE_DATA].bitnbr);
 	  }
 	while(!PinRead(MOUSE_CLOCK)) mousecheck(4);          // wait for the mouse to bring the clock high
 	while(PinRead(MOUSE_CLOCK))  mousecheck(5);          // wait for clock low
    PinSetBit(MOUSE_DATA, TRISSET);                                         // same for data
 	uSec(50);
 	while(PinRead(MOUSE_CLOCK))  mousecheck(6);         // wait for clock low
 	  /* wait for mouse to switch modes */
 	while (!PinRead(MOUSE_CLOCK)|| !PinRead(MOUSE_DATA)) mousecheck(7);
 	/* put a hold on the incoming data. */
    PinSetBit(MOUSE_CLOCK, ODCSET);                                         // same for data
 	LL_GPIO_ResetOutputPin(PinDef[MOUSE_CLOCK].sfr, PinDef[MOUSE_CLOCK].bitnbr);
}

int ReadReturn(int timeout){
	 int i;
	 Code = 0;
	 bno=0;
	 LastCode = 0;
	 readreturn=-1;
	 __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
	 MouseKBDIntEnable(1);
	 i=100;
	 MouseTimer = 0;
	 while(MouseTimer<timeout){
		 if(readreturn!=-1){
			 i=readreturn;
			 readreturn=-1;
			 routinechecks(1);
		 }
	 }
	 return i;
}


/***************************************************************************************************
change notification interrupt service routine
****************************************************************************************************/
void MNInterrupt(void) {
	static unsigned long long int lefttimer=0, righttimer=0;
    int d;
    int maxW=PageTable[0].xmax;
	int maxH=PageTable[0].ymax;
//    if(!BasicRunning) {
//        KBDClearIntFlag();     // clear interrupt flag
//        error("PS2 Timeout");
//    }
    // Make sure it was a falling edge
    if(PinRead(MOUSE_CLOCK) == 0)
    {
	    // Sample the data
	    d = PinRead(MOUSE_DATA);
        switch(PS2State){
            default:
            case PS2ERROR:                                          // this can happen if a timing or parity error occurs
                // fall through to PS2START

            case PS2START:
                if(!d) {                							// PS2DAT == 0
                    KCount = 8;         							// init bit counter
                    KParity = 0;        							// init parity check
                    Code = 0;
                    PS2State = PS2BIT;
                }
                break;

            case PS2BIT:
                Code >>= 1;            								// shift in data bit
                if(d) Code |= 0x80;                					// PS2DAT == 1
                KParity ^= Code;      								// calculate parity
                if (--KCount <= 0) PS2State = PS2PARITY;   			// all bit read
                break;

            case PS2PARITY:
                if(d) KParity ^= 0x80;                				// PS2DAT == 1
                if (KParity & 0x80)    								// parity odd, continue
                    PS2State = PS2STOP;
                else
                    {PS2State = PS2ERROR;putConsole('x');}
                break;
            case PS2STOP:
                if(d) {                 							// PS2DAT == 1
                    readreturn=Code;
                    if(runmode){
                        mouse[bno++]=Code;
                        if(!(mouse[0] & 0x08))bno=0;//bit 3 must be set in first byte
                        if(bno==(mouseID==3 ? 4: 3)){
                            bno=0;
                            if(mouse[0] & 0b10000)mouse[1] |=0xFF00;
                            if(mouse[0] & 0b100000)mouse[2] |=0xFF00;
                            mousestruct[0].ax+=mouse[1];
                            if(mousestruct[0].ax<0)mousestruct[0].ax=0;
                            if(mousestruct[0].ax>=maxW)mousestruct[0].ax=maxW-1;
                            mousestruct[0].ay-=mouse[2];
                            mouseupdated=1;
                             if(mousestruct[0].ay<0)mousestruct[0].ay=0;
                            if(mousestruct[0].ay>=maxH)mousestruct[0].ay=maxH-1;
                            TOUCH_DOWN = mousestruct[0].Z = mouse[0] & 0b1;
                            mousestruct[0].C=(mouse[0] & 0b10)>>1;
                            mousestruct[0].L=(mouse[0] & 0b100)>>2;
                            if(mousestruct[0].type>1000) mousestruct[0].R=0;
                            if((mouse[0] & 3) != (LastCode & 3)){
                            	if((mouse[0] & 1) && !(LastCode & 1) && (mSecTimer-lefttimer>16)){ //left button press
                            		mouse0foundz=1;
                            		if(mousestruct[0].type>=500 || mousestruct[0].type<100) mousestruct[0].type=0;
                            		else {
                            			mousestruct[0].R=1;
                            			mousestruct[0].type=500 ;
                            		}
                            		lefttimer=mSecTimer;
                            	}
                               	if(!(mouse[0] & 1) && (LastCode & 1)){ //left button release
                               		mouse0leftup=1;
                               	}
                            	if((mouse[0] & 2) && !(LastCode & 2) && (mSecTimer-righttimer>16)){
                            		mouse0foundc=1;  //right button press
                            		righttimer=mSecTimer;
                            	}
                            }
                            LastCode=mouse[0];
                            if(mouseID==3){
                            	if(mouse[3] & 0x80)mouse[3]|=0xFF00;
                            	mousestruct[0].az=(volatile int)mousestruct[0].az+mouse[3];
                            }
                       }
                    }
                }
                PS2State = PS2START;
                break;
	    }
	}
}
