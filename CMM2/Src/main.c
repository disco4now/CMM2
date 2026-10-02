/***************************************************************************

CMM2 MMBasic
Main program body

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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "Memory.h"
#include <time.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define progress  "\rprogress\r\n"
#define MAINCLOCKSPEED ((HAL_GetREVID()==0x1003 || G1Hardware>=2) ? (Overclock==0 ? 504 :400) : (Overclock==0 ? 504 :480) )
#define M40CLOCKSPEED (MAINCLOCKSPEED>>1)-1
#define LOGO_Y      0
#define LOGO_BYTES   1244                                           // number of bytes in logo image
#define LOGO_WIDTH   205                                            // width of the logo image in bits
#define SDRAM_TIMEOUT                    ((uint32_t)0xFFFF)
#define REFRESH_COUNT                    ((uint32_t)(G1Hardware==1 ? 0x0603 : (HAL_GetREVID()==0x1003 ? 781 : 938)))  /* SDRAM refresh counter */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

DAC_HandleTypeDef hdac1;

DMA2D_HandleTypeDef hdma2d;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c4;

JPEG_HandleTypeDef hjpeg;

extern LTDC_HandleTypeDef hltdc;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;
FDCAN_HandleTypeDef hfdcan;   //CAN added
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

SDRAM_HandleTypeDef hsdram1;
/* USER CODE BEGIN PV */
const int enableexFAT = 1;
SPI_HandleTypeDef SD_SPI;
struct s_PinDef *PinDef;
struct s_ModeDef *ModeDef;
int PromptFont, PromptFC, PromptBC;                             // the font and colours selected at the prompt
char BreakKey = BREAK_KEY;                                          // defaults to CTRL-C.  Set to zero to disable the break function
char IgnorePIN = false;
char WatchdogSet = false;
uint8_t RxBuffer, TxBuffer;
int WritePage=0;
int ReadPage=0;
//uint8_t* volatile DataPageAddress = (void *)0x58021400;
const int64_t initoptions[4] __attribute__ ((aligned (32)))  = {-1,-1,-1,-1};
volatile uint8_t pagesetdone=0;
int ShortScroll;
int SystemError=0;
void firstinit(void);
extern void JumpToBootloader(void);
int docheck=1;
FATFS fs;                 // Work area (file system object) for logical drive
struct s_PinDef *PinDef;
struct s_ModeDef *ModeDef;
int PromptFont, PromptFC, PromptBC;                             // the font and colours selected at the prompt
uint8_t RxBuffer, TxBuffer;
int MMCharPos;
int helpquotes=0;
volatile int MMAbort = false;
int use_uart;
char bootcause[12]={0};
unsigned int __attribute__((section(".my_section"))) _excep_dummy; // for some reason persistent does not work on the first variable
unsigned int __attribute__((section(".my_section"))) _excep_code;  //  __attribute__ ((persistent));  // if there was an exception this is the exception code
unsigned int __attribute__((section(".my_section"))) _excep_addr;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".my_section"))) _excep_cause;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".my_section"))) _excep_keys;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".my_section"))) rtcChange;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".backup"))) backup_ram[1024];  //  __attribute__ ((persistent));  // and this is the address
char *InterruptReturn = NULL;
int BasicRunning = false;
volatile int keyboardseen=0;
volatile char ConsoleRxBuf[CONSOLE_RX_BUF_SIZE];
volatile int ConsoleRxBufHead = 0;
volatile int ConsoleRxBufTail = 0;
volatile char ConsoleTxBuf[CONSOLE_TX_BUF_SIZE];
volatile int ConsoleTxBufHead = 0;
volatile int ConsoleTxBufTail = 0;
uint8_t BlinkSpeed = 0;//, str[20];
extern void printoptions(void);
void initConsole(void);
void EditInputLine(void);
volatile uint64_t Count5High = 0;
uint32_t ticks_per_microsecond;
uint32_t PLL3M,PLL3N,PLL3P,PLL3Q,PLL3R,PLL3RGE;
volatile uint64_t uSecTimer=0;
volatile uint64_t g_uptime=0;
volatile uint64_t FastTimer=0;
volatile unsigned int SleepTimer=0;
volatile int sleeping=0;
char errstring[256]={0};
int myDummy=0;  //Used in fix for intermittent USBKeyboard connection
int errpos=0;
int ShortScroll;
int G1Hardware;
int Overclock;
int executerun=0;
char canopen=0;	  //CAN has no pins assigned
void firstinit(void);
const uint32_t progstart[]={
		ADDR_FLASH_SECTOR_0_BANK2,
		ADDR_FLASH_SECTOR_1_BANK2,
		ADDR_FLASH_SECTOR_2_BANK2,
		ADDR_FLASH_SECTOR_3_BANK2,
		ADDR_FLASH_SECTOR_4_BANK2,   //was missing G.A.
		ADDR_FLASH_SECTOR_5_BANK2,
		ADDR_FLASH_SECTOR_6_BANK2,
};
char FunKey[NBRPROGKEYS][MAXKEYLEN + 1]= {		// data storage for the function keys
		{"FILES\r\n"},//Run immediately
		{"RUN\r\n"},
		{"LIST\r\n"},
		{"EDIT\r\n"},
//
		{"AUTOSAVE N \"\"\x82"},// Position cursor inside pair of double quotes
		{"XMODEM RECEIVE \"\"\x82"},
		{"XMODEM SEND \"\"\x82"},
		{"EDIT \"\"\x82"},
		{"LIST \"\"\x82"},
		{"RUN \"\"\x82"},
		{""},
		{""}
};
char lastcmd[STRINGSIZE*4];                                           // used to store the last command in case it is needed by the EDIT command
uint8_t OptionConsole=3;
SDRAM_HandleTypeDef      hsdram;
FMC_SDRAM_TimingTypeDef  SDRAM_Timing;
FMC_SDRAM_CommandTypeDef command;
extern int keyselect;
extern char *KeyInterrupt;
extern void setterminal(void);
extern BYTE (*xchg_byte) (BYTE data_out);
extern BYTE xchg_spi (BYTE data_out);
extern BYTE xchg_bitbang (BYTE data_out);
extern const unsigned char  Misc_12x20_LE[2854];
extern const unsigned char  Hom_16x24_LE[4564];
extern const unsigned char font1[];
extern void GT911_Scan(void);
extern void closecursor();
extern void (*xmit_byte_multi) (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
);
extern void (*rcvr_byte_multi) (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
);
extern void xmit_spi_multi (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
);
extern void rcvr_spi_multi (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
);
extern void xmit_bitbang_multi (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
);
extern void rcvr_bitbang_multi (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
);
extern char *firststmt;
extern char *nextstmt;

static inline CommandToken commandtbl_decode(const char *p)
{
#ifdef CMD16BIT	
	return ((CommandToken)(p[0] & 0x7f)) | ((CommandToken)(p[1] & 0x7f) << 7);
#else
    return ((CommandToken)(p[0])) ;
#endif
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static int MX_RTC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DAC1_Init(void);
static void MX_FMC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C4_Init(void);
static void MX_RNG_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM16_Init(void);
static void MX_TIM17_Init(void);
static void MX_UART4_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
static void MX_JPEG_Init(void);
static void MX_LTDC_Init(void);
static void MX_FDCAN1_Init(void);   //CAN added
//static void MX_DMA2D_Init(void);
void MX_USB_HOST_Process(void);
static int getVersion(void);
//static uint32_t TimeoutCalculation(uint32_t timevalue);
/* USER CODE BEGIN PFP */
void PrintStatus(int now);
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#define RoundUptoPage512(a)     ((((uint64_t)a) + (uint64_t)(512*1024 - 1)) & (uint64_t)(~(512*1024 - 1)))// round up to the nearest whole integer
void DrawLogo(void);
void InsertLastcmd(char *s);
static void MX_SD_SPI_Init(void);
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
static void USBearly(void);
void Power_Up(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
extern uint32_t _sfastcode, _efastcode, _sifastcode;
static void init_fastcode_section(void) {
    uint8_t *fastcode_ram_start = (uint8_t *) &_sfastcode,
            *fastcode_ram_end = (uint8_t *) &_efastcode,
            *fastcode_flash_start = (uint8_t *) &_sifastcode;
    size_t len = fastcode_ram_end - fastcode_ram_start;
    for(size_t i=0; i<len; i++) {
        *(fastcode_ram_start+i) = *(fastcode_flash_start+i);
    }
}
*/

void cleanend(void){
	int i, adjust=0;
	int maxH=PageTable[WritePage].ymax;
    mymemset(inpbuf,0,STRINGSIZE);
    int lastgui=gui_font_height;
	SetFont(Option.DefaultFont);
	adjust=gui_font_height-lastgui;
    if(GUIactive || autocursor){
    	GUIactive=0;
    	autocursor=0;
    	cmd_cursor("OFF");
    }
	closecursor();
	if(mouse1)i2c_disable();                                                  // close I2C
    if(mouse2)i2c2_disable();                                                  // close I2C
    if(mouse3)i2c3_disable();                                                  // close I2C
	ShareNeeded=RoundUptoPage512((uint32_t)PageTable[0].address);
	MPU_Config_nCacheable(0);
	HAL_LTDC_SetAddress(&hltdc,  (uint32_t)PageTable[0].address, LTDC_LAYER_1);
    mouse0close();
    mouse0=0; mouse1=0; mouse2=0; mouse3=0;
    OnKeyGOSUB=NULL;							            // set the next stmt to the interrupt location
    com1_interrupt=NULL;									// set the next stmt to the interrupt location
    com1_TX_interrupt=NULL;
    com2_interrupt=NULL;									// set the next stmt to the interrupt location
    com2_TX_interrupt=NULL;
    com3_interrupt=NULL;									// set the next stmt to the interrupt location
    com3_TX_interrupt=NULL;
    KeyInterrupt=NULL;									    // set the next stmt to the interrupt location
    WAVInterrupt=NULL;									    // set the next stmt to the interrupt location
    COLLISIONInterrupt=NULL;									    // set the next stmt to the interrupt location
    ADCInterrupt=NULL;									    // set the next stmt to the interrupt location
    DACInterrupt=NULL;									    // set the next stmt to the interrupt location
    IrInterrupt=NULL;									    // set the next stmt to the interrupt location
    FrameInterrupt=NULL;									    // set the next stmt to the interrupt location
    CountInterrupt=NULL;
    nun1Interruptz=NULL;
    nun1Interruptc=NULL;
    nun2Interruptz=NULL;
    nun2Interruptc=NULL;
    nun3Interruptz=NULL;
    nun3Interruptc=NULL;
    mouse0Interruptz=NULL;
    mouse0Interruptc=NULL;
    mouse1Interruptz=NULL;
    mouse1Interruptc=NULL;
    mouse2Interruptz=NULL;
    mouse2Interruptc=NULL;
    mouse3Interruptz=NULL;
    mouse3Interruptc=NULL;
    autocursor=0;
    mouseupdated=0;
    for(i = 0; i < NBRINTERRUPTS; i++) {                            // scan through the interrupt table
    	inttbl[i].intp=NULL;							// set the next stmt to the interrupt location
    }
    for(i = 0; i < NBRSETTICKS; i++) {
    	TickInt[i]=NULL;
    }
	keyselect=0;
	ScrollLCD(adjust,1);
	CurrentY-=adjust;
    if(CurrentY + gui_font_height >= maxH-(ShortScroll?gui_font_height:0)-1) {
		ShortScroll=Option.showstatus;
        if(!ShortScroll){
            ScrollLCD(CurrentY + gui_font_height- maxH , 1);
        	CurrentY -= (CurrentY + gui_font_height * 2 - maxH );
        } else {
        	ScrollLCD(lastgui,1);
        	CurrentY -= (gui_font_height * 2 - adjust);
        }
//		CurrentY= maxH-(gui_font_height*2);
//    		CurrentY=CurrentY-(gui_font_height);
	}
	sendCRLF = 3;
	SCB_CleanInvalidateDCache();
	CloseAudio(1);
	CloseAllFiles();

    longjmp(mark, 1);
}

void RAMSETUP(void){
	  /*##-1- Configure the SDRAM device #########################################*/
	  /* SDRAM device configuration */
	  hsdram.Instance = FMC_SDRAM_DEVICE;

	    /* Timing configuration for 100Mhz as SDRAM clock frequency (System clock is up to 200Mhz) */
	  SDRAM_Timing.LoadToActiveDelay    = 2;
	  SDRAM_Timing.ExitSelfRefreshDelay = 7;
	  SDRAM_Timing.SelfRefreshTime      = 4;
	  SDRAM_Timing.RowCycleDelay        = 7;
	  SDRAM_Timing.WriteRecoveryTime    = 2;
	  SDRAM_Timing.RPDelay              = 2;
	  SDRAM_Timing.RCDDelay             = 2;

	  hsdram.Init.SDBank             = FMC_SDRAM_BANK2;
	  hsdram.Init.ColumnBitsNumber   = ((G1Hardware & 1) ? FMC_SDRAM_COLUMN_BITS_NUM_8 : FMC_SDRAM_COLUMN_BITS_NUM_9);
	  hsdram.Init.RowBitsNumber      = ((G1Hardware & 1) ? FMC_SDRAM_ROW_BITS_NUM_12 :  FMC_SDRAM_ROW_BITS_NUM_13);
	  hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_16;
	  hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	  hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
	  hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	  hsdram.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
	  hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
	  hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

	  /* Initialize the SDRAM controller */
	  if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK)
	  {
	    /* Initialization Error */
	    SystemError=1;Error_Handler();
	  }

	  /* Program the SDRAM external device */
	  SDRAM_Initialization_Sequence(&hsdram, &command);

}


/* USER CODE END 0 */
void rtcChangeSet(void){
	rtcChange=0;
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	int i, firsttime=0;
	static int ErrorInPrompt;
	int64_t* volatile test;
	test=(int64_t* volatile)initoptions;
	ProgMemory=(char*)FLASH_PROGRAM_ADDR;
	/* USER CODE END 1 */
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  USBearly();
  /* USER CODE BEGIN Init */
  //Copy fastcode to ITCRAM
 // init_fastcode_section() ;

if(_excep_code == RESTART_BOOT0){
	_excep_code = 0;
	JumpToBootloader();
}

  /* USER CODE END Init */
//  Overclock=getOverclock();
  G1Hardware=getVersion();
  /* Configure the system clock */
  LoadOptions();
  G1Hardware |= (Option.CPUmode & 0x0f);
  Overclock = ((Option.CPUmode >>4 ) ? 0:1);
  SystemClock_Config();
  /* USER CODE BEGIN SysInit */
  firsttime=MX_RTC_Init();
  HAL_FLASH_Unlock();


  if(!(Option.Tab==2 || Option.Tab==3  || Option.Tab==4  || Option.Tab==8 ) || //check for valid options
//		  !(Option.RTC_Calibrate>=-511 && Option.RTC_Calibrate<=512) ||
//		  !(Option.MaxCtrls>=-0 && Option.MaxCtrls<=2000) ||
//		  !(Option.ConsolePort>=1 && Option.ConsolePort<=3) ||
//		  !(Option.rtcdrive>=0 && Option.rtcdrive<=3) ||
		  !(Option.USBKeyboard>=0 && Option.USBKeyboard<=MAXKEYBOARDS))// ||
//		  !(Option.mode==1 || Option.mode>=8 ) ||
//		  !(Option.editfont==1 || Option.editfont==2 || Option.editfont==3 || Option.editfont==4 || Option.editfont==7) ||
//		  !(Option.ProgramStartCode >= -2 && Option.ProgramStartCode <= 7) ||
//		  !(Option.RepeatStart>=100 && Option.RepeatRate>=25)) {
{
	  ResetAllFlash();
	  rtcChange=1;
	  firsttime=1;
  }

  if(*test==-1){  //first time in
	  int keyboard=Option.USBKeyboard;
	  int rtccal=Option.RTC_Calibrate;
	  char rtcdrive=Option.rtcdrive;
	  int consoleport=Option.ConsolePort;
	  int sdspeed=Option.SDspeed;
	  int mode=Option.mode;
	  int mouse=Option.Mouse;
	  uint8_t noLED=Option.noLED;
	  int RTCi=Option.RTCinstalled;
	  int editfont=Option.editfont;
	  int maxctrls=Option.MaxCtrls;
	  ResetAllFlash();
	  Option.noLED=noLED;
	  Option.USBKeyboard=keyboard;
	  Option.RTC_Calibrate=rtccal;
	  Option.rtcdrive=rtcdrive;
	  Option.SDspeed=sdspeed;
	  Option.ConsolePort=consoleport;
	  Option.mode=mode;
	  Option.editfont=editfont;
	  Option.Mouse=mouse;
	  Option.RTCinstalled=RTCi;
	  Option.MaxCtrls=maxctrls;
	  SaveOptions(0);
	  gui_fcolour = PromptFC = Option.DefaultFC = WHITE;
	  gui_bcolour = PromptBC = Option.DefaultBC = BLACK;
	  _excep_code=0;
	  uint64_t i64[4]={0};
	  int tries=0;
	  while((HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, (uint32_t)test, (uint64_t)((uint32_t)i64)) != HAL_OK) && (tries++ < 10));
	  if(tries==10)error("Flash write fail");
	  ClearSavedVars();
  }

  HAL_PWREx_EnableBkUpReg();
  HAL_PWR_EnableBkUpAccess();
//  strcpy(FunKey[10],(char *)Option.F11Key);
//  strcpy(FunKey[11],(char *)Option.F12Key);
  if(rtcChange!=0x76767676){
//	  uint32_t subsec=2048 | 0x80000000;
	  __HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);
	  int up=RTC_SMOOTHCALIB_PLUSPULSES_RESET;
	  int calibrate= -Option.RTC_Calibrate;
	  if(Option.RTC_Calibrate>0){
		  up=RTC_SMOOTHCALIB_PLUSPULSES_SET;
		  calibrate=512-Option.RTC_Calibrate;
	  }
	  HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, up, calibrate);

	  if(Option.rtcdrive==0)__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
	  else if(Option.rtcdrive==1)__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);
	  else if(Option.rtcdrive==2)__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);
	  else __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
