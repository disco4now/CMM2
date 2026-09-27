/***************************************************************************
CMM2 MMBasic
timers.c

This module manages various timers (counting variables), the date/time,
counting inputs and generates the sound.  All this is contained within the timer 4 interrupt.

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

#define INCLUDE_FUNCTION_DEFINES

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "main.h"

// timer variables
volatile unsigned int SecondsTimer = 0;
volatile unsigned int GifTimer = 0;
volatile unsigned int PauseTimer = 0;
volatile unsigned int IntPauseTimer = 0;
volatile unsigned int InkeyTimer = 0;
volatile unsigned int MouseTimer = 0;
volatile unsigned int WDTimer = 0;
volatile unsigned int ScrewUpTimer=0;
extern volatile unsigned int SleepTimer;
extern volatile int sleeping;
volatile unsigned int Timer1=0, Timer2=0, GUITimer1=0;		                       //1000Hz decrement timer
volatile int ds18b20Timer = -1;
volatile unsigned long long int mSecTimer = 0;								// this is used to count mSec
volatile long long int nunTimer = 0;								// this is used to count mSec
volatile int USBtime=0;
volatile int Touchtime=0;
volatile int second = 0;											// date/time counters
volatile int minute = 0;
volatile int hour = 0;
volatile int day = 1;
volatile int month = 1;
volatile int milliseconds = 1;
volatile int year = 2000;
volatile int day_of_week=1;
volatile int processtick = 1;
volatile unsigned int GPSTimer = 0;
volatile unsigned int AHRSTimer = 0;
volatile int keytimer=0;
volatile int DoTouch=0;
volatile int TOUCH_DOWN=0;
const char DaysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
unsigned char PulsePin[NBR_PULSE_SLOTS];
int PulseCnt[NBR_PULSE_SLOTS];
int PulseActive;
volatile int SD_LED_Triggered=0;
extern TIM_HandleTypeDef htim2;
extern void audio_checks(void);
//extern unsigned int CFuncmSec;
//extern void CallCFuncmSec(void);
extern volatile uint64_t Count5High;
extern void disk_timerproc(void);
extern int LCD_BL_Period;
extern void nunproc(int chan);
extern void classicproc(int chan);
extern void mouseproc(int chan);
extern const char readcontroller[1];
extern uint8_t nunbuff[10];
extern uint8_t mousebuff[10];
extern const uint16_t nunaddr;
extern const uint16_t mouseaddr;
//extern uint8_t* volatile DataPageAddress;
extern I2C_HandleTypeDef hi2c1, hi2c2, hi2c4;
extern void CallCFuncmSec(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncmSec;                                      // we should call the CFunction mSec function if this is non zero
extern void GT911_Scan(void);
extern volatile struct s_nunstruct mousestruct[4];
#define GET_TOUCH           3
extern int GetTouch(int axis);

/***************************************************************************************************
InitTimers
Initialise the 1 mSec timer used for internal timekeeping.
****************************************************************************************************/

