/***************************************************************************
CMM2 MMBasic
Serial.h

Include file that contains the globals and defines for serial.c in MMBasic.

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

#ifndef SERIAL_HEADER
#define SERIAL_HEADER

#define	COM_DEFAULT_BAUD_RATE       9600
#define	COM_DEFAULT_BUF_SIZE	    1024
#define TX_BUFFER_SIZE              256

// global variables
	extern int com1;														// true if COM1 is enabled
	extern int com1_buf_size;													// size of the buffer used to receive chars
	extern int com1_baud;													// determines the baud rate
	extern char *com1_interrupt, *com1_TX_interrupt;												// pointer to the interrupt routine
	extern int com1_TX_complete;
	extern int com1_ilevel;													// number nbr of chars in the buffer for an interrupt
	extern unsigned char *com1Rx_buf;											// pointer to the buffer for received characters
	extern volatile int com1Rx_head, com1Rx_tail;								// head and tail of the ring buffer for com1
	extern unsigned char *com1Tx_buf;											// pointer to the buffer for transmitted characters
	extern volatile int com1Tx_head, com1Tx_tail;								// head and tail of the ring buffer for com1
	extern volatile int com1complete;
	extern uint16_t Rx1Buffer;
	#define COM1_9B       0b001                                         // 9 bit data enabled
	#define COM1_DE       0b010                                         // RS485 enable flag in use
	extern char com2_mode;                                                     // keeps track of the settings for com1
	extern unsigned char com1_bit9;                                        // used to track the 9th bit


	// variables for com2
	extern int com2;														// true if COM2 is enabled
	extern int com2_buf_size;													// size of the buffer used to receive chars
	extern int com2_baud;													// determines the baud rate
	extern char *com2_interrupt, *com2_TX_interrupt;												// pointer to the interrupt routine
	extern int com2_TX_complete;
	extern int com2_ilevel;													// number nbr of chars in the buffer for an interrupt
	extern unsigned char *com2Rx_buf;											// pointer to the buffer for received characters
	extern volatile int com2Rx_head, com2Rx_tail;								// head and tail of the ring buffer for com2 Rx
	extern unsigned char *com2Tx_buf;											// pointer to the buffer for transmitted characters
	extern volatile int com2Tx_head, com2Tx_tail;								// head and tail of the ring buffer for com2 Tx
	extern volatile int com2complete;

	// variables for com3
	extern int com3;														// true if COM2 is enabled
	extern int com3_buf_size;													// size of the buffer used to receive chars
	extern int com3_baud;													// determines the baud rate
	extern char *com3_interrupt, *com3_TX_interrupt;												// pointer to the interrupt routine
	extern int com3_TX_complete;
	extern int com3_ilevel;													// number nbr of chars in the buffer for an interrupt
	extern unsigned char *com3Rx_buf;											// pointer to the buffer for received characters
	extern volatile int com3Rx_head, com3Rx_tail;								// head and tail of the ring buffer for com2 Rx
	extern unsigned char *com3Tx_buf;											// pointer to the buffer for transmitted characters
	extern volatile int com3Tx_head, com3Tx_tail;								// head and tail of the ring buffer for com2 Tx
	extern volatile int com3complete;

	extern uint16_t Rx2Buffer;



// global functions
void SerialOpen(char *spec);
void SerialClose(int comnbr);
unsigned char SerialPutchar(int comnbr, unsigned char c);
int SerialRxStatus(int comnbr);
int SerialTxStatus(int comnbr);
int SerialGetchar(int comnbr);
void setupuart(UART_HandleTypeDef  * huartx, USART_TypeDef  * USART_ID, int de, int inv,int s2,int b9,int b7, int baud);
extern void start_console(void);
extern void stop_console(void);
extern void start_com1(void);
extern void stop_com1e(void);
extern void start_com2(void);
extern void stop_com2(void);
extern void MX_USART1_UART_Init1(void);

#endif
#endif