//	  __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
//	  RTC->SHIFTR=subsec;
//	  __HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);

	  __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
	  rtcChange=0x76767676;
  }
  OptionConsole=Option.Console;
  PromptFont = Option.DefaultFont;
  ShortScroll = Option.showstatus;
  PeripheralBusSpeed=SystemCoreClock/2;
  ticks_per_microsecond=PeripheralBusSpeed/1000000;
  PinDef = (struct s_PinDef *)PinDef40;
  xchg_byte=(Option.SDspeed==0 ? xchg_bitbang : xchg_spi);
  xmit_byte_multi=(Option.SDspeed==0 ? xmit_bitbang_multi : xmit_spi_multi);
  rcvr_byte_multi=(Option.SDspeed==0 ? rcvr_bitbang_multi : rcvr_spi_multi);
  RAMBASE=(0x30008000 + MRoundUp(Option.MaxCtrls * sizeof(struct s_ctrl)));
  goto skip;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USB_HOST_Init();
  MX_DAC1_Init();
  MX_FMC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C4_Init();
  MX_RNG_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_TIM8_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  MX_UART4_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_JPEG_Init();
  MX_LTDC_Init();
  MX_FDCAN1_Init();
//  MX_DMA2D_Init();
  /* USER CODE BEGIN 2 */
skip:
//MPU_Config_nCacheable(0);
MX_GPIO_Init();
MX_USB_HOST_Init();   //USBFIX for KEYBOARD Call it early
MX_TIM2_Init();
MX_TIM3_Init();
MX_TIM4_Init();
MX_TIM5_Init();
MX_TIM8_Init();
MX_TIM16_Init();
MX_RNG_Init();
MX_SPI2_Init();
MX_SPI1_Init();
MX_DAC1_Init();
MX_I2C1_Init();
MX_I2C2_Init();
MX_I2C4_Init();
MX_GPIO_Init();
RAMSETUP();
FMC_Bank1_R->BTCR[1] = 0x000030D2;
if(Option.ProgramStartCode>=0){
	ProgMemory=(char *)((uint32_t)progstart[Option.ProgramStartCode]);
	if(!(*ProgMemory==0 || *ProgMemory==1)){
		int i=3;
		while(FlashWriteInit((uint32_t)ProgMemory) && i)i--;                     // erase program memory
		if(i==0)error("Failed to erase flash memory");
	    FlashWriteByte(0); FlashWriteByte(0); FlashWriteByte(0);    // terminate the program in flash
	    FlashWriteClose();
	}
} else {
	if(G1Hardware)ProgMemory = (char *)0xD0300000;
	else ProgMemory = (char *)0xD0780000;
}
{
    int reset=3;
	  /*Configure RESET  pin : Pin40 */
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	while(!HAL_GPIO_ReadPin(GPIOB,  GPIO_PIN_13) && reset){
		MM_Delay(300);
		reset--;
	}
	if(reset==0){
		int i=3;
		while(FlashWriteInit((uint32_t)ProgMemory) && i)i--;                     // erase program memory
		if(i==0)error("Failed to erase flash memory");
		ResetAllFlash();
	}
}
HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAMB2);
initExtIO();
initConsole();
if(OptionConsole & 1)start_console();
InitDisplayOther();
InitHeap();
InitBasic();
InitFileIO();
BasicRunning = true;
if(Option.USBKeyboard != NO_KEYBOARD){
  clearrepeat();
  MX_USB_HOST_Init();
  HID_MenuInit();
  i=20;
  while(i--){
		HID_MenuProcess();
		MX_USB_HOST_Process();
		HAL_Delay(Option.USBPolling);
  }
}
_excep_keys=10;
HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
for(i=0;i<=2000;i+=10){
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, i);
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, i);
	HAL_Delay(1);
}
*tknbuf = 0;
HAL_TIM_Base_Start_IT(&htim16);
if(Option.SDspeed)MX_SD_SPI_Init();
ErrorInPrompt = false;
volatile uint32_t *memtest=(uint32_t *)0xD0000000;
for(i=0;i<2097152*( G1Hardware ? 1 : 4);i++)*memtest++=0xAA5555AA;
memtest=(uint32_t *)0xD0000000;
for(i=0;i<2097152*( G1Hardware ? 1 : 4);i++){
	if(*memtest++ != 0xAA5555AA)break;
}
if(i!=2097152*( G1Hardware ? 1 : 4)){
	MMPrintString("Warning 0xAA5555AA SDRAM error at ");PIntH((uint32_t)memtest);PIntHC(*memtest);MMPrintString("\r\n");
	HAL_Delay(1000);
}
memtest=(uint32_t *)0xD0000000;
for(i=0;i<2097152*( G1Hardware ? 1 : 4);i++)*memtest++ = 0x55AAAA55;
memtest=(uint32_t *)0xD0000000;
for(i=0;i<2097152*( G1Hardware ? 1 : 4);i++){
	if(*memtest++ != 0x55AAAA55)break;
}
if(i!=2097152*( G1Hardware ? 1 : 4)){
	MMPrintString("Warning 0x55AAAA55 SDRAM error at ");PIntH((uint32_t)memtest);PIntHC(*memtest);MMPrintString("\r\n");
	HAL_Delay(1000);
}
memtest=(uint32_t *)0xD0000000;
for(i=0;i<2097152*( G1Hardware ? 1 : 4);i++)*memtest++ = 0x0;
int maxW=PageTable[WritePage].xmax;
int maxH=PageTable[WritePage].ymax;
if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)strcpy(bootcause,"Switch\r\n");
if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)strcpy(bootcause,"Power-On\r\n");
if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)strcpy(bootcause,"Software\r\n");
if(!(_excep_code == RESTART_NOAUTORUN || _excep_code == RESET_COMMAND || _excep_code == SCREWUP_TIMEOUT|| _excep_code == WATCHDOG_TIMEOUT|| _excep_code == RESTART_HEAP)){
	  if(Option.Autorun==0 ){
		  DrawLogo();
		  SerUSBPutS("\033[?25h");
		  SerUSBPutS("\033[37m");
		  SerUSBPutS("\033[m");
		  GUIPrintString(maxW/2, 70+gui_font_height, gui_font, JUSTIFY_CENTER, JUSTIFY_MIDDLE, ORIENT_NORMAL, WHITE, BLACK, (G1Hardware ? (Overclock==0 ? MES_SIGNON504 : ((HAL_GetREVID()==0x1003 || G1Hardware>=2) ? MES_SIGNON400:MES_SIGNON480)) : (Overclock==0 ? MES_SIGNON504G2 : (HAL_GetREVID()==0x1003 ? MES_SIGNON400G2:MES_SIGNON480G2))));
		  SerUSBPutS((G1Hardware ? (Overclock==0 ? MES_SIGNON504 : ((HAL_GetREVID()==0x1003 || G1Hardware>=2) ? MES_SIGNON400:MES_SIGNON480)) : (Overclock==0 ? MES_SIGNON504G2 : (HAL_GetREVID()==0x1003 ? MES_SIGNON400G2:MES_SIGNON480G2))));
		  GUIPrintString(maxW/2, 70+gui_font_height*2, gui_font, JUSTIFY_CENTER, JUSTIFY_MIDDLE, ORIENT_NORMAL, WHITE, BLACK, MES_SIGNON1);
		  SerUSBPutS(MES_SIGNON1);
		  GUIPrintString(maxW/2, 70+gui_font_height*3, gui_font, JUSTIFY_CENTER, JUSTIFY_MIDDLE, ORIENT_NORMAL, WHITE, BLACK, COPYRIGHT0);
		  SerUSBPutS(COPYRIGHT0);
		  GUIPrintString(maxW/2, 70+gui_font_height*4, gui_font, JUSTIFY_CENTER, JUSTIFY_MIDDLE, ORIENT_NORMAL, WHITE, BLACK, COPYRIGHT1);
		  SerUSBPutS(COPYRIGHT1);
		  GUIPrintString(maxW/2, 70+gui_font_height*5, gui_font, JUSTIFY_CENTER, JUSTIFY_MIDDLE, ORIENT_NORMAL, WHITE, BLACK, COPYRIGHT2);
		  SerUSBPutS(COPYRIGHT2A);
		  CurrentY=70+gui_font_height*5;
		  MMPrintString("\r\n");
		  OptionFileErrorAbort = 1;
	  }
}
__HAL_RCC_CLEAR_RESET_FLAGS();
if(Option.ProgramStartCode < 0){
	mymemset(ProgMemory,0xFF,512*1024);
    FlashWriteByte(0); FlashWriteByte(0); FlashWriteByte(0);    // terminate the program in flash
    FlashWriteClose();
}
if(Option.profile){
	if(G1Hardware)mymemset((char *)0xD0300000, 0, 512*1024);
	else mymemset((char *)0xD0700000, 0, 512*1024);
    firststmt= (char *)ProgMemory;
    nextstmt = (char *)ProgMemory;
}

if(_excep_code == WATCHDOG_TIMEOUT) {
    WatchdogSet = true;                                 // remember if it was a watchdog timeout
    MMPrintString("\r\n\nWatchdog timeout\r\n");
}
if(_excep_code == SCREWUP_TIMEOUT) {
    MMPrintString("\r\n\nCommand timeout\r\n");
}
if(_excep_code == RESTART_HEAP) {
    MMPrintString("\r\n\nError: Heap Overrun\r\n");
}

CloseAllFiles();
while ((i = getConsole()) != -1){;} //clear anything in console
if(setjmp(mark) != 0) {    //Longjump to recover from error or CNTRL+C

	  SerUSBPutS("\033[?25h");
	  SerUSBPutS("\033[37m");
	  SerUSBPutS("\033[m");
//  	  SerUSBPutS("\033[20h");
	  sendCRLF=3;
	  FontTable[0]=(unsigned char *)font1;
	  FontTable[1]=(unsigned char *)Misc_12x20_LE;
	  FontTable[2]=(unsigned char *)Hom_16x24_LE;
	  LoadOptions();
	  CMM1 = false;
	  optionangle=1.0;
	  optiony=0;
	  clearrepeat();
	  FreeMemorySafe((void *)&PageTable[WPN].address);
	  FreeMemorySafe((void *)&PageTable[BPN].address);
	  if(CurrentlyPlaying != P_NOTHING)CloseAudio(1);
	  FrameInterrupt=NULL;
	  if(gif!=NULL){
		GifTimer=5000;
		FileClose(giffnbr);
		FreeMemorySafe((void*)&frame);
		gd_close_gif(gif);
		gif=NULL;
		GifTimer=0;
		giffnbr=0;
	  }
	  deferredcopy=0;
	  if((Option.Console & 1) && !(OptionConsole & 1)){
		  SerialClose(3);
		  start_console();
	  }
	  OptionConsole=Option.Console;
	  setmode(Option.mode,DEFCOLOUR,0,0);
	  WritePage=ReadPage=0;
	  if(CurrentY<0)CurrentY=0;
	  if(errpos){
		  hidecursor(0);
		  cursorenable=0;
		  FreeMemorySafe((void *)&cursorsave);
	      CloseAllFiles();
		  ShortScroll=Option.showstatus;
		  MMPrintString(errstring);
		  while(ConsoleTxBufHead != ConsoleTxBufTail){};
	      mymemset(inpbuf,0,STRINGSIZE);
	  }
	  errpos=0;
	  errstring[0]=0;
	  MMPrintString("\r\n");
    // we got here via a long jump which means an error or CTRL-C or the program wants to exit to the command prompt
    ContinuePoint = nextstmt;                                   // in case the user wants to use the continue command
    *tknbuf = 0;                                                // we do not want to run whatever is in the token buffer
} else {  //Normal Start
	ClearProgram();
	PrepareProgram(true);
	_excep_cause = CAUSE_MMSTARTUP;
	if(FindSubFun("MM.STARTUP", 0) >= 0) {                  //Execute MM.STARTUP if it exists
	ExecuteProgram("MM.STARTUP\0");
	}
	_excep_cause = CAUSE_NOTHING;
	if(Option.Autorun && *ProgMemory == 0x01 && _excep_code != RESTART_NOAUTORUN) {
	  strcpy(inpbuf,"RUN\r\n");
	  tokenise(true);   // turn into executable code
	  executerun=1;               //G.A. Fix for AUTORUN ON required an explicit END command
	  ExecuteProgram(tknbuf);                                     // execute the line straight away
	  if(executerun)cleanend();   //G.A. Fix for AUTORUN ON required an explicit END command
	}
	if(Option.Autorun && Option.ProgramStartCode < 0 && _excep_code != RESTART_NOAUTORUN) {   //i.e. OPTION RAM is used
	  FRESULT fr;
	  FILINFO fno;
	  OptionFileErrorAbort = 0;
	  if(InitSDCard()) { //make sure the SDcard is there
		  fr = f_stat("AUTORUN.BAS", &fno); //check for the file in the current directory
		  if(fr == FR_OK){ // file was not in the current directory so insert the path
			  FileLoadProgram("AUTORUN.BAS",0);
			  strcpy(inpbuf,"RUN\r\n");
			  tokenise(true);                                             // turn into executable code
			  executerun=1;
			  ExecuteProgram(tknbuf);                                     // execute the line straight away

		  }
	  }
	  OptionFileErrorAbort = 1;
	  mymemset(inpbuf,0,STRINGSIZE);
	}
}
if(firsttime==1)firstinit();
copy_clock();
if(Option.sleep)SleepTimer=Option.sleep*1000*60;
  /* USER CODE END 2 */
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  executerun=0;
	  ShortScroll=Option.showstatus;
	  MMAbort = false;
      BreakKey = BREAK_KEY;
      EchoOption = true;
      LocalIndex = 0;                                             // this should not be needed but it ensures that all space will be cleared
      ClearTempMemory();                                           // clear temp string space (might have been used by the prompt)
      CurrentLinePtr = NULL;                                      // do not use the line number in error reporting
      if(MMCharPos > 1) MMPrintString("\r\n");                    // prompt should be on a new line
        while(Option.PIN && !IgnorePIN) {
          _excep_code = PIN_RESTART;
          if(Option.PIN == 99999999)                              // 99999999 is permanent lockdown
              MMPrintString("Console locked, press enter to restart: ");
          else
              MMPrintString("Enter PIN or 0 to restart: ");
          MMgetline(0, inpbuf);
          if(Option.PIN == 99999999) SoftReset();
          if(*inpbuf != 0) {
        	  MM_Delay(50);  ;
              i = atoi(inpbuf);
              if(i == 0) SoftReset();
              if(i == Option.PIN) {
                  IgnorePIN = true;
                  break;
              }
          }
      }
      _excep_code = 0;
      PrepareProgram(false);
      if(!ErrorInPrompt && FindSubFun("MM.PROMPT", 0) >= 0) {
          ErrorInPrompt = true;
          ExecuteProgram("MM.PROMPT\0");
      } else {
          MMPrintString("> ");                            // print the prompt
      }
      ErrorInPrompt = false;
      EditInputLine();