volatile uint32_t msTicks; /* Counts 1ms timeticks */
void Timer1msHandler(void) {                            /* ----- SysTick_Handler - */
	mSecTimer++;													// used by the TIMER function
	static int n1=0,cl1=0,m1=0;
	static int n2=0,cl2=0,m2=0;
	static int n3=0,cl3=0,m3=0;
	if(processtick){
		static int IrTimeout, IrTick, NextIrTick;
		int ElapsedMicroSec, IrDevTmp, IrCmdTmp;
		if((mSecTimer % Option.USBPolling)==0)USBtime=1; //trigger USB processing
		/////////////////////////////// count up timers /////////////////////////////////////

		// if we are measuring period increment the count
		if(ExtCurrentConfig[INT1PIN] == EXT_PER_IN) INT1Count++;
		if(ExtCurrentConfig[INT2PIN] == EXT_PER_IN) INT2Count++;
		if(ExtCurrentConfig[INT3PIN] == EXT_PER_IN) INT3Count++;
		if(ExtCurrentConfig[INT4PIN] == EXT_PER_IN) INT4Count++;
		if(CFuncmSec) CallCFuncmSec();                                  // the 1mS tick for CFunctions (see CFunction.c)
		PauseTimer++;													// used by the PAUSE command
		IntPauseTimer++;												// used by the PAUSE command inside an interrupt
		InkeyTimer++;													// used to delay on an escape character
		MouseTimer++;													// used to delay on an escape character
		GPSTimer++;
		nunTimer++;
    	AHRSTimer++;
		keytimer++;
		mousestruct[0].type++;
		mousestruct[1].type++;
		mousestruct[2].type++;
		mousestruct[3].type++;
		disk_timerproc();
		if(Timer2)Timer2--;
		if(Timer1)Timer1--;
		if(GUIactive)GUITimer1++;
		if(GifTimer)GifTimer--;
		if((mSecTimer % 360000000) == 2000)copy_clock();
		if(classic3){
			if(nunTimer % 20 ==0){
				HAL_I2C_Master_Transmit_IT(&hi2c2, (uint16_t)nunaddr,  (uint8_t*)readcontroller, 1);
				if(cl3!=2)cl3=1;
			}
			if(nunTimer % 20 ==1 && cl3){
				HAL_I2C_Master_Receive_IT(&hi2c2, nunaddr, nunbuff, 6);
				cl3=2;
			}
			if(nunTimer % 20 ==2 && cl3==2){
				classicproc(3);
				classic3=2;
			}
		}
		if(classic2){
			if(nunTimer % 20 ==3){
				HAL_I2C_Master_Transmit_IT(&hi2c4, (uint16_t)nunaddr,  (uint8_t*)readcontroller, 1);
				if(cl2!=2)cl2=1;
			}
			if(nunTimer % 20 ==4 && cl2){
				HAL_I2C_Master_Receive_IT(&hi2c4, nunaddr, nunbuff, 6);
				cl2=2;
			}
			if(nunTimer % 20 ==5 && cl2==2){
				classicproc(2);
				classic2=2;
			}
		}
		if(classic1){
			if(nunTimer % 20 ==6){
				HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)nunaddr,  (uint8_t*)readcontroller, 1);
				if(cl1!=2)cl1=1;
			}
			if(nunTimer % 20 ==7 && cl1){
				HAL_I2C_Master_Receive_IT(&hi2c1, nunaddr, nunbuff, 6);
				cl1=2;
			}
			if(nunTimer % 20 ==8 && cl1==2){
				classicproc(1);
				classic1=2;
			}
		}
		if(nun3){
			if(nunTimer % 20 ==0){
				HAL_I2C_Master_Transmit_IT(&hi2c2, (uint16_t)nunaddr,  (uint8_t*)readcontroller, 1);
				if(n3!=2)n3=1;
			}
			if(nunTimer % 20 ==1 && n3){
				HAL_I2C_Master_Receive_IT(&hi2c2, nunaddr, nunbuff, 6);
				n3=2;
			}
			if(nunTimer % 20 ==2 && n3==2){
				nunproc(3);
				nun3=2;
			}
		}
		if(nun2){
			if(nunTimer % 20 ==3){
				HAL_I2C_Master_Transmit_IT(&hi2c4, (uint16_t)nunaddr,  (uint8_t*)readcontroller, 1);
				if(n2!=2)n2=1;
			}
			if(nunTimer % 20 ==4 && n2){
				HAL_I2C_Master_Receive_IT(&hi2c4, nunaddr, nunbuff, 6);
				n2=2;
			}
			if(nunTimer % 20 ==5 && n2==2){
				nunproc(2);
				nun2=2;
			}
		}
		if(nun1){
			if(nunTimer % 20 ==6){
				HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)nunaddr,  (uint8_t*)readcontroller, 1);
				if(n1!=2)n1=1;
			}
			if(nunTimer % 20 ==7 && n1){
				HAL_I2C_Master_Receive_IT(&hi2c1, nunaddr, nunbuff, 6);
				n1=2;
			}
			if(nunTimer % 20 ==8 && n1==2){
				nunproc(1);
				nun1=2;
			}
		}
		if(mouse3){
			if(nunTimer % 20 ==9){
				HAL_I2C_Master_Transmit_IT(&hi2c2, (uint16_t)mouseaddr,  (uint8_t*)readcontroller, 1);
				if(m3!=2)m3=1;
			}
			if(nunTimer % 20 ==10 && m3){
				HAL_I2C_Master_Receive_IT(&hi2c2, mouseaddr, mousebuff, 10);
				m3=2;
			}
			if(nunTimer % 20 ==11 && m3==2){
				mouseproc(3);
				mouse3=2;
			}
		}
		if(mouse2){
			if(nunTimer % 20 ==12){
				HAL_I2C_Master_Transmit_IT(&hi2c4, (uint16_t)mouseaddr,  (uint8_t*)readcontroller, 1);
				if(m2!=2)m2=1;
			}
			if(nunTimer % 20 ==13 && m2){
				HAL_I2C_Master_Receive_IT(&hi2c4, mouseaddr, mousebuff, 10);
				m2=2;
			}
			if(nunTimer % 20 ==14 && m2==2){
				mouseproc(2);
				mouse2=2;
			}
		}
		if(mouse1){
			if(nunTimer % 20 ==15){
				HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)mouseaddr,  (uint8_t*)readcontroller, 1);
				if(m1!=2)m1=1;
			}
			if(nunTimer % 20 ==16 && m1){
				HAL_I2C_Master_Receive_IT(&hi2c1, mouseaddr, mousebuff, 10);
				m1=2;
			}
			if(nunTimer % 20 ==17 && m1==2){
				mouseproc(1);
				mouse1=2;
			}
		}
        if(InterruptUsed) {
            int i;
            for(i = 0; i < NBRSETTICKS; i++) if(TickActive[i])TickTimer[i]++;			// used in the interrupt tick
         }

		if(ScrewUpTimer) {
			if(--ScrewUpTimer == 0) {
				_excep_code = SCREWUP_TIMEOUT;
				SoftReset();                                            // crude way of implementing a watchdog timer.
			}
		}
		if(WDTimer) {
			if(--WDTimer == 0) {
				_excep_code = WATCHDOG_TIMEOUT;
				SoftReset();                                            // crude way of implementing a watchdog timer.
			}
		}
		if(SD_LED_Triggered) {
			if(--SD_LED_Triggered == 0) {
				SD_LED_GPIO_Port->BSRR = SD_LED_Pin<<16;
			}
		}


		ds18b20Timer++;
		// check if any pulse commands are running
		if(PulseActive) {
			int i;
			for(PulseActive = i = 0; i < NBR_PULSE_SLOTS; i++) {
				if(PulseCnt[i] > 0) {                                   // if the pulse timer is running
					PulseCnt[i]--;                                      // and decrement our count
					if(PulseCnt[i] == 0)                                // if this is the last count reset the pulse
						PinSetBit(PulsePin[i], LATINV);
					else
						PulseActive = true;                             // there is at least one pulse still active
				}
			}
		}
	    TouchTimer++;
	    if(CheckGuiFlag) CheckGuiTimeouts();                            // are blinking LEDs in use?  If so count down their timers

	    if(Option.Mouse>=0 && Option.MaxCtrls>0){                       // is touch enabled and the PEN IRQ pin an input?
	        if(TOUCH_DOWN) {                                            // is the pen down
	            if(!TouchState) {                                       // yes, it is.  If we have not reported this before
	                TouchState = TouchDown = true;                      // set the flags
	                TouchUp = false;
	            }
	        } else {
	            if(TouchState) {                                        // the pen is not down.  If we have not reported this before
	                TouchState = TouchDown = false;                     // set the flags
	                TouchUp = true;
	            }
	        }
	    }

		// Handle any IR remote control activity
		ElapsedMicroSec = readusclock();
		if(IrState > IR_WAIT_START && ElapsedMicroSec > 15000) IrReset();
		IrCmdTmp = -1;

		// check for any Sony IR receive activity
		if(IrState == SONY_WAIT_BIT_START && ElapsedMicroSec > 2800 && (IrCount == 12 || IrCount == 15 || IrCount == 20)) {
			IrDevTmp = ((IrBits >> 7) & 0b11111);
			IrCmdTmp = (IrBits & 0b1111111) | ((IrBits >> 5) & ~0b1111111);
		}

		// check for any NEC IR receive activity
		if(IrState == NEC_WAIT_BIT_END && IrCount == 32) {
			// check if it is a NON extended address and adjust if it is
			if((IrBits >> 24) == ~((IrBits >> 16) & 0xff)) IrBits = (IrBits & 0x0000ffff) | ((IrBits >> 8) & 0x00ff0000);
			IrDevTmp = ((IrBits >> 16) & 0xffff);
			IrCmdTmp = ((IrBits >> 8) & 0xff);
		}

		// now process the IR message, this includes handling auto repeat while the key is held down
		// IrTick counts how many mS since the key was first pressed
		// NextIrTick is used to time the auto repeat
		// IrTimeout is used to detect when the key is released
		// IrGotMsg is a signal to the interrupt handler that an interrupt is required
		if(IrCmdTmp != -1) {
			if(IrTick > IrTimeout) {
				// this is a new keypress
				IrTick = 0;
				NextIrTick = 650;
			}
			if(IrTick == 0 || IrTick > NextIrTick) {
				if(IrVarType & 0b01)
					*(MMFLOAT *)IrDev = IrDevTmp;
				else
					*(long long int *)IrDev = IrDevTmp;
				if(IrVarType & 0b10)
					*(MMFLOAT *)IrCmd = IrCmdTmp;
				else
					*(long long int *)IrCmd = IrCmdTmp;
				IrGotMsg = true;
				NextIrTick += 250;
			}
			IrTimeout = IrTick + 150;
			IrReset();
		}
		IrTick++;
		if(Option.sleep && --SleepTimer==0){
			GPIO_InitTypeDef GPIO_InitStruct = {0};
		    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
		    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		    GPIO_InitStruct.Pull = GPIO_NOPULL;
		    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
			sleeping=1;
		}
		if(++CursorTimer > CURSOR_OFF + CURSOR_ON) CursorTimer = 0;		// used to control cursor blink rate
