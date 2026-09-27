/***************************************************************************
CMM2 MMBasic
timers.h

Include file that contains the globals and defines for timers.c in MMBasic.

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

// timer variables
extern volatile unsigned long long int mSecTimer;                            // this is used to count mSec
extern volatile unsigned int PauseTimer;                            // this is used in the PAUSE command
extern volatile unsigned int IntPauseTimer;                         // this is used in the PAUSE command
extern volatile unsigned int InkeyTimer;                            // used to delay on an escape character
extern volatile unsigned int MouseTimer;                            // used to delay on an escape character
extern volatile unsigned int SecondsTimer;
extern volatile unsigned int GifTimer;
extern volatile int ds18b20Timer;
extern volatile unsigned int Timer1, Timer2, Timer3, GUITimer1;
// date/time counters
extern volatile int milliseconds;
extern volatile int second;													
extern volatile int minute;
extern volatile int hour;
extern volatile int day;
extern volatile int month;
extern volatile int year;
extern volatile int day_of_week;
// global timer functions
extern void initTimers(void);
extern void mT4IntEnable(int);
extern unsigned char PulsePin[];
extern int PulseCnt[];
extern int PulseActive;
extern volatile uint64_t SoundPlay;
extern volatile unsigned int GPSTimer;
extern volatile unsigned int AHRSTimer;
extern volatile int USBtime;
extern volatile int Touchtime;
extern volatile int keytimer;
extern volatile int flashtimer;
extern volatile int SD_LED_Triggered;