//	  MMPrintString(inpbuf);PRet();
      InsertLastcmd(inpbuf);                                  // save in case we want to edit it later
      //MMgetline(0, inpbuf);                                       // get the input
      if(!*inpbuf) continue;                                      // ignore an empty line
      i=0;
	  int toggle=0;
	  char *p=inpbuf;
	  skipspace(p);

	  if(*p=='*'){ //shortform RUN command so convert to a normal version
		  if((p[1]==' ')|(p[1]==0)){  //no filename provided
			   memmove(&p[4],&p[1],strlen(p)+1);
			   p[0]='R';p[1]='U';p[2]='N';p[3]=' ';
		  }else{

			  memmove(&p[4],&p[0],strlen(p)+1);
			  p[0]='R';p[1]='U';p[2]='N';p[3]='$';p[4]=34;
			  char  *q;
			  if((q=strchr(p,' ')) != 0){ //command line after the filename
			    *q=','; //chop the command at the first space character
			    memmove(&q[1],&q[0],strlen(q)+1);
			    q[0]=34;
			  } else{
			    strcat(p,"\"");
		      }
			  p[3]=' ';
		  }
		 // PRet();MMPrintString(inpbuf);PInt(nofile);PRet();
	  }

	  if(toupper(p[0])=='R' && toupper(p[1])=='U' && toupper(p[2])=='N' && (strlen(p)==3 || p[3]==' ')){
		  char *q, *s;
		  char fn[FF_MAX_LFN];
		  FRESULT fr;
		  FILINFO fno;
		  executerun=1;
	      if(!strchr(p,'\"')){
	    	  if(strlen(p)>3){
#ifndef CMD16BIT
				  p[0]=GetCommandValue("RUN");
	    		  if(p[4]!=','){
					  memmove(&p[1],&p[4],strlen(p)-4);
					  p[strlen(p)-3]=0;
	    		  } else {
					  memmove(&p[1],&p[5],strlen(p)-5);
					  p[strlen(p)-4]=0;
	    		  }
#else
	    		  CommandToken tkn = GetCommandValue("RUN");
                  p[0]=(tkn & 0x7f) + C_BASETOKEN;
	    		  p[1]=(tkn >> 7) + C_BASETOKEN; // tokens can be 14-bit
	    		  if(p[4]!=','){
					  memmove(&p[2],&p[4],strlen(p)-4);
					  p[strlen(p)-2]=0;
	    		  } else {
					  memmove(&p[2],&p[5],strlen(p)-5);
					  p[strlen(p)-3]=0;
	    		  }
#endif

	    		  strcpy(tknbuf,inpbuf);
	    	  } else {
	    		  tokenise(true);                                             // turn into executable code
	    	  }
	      } else {
#ifndef CMD16BIT
		      p[0]=GetCommandValue("RUN");
			  memmove(&p[1],&p[4],strlen(p)-4);
			  p[strlen(p)-3]=0;
#else
    		  CommandToken tkn = GetCommandValue("RUN");
              p[0]=(tkn & 0x7f) + C_BASETOKEN;
    		  p[1]=(tkn >> 7) + C_BASETOKEN; // tokens can be 14-bit
			  memmove(&p[2],&p[4],strlen(p)-4);
			  p[strlen(p)-2]=0;
			 // PRet();MMPrintString(p);PRet();

#endif
			  if((q=strchr(p,'\"')) != 0){
				  char qq[FF_MAX_LFN]={0};
				  if((s=strchr(&q[1],'\"')) != 0)*s=0;
				  else error("Syntax");
				  strcpy(fn,&q[1]);
				  strcpy(fn,q);
				  if(strchr(fn, '.') == NULL){
					  strcat(fn, ".BAS");
				  }
				  if(!InitSDCard()) return 0; //make sure the SDcard is there
				  getfullfilepath(&fn[1],qq);
				  fr = f_stat(qq, &fno); //check for the file in the current directory
				  *s='\"';
				  if(fr != FR_OK || (fno.fattrib & AM_DIR)){ // file was not in the current directory so insert the path
					  int slen=strlen(p) - (uint32_t)p + (uint32_t)&q[1];
					  memmove(&q[1+strlen((char *)Option.path)],&q[1],slen+1);
					  memcpy(&q[1],(char *)Option.path,strlen((char *)Option.path));
				  }
			  }
			  strcpy(tknbuf,inpbuf);
	      }
	  } else {
		  if(toupper(p[0])=='L' && toupper(p[1])=='S' && (strlen(p)==2 || p[2]==32)){
			  memmove(&p[10], &p[2], strlen(p)-2);
			  p[0]='L';			  p[1]='I';			  p[2]='S';			  p[3]='T';			  p[4]=' ';
			  p[5]='F';			  p[6]='I';			  p[7]='L';			  p[8]='E';			  p[9]='S';
		  }
		  i=0;
		  while(inpbuf[i]){
				if(inpbuf[i]==34){
					if(toggle==0)toggle=1;
					else toggle=0;
				}
				if(!toggle)inpbuf[i]=toupper(inpbuf[i]);
				i++;
		  }
		  if(toggle)error("Unterminated string");
		  tokenise(true); // turn into executable code
	  }
	  if(ShortScroll){
		DrawRectangle(0, maxH-gui_font_height, maxW - 1, maxH - 1, gui_bcolour);
	  }
	  if(setjmp(run) != 0) {
		  executerun=1;
	  }
	  PrepareProgram(false);
      CurrentLinePtr=0;
      ExecuteProgram(tknbuf);                                     // execute the line straight away
	  if(executerun)cleanend();
  	  if(Option.showstatus && CurrentY >= maxH-(gui_font_height*2)-1){
		  MX470PutS("\r\n",WHITE,BLACK);
		  CurrentY= maxH-(gui_font_height*2);
		  ShortScroll=Option.showstatus;
  	  }
  	  sendCRLF = 3;
      if(Option.showstatus)PrintStatus(1);
      mymemset(inpbuf,0,STRINGSIZE);
  }
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
    {
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable 
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  if(HAL_GetREVID()==0x1003)__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  else __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Configure LSE Drive Capability
  */
  __HAL_RCC_BKPRAM_CLK_ENABLE();
  HAL_PWREx_EnableBkUpReg();
  HAL_PWR_EnableBkUpAccess();

  /** Macro to configure the PLL clock source 
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON  ;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = (MAINCLOCKSPEED>>2);
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = (Overclock==0 ?  21 : 20);
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, (HAL_GetREVID()==0x1003 ? FLASH_LATENCY_2:FLASH_LATENCY_2)) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_LTDC
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RNG
                              |RCC_PERIPHCLK_SPI3|RCC_PERIPHCLK_SPI1
                              |RCC_PERIPHCLK_SPI2|RCC_PERIPHCLK_I2C2
                              |RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C4|RCC_PERIPHCLK_USB
                              |RCC_PERIPHCLK_FMC;
  PeriphClkInitStruct.PLL2.PLL2M = 1;
  PeriphClkInitStruct.PLL2.PLL2N = (HAL_GetREVID()!=0x1003 && G1Hardware==0? 120:100);
  PeriphClkInitStruct.PLL2.PLL2P = 16;
  PeriphClkInitStruct.PLL2.PLL2Q = 16;
  PeriphClkInitStruct.PLL2.PLL2R = 4;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;

  PeriphClkInitStruct.PLL3.PLL3M = 1;
  PeriphClkInitStruct.PLL3.PLL3N = 100;
  PeriphClkInitStruct.PLL3.PLL3P = 20;
  PeriphClkInitStruct.PLL3.PLL3Q = 20;
  PeriphClkInitStruct.PLL3.PLL3R = 20;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL2;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PLL2;
  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_HSI;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if(G1Hardware==1)HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
  /** Enable USB Voltage detector 
  */
  HAL_PWREx_EnableUSBVoltageDetector();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure the ADC multi-mode 
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */
  /** Common config 
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc2.Init.Resolution = ADC_RESOLUTION_16B;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */
  /** Common config 
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */
  /** DAC Initialization 
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** DAC channel OUT1 config 
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** DAC channel OUT2 config 
  */
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
/*static void MX_DMA2D_Init(void)
{

  // USER CODE BEGIN DMA2D_Init 0

  // USER CODE END DMA2D_Init 0

  // USER CODE BEGIN DMA2D_Init 1

  // USER CODE END DMA2D_Init 1
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  // USER CODE BEGIN DMA2D_Init 2

  // USER CODE END DMA2D_Init 2

}*/

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10707DBC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10707DBC;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x10707DBC;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief JPEG Initialization Function
  * @param None
  * @retval None
  */
static void MX_JPEG_Init(void)
{

  /* USER CODE BEGIN JPEG_Init 0 */

  /* USER CODE END JPEG_Init 0 */

  /* USER CODE BEGIN JPEG_Init 1 */

  /* USER CODE END JPEG_Init 1 */
  hjpeg.Instance = JPEG;
  if (HAL_JPEG_Init(&hjpeg) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN JPEG_Init 2 */

  /* USER CODE END JPEG_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AH;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AH;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 127;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 215;
  hltdc.Init.AccumulatedVBP = 26;
  hltdc.Init.AccumulatedActiveW = 1015;
  hltdc.Init.AccumulatedActiveH = 626;
  hltdc.Init.TotalWidth = 1055;
  hltdc.Init.TotalHeigh = 627;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 800;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 600;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_L8;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 255;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0x24000000;
  pLayerCfg.ImageWidth = 800;
  pLayerCfg.ImageHeight = 600;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
/* RTC init function */
static int MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
	int resetclock=0;
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

    /**Initialize RTC Only
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 7;
  hrtc.Init.SynchPrediv = 4095;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  /* USER CODE BEGIN RTC_Init 2 */
  if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
	  SystemError=1;Error_Handler();
  }

  if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
	  SystemError=1;Error_Handler();
  }
//  RtcGetTime();
  if(sDate.Year<18 || sTime.Hours>23 || sTime.Minutes>59){
  /* USER CODE END RTC_Init 2 */

    /**Initialize RTC and set the Time and Date
    */
	  if (HAL_RTC_Init(&hrtc) != HAL_OK)
	  {
		  SystemError=1;Error_Handler();
	  }
	  sTime.Hours = 0;
	  sTime.Minutes = 0;
	  sTime.Seconds = 0;
	  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	  {
		  SystemError=1;Error_Handler();
	  }
	  /* USER CODE BEGIN RTC_Init 3 */

	  /* USER CODE END RTC_Init 3 */

	  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	  sDate.Month = RTC_MONTH_JANUARY;
	  sDate.Date = 1;
	  sDate.Year = 0;

	  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	  {
		  SystemError=1;Error_Handler();
	  }
  /* USER CODE BEGIN RTC_Init 4 */
	  resetclock=1;
  }
  return resetclock;
  /* USER CODE END RTC_Init 4 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan.Instance = FDCAN1;
  hfdcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan.Init.AutoRetransmission = DISABLE;
  hfdcan.Init.TransmitPause = DISABLE;
  hfdcan.Init.ProtocolException = DISABLE;
  hfdcan.Init.NominalPrescaler = 1;
  hfdcan.Init.NominalSyncJumpWidth = 1;
  hfdcan.Init.NominalTimeSeg1 = 2;
  hfdcan.Init.NominalTimeSeg2 = 2;
  hfdcan.Init.DataPrescaler = 1;
  hfdcan.Init.DataSyncJumpWidth = 1;
  hfdcan.Init.DataTimeSeg1 = 1;
  hfdcan.Init.DataTimeSeg2 = 1;
  hfdcan.Init.MessageRAMOffset = 0;
  hfdcan.Init.StdFiltersNbr = 0;
  hfdcan.Init.ExtFiltersNbr = 0;
  hfdcan.Init.RxFifo0ElmtsNbr = 0;
  hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan.Init.RxFifo1ElmtsNbr = 0;
  hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan.Init.RxBuffersNbr = 0;
  hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan.Init.TxEventsNbr = 0;
  hfdcan.Init.TxBuffersNbr = 0;
  hfdcan.Init.TxFifoQueueElmtsNbr = 0;
  hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 0x0;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi3.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 0;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 2;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 0xFFFF;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 1249;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 0xFFFFFFFF;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */
	HAL_TIM_Base_Start_IT(&htim5);
	  WriteCoreTimer(0);
	  FastTimer=0;

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 0;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 0;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */
  HAL_TIM_MspPostInit(&htim8);

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 0;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 49999;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */
  htim16.Init.Prescaler = M40CLOCKSPEED;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  HAL_TIM_Base_Start_IT(&htim16);
  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 0;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 0;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV2;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK2;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 6;
  SdramTiming.WriteRecoveryTime = 2;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void USBearly(void){
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  GPIO_InitStruct.Pin = GPIO_PIN_12 |  GPIO_PIN_11;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
static int getVersion(void){ //check for one of the test keys being tied low
	  int j, k=1000;
	  __HAL_RCC_GPIOF_CLK_ENABLE();
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = /*GPIO_PIN_1| GPIO_PIN_3 | */GPIO_PIN_9;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	  while(k--){
		  j = HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_9);
	  }
	  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	  if(j==0)return 0;
	  else return 1;
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();


  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_LED_GPIO_Port, SD_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_CE_GPIO_Port, SD_CE_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PE2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PI8 PI3 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_LED_Pin */
  GPIO_InitStruct.Pin = SD_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : COUNT3_Pin COUNT4_Pin */
  GPIO_InitStruct.Pin = COUNT3_Pin|COUNT4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_WP_Pin */
  GPIO_InitStruct.Pin = SD_WP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SD_WP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Mouse_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_Pin */
  GPIO_InitStruct.Pin = IR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(IR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  if(G1Hardware==1){
	  GPIO_InitStruct.Pin = GPIO_PIN_8;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

  /*Configure GPIO pin : SD_CE_Pin */
  GPIO_InitStruct.Pin = SD_CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CE_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(SD_CE_GPIO_Port, SD_CE_Pin, GPIO_PIN_SET);
  /*Configure GPIO pin : SD_CD_Pin */
  GPIO_InitStruct.Pin = SD_CD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SD_CD_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void MPU_Config_nCacheable(int jpg)
{
  static int neverrun=1, lastjpg=-1;
  int dp1=0, dp2=0, dp3=0, dp4=0, dp5=0 ,dp6=0;
  static MPU_Region_InitTypeDef MPU_InitStruct0; //  1Mb of variable space if G2 H/W
  static MPU_Region_InitTypeDef MPU_InitStruct1; //  512K main video memory
  static MPU_Region_InitTypeDef MPU_InitStruct2; //  512K main video memory
  static MPU_Region_InitTypeDef MPU_InitStruct3; //  512K main video memory
  static MPU_Region_InitTypeDef MPU_InitStruct4; //  512K main video memory
  static MPU_Region_InitTypeDef MPU_InitStruct5; //  512K main video memory
  static MPU_Region_InitTypeDef MPU_InitStruct6; //  512K main video memory
  static MPU_Region_InitTypeDef MPU_InitStruct7; //  OPTION RAM memory or first 512Kb of variable space
  static MPU_Region_InitTypeDef MPU_InitStruct8; //  2nd 512Kb of variable space
  static MPU_Region_InitTypeDef MPU_InitStruct9; //  4Mb of variable space
  static MPU_Region_InitTypeDef MPU_InitStruct10;//  vartbl
  static MPU_Region_InitTypeDef MPU_InitStruct11;//  funtbl
  static MPU_Region_InitTypeDef MPU_InitStruct12;//  page 0 video memory
  static MPU_Region_InitTypeDef MPU_InitStruct13;//  8Mb of variable space if G2 H/W
  static MPU_Region_InitTypeDef MPU_InitStruct14;//  16Mb of variable space if G2 H/W
  static MPU_Region_InitTypeDef MPU_InitStruct15;//  2Mb of variable space if G2 H/W
  switch(ShareNeeded){
	  case 0xD0300000:
		  dp6=1;
	  case 0xD0280000:
		  dp5=1;
	  case 0xD0200000:
		  dp4=1;
	  case 0xD0180000:
		  dp3=1;
	  case 0xD0100000:
		  dp2=1;
	  case 0xD0080000:
		  dp1=1;
  }
  /* Disable the MPU */
  HAL_MPU_Disable();
	  /* Configure the MPU attributes for Video Memory */
  if(dp1!=MPU_InitStruct1.IsShareable || neverrun){
	  MPU_InitStruct1.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct1.BaseAddress = 0xD0000000;
	  MPU_InitStruct1.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct1.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct1.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct1.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct1.IsShareable = dp1;
	  MPU_InitStruct1.Number = MPU_REGION_NUMBER1;
	  MPU_InitStruct1.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct1.SubRegionDisable = 0x00;
	  MPU_InitStruct1.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct1);
	  dp1=MPU_InitStruct1.IsShareable;
  }
  if(dp2!=MPU_InitStruct2.IsShareable || neverrun){
	  MPU_InitStruct2.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct2.BaseAddress = 0xD0080000;
	  MPU_InitStruct2.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct2.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct2.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct2.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct2.IsShareable = dp2;
	  MPU_InitStruct2.Number = MPU_REGION_NUMBER2;
	  MPU_InitStruct2.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct2.SubRegionDisable = 0x00;
	  MPU_InitStruct2.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct2);
	  dp2=MPU_InitStruct2.IsShareable;
  }
  if(dp3!=MPU_InitStruct3.IsShareable || neverrun){
	  MPU_InitStruct3.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct3.BaseAddress = 0xD0100000;
	  MPU_InitStruct3.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct3.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct3.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct3.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct3.IsShareable = dp3;
	  MPU_InitStruct3.Number = MPU_REGION_NUMBER3;
	  MPU_InitStruct3.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct3.SubRegionDisable = 0x00;
	  MPU_InitStruct3.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct3);
	  dp3=MPU_InitStruct3.IsShareable;
  }
  if(dp4!=MPU_InitStruct4.IsShareable || neverrun){
	  MPU_InitStruct4.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct4.BaseAddress = 0xD0180000;
	  MPU_InitStruct4.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct4.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct4.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct4.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct4.IsShareable = dp4;
	  MPU_InitStruct4.Number = MPU_REGION_NUMBER4;
	  MPU_InitStruct4.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct4.SubRegionDisable = 0x00;
	  MPU_InitStruct4.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct4);
	  dp4=MPU_InitStruct4.IsShareable;
  }
  if(dp5!=MPU_InitStruct5.IsShareable || neverrun){
	  MPU_InitStruct5.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct5.BaseAddress = 0xD0200000;
	  MPU_InitStruct5.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct5.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct5.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct5.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct5.IsShareable = dp5;
	  MPU_InitStruct5.Number = MPU_REGION_NUMBER5;
	  MPU_InitStruct5.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct5.SubRegionDisable = 0x00;
	  MPU_InitStruct5.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct5);
	  dp5=MPU_InitStruct5.IsShareable;
  }
  if(dp6!=MPU_InitStruct6.IsShareable || neverrun){
	  MPU_InitStruct6.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct6.BaseAddress = 0xD0280000;
	  MPU_InitStruct6.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct6.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct6.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct6.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct6.IsShareable = dp6;
	  MPU_InitStruct6.Number = MPU_REGION_NUMBER6;
	  MPU_InitStruct6.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct6.SubRegionDisable = 0x00;
	  MPU_InitStruct6.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct6);
	  dp5=MPU_InitStruct5.IsShareable;
  }
  if(neverrun){
	  MPU_InitStruct10.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct10.BaseAddress = 0x38000000;
	  MPU_InitStruct10.Size = MPU_REGION_SIZE_64KB;
	  MPU_InitStruct10.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct10.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct10.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct10.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	  MPU_InitStruct10.Number = MPU_REGION_NUMBER10;
	  MPU_InitStruct10.TypeExtField = MPU_TEX_LEVEL1;
	  MPU_InitStruct10.SubRegionDisable = 0x00;
	  MPU_InitStruct10.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct10);

	  /* Configure the MPU attributes for Function hash memory*/
	  MPU_InitStruct11.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct11.BaseAddress = 0x30040000;
	  MPU_InitStruct11.Size = MPU_REGION_SIZE_32KB;
	  MPU_InitStruct11.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct11.IsBufferable = MPU_ACCESS_BUFFERABLE;
	  MPU_InitStruct11.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct11.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	  MPU_InitStruct11.Number = MPU_REGION_NUMBER11;
	  MPU_InitStruct11.TypeExtField = MPU_TEX_LEVEL1;
	  MPU_InitStruct11.SubRegionDisable = 0x00;
	  MPU_InitStruct11.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct11);

	  /* Configure the MPU attributes for Video Memory */
	  MPU_InitStruct12.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct12.BaseAddress = 0x24000000;
	  MPU_InitStruct12.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct12.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct12.IsBufferable = MPU_ACCESS_BUFFERABLE ;
	  MPU_InitStruct12.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct12.IsShareable = MPU_ACCESS_SHAREABLE;
	  MPU_InitStruct12.Number = MPU_REGION_NUMBER12;
	  MPU_InitStruct12.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct12.SubRegionDisable = 0x00;
	  MPU_InitStruct12.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  HAL_MPU_ConfigRegion(&MPU_InitStruct12);

  }
  if(G1Hardware==0){ //G2 hardware
	  if(neverrun){
		  /* Configure the MPU attributes for Video Memory */
		  MPU_InitStruct7.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct7.BaseAddress = 0xD0300000;
		  MPU_InitStruct7.Size = MPU_REGION_SIZE_1MB;
		  MPU_InitStruct7.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct7.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct7.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct7.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		  MPU_InitStruct7.Number = MPU_REGION_NUMBER7;
		  MPU_InitStruct7.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct7.SubRegionDisable = 0x00;
		  MPU_InitStruct7.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct7);
		  MPU_InitStruct0.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct0.BaseAddress = 0xD0400000;
		  MPU_InitStruct0.Size = MPU_REGION_SIZE_2MB;
		  MPU_InitStruct0.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct0.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct0.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct0.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		  MPU_InitStruct0.Number = MPU_REGION_NUMBER0;
		  MPU_InitStruct0.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct0.SubRegionDisable = 0x00;
		  MPU_InitStruct0.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct0);
		  MPU_InitStruct15.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct15.BaseAddress = 0xD0600000;
		  MPU_InitStruct15.Size = MPU_REGION_SIZE_1MB;
		  MPU_InitStruct15.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct15.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct15.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct15.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		  MPU_InitStruct15.Number = MPU_REGION_NUMBER15;
		  MPU_InitStruct15.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct15.SubRegionDisable = 0x00;
		  MPU_InitStruct15.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct15);
		  /* Configure the MPU attributes for Video Memory */
		  MPU_InitStruct8.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct8.BaseAddress = 0xD0700000;
		  MPU_InitStruct8.Size = MPU_REGION_SIZE_512KB;
		  MPU_InitStruct8.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct8.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct8.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct8.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		  MPU_InitStruct8.Number = MPU_REGION_NUMBER8;
		  MPU_InitStruct8.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct8.SubRegionDisable = 0x00;
		  MPU_InitStruct8.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct8);
		  /* Configure the MPU attributes for SDRAM used for program*/
		  MPU_InitStruct9.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct9.BaseAddress = 0xD0780000;
		  MPU_InitStruct9.Size = MPU_REGION_SIZE_512KB;
		  MPU_InitStruct9.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct9.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct9.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct9.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		  MPU_InitStruct9.Number = MPU_REGION_NUMBER9;
		  MPU_InitStruct9.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct9.SubRegionDisable = 0x00;
		  MPU_InitStruct9.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct9);
		  neverrun=0;
	  }
	  if(lastjpg!=jpg || neverrun){
	  /* Configure the MPU attributes for Variable SDRAM */
		  MPU_InitStruct13.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct13.BaseAddress = 0xD0800000;
		  MPU_InitStruct13.Size = MPU_REGION_SIZE_8MB;
		  MPU_InitStruct13.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct13.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct13.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct13.IsShareable = (jpg ? MPU_ACCESS_SHAREABLE : MPU_ACCESS_NOT_SHAREABLE);
		  MPU_InitStruct13.Number = MPU_REGION_NUMBER13;
		  MPU_InitStruct13.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct13.SubRegionDisable = 0x00;
		  MPU_InitStruct13.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct13);
		  /* Configure the MPU attributes for Variable SDRAM */
		  MPU_InitStruct14.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct14.BaseAddress = 0xD1000000;
		  MPU_InitStruct14.Size = MPU_REGION_SIZE_16MB;
		  MPU_InitStruct14.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct14.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct14.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct14.IsShareable = (jpg ? MPU_ACCESS_SHAREABLE : MPU_ACCESS_NOT_SHAREABLE);
		  MPU_InitStruct14.Number = MPU_REGION_NUMBER14;
		  MPU_InitStruct14.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct14.SubRegionDisable = 0x00;
		  MPU_InitStruct14.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct14);
	  }
  } else { //Original H/W
	  if(lastjpg!=jpg || neverrun){
		  MPU_InitStruct7.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct7.BaseAddress = 0xD0300000;
		  MPU_InitStruct7.Size = MPU_REGION_SIZE_512KB;
		  MPU_InitStruct7.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct7.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct7.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct7.IsShareable = (jpg ? MPU_ACCESS_SHAREABLE : MPU_ACCESS_NOT_SHAREABLE);
		  MPU_InitStruct7.Number = MPU_REGION_NUMBER7;
		  MPU_InitStruct7.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct7.SubRegionDisable = 0x00;
		  MPU_InitStruct7.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct7);

		  MPU_InitStruct8.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct8.BaseAddress = 0xD0380000;
		  MPU_InitStruct8.Size = MPU_REGION_SIZE_512KB;
		  MPU_InitStruct8.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct8.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct8.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct8.IsShareable = (jpg ? MPU_ACCESS_SHAREABLE : MPU_ACCESS_NOT_SHAREABLE);
		  MPU_InitStruct8.Number = MPU_REGION_NUMBER8;
		  MPU_InitStruct8.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct8.SubRegionDisable = 0x00;
		  MPU_InitStruct8.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct8);

		  /* Configure the MPU attributes for SDRAM */
		  MPU_InitStruct9.Enable = MPU_REGION_ENABLE;
		  MPU_InitStruct9.BaseAddress = 0xD0400000;
		  MPU_InitStruct9.Size = MPU_REGION_SIZE_4MB;
		  MPU_InitStruct9.AccessPermission = MPU_REGION_FULL_ACCESS;
		  MPU_InitStruct9.IsBufferable = MPU_ACCESS_BUFFERABLE;
		  MPU_InitStruct9.IsCacheable = MPU_ACCESS_CACHEABLE;
		  MPU_InitStruct9.IsShareable = (jpg ? MPU_ACCESS_SHAREABLE : MPU_ACCESS_NOT_SHAREABLE);
		  MPU_InitStruct9.Number = MPU_REGION_NUMBER9;
		  MPU_InitStruct9.TypeExtField = MPU_TEX_LEVEL0;
		  MPU_InitStruct9.SubRegionDisable = 0x00;
		  MPU_InitStruct9.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		  HAL_MPU_ConfigRegion(&MPU_InitStruct9);
	  }
	  if(neverrun){
		  if(Option.ProgramStartCode==-1){
		  /* Configure the MPU attributes for SDRAM */
			  MPU_InitStruct7.Enable = MPU_REGION_ENABLE;
			  MPU_InitStruct7.BaseAddress = 0xD0300000;
			  MPU_InitStruct7.Size = MPU_REGION_SIZE_512KB;
			  MPU_InitStruct7.AccessPermission = MPU_REGION_FULL_ACCESS;
			  MPU_InitStruct7.IsBufferable = MPU_ACCESS_BUFFERABLE;
			  MPU_InitStruct7.IsCacheable = MPU_ACCESS_CACHEABLE;
			  MPU_InitStruct7.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
			  MPU_InitStruct7.Number = MPU_REGION_NUMBER7;
			  MPU_InitStruct7.TypeExtField = MPU_TEX_LEVEL0;
			  MPU_InitStruct7.SubRegionDisable = 0x00;
			  MPU_InitStruct7.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
			  HAL_MPU_ConfigRegion(&MPU_InitStruct7);
		  }
		  /* Configure the MPU attributes for Variable memory*/
		  neverrun=0;
	  }
  }
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  lastjpg=jpg;

}

