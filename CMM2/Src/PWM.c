/***************************************************************************

CMM2 MMBasic
PWM.c

Handles the PWM and SERVO commands

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
#include "main.h"
//TM_PWM_TIM_t TIM4_Data, TIM3_Data;

static unsigned int f1=0xFFFFFFFF, f2=0xFFFFFFFF;            // a place to save the last used frequency
char oc1, oc2, oc3, oc4, oc5;
//char s[20];
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim8;
static int dcy[2][3];
static int invertchannel[2][3]={0};

// flags which are true is the OCx module is opened

// the PWM and SERVO commands use the same function
void cmd_pwm(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_OC_InitTypeDef sConfigOC;
	int i, j, channel, f, centre=0;
	MMFLOAT  a, duty;
	int prescale=0, counts;

	getargs(&cmdline, 11, ",");
	if((argc & 0x01) == 0 || argc < 3) error("Invalid syntax");

    channel = getint(argv[0], 1, 2)-1;

    if(checkstring(argv[2], "STOP")){
        PWMClose(channel);
        return;
    }
    if(cmdtoken == GetCommandValue("PWM")) {
        if(argc==11)centre=getint(argv[10],0,1);
        //  f = getint(argv[2], 1, 20000000)*(centre+1);
        f = getint(argv[2],-20000000, 20000000);
	    if (f==0)error("frequency cannot be 0");
        f=f*(centre+1);
        do {
        	prescale++;
        	//counts=round((double)SystemCoreClock /(double)2.0/(double)prescale/(double)f);
        	counts=round((double)SystemCoreClock /(double)2.0/(double)prescale/(double)abs(f));
        } while(counts>65535);
        counts--;
        prescale--;
        j = 4;
        for(i = 0; i < (3); i++, j += 2) {
            if(argc < j  || !*argv[j])
                dcy[channel][i] = -1;
            else {
                duty = getnumber(argv[j]);
                if(duty < -100.0 || duty > 100.0)error("Number out of bounds");
            	invertchannel[channel][i]=0;
                if(duty<0){
                	duty=-duty;
                	invertchannel[channel][i]=1;
                }
                dcy[channel][i] = duty * 100.0;
                if(duty == 100.0) dcy[channel][i] = 10100;
                dcy[channel][i] = (counts * dcy[channel][i]) / 10000;
            }
        }
    } else {
        // Command must be SERVO
        f = getinteger(argv[2]);
        if (f<0)error("OC not supported for SERVO");
        if(f >= 20) { //must be a frequency
            if(f > 1000) error("% out of bounds", f);
            j = 4;
        } else {
            f = 50;
            j = 2;
        }
        do {
        	prescale++;
        	counts=round((double)SystemCoreClock /(double)2.0/(double)prescale/(double)f);
        } while(counts>65535);
        counts--;
        prescale--;

        for(i = 0; i < 3; i++, j += 2) {
            if(argc < j)
                dcy[channel][i] = -1;
            else {
                long double ontime = getnumber(argv[j]);
                if(ontime < 0.01 || ontime > 18.9) error("Number out of bounds");
                a=((double)1.0)/((double)f) * ((double)1000.0);
                duty = ontime / a *((double)100.0);
                if(duty>99.9) error("Number out of bounds");
                dcy[channel][i] = duty * 100.0;
                dcy[channel][i] = (counts * dcy[channel][i]) / 10000;
            }
        }
    }
    if(channel==0 && f!=f1){ //reset the clock if needed
//        PInt(prescale);PIntComma(counts);
    	htim3.Instance = TIM3;
    	htim3.Init.Prescaler = prescale;
    	htim3.Init.CounterMode = (centre ? TIM_COUNTERMODE_CENTERALIGNED3 :TIM_COUNTERMODE_UP);
    	htim3.Init.Period = counts;
    	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    	{
    		error("HAL_TIM_Base_Init(&htim3)");
    	}

    	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    	{
    		error("HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig)");
    	}

    	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    	{
    		error("HAL_TIM_PWM_Init(&htim3)");
    	}

    	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    	{
    		error("HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)");
    	}
    } else if(f!=f2){
    	htim8.Instance = TIM8;
    	htim8.Init.Prescaler = prescale;
    	htim8.Init.CounterMode =  (centre ? TIM_COUNTERMODE_CENTERALIGNED3 :TIM_COUNTERMODE_UP);
    	htim8.Init.Period = counts;
    	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    	htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    	if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
    	{
    		error("HAL_TIM_Base_Init(&htim8)");
    	}

    	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    	if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
    	{
    		error("HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig)");
    	}

    	if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
    	{
    		error("HAL_TIM_PWM_Init(&htim8)");
    	}

    	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    	if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
    	{
    		error("HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig)");
    	}
    }

    // this is channel 1
    if(channel == 0) {
        // this is output 1A
        if(!oc1) {
//        	PIntComma(dcy[0][0]);MMPrintString("\r\n");
            ExtCfg(PWM_CH1_PIN, EXT_COM_RESERVED, 0);
            sConfigOC.OCMode = TIM_OCMODE_PWM1;
            sConfigOC.Pulse = dcy[0][0];
            sConfigOC.OCPolarity = (invertchannel[channel][0] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH);
            sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
            if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
            {
              error("HAL_TIM_PWM_ConfigChannel");
            }
            GPIO_InitStruct.Pin = PWM_1A_Pin;
           // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			if (f<0){
				//testing to allow config for Open Drain.
			    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; //'OPEN DRAIN
			}else{
			   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			}
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
            HAL_GPIO_Init(PWM_1A_GPIO_Port, &GPIO_InitStruct);
            /* Start channel 1 */
            if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK)
            {
              /* PWM Generation Error */
            	error("HAL_TIM_PWM_Start");
            }
            f1=f;
            oc1 = true;
            oc2 = false; oc3=false;
        } else {
        	htim3.Instance->CCR1 = dcy[0][0];
        	htim3.Instance->CCER &=~(1<<1);
        	htim3.Instance->CCER |= (invertchannel[0][0] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH)<<1;
        }

        // this is output 1B
        if(dcy[0][1] >= 0) {
            if(!oc2 || f1!=f) {
                ExtCfg(PWM_CH2_PIN, EXT_COM_RESERVED, 0);
                sConfigOC.OCMode = TIM_OCMODE_PWM1;
                sConfigOC.Pulse = dcy[0][1];
                sConfigOC.OCPolarity = (invertchannel[channel][1] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH);
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
                if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
                {
                  error("HAL_TIM_PWM_ConfigChannel");
                }
                GPIO_InitStruct.Pin = PWM_1B_Pin;
              //  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    			if (f<0){
    				//testing to allow config for Open Drain.
    			    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; //'OPEN DRAIN
    			}else{
    			   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    			}
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
                HAL_GPIO_Init(PWM_1B_GPIO_Port, &GPIO_InitStruct);
                /* Start channel 1 */
                if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) != HAL_OK)
                {
                  /* PWM Generation Error */
                	error("HAL_TIM_PWM_Start");
                }
                oc2 = true;
            } else {
            	htim3.Instance->CCR1 = dcy[0][1];
            	htim3.Instance->CCER &=~(1<<5);
            	htim3.Instance->CCER |= (invertchannel[0][1] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH)<<5;
            }
        }
        // this is output 1C
        if(dcy[0][2] >= 0) {
            if(!oc3) {
                ExtCfg(PWM_CH3_PIN, EXT_COM_RESERVED, 0);
                sConfigOC.OCMode = TIM_OCMODE_PWM1;
                sConfigOC.Pulse = dcy[0][2];
                sConfigOC.OCPolarity = (invertchannel[channel][2] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH);
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
                if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
                {
                  error("HAL_TIM_PWM_ConfigChannel");
                }
                GPIO_InitStruct.Pin = PWM_1C_Pin;
               // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    			if (f<0){
    				//testing to allow config for Open Drain.
    			    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; //'OPEN DRAIN
    			}else{
    			   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    			}
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
                HAL_GPIO_Init(PWM_1C_GPIO_Port, &GPIO_InitStruct);
                /* Start channel 1 */
                if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3) != HAL_OK)
                {
                  /* PWM Generation Error */
                	error("HAL_TIM_PWM_Start");
                }
                oc3 = true;
            } else {
            	htim3.Instance->CCR1 = dcy[0][2];
            	htim3.Instance->CCER &=~(1<<9);
            	htim3.Instance->CCER |= (invertchannel[0][2] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH)<<9;
            }
        }
        // now set the timer for channel 2
    } else {
        // this is channel 2A
        if(!oc4 || f2!=f) {
            ExtCfg(PWM_CH4_PIN, EXT_COM_RESERVED, 0);
            sConfigOC.OCMode = TIM_OCMODE_PWM1;
            sConfigOC.Pulse = dcy[1][0];
            sConfigOC.OCPolarity = (invertchannel[channel][0] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH);
            sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
            if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
            {
              error("HAL_TIM_PWM_ConfigChannel");
            }
            GPIO_InitStruct.Pin = PWM_2A_Pin;
           // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			if (f<0){
				//testing to allow config for Open Drain.
			    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; //'OPEN DRAIN
			}else{
			   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			}
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
            HAL_GPIO_Init(PWM_2A_GPIO_Port, &GPIO_InitStruct);
            /* Start channel 1 */
            if (HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1) != HAL_OK)
            {
              /* PWM Generation Error */
            	error("HAL_TIM_PWM_Start");
            }
            f2=f;
            oc5 = false;
            oc4 = true;
        } else {
        	htim8.Instance->CCR1 = dcy[1][0];
        	htim8.Instance->CCER &=~(1<<1);
        	htim8.Instance->CCER |= (invertchannel[1][0] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH)<<1;
        }

        // this is output 2B
        if(dcy[1][1] >= 0) {
            if(!oc5) {
                ExtCfg(PWM_CH5_PIN, EXT_COM_RESERVED, 0);
                sConfigOC.OCMode = TIM_OCMODE_PWM1;
                sConfigOC.Pulse = dcy[1][1];
                sConfigOC.OCPolarity = (invertchannel[channel][1] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH);
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
                if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
                {
                  error("HAL_TIM_PWM_ConfigChannel");
                }
                GPIO_InitStruct.Pin = PWM_2B_Pin;
 //               GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    			if (f<0){
    				//testing to allow config for Open Drain.
    			    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; //'OPEN DRAIN
    			}else{
    			   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    			}
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
                HAL_GPIO_Init(PWM_2B_GPIO_Port, &GPIO_InitStruct);
                /* Start channel 1 */
                if (HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2) != HAL_OK)
                {
                  /* PWM Generation Error */
                	error("HAL_TIM_PWM_Start");
                }
                oc5 = true;
            } else {
            	htim8.Instance->CCR1 = dcy[1][1];
            	htim8.Instance->CCER &=~(1<<5);
            	htim8.Instance->CCER |= (invertchannel[1][1] ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH)<<5;
            }
        }
    }
}


// close the PWM output
void PWMClose(int channel) {
    switch(channel) {
        case 0:
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
            HAL_TIM_Base_DeInit(&htim3);
            if(oc1) ExtCfg(PWM_CH1_PIN, EXT_NOT_CONFIG, 0);
            if(oc2) ExtCfg(PWM_CH2_PIN, EXT_NOT_CONFIG, 0);
            if(oc3) ExtCfg(PWM_CH3_PIN, EXT_NOT_CONFIG, 0);
            oc1 = oc2 = oc3 = false;
            f1=0;
            break;
        case 1:
            HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
            HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
            HAL_TIM_Base_DeInit(&htim8);
            if(oc4) ExtCfg(PWM_CH4_PIN, EXT_NOT_CONFIG, 0);
            if(oc5) ExtCfg(PWM_CH5_PIN, EXT_NOT_CONFIG, 0);
            oc4 = oc5 = false;
            f2=0;
            break;
    }
}
