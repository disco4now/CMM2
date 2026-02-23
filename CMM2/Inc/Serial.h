/***********************************************************************************************************************
MMBasic

Serial.h

Include file that contains the globals and defines for serial.c in MMBasic.

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
