/***************************************************************************

CMM2 MMBasic
Flash.c

Handles saving and restoring from flash.

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
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
uint32_t GetSector(uint32_t Address);
// The CFUNCTION data comes after the program in program memory
// and this is used to point to its start
unsigned char *CFunctionFlash = NULL;
volatile struct option_s Option;
char *SDMemory;
char *ProgMemory=(char*)FLASH_PROGRAM_ADDR;
static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t SectorError = 0;
volatile union u_flash {
  uint64_t i64[4];
  uint8_t  i8[32];
  uint32_t  i32[8];
} FlashWord, MemWord;
volatile int i8p=0,mi8p=0;
extern volatile int ConsoleTxBufHead, ConsoleTxBufTail;
extern RTC_HandleTypeDef hrtc;
int sectorsave;
// globals used when writing bytes to flash
volatile uint32_t realflashpointer, realmempointer;
volatile uint8_t FlashDone=0;
extern const int xres[MAX_MODES+1];
extern const int yres[MAX_MODES+1];

// erase the flash and init the variables used to buffer bytes for writing to the flash
int FlashWriteInit(uint32_t sector) {
//	__IO uint32_t SectorsWRPStatus = 0xFFF;
    // Unlock the Flash to enable the flash control register access
	if(Option.ProgramStartCode<0){ realflashpointer=(uint32_t)ProgMemory;mymemset(ProgMemory,0xFF,512*1024); return 0; }
	int i=0;
	uint32_t *j;
    SCB_DisableICache() ;
    SCB_DisableDCache() ;
    HAL_FLASH_Unlock();
	i8p=0;
	for(i=0;i<8;i++)FlashWord.i32[i]=0xFFFFFFFF;
	i=0;
	sectorsave=sector;
    if(sector >= (uint32_t)ProgMemory){
		sectorsave=(uint32_t)ProgMemory;
		realflashpointer=(uint32_t)ProgMemory;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		EraseInitStruct.Banks         = FLASH_BANK_2;
		EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		EraseInitStruct.Sector = GetSector(sector);
		EraseInitStruct.NbSectors = 1;
		FlashDone=0;
		if(HAL_FLASHEx_Erase_IT(&EraseInitStruct)== HAL_OK){
			while(FlashDone==0){
				routinechecks(0);
			}
			j=(uint32_t *)sector;
			while(j<(uint32_t *)(sector+0x20000)){
			  if(*j++ != 0xFFFFFFFF)return 1;
			}
		} else return 1;
	}
	SCB_EnableICache() ;
	SCB_EnableDCache() ;
	return 0;
}
void MemWriteBlock(void){
    int i;
    uint32_t address=realmempointer-32;
    if(address % 32)error("Memory write address");
    mycpy((char *)address,(char*)&MemWord.i64[0],32);
	for(i=0;i<8;i++)MemWord.i32[i]=0xFFFFFFFF;
}
void FlashWriteBlock(void){
	if(Option.ProgramStartCode<0){realmempointer=realflashpointer; MemWriteBlock();realflashpointer=realmempointer;return;}
    int i, tries=0;
    uint32_t address=realflashpointer-32;
    uint32_t *there = (uint32_t *)address;
    if(address % 32)error("Flash write address");
    if(sectorsave == (uint32_t)ProgMemory && (address<(uint32_t)ProgMemory || address>=(uint32_t)ProgMemory+PROG_FLASH_SIZE))error("PROGRAM_FLASH location");
//    if(sectorsave == SAVED_OPTIONS_FLASH && (address<FLASH_SAVED_OPTION_ADDR || address>=FLASH_SAVED_OPTION_ADDR+0x20000))error("SAVED_OPTION_FLASH location");
//    if(sectorsave == SAVED_VARS_FLASH && (address<SAVED_VAR_RAM_ADDR || address>=SAVED_VAR_RAM_ADDR+0x20000))error("SAVED_VARS_FLASH location");
    for(i=0;i<8;i++){
    	if(there[i]!=0xFFFFFFFF) error("flash not erased");
    }
	while((HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint64_t)((uint32_t)FlashWord.i64)) != HAL_OK) && (tries++ < 10));
	if(tries==10)error("Flash write fail");
	for(i=0;i<8;i++)FlashWord.i32[i]=0xFFFFFFFF;
}
void MemWriteByte(unsigned char b) {
	realmempointer++;
	MemWord.i8[mi8p]=b;
	mi8p++;
	mi8p %= 32;
	if(mi8p==0){
		MemWriteBlock();
	}
}
// write a byte to flash
// this will buffer four bytes so that the write to flash can be a word
void FlashWriteByte(unsigned char b) {
	if(Option.ProgramStartCode<0){realmempointer=realflashpointer; MemWriteByte(b);realflashpointer=realmempointer;return;}
	realflashpointer++;
	FlashWord.i8[i8p]=b;
	i8p++;
	i8p %= 32;
	if(i8p==0){
		FlashWriteBlock();
	}
}
void MemWriteAlign(void) {
	  while(mi8p != 0) {
		  MemWriteByte(0x0);
	  }
	  MemWriteWord(0xFFFFFFFF);
}

void FlashWriteAlign(void) {
	if(Option.ProgramStartCode<0){MemWriteAlign();return;}
	  while(i8p != 0) {
		  FlashWriteByte(0x0);
	  }
	  FlashWriteWord(0xFFFFFFFF);
}
void MemWriteWord(unsigned int i) {
	MemWriteByte(i & 0xFF);
	MemWriteByte((i>>8) & 0xFF);
	MemWriteByte((i>>16) & 0xFF);
	MemWriteByte((i>>24) & 0xFF);
}


// utility routine used by SaveProgramToFlash() and cmd_var to write a byte to flash while erasing the page if needed
void FlashWriteWord(unsigned int i) {
	if(Option.ProgramStartCode<0){MemWriteWord(i); return;}
	FlashWriteByte(i & 0xFF);
	FlashWriteByte((i>>8) & 0xFF);
	FlashWriteByte((i>>16) & 0xFF);
	FlashWriteByte((i>>24) & 0xFF);
}
void MemWriteClose(void){
	  while(mi8p != 0) {
		  MemWriteByte(0xff);
	  }

}

// flush any bytes in the buffer to flash
void FlashWriteClose(void) {
	if(Option.ProgramStartCode<0){MemWriteClose(); return;}
	  while(i8p != 0) {
		  FlashWriteByte(0xff);
	  }
}


/*******************************************************************************************************************
 Code to execute a CFunction saved in flash
*******************************************************************************************************************/
/*******************************************************************************************************************
 VARSAVE and VARSAVE RESTORE commands

 Variables are saved in flash as follows:
 Numeric variables:
     1 byte  = variable type
     ? bytes = the variable's name in uppercase
     1 byte  = zero byte terminating the variable's name
     4 or 8 bytes = the value of the variable
 String variables:
     1 byte  = variable type
     ? bytes = the variable's name in uppercase
     1 byte  = zero byte terminating the variable's name
     1 bytes = length of the variable's string
     ? bytes = the variables string

********************************************************************************************************************/


