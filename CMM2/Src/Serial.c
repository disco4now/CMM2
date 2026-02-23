/***********************************************************************************************************************
MMBasic

Serial.c

Handles the serial I/O  commands and functions in MMBasic..

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
#include "Hardware_Includes.h"
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;

// variables for com1
int com1 = 0;														// true if COM1 is enabled
int com1_buf_size;													// size of the buffer used to receive chars
int com1_baud = 0;													// determines the baud rate
char *com1_interrupt, *com1_TX_interrupt;												// pointer to the interrupt routine
int com1_ilevel;													// number nbr of chars in the buffer for an interrupt
int com1_TX_complete = false;
unsigned char *com1Rx_buf;											// pointer to the buffer for received characters
volatile int com1Rx_head, com1Rx_tail;								// head and tail of the ring buffer for com1
unsigned char *com1Tx_buf;											// pointer to the buffer for transmitted characters
volatile int com1Tx_head, com1Tx_tail;								// head and tail of the ring buffer for com1
volatile int com1complete=1;
uint16_t Rx1Buffer;
char com1_mode;                                                     // keeps track of the settings for com4
unsigned char com1_bit9 = 0;                                        // used to track the 9th bit
extern uint32_t ticks_per_microsecond;

// variables for com2
int com2 = 0;														// true if COM2 is enabled
int com2_buf_size;													// size of the buffer used to receive chars
int com2_baud = 0;													// determines the baud rate
char *com2_interrupt, *com2_TX_interrupt;												// pointer to the interrupt routine
int com2_ilevel;													// number nbr of chars in the buffer for an interrupt
int com2_TX_complete = false;
unsigned char *com2Rx_buf;											// pointer to the buffer for received characters
volatile int com2Rx_head, com2Rx_tail;								// head and tail of the ring buffer for com2 Rx
unsigned char *com2Tx_buf;											// pointer to the buffer for transmitted characters
volatile int com2Tx_head, com2Tx_tail;								// head and tail of the ring buffer for com2 Tx
volatile int com2complete=1;
char com2_mode;                                                     // keeps track of the settings for com4
unsigned char com2_bit9 = 0;                                        // used to track the 9th bit

// variables for com3
int com3 = 0;														// true if COM2 is enabled
int com3_buf_size;													// size of the buffer used to receive chars
int com3_baud = 0;													// determines the baud rate
char *com3_interrupt, *com3_TX_interrupt;												// pointer to the interrupt routine
int com3_ilevel;													// number nbr of chars in the buffer for an interrupt
int com3_TX_complete = false;
unsigned char *com3Rx_buf;											// pointer to the buffer for received characters
volatile int com3Rx_head, com3Rx_tail;								// head and tail of the ring buffer for com2 Rx
unsigned char *com3Tx_buf;											// pointer to the buffer for transmitted characters
volatile int com3Tx_head, com3Tx_tail;								// head and tail of the ring buffer for com2 Tx
volatile int com3complete=1;
char com3_mode;                                                     // keeps track of the settings for com4
unsigned char com3_bit9 = 0;                                        // used to track the 9th bit
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;

uint16_t Rx2Buffer;
extern uint8_t RxBuffer, TxBuffer;

void start_console(void){
	  if(Option.ConsolePort==3){
		  MX_USART1_UART_Init1();
		  HAL_UART_DeInit(&huart1);
		  huart1.Init.BaudRate = Option.Baudrate;
		  HAL_UART_Init(&huart1);
		  HAL_UART_Receive_IT(&huart1, &RxBuffer, 1);
	  } else if(Option.ConsolePort==1){
	      ExtCfg(COM1_RX_PIN, EXT_BOOT_RESERVED, 0);
	      ExtCfg(COM1_TX_PIN, EXT_BOOT_RESERVED, 0);
		  MX_USART2_UART_Init2();
		  HAL_UART_DeInit(&huart2);
		  huart2.Init.BaudRate = Option.Baudrate;
		  HAL_UART_Init(&huart2);
		  HAL_UART_Receive_IT(&huart2, &RxBuffer, 1);

	  } else if(Option.ConsolePort==2){
	      ExtCfg(COM2_RX_PIN, EXT_BOOT_RESERVED, 0);
	      ExtCfg(COM2_TX_PIN, EXT_BOOT_RESERVED, 0);
		  MX_UART4_Init4();
		  HAL_UART_DeInit(&huart4);
		  huart4.Init.BaudRate = Option.Baudrate;
		  HAL_UART_Init(&huart4);
		  HAL_UART_Receive_IT(&huart4, &RxBuffer, 1);
	  }
}
void stop_console(void){
	  if(Option.ConsolePort==3){
			while(ConsoleTxBufTail != ConsoleTxBufHead);
			MM_Delay(2);
			HAL_UART_DeInit(&huart1);
	  } else if(Option.ConsolePort==1){
			while(ConsoleTxBufTail != ConsoleTxBufHead);
			MM_Delay(2);
			HAL_UART_DeInit(&huart2);
		    ExtCfg(COM1_RX_PIN, EXT_NOT_CONFIG, 0);
		    ExtCfg(COM1_TX_PIN, EXT_NOT_CONFIG, 0);
	  } else if(Option.ConsolePort==2){
			while(ConsoleTxBufTail != ConsoleTxBufHead);
			MM_Delay(2);
			HAL_UART_DeInit(&huart4);
		    ExtCfg(COM2_RX_PIN, EXT_NOT_CONFIG, 0);
		    ExtCfg(COM2_TX_PIN, EXT_NOT_CONFIG, 0);
	  }
}
int get_baudrate(int comport, int timeout){
    float rates[14]={110,300,600,1200,2400,4800,9600,14400,19200,38400,57600,115200,230400, 460800};
    int *t=GetTempMemory(1024),i,j,k,m;
    float a, b,c;
    int pin=0;
    switch(comport){
        case 1:
            if(com1) error("Already open");
            pin=COM1_RX_PIN;
            break;
        case 2:
            if(com2) error("Already open");
            pin=COM2_RX_PIN;
            break;
    }
    WriteCoreTimer(0); j=0;t[j]=0;
    k=PinRead(pin);
    while((ReadCoreTimer() < ticks_per_microsecond * timeout * 1000000) && (j<256)){
    	routinechecks(1);
        m=PinRead(pin);
        if(m!=k){
            t[j]=ReadCoreTimer()*10/ticks_per_microsecond;
            j++;
            k=m;
        }
    }
    if(j>=2){
        c=10000000;
        k=0;
        m=10000000;
        for(i=2;i<j;i++){ //the first pulse may be spurious so ignore
            if(t[i]-t[i-1]<m)m=t[i]-t[i-1];
        }
        a=10000000.0/(float)m;
        for(i=0;i<14;i++){
            b=a/rates[i];
            if(b<1)b=rates[i]/a;
            if(b<c){c=b;k=i;}
        }
        a=rates[k];
    } else a=0;
    return a;
}
void fun_baudrate(void){
    getargs(&ep, 3,",");
    int i= getint(argv[0] ,1 ,3), timeout;
    if(argc==3)timeout=getint(argv[2],1,10);
    else timeout=1;
    iret = get_baudrate(i,timeout);
    targ = T_INT;
}
/***************************************************************************************************
Initialise the serial function including the timer and interrupts.
****************************************************************************************************/

