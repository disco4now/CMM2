/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
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
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <dr_mp3.h>
#include "main.h"
#include "allcommands.h"
#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "configuration.h"
#include "serial.h"
#include "gps.h"
#include "audio.h"
#include "sam.h"
#include "reciter.h"
#include "Memory.h"
#include "flash.h"
#include "IOPorts.h"
#include "External.h"
/* USER CODE END Includes */


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern FDCAN_HandleTypeDef hfdcan;    //CAN added
extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
extern DAC_HandleTypeDef hdac1;
extern DMA2D_HandleTypeDef hdma2d;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;
extern LTDC_HandleTypeDef hltdc;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim13;
extern TIM_HandleTypeDef htim16;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern void Timer1msHandler(void);
extern void Audio_Interrupt(void);
extern volatile uint64_t Count5High;
extern volatile int ConsoleRxBufHead;
extern volatile int ConsoleRxBufTail;
extern volatile char ConsoleTxBuf[];
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
extern volatile char ConsoleRxBuf[];
extern char BreakKey;
extern volatile int MMAbort;
extern int64_t *d1point, *d2point;
extern int d1max, d2max;
extern volatile int d1pos, d2pos;
extern volatile uint64_t * volatile a1point, * volatile a2point, * volatile a3point;
extern volatile MMFLOAT  * volatile a1float, * volatile a2float, * volatile a3float;
extern MMFLOAT ADCscale[3], ADCbottom[3];
extern int ADCmax;
extern int ADCScaled;
extern volatile int ADCpos;
extern volatile int ADCchannelA;
extern volatile int ADCchannelB;
extern volatile int ADCchannelC;
extern DAC_HandleTypeDef hdac1;
extern int ADCtriggervalue;
extern int ADCtriggertimeout;
extern int ADCtriggerchannel;
extern int ADCnegativeslope;
extern volatile int ADCcomplete;
extern void error(char *msg, ...);
extern volatile MMFLOAT VCC;
extern const MMFLOAT ADCdiv[];
extern volatile unsigned char ADCbits[];
extern volatile uint64_t uSecTimer;
extern volatile uint64_t g_uptime;
extern volatile uint64_t FastTimer;
extern uint8_t OptionConsole;
extern volatile uint8_t FlashDone;
extern volatile int sound_v_left[MAXSOUNDS];
extern volatile int sound_v_right[MAXSOUNDS];
extern volatile float sound_PhaseAC_left[MAXSOUNDS], sound_PhaseAC_right[MAXSOUNDS];
extern volatile float sound_PhaseM_left[MAXSOUNDS], sound_PhaseM_right[MAXSOUNDS];
extern volatile unsigned short * sound_mode_left[MAXSOUNDS];
extern volatile unsigned short * sound_mode_right[MAXSOUNDS];
extern volatile uint64_t SoundPlay;
extern volatile int ppos;                                                       // playing position for PLAY WAV
extern volatile int ppose;                                                       // playing position for PLAY WAV
extern int nchannels;
extern volatile e_CurrentlyPlaying CurrentlyPlaying;
extern volatile e_CurrentlyPlaying CurrentlyPlayinge;
extern volatile int swingbuf,nextbuf, playreadcomplete;
extern volatile int swingbufe ,nextbufe , playreadcompletee;
extern char *sbuff1, *sbuff2;
extern uint16_t *ibuff1, *ibuff2;
extern char *sbuff1e, *sbuff2e;
extern uint16_t *ibuff1e, *ibuff2e;
extern int WAVcomplete;
extern DAC_HandleTypeDef hdac1;
extern volatile float PhaseM_left, PhaseM_right;
extern const unsigned short SineTable[4096];
extern uint16_t *flacbuff;
extern volatile float PhaseAC_left, PhaseAC_right;
extern TIM_HandleTypeDef htim4;
extern volatile uint64_t bcount[3];
extern volatile uint64_t bcounte[3];
extern volatile unsigned char PWM_count;
extern char *pbuffp;
extern const unsigned short whitenoise[2];
extern const unsigned short nulltable[4096];
extern int mono;
extern RNG_HandleTypeDef hrng;/* USER CODE END EV */
extern int overrun;                                               // value of MM.OW
extern volatile char *DACInterrupt;
extern volatile int DACcomplete;
extern void dacclose(void);
extern char *KeyInterrupt;
extern volatile int Keycomplete;
extern int keyselect;
extern volatile int TickTimer[NBRSETTICKS+1];
extern int ExtCurrentConfig[];
extern void SerialConsolePutC(int c);

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  Timer1msHandler();
    /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles FDCAN1 interrupt 0.
  */
