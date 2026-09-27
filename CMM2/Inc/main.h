/***************************************************************************
CMM2 MMBasic
main.h
Header for main.c file.
This file contains the common defines of the application.

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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "configuration.h"
#include "ff.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern int SystemError;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Error_Handler(void);
void MX_USART1_UART_Init1(void);
void MX_USART2_UART_Init2(void);
void MX_UART4_Init4(void);
void MMErrorString(char *msg);
extern void cleanend(void);
extern int WWDGdelay;
extern int WWDGdelayCalc;
extern void uSec(unsigned int us);
extern void MM_Delay(int n);
extern uint32_t ticks_per_microsecond;
extern int myDummy;
extern int helpquotes;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SD_LED_Pin GPIO_PIN_11
#define SD_LED_GPIO_Port GPIOI
#define COUNT1_Pin GPIO_PIN_1
#define COUNT1_GPIO_Port GPIOC
#define COUNT2_Pin GPIO_PIN_2
#define COUNT2_GPIO_Port GPIOC
#define COUNT3_Pin GPIO_PIN_3
#define COUNT3_GPIO_Port GPIOC
#define COM2_TX_Pin GPIO_PIN_0
#define COM2_TX_GPIO_Port GPIOA
#define COM1_DE_Pin GPIO_PIN_1
#define COM1_DE_GPIO_Port GPIOA
#define COM1_TX_Pin GPIO_PIN_2
#define COM1_TX_GPIO_Port GPIOA
#define COM1_RX_Pin GPIO_PIN_3
#define COM1_RX_GPIO_Port GPIOA
#define DAC1_Pin GPIO_PIN_4
#define DAC1_GPIO_Port GPIOA
#define DAC2_Pin GPIO_PIN_5
#define DAC2_GPIO_Port GPIOA
#define PWM_1A_Pin GPIO_PIN_6
#define PWM_1A_GPIO_Port GPIOA
#define PWM_1B_Pin GPIO_PIN_7
#define PWM_1B_GPIO_Port GPIOA
#define COUNT4_Pin GPIO_PIN_4
#define COUNT4_GPIO_Port GPIOC
#define PWM_1C_Pin GPIO_PIN_0
#define PWM_1C_GPIO_Port GPIOB
#define SD_WP_Pin GPIO_PIN_12
#define SD_WP_GPIO_Port GPIOH
#define IR_Pin GPIO_PIN_12
#define IR_GPIO_Port GPIOB
#define PWM_2A_Pin GPIO_PIN_6
#define PWM_2A_GPIO_Port GPIOC
#define PWM_2B_Pin GPIO_PIN_7
#define PWM_2B_GPIO_Port GPIOC
#define CONSOLE_TX_Pin GPIO_PIN_9
#define CONSOLE_TX_GPIO_Port GPIOA
#define CONSOLE_RX_Pin GPIO_PIN_10
#define CONSOLE_RX_GPIO_Port GPIOA
#define COM2_RX_Pin GPIO_PIN_14
#define COM2_RX_GPIO_Port GPIOH
#define SD_CLK_Pin GPIO_PIN_10
#define SD_CLK_GPIO_Port GPIOC
#define SD_MISO_Pin GPIO_PIN_11
#define SD_MISO_GPIO_Port GPIOC
#define SD_MOSI_Pin GPIO_PIN_12
#define SD_MOSI_GPIO_Port GPIOC
#define SD_CE_Pin GPIO_PIN_2
#define SD_CE_GPIO_Port GPIOD
#define SD_CD_Pin GPIO_PIN_3
#define SD_CD_GPIO_Port GPIOD
#define SCL_Pin GPIO_PIN_5
#define SCL_GPIO_Port GPIOD
#define SDA_Pin GPIO_PIN_4
#define SDA_GPIO_Port GPIOD
#define INT_Pin GPIO_PIN_7
#define INT_GPIO_Port GPIOD
#define MOUSE_CLK_PIN GPIO_PIN_3
#define MOUSE_CLK_PORT GPIOI
#define MOUSE_DATA_PIN GPIO_PIN_8
#define MOUSE_DATA_PORT GPIOI
#define INT_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */
#define CONSOLE_RX_BUF_SIZE 256
#define CONSOLE_TX_BUF_SIZE 8192                    // this is made a large size so that the serial console does not slow down the USB and LCD consoles
#define  MAX_BMP_FILES  25
#define  MAX_BMP_FILE_NAME 11
#define USARTx                           USART1
#define BREAK_KEY           3                       // the default value (CTRL-C) for the break key.  Reset at the command prompt.
#define forever 1
#define true	1
#define false	0
// used to determine if the exception occured during setup
#define CAUSE_NOTHING           0
#define CAUSE_DISPLAY           1
#define CAUSE_FILEIO            2
#define CAUSE_KEYBOARD          3
#define CAUSE_RTC               4
#define CAUSE_TOUCH             5
#define CAUSE_MMSTARTUP         6
#define MOUSE_CLOCK (G1Hardware & 1 ? 33 : 45)
#define MOUSE_DATA  (G1Hardware & 1 ? 32 : 46)

//#define ID_UNIQUE_ADDRESS        0x1FFF7A10  /*!< STM32F4xx address */
#define ID_UNIQUE_ADDRESS        0x1FF1E800  /*!< STM32H743 address */
#define TM_ID_GetUnique8(x)      ((x >= 0 && x < 12) ? (*(__IO uint8_t *) (ID_UNIQUE_ADDRESS + (x))) : 0)
#define TM_ID_GetUnique32(x)     ((x >= 0 && x < 3) ? (*(__IO uint32_t *) (ID_UNIQUE_ADDRESS + 4 * (x))) : 0)
#define TM_ID_GetUnique64(x)     ((x >= 0 && x < 1) ? (*(__IO uint64_t *) (ID_UNIQUE_ADDRESS + 8 * (x))) : 0)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