/*******************************************************************************************************************
 The variables are stored in a reserved flash area (which in total is 2K).
 The first few bytes are used for the options. So we must save the options in RAM before we erase, then write the
 options back.  The variables saved by this command are then written to flash starting just after the options.
********************************************************************************************************************/
void cmd_var(void) {
    char *p, *buf, *bufp, *varp, *vdata, lastc;
    int i, j, nbr = 1, nbr2=1, array, type, SaveDefaultType;
    int VarList[MAX_ARG_COUNT];
    char *VarDataList[MAX_ARG_COUNT];
    char *SavedVarsFlash;
    char *w;
    if((p = checkstring(cmdline, "CLEAR"))) {
        checkend(p);
        ClearSavedVars();
        return;
    }
    if((p = checkstring(cmdline, "RESTORE"))) {
        char b[MAXVARLEN + 3];
        checkend(p);
        SavedVarsFlash = (char*)SAVED_VAR_RAM_ADDR;      // point to where the variables were saved
        SaveDefaultType = DefaultType;                              // save the default type
        bufp = SavedVarsFlash;   // point to where the variables were saved
        while(*bufp != 0xff) {                                      // 0xff is the end of the variable list
            type = *bufp++;                                         // get the variable type
            array = type & 0x80;  type &= 0x6f;                     // set array to true if it is an array
            if(!(type==T_INT || type==T_STR || type==T_NBR)){
            	ClearSavedVars();
            	error("Saved VARS corrupt - memory cleared %",type);
            }
            DefaultType = TypeMask(type);                           // and set the default type to this
            if(array) {
                strcpy(b, bufp);
                strcat(b, "()");
                vdata = findvar(b, type | V_EMPTY_OK | V_NOFIND_ERR);     // find an array
            } else
                vdata = findvar(bufp, type | V_FIND);               // find or create a non arrayed variable
            if(TypeMask(vartbl[VarIndex].type) != TypeMask(type)) error("$ type conflict", bufp);
            if(vartbl[VarIndex].type & T_CONST) error("$ is a constant", bufp);
            bufp += strlen((char *)bufp) + 1;                       // step over the name and the terminating zero byte
            if(array) {                                             // an array has the data size in the next two bytes
                nbr = *bufp++;
                nbr |= (*bufp++) << 8;
                nbr |= (*bufp++) << 16;
                nbr |= (*bufp++) << 24;
                nbr2 = 1;
                for(j = 0; vartbl[VarIndex].dims[j] != 0 && j < MAXDIM; j++)
                    nbr2 *= (vartbl[VarIndex].dims[j] + 1 - OptionBase);
                if(type & T_STR) nbr2 *= vartbl[VarIndex].size +1;
                if(type & T_NBR) nbr2 *= sizeof(MMFLOAT);
                if(type & T_INT) nbr2 *= sizeof(long long int);
                if(nbr2!=nbr)error("Array size");
            } else {
               if(type & T_STR) nbr = *bufp + 1;
               if(type & T_NBR) nbr = sizeof(MMFLOAT);
               if(type & T_INT) nbr = sizeof(long long int);
            }
            while(nbr--) *vdata++ = *bufp++;                        // copy the data
        }
        DefaultType = SaveDefaultType;
        return;
    }

     if((p = checkstring(cmdline, "SAVE"))) {
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");                  // getargs macro must be the first executable stmt in a block
        if(argc && (argc & 0x01) == 0) error("Invalid syntax");

        // befor we start, run through the arguments checking for errors
        // before we start, run through the arguments checking for errors
        for(i = 0; i < argc; i += 2) {
            checkend(skipvar(argv[i], false));
            VarDataList[i/2] = findvar(argv[i], V_NOFIND_ERR | V_EMPTY_OK);
            VarList[i/2] = VarIndex;
            if((vartbl[VarIndex].type & (T_CONST | T_PTR)) || vartbl[VarIndex].level != 0) error("Invalid variable");
            p = &argv[i][strlen(argv[i]) - 1];                      // pointer to the last char
            if(*p == ')') {                                         // strip off any empty brackets which indicate an array
                p--;
                if(*p == ' ') p--;
                if(*p == '(')
                    *p = 0;
                else
                    error("Invalid variable");
            }
        }
        // load the current variable save table into RAM
        // while doing this skip any variables that are in the argument list for this save
        bufp = buf = GetTempMemory(SAVED_VAR_RAM_SIZE);           // build the saved variable table in RAM
        uint32_t *writebuf = (uint32_t *)buf;
        uint32_t *readbuf = (uint32_t *)0x38800000;
        SavedVarsFlash = (char*)SAVED_VAR_RAM_ADDR;      // point to where the variables were saved
        varp = SavedVarsFlash;   // point to where the variables were saved
        while(*varp != 0 && *varp != 0xff) {                        // 0 or 0xff is the end of the variable list
            type = *varp++;                                         // get the variable type
            array = type & 0x80;  type &= 0x6f;                     // set array to true if it is an array
            if(!(type==T_INT || type==T_STR || type==T_NBR)){
            	ClearSavedVars();
            	error("Saved VARS corrupt - memory cleared");
            }
            vdata = varp;                                           // save a pointer to the name
            while(*varp) varp++;                                    // skip the name
            varp++;                                                 // and the terminating zero byte
            if(array) {                                             // an array has the data size in the next two bytes
                 nbr = (varp[0] | (varp[1] << 8) | (varp[2] << 16) | (varp[3] << 24)) + 4;
            } else {
                if(type & T_STR) nbr = *varp + 1;
                if(type & T_NBR) nbr = sizeof(MMFLOAT);
                if(type & T_INT) nbr = sizeof(long long int);
            }
            for(i = 0; i < argc; i += 2) {                          // scan the argument list
                p = &argv[i][strlen(argv[i]) - 1];                  // pointer to the last char
                lastc = *p;                                         // get the last char
                if(lastc <= '%') *p = 0;                            // remove the type suffix for the compare
                if(strncasecmp(vdata, argv[i], MAXVARLEN) == 0) {   // does the entry have the same name?
                    while(nbr--) varp++;                            // found matching variable, skip over the entry in flash (ie, do not copy to RAM)
                    i = 9999;                                       // force the termination of the for loop
                }
                *p = lastc;                                         // restore the type suffix
            }
            // finished scanning the argument list, did we find a matching variable?
            // if not, copy this entry to RAM
            if(i < 9999) {
                *bufp++ = type | array;
                while(*vdata) *bufp++ = *vdata++;                   // copy the name
                *bufp++ = *vdata++;                                 // and the terminating zero byte
                while(nbr--) *bufp++ = *varp++;                     // copy the data
            }
        }


        // initialise for writing to the flash
//        FlashWriteInit(SAVED_VARS_FLASH);
        ClearSavedVars();
        w=(char*)SAVED_VAR_RAM_ADDR;
//        for(i=0;i<SAVED_VAR_RAM_SIZE;i++)*w++=0xFF;

        // now write the variables in RAM recovered from the var save list
        w=(char*)SAVED_VAR_RAM_ADDR;
        while(buf < bufp){
        	*w++=*buf++;
        }

        // now save the variables listed in this invocation of VAR SAVE
        for(i = 0; i < argc; i += 2) {
            VarIndex = VarList[i/2];                                // previously saved index to the variable
            vdata = VarDataList[i/2];                               // pointer to the variable's data
            type = TypeMask(vartbl[VarIndex].type);                 // get the variable's type
            type |= (vartbl[VarIndex].type & T_IMPLIED);            // set the implied flag
            array = (vartbl[VarIndex].dims[0] != 0);

            nbr = 1;                                                // number of elements to save
            if(array) {                                             // if this is an array calculate the number of elements
                for(j = 0; vartbl[VarIndex].dims[j] != 0 && j < MAXDIM; j++)
                    nbr *= (vartbl[VarIndex].dims[j] + 1 - OptionBase);
                type |= 0x80;                                       // an array has the top bit set
            }

            if(type & T_STR) {
                if(array)
                    nbr *= (vartbl[VarIndex].size + 1);
                else
                    nbr = *vdata + 1;                               // for a simple string variable just save the string
            }
            if(type & T_NBR) nbr *= sizeof(MMFLOAT);
            if(type & T_INT) nbr *= sizeof(long long int);
            if((uint32_t)w - (uint32_t)SavedVarsFlash + 36 + nbr > SAVED_VAR_RAM_SIZE) {
                nbr= (0x1000>>2);
                while(nbr--)*writebuf++ = *readbuf++;
                SCB_CleanDCache_by_Addr ((uint32_t *)0x38800000, 0x1000);
                error("Not enough memory");
            }
            *w++=type;                              // save its type
            for(j = 0, p = vartbl[VarIndex].name; *p && j < MAXVARLEN; p++, j++)
                *w++=*p;                            // save the name
            *w++=0;                                 // terminate the name
            if(array) {                                             // if it is an array save the number of data bytes
               *w++=nbr; *w++=(nbr >> 8); *w++=(nbr >>16); *w++=(nbr >>24);
            }
            while(nbr--) *w++=(*vdata++);             // write the data
        }
        nbr= (0x1000>>2);
        while(nbr--)*writebuf++ = *readbuf++;
        SCB_CleanDCache_by_Addr ((uint32_t *)0x38800000, 0x1000);
        return;
     }
    error("Unknown command");
}
// erase the flash and init the variables used to buffer bytes for writing to the flash
int FlashOptionInit(int sector, int check) {
//	__IO uint32_t SectorsWRPStatus = 0xFFF;
    // Unlock the Flash to enable the flash control register access
	int i;
	uint32_t *j;
    SCB_DisableICache() ;
    SCB_DisableDCache() ;
	HAL_FLASH_Unlock();
	i8p=0;
	for(i=0;i<8;i++)FlashWord.i32[i]=0xFFFFFFFF;
	sectorsave=sector;
    // Clear pending flags (if any)

    // Get the number of the start and end sectors

       // Device voltage range supposed to be [2.7V to 3.6V], the operation will
       //  be done by word
      if(sector == SAVED_OPTIONS_FLASH){
      	  realflashpointer=FLASH_SAVED_OPTION_ADDR ;
    	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    	  EraseInitStruct.Banks         = FLASH_BANK_2;
    	  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    	  EraseInitStruct.Sector = FLASH_SECTOR_7;
    	  EraseInitStruct.NbSectors = 1;
   		  FlashDone=0;

		  if(HAL_FLASHEx_Erase_IT(&EraseInitStruct)== HAL_OK){
			  while(FlashDone==0){
				  if(check)routinechecks(0);

			  }
			  j=(uint32_t *)FLASH_SAVED_OPTION_ADDR;
			  while(j<(uint32_t *)(FLASH_SAVED_OPTION_ADDR+0x20000)){
				  if(*j++ != 0xFFFFFFFF)return 1;
			  }
    	  } else return 1;
      }
      SCB_EnableICache() ;
      SCB_EnableDCache() ;
      return 0;
}