/* SPI5 init function */
static void MX_SD_SPI_Init(void)
{

  /* SPI5 parameter configuration*/
  SD_SPI.Instance = SPI3;
  SD_SPI.Init.Mode = SPI_MODE_MASTER;
  SD_SPI.Init.Direction = SPI_DIRECTION_2LINES;
  SD_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
  SD_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
  SD_SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
  SD_SPI.Init.NSS = SPI_NSS_SOFT;
  SD_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  SD_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SD_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
  SD_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SD_SPI.Init.CRCPolynomial = 0x0;
  SD_SPI.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  SD_SPI.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  SD_SPI.Init.FifoThreshold = SPI_FIFO_THRESHOLD_16DATA;
  SD_SPI.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  SD_SPI.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  SD_SPI.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  SD_SPI.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  SD_SPI.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  SD_SPI.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  SD_SPI.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&SD_SPI) != HAL_OK)
  {
	  Error_Handler( );
  }
}

void MX_USART1_UART_Init1(void){
	MX_USART1_UART_Init();
}

void MX_USART2_UART_Init2(void){
	MX_USART2_UART_Init();
}
void MX_UART4_Init4(void){
	MX_UART4_Init();
}
void MX_TIM2_Init1(void){
	MX_TIM2_Init();
}

void CheckAbort(void) {
	routinechecks(1);
	if(deferredcopy && pagesetdone){
		int cursorhidden=0;
		deferredcopy=0;
    	if(cursoron && (deferredtadd==(VideoColour==12 ? 1 : 0) || deferredfadd==(VideoColour==12 ? 1 : 0))){
    	    uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
    	    ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
    		hidecursor(0);
    		cursorhidden=1;
    	    ReadPage=readsave;
    	    WritePage=writesave;
    	}
		PageCopy(deferredfadd, deferredtadd, deferredtransparent);
		if(cursorhidden){
			uint8_t readsave=ReadPage, writesave=WritePage; //save the current read and write page
			ReadPage = WritePage = (VideoColour==12 ? 1 : 0);
			showcursor(0, xcursor, ycursor);
			ReadPage=readsave;
			WritePage=writesave;
		}
	}
    if(MMAbort) {
        WDTimer = 0;                                                // turn off the watchdog timer
        ShowCursor(0);
    	ScrewUpTimer=0;
    	cleanend();
    }
}
void routinechecks(int all){
	if(USBtime){
		mT4IntEnable(0);
		CheckKeyboard();
		if(all){
			CheckSDCard();
			if(Option.MaxCtrls)ProcessTouch();
			if(GPSchannel)processgps();
		}
		USBtime=0;
		mT4IntEnable(1);
	}
}
void PrintStatus(int now){
	char *c;
	static int lastsecond=-1, len;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	int lastx=CurrentX,lasty=CurrentY;
	RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
	if(second==lastsecond && !now)return;
	lastsecond=second;
	char buff[STRINGSIZE];
	mymemset(buff, ' ',STRINGSIZE);
	buff[255]=0;
	len=maxW/gui_font_width;
	OptionFileErrorAbort = 0;
	int sdok=InitSDCard(); //returns 2 if it has done an initialise
	pagesetdone=0;
	while(!pagesetdone){};
	int cursorhidden=0;
	if(cursoron && ycursor > maxH - gui_font_height- hcursor){
	   hidecursor(0);
	   cursorhidden=1;
	}
	GUIPrintString(0, maxH-gui_font_height, gui_font, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, BROWN, gui_bcolour, &buff[255-len]);
	c=(char *)ProgMemory;
	if(*c++ == 1 && *c++ == 39){
	    FRESULT fr=0;
	    FILINFO fno;
	    mymemset(&fno,0,sizeof(FILINFO));
		c++;
		strcpy(buff,"A:/");
		strcat(buff,c);
		if(docheck>=2 && sdok)strcat(buff," - Check Disk");
		if(!sdok){
			docheck=1;
			strcat(buff," - Check Disk");
		}
		else if(docheck==1){ //new disk so check my file is on it
			c--;
		    fr = f_stat(c, &fno);
		    if(fr != FR_OK || (fno.fattrib & AM_DIR)){
		    	docheck++;
		    	strcat(buff," - Wrong Disk");
		    }
		    else docheck=0;
		}
	} else if(!sdok)strcpy(buff,"Check Disk");


	GUIPrintString(maxW/2, maxH-gui_font_height, gui_font, JUSTIFY_MIDDLE, JUSTIFY_TOP, ORIENT_NORMAL, BROWN, gui_bcolour, buff);

	GUIPrintString(0, maxH-gui_font_height, gui_font, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, BROWN, gui_bcolour, filepath);

	myset(buff,0,256);
	c=buff;
	IntToStrPad(c, hour, '0', 2, 10);
	c[2] = ':'; IntToStrPad(c + 3, minute, '0', 2, 10);
	c[5] = ':'; IntToStrPad(c + 6, second, '0', 2, 10);
	c[8] = ' ';
	IntToStrPad(c+9, day, '0', 2, 10);
	c[11] = '-'; IntToStrPad(c + 12, month, '0', 2, 10);
	c[14] = '-'; IntToStr(c + 15, year, 10);
	c[19]=0;
	GUIPrintString(maxW-1, maxH-gui_font_height, gui_font, JUSTIFY_RIGHT, JUSTIFY_TOP, ORIENT_NORMAL, BROWN, gui_bcolour, buff);
    if(cursorhidden)showcursor(0, xcursor,ycursor);
	CurrentY=lasty;
	CurrentX=lastx;
	OptionFileErrorAbort = true;

}	// send a character to the Console serial port
void SerialConsolePutC(int c) {
	if(!(OptionConsole & 1))return;
	int empty;
	if(Option.ConsolePort==3){
		empty=(huart1.Instance->ICR & USART_ICR_TCCF) | !(huart1.Instance->CR1 & USART_CR1_TCIE) ;
		while(ConsoleTxBufTail == ((ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE))routinechecks(1); //wait if buffer full
		ConsoleTxBuf[ConsoleTxBufHead] = c;							// add the char
		ConsoleTxBufHead = (ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE;		   // advance the head of the queue
		if(empty){
			huart1.Instance->CR1 |= USART_CR1_TCIE;
		}
	} else if(Option.ConsolePort==1){
		empty=(huart2.Instance->ICR & USART_ICR_TCCF) | !(huart2.Instance->CR1 & USART_CR1_TCIE) ;
		while(ConsoleTxBufTail == ((ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE))routinechecks(1); //wait if buffer full
		ConsoleTxBuf[ConsoleTxBufHead] = c;							// add the char
		ConsoleTxBufHead = (ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE;		   // advance the head of the queue
		if(empty){
			huart2.Instance->CR1 |= USART_CR1_TCIE;
		}
	} else if(Option.ConsolePort==2){
		empty=(huart4.Instance->ICR & USART_ICR_TCCF) | !(huart4.Instance->CR1 & USART_CR1_TCIE) ;
		while(ConsoleTxBufTail == ((ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE))routinechecks(1); //wait if buffer full
		ConsoleTxBuf[ConsoleTxBufHead] = c;							// add the char
		ConsoleTxBufHead = (ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE;		   // advance the head of the queue
		if(empty){
			huart4.Instance->CR1 |= USART_CR1_TCIE;
		}
	}
}

	void putConsole(int c) {
	    DisplayPutC(c);
	    SerialConsolePutC(c);
	}

	PUTCHAR_PROTOTYPE
	{
	  /* Place your implementation of fputc here */
	  /* e.g. write a character to the USART1 and Loop until the end of transmission */
	  putConsole(ch);

	  return ch;
	}
	void initConsole(void) {
	    ConsoleRxBufHead = ConsoleRxBufTail = 0;
	    ConsoleTxBufHead = ConsoleTxBufTail = 0;
	}
	// get a char from the UART1 serial port (the console)
	// will return immediately with -1 if there is no character waiting
	int getConsole(void) {
	    int c=-1;
	    CheckAbort();
		if(Option.ConsolePort==3){
			huart1.Instance->CR1 &= ~USART_CR1_RXNEIE;
			if(ConsoleRxBufHead != ConsoleRxBufTail) {                            // if the queue has something in it
				c = ConsoleRxBuf[ConsoleRxBufTail];
				ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;   // advance the head of the queue
			}
			huart1.Instance->CR1 |= USART_CR1_RXNEIE;
		} else if(Option.ConsolePort==1){
			huart2.Instance->CR1 &= ~USART_CR1_RXNEIE;
			if(ConsoleRxBufHead != ConsoleRxBufTail) {                            // if the queue has something in it
				c = ConsoleRxBuf[ConsoleRxBufTail];
				ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;   // advance the head of the queue
			}
			huart2.Instance->CR1 |= USART_CR1_RXNEIE;
		} else if(Option.ConsolePort==2){
			huart4.Instance->CR1 &= ~USART_CR1_RXNEIE;
			if(ConsoleRxBufHead != ConsoleRxBufTail) {                            // if the queue has something in it
				c = ConsoleRxBuf[ConsoleRxBufTail];
				ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;   // advance the head of the queue
			}
			huart4.Instance->CR1 |= USART_CR1_RXNEIE;
		}
	    return c;
	}
	GETCHAR_PROTOTYPE
	{
	  /* Place your implementation of fputc here */
	  /* e.g. write a character to the USART1 and Loop until the end of transmission */

	  return getConsole();
	}
	int kbhitConsole(void) {
	    int i;
	    i = ConsoleRxBufHead - ConsoleRxBufTail;
	    if(i < 0) i += CONSOLE_RX_BUF_SIZE;
	    return i;
	}
	// print a char on the Serial and USB consoles only (used in the EDIT command and dp() macro)
	void SerUSBPutC(char c) {
	    SerialConsolePutC(c);
	}
	// print a string on the Serial and USB consoles only (used in the EDIT command and dp() macro)
	void SerUSBPutS(char *s) {
	    while(*s) SerUSBPutC(*s++);
	}

	/*****************************************************************************************
	The vt100 escape code sequences
	===============================
	3 char codes            Arrow Up    esc [ A
	                        Arrow Down  esc [ B
	                        Arrow Right esc [ C
	                        Arrow Left  esc [ D

	4 char codes            Home        esc [ 1 ~
	                        Insert      esc [ 2 ~
	                        Del         esc [ 3 ~
	                        End         esc [ 4 ~
	                        Page Up     esc [ 5 ~
	                        Page Down   esc [ 6 ~

	5 char codes            F1          esc [ 1 1 ~
	                        F2          esc [ 1 2 ~
	                        F3          esc [ 1 3 ~
	                        F4          esc [ 1 4 ~
	                        F5          esc [ 1 5 ~         note the
	                        F6          esc [ 1 7 ~         disconnect
	                        F7          esc [ 1 8 ~
	                        F8          esc [ 1 9 ~
	                        F9          esc [ 2 0 ~
	                        F10         esc [ 2 1 ~         note the
	                        F11         esc [ 2 3 ~         disconnect
	                        F12         esc [ 2 4 ~

	                        SHIFT-F3    esc [ 2 5 ~         used in the editor
	                        SHIFT-F4    esc [ 2 6 ~
	                        SHIFT-F5    esc [ 2 8 ~
	                        SHIFT-F6    esc [ 2 9 ~
	                        SHIFT-F7    esc [ 3 1 ~
	                        SHIFT-F8    esc [ 3 2 ~

	*****************************************************************************************/

	// check if there is a keystroke waiting in the buffer and, if so, return with the char
	// returns -1 if no char waiting
	// the main work is to check for vt100 escape code sequences and map to Maximite codes
	// SHIFT F4-F12 Added as per piciomite 6.00.02B0
	//NB: SHIFT F1, F2, F11, and F12 don't appear to generate anything

 	int MMInkey(void) {
	    unsigned int c = -1;                                            // default no character
	    unsigned int tc = -1;                                           // default no character
	    unsigned int ttc = -1;                                          // default no character
	    static unsigned int c1 = -1;
	    static unsigned int c2 = -1;
	    static unsigned int c3 = -1;
	    static unsigned int c4 = -1;
	    static int crseen = 0;

	    if(c1 != -1) {                                                  // check if there are discarded chars from a previous sequence
	        c = c1; c1 = c2; c2 = c3; c3 = c4; c4 = -1;                 // shuffle the queue down
	        return c;                                                   // and return the head of the queue
	    }

	    c = getConsole();                                               // do discarded chars so get the char
	    if(c == '\r'){
	    	crseen = 1;
	    	InkeyTimer = 0;
	    }
	    if(c==-1 && InkeyTimer>2 && crseen){
	    	c='\n';
	    }
	    if(!(c==-1 || c=='\r'))crseen=0;
	    if(c == 0x1b) {
	        InkeyTimer = 0;                                             // start the timer
	        while((c = getConsole()) == -1 && InkeyTimer < 30);         // get the second char with a delay of 30mS to allow the next char to arrive
	        if(c == 'O'){   //support for many linux terminal emulators
	            while((c = getConsole()) == -1 && InkeyTimer < 50);        // delay some more to allow the final chars to arrive, even at 1200 baud
	            if(c == 'P') return F1;
	            if(c == 'Q') return F2;
	            if(c == 'R') return F3;
	            if(c == 'S') return F4;
	            if(c == 'T') return F5;
	            if(c == '2'){
	                while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
	                if(tc == 'R') return F3 + 0x20;
	                c1 = 'O'; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
	            }
	            c1 = 'O'; c2 = c; return 0x1b;                 // not a valid 4 char code
	        }
	        if(c != '[') { c1 = c; return 0x1b; }                       // must be a square bracket
	        while((c = getConsole()) == -1 && InkeyTimer < 50);         // get the third char with delay
	        if(c == 'A') return UP;                                     // the arrow keys are three chars
	        if(c == 'B') return DOWN;
	        if(c == 'C') return RIGHT;
	        if(c == 'D') return LEFT;
	        if(c < '1' && c > '6') { c1 = '['; c2 = c; return 0x1b; }   // the 3rd char must be in this range
	        while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
	        if(tc == '~') {                                             // all 4 char codes must be terminated with ~
	            if(c == '1') return HOME;
	            if(c == '2') return INSERT;
	            if(c == '3') return DEL;
	            if(c == '4') return END;
	            if(c == '5') return PUP;
	            if(c == '6') return PDOWN;
	            c1 = '['; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
	        }
	        while((ttc = getConsole()) == -1 && InkeyTimer < 90);       // get the 5th char with delay
	        if(ttc == '~') {                                            // must be a ~
	            if(c == '1') {
	                if(tc >='1' && tc <= '5') return F1 + (tc - '1');   // F1 to F5
	                if(tc >='7' && tc <= '9') return F6 + (tc - '7');   // F6 to F8
	            }
	            if(c == '2') {
	                if(tc =='0' || tc == '1') return F9 + (tc - '0');   // F9 and F10
	                if(tc =='3' || tc == '4') return F11 + (tc - '3');  // F11 and F12
	                //if(tc =='5') return F3 + 0x20;                      // SHIFT-F3
                    if(tc =='5' || tc=='6') return F3 + 0x20 + tc-'5';   // SHIFT-F3 and F4
                    if(tc =='8' || tc=='9') return F5 + 0x20 + tc-'8';   // SHIFT-F5 and F6
	            }
                    if(c == '3') {
                    if(tc >='1' && tc <= '4') return F7 + 0x20 + (tc - '1');   // SHIFT-F7 to F10
                }
                //NB: SHIFT F1, F2,F9,F10, F11 and F12 don't appear to generate anything
            }
	        // nothing worked so bomb out
	        c1 = '['; c2 = c; c3 = tc; c4 = ttc;
	        return 0x1b;
	    }
	    return c;
	}

/*
	int MMInkey(void) {
	    unsigned int c = -1;                                            // default no character
	    unsigned int tc = -1;                                           // default no character
	    unsigned int ttc = -1;                                          // default no character
	    static unsigned int c1 = -1;
	    static unsigned int c2 = -1;
	    static unsigned int c3 = -1;
	    static unsigned int c4 = -1;
	    static int crseen = 0;

	    if(c1 != -1) {                                                  // check if there are discarded chars from a previous sequence
	        c = c1; c1 = c2; c2 = c3; c3 = c4; c4 = -1;                 // shuffle the queue down
	        return c;                                                   // and return the head of the queue
	    }

	    c = getConsole();                                               // do discarded chars so get the char
	    if(c == '\r'){
	    	crseen = 1;
	    	InkeyTimer = 0;
	    }
	    if(c==-1 && InkeyTimer>2 && crseen){
	    	c='\n';
	    }
	    if(!(c==-1 || c=='\r'))crseen=0;
	    if(c == 0x1b) {
	        InkeyTimer = 0;                                             // start the timer
	        while((c = getConsole()) == -1 && InkeyTimer < 30);         // get the second char with a delay of 30mS to allow the next char to arrive
	        if(c == 'O'){   //support for many linux terminal emulators
	            while((c = getConsole()) == -1 && InkeyTimer < 50);        // delay some more to allow the final chars to arrive, even at 1200 baud
	            if(c == 'P') return F1;
	            if(c == 'Q') return F2;
	            if(c == 'R') return F3;
	            if(c == 'S') return F4;
	            if(c == 'T') return F5;
	            if(c == '2'){
	                while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
	                if(tc == 'R') return F3 + 0x20;
	                c1 = 'O'; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
	            }
	            c1 = 'O'; c2 = c; return 0x1b;                 // not a valid 4 char code
	        }
	        if(c != '[') { c1 = c; return 0x1b; }                       // must be a square bracket
	        while((c = getConsole()) == -1 && InkeyTimer < 50);         // get the third char with delay
	        if(c == 'A') return UP;                                     // the arrow keys are three chars
	        if(c == 'B') return DOWN;
	        if(c == 'C') return RIGHT;
	        if(c == 'D') return LEFT;
	        if(c < '1' && c > '6') { c1 = '['; c2 = c; return 0x1b; }   // the 3rd char must be in this range
	        while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
	        if(tc == '~') {                                             // all 4 char codes must be terminated with ~
	            if(c == '1') return HOME;
	            if(c == '2') return INSERT;
	            if(c == '3') return DEL;
	            if(c == '4') return END;
	            if(c == '5') return PUP;
	            if(c == '6') return PDOWN;
	            c1 = '['; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
	        }
	        while((ttc = getConsole()) == -1 && InkeyTimer < 90);       // get the 5th char with delay
	        if(ttc == '~') {                                            // must be a ~
	            if(c == '1') {
	                if(tc >='1' && tc <= '5') return F1 + (tc - '1');   // F1 to F5
	                if(tc >='7' && tc <= '9') return F6 + (tc - '7');   // F6 to F8
	            }
	            if(c == '2') {
	                if(tc =='0' || tc == '1') return F9 + (tc - '0');   // F9 and F10
	                if(tc =='3' || tc == '4') return F11 + (tc - '3');  // F11 and F12
	                if(tc =='5') return F3 + 0x20;                      // SHIFT-F3
	            }
	        }
	        // nothing worked so bomb out
	        c1 = '['; c2 = c; c3 = tc; c4 = ttc;
	        return 0x1b;
	    }
	    return c;
	}

*/
	// get a keystroke from the console.  Will wait forever for input
		// if the char is a cr then replace it with a newline (lf)
	int MMgetchar(int update) {
		    int c;
		    static char prevchar = 0;
	//	    static char sendesc = 0;
		    loopback:
		    do {
//		       ProcessTouch();
		       ShowCursor(1);
		       c = MMInkey();
		       if(update && Option.showstatus)PrintStatus(0);
		    } while(c == -1);
		    if(sleeping){
				GPIO_InitTypeDef GPIO_InitStruct = {0};
			    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
			    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			    GPIO_InitStruct.Pull = GPIO_NOPULL;
			    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
			    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
			    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
		    	sleeping=0;
		    }
	    	if(Option.sleep)SleepTimer=Option.sleep*1000*60;
	//	    if(sendesc==0){
	//	    	sendesc=1;
	//	    	SerUSBPutS("\033[20h"); //tell the terminal to send CRLF
	//	    }
		    ShowCursor(0);
		    if(c == '\n' && prevchar == '\r') {
		        prevchar = 0;
		        goto loopback;
		    }
		    prevchar = c;
		    if(c == '\n') c = '\r';
		    return c;
		}
		// put a character out to the serial console
		char MMputchar(char c) {
		    putConsole(c);
		    if(IsPrint(c)) MMCharPos++;
		    if(c == '\r') {
		        MMCharPos = 1;
		    }
		    return c;
		}
#define SetupTime 22
//void uSec(unsigned int us) {
//  us=us*ticks_per_microsecond-SetupTime;
//  unsigned int start= _HAL_TIM_GET_COUNTER(&htim5);
//  while (ReadCoreTimer()<us);
//}
#define tim5_tick() (int16_t)(TIM16->CNT)
void uSec(unsigned int ucnt)
{
	uint16_t start = TIM16->CNT;
//	ucnt=ucnt*ticks_per_microsecond-SetupTime;
	while((uint16_t)(tim5_tick() - start) < ucnt){}

}
void shortpause(unsigned int ticks){
	  WriteCoreTimer(0);
	  while (ReadCoreTimer()<ticks);
}
void MM_Delay(int n){
	while(n--){
		uSec(1000);
    	routinechecks(1);
	}
}
void SoftReset(void){
	NVIC_SystemReset();
}

// takes a pointer to RAM containing a program (in clear text) and writes it to program flash or Program RAM in tokenised format
// If  Option.RAM is set then it clears and then writes to the Progmemory in SDRAM.
// If OPTION FLASH is set writes to the Program FLASH.
void SaveProgramToFlash(char *pm, int msg, char *fname, int size) {
    char *p, fontnbr, prevchar = 0, buf[STRINGSIZE];
    CommandToken endtoken;
    //if(Option.ProgramStartCode>=0){MMPrintString("Program Changed - Writing to Flash");PRet();}
    int nbr, i, n, SaveSizeAddr;
    uint32_t storedupdates[MAXCFUNCTION], updatecount=0, realflashsave;
	SCB_CleanInvalidateDCache();
	clearrepeat();
    mycpy(buf, tknbuf, STRINGSIZE);                                // save the token buffer because we are going to use it
	i=3;
    while(FlashWriteInit((uint32_t)ProgMemory) && i)i--;                     // erase program memory
    if(i==0)error("Failed to erase flash memory");
    if(size>384*1024){
    	if((uint32_t)ProgMemory+0x60000> FLASH_SAVED_OPTION_ADDR)error("OPTION FLASH must be in range % to 3 for this program",PROGRAMSTARTCODE); //Should be 0 to 3
    	i=3;
        while(FlashWriteInit((uint32_t)ProgMemory+0x60000) && i)i--;                     // erase program memory
        if(i==0)error("Failed to erase flash memory");
    }
    if(size>256*1024){
    	if((uint32_t)ProgMemory+0x40000> FLASH_SAVED_OPTION_ADDR)error("OPTION FLASH must be in range % to 4 for this program",PROGRAMSTARTCODE); //should be 0 to 4
    	i=3;
        while(FlashWriteInit((uint32_t)ProgMemory+0x40000) && i)i--;                     // erase program memory
        if(i==0)error("Failed to erase flash memory");
    }
    if(size>128*1024){
    	if((uint32_t)ProgMemory+0x20000> FLASH_SAVED_OPTION_ADDR)error("OPTION FLASH must be in range % to 5 for this program",PROGRAMSTARTCODE);// should be 0 to 5
    	i=3;
        while(FlashWriteInit((uint32_t)ProgMemory+0x20000) && i)i--;                     // erase program memory
        if(i==0)error("Failed to erase flash memory");
    }
    nbr = 0;
    // this is used to count the number of bytes written to flash
    inpbuf[0]=39;
    memcpy(&inpbuf[1],fname,strlen(fname)+1);
    tokenise(false);                                            // turn into executable code
    p = tknbuf;
    while(!(p[0] == 0 && p[1] == 0)) {
        FlashWriteByte(*p++); nbr++;

        if((realflashpointer - (uint32_t)ProgMemory) >= PROG_FLASH_SIZE - 5)
            goto exiterror1;
    }
    FlashWriteByte(0); nbr++;                              // terminate that line in flash
	routinechecks(1);
    while(*pm) {
        p = inpbuf;
        while(!(*pm == 0 || *pm == '\r' || (*pm == '\n' && prevchar != '\r'))) {
            if(*pm == TAB) {
                do {*p++ = ' ';
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                } while((p - inpbuf) % 2);
            } else {
                if(IsPrint(*pm)) {
                    *p++ = *pm;
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                }
            }
            prevchar = *pm++;
        }
        if(*pm) prevchar = *pm++;                                   // step over the end of line char but not the terminating zero
        *p = 0;                                                     // terminate the string in inpbuf

        if(*inpbuf == 0 && (*pm == 0 || (!IsPrint(*pm) && pm[1] == 0))) break; // don't save a trailing newline

        if(toupper(inpbuf[0])=='R' && toupper(inpbuf[1])=='U' && toupper(inpbuf[2])=='N' && (strlen(inpbuf)==3 || inpbuf[3]==' ')){
#ifndef CMD16BIT
			inpbuf[0]=GetCommandValue("RUN");
			memmove(&inpbuf[1],&inpbuf[4],strlen(inpbuf)-4);
			inpbuf[strlen(inpbuf)-3]=0;
#else
			CommandToken tkn = GetCommandValue("RUN");
	    	inpbuf[0]=(tkn & 0x7f) + C_BASETOKEN;
			inpbuf[1]=(tkn >> 7) + C_BASETOKEN; // tokens can be 14-bit
			memmove(&inpbuf[2],&inpbuf[4],strlen(inpbuf)-4);
			inpbuf[strlen(inpbuf)-2]=0;
#endif
 			strcpy(tknbuf,inpbuf);
        } else tokenise(false);                                            // turn into executable code
        p = tknbuf;
        while(!(p[0] == 0 && p[1] == 0)) {
            FlashWriteByte(*p++); nbr++;

            if((realflashpointer - (uint32_t)ProgMemory) >= PROG_FLASH_SIZE - 5)
                goto exiterror1;
        }
        FlashWriteByte(0); nbr++;                              // terminate that line in flash
    	routinechecks(1);
    }
    FlashWriteByte(0);
    FlashWriteAlign();                                            // this will flush the buffer and step the flash write pointer to the next word boundary
    // now we must scan the program looking for CFUNCTION/CSUB/DEFINEFONT statements, extract their data and program it into the flash used by  CFUNCTIONs
     // programs are terminated with two zero bytes and one or more bytes of 0xff.  The CFunction area starts immediately after that.
     // the format of a CFunction/CSub/Font in flash is:
     //   Unsigned Int - Address of the CFunction/CSub in program memory (points to the token representing the "CFunction" keyword) or NULL if it is a font
     //   Unsigned Int - The length of the CFunction/CSub/Font in bytes including the Offset (see below)
     //   Unsigned Int - The Offset (in words) to the main() function (ie, the entry point to the CFunction/CSub).  Omitted in a font.
     //   word1..wordN - The CFunction/CSub/Font code
     // The next CFunction/CSub/Font starts immediately following the last word of the previous CFunction/CSub/Font

    int firsthex=1;
    realflashsave= realflashpointer;
    p = (char *)ProgMemory;                                              // start scanning program memory
   // while(*p != 0xff) {
   	while(!(p[0] == 0xff && p[1] == 0xff)){
    	nbr++;
    	if((nbr % 256)==0){
        	routinechecks(1);
    	}
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) break;                                          // end of the program
        if(*p == T_NEWLINE) {
            CurrentLinePtr = p;
            p++;                                                    // skip the newline token
        }
        if(*p == T_LINENBR) p += 3;                                 // step over the line number

        skipspace(p);
        if(*p == T_LABEL) {
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
       // CommandToken tkn = GetCommandValue(p);
        CommandToken tkn = commandtbl_decode(p);
        if(tkn == cmdCSUB || tkn == cmdCFUN || tkn == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
       // if(*p == cmdCSUB || *p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
            if(tkn==GetCommandValue("DefineFont")) {
        	   endtoken = GetCommandValue("End DefineFont");
               p += sizeof(CommandToken);
              skipspace(p);
              if(*p == '#') p++;
              fontnbr = getint(p, 1, FONT_TABLE_SIZE);
              // font 6 has some special characters, some of which depend on font 1
              if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) error("Cannot redefine fonts 1, 6 or 7");
              realflashpointer+=4;
              skipelement(p);                                     // go to the end of the command
              p--;
            } else if(tkn == cmdCFUN) {
                  endtoken = GetCommandValue("End CFunction");
                  fontnbr = 0;
                  firsthex=0;
#ifdef CMD16BIT
                  p++;
#endif

            } else {
            	//MMPrintString("Found CSUB \r\n");
                endtoken = GetCommandValue("End CSub");
                realflashpointer+=4;
                fontnbr = 0;
                firsthex=0;
#ifdef CMD16BIT
                p++;
#endif
            }
             SaveSizeAddr = realflashpointer;                                // save where we are so that we can write the CFun size in here
             realflashpointer+=4;
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!isnamestart(*p))  error("Function name");
                 do { p++; } while(isnamechar(*p));
                 skipspace(p);
                 if(!(IsxDigit(p[0]) && IsxDigit(p[1]) && IsxDigit(p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit(*p)) error("Invalid hex word");
                         if((int)((char *)realflashpointer - ProgMemory) >= PROG_FLASH_SIZE - 5) error("Not enough memory");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }
                     realflashpointer+=4;
                     skipspace(p);
                     if(firsthex){
                    	 firsthex=0;
                    	 if(((n>>16) & 0xff) < 0x20)error("Can't define non-printing characters");
                     }
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0) error("Missing END declaration");
                 if(*p == T_NEWLINE) {
                     CurrentLinePtr = p;
                     p++;                                            // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                         // skip over the line number
                 skipspace(p);

            // } while(*p != endtoken);
                 //tkn = GetCommandValue(p);
                 tkn = commandtbl_decode(p);

             } while(tkn != endtoken);
             storedupdates[updatecount++]=realflashpointer - SaveSizeAddr - 4;
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }
    realflashpointer = realflashsave ;
    updatecount=0;
    p = (char *)ProgMemory;                                              // start scanning program memory
    // while(*p != 0xff) {
   	 while(!(p[0] == 0xff && p[1] == 0xff)){	                        // guard against FF in Token.
     	nbr++;
     	if((nbr % 256)==0){
        	routinechecks(1);
     	}
         if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
         if(*p == 0) break;                                          // end of the program
         if(*p == T_NEWLINE) {
             CurrentLinePtr = p;
             p++;                                                    // skip the newline token
         }
         if(*p == T_LINENBR) p += 3;                                 // step over the line number

         skipspace(p);
         if(*p == T_LABEL) {
             p += p[1] + 2;                                          // skip over the label
             skipspace(p);                                           // and any following spaces
         }
         CommandToken tkn = commandtbl_decode(p);
         if(tkn == cmdCSUB || tkn == cmdCFUN || tkn == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
        	//MMPrintString("Found CSUB or DEFINEFONT 2\r\n");
            if(tkn == GetCommandValue("DefineFont")){
               endtoken = GetCommandValue("End DefineFont");

             p += sizeof(CommandToken);
             skipspace(p);
             if(*p == '#') p++;
             fontnbr = getint(p, 1, FONT_TABLE_SIZE);
                                                 // font 6 has some special characters, some of which depend on font 1
             if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) error("Cannot redefine fonts 1, 6, or 7");

             FlashWriteWord(fontnbr - 1);             // a low number (< FONT_TABLE_SIZE) marks the entry as a font
             skipelement(p);                                     // go to the end of the command
             p--;
            } else if(tkn == cmdCFUN) {
              endtoken = GetCommandValue("End CFunction");
              FlashWriteWord((unsigned int)p);               // if a CFunction/CSub save a pointer to the declaration
              fontnbr = 0;
#ifdef CMD16BIT
              p++;
#endif
            } else {
             endtoken = GetCommandValue("End CSub");
             FlashWriteWord((unsigned int)p);               // if a CFunction/CSub save a pointer to the declaration
             fontnbr = 0;
#ifdef CMD16BIT
             p++;
#endif
         }
            SaveSizeAddr = realflashpointer;                                // save where we are so that we can write the CFun size in here
             FlashWriteWord(storedupdates[updatecount++]);                        // leave this blank so that we can later do the write
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!isnamestart(*p))  error("Function name");
                 do { p++; } while(isnamechar(*p));
                 skipspace(p);
                 if(!(IsxDigit(p[0]) && IsxDigit(p[1]) && IsxDigit(p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit(*p)) error("Invalid hex word");
                         if((int)((char *)realflashpointer - ProgMemory) >= PROG_FLASH_SIZE - 5) error("Not enough memory");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }

                     FlashWriteWord(n);
                     skipspace(p);
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0) error("Missing END declaration");
                 if(*p == T_NEWLINE) {
                    CurrentLinePtr = p;
                    p++;                                        // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 skipspace(p);
             //    tkn=p[0] & 0x7f;
             //    tkn |= ((unsigned short)(p[1] & 0x7f)<<7);
            //   } while(tkn != endtoken);
                 //tkn = GetCommandValue(p);
                  tkn = commandtbl_decode(p);
             } while(tkn != endtoken);
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }
     FlashWriteWord(0xffffffff);                                // make sure that the end of the CFunctions is terminated with an erased word
     FlashWriteClose();                                              // this will flush the buffer and step the flash write pointer to the next word boundary

    if(msg) {                                                       // if requested by the caller, print an informative message
        if(MMCharPos > 1) MMPrintString("\r\n");                    // message should be on a new line
        MMPrintString("Saved ");
        IntToStr(tknbuf, nbr + 3, 10);
        MMPrintString(tknbuf);
        MMPrintString(" bytes\r\n");
    }
    memcpy(tknbuf, buf, STRINGSIZE);                                // restore the token buffer in case there are other commands in it
    initConsole();
	clearrepeat();
//    SCB_EnableICache() ;
//    SCB_EnableDCache() ;
    return;

    // we only get here in an error situation while writing the program to flash
    exiterror1:
        FlashWriteByte(0); FlashWriteByte(0); FlashWriteByte(0);    // terminate the program in flash
        FlashWriteClose();
        error("Not enough memory");
}

// takes a pointer to RAM containing a program (in clear text) and writes it to memory in tokenised format
// It is used to compared against the program already in Flash to see if we need to rewrite the Flash.
// It also returns the total program size including Font,CSub and CFunction binary copies so we know how
// much flash need to be erased in preparation for saving the program.
uint32_t SaveProgramToMemory(char *pm, int msg, char *fname) {
    char *p, prevchar = 0, buf[STRINGSIZE];
    CommandToken endtoken;
  //  int nbr;
  //  uint32_t  retvalue;

    uint32_t storedupdates[MAXCFUNCTION], updatecount=0, realmemsave, retvalue;
    int nbr, i, n, SaveSizeAddr;
    char  fontnbr;

	SCB_CleanInvalidateDCache();
	clearrepeat();
    mycpy(buf, tknbuf, STRINGSIZE);                                // save the token buffer because we are going to use it
    SDMemory=GetMemory(512*1024);
    realmempointer=(uint32_t)SDMemory;
    mymemset(SDMemory,0xFF,512*1024);
    nbr = 0;
    // this is used to count the number of bytes written to flash
    inpbuf[0]=39;
    memcpy(&inpbuf[1],fname,strlen(fname)+1);
    tokenise(false);                                            // turn into executable code
    p = tknbuf;
    while(!(p[0] == 0 && p[1] == 0)) {
        MemWriteByte(*p++); nbr++;

        if((realmempointer - (uint32_t)SDMemory) >= PROG_FLASH_SIZE - 5)
            goto exiterror1;
    }
    MemWriteByte(0); nbr++;                              // terminate that line in flash
	routinechecks(1);
    while(*pm) {
        p = inpbuf;
        while(!(*pm == 0 || *pm == '\r' || (*pm == '\n' && prevchar != '\r'))) {
            if(*pm == TAB) {
                do {*p++ = ' ';
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                } while((p - inpbuf) % 2);
            } else {
                if(IsPrint(*pm)) {
                    *p++ = *pm;
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                }
            }
            prevchar = *pm++;
        }
        if(*pm) prevchar = *pm++;                                   // step over the end of line char but not the terminating zero
        *p = 0;                                                     // terminate the string in inpbuf

        if(*inpbuf == 0 && (*pm == 0 || (!IsPrint(*pm) && pm[1] == 0))) break; // don't save a trailing newline
        if(toupper(inpbuf[0])=='R' && toupper(inpbuf[1])=='U' && toupper(inpbuf[2])=='N' && (strlen(inpbuf)==3 || inpbuf[3]==' ')){
#ifndef CMD16BIT
			inpbuf[0]=GetCommandValue("RUN");
			memmove(&inpbuf[1],&inpbuf[4],strlen(inpbuf)-4);
			inpbuf[strlen(inpbuf)-3]=0;
#else
			CommandToken tkn = GetCommandValue("RUN");
	    	inpbuf[0]=(tkn & 0x7f) + C_BASETOKEN;
			inpbuf[1]=(tkn >> 7) + C_BASETOKEN; // tokens can be 14-bit
			memmove(&inpbuf[2],&inpbuf[4],strlen(inpbuf)-4);
			inpbuf[strlen(inpbuf)-2]=0;
#endif
 			strcpy(tknbuf,inpbuf);
        } else tokenise(false);                                            // turn into executable code
        p = tknbuf;
        while(!(p[0] == 0 && p[1] == 0)) {
            MemWriteByte(*p++); nbr++;

            if((realmempointer - (uint32_t)SDMemory) >= PROG_FLASH_SIZE - 5)
                goto exiterror1;
        }
        MemWriteByte(0); nbr++;                              // terminate that line in flash
    	routinechecks(1);
    }
    MemWriteByte(0);
    MemWriteAlign();                                            // this will flush the buffer and step the flash write pointer to the next word boundary
    retvalue=((uint32_t)realmempointer-(uint32_t)SDMemory);

    // now we must scan the program looking for CFUNCTION/CSUB/DEFINEFONT statements, extract their data and program it into the flash used by  CFUNCTIONs
     // programs are terminated with two zero bytes and one or more bytes of 0xff.  The CFunction area starts immediately after that.
     // the format of a CFunction/CSub/Font in flash is:
     //   Unsigned Int - Address of the CFunction/CSub in program memory (points to the token representing the "CFunction" keyword) or NULL if it is a font
     //   Unsigned Int - The length of the CFunction/CSub/Font in bytes including the Offset (see below)
     //   Unsigned Int - The Offset (in words) to the main() function (ie, the entry point to the CFunction/CSub).  Omitted in a font.
     //   word1..wordN - The CFunction/CSub/Font code
     // The next CFunction/CSub/Font starts immediately following the last word of the previous CFunction/CSub/Font
    realmemsave= realmempointer;
    p = SDMemory;                                              // start scanning program memory
   // while(*p != 0xff) {
   	while(!(p[0] == 0xff && p[1] == 0xff)){
    	nbr++;
    	if((nbr % 256)==0){
        	routinechecks(1);
    	}
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) break;                                          // end of the program
        if(*p == T_NEWLINE) {
            CurrentLinePtr = p;
            p++;                                                    // skip the newline token
        }
        if(*p == T_LINENBR) p += 3;                                 // step over the line number

        skipspace(p);
        if(*p == T_LABEL) {
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }

        CommandToken tkn = commandtbl_decode(p);
        if(tkn == cmdCSUB || tkn == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
			if(tkn == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
				 endtoken = GetCommandValue("End DefineFont");
				 p += sizeof(CommandToken);                               // step over the token
				 skipspace(p);
				 if(*p == '#') p++;
				 fontnbr = getint(p, 1, FONT_TABLE_SIZE);
			     // font 6 has some special characters, some of which depend on font 1
				 if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) {
					 FreeMemory(SDMemory);
					 error("Cannot redefine fonts 1, 6 or 7");
				 }
				 realmempointer+=4;
				 skipelement(p);                                     // go to the end of the command
				 p--;
			} else {
				endtoken = GetCommandValue("End CSub");
				realmempointer+=4;
				fontnbr = 0;
				//firsthex=0;
#ifdef CMD16BIT
                p++;
#endif
			}

              SaveSizeAddr = realmempointer;                                // save where we are so that we can write the CFun size in here
             realmempointer+=4;
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!isnamestart(*p)){
					 FreeMemory(SDMemory);
                	 error("Function name");
                 }
                 do { p++; } while(isnamechar(*p));
                 skipspace(p);
                 if(!(IsxDigit(p[0]) && IsxDigit(p[1]) && IsxDigit(p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit(*p)){
        					 FreeMemory(SDMemory);
                        	 error("Invalid hex word");
                         }
                         if((int)((char *)realmempointer - (uint32_t)SDMemory) >= PROG_FLASH_SIZE - 5) error("Not enough memory");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }
                     realmempointer+=4;
                     skipspace(p);
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0){
					 FreeMemory(SDMemory);
                	 error("Missing END declaration");
                 }
                 if(*p == T_NEWLINE) {
                     CurrentLinePtr = p;
                     p++;                                            // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                         // skip over the line number
                 skipspace(p);
            // } while(*p != endtoken);
                  tkn = commandtbl_decode(p);
                } while(tkn != endtoken);
             storedupdates[updatecount++]=realmempointer - SaveSizeAddr - 4;
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }
    realmempointer = realmemsave ;
    updatecount=0;
    p = SDMemory;                                              // start scanning program memory
     //while(*p != 0xff) {
     while(!(p[0] == 0xff && p[1] == 0xff)){
     	nbr++;
     	if((nbr % 256)==0){
        	routinechecks(1);
     	}
         if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
         if(*p == 0) break;                                          // end of the program
         if(*p == T_NEWLINE) {
             CurrentLinePtr = p;
             p++;                                                    // skip the newline token
         }
         if(*p == T_LINENBR) p += 3;                                 // step over the line number

         skipspace(p);
         if(*p == T_LABEL) {
             p += p[1] + 2;                                          // skip over the label
             skipspace(p);                                           // and any following spaces
         }
         CommandToken tkn = commandtbl_decode(p);
         if(tkn == cmdCSUB || tkn == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
            if(tkn == GetCommandValue("DefineFont")){
              //if(*p == cmdCSUB || *p == GetCommandValue("DefineFont")) {   // found a CFUNCTION, CSUB or DEFINEFONT token
			  //if(*p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
				 endtoken = GetCommandValue("End DefineFont");
				 p += sizeof(CommandToken);                                  // step over the token
				 skipspace(p);
				 if(*p == '#') p++;
				 fontnbr = getint(p, 1, FONT_TABLE_SIZE);
													 // font 6 has some special characters, some of which depend on font 1
				 if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) error("Cannot redefine fonts 1, 6, or 7");

				 MemWriteWord(fontnbr - 1);             // a low number (< FONT_TABLE_SIZE) marks the entry as a font
				 skipelement(p);                                     // go to the end of the command
				 p--;
			 } else {
				 endtoken = GetCommandValue("End CSub");
				 MemWriteWord((unsigned int)p);               // if a CFunction/CSub save a pointer to the declaration
				 fontnbr = 0;
#ifdef CMD16BIT
                  p++;
#endif


			 }
             SaveSizeAddr = realmempointer;                                // save where we are so that we can write the CFun size in here
             MemWriteWord(storedupdates[updatecount++]);                        // leave this blank so that we can later do the write
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!isnamestart(*p)){
					 FreeMemory(SDMemory);
                	 error("Function name");
                 }
                 do { p++; } while(isnamechar(*p));
                 skipspace(p);
                 if(!(IsxDigit(p[0]) && IsxDigit(p[1]) && IsxDigit(p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit(*p)){
        					 FreeMemory(SDMemory);
                        	 error("Invalid hex word");
                         }
                         if((int)((char *)realmempointer - (uint32_t)SDMemory) >= PROG_FLASH_SIZE - 5) error("Not enough memory");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }
                     MemWriteWord(n);
                     skipspace(p);
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0){
					 FreeMemory(SDMemory);
                	 error("Missing END declaration");
                 }
                 if(*p == T_NEWLINE) {
                    CurrentLinePtr = p;
                    p++;                                        // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 skipspace(p);
             //} while(*p != endtoken);
                  tkn = commandtbl_decode(p);
             } while(tkn != endtoken);
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }

     MemWriteWord(0xffffffff);                                // make sure that the end of the CFunctions is terminated with an erased word
     MemWriteClose();                                              // this will flush the buffer and step the flash write pointer to the next word boundary
     //retvalue=((uint32_t)realmempointer-(uint32_t)SDMemory);

    if(msg) {                                                       // if requested by the caller, print an informative message
        if(MMCharPos > 1) MMPrintString("\r\n");                    // message should be on a new line
        MMPrintString("Saved ");
        IntToStr(tknbuf, nbr + 3, 10);
        MMPrintString(tknbuf);
        MMPrintString(" bytes\r\n");
    }
    memcpy(tknbuf, buf, STRINGSIZE);                                // restore the token buffer in case there are other commands in it
    initConsole();
	clearrepeat();
//    SCB_EnableICache() ;
//    SCB_EnableDCache() ;
    return retvalue;

    // we only get here in an error situation while writing the program to flash
    exiterror1:
        MemWriteByte(0); MemWriteByte(0); MemWriteByte(0);    // terminate the program in flash
        MemWriteClose();
		FreeMemory(SDMemory);
        error("Not enough memory");
        return 0;
}
// insert a string into the start of the lastcmd buffer.
// the buffer is a sequence of strings separated by a zero byte.
// using the up arrow usere can call up the last few commands executed.
void InsertLastcmd(char *s) {
int i, slen;
    if(strcmp(lastcmd, s) == 0) return;                             // don't duplicate
    slen = strlen(s);
    if(slen < 1 || slen > STRINGSIZE*4 - 1) return;
    slen++;
    for(i = STRINGSIZE*4 - 1; i >=  slen ; i--)
        lastcmd[i] = lastcmd[i - slen];                             // shift the contents of the buffer up
    strcpy(lastcmd, s);                                             // and insert the new string in the beginning
    for(i = STRINGSIZE*4 - 1; lastcmd[i]; i--) lastcmd[i] = 0;             // zero the end of the buffer
}

void EditInputLine(void) {
    char *p = NULL;
    char buf[MAXKEYLEN + 3];
    int lastcmd_idx, lastcmd_edit;
    int insert, startline, maxchars;
    int CharIndex, BufEdited;
    int c, i, j;
    int maxW=PageTable[WritePage].xmax;

    maxchars = maxW / (gui_font_width);
    if(strlen(inpbuf) >= maxchars) {
        MMPrintString(inpbuf);
        error("Line is too long to edit");
    }
    startline = MMCharPos - 1;                                                          // save the current cursor position
    MMPrintString(inpbuf);                                                              // display the contents of the input buffer (if any)
    CharIndex = strlen(inpbuf);                                                         // get the current cursor position in the line
    insert = false;
//    Cursor = C_STANDARD;
    lastcmd_edit = lastcmd_idx = 0;
    BufEdited = false; //(CharIndex != 0);
    while(1) {
        c = MMgetchar(1);
        if(c == TAB) {
            strcpy(buf, "        ");
            switch (Option.Tab) {
              case 2:
                buf[2 - (CharIndex % 2)] = 0; break;
              case 3:
                buf[3 - (CharIndex % 3)] = 0; break;
              case 4:
                buf[4 - (CharIndex % 4)] = 0; break;
              case 8:
                buf[8 - (CharIndex % 8)] = 0; break;
            }
        } else {
            buf[0] = c;
            buf[1] = 0;
        }
        do {
            switch(buf[0]) {
                case '\r':
                case '\n':  //if(autoOn && atoi(inpbuf) > 0) autoNext = atoi(inpbuf) + autoIncr;
                            //if(autoOn && !BufEdited) *inpbuf = 0;
                            goto saveline;
                            break;

                case '\b':  if(CharIndex > 0) {
                                BufEdited = true;
                                i = CharIndex - 1;
                                for(p = inpbuf + i; *p; p++) *p = *(p + 1);                 // remove the char from inpbuf
                                while(CharIndex)  { MMputchar('\b'); CharIndex--; }         // go to the beginning of the line
                                MMPrintString(inpbuf); MMputchar(' '); MMputchar('\b');     // display the line and erase the last char
                                for(CharIndex = strlen(inpbuf); CharIndex > i; CharIndex--)
                                    MMputchar('\b');                                        // return the cursor to the righ position
                            }
                            break;

                case CTRLKEY('S'):
                case LEFT:  if(CharIndex > 0) {
                                if(CharIndex == strlen(inpbuf)) {
                                    insert = true;
      //                              Cursor = C_INSERT;
                                }
                                MMputchar('\b');
                                CharIndex--;
                            }
                            break;

                case CTRLKEY('D'):
                case RIGHT: if(CharIndex < strlen(inpbuf)) {
                                MMputchar(inpbuf[CharIndex]);
                                CharIndex++;
                            }
                            break;

                case CTRLKEY(']'):
                case DEL:   if(CharIndex < strlen(inpbuf)) {
                                BufEdited = true;
                                i = CharIndex;
                                for(p = inpbuf + i; *p; p++) *p = *(p + 1);                 // remove the char from inpbuf
                                while(CharIndex)  { MMputchar('\b'); CharIndex--; }         // go to the beginning of the line
                                MMPrintString(inpbuf); MMputchar(' '); MMputchar('\b');     // display the line and erase the last char
                                for(CharIndex = strlen(inpbuf); CharIndex > i; CharIndex--)
                                    MMputchar('\b');                                        // return the cursor to the right position
                            }
                            break;

                case CTRLKEY('N'):
                case INSERT:insert = !insert;
//                            Cursor = C_STANDARD + insert;
                            break;

                case CTRLKEY('U'):
                case HOME:  if(CharIndex > 0) {
                                if(CharIndex == strlen(inpbuf)) {
                                    insert = true;
//                                    Cursor = C_INSERT;
                                }
                                while(CharIndex)  { MMputchar('\b'); CharIndex--; }
                            }
                            break;

                case CTRLKEY('K'):
                case END:   while(CharIndex < strlen(inpbuf))
                                MMputchar(inpbuf[CharIndex++]);
                            break;

                case 0x91:
                case 0x92:
                case 0x93:
                case 0x94:
                case 0x95:
                case 0x96:
                case 0x97:
                case 0x98:
                case 0x99:
                case 0x9a:
               // case 0x9b:
               // case 0x9c:
                      if(*FunKey[buf[0] - 0x91])
                        strcpy(&buf[1], (char *)FunKey[buf[0] - 0x91]);                     // copy a function key string into the buffer
                      break;

                case 0x9b:
                      if(*Option.F11Key)strcpy(&buf[1],(char *)Option.F11Key);
                      break;
                case 0x9c:
                      if(*Option.F12Key)strcpy(&buf[1],(char *)Option.F12Key);
                      break;

                case 0xb3:      //Shift+F3
                     if(*Option.F15Key)strcpy(&buf[1],(char *)Option.F15Key);
                     break;

                case 0xb4:      //Shift+F4
                     if(*Option.F16Key)strcpy(&buf[1],(char *)Option.F16Key);
                     break;

                case 0xb5:   //Shift +F5    Clear Display and attached VT100
                	 CurrentX = CurrentY = 0;
    		         ClearScreen(gui_bcolour);
    		         SerUSBPutS("\0337\033[2J\033[H");
    		         MMPrintString("> ");
                	 break;

                case 0xb6:   //Shift + F6   Send sequence to setterminal
                	 setterminal();
                	 SerUSBPutS("> ");
                	 break;

                case 0xb7:   //Shift+F7
                     if(*Option.F19Key)strcpy(&buf[1],(char *)Option.F19Key);
                     break;

                case 0xb8:   //Shift+F8
                     if(*Option.F20Key)strcpy(&buf[1],(char *)Option.F20Key);
                     break;

                case CTRLKEY('E'):
                case UP:    if(!(BufEdited /*|| autoOn || CurrentLineNbr */)) {
                                while(CharIndex)  { MMputchar('\b'); CharIndex--; }         // go to the beginning of line
                                if(lastcmd_edit) {
                                    i = lastcmd_idx + strlen(&lastcmd[lastcmd_idx]) + 1;    // find the next command
                                    if(lastcmd[i] != 0 && i < STRINGSIZE*4 - 1) lastcmd_idx = i;  // and point to it for the next time around
                                } else
                                    lastcmd_edit = true;
                                strcpy(inpbuf, &lastcmd[lastcmd_idx]);                      // get the command into the buffer for editing
                                goto insert_lastcmd;
                            }
                            break;


                case CTRLKEY('X'):
                case DOWN:  if(!(BufEdited /*|| autoOn || CurrentLineNbr */)) {
                                while(CharIndex)  { MMputchar('\b'); CharIndex--; }         // go to the beginning of line
                                if(lastcmd_idx == 0)
                                    *inpbuf = lastcmd_edit = 0;
                                else {
                                    for(i = lastcmd_idx - 2; i > 0 && lastcmd[i - 1] != 0; i--);// find the start of the previous command
                                    lastcmd_idx = i;                                        // and point to it for the next time around
                                    strcpy(inpbuf, &lastcmd[i]);                            // get the command into the buffer for editing
                                }
                                goto insert_lastcmd;                                        // gotos are bad, I know, I know
                            }
                            break;

                insert_lastcmd:                                                             // goto here if we are just editing a command
                            if(strlen(inpbuf) + startline >= maxchars) {                    // if the line is too long
                                while(CharIndex)  { MMputchar('\b'); CharIndex--; }         // go to the start of the line
                                MMPrintString(inpbuf);                                      // display the offending line
                                error("Line is too long to edit");
                            }
                            MMPrintString(inpbuf);                                          // display the line
                            CharIndex = strlen(inpbuf);                                     // get the current cursor position in the line
                            for(i = 1; i <= maxchars - strlen(inpbuf) - startline; i++) {
                                MMputchar(' ');                                             // erase the rest of the line
                                CharIndex++;
                            }
                            while(CharIndex > strlen(inpbuf)) { MMputchar('\b'); CharIndex--; } // return the cursor to the right position
                            break;

                default:    if(buf[0] >= ' ' && buf[0] < 0x7f) {
                                BufEdited = true;                                           // this means that something was typed
                                i = CharIndex;
                                j = strlen(inpbuf);
                                if(insert) {
                                    if(strlen(inpbuf) >= maxchars - 1) break;               // sorry, line full
                                    for(p = inpbuf + strlen(inpbuf); j >= CharIndex; p--, j--) *(p + 1) = *p;
                                    inpbuf[CharIndex] = buf[0];                             // insert the char
                                    MMPrintString(&inpbuf[CharIndex]);                      // display new part of the line
                                    CharIndex++;
                                    for(j = strlen(inpbuf); j > CharIndex; j--)
                                        MMputchar('\b');                                    // return the cursor to the right position
                                } else {
                                    inpbuf[strlen(inpbuf) + 1] = 0;                         // incase we are adding to the end of the string
                                    inpbuf[CharIndex++] = buf[0];                           // overwrite the char
                                    MMputchar(buf[0]);                                      // display it
                                    if(CharIndex + startline >= maxchars) {                 // has the input gone beyond the end of the line?
                                        MMgetline(0, inpbuf);                               // use the old fashioned way of getting the line
                                        //if(autoOn && atoi(inpbuf) > 0) autoNext = atoi(inpbuf) + autoIncr;
                                        goto saveline;
                                    }
                                }
                            }
                            break;
            }
            for(i = 0; i < MAXKEYLEN + 1; i++) buf[i] = buf[i + 1];                             // shuffle down the buffer to get the next char
        } while(*buf);
    if(CharIndex == strlen(inpbuf)) {
        insert = false;
//        Cursor = C_STANDARD;
        }
    }

    saveline:
//    Cursor = C_STANDARD;
    MMPrintString("\r\n");
}
/**
  * @brief  Perform the SDRAM exernal memory inialization sequence
  * @param  hsdram: SDRAM handle
  * @param  Command: Pointer to SDRAM command structure
  * @retval None
  */
/**
  * @brief  Perform the SDRAM exernal memory inialization sequence
  * @param  hsdram: SDRAM handle
  * @param  Command: Pointer to SDRAM command structure
  * @retval None
  */
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
  __IO uint32_t tmpmrd =0;
  /* Step 1:  Configure a clock configuration enable command */
  Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;;
  Command->AutoRefreshNumber = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 2: Insert 100 us minimum delay */
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  HAL_Delay(1);

  /* Step 3: Configure a PALL (precharge all) command */
  Command->CommandMode = FMC_SDRAM_CMD_PALL;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 4 : Configure a Auto-Refresh command */
  Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber = 8;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 5: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber = 1;
  Command->ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 6: Set the refresh rate counter */
  /* Set the device refresh rate */
  HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);

}
/****************************************************************************************************************
This is the startup logo
It uses a simple form of run length encoding
For each byte:
     - The top three bits is the colour
     - The lower 5 bits is the number of horizontal bits of that colour to draw
****************************************************************************************************************/
// define where the elements of the start up messages will be placed

