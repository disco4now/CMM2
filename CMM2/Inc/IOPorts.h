/***********************************************************************************************************************
MMBasic

IOPorts.h

Include file that defines the IOPins for the PIC32 chip in MMBasic.

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

#ifndef IOPORTS_H
#define IOPORTS_H

// these are the valid peek/poke memory ranges for the STM32F407VG
#define RAM(a)  ((a >= RAMBASE && a < RAMEND) || (a >= 0x20000000 && a < 0x20020000) || (a >= 0xd0000000 && a < (G1Hardware ?0xd0800000 : 0xd2000000)) || (a >= 0x24000000 && a < 0x24080000) || (a >= 0x38000000 && a<  0x38010000) || (a >= 0x38800000 && a<  0x38801000)|| (a >= 0x40000000 && a < 0x59000000) )
#define ROM(a)  ((a >= 0x08000000 && a < 0x08200000) || (a >= 0x10000000 && a < 0x20000000))
#define PEEKRANGE(a) (RAM(a) || ROM(a))
#define POKERANGE(a) (RAM(a))
// General defines
#define P_INPUT				1						// for setting the TRIS on I/O bits
#define P_OUTPUT			0
#define P_ON				1
#define P_OFF				0
// Structure that defines the SFR, bit number and mode for each I/O pin
struct s_PinDef {
	GPIO_TypeDef *sfr;
    unsigned int bitnbr;
    unsigned char mode;
    ADC_TypeDef *  ADC;
    uint32_t ADCchannel;
};
typedef struct s_PinDef PinDefAlias;

// Defines for the various modes that an I/O pin can be set to
#define PUNUSED       1
#define ANALOG_IN    2
#define DIGITAL_IN   4
#define COUNTING     8
#define INTERRUPT    16
#define DIGITAL_OUT  32
#define OC_OUT       64
#define DO_NOT_RESET 128
//#define HAS_64PINS 0
#define NBRPINS             46

#define NBRINTPINS          10
#if defined(DEFINE_PINDEF_TABLE)
const struct s_PinDef PinDef40[NBRPINS+1]={
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 0
		{ NULL,  0, PUNUSED , NULL, 0},                                                         // pin 1 3V3
		{ NULL,  0, PUNUSED , NULL, 0},                                                         // pin 2 5V
        { GPIOB,  GPIO_PIN_9,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 3 I2C-SDA
		{ NULL,  0, PUNUSED , NULL, 0},                                                         // pin 4 5V
        { GPIOB,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 5 I2C-SCK
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 6 GND
        { GPIOC,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_11},    // pin 7 COUNT1
		{ GPIOA,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_14},    // pin 8 COM1-TX
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 9 GND
		{ GPIOA,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_15},    // pin 10 COM1-RX
        { GPIOH,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 11 COM2-RX
		{ GPIOA,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_3},     // pin 12 PWM-1A
		{ GPIOC,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_0},     // pin 13 COUNT2,I2S-SDI
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 14 GND
		{ GPIOC,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_1},     // pin 15 COUNT3,I2S-SDO
		{ GPIOA,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_16},    // pin 16 COM2-TX
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 17 3V3
        { GPIOA,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 18 COUNT5 - FAST
        { GPIOB,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 19 SPI-OUT
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 20 GND
        { GPIOB,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 21 SPI-IN
        { GPIOA,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_7},     // pin 22 PWM-1B
        { GPIOB,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 23 SPI-CLK
		{ GPIOC,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_4},     // pin 24 COUNT4
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 25 GND
		{ GPIOC,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_8},     // pin 26
        { GPIOB,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 27 I2C2-SDA
        { GPIOH,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 28 I2C2-SCK
		{ GPIOB,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_9},     // pin 29 PWM-1C
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 30 GND
        { GPIOC,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 31 PWM-2B
        { GPIOI,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 32
        { GPIOI,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 33
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 34 GND
        { GPIOB,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 35 SPI2-IN
        { GPIOC,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 36 PWM-2A
        { GPIOA,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_17},    // pin 37 COM1-DE
        { GPIOB,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 38 SPI2-OUT
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 39 GND
        { GPIOB,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 40 SPI2-CLK
        { GPIOB,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 41 IR
        { GPIOE,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 42 DS18B20
        { GPIOB,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 43 Nunchuck SDA
        { GPIOH,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_15},    // pin 44 Nunchuck SCK
        { GPIOE,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 45 G2 Mouse CLK
        { GPIOA,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 46 G2 Mouse DATA
};


#else
	const extern struct s_PinDef PinDef40[];
#endif      // DEFINE_PINDEF_TABLE
    extern struct s_PinDef *PinDef;
// Define the counting pin numbers
// INT1PIN refers to the PIC32 external interrupt #1, an so on for the others
#define INT1PIN               7 //PC1
#define INT2PIN              13 //PC2
#define INT3PIN              15 //PC3
#define INT4PIN              24 //PC4
#define IRPIN                41 //PB12
#define COUNT5				 18 //PA15
// I2C pin numbers
#define P_I2C_SCL           5  //PB8
#define P_I2C_SDA           3  //PB9
#define P_I2C2_SCL          28 //PH11
#define P_I2C2_SDA          27 //PB7
#define P_I2C3_SCL          44 //PH4 Nunchuck
#define P_I2C3_SDA          43 //PB11

// COMx: port pin numbers

#define COM1_TX_PIN         8  //PA2 USART2
#define COM1_RX_PIN         10 //PA3
#define COM1_EN_PIN			37 //PA1

#define COM2_TX_PIN         16  //PA0 UART4
#define COM2_RX_PIN         11  //PH14

// SPI pin numbers
#define SPI_INP_PIN         21  //PB4
#define SPI_OUT_PIN         19  //PB5
#define SPI_CLK_PIN         23  //PB3
// SPI2 pin numbers
#define SPI2_INP_PIN        35  //PB14
#define SPI2_OUT_PIN        38  //PB15
#define SPI2_CLK_PIN        40  //PB13
//
// PWM pin numbers
#define PWM_CH1_PIN         12 //PA6  - PWM1A
#define PWM_CH2_PIN         22 //PA7  - PWM1B
#define PWM_CH3_PIN         29 //PB0  - PWM1C
#define PWM_CH4_PIN         36 //PC6  - PWM2A
#define PWM_CH5_PIN         31 //PC7  - PWM2B
//CAN Pins
#define CAN_1A_RX           5  //PB8  Shared with I2C_SCL
#define CAN_1A_TX           3  //PB9  shared with I2C_SDA
#define CAN_2A_RX           19 //PB5  shared with SPI MOSI
#define CAN_2A_TX           40 //PB13 shared with SPI2 CLK


#endif

