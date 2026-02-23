/***********************************************************************************************************************
MMBasic

External.c

Handles reading and writing to the digital and analog input/output pins ising the SETPIN and PIN commands

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

#include "MMBasic_Includes.h"
#define DEFINE_PINDEF_TABLE

#include "Hardware_Includes.h"

/* Definition for ADCx clock resources */
#define TRIGGEREDMODE                   ADC_TRIGGEREDMODE_SINGLE_TRIGGER   /* A single trigger for all channel oversampled conversions */
#define OVERSAMPLINGSTOPRESET           ADC_REGOVERSAMPLING_CONTINUED_MODE /* Oversampling buffer maintained during injection sequence */

int ExtCurrentConfig[NBRPINS+1];
volatile unsigned char ADCbits[NBRPINS+1];
volatile int INT1Count, INT1Value, INT1InitTimer, INT1Timer;
volatile int INT2Count, INT2Value, INT2InitTimer, INT2Timer;
volatile int INT3Count, INT3Value, INT3InitTimer, INT3Timer;
volatile int INT4Count, INT4Value, INT4InitTimer, INT4Timer;
volatile uint64_t INT5Count, INT5Value, INT5InitTimer, INT5Timer;
GPIO_InitTypeDef GPIO_InitDef, IR_InitDef;
int InterruptUsed;
extern TIM_HandleTypeDef htim5;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern void MX_TIM2_Init1(void);
extern TIM_HandleTypeDef htim2;
extern volatile uint64_t Count5High;
extern void dacclose(void);
extern void ADCclose(void);
extern uint32_t ticks_per_microsecond;
extern char *FrameInterrupt;
extern TIM_HandleTypeDef htim17;
extern void MNInterrupt(void);
void WS2812(char *q);
extern void CallCFuncInt1(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt1;                                      // we should call the CFunction Int2 function if this is non zero
extern void CallCFuncInt2(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt2;                                      // we should call the CFunction Int2 function if this is non zero

extern void CallCFuncInt3(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt3;                                      // we should call the CFunction Int3 function if this is non zero
extern void CallCFuncInt4(void);                                    // this is implemented in CFunction.c
extern unsigned int CFuncInt4;                                      // we should call the CFunction Int4 function if this is non zero

extern long long int GetuSec(void);

extern volatile uint64_t FastTimer;
char *CountInterrupt=NULL;
uint64_t lastCount5High=0;
static void MX_TIM17_Init(int scale)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = (SystemCoreClock/40000000) * scale -1;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 50000;
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

void (cmd_sync)(void){
	uint64_t i;
    static uint64_t synctime=0,endtime=0;
	getargs(&cmdline,3,",");
	if(synctime && argc==0){
        while(GetuSec()<endtime){
            if(synctime-GetuSec()> 2000)CheckAbort();
        }
        endtime+=synctime;
	} else {
		if(argc==0)error("sync not initialised");
		i=getint(argv[0],0,0x7FFFFFFFFFFFFFFF);
		if(i){
			if(argc==3){
				if(checkstring(argv[2],"U")){
					i *= 1;
				} else if(checkstring(argv[2],"M")){
					i *= 1000;
				} else if(checkstring(argv[2],"S")){
					i *= 1000000;
				}
            }
            synctime=i;
            endtime=GetuSec()+synctime;
		} else {
			synctime=endtime=0;
		}
	}
}

const MMFLOAT ADCdiv[17]={0,0,0,0,0,0,0,0,(MMFLOAT)0xFF00,0,(MMFLOAT)0xFFC0,0,(MMFLOAT)0xFFF0,0,(MMFLOAT)0xFFFC,0,(MMFLOAT)0xFFFF};
int ADC_init(int32_t pin, int fast){
	int ADCinuse;
	  /*##-1- Configure the ADC peripheral #######################################*/
	  if(pin>0){
		  if(PinDef[pin].ADC==ADC1)  {
			  ADCinuse=1;
				  HAL_ADC_DeInit(&hadc1);
				  if(ADCbits[pin]==8){
					  hadc1.Init.Resolution  = ADC_RESOLUTION_8B;
					  hadc1.Init.Oversampling.Ratio                 = 2;    /* Oversampling ratio */
					  hadc1.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc1.Init.LeftBitShift						= ADC_LEFTBITSHIFT_7;
				  }
				  else if(ADCbits[pin]==10){
					  hadc1.Init.Resolution  = ADC_RESOLUTION_10B;
					  hadc1.Init.Oversampling.Ratio                 = 8;    /* Oversampling ratio */
					  hadc1.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc1.Init.LeftBitShift						= ADC_LEFTBITSHIFT_3;
				  }
				  else if(ADCbits[pin]==12){
					  hadc1.Init.Resolution  = ADC_RESOLUTION_12B;
					  hadc1.Init.Oversampling.Ratio                 = 16;    /* Oversampling ratio */
					  hadc1.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc1.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
				  else if(ADCbits[pin]==14){
					  hadc1.Init.Resolution  = ADC_RESOLUTION_14B;
					  hadc1.Init.Oversampling.Ratio                 = 32;    /* Oversampling ratio */
					  hadc1.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_3;         /* Right shift of the oversampled summation */
					  hadc1.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
				  else if(ADCbits[pin]==16){
					  hadc1.Init.Resolution  = ADC_RESOLUTION_16B;
					  hadc1.Init.Oversampling.Ratio                 = 64;    /* Oversampling ratio */
					  hadc1.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_6;         /* Right shift of the oversampled summation */
					  hadc1.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
		  } else if(PinDef[pin].ADC==ADC2) {
			  ADCinuse=2;
				  HAL_ADC_DeInit(&hadc2);
				  if(ADCbits[pin]==8){
					  hadc2.Init.Resolution  = ADC_RESOLUTION_8B;
					  hadc2.Init.Oversampling.Ratio                 = 2;    /* Oversampling ratio */
					  hadc2.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc2.Init.LeftBitShift						= ADC_LEFTBITSHIFT_7;
				  }
				  else if(ADCbits[pin]==10){
					  hadc2.Init.Resolution  = ADC_RESOLUTION_10B;
					  hadc2.Init.Oversampling.Ratio                 = 8;    /* Oversampling ratio */
					  hadc2.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc2.Init.LeftBitShift						= ADC_LEFTBITSHIFT_3;
				  }
				  else if(ADCbits[pin]==12){
					  hadc2.Init.Resolution  = ADC_RESOLUTION_12B;
					  hadc2.Init.Oversampling.Ratio                 = 16;    /* Oversampling ratio */
					  hadc2.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc2.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
				  else if(ADCbits[pin]==14){
					  hadc2.Init.Resolution  = ADC_RESOLUTION_14B;
					  hadc2.Init.Oversampling.Ratio                 = 32;    /* Oversampling ratio */
					  hadc2.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_3;         /* Right shift of the oversampled summation */
					  hadc2.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
				  else if(ADCbits[pin]==16){
					  hadc2.Init.Resolution  = ADC_RESOLUTION_16B;
					  hadc2.Init.Oversampling.Ratio                 = 64;    /* Oversampling ratio */
					  hadc2.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_6;         /* Right shift of the oversampled summation */
					  hadc2.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
		  } else {
			  ADCinuse=3;
				  HAL_ADC_DeInit(&hadc3);
				  if(ADCbits[pin]==8){
					  hadc3.Init.Resolution  = ADC_RESOLUTION_8B;
					  hadc3.Init.Oversampling.Ratio                 = 2;    /* Oversampling ratio */
					  hadc3.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc3.Init.LeftBitShift						= ADC_LEFTBITSHIFT_7;
				  }
				  else if(ADCbits[pin]==10){
					  hadc3.Init.Resolution  = ADC_RESOLUTION_10B;
					  hadc3.Init.Oversampling.Ratio                 = 8;    /* Oversampling ratio */
					  hadc3.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc3.Init.LeftBitShift						= ADC_LEFTBITSHIFT_3;
				  }
				  else if(ADCbits[pin]==12){
					  hadc3.Init.Resolution  = ADC_RESOLUTION_12B;
					  hadc3.Init.Oversampling.Ratio                 = 16;    /* Oversampling ratio */
					  hadc3.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_NONE;         /* Right shift of the oversampled summation */
					  hadc3.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
				  else if(ADCbits[pin]==14){
					  hadc3.Init.Resolution  = ADC_RESOLUTION_14B;
					  hadc3.Init.Oversampling.Ratio                 = 32;    /* Oversampling ratio */
					  hadc3.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_3;         /* Right shift of the oversampled summation */
					  hadc3.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
				  else if(ADCbits[pin]==16){
					  hadc3.Init.Resolution  = ADC_RESOLUTION_16B;
					  hadc3.Init.Oversampling.Ratio                 = 64;    /* Oversampling ratio */
					  hadc3.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_6;         /* Right shift of the oversampled summation */
					  hadc3.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
				  }
			  }
	  } else if(pin==0) {
		  ADCinuse=2;
			  hadc2.Init.Resolution  = ADC_RESOLUTION_16B;
			  hadc2.Init.Oversampling.Ratio                 = 256;    /* Oversampling ratio */
			  hadc2.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_8;         /* Right shift of the oversampled summation */
			  hadc2.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
	  } else {
		  ADCinuse=3;
			  hadc3.Init.Resolution  = ADC_RESOLUTION_16B;
			  hadc3.Init.Oversampling.Ratio                 = 256;    /* Oversampling ratio */
			  hadc3.Init.Oversampling.RightBitShift         = ADC_RIGHTBITSHIFT_8;         /* Right shift of the oversampled summation */
			  hadc3.Init.LeftBitShift						= ADC_LEFTBITSHIFT_NONE;
	  }
	  if(ADCinuse==1){
		  hadc1.Instance = ADC1;
		  hadc1.Init.ClockPrescaler           = (fast || ADCbits[pin]==8 ? ADC_CLOCK_SYNC_PCLK_DIV4 : ADC_CLOCK_ASYNC_DIV6);      /* Synchronous clock mode, input ADC clock divided by 4*/
		  hadc1.Init.ScanConvMode             = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
		  hadc1.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
		  hadc1.Init.LowPowerAutoWait         = DISABLE;                       /* Auto-delayed conversion feature disabled */
		  hadc1.Init.ContinuousConvMode       = DISABLE;                        /* Continuous mode enabled (automatic conversion restart after each conversion) */
		  hadc1.Init.NbrOfConversion          = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc1.Init.DiscontinuousConvMode    = DISABLE;                       /* Parameter discarded because sequencer is disabled */
		  hadc1.Init.NbrOfDiscConversion      = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
		  hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
		  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;         /* DR mode selected */
		  hadc1.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;      /* DR register is overwritten with the last conversion result in case of overrun */
		  hadc1.Init.OversamplingMode         = ENABLE;                        /* Oversampling enabled */
		  hadc1.Init.Oversampling.TriggeredMode         = ADC_TRIGGEREDMODE_SINGLE_TRIGGER ;         /* Specifies whether or not a trigger is needed for each sample */
		  hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE ; /* Specifies whether or not the oversampling buffer is maintained during injection sequence */
		  if (HAL_ADC_Init(&hadc1) != HAL_OK)
		  {
			  /* ADC initialization Error */
			  error("HAL_ADC_Init");
		  }


		  /* Run the ADC calibration in single-ended mode */
		  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
		  {
			  /* Calibration Error */
			  error("HAL_ADCEx_Calibration_Start");
		  }
	  } else if(ADCinuse==2){
		  hadc2.Instance = ADC2;
		  hadc2.Init.ClockPrescaler           = (fast || ADCbits[pin]==8 ? ADC_CLOCK_SYNC_PCLK_DIV4 : ADC_CLOCK_ASYNC_DIV6);      /* Synchronous clock mode, input ADC clock divided by 4*/
		  hadc2.Init.ScanConvMode             = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
		  hadc2.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
		  hadc2.Init.LowPowerAutoWait         = DISABLE;                       /* Auto-delayed conversion feature disabled */
		  hadc2.Init.ContinuousConvMode       = DISABLE;                        /* Continuous mode enabled (automatic conversion restart after each conversion) */
		  hadc2.Init.NbrOfConversion          = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc2.Init.DiscontinuousConvMode    = DISABLE;                       /* Parameter discarded because sequencer is disabled */
		  hadc2.Init.NbrOfDiscConversion      = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc2.Init.ExternalTrigConv         = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
		  hadc2.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
		  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;         /* DR mode selected */
		  hadc2.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;      /* DR register is overwritten with the last conversion result in case of overrun */
		  hadc2.Init.OversamplingMode         = ENABLE;                        /* Oversampling enabled */
		  hadc2.Init.Oversampling.TriggeredMode         = ADC_TRIGGEREDMODE_SINGLE_TRIGGER ;         /* Specifies whether or not a trigger is needed for each sample */
		  hadc2.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE ; /* Specifies whether or not the oversampling buffer is maintained during injection sequence */
		  if (HAL_ADC_Init(&hadc2) != HAL_OK)
		  {
			  /* ADC initialization Error */
			  error("HAL_ADC_Init");
		  }


		  /* Run the ADC calibration in single-ended mode */
		  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
		  {
			  /* Calibration Error */
			  error("HAL_ADCEx_Calibration_Start");
		  }
	  }	else if(ADCinuse==3 ){
		  hadc3.Instance = ADC3;
		  hadc3.Init.ClockPrescaler           = (fast || ADCbits[pin]==8 ? ADC_CLOCK_SYNC_PCLK_DIV4 : ADC_CLOCK_ASYNC_DIV6);      /* Synchronous clock mode, input ADC clock divided by 4*/
		  hadc3.Init.ScanConvMode             = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
		  hadc3.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
		  hadc3.Init.LowPowerAutoWait         = DISABLE;                       /* Auto-delayed conversion feature disabled */
		  hadc3.Init.ContinuousConvMode       = DISABLE;                        /* Continuous mode enabled (automatic conversion restart after each conversion) */
		  hadc3.Init.NbrOfConversion          = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc3.Init.DiscontinuousConvMode    = DISABLE;                       /* Parameter discarded because sequencer is disabled */
		  hadc3.Init.NbrOfDiscConversion      = 1;                             /* Parameter discarded because sequencer is disabled */
		  hadc3.Init.ExternalTrigConv         = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
		  hadc3.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
		  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;         /* DR mode selected */
		  hadc3.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;      /* DR register is overwritten with the last conversion result in case of overrun */
		  hadc3.Init.OversamplingMode         = ENABLE;                        /* Oversampling enabled */
		  hadc3.Init.Oversampling.TriggeredMode         = ADC_TRIGGEREDMODE_SINGLE_TRIGGER ;         /* Specifies whether or not a trigger is needed for each sample */
		  hadc3.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE ; /* Specifies whether or not the oversampling buffer is maintained during injection sequence */
		  if (HAL_ADC_Init(&hadc3) != HAL_OK)
		  {
			  /* ADC initialization Error */
			  error("HAL_ADC_Init");
		  }


		  /* Run the ADC calibration in single-ended mode */
		  if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
		  {
			  /* Calibration Error */
			  error("HAL_ADCEx_Calibration_Start");
		  }
	  }
	  if(pin>0){

		  GPIO_InitTypeDef GPIO_InitStruct;
		  /*##-2- Configure peripheral GPIO ##########################################*/
		  /* ADC Channel GPIO pin configuration */
		  GPIO_InitStruct.Pin = PinDef[pin].bitnbr;
		  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitStruct);
	  }
	  return ADCinuse;
}


extern inline void PinSetBit(int pin, unsigned int offset) {
	GPIO_InitTypeDef GPIO_InitDef;
	switch (offset){
	case LATCLR:
		PinDef[pin].sfr->BSRR = (PinDef[pin].bitnbr)<<16;return;
	case LATSET:
		PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr;return;
	case LATINV:
		PinDef[pin].sfr->ODR ^= PinDef[pin].bitnbr;return;
	case TRISSET:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	case TRISCLR:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as output with no pullup or down
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	case CNPUSET:
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Pull = GPIO_PULLUP; //set as input with pullup
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	    return;
	case CNPDSET:
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Pull = GPIO_PULLDOWN; //set as input with pulldown
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
	    return;
	case CNPUCLR:
	case CNPDCLR:
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
		return;
	case ODCCLR:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as push-pull output
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
		return;
	case ODCSET:
		GPIO_InitDef.Pull = GPIO_NOPULL; //set as output with open drain
		GPIO_InitDef.Pin = PinDef[pin].bitnbr;
		GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
		return;
	default: error("Unknown PinSetBit command");
	}
}
// return the value of a pin's input
int PinRead(int pin) {
    return  PinDef[pin].sfr->IDR & PinDef[pin].bitnbr? 1: 0;
}
// return a pointer to the pin's sfr register
volatile unsigned int GetPinStatus(int pin) {
    return (PinDef[pin].sfr->ODR>>PinDef[pin].bitnbr) & 1;
}
// return an integer representing the bit number in the sfr corresponding to the pin's bit
int GetPinBit(int pin) {
    return PinDef[pin].bitnbr;
}
void WriteCoreTimer(unsigned long timeset){
	FastTimer+=__HAL_TIM_GET_COUNTER(&htim5);
	__HAL_TIM_SET_COUNTER(&htim5, timeset);
}
unsigned long ReadCoreTimer(void){
	return __HAL_TIM_GET_COUNTER(&htim5);
}
unsigned long readusclock(void){
	return __HAL_TIM_GET_COUNTER(&htim5)/ticks_per_microsecond;
}
void writeusclock(unsigned long timeset){
	FastTimer+=__HAL_TIM_GET_COUNTER(&htim5);
	__HAL_TIM_SET_COUNTER(&htim5, timeset*ticks_per_microsecond);
}
unsigned long ReadCount5(void){
	return __HAL_TIM_GET_COUNTER(&htim2);
}
void WriteCount5(unsigned long timeset){
	__HAL_TIM_SET_COUNTER(&htim2, timeset);
}

/*******************************************************************************************
External I/O related commands in MMBasic
========================================
These are the functions responsible for executing the ext I/O related  commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

Each function is responsible for decoding a command
all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a
basic function) so, if you want to search for the function responsible for the LOCATE command
look for cmd_name

There are 4 items of information that are setup before the command is run.
All these are globals.

int cmdtoken	This is the token number of the command (some commands can handle multiple
				statement types and this helps them differentiate)

char *cmdline	This is the command line terminated with a zero char and trimmed of leading
				spaces.  It may exist anywhere in memory (or even ROM).

char *nextstmt	This is a pointer to the next statement to be executed.  The only thing a
				y=spi(1,2,3command can do with it is save it or change it to some other location.

char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

The only actions a command can do to change the program flow is to change nextstmt or
execute longjmp(mark, 1) if it wants to abort the program.

********************************************************************************************/


// this is invoked as a command (ie, pin(3) = 1)
// first get the argument then step over the closing bracket.  Search through the rest of the command line looking
// for the equals sign and step over it, evaluate the rest of the command and set the pin accordingly
void cmd_pin(void) {
	int pin;
    long long int value;
	pin = getinteger(cmdline);
    if(IsInvalidPin(pin)) error("Invalid pin");
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Invalid syntax");
	++cmdline;
	if(!*cmdline) error("Invalid syntax");
	value = getinteger(cmdline);
    if(ExtCurrentConfig[pin] == EXT_NOT_CONFIG || ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)
		PinDef[pin].sfr->BSRR = (PinDef[pin].bitnbr)<<(value ? 0:16);
		else ExtSet(pin, value);
}



// this is invoked as a function (ie, x = pin(3) )
void fun_pin(void) {
	int ADCinuse;
    MMFLOAT f;
    long long int i64;
    char *ss;
    int t=0;
    evaluate(ep, &f, &i64, &ss, &t, false);
	ADC_ChannelConfTypeDef sConfig;
	int pin;
	if(t & T_STR){
		ss=MtoC(ss);
		if(strcasecmp(ss, "DAC1")==0){
			ADC_init(0 ,0);
		      sConfig.Channel      = ADC_CHANNEL_DAC1CH1_ADC2;             /* Sampled channel number */
			  sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
			  sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;    /* Sampling time (number of clock cycles unit) */
			  sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
			  sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
			  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


			  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
			  {
			    /* Channel Configuration Error */
				    error("HAL_ADC_ConfigChannel");
			  }
		    if (HAL_ADC_Start(&hadc2) != HAL_OK)
		    {
		      /* Start Conversation Error */
		      error("HAL_ADC_Start");
		    }

		    /*##-4- Wait for the end of conversion #####################################*/
		    /*  For simplicity reasons, this example is just waiting till the end of the
		        conversion, but application may perform other tasks while conversion
		        operation is ongoing. */
		    if (HAL_ADC_PollForConversion(&hadc2, 50) != HAL_OK)
		    {
		      /* End Of Conversion flag not set on time */
		        error("HAL_ADC_PollForConversion");
		    }
		    else
		    {
		      /* ADC conversion completed */
		      /*##-5- Get the converted value of regular channel  ########################*/
    			fret=(double)(HAL_ADC_GetValue(&hadc2))/ADCdiv[16] * VCC;
		    }
			if (HAL_ADC_DeInit(&hadc2) != HAL_OK)
			{
			    /* ADC de-initialization Error */
			    error("HAL_ADC_DeInit");
			}


		    targ = T_NBR;
		    return;
		} else if(strcasecmp(ss, "DAC2")==0){
			ADC_init(0 ,0);
		      sConfig.Channel      = ADC_CHANNEL_DAC1CH2_ADC2;             /* Sampled channel number */
			  sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
			  sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;    /* Sampling time (number of clock cycles unit) */
			  sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
			  sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
			  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


			  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
			  {
			    /* Channel Configuration Error */
				    error("HAL_ADC_ConfigChannel");
			  }
		    if (HAL_ADC_Start(&hadc2) != HAL_OK)
		    {
		      /* Start Conversation Error */
		      error("HAL_ADC_Start");
		    }

		    /*##-4- Wait for the end of conversion #####################################*/
		    /*  For simplicity reasons, this example is just waiting till the end of the
		        conversion, but application may perform other tasks while conversion
		        operation is ongoing. */
		    if (HAL_ADC_PollForConversion(&hadc2, 50) != HAL_OK)
		    {
		      /* End Of Conversion flag not set on time */
		        error("HAL_ADC_PollForConversion");
		    }
		    else
		    {
		      /* ADC conversion completed */
		      /*##-5- Get the converted value of regular channel  ########################*/
    			fret=(double)(HAL_ADC_GetValue(&hadc2))/ADCdiv[16] * VCC;
		    }

			if (HAL_ADC_DeInit(&hadc2) != HAL_OK)
			{
			    /* ADC de-initialization Error */
			    error("HAL_ADC_DeInit");
			}

		    targ = T_NBR;
		    return;
		} else if(strcasecmp(ss, "BAT")==0){
			ADC_init(-1 ,0);
		      sConfig.Channel      = ADC_CHANNEL_VBAT_DIV4;             /* Sampled channel number */
			  sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
			  sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;    /* Sampling time (number of clock cycles unit) */
			  sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
			  sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
			  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


			  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
			  {
			    /* Channel Configuration Error */
				    error("HAL_ADC_ConfigChannel");
			  }
		    if (HAL_ADC_Start(&hadc3) != HAL_OK)
		    {
		      /* Start Conversation Error */
		      error("HAL_ADC_Start");
		    }

		    /*##-4- Wait for the end of conversion #####################################*/
		    /*  For simplicity reasons, this example is just waiting till the end of the
		        conversion, but application may perform other tasks while conversion
		        operation is ongoing. */
		    if (HAL_ADC_PollForConversion(&hadc3, 50) != HAL_OK)
		    {
		      /* End Of Conversion flag not set on time */
		        error("HAL_ADC_PollForConversion");
		    }
		    else
		    {
		      /* ADC conversion completed */
		      /*##-5- Get the converted value of regular channel  ########################*/
    			fret=(double)(HAL_ADC_GetValue(&hadc3))/ADCdiv[16] * VCC * (double)4.0;
		    }
			if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
			{
			    /* ADC de-initialization Error */
			    error("HAL_ADC_DeInit");
			}


		    targ = T_NBR;
		    return;
		} else if(strcasecmp(ss, "TEMP")==0){
			uint16_t *TS_CAL1=(uint16_t *)0x1FF1E820;
			uint16_t *TS_CAL2=(uint16_t *)0x1FF1E840;
			ADC_init(-1 ,0);
		      sConfig.Channel      = ADC_CHANNEL_TEMPSENSOR;             /* Sampled channel number */
			  sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
			  sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;    /* Sampling time (number of clock cycles unit) */
			  sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
			  sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
			  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */

			  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
			  {
			    /* Channel Configuration Error */
				    error("HAL_ADC_ConfigChannel");
			  }
		    if (HAL_ADC_Start(&hadc3) != HAL_OK)
		    {
		      /* Start Conversation Error */
		      error("HAL_ADC_Start");
		    }

		    /*##-4- Wait for the end of conversion #####################################*/
		    /*  For simplicity reasons, this example is just waiting till the end of the
		        conversion, but application may perform other tasks while conversion
		        operation is ongoing. */
		    if (HAL_ADC_PollForConversion(&hadc3, 50) != HAL_OK)
		    {
		      /* End Of Conversion flag not set on time */
		        error("HAL_ADC_PollForConversion");
		    }
		    else
		    {
		      /* ADC conversion completed */
		      /*##-5- Get the converted value of regular channel  ########################*/
		      fret = ((double)(80.0)) / ((double)(*TS_CAL2-*TS_CAL1)) * (((double)(HAL_ADC_GetValue(&hadc3)))-((double)*TS_CAL1)) + ((double)30.0) ;
		    }
			if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
			{
			    /* ADC de-initialization Error */
			    error("HAL_ADC_DeInit");
			}


		    targ = T_NBR;
		    return;
		} else if(strcasecmp(ss, "SREF")==0){
			uint16_t *VREF=(uint16_t *)0x1FF1E860;
			fret=(MMFLOAT)*VREF/(MMFLOAT)0xFFFF * (MMFLOAT)3.3;
		    targ = T_NBR;
			return;
		} else if(strcasecmp(ss, "IREF")==0){
			  ADC_init(-1 ,0);
		      sConfig.Channel      = ADC_CHANNEL_VREFINT;             /* Sampled channel number */
			  sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
			  sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;    /* Sampling time (number of clock cycles unit) */
			  sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
			  sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
			  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


			  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
			  {
			    /* Channel Configuration Error */
				    error("HAL_ADC_ConfigChannel");
			  }
		    if (HAL_ADC_Start(&hadc3) != HAL_OK)
		    {
		      /* Start Conversation Error */
		      error("HAL_ADC_Start");
		    }

		    /*##-4- Wait for the end of conversion #####################################*/
		    /*  For simplicity reasons, this example is just waiting till the end of the
		        conversion, but application may perform other tasks while conversion
		        operation is ongoing. */
		    if (HAL_ADC_PollForConversion(&hadc3, 50) != HAL_OK)
		    {
		      /* End Of Conversion flag not set on time */
		        error("HAL_ADC_PollForConversion");
		    }
		    else
		    {
		      /* ADC conversion completed */
		      /*##-5- Get the converted value of regular channel  ########################*/
    			fret=(double)(HAL_ADC_GetValue(&hadc3))/ADCdiv[16] * VCC;
		    }

			if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
			{
			    /* ADC de-initialization Error */
			    error("HAL_ADC_DeInit");
			}

		    targ = T_NBR;
		    return;
		} else error("Syntax");
	} else {
		if(t & T_INT) pin = i64;
		else pin = FloatToInt32(f);
        if(IsInvalidPin(pin)) error("Invalid pin");
        switch(ExtCurrentConfig[pin]) {
            case EXT_DIG_IN:
            					iret = PinDef[pin].sfr->IDR & PinDef[pin].bitnbr? 1: 0;
            					targ = T_INT;
            					return;
            case EXT_CNT_IN:
            case EXT_INT_HI:
            case EXT_INT_LO:
            case EXT_INT_BOTH:
            case EXT_DIG_OUT:
            case EXT_OC_OUT:    iret = ExtInp(pin);
                                targ = T_INT;
                                return;
            case EXT_PER_IN:	// if period measurement get the count and average it over the number of cycles
                                if(pin == INT1PIN) fret = (MMFLOAT)ExtInp(pin) / INT1InitTimer;
                                else if(pin == INT2PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT2InitTimer;
                                else if(pin == INT3PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT3InitTimer;
                                else if(pin == INT4PIN)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT4InitTimer;
                                else if(pin == COUNT5)  fret = (MMFLOAT)ExtInp(pin) / (MMFLOAT)INT5InitTimer;
                                targ = T_NBR;
                                return;
            case EXT_FREQ_IN:	// if frequency measurement get the count and scale the reading
                                if(pin == INT1PIN) fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT1InitTimer;
                                else if(pin == INT2PIN)  fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT2InitTimer;
                                else if(pin == INT3PIN)  fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT3InitTimer;
                                else if(pin == INT4PIN)  fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT4InitTimer;
                                else if(pin == COUNT5)  fret = (MMFLOAT)(ExtInp(pin)) * (MMFLOAT)1000.0 / (MMFLOAT)INT5InitTimer;
                                 targ = T_NBR;
                                return;
            case EXT_ANA_IN:    break;
            default:            error("Pin is not an input");
        }

		/*##-2- Configure ADC regular channel ######################################*/
	    ADCinuse=ADC_init(pin ,0);
	    if(ADCinuse==1){
	    	sConfig.Channel      = PinDef[pin].ADCchannel;                /* Sampled channel number */
	    	sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
	    	sConfig.SamplingTime = (ADCbits[pin]<10 ? ADC_SAMPLETIME_2CYCLES_5 : (ADCbits[pin]<12 ? ADC_SAMPLETIME_8CYCLES_5 : ADC_SAMPLETIME_64CYCLES_5));    /* Sampling time (number of clock cycles unit) */
	    	sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
	    	sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
	    	sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


	    	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	    	{
	    		/* Channel Configuration Error */
			    error("HAL_ADC_ConfigChannel");
	    	}
	    	if (HAL_ADC_Start(&hadc1) != HAL_OK)
	    	{
	    		/* Start Conversation Error */
	    		error("HAL_ADC_Start");
	    	}

	    	/*##-4- Wait for the end of conversion #####################################*/
	    	/*  For simplicity reasons, this example is just waiting till the end of the
	        	conversion, but application may perform other tasks while conversion
	        	operation is ongoing. */
	    	if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
	    	{
	    		/* End Of Conversion flag not set on time */
	    		error("HAL_ADC_PollForConversion");
	    	}
	    	else
	    	{
	    		/* ADC conversion completed */
	    		/*##-5- Get the converted value of regular channel  ########################*/
				fret=(double)(HAL_ADC_GetValue(&hadc1))/ADCdiv[ADCbits[pin]] * VCC;
	    	}
	    	if (HAL_ADC_DeInit(&hadc1) != HAL_OK)
	    	{
	    		/* ADC de-initialization Error */
	    		error("HAL_ADC_DeInit");
	    	}

	    	targ = T_NBR;
	    	return;
	    } else if(ADCinuse==2){
	    	sConfig.Channel      = PinDef[pin].ADCchannel;                /* Sampled channel number */
	    	sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
	    	sConfig.SamplingTime = (ADCbits[pin]<10 ? ADC_SAMPLETIME_2CYCLES_5 : (ADCbits[pin]<12 ? ADC_SAMPLETIME_8CYCLES_5 : ADC_SAMPLETIME_64CYCLES_5));    /* Sampling time (number of clock cycles unit) */
	    	sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
	    	sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
	    	sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


	    	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
	    	{
	    		/* Channel Configuration Error */
	  		    error("HAL_ADC_ConfigChannel");
	    	}
	    	if (HAL_ADC_Start(&hadc2) != HAL_OK)
	    	{
	    		/* Start Conversation Error */
	    		error("HAL_ADC_Start");
	    	}

	    	/*##-4- Wait for the end of conversion #####################################*/
	    	/*  For simplicity reasons, this example is just waiting till the end of the
	          	  conversion, but application may perform other tasks while conversion
	          	  operation is ongoing. */
	    	if (HAL_ADC_PollForConversion(&hadc2, 10) != HAL_OK)
	    	{
	    		/* End Of Conversion flag not set on time */
	    		error("HAL_ADC_PollForConversion");
	    	}
	    	else
	    	{
	    		/* ADC conversion completed */
	    		/*##-5- Get the converted value of regular channel  ########################*/
				fret=(double)(HAL_ADC_GetValue(&hadc2))/ADCdiv[ADCbits[pin]] * VCC;
	    	}
	    	if (HAL_ADC_DeInit(&hadc2) != HAL_OK)
	    	{
	    		/* ADC de-initialization Error */
	    		error("HAL_ADC_DeInit");
	    	}

	    	targ = T_NBR;
	    	return;
    	} else {
    		sConfig.Channel      = PinDef[pin].ADCchannel;                /* Sampled channel number */
    		sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
    		sConfig.SamplingTime = (ADCbits[pin]<10 ? ADC_SAMPLETIME_2CYCLES_5 : (ADCbits[pin]<12 ? ADC_SAMPLETIME_8CYCLES_5 : ADC_SAMPLETIME_64CYCLES_5));     /* Sampling time (number of clock cycles unit) */
    		sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
    		sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
    		sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


    		if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    		{
    			/* Channel Configuration Error */
    			error("HAL_ADC_ConfigChannel");
    		}
    		if (HAL_ADC_Start(&hadc3) != HAL_OK)
    		{
    			/* Start Conversation Error */
    			error("HAL_ADC_Start");
    		}

    		/*##-4- Wait for the end of conversion #####################################*/
    		/*  For simplicity reasons, this example is just waiting till the end of the
          	  conversion, but application may perform other tasks while conversion
          	  operation is ongoing. */
    		if (HAL_ADC_PollForConversion(&hadc3, 10) != HAL_OK)
    		{
    			/* End Of Conversion flag not set on time */
    			error("HAL_ADC_PollForConversion");
    		}
    		else
    		{
    			/* ADC conversion completed */
    			/*##-5- Get the converted value of regular channel  ########################*/
    			fret=(double)(HAL_ADC_GetValue(&hadc3))/ADCdiv[ADCbits[pin]] * VCC;
    		}
    		if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
    		{
    			/* ADC de-initialization Error */
    			error("HAL_ADC_DeInit");
    		}

    		targ = T_NBR;
    		return;
    	}
	}
}



// this is invoked as a command (ie, port(3, 8) = Value)
// first get the arguments then step over the closing bracket.  Search through the rest of the command line looking
// for the equals sign and step over it, evaluate the rest of the command and set the pins accordingly
void cmd_port(void) {
	int pin, nbr, value;
    int i;
	getargs(&cmdline, NBRPINS * 4, ",");

	if((argc & 0b11) != 0b11) error("Invalid syntax");

    // step over the equals sign and get the value for the assignment
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Invalid syntax");
	++cmdline;
	if(!*cmdline) error("Invalid syntax");
	value = getinteger(cmdline);

    for(i = 0; i < argc; i += 4) {
        pin = getinteger(argv[i]);
        nbr = getinteger(argv[i + 2]);

        if(nbr < 0 || pin <= 0) error("Invalid argument");

        while(nbr) {
            if(IsInvalidPin(pin) || !(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Invalid output pin");
            ExtSet(pin, value & 1);
            value >>= 1;
            nbr--;
            pin++;
        }
    }
}



// this is invoked as a function (ie, x = port(10,8) )
void fun_port(void) {
	int pin, nbr, i, value = 0;

	getargs(&ep, NBRPINS * 4, ",");
	if((argc & 0b11) != 0b11) error("Invalid syntax");

    for(i = argc - 3; i >= 0; i -= 4) {
        pin = getinteger(argv[i]);
        nbr = getinteger(argv[i + 2]);

        if(nbr < 0 || pin <= 0) error("Invalid argument");
        pin += nbr - 1;                                             // we start by reading the most significant bit

        while(nbr) {
            if(IsInvalidPin(pin) || !(ExtCurrentConfig[pin] == EXT_DIG_IN || ExtCurrentConfig[pin] == EXT_INT_HI || ExtCurrentConfig[pin] == EXT_INT_LO || ExtCurrentConfig[pin] == EXT_INT_BOTH)) error("Invalid input pin");
            value <<= 1;
            value |= PinRead(pin);
            nbr--;
            pin--;
        }
    }

    iret = value;
    targ = T_INT;
}



void cmd_setpin(void) {
	int i, pin, value, option = 0;
	getargs(&cmdline, 7, ",");
	if(argc%2 == 0 || argc < 3) error("Argument count");
	pin = getinteger(argv[0]);

    if(checkstring(argv[2], "OFF") || checkstring(argv[2], "0"))
        value = EXT_NOT_CONFIG;
    else if(checkstring(argv[2], "AIN"))
        value = EXT_ANA_IN;
    else if(checkstring(argv[2], "DIN"))
        value = EXT_DIG_IN;
    else if(checkstring(argv[2], "FIN"))
        value = EXT_FREQ_IN;
    else if(checkstring(argv[2], "PIN"))
        value = EXT_PER_IN;
    else if(checkstring(argv[2], "CIN"))
        value = EXT_CNT_IN;
    else if(checkstring(argv[2], "INTH"))
        value = EXT_INT_HI;
    else if(checkstring(argv[2], "INTL"))
        value = EXT_INT_LO;
    else if(checkstring(argv[2], "DOUT"))
        value = EXT_DIG_OUT;
    else if(checkstring(argv[2], "OOUT"))
        value = EXT_OC_OUT;
    else if(checkstring(argv[2], "INTB"))
        value = EXT_INT_BOTH;
    else
        value = getint(argv[2], 1, 9);

    // check for any options
    switch(value) {
    	case EXT_ANA_IN:if(argc == 5) {
    						option = getint((argv[4]), 8, 16);
    						if(option & 1)error("Invalid bit count");
        					} else
        					option = 16;
        break;

        case EXT_DIG_IN:    if(argc == 5) {
                                if(checkstring(argv[4], "PULLUP")) option = CNPUSET;
                                else if(checkstring(argv[4], "PULLDOWN")) option = CNPDSET;
                                else error("Invalid option");
                            } else
                                option = 0;
                            break;
        case EXT_INT_HI:
        case EXT_INT_LO:
        case EXT_INT_BOTH:  if(argc == 7) {
                                if(checkstring(argv[6], "PULLUP")) option = CNPUSET;
                                else if(checkstring(argv[6], "PULLDOWN")) option = CNPDSET;
                                else error("Invalid option");
                            } else
                                option = 0;
                            break;
        case EXT_FREQ_IN:   if(argc == 5)
                                option = getint((argv[4]), 10, 100000);
                            else
                                option = 1000;
                            break;
        case EXT_CNT_IN:    if(argc == 5){
        	                   if (pin==18){
        	                	    option = (int)GetIntAddress(argv[4]);
        	                   }else{
        	                         option = getint((argv[4]), 1, 5);
        	                   }
                            }else{
                                option = 0;
                            }
                            break;
        case EXT_PER_IN:   if(argc == 5)
                                option = getint((argv[4]), 1, 10000);
                            else
                                option = 1;
                            break;
        case EXT_DIG_OUT:   if(argc == 5) {
                                if(checkstring(argv[4], "OC"))
                                    value = EXT_OC_OUT;
                            else
                                error("Invalid option");
                            }
                            break;
        default:            if(argc > 3) error("Unexpected text");
    }

	//pin = getinteger(argv[0]);
	//if(value==EXT_CNT_IN && argc==5 && pin!=18)error("interrupt only available on pin 18");
    {
        CheckPin(pin, CP_IGNORE_INUSE);
        ExtCfg(pin, value, option);
    }

	if(value == EXT_INT_HI || value == EXT_INT_LO || value == EXT_INT_BOTH) {
		// we need to set up a software interrupt
		if(argc < 5) error("Argument count");
        for(i = 0; i < NBRINTERRUPTS; i++) if(inttbl[i].pin == 0) break;
        if(i >= NBRINTERRUPTS) error("Too many interrupts");
        inttbl[i].pin = pin;
		inttbl[i].intp = GetIntAddress(argv[4]);					// get the interrupt routine's location
		inttbl[i].last = ExtInp(pin);								// save the current pin value for the first test
        switch(value) {                                             // and set trigger polarity
            case EXT_INT_HI:    inttbl[i].lohi = T_LOHI; break;
            case EXT_INT_LO:    inttbl[i].lohi = T_HILO; break;
            case EXT_INT_BOTH:  inttbl[i].lohi = T_BOTH; break;
        }
		InterruptUsed = true;
	}
}



void cmd_pulse(void) {
    int pin, i, x, y;
    MMFLOAT f;

	getargs(&cmdline, 3, ",");
	if(argc != 3) error("Invalid syntax");
	pin = getinteger(argv[0]);
	if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin is not an output");

    f = getnumber(argv[2]);                                         // get the pulse width
    if(f < 0) error("Number out of bounds");
    x = f;                                                          // get the integer portion (in mSec)
    y = (int)((f - (MMFLOAT)x) * 1000.0);                             // get the fractional portion (in uSec)

    for(i = 0; i < NBR_PULSE_SLOTS; i++)                            // search looking to see if the pin is in use
        if(PulseCnt[i] != 0 && PulsePin[i] == pin) {
            mT4IntEnable(0);       									// disable the timer interrupt to prevent any conflicts while updating
            PulseCnt[i] = x;                                        // and if the pin is in use, set its time to the new setting or reset if the user wants to terminate
            mT4IntEnable(1);
            if(x == 0) PinSetBit(PulsePin[i], LATINV);
            return;
        }

    if(x == 0 && y == 0) return;                                    // silently ignore a zero pulse width

    if(x < 3) {                                                     // if this is under 3 milliseconds just do it now
        PinSetBit(pin, LATINV);                    // starting edge of the pulse
        uSec(x * 1000 + y);
        PinSetBit(pin, LATINV);                    // finishing edge
        return;
    }

    for(i = 0; i < NBR_PULSE_SLOTS; i++)
        if(PulseCnt[i] == 0) break;                                 // find a spare slot

    if(i >= NBR_PULSE_SLOTS) error("Too many concurrent PULSE commands");

    PinSetBit(pin, LATINV);                        // starting edge of the pulse
    if(x == 1) uSec(500);                                           // prevent too narrow a pulse if there is just one count
    PulsePin[i] = pin;                                              // save the details
    PulseCnt[i] = x;
    PulseActive = true;
}


void fun_pulsin(void) { //allowas timeouts up to 10 seconds
    int pin, polarity;
    unsigned int t1, t2;

	getargs(&ep, 7, ",");
	if((argc &1) != 1 || argc < 3) error("Invalid syntax");
    pin = getinteger(argv[0]);
    if(IsInvalidPin(pin)) error("Invalid pin");
	if(ExtCurrentConfig[pin] != EXT_DIG_IN) error("Pin is not an input");
    polarity = getinteger(argv[2]);

    t1 = t2 = 100000;                                               // default timeout is 100mS
    if(argc >= 5) t1 = t2 = getint(argv[4], 5, 10000000);
    if(argc == 7) t2 = getint(argv[6], 5, 10000000);
    iret = -1;                                                      // in anticipation of a timeout
    writeusclock(0);
    if(polarity) {
        while(PinRead(pin)) if(readusclock() > t1) return;
        while(!PinRead(pin)) if(readusclock() > t1) return;
        writeusclock(0);
        while(PinRead(pin)) if(readusclock() > t2) return;
    } else {
        while(!PinRead(pin)) if(readusclock() > t1) return;
        while(PinRead(pin)) if(readusclock() > t1) return;
        writeusclock(0);
        while(!PinRead(pin)) if(readusclock() > t2) return;
    }
    t1 = readusclock();
    iret = t1;
    targ = T_INT;
}



/****************************************************************************************************************************
IR routines
*****************************************************************************************************************************/

void cmd_ir(void) {
    char *p;
    int i, pin, dev, cmd;
    if(checkstring(cmdline, "CLOSE")) {
        IrState = IR_CLOSED;
    	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
        IrInterrupt = NULL;
    } else if((p = checkstring(cmdline, "SEND"))) {
        getargs(&p, 5, ",");
        pin = getinteger(argv[0]);
        dev = getint(argv[2], 0, 0b11111);
        cmd = getint(argv[4], 0, 0b1111111);
        ExtCfg(pin, EXT_DIG_OUT, 0);
        cmd = (dev << 7) | cmd;
        IRSendSignal(pin, 186);
        for(i = 0; i < 12; i++) {
            uSec(600);
            if(cmd & 1)
                IRSendSignal(pin, 92);
            else
                IRSendSignal(pin, 46);
            cmd >>= 1;
        }
    } else {
        getargs(&cmdline, 5, ",");
        if(IrState != IR_CLOSED) error("Already open");
        if(argc%2 == 0 || argc == 0) error("Invalid syntax");
        IrVarType = 0;
        IrDev = findvar(argv[0], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR)  error("Invalid variable");
        if(vartbl[VarIndex].type & T_NBR) IrVarType |= 0b01;
        IrCmd = findvar(argv[2], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR)  error("Invalid variable");
        if(vartbl[VarIndex].type & T_NBR) IrVarType |= 0b10;
        InterruptUsed = true;
        IrInterrupt = GetIntAddress(argv[4]);							// get the interrupt location
        IrInit();
    }
}


void IrInit(void) {
    writeusclock(0);
//******    ExtCfg(IRPIN, EXT_DIG_IN, 0);
	IR_InitDef.Pull = GPIO_PULLUP; //set as input with no pullup or down
	IR_InitDef.Pin = PinDef[IRPIN].bitnbr;
	IR_InitDef.Mode = GPIO_MODE_IT_RISING_FALLING;
	IR_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	HAL_GPIO_Init(PinDef[IRPIN].sfr, &IR_InitDef);
    IrReset();
}


void IrReset(void) {
	IrState = IR_WAIT_START;
    IrCount = 0;
    writeusclock(0);
}


// this modulates (at about 38KHz) the IR beam for transmit
// half_cycles is the number of half cycles to send.  ie, 186 is about 2.4mSec
void IRSendSignal(int pin, int half_cycles) {
    while(half_cycles--) {
        PinSetBit(pin, LATINV);
        uSec(13);
    }
}




/****************************************************************************************************************************
 The LCD command
*****************************************************************************************************************************/
/*
void LCD_Nibble(int Data, int Flag, int Wait_uSec);
void LCD_Byte(int Data, int Flag, int Wait_uSec);
void LcdPinSet(int pin, int val);
static char lcd_pins[6];

void cmd_lcd(void)
 {
    char *p;
    int i, j;

    if((p = checkstring(cmdline, "INIT"))) {
        getargs(&p, 11, ",");
        if(argc != 11) error("Invalid syntax");
        if(*lcd_pins) error("Already open");
        for(i = 0; i < 6; i++) {
            lcd_pins[i] = getinteger(argv[i * 2]);
            if(ExtCurrentConfig[(int)lcd_pins[i]] >= EXT_COM_RESERVED)  error("Pin is in use");
            ExtCfg(lcd_pins[i], EXT_DIG_OUT, 0);
            ExtCfg(lcd_pins[i], EXT_COM_RESERVED, 0);
        }
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0011, 0, 5000);                                // reset
        LCD_Nibble(0b0010, 0, 2000);                                // 4 bit mode
        LCD_Byte(0b00101100, 0, 600);                               // 4 bits, 2 lines
        LCD_Byte(0b00001100, 0, 600);                               // display on, no cursor
        LCD_Byte(0b00000110, 0, 600);                               // increment on write
        LCD_Byte(0b00000001, 0, 3000);                              // clear the display
        return;
    }

    if(!*lcd_pins) error("Not open");
    if(checkstring(cmdline, "CLOSE")) {
        for(i = 0; i < 6; i++) {
			ExtCfg(lcd_pins[i], EXT_NOT_CONFIG, 0);					// all set to unconfigured
			ExtSet(lcd_pins[i], 0);									// all outputs (when set) default to low
            *lcd_pins = 0;
        }
    } else if((p = checkstring(cmdline, "CLEAR"))) {                // clear the display
        LCD_Byte(0b00000001, 0, 3000);
    } else if((p = checkstring(cmdline, "CMD")) || (p = checkstring(cmdline, "DATA"))) { // send a command or data
        getargs(&p, MAX_ARG_COUNT * 2, ",");
        for(i = 0; i < argc; i += 2) {
            j = getint(argv[i], 0, 255);
            LCD_Byte(j, toupper(*cmdline) == 'D', 0);
        }
    } else {
        const char linestart[4] = {0, 64, 20, 84};
        int center, pos;

        getargs(&cmdline, 5, ",");
        if(argc != 5) error("Invalid syntax");
        i = getint(argv[0], 1, 4);
        pos = 1;
        if(checkstring(argv[2], "C8"))
            center = 8;
        else if(checkstring(argv[2], "C16"))
            center = 16;
        else if(checkstring(argv[2], "C20"))
            center = 20;
        else if(checkstring(argv[2], "C40"))
            center = 40;
        else {
            center = 0;
            pos = getint(argv[2], 1, 256);
        }
        p = getstring(argv[4]);                                     // returns an MMBasic string
        i = 128 + linestart[i - 1] + (pos - 1);
        LCD_Byte(i, 0, 600);
        for(j = 0; j < (center - *p) / 2; j++) {
            LCD_Byte(' ', 1, 0);
        }
        for(i = 1; i <= *p; i++) {
            LCD_Byte(p[i], 1, 0);
            j++;
        }
        for(; j < center; j++) {
            LCD_Byte(' ', 1, 0);
        }
    }
}



void LCD_Nibble(int Data, int Flag, int Wait_uSec) {
    int i;
    LcdPinSet(lcd_pins[4], Flag);
    for(i = 0; i < 4; i++)
        LcdPinSet(lcd_pins[i], (Data >> i) & 1);
    LcdPinSet(lcd_pins[5], 1); uSec(250); LcdPinSet(lcd_pins[5], 0);
    if(Wait_uSec)
        uSec(Wait_uSec);
    else
        uSec(250);
}


void LCD_Byte(int Data, int Flag, int Wait_uSec) {
    LCD_Nibble(Data/16, Flag, 0);
    LCD_Nibble(Data, Flag, Wait_uSec);
}


void LcdPinSet(int pin, int val) {
    PinSetBit(pin, val ? LATSET : LATCLR);
}
*/
// round a float to an integer
unsigned short FloatToUint16(MMFLOAT x) {
    if(x<0 || x > 65535 + 0.5)
        error("Number range");
    return (x >= 0 ? (unsigned short)(x + 0.5) : (unsigned short)(x - 0.5)) ;
}

/****************************************************************************************************************************
 The DHT22 function
*****************************************************************************************************************************/

//#define CoreTicks(us) (((us * 1000u) / (2000000000u/PeripheralBusSpeed)))  // how many core timer ticks does the argument in uS represent

void DHT22(char *p) {
    int pin;
    long long int r;
    int i, timeout, scale=0;;
    MMFLOAT *temp, *humid;

    getargs(&p, 7, ",");
    if(!(argc == 5 || argc == 7)) error("Incorrect number of arguments");

    // get the two variables
	temp = findvar(argv[2], V_FIND);
	if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");
	humid = findvar(argv[4], V_FIND);
	if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");

    // get the pin number and set it up
    pin = getinteger(argv[0]);
    if(IsInvalidPin(pin)) error("Invalid pin");
    if(ExtCurrentConfig[pin] != EXT_NOT_CONFIG)  error("Pin is in use");
    ExtCfg(pin, EXT_OC_OUT, 0);
    if(argc==7){
    	scale=getint(argv[6],0,1);
    }
    // pulse the pin low for 1mS
    PinSetBit(pin, LATCLR);
    uSec(1000+scale*18000);
    writeusclock(0);
    timeout = 400;
    PinSetBit(pin, CNPUSET);
    uSec(5);
    // wait for the DHT22 to pull the pin low and return it high then take it low again
    while(PinRead(pin)) if(readusclock() > timeout) goto error_exit;
    while(!PinRead(pin)) if(readusclock() > timeout) goto error_exit;
    while(PinRead(pin)) if(readusclock() > timeout) goto error_exit;

    // now we wait for the pin to go high and measure how long it stays high (> 50uS is a one bit, < 50uS is a zero bit)
    for(r = i = 0; i < 40; i++) {
    	timeout=400;
        while(!PinRead(pin)) if(readusclock() > timeout) goto error_exit;
        timeout=400;writeusclock(0);
        while(PinRead(pin)) if(readusclock() > timeout) goto error_exit;
        r <<= 1;
        r |= (readusclock() > 50);
    }

    // we have all 40 bits
    // first validate against the checksum
    if( ( ( ((r >> 8) & 0xff) + ((r >> 16) & 0xff) + ((r >> 24) & 0xff) + ((r >> 32) & 0xff) ) & 0xff) != (r & 0xff)) goto error_exit;                                           // returning temperature
    if(scale==0){
		*temp = (MMFLOAT)((r >> 8) &0x7fff) / 10.0;                       // get the temperature
		if((r >> 8) &0x8000) *temp = -*temp;                            // the top bit is the sign
		*humid = (MMFLOAT)(r >> 24) / 10.0;                               // get the humidity
    } else {
		*temp = (MMFLOAT)((signed char)((r>>16) & 0xFF));                       // get the temperature
		*humid = (MMFLOAT)((signed char)(r >> 32));                               // get the humidity
    }
    goto normal_exit;

error_exit:
    *temp = *humid = 1000.0;                                        // an obviously incorrect reading

normal_exit:
    ExtCfg(pin, EXT_NOT_CONFIG, 0);
    PinSetBit(pin, LATCLR);
}

//Updated to use parsenumberarray G.A. 02/11/2025
void cmd_bitbang(void){
	char *tp;
	tp = checkstring(cmdline, "WS2812");
	if(tp) {
		//WS2812(tp);
		//return;
        cmdline = tp;
        cmd_WS2812();
        return;
	}
	tp = checkstring(cmdline, "DHT22");
	if(tp) {
		DHT22(tp);
		return;
	}
	tp = checkstring(cmdline, "BITSTREAM");
	if(tp) {
	//	void *ptr1 = NULL;
		int i,num,size;
        //int i,num;
		uint32_t pin;
		MMFLOAT *a1float=NULL;
		int64_t *a1int=NULL;
		unsigned short *data, now;
		getargs(&tp, 5,",");
		if(!(argc == 5)) error("Argument count");
		num=getint(argv[2],1,250000);
        pin=getinteger(argv[0]);
       if(IsInvalidPin(pin)) error("Invalid pin");
    	if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin is not an output");
	   /*
		ptr1 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
		if(vartbl[VarIndex].type & T_NBR) {
			if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
			if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
				error("Argument 2 must be an array");
			}
			if((vartbl[VarIndex].dims[0] - OptionBase) < num-1)error("Array too small");
			a1float = (MMFLOAT *)ptr1;
		} else if(vartbl[VarIndex].type & T_INT) {
			if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
			if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
				error("Argument 2 must be an array");
			}
			if((vartbl[VarIndex].dims[0] - OptionBase) < num-1)error("Array too small");
			a1int = (int64_t *)ptr1;
		} else error("Argument 2 must be an array");
        */
    	size=parsenumberarray(argv[4],&a1float, &a1int, 3, 1, NULL, false,NULL);
    	if(size < num)error("Array too small");
		data=GetTempMemory(num * sizeof(unsigned short));
		if(a1float!=NULL){
			for(i=0; i< num;i++)data[i]= FloatToUint16(*a1float++);
		} else {
			for(i=0; i< num;i++){
				if(*a1int <0 || *a1int>65535)error("Number range");
				data[i]= *a1int++ ;
			}
		}
        __disable_irq();
        MX_TIM17_Init(20);
    	for(i=0;i<num;i++){
     		__HAL_TIM_SET_COUNTER(&htim17,0);
            if(i==0)__HAL_TIM_ENABLE(&htim17);
       		PinDef[pin].sfr->ODR ^= PinDef[pin].bitnbr;
     		now=data[i];
    		while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    	}
        HAL_TIM_Base_Stop(&htim17);
        __enable_irq();
		return;
	}
	error("Unknown BITBANG option");
}
/* Added type W for  support for SK6812 RGBW Leds */
//void WS2812(char *q){
void cmd_WS2812(void){
       void *ptr1 = NULL;
        int64_t *dest=NULL;
        uint32_t pin, red , green, blue,white, colour;
        short T0H=0,T0L=0,T1H=0,T1L=0;
        char *p;
        short now;
        int i, j, bit, nbr=0,colours=3;
    	getargs(&cmdline, 7, ",");
        if(argc!=7)error("Argument count");
    	p=argv[0];
    	if(toupper(*p)=='O'){
    		T1H=13;
    		T1L=11;
    		T0H=6;
    		T0L=15;
    	} else if(toupper(*p)=='B'){
    		T1H=15;
    		T1L=8;
    		T0H=7;
    		T0L=16;
    	} else if(toupper(*p)=='S'){
    		T1H=11;
    		T1L=11;
    		T0H=5;
    		T0L=17;
       	} else if(toupper(*p)=='W'){
       	    colours=4;
            T1H=12;      // 0.6uS
            T1L=12;      // 0.6uS
            T0H=6;       // 0.3uS
            T0L=18;      // 0.9uS
    	} else error("Syntax");

            nbr=getint(argv[4],1,512);
            if(nbr>1){
                ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK);
                if(vartbl[VarIndex].type & T_INT) {
                    if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                    if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                        error("Argument 1 must be integer array");
                    } else {
                        if((vartbl[VarIndex].dims[0] - OptionBase + 1)<nbr)error("Array size");
                    }
                    dest = (long long int *)ptr1;
                } else error("Argument 1 must be integer array");
            } else {
                colour=getint(argv[6],0,0xFFFFFF);
                dest = (long long int *)&colour;
            }

        pin=getinteger(argv[2]);
        if(IsInvalidPin(pin)) error("Invalid pin");
    	//if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin is not an output");
    	if(!(ExtCurrentConfig[pin] == EXT_NOT_CONFIG || ExtCurrentConfig[pin] == EXT_DIG_OUT))  error("Pin | is in use",pin);
    	ExtCfg(pin, EXT_DIG_OUT, 0);
		PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;
	    //p=GetTempMemory((nbr+1)*3);
		//uSec(60);
		p=GetTempMemory(nbr+1);
		uSec(80);
    	for(i=0;i<nbr;i++){
    		green=(dest[i]>>8) & 0xFF;
    		red=(dest[i]>>16) & 0xFF;
    		blue=dest[i] & 0xFF;
    		if(colours==4)white=(dest[i]>>24) & 0xFF;
			p[0]=0;p[1]=0;p[2]=0;
			if(colours==4){p[3]=0;}
    		for(j=0;j<8;j++){
    			bit=1<<j;
    			if( green &  (1<<(7-j)) )p[0] |= bit;
    			if(red   & (1<<(7-j)))p[1] |= bit;
    			if(blue  & (1<<(7-j)))p[2] |= bit;
                if(colours==4){
    			    if(white  & (1<<(7-j)))p[3] |= bit;
                }
    		}
    		p+=colours;
    	}
    	p-=(nbr*colours);
        __disable_irq();
        MX_TIM17_Init(1);
        HAL_TIM_Base_Start(&htim17);
    	for(i=0;i<nbr*colours;i++){
    		__HAL_TIM_SET_COUNTER(&htim17,0);
    		for(j=0;j<8;j++){
    			if(*p & 1){
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr;now=__HAL_TIM_GET_COUNTER(&htim17)+T1H;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;now=__HAL_TIM_GET_COUNTER(&htim17)+T1L;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    			} else {
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr;now=__HAL_TIM_GET_COUNTER(&htim17)+T0H;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;now=__HAL_TIM_GET_COUNTER(&htim17)+T0L;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    			}
    			*p>>=1;
    		}
    		p++;
    	}
        HAL_TIM_Base_Stop(&htim17);
        __enable_irq();
}


/*
void cmd_bitbang(void){
	char *tp;
	tp = checkstring(cmdline, "WS2812");
	if(tp) {
		WS2812(tp);
		return;
	}
	tp = checkstring(cmdline, "DHT22");
	if(tp) {
		DHT22(tp);
		return;
	}
	tp = checkstring(cmdline, "BITSTREAM");
	if(tp) {
		void *ptr1 = NULL;
		int i,num;
		uint32_t pin;
		MMFLOAT *a1float=NULL;
		int64_t *a1int=NULL;
		unsigned short *data, now;
		getargs(&tp, 5,",");
		if(!(argc == 5)) error("Argument count");
		num=getint(argv[2],1,250000);
        pin=getinteger(argv[0]);
        if(IsInvalidPin(pin)) error("Invalid pin");
    	if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin is not an output");
		ptr1 = findvar(argv[4], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
		if(vartbl[VarIndex].type & T_NBR) {
			if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
			if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
				error("Argument 2 must be an array");
			}
			if((vartbl[VarIndex].dims[0] - OptionBase) < num-1)error("Array too small");
			a1float = (MMFLOAT *)ptr1;
		} else if(vartbl[VarIndex].type & T_INT) {
			if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
			if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
				error("Argument 2 must be an array");
			}
			if((vartbl[VarIndex].dims[0] - OptionBase) < num-1)error("Array too small");
			a1int = (int64_t *)ptr1;
		} else error("Argument 2 must be an array");
		data=GetTempMemory(num * sizeof(unsigned short));
		if(a1float!=NULL){
			for(i=0; i< num;i++)data[i]= FloatToUint16(*a1float++);
		} else {
			for(i=0; i< num;i++){
				if(*a1int <0 || *a1int>65535)error("Number range");
				data[i]= *a1int++ ;
			}
		}
        __disable_irq();
        MX_TIM17_Init(20);
    	for(i=0;i<num;i++){
     		__HAL_TIM_SET_COUNTER(&htim17,0);
            if(i==0)__HAL_TIM_ENABLE(&htim17);
       		PinDef[pin].sfr->ODR ^= PinDef[pin].bitnbr;
     		now=data[i];
    		while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    	}
        HAL_TIM_Base_Stop(&htim17);
        __enable_irq();
		return;
	}
}
*/
/*
void WS2812(char *q){
       void *ptr1 = NULL;
        int64_t *dest=NULL;
        uint32_t pin, red , green, blue, colour;
        short T0H=0,T0L=0,T1H=0,T1L=0;
        char *p;
        short now;
        int i, j, bit, nbr=0;
    	getargs(&q, 7, ",");
        if(!(argc == 5 || argc==7))error("Argument count");
    	p=argv[0];
    	if(toupper(*p)=='O'){
    		T1H=13;
    		T1L=11;
    		T0H=6;
    		T0L=15;
    	} else if(toupper(*p)=='B'){
    		T1H=15;
    		T1L=8;
    		T0H=7;
    		T0L=16;
    	} else if(toupper(*p)=='S'){
    		T1H=11;
    		T1L=11;
    		T0H=5;
    		T0L=17;
    	} else error("Syntax");
    	if(argc==7){
            nbr=getint(argv[4],1,64);
            if(nbr>1){
                ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK);
                if(vartbl[VarIndex].type & T_INT) {
                    if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                    if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                        error("Argument 1 must be integer array");
                    } else {
                        if((vartbl[VarIndex].dims[0] - OptionBase + 1)<nbr)error("Array size");
                    }
                    dest = (long long int *)ptr1;
                } else error("Argument 1 must be integer array");
            } else {
                colour=getint(argv[6],0,0xFFFFFF);
                dest = (long long int *)&colour;
            }
    	} else {
			ptr1 = findvar(argv[4], V_FIND | V_EMPTY_OK);
			if(vartbl[VarIndex].type & T_INT) {
				if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
				if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
					error("Argument 1 must be integer array");
				}
				nbr=(vartbl[VarIndex].dims[0] - OptionBase)+1;
				dest = (long long int *)ptr1;
			} else error("Argument 1 must be integer array");
			}
        pin=getinteger(argv[2]);
        if(IsInvalidPin(pin)) error("Invalid pin");
    	if(!(ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT)) error("Pin is not an output");
		PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;
		p=GetTempMemory((nbr+1)*3);
		uSec(60);
    	for(i=0;i<nbr;i++){
    		green=(dest[i]>>8) & 0xFF;
    		red=(dest[i]>>16) & 0xFF;
    		blue=dest[i] & 0xFF;
			p[0]=0;p[1]=0;p[2]=0;
    		for(j=0;j<8;j++){
    			bit=1<<j;
    			if( green &  (1<<(7-j)) )p[0] |= bit;
    			if(red   & (1<<(7-j)))p[1] |= bit;
    			if(blue  & (1<<(7-j)))p[2] |= bit;
    		}
    		p+=3;
    	}
    	p-=(nbr*3);
        __disable_irq();
        MX_TIM17_Init(1);
        HAL_TIM_Base_Start(&htim17);
    	for(i=0;i<nbr*3;i++){
    		__HAL_TIM_SET_COUNTER(&htim17,0);
    		for(j=0;j<8;j++){
    			if(*p & 1){
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr;now=__HAL_TIM_GET_COUNTER(&htim17)+T1H;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;now=__HAL_TIM_GET_COUNTER(&htim17)+T1L;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    			} else {
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr;now=__HAL_TIM_GET_COUNTER(&htim17)+T0H;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    				PinDef[pin].sfr->BSRR = PinDef[pin].bitnbr<<16;now=__HAL_TIM_GET_COUNTER(&htim17)+T0L;while(__HAL_TIM_GET_COUNTER(&htim17)<now){};
    			}
    			*p>>=1;
    		}
    		p++;
    	}
        HAL_TIM_Base_Stop(&htim17);
        __enable_irq();
}
*/
/****************************************************************************************************************************
 The DISTANCE function
*****************************************************************************************************************************/

void fun_distance(void) {
    int trig, echo,techo;

	getargs(&ep, 3, ",");
	if((argc &1) != 1) error("Invalid syntax");
    trig = getinteger(argv[0]);
    if(argc == 3)
        echo = getinteger(argv[2]);
    else
        echo = trig;                                                // they are the same if it is a 3-pin device
    if(IsInvalidPin(trig) || IsInvalidPin(echo)) error("Invalid pin");
    if(ExtCurrentConfig[trig] >= EXT_COM_RESERVED || ExtCurrentConfig[echo] >= EXT_COM_RESERVED)  error("Pin is in use");
    ExtCfg(echo, EXT_DIG_IN, CNPUSET);                              // setup the echo input
    PinSetBit(trig, LATCLR);                                        // trigger output must start low
    ExtCfg(trig, EXT_DIG_OUT, 0);                                   // setup the trigger output
    PinSetBit(trig, LATSET); uSec(20); PinSetBit(trig, LATCLR);     // pulse the trigger
    uSec(50);
    ExtCfg(echo, EXT_DIG_IN, CNPUSET);                              // this is in case the sensor is a 3-pin type
    uSec(50);
    PauseTimer = 0;                                                 // this is our timeout
    while(PinRead(echo)) if(PauseTimer > 50) { fret = -2; return; } // wait for the acknowledgement pulse start
    while(!PinRead(echo)) if(PauseTimer > 100) { fret = -2; return;}// then its end
    PauseTimer = 0;
    WriteCoreTimer(0);
    while(PinRead(echo)) {                                          // now wait for the echo pulse
        if(PauseTimer > 38) {                                       // timeout is 38mS
            fret = -1;
            return;
        }
    }
    techo=ReadCoreTimer();
    // we have the echo, convert the time to centimeters
    fret = ((MMFLOAT)techo)/11600.0;  //200 ticks per us, 58 us per cm
    targ = T_NBR;
}




/****************************************************************************************************************************
 The KEYPAD command
*****************************************************************************************************************************/
/*
static char keypad_pins[8];
MMFLOAT *KeypadVar;
char *KeypadInterrupt = NULL;
void KeypadClose(void);

void cmd_keypad(void) {
    int i, j;

    if(checkstring(cmdline, "CLOSE"))
        KeypadClose();
    else {
        getargs(&cmdline, 19, ",");
        if(argc%2 == 0 || argc < 17) error("Invalid syntax");
        if(KeypadInterrupt != NULL) error("Already open");
        KeypadVar = findvar(argv[0], V_FIND);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(!(vartbl[VarIndex].type & T_NBR)) error("Floating point variable required");
        InterruptUsed = true;
        KeypadInterrupt = GetIntAddress(argv[2]);					// get the interrupt location
        for(i = 0; i < 8; i++) {
            if(i == 7 && argc < 19) {
                keypad_pins[i] = 0;
                break;
            }
            j = getinteger(argv[(i + 2) * 2]);
            if(ExtCurrentConfig[j] >= EXT_COM_RESERVED)  error("Pin is in use");
            if(i < 4) {
                ExtCfg(j, EXT_DIG_IN, CNPUSET);
            } else {
                ExtCfg(j, EXT_OC_OUT, 0);
                PinSetBit(j, LATSET);
            }
            ExtCfg(j, EXT_COM_RESERVED, 0);
            keypad_pins[i] = j;
        }
    }
}


void KeypadClose(void) {
    int i;
    if(KeypadInterrupt == NULL) return;
    for(i = 0; i < 8; i++) {
        if(keypad_pins[i]) {
            ExtCfg(keypad_pins[i], EXT_NOT_CONFIG, 0);				// all set to unconfigured
        }
    }
    KeypadInterrupt = NULL;
}


int KeypadCheck(void) {
    static unsigned char count = 0, keydown = false;
    int i, j;
    const char PadLookup[16] = { 1, 2, 3, 20, 4, 5, 6, 21, 7, 8, 9, 22, 10, 0, 11, 23 };

    if(count++ % 64) return false;                                  // only check every 64 loops through the interrupt processor

    for(j = 4; j < 8; j++) {                                        // j controls the pull down pins
        if(keypad_pins[j]) {                                        // we might just have 3 pull down pins
            PinSetBit(keypad_pins[j], LATCLR);                      // pull it low
            for(i = 0; i < 4; i++) {                                // i is the row sense inputs
                if(PinRead(keypad_pins[i]) == 0) {                  // if it is low we have found a keypress
                    if(keydown) goto exitcheck;                     // we have already reported this, so just exit
                    uSec(40 * 1000);                                // wait 40mS and check again
                    if(PinRead(keypad_pins[i]) != 0) goto exitcheck;// must be contact bounce if it is now high
                    *KeypadVar = PadLookup[(i << 2) | (j - 4)];     // lookup the key value and set the variable
                    PinSetBit(keypad_pins[j], LATSET);
                    keydown = true;                                 // record that we know that the key is down
                    return true;                                    // and tell the interrupt processor that we are good to go
                }
            }
            PinSetBit(keypad_pins[j], LATSET);                      // wasn't this pin, clear the pulldown
        }
    }
    keydown = false;                                                // no key down, record the fact
    return false;

exitcheck:
    PinSetBit(keypad_pins[j], LATSET);
    return false;
}
*/





/*******************************************************************************************
********************************************************************************************

Utility routines for the external I/O commands and functions in MMBasic

********************************************************************************************
********************************************************************************************/


void ClearExternalIO(void) {
	int i;

    PWMClose(0); PWMClose(1);                                       // close any running PWM output
    SerialClose(1); SerialClose(2); SerialClose(3);                             // same for serial ports
    SPIClose();SPI2Close();                                            // and the SPI
    ADCclose();
	dacclose();
	CloseAudio(1);
    ResetDisplay();
    i2c_disable();                                                  // close I2C
    i2c2_disable();                                                  // close I2C
    i2c3_disable();                                                  // close I2C
    mouse0close();
    IrState = IR_CLOSED;
    IrInterrupt = NULL;
    IrGotMsg = false;
//    KeypadInterrupt = NULL;
    FrameInterrupt=NULL;
    nun1=0; nun2=0; nun3=0;
    classic1=0; classic2=0; classic3=0;
    mouse0=0; mouse1=0; mouse2=0; mouse3=0;
    nun1Interruptc=NULL;
    nun2Interruptc=NULL;
    nun3Interruptc=NULL;
    nun1Interruptz=NULL;
    nun2Interruptz=NULL;
    nun3Interruptz=NULL;
    mouse0Interruptc=NULL;
    mouse1Interruptc=NULL;
    mouse2Interruptc=NULL;
    mouse3Interruptc=NULL;
    mouse0Interruptz=NULL;
    mouse1Interruptz=NULL;
    mouse2Interruptz=NULL;
    mouse3Interruptz=NULL;
    mouse0Interruptu=NULL;
    mouse1Interruptu=NULL;
    mouse2Interruptu=NULL;
    mouse3Interruptu=NULL;
//    *lcd_pins = 0;                                                  // close the LCD
    ds18b20Timer = -1;                                              // turn off the ds18b20 timer

	for(i = 1; i < NBRPINS + 1; i++) {
		if(CheckPin(i, CP_NOABORT | CP_IGNORE_INUSE | CP_IGNORE_RESERVED)) {    // don't reset invalid or boot reserved pins
			ExtCfg(i, EXT_NOT_CONFIG, 0);							// all set to unconfigured
		}
	}

	for(i = 0; i < NBRINTERRUPTS; i++) {
		inttbl[i].pin = 0;                                          // disable all interrupts
	}
	InterruptReturn = NULL;
	InterruptUsed = false;
    OnKeyGOSUB = NULL;

    for(i = 0; i <= NBRSETTICKS; i++) TickInt[i] = NULL;
    for(i = 0; i < NBRSETTICKS; i++) TickActive[i] = 1;


	for(i = 0; i < NBR_PULSE_SLOTS; i++) PulseCnt[i] = 0;           // disable any pending pulse commands
    PulseActive = false;
	if(gif!=NULL){
		GifTimer=5000;
		FileClose(giffnbr);
		FreeMemorySafe((void*)&frame);
		gd_close_gif(gif);
		gif=NULL;
		GifTimer=0;
		giffnbr=0;
	}
	autocursor=0;
}



/****************************************************************************************************************************
Initialise the I/O pins
*****************************************************************************************************************************/
void initExtIO(void) {
	int i;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	for(i = 1; i < NBRPINS + 1; i++) {
        if(CheckPin(i, CP_NOABORT | CP_IGNORE_INUSE | CP_IGNORE_RESERVED | CP_IGNORE_BOOTRES)){
            ExtCfg(i, EXT_NOT_CONFIG, 0);							// all set to unconfigured
        }
	}
	oc1=0;oc2=0;oc3=0;oc4=0;oc5=0; //set all PWM to off
}

int getinttblpos(int tbl[],int pin){
    int i=0;
    while (tbl[i]!=pin){
        i++;
    }
    return i;
}

/****************************************************************************************************************************
Configure an I/O pin
*****************************************************************************************************************************/
void ExtCfg(int pin, int cfg, int option) {
	int i,edge,pull;
    if(IsInvalidPin(pin)) error("Invalid pin");

    CheckPin(pin, CP_IGNORE_INUSE | CP_IGNORE_RESERVED);

    if(cfg >= EXT_COM_RESERVED) {
        ExtCurrentConfig[pin] = cfg;                                // don't do anything except set the config type
        return;
    }

	// make sure that interrupts are disabled in case we are changing from an interrupt input
    if(pin == INT1PIN) HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    if(pin == INT2PIN) HAL_NVIC_DisableIRQ(EXTI2_IRQn);
    if(pin == INT3PIN) HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    if(pin == INT4PIN) HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    if(pin == IRPIN) HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

    // make sure any pullups/pulldowns are removed in case we are changing from a digital input
    PinSetBit(pin, CNPUCLR);  PinSetBit(pin, CNPDCLR);

	for(i = 0; i < NBRINTERRUPTS; i++)
        if(inttbl[i].pin == pin)
            inttbl[i].pin = 0;                                           // start off by disable a software interrupt (if set) on this pin

	switch(cfg) {
		case EXT_NOT_CONFIG:
								GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
								GPIO_InitDef.Pin = PinDef[pin].bitnbr;
								GPIO_InitDef.Mode = GPIO_MODE_ANALOG;
								GPIO_InitDef.Speed = GPIO_SPEED_FREQ_LOW;
								HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
								break;


		case EXT_ANA_IN:        if(!(PinDef[pin].mode & ANALOG_IN)) error("Invalid configuration");
								ADCbits[pin]=option;
								break;

		case EXT_PER_IN:		// same as counting, so fall through unless high speed
								if(pin == COUNT5) error("Invalid configuration");
   		case EXT_FREQ_IN:		// same as counting, so fall through
		case EXT_CNT_IN:		if(pin == INT3PIN && mouse0) error("H/W Interrupt in use for mouse");

		                        //Set the options for CIN used when call CSUB
					            edge = GPIO_MODE_IT_RISING;
					            pull = GPIO_PULLDOWN;
					            if(cfg==EXT_CNT_IN && option==2)edge = GPIO_MODE_IT_FALLING;
					            if(cfg==EXT_CNT_IN && option>=3)edge = GPIO_MODE_IT_RISING | GPIO_MODE_IT_FALLING;
					            //if(option==1 || option==4)pull = GPIO_PULLDOWN;  //Rising and Pull Down or both and pulldown
					            if(option==2 || option==5)pull = GPIO_PULLUP;    //Falling and pullup or both and pullup
					            if(option==3 )pull = GPIO_NOPULL;    //Falling and pullup or both and pullup

        						if(pin == INT1PIN) {
        							GPIO_InitDef.Pull = pull; //set  pull up/down
									GPIO_InitDef.Pin = COUNT1_Pin;
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI1_IRQn);
									HAL_GPIO_Init(COUNT1_GPIO_Port, &GPIO_InitDef);
									INT1Count = INT1Value = 0;
                                    INT1Timer = INT1InitTimer = option;  // only used for frequency and period measurement
									break;
								}
								if(pin == INT2PIN) {
									GPIO_InitDef.Pull = pull; //set  pull up/down
									GPIO_InitDef.Pin = COUNT2_Pin;
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI2_IRQn);
									HAL_GPIO_Init(COUNT2_GPIO_Port, &GPIO_InitDef);
									INT2Count = INT2Value = 0;
                                    INT2Timer = INT2InitTimer = option;  // only used for frequency and period measurement
									break;
								}
								if(pin == INT3PIN) {
									GPIO_InitDef.Pull = pull; //set  pull up/down
									GPIO_InitDef.Pin = COUNT3_Pin;
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI3_IRQn);
									HAL_GPIO_Init(COUNT3_GPIO_Port, &GPIO_InitDef);
									INT3Count = INT3Value = 0;
                                    INT3Timer = INT3InitTimer = option;  // only used for frequency and period measurement
									break;
								}
								if(pin == INT4PIN) {
									GPIO_InitDef.Pull = pull; //set  pull up/down
									GPIO_InitDef.Pin = COUNT4_Pin;
									GPIO_InitDef.Mode = edge;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
									HAL_NVIC_EnableIRQ(EXTI4_IRQn);
									HAL_GPIO_Init(COUNT4_GPIO_Port, &GPIO_InitDef);
									INT4Count = INT4Value = 0;
                                    INT4Timer = INT4InitTimer = option;  // only used for frequency and period measurement
									break;
								}
								if(pin == COUNT5) {
									HAL_TIM_Base_DeInit(&htim2);
									MX_TIM2_Init1();
									Count5High=0 ;
									WriteCount5(0);
									lastCount5High=0;
									HAL_TIM_Base_Start_IT(&htim2);
									INT5Count = INT5Value = 0;
                                    if(option && cfg==EXT_CNT_IN){
                                    	CountInterrupt=(char *)option;
                                    	InterruptUsed=1;
                                    } else {
                                    	INT5Timer = INT5InitTimer = option;  // only used for frequency and period measurement
                                    }
									break;
								}
				                error("Invalid configuration");		// not an interrupt enabled pin
								return;

		case EXT_INT_LO:											// same as digital input, so fall through
		case EXT_INT_HI:											// same as digital input, so fall through
		case EXT_INT_BOTH:											// same as digital input, so fall through
		case EXT_DIG_IN:		if(!(PinDef[pin].mode & DIGITAL_IN)) error("Invalid configuration");
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
									if(option==CNPUSET)GPIO_InitDef.Pull = GPIO_PULLUP;
									if(option==CNPDSET)GPIO_InitDef.Pull = GPIO_PULLDOWN;
									GPIO_InitDef.Mode = GPIO_MODE_INPUT;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
									break;

		case EXT_DIG_OUT:		if(!(PinDef[pin].mode & DIGITAL_OUT)) error("Invalid configuration");
									GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_HIGH;
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
								break;

		case EXT_OC_OUT:		if(!(PinDef[pin].mode & DIGITAL_OUT)) error("Invalid configuration");
									GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
									GPIO_InitDef.Pin = PinDef[pin].bitnbr;
									GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_OD;
									GPIO_InitDef.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
									HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
								break;

		default:				error("Invalid configuration");
		                        return;
	}
	ExtCurrentConfig[pin] = cfg;
	if(cfg == EXT_NOT_CONFIG) ExtSet(pin, 0);						// set the default output to low

}



/****************************************************************************************************************************
Set the output of a digital I/O pin or set the current count for an interrupt pin
*****************************************************************************************************************************/
void ExtSet(int pin, long long int val){

    if(ExtCurrentConfig[pin] == EXT_NOT_CONFIG || ExtCurrentConfig[pin] == EXT_DIG_OUT || ExtCurrentConfig[pin] == EXT_OC_OUT) {
		PinDef[pin].sfr->BSRR = (PinDef[pin].bitnbr)<<(val ? 0:16);return;
    }
    else if(ExtCurrentConfig[pin] == EXT_CNT_IN){ //allow the user to zero the count
        if(pin == INT1PIN) INT1Count=val;
        if(pin == INT2PIN) INT2Count=val;
        if(pin == INT3PIN) INT3Count=val;
        if(pin == INT4PIN) INT4Count=val;
        if(pin == COUNT5) {Count5High=val>>32 ; WriteCount5(val & 0xFFFFFFFF);}

    }
    else
        error("Pin is not an output");
}



/****************************************************************************************************************************
Get the value of an I/O pin and returns it
For digital returns 0 if low or 1 if high
For analog returns the reading as a 10 bit number with 0b1111111111 = 3.3V
*****************************************************************************************************************************/
int64_t ExtInp(int pin){
	// read from a digital input
	if(ExtCurrentConfig[pin] == EXT_DIG_IN || ExtCurrentConfig[pin] == EXT_INT_HI || ExtCurrentConfig[pin] == EXT_INT_LO || ExtCurrentConfig[pin] == EXT_INT_BOTH || ExtCurrentConfig[pin] == EXT_OC_OUT || ExtCurrentConfig[pin] == EXT_DIG_OUT) {
        return  PinRead(pin);
	}

	// read from a frequency/period input
	if(ExtCurrentConfig[pin] == EXT_FREQ_IN || ExtCurrentConfig[pin] == EXT_PER_IN) {
		// select input channel
        if(pin == INT1PIN) return INT1Value;
        if(pin == INT2PIN) return INT2Value;
        if(pin == INT3PIN) return INT3Value;
        if(pin == INT4PIN) return INT4Value;
        if(pin == COUNT5) return INT5Value;
	}

	// read from a counter input
	if(ExtCurrentConfig[pin] == EXT_CNT_IN) {
		// select input channel
        if(pin == INT1PIN) return INT1Count;
        if(pin == INT2PIN) return INT2Count;
        if(pin == INT3PIN) return INT3Count;
        if(pin == INT4PIN) return INT4Count;
        if(pin == COUNT5) return Count5High<<32 | ReadCount5();
	}
	return 0;
}



/****************************************************************************************************************************
New, more portable, method of manipulating an I/O pin
*****************************************************************************************************************************/













void *IrDev, *IrCmd;
volatile char IrVarType;
volatile char IrState, IrGotMsg;
int IrBits, IrCount;
char *IrInterrupt;



/****************************************************************************************************************************
Interrupt service routines for the counting functions (eg, frequency, period)
*****************************************************************************************************************************/


// perform the counting functions for INT0
// this interrupt is also used by the IR command and also to wake us from sleep


// perform the counting functions for INT1
void TM_EXTI_Handler_1(void) {
	if(ExtCurrentConfig[INT1PIN] == EXT_PER_IN) {
        if(--INT1Timer <= 0) {
            INT1Value = INT1Count;
            INT1Timer = INT1InitTimer;
            INT1Count = 0;
        }
	}
    else {
        if(CFuncInt1)
            CallCFuncInt1();                                        // Hardware interrupt 2 for a CFunction (see CFunction.c)
        else
            INT1Count++;
    }

}



// perform the counting functions for INT2
void TM_EXTI_Handler_2(void) {
    if(ExtCurrentConfig[INT2PIN] == EXT_PER_IN) {
        if(--INT2Timer <= 0) {
            INT2Value = INT2Count;
            INT2Timer = INT2InitTimer;
            INT2Count = 0;
        }
    }
    else {
        if(CFuncInt2)
            CallCFuncInt2();                                        // Hardware interrupt 2 for a CFunction (see CFunction.c)
        else
            INT2Count++;
    }
}




// perform the counting functions for INT3
void TM_EXTI_Handler_3(void) {
	if(mouse0)MNInterrupt();
		else {
		if(ExtCurrentConfig[INT3PIN] == EXT_PER_IN) {
			if(--INT3Timer <= 0) {
				INT3Value = INT3Count;
				INT3Timer = INT3InitTimer;
				INT3Count = 0;
			}
		}
		else
	        if(CFuncInt3)
	            CallCFuncInt3();                                        // Hardware interrupt 2 for a CFunction (see CFunction.c)
	        else
	            INT3Count++;
		}
}




// perform the counting functions for INT4
void TM_EXTI_Handler_4(void) {
	if(ExtCurrentConfig[INT4PIN] == EXT_PER_IN) {
        if(--INT4Timer <= 0) {
            INT4Value = INT4Count;
            INT4Timer = INT4InitTimer;
            INT4Count = 0;
        }
	}
	else
        if(CFuncInt4)
            CallCFuncInt4();                                        // Hardware interrupt 2 for a CFunction (see CFunction.c)
        else
           	INT4Count++;

}

void TM_EXTI_Handler_5(void){
    int ElapsedMicroSec;
    static unsigned int LastIrBits;
        ElapsedMicroSec = readusclock();
        switch(IrState) {
            case IR_WAIT_START:
                writeusclock(0);                                           // reset the timer
                IrState = IR_WAIT_START_END;                        // wait for the end of the start bit
                break;
            case IR_WAIT_START_END:
                if(ElapsedMicroSec > 2000 && ElapsedMicroSec < 2800)
                    IrState = SONY_WAIT_BIT_START;                  // probably a Sony remote, now wait for the first data bit
                else if(ElapsedMicroSec > 8000 && ElapsedMicroSec < 10000)
                    IrState = NEC_WAIT_FIRST_BIT_START;             // probably an NEC remote, now wait for the first data bit
                else {
                    IrReset();                                      // the start bit was not valid
                    break;
                }
                IrCount = 0;                                        // count the bits in the message
                IrBits = 0;                                         // reset the bit accumulator
                writeusclock(0);                                           // reset the timer
                break;
            case SONY_WAIT_BIT_START:
                if(ElapsedMicroSec < 300 || ElapsedMicroSec > 900) { IrReset(); break; }
                writeusclock(0);                                           // reset the timer
                IrState = SONY_WAIT_BIT_END;                         // wait for the end of this data bit
                break;
            case SONY_WAIT_BIT_END:
                if(ElapsedMicroSec < 300 || ElapsedMicroSec > 1500 || IrCount > 20) { IrReset(); break; }
                IrBits |= (ElapsedMicroSec > 900) << IrCount;       // get the data bit
                IrCount++;                                          // and increment our count
                writeusclock(0);                                           // reset the timer
                IrState = SONY_WAIT_BIT_START;                       // go back and wait for the next data bit
                break;
            case NEC_WAIT_FIRST_BIT_START:
            	if(ElapsedMicroSec > 2000 && ElapsedMicroSec < 2500) {
                    IrBits = LastIrBits;                            // key is held down so just repeat the last code
                    IrCount = 32;                                   // and signal that we are finished
                    IrState = NEC_WAIT_BIT_END;
                    break;
                }
                else if(ElapsedMicroSec > 4000 && ElapsedMicroSec < 5000)
                    IrState = NEC_WAIT_BIT_END;                     // wait for the end of this data bit
                else {
                    IrReset();                                      // the start bit was not valid
                    break;
                }
                writeusclock(0);                                           // reset the timer
                break;
            case NEC_WAIT_BIT_START:
                if(ElapsedMicroSec < 400 || ElapsedMicroSec > 1800) { IrReset(); break; }
                IrBits |= (ElapsedMicroSec > 840) << (31 - IrCount);// get the data bit
                LastIrBits = IrBits;
                IrCount++;                                          // and increment our count
                writeusclock(0);                                           // reset the timer
                IrState = NEC_WAIT_BIT_END;                         // wait for the end of this data bit
                break;
            case NEC_WAIT_BIT_END:
                if(ElapsedMicroSec < 400 || ElapsedMicroSec > 700) { IrReset(); break; }
                if(IrCount == 32) break;
                writeusclock(0);                                           // reset the timer
                IrState = NEC_WAIT_BIT_START;                       // go back and wait for the next data bit
                break;
        }
    }

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_1)
  {
	  TM_EXTI_Handler_1();
  }
  if (GPIO_Pin == GPIO_PIN_2)
  {
	  TM_EXTI_Handler_2();
  }
  if (GPIO_Pin == GPIO_PIN_3)
  {
	  TM_EXTI_Handler_3();
  }
  if (GPIO_Pin == GPIO_PIN_4)
  {
	  TM_EXTI_Handler_4();
  }
  if (GPIO_Pin == GPIO_PIN_12)
  {
	  TM_EXTI_Handler_5();
  }
}
int IsInvalidPin(int pin) {
    if(pin < 1 || pin > NBRPINS) return true;
    if(PinDef[pin].mode & PUNUSED) return true;
    return false;
}

int MIPS16 CheckPin(int pin, int action) {


    if(pin < 1 || pin > NBRPINS || (PinDef[pin].mode & PUNUSED)) {
        if(!(action & CP_NOABORT)) error("Pin % is invalid", pin);
        return false;
    }

    if(!(action & CP_IGNORE_INUSE) && ExtCurrentConfig[pin] > EXT_NOT_CONFIG && ExtCurrentConfig[pin] < EXT_COM_RESERVED) {
        if(!(action & CP_NOABORT)) error("Pin % is in use", pin);
        return false;
    }

    if(!(action & CP_IGNORE_BOOTRES) && ExtCurrentConfig[pin] == EXT_BOOT_RESERVED) {
        if(!(action & CP_NOABORT)) {
            error("Pin % is reserved on startup", pin);
            MM_Delay(1000);
        }
        return false;
    }

    if(!(action & CP_IGNORE_RESERVED) && ExtCurrentConfig[pin] >= EXT_COM_RESERVED) {
        if(!(action & CP_NOABORT)) error("Pin % is in use", pin);
        return false;
    }

    return true;
}