void setupuart(UART_HandleTypeDef * huartx, USART_TypeDef  * USART_ID, int de, int inv,int s2,int parity, int b7, int baud){
	  huartx->Instance = USART_ID;
	  huartx->Init.BaudRate = baud;
	  huartx->Init.StopBits = (s2 ? UART_STOPBITS_2 :UART_STOPBITS_1);
	  if(parity){
		  huartx->Init.WordLength = (b7 ? UART_WORDLENGTH_8B :UART_WORDLENGTH_9B);
		  parity--;
		  huartx->Init.Parity = (parity ? UART_PARITY_ODD : UART_PARITY_EVEN ) ;
	  } else {
		  huartx->Init.WordLength = (b7 ? UART_WORDLENGTH_7B :UART_WORDLENGTH_8B);
		  huartx->Init.Parity = UART_PARITY_NONE;
	  }
	  huartx->Init.Mode = UART_MODE_TX_RX;
	  huartx->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  huartx->Init.OverSampling = UART_OVERSAMPLING_8;
	  huartx->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	  huartx->Init.ClockPrescaler = UART_PRESCALER_DIV2;
	  if(inv){
		  huartx->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT|UART_ADVFEATURE_RXINVERT_INIT;
		  huartx->AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
		  huartx->AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
	  } else huartx->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXINVERT_INIT;
	  if(USART_ID == USART2 && de==1){
		  if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_HIGH, 0b11111, 0b11111) != HAL_OK)
	  	  	  {
	  		  	error("UART");
	  	  	  }
	  } else if(USART_ID == USART2 && de==2){
		  if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_LOW, 0b11111, 0b11111) != HAL_OK)
	  	  	  {
	  		  	error("UART");
	  	  	  }
	  } else {
		  if (HAL_UART_Init(huartx) != HAL_OK)
	  	  	  {
	  		  	error("UART");
	  	  	  }
	  }
	  if (HAL_UARTEx_SetTxFifoThreshold(huartx, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	  {
	    SystemError=1;Error_Handler();
	  }
	  if (HAL_UARTEx_SetRxFifoThreshold(huartx, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	  {
	    SystemError=1;Error_Handler();
	  }
	  if (HAL_UARTEx_EnableFifoMode(huartx) != HAL_OK)
	  {
	    SystemError=1;Error_Handler();
	  }
}
/***************************************************************************************************
Initialise the serial function including the timer and interrupts.
****************************************************************************************************/
void SerialOpen(char *spec) {
	int baud, i, inv, oc, s2, de, parity, b7, bufsize, ilevel;
	char *interrupt, *TXinterrupt;
	GPIO_InitTypeDef GPIO_InitStruct;

	getargs(&spec, 21, ":,");										// this is a macro and must be the first executable stmt
	if(argc != 2 && (argc & 0x01) == 0) error("COM specification");

    b7 = de = parity = inv = oc = s2 = false;
    for(i = 0; i < 6; i++) {
    	if(str_equal(argv[argc - 1], "OC")) { oc = true; argc -= 2; }	// get the open collector option
    	if(str_equal(argv[argc - 1], "DEP")) { de = 1; argc -= 2; }	// get the data enable option
    	if(str_equal(argv[argc - 1], "DEN")) {
    		if(de)error("DE Can't both be negative and positive");
    		de = 2;
    		argc -= 2;
    	}	// get the data enable option
    	if(str_equal(argv[argc - 1], "EVEN")) {
    		if(parity)error("Syntax");
    		else {parity = 1; argc -= 2; }	// set even parity
    	}
    	if(str_equal(argv[argc - 1], "ODD")) {
    		if(parity)error("Syntax");
    		else {parity = 2; argc -= 2; }	// set even parity
    	}
    	if(str_equal(argv[argc - 1], "S2")) { s2 = true; argc -= 2; }	// get the two stop bit option
    	if(str_equal(argv[argc - 1], "7BIT")) { b7 = true; argc -= 2; }	// set the 7 bit byte option
    	if(str_equal(argv[argc - 1], "INV")) { inv = true; argc -= 2; }	// get the invert option
    }

	if(argc < 1 || argc > 13) error("COM specification");

	if(argc >= 3 && *argv[2]) {
		baud = getinteger(argv[2]);									// get the baud rate as a number
		if(baud<1200)error("1200 baud is minimum supported");
	} else
		baud = COM_DEFAULT_BAUD_RATE;

	if(argc >= 5 && *argv[4])
		bufsize = getinteger(argv[4]);								// get the buffer size as a number
	else
		bufsize = COM_DEFAULT_BUF_SIZE;

	if(argc >= 7) {
    	InterruptUsed = true;
    	argv[6]=strupr(argv[6]);
		interrupt = GetIntAddress(argv[6]);							// get the interrupt location
	} else
		interrupt = NULL;

	if(argc >= 9) {
		ilevel = getinteger(argv[8]);								// get the buffer level for interrupt as a number
		if(ilevel < 1 || ilevel > bufsize) error("COM specification");
	} else
		ilevel = 1;

	if(argc >= 11) {
    	InterruptUsed = true;
    	argv[6]=strupr(argv[10]);
		TXinterrupt = GetIntAddress(argv[10]);							// get the interrupt location
	} else
		TXinterrupt = NULL;


	if(spec[3] == '1') {
	///////////////////////////////// this is COM1 ////////////////////////////////////

		if(com1) error("Already open");
        CheckPin(COM1_RX_PIN, CP_CHECKALL);
        CheckPin(COM1_TX_PIN, CP_CHECKALL);

 		com1_buf_size = bufsize;									// extracted from the comspec above
		com1_interrupt = interrupt;
		com1_ilevel	= ilevel;
		com1_TX_interrupt = TXinterrupt;
		com1_TX_complete = false;

		// setup for receive
		com1Rx_buf = GetMemory(com1_buf_size);						// setup the buffer
		com1Rx_head = com1Rx_tail = 0;
		ExtCfg(COM1_RX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use


		// setup for transmit
		com1Tx_buf = GetMemory(TX_BUFFER_SIZE);						// setup the buffer
		com1Tx_head = com1Tx_tail = 0;
		ExtCfg(COM1_TX_PIN, EXT_COM_RESERVED, 0);
		com1_bit9 = com1_mode = 0;
        if(parity)  com1_mode |= COM1_9B;
        if(de) {
            CheckPin(COM1_EN_PIN, CP_CHECKALL);
            ExtCfg(COM1_EN_PIN, EXT_COM_RESERVED, 0);               // reserve the pin for com use
            com1_mode |= COM1_DE;
       }

        setupuart(&huart2, USART2, de, inv, s2, parity, b7, baud);
        if(oc){
        	GPIO_InitStruct.Pin = COM1_TX_Pin;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        	GPIO_InitStruct.Pull = GPIO_NOPULL;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        	HAL_GPIO_Init(COM1_TX_GPIO_Port, &GPIO_InitStruct);
        }
        if(Option.SerialPullup){
        	GPIO_InitStruct.Pin = COM1_RX_Pin;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        	GPIO_InitStruct.Pull = GPIO_PULLUP;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        	HAL_GPIO_Init(COM1_RX_GPIO_Port, &GPIO_InitStruct);
        }
        if(de){
        	GPIO_InitStruct.Pin = COM1_DE_Pin;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        	HAL_GPIO_Init(COM1_DE_GPIO_Port, &GPIO_InitStruct);
        }
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        huart2.Instance->CR1 |= USART_CR1_RXNEIE;
        com1 = true;
	}
	else if (spec[3] == '2') {
	///////////////////////////////// this is COM2 ////////////////////////////////////

		if(com2) error("Already open");
        CheckPin(COM2_RX_PIN, CP_CHECKALL);
        CheckPin(COM2_TX_PIN, CP_CHECKALL);

 		com2_buf_size = bufsize;									// extracted from the comspec above
		com2_interrupt = interrupt;
		com2_ilevel	= ilevel;
		com2_TX_interrupt = TXinterrupt;
		com2_TX_complete = false;

		// setup for receive
		com2Rx_buf = GetMemory(com2_buf_size);						// setup the buffer
		com2Rx_head = com2Rx_tail = 0;
		ExtCfg(COM2_RX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use


		// setup for transmit
		com2Tx_buf = GetMemory(TX_BUFFER_SIZE);						// setup the buffer
		com2Tx_head = com2Tx_tail = 0;
		ExtCfg(COM2_TX_PIN, EXT_COM_RESERVED, 0);                   // reserve the pin for com use
		com2_bit9 = com2_mode = 0;
        if(de) {
        	error("Not available on COM2");
        }
        setupuart(&huart4, UART4, de, inv, s2, parity, b7, baud);
        if(oc){
        	GPIO_InitStruct.Pin = COM2_TX_Pin;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        	GPIO_InitStruct.Pull = GPIO_NOPULL;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        	HAL_GPIO_Init(COM2_TX_GPIO_Port, &GPIO_InitStruct);
        }
        if(Option.SerialPullup){
        	GPIO_InitStruct.Pin = COM2_RX_Pin;
        	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        	GPIO_InitStruct.Pull = GPIO_PULLUP;
        	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        	GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        	HAL_GPIO_Init(COM2_RX_GPIO_Port, &GPIO_InitStruct);
        }
        HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
        huart4.Instance->CR1 |= USART_CR1_RXNEIE;
        com2 = true;
	}
	else if (spec[3] == '3') {
///////////////////////////////// this is COM3 ////////////////////////////////////

	if(com3) error("Already open");
	if(Option.ConsolePort==3 && (Option.Console & 1 || OptionConsole & 1))error("Serial Console must be disabled");
	com3_buf_size = bufsize;									// extracted from the comspec above
	com3_interrupt = interrupt;
	com3_ilevel	= ilevel;
	com3_TX_interrupt = TXinterrupt;
	com3_TX_complete = false;

	// setup for receive
	com3Rx_buf = GetMemory(com3_buf_size);						// setup the buffer
	com3Rx_head = com3Rx_tail = 0;


	// setup for transmit
	com3Tx_buf = GetMemory(TX_BUFFER_SIZE);						// setup the buffer
	com3Tx_head = com2Tx_tail = 0;
	com3_bit9 = com3_mode = 0;
    if(de) {
    	error("Not available on COM3");
    }
    setupuart(&huart1, USART1, de, inv, s2, parity, b7, baud);
    if(oc){
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    if(Option.SerialPullup){
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    huart1.Instance->CR1 |= USART_CR1_RXNEIE;
    com3 = true;
	} else error("Invalid COM port");
}




/***************************************************************************************************
Close a serial port.
****************************************************************************************************/
void SerialClose(int comnbr) {

	if(comnbr == 1 && com1) {
		HAL_UART_DeInit(&huart2);
		if(com1_mode & COM1_DE) {
			ExtCfg(COM1_EN_PIN, EXT_NOT_CONFIG, 0);
		}
		com1 = false;
		com1_interrupt = NULL;
        PinSetBit(COM1_RX_PIN, CNPUCLR);                            // clear the pullup or pulldown on Rx
        PinSetBit(COM1_RX_PIN, CNPDCLR);
		ExtCfg(COM1_RX_PIN, EXT_NOT_CONFIG, 0);
		ExtCfg(COM1_TX_PIN, EXT_NOT_CONFIG, 0);
		FreeMemorySafe((void *)&com1Rx_buf);
		FreeMemorySafe((void *)&com1Tx_buf);
	}

	else if(comnbr == 2 && com2) {
		HAL_UART_DeInit(&huart4);
		com2 = false;
		com2_interrupt = NULL;
        PinSetBit(COM2_RX_PIN, CNPUCLR);                            // clear the pullup or pulldown on Rx
        PinSetBit(COM2_RX_PIN, CNPDCLR);
		ExtCfg(COM2_RX_PIN, EXT_NOT_CONFIG, 0);
		ExtCfg(COM2_TX_PIN, EXT_NOT_CONFIG, 0);
		FreeMemorySafe((void *)&com2Rx_buf);
		FreeMemorySafe((void *)&com2Tx_buf);
	}

	else if(comnbr == 3 && com3) {
		HAL_UART_DeInit(&huart1);
		com3 = false;
		com3_interrupt = NULL;
		FreeMemorySafe((void *)&com3Rx_buf);
		FreeMemorySafe((void *)&com3Tx_buf);
	}
}



/***************************************************************************************************
Add a character to the serial output buffer.
****************************************************************************************************/
unsigned char SerialPutchar(int comnbr, unsigned char c) {
	if(comnbr == 1) {
        int empty=(huart2.Instance->ICR & USART_ICR_TCCF) | !(huart2.Instance->CR1 & USART_CR1_TCIE) ;
		while(com1Tx_tail == ((com1Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com1Tx_buf[com1Tx_head] = c;							// add the char
		com1Tx_head = (com1Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart2.Instance->CR1 |= USART_CR1_TCIE;
		}
	}
	else if(comnbr == 2) {
        int empty=(huart4.Instance->ICR & USART_ICR_TCCF) | !(huart4.Instance->CR1 & USART_CR1_TCIE) ;
		while(com2Tx_tail == ((com2Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com2Tx_buf[com2Tx_head] = c;							// add the char
		com2Tx_head = (com2Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart4.Instance->CR1 |= USART_CR1_TCIE;
		}
	}
	else if(comnbr == 3) {
        int empty=(huart1.Instance->ICR & USART_ICR_TCCF) | !(huart1.Instance->CR1 & USART_CR1_TCIE) ;
		while(com3Tx_tail == ((com3Tx_head + 1) % TX_BUFFER_SIZE)); //wait if buffer full
		com3Tx_buf[com3Tx_head] = c;							// add the char
		com3Tx_head = (com3Tx_head + 1) % TX_BUFFER_SIZE;		   // advance the head of the queue
		if(empty){
	        huart1.Instance->CR1 |= USART_CR1_TCIE;
		}
	}
	return c;
}



/***************************************************************************************************
Get the status the serial receive buffer.
Returns the number of characters waiting in the buffer
****************************************************************************************************/
int SerialRxStatus(int comnbr) {
	int i = 0;
	if(comnbr == 1) {
	    huart2.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com1Rx_head - com1Rx_tail;
	    huart2.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com1_buf_size;
	}
	else if(comnbr == 2) {
	    huart4.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com2Rx_head - com2Rx_tail;
	    huart4.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com2_buf_size;
	}
	else if(comnbr == 3) {
	    huart1.Instance->CR1 &= ~USART_CR1_RXNEIE;
		i = com3Rx_head - com3Rx_tail;
	    huart1.Instance->CR1 |= USART_CR1_RXNEIE;
		if(i < 0) i += com3_buf_size;
	}

	return i;
}


/***************************************************************************************************
Get the status the serial transmit buffer.
Returns the number of characters waiting in the buffer
****************************************************************************************************/
int SerialTxStatus(int comnbr) {
	int i = 0;
	if(comnbr == 0) {
		i = ConsoleTxBufHead - ConsoleTxBufTail;
		if(i < 0) i += CONSOLE_TX_BUF_SIZE;
	}
	if(comnbr == 1) {
		i = com1Tx_head - com1Tx_tail;
		if(i < 0) i += TX_BUFFER_SIZE;
	}
	else if(comnbr == 2) {
		i = com2Tx_head - com2Tx_tail;
		if(i < 0) i += TX_BUFFER_SIZE;
	}
	else if(comnbr == 3) {
		i = com3Tx_head - com3Tx_tail;
		if(i < 0) i += TX_BUFFER_SIZE;
	}
	return i;
}



/***************************************************************************************************
Get a character from the serial receive buffer.
Note that this is returned as an integer and -1 means that there are no characters available
****************************************************************************************************/
int SerialGetchar(int comnbr) {
	int c;
    c = -1;                                                         // -1 is no data
	if(comnbr == 1) {
	    huart2.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com1Rx_head != com1Rx_tail) {                            // if the queue has something in it
			c = com1Rx_buf[com1Rx_tail];                            // get the char
 			com1Rx_tail = (com1Rx_tail + 1) % com1_buf_size;        // and remove from the buffer
		}
	    huart2.Instance->CR1 |= USART_CR1_RXNEIE;
	}
	else if(comnbr == 2) {

	    huart4.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com2Rx_head != com2Rx_tail) {                            // if the queue has something in it
			c = com2Rx_buf[com2Rx_tail];                            // get the char
 			com2Rx_tail = (com2Rx_tail + 1) % com2_buf_size;        // and remove from the buffer
		}
	    huart4.Instance->CR1 |= USART_CR1_RXNEIE;
	}
	else if(comnbr == 3) {

	    huart1.Instance->CR1 &= ~USART_CR1_RXNEIE;
		if(com3Rx_head != com3Rx_tail) {                            // if the queue has something in it
			c = com3Rx_buf[com3Rx_tail];                            // get the char
 			com3Rx_tail = (com3Rx_tail + 1) % com3_buf_size;        // and remove from the buffer
		}
	    huart1.Instance->CR1 |= USART_CR1_RXNEIE;
	}
	return c;
}