/**********************************************************************************************
   These routines are used to load or save the global Option structure from/to flash.
   These options are stored in the beginning of the flash used to save stored variables.
***********************************************************************************************/
void SaveOptions(int check) {
	int i,tries;
    uint32_t address=FLASH_SAVED_OPTION_ADDR;
    char *p, *SavedOptionsFlash;
    SavedOptionsFlash=(char*)FLASH_SAVED_OPTION_ADDR;
    p = (char *)&Option;
    for(i = 0; i < sizeof(struct option_s); i++) {
    	if(SavedOptionsFlash[i] != *p) goto skipreturn;
    	p++;
    }
    return;                                                         // exit if the option has already been set (ie, nothing to do)
    skipreturn:

    while(!(ConsoleTxBufHead == ConsoleTxBufTail));                    // wait for the console UART to send whatever is in its buffer
    p = (char *)&Option;
	i=3;
	while(FlashOptionInit(SAVED_OPTIONS_FLASH, check) && i)i--;                     // erase program memory
	if(i==0)error("Failed to erase flash memory");
	p = (char *)&Option;
	while(p<(char *)&Option.end){
		for(i=0;i<8;i++)FlashWord.i32[i]=0xFFFFFFFF;
		for(i=0;i<32;i++)if(p<(char *)&Option.end)FlashWord.i8[i]=*p++;
		tries=0;
		while((HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint64_t)((uint32_t)FlashWord.i64)) != HAL_OK) && (tries++ < 10));
		if(tries==10)error("Option write fail");
		address+=32;
	}
