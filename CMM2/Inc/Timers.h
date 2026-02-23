/***********************************************************************************************************************
MMBasic

timers.h

Include file that contains the globals and defines for timers.c in MMBasic.
  
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