//		*DataPageAddress &=0xFFF00FFF;

		// if we are measuring frequency grab the count for the last second
		if(ExtCurrentConfig[INT1PIN] == EXT_FREQ_IN && --INT1Timer <= 0) { INT1Value = INT1Count; INT1Count = 0; INT1Timer = INT1InitTimer; }
		if(ExtCurrentConfig[INT2PIN] == EXT_FREQ_IN && --INT2Timer <= 0) { INT2Value = INT2Count; INT2Count = 0; INT2Timer = INT2InitTimer; }
		if(ExtCurrentConfig[INT3PIN] == EXT_FREQ_IN && --INT3Timer <= 0) { INT3Value = INT3Count; INT3Count = 0; INT3Timer = INT3InitTimer; }
		if(ExtCurrentConfig[INT4PIN] == EXT_FREQ_IN && --INT4Timer <= 0) { INT4Value = INT4Count; INT4Count = 0; INT4Timer = INT4InitTimer; }
		if(ExtCurrentConfig[COUNT5] == EXT_FREQ_IN && --INT5Timer <= 0) { INT5Value = Count5High<<16 | ReadCount5(); WriteCount5(0); Count5High=0; INT5Timer = INT5InitTimer; }
}

}
void mT4IntEnable(int status){
	if(status){
		processtick=1;
	} else{
		processtick=0;
	}
}

