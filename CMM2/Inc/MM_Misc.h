/***************************************************************************
CMM2 MMBasic
MM_Misc.h

Include file that contains the globals and defines for MM_Misc.c in MMBasic.
These are miscelaneous commands and functions that do not easily sit anywhere else.

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


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    // General definitions used by other modules

    #ifndef MISC_HEADER
    #define MISC_HEADER

   	extern void MIPS16 OtherOptions(void);

    extern char *InterruptReturn;
    extern int check_interrupt(void);
    extern char *GetIntAddress(char *p);
    extern long long int Getuptime(void);
    extern volatile uint64_t g_uptime;
    extern void MIPS16 CrunchData(char **p, int c);
    extern unsigned int GetPeekAddr(char *p);
    extern unsigned int GetPokeAddr(char *p);
    // struct for the interrupt configuration
    #define T_LOHI   1
    #define T_HILO   2
    #define T_BOTH   3
    struct s_inttbl {
            int pin;                                   // the pin on which the interrupt is set
            int last;					// the last value of the pin (ie, hi or low)
            char *intp;					// pointer to the interrupt routine
            int lohi;                                  // trigger condition (T_LOHI, T_HILO, etc).
    };
    struct s_nunstruct {
    	char x;
    	char y;
    	int ax; //classic left x
    	int ay; //classic left y
    	int az; //classic centre
    	int Z;  //classic right x
    	int C;  //classic right y
    	int L;  //classic left analog
    	int R;  //classic right analog
    	unsigned short x0; //classic buttons
    	unsigned short y0;
    	unsigned short z0;
    	unsigned short x1;
    	unsigned short y1;
    	unsigned short z1;
    	uint32_t type;
    	uint8_t calib[16];
    	uint8_t classic[6];
    };
    #define NBRINTERRUPTS	    10			// number of interrupts that can be set
    extern struct s_inttbl inttbl[NBRINTERRUPTS];
    extern void floatindexsort(MMFLOAT a[], long long index[],int n, int flags, int startpoint);
    extern int TickPeriod[NBRSETTICKS];
    extern volatile int TickTimer[NBRSETTICKS+1];
    extern char *TickInt[NBRSETTICKS+1];
	extern volatile unsigned char TickActive[NBRSETTICKS];

	extern unsigned int CurrentCpuSpeed;
	extern unsigned int PeripheralBusSpeed;
	extern char *OnKeyGOSUB;
	extern char EchoOption;
	extern char *FrameInterrupt;
	extern volatile int Framecomplete;
	extern char *nun1Interruptc;
	extern char *nun2Interruptc;
	extern char *nun3Interruptc;
	extern char *nun1Interruptz;
	extern char *nun2Interruptz;
	extern char *nun3Interruptz;
	extern volatile int nun1foundz,nun2foundz,nun3foundz;
	extern volatile int nun1foundc,nun2foundc,nun3foundc;
	extern char *mouse0Interruptc;
	extern char *mouse1Interruptc;
	extern char *mouse2Interruptc;
	extern char *mouse3Interruptc;
	extern char *mouse0Interruptz;
	extern char *mouse1Interruptz;
	extern char *mouse2Interruptz;
	extern char *mouse3Interruptz;
	extern char *mouse0Interruptu;
	extern char *mouse1Interruptu;
	extern char *mouse2Interruptu;
	extern char *mouse3Interruptu;
	extern volatile int mouse0foundz,mouse1foundz,mouse2foundz,mouse3foundz;
	extern volatile int mouse0foundc,mouse1foundc,mouse2foundc,mouse3foundc;
	extern volatile int mouse0leftup,mouse1leftup,mouse2leftup,mouse3leftup;

	extern char *KeyInterrupt;
	extern char *ADCInterrupt;
	extern volatile char *DACInterrupt;
	extern char *CountInterrupt;
	extern void mouse0close(void);
	extern void initMouse0(int sensitivity);
	extern int CMM1;
	extern int sendCRLF;
	extern void update_clock(void);
	extern void copy_clock(void);
	extern MMFLOAT optionangle;
	extern int optiony;
#endif
#endif
