/***********************************************************************************************************************
MMBasic

MM_Misc.h

Include file that contains the globals and defines for Misc.c in MMBasic.
These are miscelaneous commands and functions that do not easily sit anywhere else.

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