//    for(i = 0; i < sizeof(struct option_s); i++){
//    	FlashWriteByte(*p);    // write the changed page back to flash
//    	p++;
//    }
//    FlashWriteClose();

}


void LoadOptions(void) {
    memcpy((struct option_s *)&Option, (struct option_s *)FLASH_SAVED_OPTION_ADDR, sizeof(struct option_s));
}


// erase all flash memory and reset the options to their defaults
// used on initial firmware run
void ResetAllOptions(void) {
    Option.PIN = 0;
    Option.Baudrate = CONSOLE_BAUDRATE;
    Option.Autorun = false;
    Option.Listcase = CONFIG_TITLE;
    Option.Tab = 2;
    Option.Invert = false;
    Option.Console = 3;
    Option.fulltime = 0;
    Option.mode=1;
    Option.DefaultFC = WHITE;
    Option.DefaultBC = BLACK;
    Option.RTC_Calibrate = 0;
    Option.SerialPullup = 1;
    Option.USBKeyboard = CONFIG_UK;
	Option.USBPolling = 2;
	Option.colourmode=1;
	Option.showstatus=1;
	Option.rtcdrive=(HAL_GetREVID()==0x1003? 2 : 2);
	Option.colourmap=2;
	Option.SDspeed=0;
	Option.RepeatStart=500;
	Option.RepeatRate=75;
	Option.editfont=1;
	Option.noLED=0;
	Option.ProgramStartCode=PROGRAMSTARTCODE;  //PROGRAMSTARTCODE is set in flash.h
	Option.ConsolePort=3;
	Option.Mouse=-1;
	Option.Sensitivity=0;
	Option.RTCinstalled=0;
	Option.sleep=0;
	Option.profile=0;
	Option.CPUmode=0;
	Option.MaxCtrls=0;
    gui_font_width = FontTable[Option.editfont >> 4][0] * (Option.editfont & 0b1111);
    gui_font_height = FontTable[Option.editfont >> 4][1] * (Option.editfont & 0b1111);
    Option.Height = yres[Option.mode] / gui_font_height;
    Option.Width = xres[Option.mode] / gui_font_width;

	mymemset((char *)Option.F12Key,0,sizeof(Option.F12Key));
	mymemset((char *)Option.F11Key,0,sizeof(Option.F11Key));
	mymemset((char *)Option.path,0,sizeof(Option.path));
	mymemset((char *)Option.offsets,0,sizeof(Option.offsets));
	mymemset((char *)Option.F15Key,0,sizeof(Option.F15Key));
	mymemset((char *)Option.F16Key,0,sizeof(Option.F16Key));
	mymemset((char *)Option.F19Key,0,sizeof(Option.F19Key));
	mymemset((char *)Option.F20Key,0,sizeof(Option.F20Key));
}
void ClearSavedVars(void) {
	int i;
	uint32_t *w;
    w=(uint32_t*)SAVED_VAR_RAM_ADDR;
    for(i=0;i<SAVED_VAR_RAM_SIZE>>2;i++)*w++=0xFFFFFFFF;
    w=(uint32_t*)SAVED_VAR_RAM_ADDR;
    for(i=0;i<SAVED_VAR_RAM_SIZE>>2;i++){
    	if(*w++ != 0xFFFFFFFF)error("Var Clear");
    }
    SCB_CleanDCache_by_Addr ((uint32_t *)0x38800000, 0x1000);
}