const char logo[LOGO_BYTES] = {
                    0x1F, 0x1D, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E,
                    0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5C, 0x09, 0x29, 0x09, 0x25,
                    0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5A, 0x0D, 0x25, 0x0D, 0x23, 0xDE, 0x1F, 0x05, 0xE7, 0x0B, 0xE7, 0x17, 0x9E, 0x5A, 0x04, 0xE7, 0x04, 0x23, 0x04, 0xE7, 0x04, 0x21, 0xDE, 0x1F, 0x04, 0xEB, 0x07,
                    0xEB, 0x15, 0x9E, 0x59, 0x03, 0xEB, 0x03, 0x21, 0x03, 0xEB, 0x03, 0xDE, 0x1F, 0x03, 0xED, 0x05, 0xED, 0x13, 0x9E, 0x59, 0x03, 0xED, 0x05, 0xED, 0x03, 0xDC, 0x1F, 0x03, 0xEF, 0x03, 0xEF, 0x12,
                    0x9E, 0x58, 0x03, 0xEF, 0x03, 0xEF, 0x03, 0xDB, 0x1F, 0x02, 0xF1, 0x01, 0xF1, 0x10, 0x9E, 0x59, 0x02, 0xF1, 0x01, 0xF1, 0x02, 0xDA, 0x1F, 0x03, 0xFF, 0xE4, 0x10, 0x9E, 0x58, 0x03, 0xFF, 0xE4,
                    0x03, 0xD9, 0x1F, 0x02, 0xFF, 0xE6, 0x0E, 0x9E, 0x59, 0x02, 0xFF, 0xE6, 0x02, 0xD8, 0x1F, 0x03, 0xFF, 0xE6, 0x15, 0x88, 0x0B, 0x84, 0x0B, 0x41, 0x0A, 0x42, 0x03, 0xFF, 0xE6, 0x03, 0xC1, 0x0A,
                    0xC2, 0x1F, 0x0C, 0xFF, 0xE8, 0x16, 0x85, 0x0D, 0x82, 0x18, 0x41, 0x02, 0xFF, 0xE8, 0x1F, 0x1B, 0xFF, 0xE8, 0x0B, 0xE8, 0x05, 0x83, 0x02, 0xE9, 0x06, 0xE9, 0x03, 0xE8, 0x02, 0x41, 0x02, 0xFF,
                    0xE8, 0x04, 0xE8, 0x04, 0xF6, 0x08, 0xEB, 0x01, 0xFF, 0xE8, 0x09, 0xEC, 0x04, 0x82, 0x02, 0xEA, 0x04, 0xEA, 0x03, 0xE8, 0x02, 0x41, 0x02, 0xFF, 0xE8, 0x04, 0xE8, 0x04, 0xF6, 0x06, 0xED, 0x01,
                    0xEB, 0x02, 0xED, 0x02, 0xEB, 0x07, 0xF0, 0x03, 0x81, 0x03, 0xE9, 0x04, 0xE9, 0x04, 0xE8, 0x02, 0x41, 0x02, 0xEB, 0x02, 0xED, 0x02, 0xEB, 0x04, 0xE8, 0x04, 0xF6, 0x05, 0xEE, 0x01, 0xEA, 0x04,
                    0xEB, 0x04, 0xEA, 0x06, 0xF2, 0x02, 0x82, 0x02, 0xEA, 0x02, 0xEA, 0x04, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x04, 0xF6, 0x04, 0xEF, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x06, 0xF2, 0x03, 0x81, 0x03, 0xE9, 0x02, 0xE9, 0x05, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x04, 0xF6, 0x04, 0xEF, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x05,
                    0xF4, 0x02, 0x82, 0x03, 0xF2, 0x03, 0x41, 0x02, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x04, 0xF6, 0x03, 0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x05, 0xF4, 0x03,
                    0x82, 0x02, 0xF2, 0x02, 0x42, 0x02, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x04, 0xF6, 0x03, 0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x82, 0x03,
                    0xF0, 0x03, 0x42, 0x02, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x04, 0xF6, 0x03, 0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE9, 0x04, 0xE9, 0x02, 0x83, 0x02,
                    0xF0, 0x02, 0x43, 0x02, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0A, 0xE8, 0x09, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x06, 0xE8, 0x02, 0x83, 0x03,
                    0xEE, 0x03, 0x43, 0x02, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC1, 0x08, 0xE8, 0x0A, 0xE8, 0x09, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0x82, 0x02,
                    0xE8, 0x02, 0x84, 0x02, 0xEE, 0x02, 0x44, 0x02, 0xE8, 0x02, 0x41, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC5, 0x04, 0xE8, 0x0A, 0xE8, 0x09, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04,
                    0xE8, 0x02, 0x82, 0x02, 0xE8, 0x02, 0x84, 0x03, 0xEC, 0x03, 0x44, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC4, 0x05, 0xE8, 0x0A, 0xF0, 0x01, 0xEA, 0x04,
                    0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x06, 0xE8, 0x02, 0x85, 0x02, 0xEC, 0x02, 0x45, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC4, 0x05, 0xE8, 0x0A, 0xF0, 0x01,
                    0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x06, 0xE8, 0x02, 0x85, 0x03, 0xEA, 0x03, 0x45, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC3, 0x06, 0xE8, 0x0A,
                    0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x86, 0x02, 0xEA, 0x02, 0x46, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC3, 0x06, 0xE8, 0x0A,
                    0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x85, 0x03, 0xEA, 0x03, 0x45, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC2, 0x07, 0xE8, 0x0A,
                    0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x85, 0x02, 0xEC, 0x02, 0x45, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC2, 0x07, 0xE8, 0x0A,
                    0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x84, 0x03, 0xEC, 0x03, 0x44, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC1, 0x08, 0xE8, 0x0A,
                    0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x84, 0x02, 0xEE, 0x02, 0x44, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x02, 0xC1, 0x08, 0xE8, 0x0A,
                    0xE8, 0x09, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x83, 0x03, 0xEE, 0x03, 0x43, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0A, 0xE8, 0x09,
                    0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x83, 0x02, 0xF0, 0x02, 0x43, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0A, 0xE8, 0x09, 0xEA, 0x04,
                    0xEB, 0x04, 0xEA, 0x04, 0xF6, 0x02, 0x82, 0x03, 0xF0, 0x03, 0x42, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0A, 0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x04, 0xE8, 0x06, 0xE8, 0x02, 0x82, 0x02, 0xF2, 0x02, 0x42, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0A, 0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x04, 0xE8, 0x06, 0xE8, 0x02, 0x81, 0x03, 0xF2, 0x03, 0x41, 0x02, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0A, 0xF0, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x04, 0xE8, 0x02, 0x82, 0x02, 0xE8, 0x05, 0xE9, 0x02, 0xE9, 0x05, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0B, 0xEF, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x04, 0xE8, 0x02, 0x82, 0x02, 0xE8, 0x04, 0xEA, 0x02, 0xEA, 0x04, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0B, 0xEF, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x04, 0xE8, 0x02, 0x82, 0x02, 0xE8, 0x04, 0xE9, 0x04, 0xE9, 0x04, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0C, 0xEE, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x04, 0xE8, 0x02, 0x82, 0x02, 0xE8, 0x03, 0xEA, 0x04, 0xEA, 0x03, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0D, 0xED, 0x01, 0xEA, 0x04, 0xEB, 0x04,
                    0xEA, 0x04, 0xE8, 0x02, 0x82, 0x02, 0xE8, 0x03, 0xE9, 0x06, 0xE9, 0x03, 0xE8, 0x02, 0x21, 0x02, 0xEA, 0x04, 0xEB, 0x04, 0xEA, 0x04, 0xE8, 0x0B, 0xE8, 0x0F, 0xEB, 0x1F, 0x17, 0x82, 0x18, 0x42,
                    0x18, 0x21, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x82, 0x0A, 0x84, 0x0A, 0x41, 0x0B, 0x44, 0x0B, 0x41, 0x0A, 0x23, 0x0C, 0x22, 0x0D, 0xC2, 0x0C, 0xC2, 0x1F, 0x1F, 0x1C, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F,
                    0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E,
                    0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F,
                    0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x16, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x1F, 0x17, 0x9E, 0x5E, 0x3E, 0xDE, 0x1F, 0x18
};