void FDCAN1_IT0_IRQHandler(void)
{
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 0 */

  /* USER CODE END FDCAN1_IT0_IRQn 0 */
  HAL_FDCAN_IRQHandler(&hfdcan);
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 1 */

  /* USER CODE END FDCAN1_IT0_IRQn 1 */
}


/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */
  if(ExtCurrentConfig[COUNT5]==EXT_CNT_IN)Count5High++;

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */
  Audio_Interrupt();
  /* USER CODE END TIM4_IRQn 1 */
}

/**
  * @brief This function handles I2C1 event interrupt.
  */
void I2C1_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */

  /* USER CODE END I2C1_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c1);
  /* USER CODE BEGIN I2C1_EV_IRQn 1 */

  /* USER CODE END I2C1_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C2 event interrupt.
  */
void I2C2_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_EV_IRQn 0 */

  /* USER CODE END I2C2_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c2);
  /* USER CODE BEGIN I2C2_EV_IRQn 1 */

  /* USER CODE END I2C2_EV_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
  	uint32_t isrflags   = READ_REG(huart1.Instance->ISR);
  	__HAL_UART_CLEAR_OREFLAG(&huart1);
  	__HAL_UART_CLEAR_NEFLAG(&huart1);
  	__HAL_UART_CLEAR_FEFLAG(&huart1);
  	if(OptionConsole & 1 && Option.ConsolePort==3){
  		if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET){
  			ConsoleRxBuf[ConsoleRxBufHead]  = huart1.Instance->RDR;   // store the byte in the ring buffer
  			if(BreakKey && ConsoleRxBuf[ConsoleRxBufHead] == BreakKey) {// if the user wants to stop the progran
  				MMAbort = true;                                        // set the flag for the interpreter to see
  				ConsoleRxBufHead = ConsoleRxBufTail;                    // empty the buffer
  			} else if(ConsoleRxBuf[ConsoleRxBufHead] ==keyselect && KeyInterrupt!=NULL){
  					Keycomplete=1;
  			} else {
  				ConsoleRxBufHead = (ConsoleRxBufHead + 1) % CONSOLE_RX_BUF_SIZE;     // advance the head of the queue
  				int i = ConsoleRxBufHead - ConsoleRxBufTail;
  				if(i < 0) i += CONSOLE_RX_BUF_SIZE;
  				if(ConsoleRxBufHead == ConsoleRxBufTail) {                           // if the buffer has overflowed
  					ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE; // throw away the oldest char
  				}
  			}
  		}
  		if ((isrflags & USART_ISR_TC) != RESET){
  			if(ConsoleTxBufTail != ConsoleTxBufHead) {
  				huart1.Instance->TDR = ConsoleTxBuf[ConsoleTxBufTail];
  				ConsoleTxBufTail = (ConsoleTxBufTail + 1) % CONSOLE_TX_BUF_SIZE; // advance the tail of the queue
  			} else {
  				huart1.Instance->CR1 &= ~USART_CR1_TCIE;
  			}

  		}
  	} else {
  		if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET){
  			char cc = huart1.Instance->RDR;
  	        if(GPSchannel==3){
  	            *gpsbuf=cc;
  	            gpsbuf++;
  	            gpscount++;
  	            if((char)cc==10 || gpscount==128){
  	               if(gpscurrent){
  	                   *gpsbuf=0;
  	                   gpscurrent=0;
  	                   gpscount=0;
  	                   gpsbuf=gpsbuf1;
  	                   gpsready=gpsbuf2;
  	               } else {
  	                   *gpsbuf=0;
  	                   gpscurrent=1;
  	                   gpscount=0;
  	                   gpsbuf=gpsbuf2;
  	                   gpsready=gpsbuf1;

  	               }
  	            }
  	        } else {
  	        	com3Rx_buf[com3Rx_head]  = cc;   // store the byte in the ring buffer
  	        	com3Rx_head = (com3Rx_head + 1) % com3_buf_size;     // advance the head of the queue
  	        	if(com3Rx_head == com3Rx_tail) {                           // if the buffer has overflowed
  	        		com3Rx_tail = (com3Rx_tail + 1) % com3_buf_size; // throw away the oldest char
  	        	}
  	        }
  		}
  		if ((isrflags & USART_ISR_TC) != RESET){
  			if(com3Tx_head != com3Tx_tail) {
  				huart1.Instance->TDR = com3Tx_buf[com3Tx_tail];
  			    com3Tx_tail = (com3Tx_tail + 1) % TX_BUFFER_SIZE;       // advance the tail of the queue
  			} else {
  		        huart1.Instance->CR1 &= ~USART_CR1_TCIE;
  			}
  		}
  	}
  	return;

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
	uint32_t isrflags   = READ_REG(huart2.Instance->ISR);
 	__HAL_UART_CLEAR_OREFLAG(&huart2);
  	__HAL_UART_CLEAR_NEFLAG(&huart2);
  	__HAL_UART_CLEAR_FEFLAG(&huart2);
  	if(OptionConsole & 1 && Option.ConsolePort==1){
  		if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET){
  			ConsoleRxBuf[ConsoleRxBufHead]  = huart2.Instance->RDR;   // store the byte in the ring buffer
  			if(BreakKey && ConsoleRxBuf[ConsoleRxBufHead] == BreakKey) {// if the user wants to stop the progran
  				MMAbort = true;                                        // set the flag for the interpreter to see
  				ConsoleRxBufHead = ConsoleRxBufTail;                    // empty the buffer
  			} else if(ConsoleRxBuf[ConsoleRxBufHead] ==keyselect && KeyInterrupt!=NULL){
  					Keycomplete=1;
  			} else {
  				ConsoleRxBufHead = (ConsoleRxBufHead + 1) % CONSOLE_RX_BUF_SIZE;     // advance the head of the queue
  				if(ConsoleRxBufHead == ConsoleRxBufTail) {                           // if the buffer has overflowed
  					ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE; // throw away the oldest char
  				}
  			}
  		}
  		if ((isrflags & USART_ISR_TC) != RESET){
  			if(ConsoleTxBufTail != ConsoleTxBufHead) {
  				huart2.Instance->TDR = ConsoleTxBuf[ConsoleTxBufTail];
  				ConsoleTxBufTail = (ConsoleTxBufTail + 1) % CONSOLE_TX_BUF_SIZE; // advance the tail of the queue
  			} else {
  				huart2.Instance->CR1 &= ~USART_CR1_TCIE;
  			}

  		}
  	} else {
		if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET){
			char cc = huart2.Instance->RDR;
			if(GPSchannel==1){
				*gpsbuf=cc;
				gpsbuf++;
				gpscount++;
				if((char)cc==10 || gpscount==128){
				   if(gpscurrent){
					   *gpsbuf=0;
					   gpscurrent=0;
					   gpscount=0;
					   gpsbuf=gpsbuf1;
					   gpsready=gpsbuf2;
				   } else {
					   *gpsbuf=0;
					   gpscurrent=1;
					   gpscount=0;
					   gpsbuf=gpsbuf2;
					   gpsready=gpsbuf1;
				   }
				}
			} else {
				com1Rx_buf[com1Rx_head]  = cc;   // store the byte in the ring buffer
				com1Rx_head = (com1Rx_head + 1) % com1_buf_size;     // advance the head of the queue
				if(com1Rx_head == com1Rx_tail) {                           // if the buffer has overflowed
					com1Rx_tail = (com1Rx_tail + 1) % com1_buf_size; // throw away the oldest char
				}
			}
		}

		if ((isrflags & USART_ISR_TC) != RESET){
			if(com1Tx_head != com1Tx_tail) {
				huart2.Instance->TDR = com1Tx_buf[com1Tx_tail];
				com1Tx_tail = (com1Tx_tail + 1) % TX_BUFFER_SIZE;       // advance the tail of the queue
			} else {
				huart2.Instance->CR1 &= ~USART_CR1_TCIE;
			}
		}
  	}
	return;

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */
	uint32_t isrflags   = READ_REG(huart4.Instance->ISR);
 	__HAL_UART_CLEAR_OREFLAG(&huart4);
  	__HAL_UART_CLEAR_NEFLAG(&huart4);
  	__HAL_UART_CLEAR_FEFLAG(&huart4);
  	if(OptionConsole & 1 && Option.ConsolePort==2){
  		if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET){
  			ConsoleRxBuf[ConsoleRxBufHead]  = huart4.Instance->RDR;   // store the byte in the ring buffer
  			if(BreakKey && ConsoleRxBuf[ConsoleRxBufHead] == BreakKey) {// if the user wants to stop the progran
  				MMAbort = true;                                        // set the flag for the interpreter to see
  				ConsoleRxBufHead = ConsoleRxBufTail;                    // empty the buffer
  			} else if(ConsoleRxBuf[ConsoleRxBufHead] ==keyselect && KeyInterrupt!=NULL){
  					Keycomplete=1;
  			} else {
  				ConsoleRxBufHead = (ConsoleRxBufHead + 1) % CONSOLE_RX_BUF_SIZE;     // advance the head of the queue
  				if(ConsoleRxBufHead == ConsoleRxBufTail) {                           // if the buffer has overflowed
  					ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE; // throw away the oldest char
  				}
  			}
  		}
  		if ((isrflags & USART_ISR_TC) != RESET){
  			if(ConsoleTxBufTail != ConsoleTxBufHead) {
  				huart4.Instance->TDR = ConsoleTxBuf[ConsoleTxBufTail];
  				ConsoleTxBufTail = (ConsoleTxBufTail + 1) % CONSOLE_TX_BUF_SIZE; // advance the tail of the queue
  			} else {
  				huart4.Instance->CR1 &= ~USART_CR1_TCIE;
  			}

  		}
  	} else {
		if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET){
			char cc = huart4.Instance->RDR;
			if(GPSchannel==2){
				*gpsbuf=cc;
				gpsbuf++;
				gpscount++;
				if((char)cc==10 || gpscount==128){
				   if(gpscurrent){
					   *gpsbuf=0;
					   gpscurrent=0;
					   gpscount=0;
					   gpsbuf=gpsbuf1;
					   gpsready=gpsbuf2;
				   } else {
					   *gpsbuf=0;
					   gpscurrent=1;
					   gpscount=0;
					   gpsbuf=gpsbuf2;
					   gpsready=gpsbuf1;

				   }
				}
			} else {
				com2Rx_buf[com2Rx_head]  = cc;   // store the byte in the ring buffer
				com2Rx_head = (com2Rx_head + 1) % com2_buf_size;     // advance the head of the queue
				if(com2Rx_head == com2Rx_tail) {                           // if the buffer has overflowed
					com2Rx_tail = (com2Rx_tail + 1) % com2_buf_size; // throw away the oldest char
				}
			}
		}
		if ((isrflags & USART_ISR_TC) != RESET){
			if(com2Tx_head != com2Tx_tail) {
				huart4.Instance->TDR = com2Tx_buf[com2Tx_tail];
				com2Tx_tail = (com2Tx_tail + 1) % TX_BUFFER_SIZE;       // advance the tail of the queue
			} else {
				huart4.Instance->CR1 &= ~USART_CR1_TCIE;
			}
		}
  	}
	return;

  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */

  /* USER CODE END UART4_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1_CH1 and DAC1_CH2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_DAC_IRQHandler(&hdac1);
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */
	if(d1max){
		if(!d2max){
			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, d1point[d1pos]);
			d1pos++;
			if(d1pos>d1max){
				if(DACInterrupt != NULL){
					DACcomplete=true;
					dacclose();
				}
				else d1pos=0;
			}
		} else {
			HAL_DACEx_DualSetValue(&hdac1, DAC_ALIGN_12B_R, d1point[d1pos], d2point[d2pos]);
			d1pos++;
			if(d1pos>d1max){
				if(DACInterrupt != NULL){
					DACcomplete=true;
					dacclose();
				}
				else d1pos=0;
			}
			d2pos++;
			if(d2pos>d2max) d2pos=0;
		}
	}

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */
	static int lastread, ADCtriggerfound,timeout;
	int c, c1, c2=0, c3=0, a;
	if (!ADCScaled){
		ADCscale[0]= VCC/ADCdiv[ADCbits[ADCchannelA]];
		ADCscale[1]= VCC/ADCdiv[ADCbits[ADCchannelB]];
		ADCscale[2]= VCC/ADCdiv[ADCbits[ADCchannelC]];
		ADCbottom[0]=0;
		ADCbottom[1]=0;
	    ADCbottom[2]=0;
	}
	if( (HAL_IS_BIT_SET(hadc1.Instance->ISR, EOC_SINGLE_CONV))){
		c1=HAL_ADC_GetValue(&hadc1);
		if(ADCpos < ADCmax)HAL_ADC_Start(&hadc1);
		if(ADCchannelB){
			a=10000; while (HAL_IS_BIT_CLR(hadc2.Instance->ISR, EOC_SINGLE_CONV) && a--);
			c2=HAL_ADC_GetValue(&hadc2);
			if(ADCpos < ADCmax)HAL_ADC_Start(&hadc2);
		}
		if(ADCchannelC){
			a=10000; while (HAL_IS_BIT_CLR(hadc3.Instance->ISR, EOC_SINGLE_CONV) && a--);
			c3=HAL_ADC_GetValue(&hadc3);
			if(ADCpos < ADCmax)HAL_ADC_Start(&hadc3);
		}

		a1float[ADCpos]=((MMFLOAT)(ADCscale[0]*c1) + ADCbottom[0] );
		if(ADCchannelB && a2float!=NULL)a2float[ADCpos]=((MMFLOAT)(ADCscale[1]*c2) + ADCbottom[1] );
		if(ADCchannelC && a3float!=NULL)a3float[ADCpos]=((MMFLOAT)(ADCscale[2]*c3) + ADCbottom[2] );

		//a1float[ADCpos]=a1float[ADCpos]=(MMFLOAT)c1/ADCdiv[ADCbits[ADCchannelA]] * VCC;
		//if(ADCchannelB && a2float!=NULL)a2float[ADCpos]=(MMFLOAT)c2/ADCdiv[ADCbits[ADCchannelA]] * VCC;
		//if(ADCchannelC && a3float!=NULL)a3float[ADCpos]=(MMFLOAT)c3/ADCdiv[ADCbits[ADCchannelA]] * VCC;

		if(ADCpos==ADCmax){
			__HAL_TIM_SET_COUNTER(&htim7, 0);
			HAL_NVIC_DisableIRQ(TIM7_IRQn);
			HAL_TIM_Base_Stop(&htim7);
			HAL_TIM_Base_DeInit(&htim7);
			__HAL_RCC_TIM7_CLK_DISABLE();
			ADCcomplete=true;
			TIM7->SR=0;
			return;
		}
		if(ADCtriggerchannel){
			if(ADCtriggerchannel==1)c=c1;
			else if(ADCtriggerchannel==2)c=c2;
			else c=c3;
			if(ADCpos==0){
				ADCtriggerfound=0;
				lastread=c;
				timeout=ADCtriggertimeout;
			} else if(!ADCtriggerfound){
				if(ADCnegativeslope){ //if looking for down slope
					if(lastread>=ADCtriggervalue && c<ADCtriggervalue){
						ADCtriggerfound=1;
					} else {
						lastread=c;
						ADCpos--;
					    a1float[ADCpos]=((MMFLOAT)(ADCscale[0]*c1) + ADCbottom[0] );
						if(ADCchannelB)a2float[ADCpos]=((MMFLOAT)(ADCscale[1]*c2) + ADCbottom[1] );
						if(ADCchannelC)a3float[ADCpos]=((MMFLOAT)(ADCscale[2]*c3) + ADCbottom[2] );
						//a1float[ADCpos]=(MMFLOAT)c1/ADCdiv[ADCbits[ADCchannelA]] * VCC;
						//if(ADCchannelB)a2float[ADCpos]=(MMFLOAT)c2/ADCdiv[ADCbits[ADCchannelA]] * VCC;
						//if(ADCchannelC)a3point[ADCpos]=(MMFLOAT)c3/ADCdiv[ADCbits[ADCchannelA]] * VCC;
					}
				} else {
					if(lastread<=ADCtriggervalue && c>ADCtriggervalue){
						ADCtriggerfound=1;
					} else {
						lastread=c;
						ADCpos--;
						a1float[ADCpos]=((MMFLOAT)(ADCscale[0]*c1) + ADCbottom[0] );
						if(ADCchannelB)a2float[ADCpos]=((MMFLOAT)(ADCscale[1]*c2) + ADCbottom[1] );
						if(ADCchannelC)a3float[ADCpos]=((MMFLOAT)(ADCscale[2]*c3) + ADCbottom[2] );
						//a1float[ADCpos]=(MMFLOAT)c1/ADCdiv[ADCbits[ADCchannelA]] * VCC;
						//if(ADCchannelB)a2float[ADCpos]=(MMFLOAT)c2/ADCdiv[ADCbits[ADCchannelA]] * VCC;
						//if(ADCchannelC)a3point[ADCpos]=(MMFLOAT)c3/ADCdiv[ADCbits[ADCchannelA]] * VCC;
					}
				}
			}//trigger not found
		    timeout--;
		 	if(timeout==0){ADCtriggerfound=1;}
		}
		ADCpos++;
	} else overrun++;
  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_IRQn 0 */

  /* USER CODE END LTDC_IRQn 0 */
  HAL_LTDC_IRQHandler(&hltdc);
  /* USER CODE BEGIN LTDC_IRQn 1 */

  /* USER CODE END LTDC_IRQn 1 */
}

