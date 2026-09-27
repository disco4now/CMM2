
/***************************************************************************
CMM2 MMBasic
Flash.h

Include file that contains the globals and defines for flash save/load in MMBasic.

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

*******************************************************************************/
#include "stdint.h"
#include "Configuration.h"
#define FLASH_BASE_ADDR      (uint32_t)(FLASH_BASE)
#define FLASH_END_ADDR       (uint32_t)(0x081E0000)


	/* Base address of the Flash sectors Bank 1 */
	#define ADDR_FLASH_SECTOR_0_BANK1     ((uint32_t)0x08000000) /* Base @ of Sector 0, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_1_BANK1     ((uint32_t)0x08020000) /* Base @ of Sector 1, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_2_BANK1     ((uint32_t)0x08040000) /* Base @ of Sector 2, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_3_BANK1     ((uint32_t)0x08060000) /* Base @ of Sector 3, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_4_BANK1     ((uint32_t)0x08080000) /* Base @ of Sector 4, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_5_BANK1     ((uint32_t)0x080A0000) /* Base @ of Sector 5, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_6_BANK1     ((uint32_t)0x080C0000) /* Base @ of Sector 6, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_7_BANK1     ((uint32_t)0x080E0000) /* Base @ of Sector 7, 128 Kbytes */
	/* Base address of the Flash sectors Bank 2 */
	#define ADDR_FLASH_SECTOR_0_BANK2     ((uint32_t)0x08100000) /* Base @ of Sector 0, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_1_BANK2     ((uint32_t)0x08120000) /* Base @ of Sector 1, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_2_BANK2     ((uint32_t)0x08140000) /* Base @ of Sector 2, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_3_BANK2     ((uint32_t)0x08160000) /* Base @ of Sector 3, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_4_BANK2     ((uint32_t)0x08180000) /* Base @ of Sector 4, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_5_BANK2     ((uint32_t)0x081A0000) /* Base @ of Sector 5, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_6_BANK2     ((uint32_t)0x081C0000) /* Base @ of Sector 6, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_7_BANK2     ((uint32_t)0x081E0000) /* Base @ of Sector 7, 128 Kbytes */
	#define SAVED_VAR_RAM_ADDR     ((uint32_t)0x38800000)   /* Start of Saved Variables flash area */
    /* Setup the start of flash used by MMBasic ProgMemory  */
    #define PROGRAMSTARTCODE 0
 	#define FLASH_PROGRAM_ADDR    ADDR_FLASH_SECTOR_0_BANK2   /* Start Basic Program flash area */
   //#define PROGRAMSTARTCODE 1
   //#define FLASH_PROGRAM_ADDR   ADDR_FLASH_SECTOR_1_BANK2   /* Start Basic Program flash area */

	#define SAVED_VAR_RAM_SIZE 0x1000  // amount of flash reserved for saved variables
	#define FLASH_SAVED_OPTION_ADDR  ADDR_FLASH_SECTOR_7_BANK2   /* Start of Saved Options flash area */
	#define SAVED_OPTIONS_FLASH 4
/* Base address of the Flash sectors */

/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE) && !defined(FLASH_INCLUDED)
#define FLASH_INCLUDED

    // IMPORTANT: Change the string constant in cmd_memory() if you change PROG_FLASH_SIZE
#define EDIT_BUFFER_SIZE 512*1024// ((unsigned int)(RAMEND - (unsigned int)RAMBASE - 1024))  // this is the maximum RAM that we can get
struct option_s {
    char Autorun;
    char Tab;
    char Invert;
    char Listcase;
// 4
    uint32_t  PIN;
    uint32_t  Baudrate;
    int DefaultFC, DefaultBC;      // the default colours
    short DISPLAY_WIDTH;
    short DISPLAY_HEIGHT;
// 24
    short RTC_Calibrate;
    volatile uint8_t USBPolling;
    uint8_t mode;
// 28
    char Height;
    char Width;
    char Console;
    char DefaultFont;
// 32
    char SerialPullup;
    char fulltime;
    char USBKeyboard;
    int8_t colourmode;
// 36
    uint8_t showstatus;
    char colourmap;
    char SDspeed;
    uint8_t editfont;
// 40
    uint8_t F11Key[64];
    uint8_t F12Key[64];
// 168
    uint8_t rtcdrive;
    uint8_t noLED;
    uint8_t ConsolePort;
    int8_t Mouse;
    uint8_t Sensitivity;
    uint8_t RTCinstalled;
    uint8_t profile;
    uint8_t sleep;
    uint8_t CPUmode;
    int8_t ProgramStartCode;
// 178
	short RepeatStart;
	short RepeatRate;                   //180
	uint8_t path[128];                  //182
	uint16_t MaxCtrls;                  //310
	signed char offsets[MAX_MODES+1];   //312
	uint8_t spare1;
	uint8_t spare2;
	uint8_t spare3;                        //315
// 316
    uint8_t F15Key[64];
    uint8_t F16Key[64];                   //380
    uint8_t F19Key[64];                   //444
    uint8_t F20Key[64];                  //508
    uint8_t end;                         //572
};

extern volatile struct option_s Option, SOption;
extern unsigned char *CFunctionFlash;
extern volatile uint32_t  realflashpointer, realmempointer;
void ResetAllOptions(void);
void ResetAllFlash(void);
void SaveOptions(int check);
void LoadOptions(void);
int FlashWriteInit(uint32_t sector);
void FlashWriteByte(unsigned char b);
void FlashWriteWord(unsigned int i);
void FlashWriteAlign(void);
void FlashWriteClose(void);
void MemWriteByte(unsigned char b);
void MemWriteWord(unsigned int i);
void MemWriteAlign(void);
void MemWriteClose(void);
void UpdateFlash(uint32_t address, uint32_t data);
int GetFlashOption(const unsigned int *w) ;
void SetFlashOption(const unsigned int *w, int x) ;
uint32_t GetSector(uint32_t Address);
//void cmd_var(void);
long long int CallCFunction(char *CmdPtr, char *ArgList, char *DefP, char *CallersLinePtr);
extern char *SDMemory;
extern char *ProgMemory;
extern void ClearSavedVars(void);


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


#endif


/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	//{ "Var",	    	T_CMD,				0, cmd_var	},

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

#endif