void DrawLogo(void) {
    int i, j, n;
    int maxW=PageTable[WritePage].xmax;
    int LOGO_X=maxW/2-102;
    int cmap[8]={0x0,0xff,0xff00,0xffff,0xff0000,0xff00ff,0xffff00,0xffffff};
    n = 0;
    for(i = 0; i < LOGO_BYTES; i++) {
        for(j = 0; j < (logo[i] & 0b00011111); j++) {
        	DrawPixel(LOGO_X + (n % LOGO_WIDTH), LOGO_Y + (n / LOGO_WIDTH), cmap[logo[i] >> 5]);
            n++;
        }
    }
    CurrentY=70;
}

void firstinit(void){
	struct tm  *tm;
	struct tm tma;
	tm=&tma;
	  int loop, redo, c;
	  int dd=01, mm=0, yy=24, h=0, m=0, s=0, keyboard;
	  //prompt for various settings
	  MMAbort = false;
      BreakKey = 0;
	  MMPrintString("Initial setup - all parameters can be changed later\r\n");
	  loop=1;
	  MMPrintString("Keyboard type: 1=UK, 2=US, 3=DE , 4=FR, 5=ES , 6=BE ? ");
	  while(loop){
		  while((c=getConsole())==-1){ShowCursor(true);}
		  if(!(c>='1' && c<=(MAXKEYBOARDS | 48)))continue;
		  loop=0;
		  ShowCursor(false);
		  putConsole(c);
		  MMPrintString("\r\n");
		  keyboard=c-48;
	  }
	  MMPrintString("Enter time (24hr clock) HH:MM:SS ? ");
	      loop=1;
	      while(loop){ //tens of hours
	    	  while((c=getConsole())==-1){ShowCursor(true);}
	    	  if(!(c>='0' || c<='2'))continue;
	    	  ShowCursor(false);
	    	  putConsole(c);
	    	  loop=0;
	    	  h=(c-48)*10;
	      }
	      loop=1;
	      while(loop){ //units of hours
	    	  while((c=getConsole())==-1){ShowCursor(true);}
	    	  if(!IsDigitinline(c))continue;
	    	  if(!(c>='0' && c<='3') && h==20)continue;
	    	  ShowCursor(false);
	    	  putConsole(c);
	    	  loop=0;
	    	  h+=(c-48);
	      }
	      MMPrintString(":");
	      loop=1;
	      while(loop){ //tens of minutes
	    	  while((c=getConsole())==-1){ShowCursor(true);}
	    	  if(!(c>='0' && c<='5'))continue;
	    	  ShowCursor(false);
	    	  putConsole(c);
	    	  loop=0;
	    	  m=(c-48)*10;
	      }
	      loop=1;
	      while(loop){ //units of minutes
	    	  while((c=getConsole())==-1){ShowCursor(true);}
	    	  if(!IsDigitinline(c))continue;
	    	  ShowCursor(false);
	    	  putConsole(c);
	    	  loop=0;
	    	  m+=(c-48);
	      }
	      MMPrintString(":");
	      loop=1;
	      while(loop){ //tens of seconds
	    	  while((c=getConsole())==-1){ShowCursor(true);}
	    	  if(!(c>='0' && c<='5'))continue;
	    	  ShowCursor(false);
	    	  putConsole(c);
	    	  loop=0;
	    	  s=(c-48)*10;
	      }
	      loop=1;
	      while(loop){ //units of seconds
	    	  while((c=getConsole())==-1){ShowCursor(true);}
	    	  if(!IsDigitinline(c))continue;
	    	  ShowCursor(false);
	    	  putConsole(c);
	    	  loop=0;
	    	  s+=(c-48);
	      }
		  mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		  hour = h;
		  minute = m;
		  second = s;
		  SecondsTimer = 0;
		  update_clock();
	      mT4IntEnable(1);       										// enable interrupt
	      do {
	    	  redo=0;
		      MMPrintString("\r\nEnter date DD/MM/YY ? ");
		      loop=1;
		      while(loop){ //tens of days
		    	  while((c=getConsole())==-1){ShowCursor(true);}
		    	  if(!(c>='0' && c<='3'))continue;
		    	  ShowCursor(false);
		    	  putConsole(c);
		    	  loop=0;
		    	  dd=(c-48)*10;
		      }
		      loop=1;
		      while(loop){ //units of days
		    	  while((c=getConsole())==-1){ShowCursor(true);}
		    	  if(!IsDigitinline(c))continue;
		    	  if(!(c>='0' && c<='1') && dd==30)continue;
		    	  if(!(c>'0' && c<='9') && dd==0)continue;
		    	  ShowCursor(false);
		    	  putConsole(c);
		    	  loop=0;
		    	  dd+=(c-48);
		      }
		      MMPrintString("/");
		      loop=1;
		      while(loop){ //tens of months
		    	  while((c=getConsole())==-1){ShowCursor(true);}
		    	  if(!(c>='0' && c<='1'))continue;
		    	  ShowCursor(false);
		    	  putConsole(c);
		    	  loop=0;
		    	  mm=(c-48)*10;
		      }
		      loop=1;
		      while(loop){ //units of months
		    	  while((c=getConsole())==-1){ShowCursor(true);}
		    	  if(!IsDigitinline(c))continue;
		    	  if(!(c>='0' && c<='2') && mm)continue;
		    	  if(!(c>'0' && c<='9') && mm==0)continue;
		    	  ShowCursor(false);
		    	  putConsole(c);
		    	  loop=0;
		    	  mm+=(c-48);
		      }
		      MMPrintString("/");
		      loop=1;
		      while(loop){ //tens of years
		    	  while((c=getConsole())==-1){ShowCursor(true);}
		    	  if(!IsDigitinline(c))continue;
		    	  ShowCursor(false);
		    	  putConsole(c);
		    	  loop=0;
		    	  yy=(c-48)*10;
		      }
		      loop=1;
		      while(loop){ //units of years
		    	  while((c=getConsole())==-1){ShowCursor(true);}
		    	  if(!IsDigitinline(c))continue;
		    	  ShowCursor(false);
		    	  putConsole(c);
		    	  loop=0;
		    	  yy+=(c-48);
		      }
		      yy+=2000;
			  //check days
	          if((dd>=1 && dd<=31) && (mm==1 || mm==3 || mm==5 || mm==7 || mm==8 || mm==10 || mm==12))
	              {}
	          else if((dd>=1 && dd<=30) && (mm==4 || mm==6 || mm==9 || mm==11))
	              {}
	          else if((dd>=1 && dd<=28) && (mm==2))
	              {}
	          else if(dd==29 && mm==2 && (yy%400==0 ||(yy%4==0 && yy%100!=0)))
	              {}
	          else
	              {MMPrintString(" !Date is invalid"); redo=1;}
	      } while(redo);
			mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
			day = dd;
			month = mm;
			year = yy;
		    tm->tm_year = year - 1900;
		    tm->tm_mon = month - 1;
		    tm->tm_mday = day;
		    tm->tm_hour = hour;
		    tm->tm_min = minute;
		    tm->tm_sec = second;
		    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
		    tm=gmtime(&timestamp);
		    day_of_week=tm->tm_wday;
		    if(day_of_week==0)day_of_week=7;
			update_clock();
			mT4IntEnable(1);       										// enable interrupt
			Option.USBKeyboard=keyboard;
		    SaveOptions(0);
		    MMPrintString("\r\n");
		    while(ConsoleTxBufTail != ConsoleTxBufHead);
		    _excep_code = 0;                            // otherwise do an automatic reset
		    SoftReset();                                                // this will restart the processor

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	MMErrorString("Error:");
    char buf[20];
    IntToStr(buf, SystemError, 10);
    MMErrorString(buf);
    MMErrorString("\r\n");

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