/**
  * @brief This function handles DMA2D global interrupt.
  */
void DMA2D_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2D_IRQn 0 */

  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&hdma2d);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/**
  * @brief This function handles I2C4 event interrupt.
  */
void I2C4_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C4_EV_IRQn 0 */

  /* USER CODE END I2C4_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c4);
  /* USER CODE BEGIN I2C4_EV_IRQn 1 */

  /* USER CODE END I2C4_EV_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}
/**
  * @brief This function handles TIM8 update interrupt and TIM13 global interrupt.
  */
void TIM8_UP_TIM13_IRQHandler(void)
{
  /* USER CODE BEGIN TIM8_UP_TIM13_IRQn 0 */

  /* USER CODE END TIM8_UP_TIM13_IRQn 0 */
  HAL_TIM_IRQHandler(&htim13);
  /* USER CODE BEGIN TIM8_UP_TIM13_IRQn 1 */
  TickTimer[NBRSETTICKS]++;

  /* USER CODE END TIM8_UP_TIM13_IRQn 1 */
}

/**
  * @brief This function handles TIM16 global interrupt.
  */
void TIM16_IRQHandler(void)
{
  /* USER CODE BEGIN TIM16_IRQn 0 */

  /* USER CODE END TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim16);
  /* USER CODE BEGIN TIM16_IRQn 1 */
	uSecTimer++;
	g_uptime++;

  /* USER CODE END TIM16_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void TIM5_IRQHandler(void)
{
  /* USER CODE BEGIN TIM16_IRQn 0 */

  /* USER CODE END TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim5);
  /* USER CODE BEGIN TIM16_IRQn 1 */
  FastTimer+=0x100000000;
  /* USER CODE END TIM16_IRQn 1 */
}
void FLASH_IRQHandler(void)
{
  /* USER CODE BEGIN FLASH_IRQn 0 */

  /* USER CODE END FLASH_IRQn 0 */
  HAL_FLASH_IRQHandler();
  /* USER CODE BEGIN FLASH_IRQn 1 */
  FlashDone=1;
  /* USER CODE END FLASH_IRQn 1 */
}
void EXTI4_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4); //PC4 COUNT1
}
void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1); //PG2 COUNT2
}
void EXTI2_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2); //PG2 COUNT3
}
void EXTI3_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3); //PG3 COUNT4, PI3 MOUSE
}
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12); //PI8 IR
}
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