// erase all flash memory and reset the options to their defaults
// used on initial firmware run or when the user shorts pins 9 an 10 together on startup
void ResetAllFlash(void) {
	int i=3;
	ResetAllOptions();
	SaveOptions(0);                                     //  and write them to flash
	ClearSavedVars();					           	   // erase saved vars
    while(FlashWriteInit((uint32_t)ProgMemory) && i)i--;                     // erase program memory
    if(i==0)error("Failed to erase flash memory");
    FlashWriteByte(0); FlashWriteByte(0);              // terminate the program in flash
    FlashWriteClose();
    LoadOptions();
}

/**
  * @brief  Gets the sector of a given address
  * @param  Address Address of the FLASH Memory
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if(((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) || \
     ((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2)))
  {
    sector = FLASH_SECTOR_0;
  }
  else if(((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_2_BANK2) && (Address >= ADDR_FLASH_SECTOR_1_BANK2)))
  {
    sector = FLASH_SECTOR_1;
  }
  else if(((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_3_BANK2) && (Address >= ADDR_FLASH_SECTOR_2_BANK2)))
  {
    sector = FLASH_SECTOR_2;
  }
  else if(((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_4_BANK2) && (Address >= ADDR_FLASH_SECTOR_3_BANK2)))
  {
    sector = FLASH_SECTOR_3;
  }
  else if(((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_5_BANK2) && (Address >= ADDR_FLASH_SECTOR_4_BANK2)))
  {
    sector = FLASH_SECTOR_4;
  }
  else if(((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_6_BANK2) && (Address >= ADDR_FLASH_SECTOR_5_BANK2)))
  {
    sector = FLASH_SECTOR_5;
  }
  else if(((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_7_BANK2) && (Address >= ADDR_FLASH_SECTOR_6_BANK2)))
  {
    sector = FLASH_SECTOR_6;
  }
  else if(((Address < ADDR_FLASH_SECTOR_0_BANK2) && (Address >= ADDR_FLASH_SECTOR_7_BANK1)) || \
          ((Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7_BANK2)))
  {
     sector = FLASH_SECTOR_7;
  }
  else
  {
    sector = FLASH_SECTOR_7;
  }

  return sector;
}