void Audio_Interrupt(void)  {
    // play a tone
    if(CurrentlyPlaying == P_TONE){
        if(!SoundPlay){
        	CloseAudio(1);
            WAVcomplete = true;
        } else {
        	SoundPlay--;
        	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, (((SineTable[(int)PhaseAC_left]-2000)  * mapping[vol_left]) / 2000)+2000);
        	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, (((SineTable[(int)PhaseAC_right]-2000)  * mapping[vol_right]) / 2000)+2000);
        	PhaseAC_left = PhaseAC_left + PhaseM_left;
        	PhaseAC_right = PhaseAC_right + PhaseM_right;
        	if(PhaseAC_left>=4096.0)PhaseAC_left-=4096.0;
        	if(PhaseAC_right>=4096.0)PhaseAC_right-=4096.0;
        }
    } else if(CurrentlyPlaying == P_MP3 || CurrentlyPlaying == P_WAV  || CurrentlyPlaying == P_FLAC) {
        if(bcount[1]==0 && bcount[2]==0 && playreadcomplete==1){
        	HAL_TIM_Base_Stop_IT(&htim4);
        }
        if(swingbuf){ //buffer is primed
        	if(swingbuf==1)flacbuff=(uint16_t *)sbuff1;
        	else flacbuff=(uint16_t *)sbuff2;
        	if(CurrentlyPlaying == P_WAV && mono){
				HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, flacbuff[ppos]);
				HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, flacbuff[ppos++]);
        	} else {
				if(ppos<bcount[swingbuf]){
					HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, flacbuff[ppos++]);
					HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, flacbuff[ppos++]);
				}
        	}
        	if(ppos==bcount[swingbuf]){
        		int psave=ppos;
        		bcount[swingbuf]=0;
        		ppos=0;
        		if(swingbuf==1)swingbuf=2;
        		else swingbuf=1;
        		if(bcount[swingbuf]==0 && !playreadcomplete){ //nothing ready yet so flip back
            		if(swingbuf==1){
            			swingbuf=2;
            			nextbuf=1;
            		}
            		else {
            			swingbuf=1;
            			nextbuf=2;
            		}
            		bcount[swingbuf]=psave;
            		ppos=0;
        		}
        	}
        }
    } else if(CurrentlyPlaying == P_MOD ) {
    	int32_t c1=0,c2=0,c3=0,c4=0;
        if(swingbuf){ //buffer is primed
        	if(swingbuf==1)flacbuff=(uint16_t *)sbuff1;
        	else flacbuff=(uint16_t *)sbuff2;
        	if(ppos<bcount[swingbuf]){
        		c1=flacbuff[ppos++];
        		c2=flacbuff[ppos++];
        		if(CurrentlyPlayinge == P_NOTHING){
        			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, ((uint16_t)(c1+32768))>>4);
        			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, ((uint16_t)(c2+32768))>>4);
        		}
        	}
        	if(ppos==bcount[swingbuf]){
        		int psave=ppos;
        		bcount[swingbuf]=0;
        		ppos=0;
        		if(swingbuf==1)swingbuf=2;
        		else swingbuf=1;
        		if(bcount[swingbuf]==0){ //nothing ready yet so flip back
            		if(swingbuf==1){
            			swingbuf=2;
            			nextbuf=1;
            		}
            		else {
            			swingbuf=1;
            			nextbuf=2;
            		}
            		bcount[swingbuf]=psave;
            		ppos=0;
        		}
        	}
        }
        if(CurrentlyPlayinge == P_WAV){
    		if(swingbufe==1)flacbuff=(uint16_t *)sbuff1e;
    		else flacbuff=(uint16_t *)sbuff2e;
    		if(ppose<bcounte[swingbufe]){
    			c3=(mono ? flacbuff[ppose] : flacbuff[ppose++]);
    			c4=flacbuff[ppose++];
    			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R,((uint16_t)(c3+c1+32768))>>4);
    			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R,((uint16_t)(c4+c2+32768))>>4);
    		}
        	if(ppose==bcounte[swingbufe]){
        		int psave=ppose;
        		bcounte[swingbufe]=0;
        		ppose=0;
        		if(swingbufe==1)swingbufe=2;
        		else swingbufe=1;
        		if(bcounte[swingbufe]==0 && !playreadcompletee){ //nothing ready yet so flip back
            		if(swingbufe==1){
            			swingbufe=2;
            			nextbufe=1;
            		}
            		else {
            			swingbufe=1;
            			nextbufe=2;
            		}
            		bcounte[swingbufe]=psave;
            		ppose=0;
        		}
        	}
        }
    } else if(CurrentlyPlaying == P_TTS) {
    	int a, b;
        if(bcount[1]==0){
        	CloseAudio(1);
            WAVcomplete = true;
        } else {
			a = sbuff1[ppos++];
			b = a;
			a = (a * mapping[vol_left]) / 2000 ;
			b = (b * mapping[vol_right]) / 2000 ;
			a <<= 4;
			b <<= 4;
			if(ppos == bcount[1])bcount[1] = 0;                       //buffer used up
			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, a);
			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, b);
        }
    } else if(CurrentlyPlaying == P_SOUND) {
    	static int noisedwellleft[MAXSOUNDS]={0}, noisedwellright[MAXSOUNDS]={0};
    	static uint32_t noiseleft[MAXSOUNDS]={0}, noiseright[MAXSOUNDS]={0};
    	int i,j;
    	int leftv=0, rightv=0;
    	for(i=0;i<MAXSOUNDS;i++){ //first update the 8 sound pointers
    		if(sound_mode_left[i]!=nulltable){
    			if(sound_mode_left[i]!=whitenoise){
    				sound_PhaseAC_left[i] = sound_PhaseAC_left[i] + sound_PhaseM_left[i];
    				if(sound_PhaseAC_left[i]>=4096.0)sound_PhaseAC_left[i]-=4096.0;
    				j = (int)sound_mode_left[i][(int)sound_PhaseAC_left[i]];
    				j= (j-2000)*mapping[sound_v_left[i]]/2000;
    				leftv+=j;
    			} else {
    				if(noisedwellleft[i]<=0){
    					noisedwellleft[i]=sound_PhaseM_left[i];
    				    HAL_RNG_GenerateRandomNumber(&hrng, &noiseleft[i]);
    				    noiseleft[i]=(float)noiseleft[i]/(float)0xFFFFFFFF*3800.0+100;
    				}
    				if(noisedwellleft[i])noisedwellleft[i]--;
    				j = (int)noiseleft[i];
    				j= (j-2000)*mapping[sound_v_left[i]]/2000;
    				leftv+=j;
    			}
    		}
    		if(sound_mode_right[i]!=nulltable){
    			if(sound_mode_right[i]!=whitenoise){
        			sound_PhaseAC_right[i] = sound_PhaseAC_right[i] + sound_PhaseM_right[i];
        			if(sound_PhaseAC_right[i]>=4096.0)sound_PhaseAC_right[i]-=4096.0;
        			j = (int)sound_mode_right[i][(int)sound_PhaseAC_right[i]];
        			j= (j-2000)*mapping[sound_v_right[i]]/2000;
        			rightv += j;
    			} else {
    				if(noisedwellright[i]<=0){
    					noisedwellright[i]=sound_PhaseM_right[i];
    				    HAL_RNG_GenerateRandomNumber(&hrng, &noiseright[i]);
    				    noiseright[i]=(float)noiseright[i]/(float)0xFFFFFFFF*3800.0+100;
    				}
    				if(noisedwellright[i])noisedwellright[i]--;
    				j = (int)noiseright[i];
    				j= (j-2000)*mapping[sound_v_right[i]]/2000;
    				rightv+=j;
    			}
    		}
    	}
		leftv+=2000;
		rightv+=2000;
    	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint16_t)leftv);
    	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint16_t)rightv);
    } else {
        // play must be paused
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2000);
        HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2000);
    }
}


/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
