/***********************************************************************************************************************
MMBasic

PWM.c

Handles the PWM command.

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

unsigned int *GetSendDataList(char *p, unsigned int *nbr);
long long int *GetReceiveDataBuffer(char *p, unsigned int *nbr);
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

//void cmd_spi1(char *spi) {
void cmd_spi(void) {

	    int speed;
    char *p;
    unsigned int nbr, *d;
    long long int *dd;

    if(checkstring(cmdline, "CLOSE")) {
        SPIClose();
        return;
    }

    if((p = checkstring(cmdline, "WRITE")) != NULL) {
    	union car
    	{
    		uint32_t aTxBuffer;
    		uint8_t cTxBuffer[4];
    	}mybuff;
       if(ExtCurrentConfig[SPI_OUT_PIN] != EXT_COM_RESERVED) error("Not open");
        d = GetSendDataList(p, &nbr);
        while(nbr--) {
        	mybuff.aTxBuffer=*d++;
        	HAL_SPI_Transmit(&hspi1, mybuff.cTxBuffer,1, 5000);
        }
        return;
    }

    if((p = checkstring(cmdline, "READ")) != NULL) {
    	union car
    	{
    		uint32_t aRxBuffer;
    		uint8_t cRxBuffer[4];
    	}mybuff;
        if(ExtCurrentConfig[SPI_OUT_PIN] != EXT_COM_RESERVED) error("Not open");
        dd = GetReceiveDataBuffer(p, &nbr);
        while(nbr--) {
        	mybuff.aRxBuffer=0;
        	HAL_SPI_Receive(&hspi1, mybuff.cRxBuffer, 1, 5000);
        	*dd++ = mybuff.aRxBuffer;
        }
        return;
    }

    p = checkstring(cmdline, "OPEN");
    if(p == NULL) error("Invalid syntax");
    if(ExtCurrentConfig[SPI_OUT_PIN] == EXT_COM_RESERVED) error("Already open");

    { // start a new block for getargs()
    	int mode,bits=8;
    	getargs(&p, 5, ",");
        if(argc < 3) error("Incorrect argument count");
        mode=getinteger(argv[2]);
        speed = getinteger(argv[0]);
        if(argc==5)bits=getint(argv[4],4,32);
       hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
       hspi1.Init.DataSize = bits-1;
        if(speed<390625)
		   hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_256;
        else if(speed<781250)
		   hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_128;
        else if(speed<1562500)
		   hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_64;
        else if(speed<3125000)
		   hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_32;
        else if(speed<6250000)
		   hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_16;
        else if(speed<12500000)
		   hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_8;
        else if(speed<25000000)
		   hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_4;
        else
           hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_2;
           /* you can have any number of case statements */
        hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
        if(mode &2)hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
        hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
        if(mode &1)hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;

        ExtCfg(SPI_OUT_PIN, EXT_COM_RESERVED, 0);
        ExtCfg(SPI_INP_PIN, EXT_COM_RESERVED, 0);
        ExtCfg(SPI_CLK_PIN, EXT_COM_RESERVED, 0);
        HAL_SPI_DeInit(&hspi1);
        HAL_SPI_Init(&hspi1);
    }
}





// output and get a byte via SPI
void fun_spi(void) {
	union car
	{
		uint64_t aTxBuffer;
		uint8_t cTxBuffer[8];
	}mybufft;
	union car1
	{
		uint64_t aRxBuffer;
		uint8_t cRxBuffer[8];
	}mybuffr;
	mybuffr.aRxBuffer=0;
	mybufft.aTxBuffer=getinteger(ep);
    if(ExtCurrentConfig[SPI_OUT_PIN] != EXT_COM_RESERVED) error("Not open");
	HAL_SPI_TransmitReceive(&hspi1, mybufft.cTxBuffer, mybuffr.cRxBuffer, 1, 5000);
	iret=mybuffr.aRxBuffer;
    targ = T_INT;

}



void SPIClose(void){
	HAL_SPI_DeInit(&hspi1);
	if(ExtCurrentConfig[SPI_OUT_PIN] != EXT_BOOT_RESERVED) ExtCfg(SPI_OUT_PIN, EXT_NOT_CONFIG, 0);
	if(ExtCurrentConfig[SPI_INP_PIN] != EXT_BOOT_RESERVED)  ExtCfg(SPI_INP_PIN, EXT_NOT_CONFIG, 0);      // reset to not in use
	if(ExtCurrentConfig[SPI_CLK_PIN] != EXT_BOOT_RESERVED)  ExtCfg(SPI_CLK_PIN, EXT_NOT_CONFIG, 0);
}

//void cmd_spi2(char *spi) {
void cmd_spi2(void) {
	    int speed;
    char *p;
    unsigned int nbr, *d;
    long long int *dd;

    if(checkstring(cmdline, "CLOSE")) {
        SPI2Close();
        return;
    }

    if((p = checkstring(cmdline, "WRITE")) != NULL) {
    	union car
    	{
    		uint32_t aTxBuffer;
    		uint8_t cTxBuffer[4];
    	}mybuff;
       if(ExtCurrentConfig[SPI2_OUT_PIN] != EXT_COM_RESERVED) error("Not open");
        d = GetSendDataList(p, &nbr);
        while(nbr--) {
        	mybuff.aTxBuffer=*d++;
        	HAL_SPI_Transmit(&hspi2, mybuff.cTxBuffer,1, 5000);
        }
        return;
    }

    if((p = checkstring(cmdline, "READ")) != NULL) {
    	union car
    	{
    		uint32_t aRxBuffer;
    		uint8_t cRxBuffer[4];
    	}mybuff;
        if(ExtCurrentConfig[SPI2_OUT_PIN] != EXT_COM_RESERVED) error("Not open");
        dd = GetReceiveDataBuffer(p, &nbr);
        while(nbr--) {
        	mybuff.aRxBuffer=0;
        	HAL_SPI_Receive(&hspi2, mybuff.cRxBuffer, 1, 5000);
        	*dd++ = mybuff.aRxBuffer;
        }
        return;
    }

    p = checkstring(cmdline, "OPEN");
    if(p == NULL) error("Invalid syntax");
    if(ExtCurrentConfig[SPI2_CLK_PIN] == EXT_COM_RESERVED) error("Already open");
    if(ExtCurrentConfig[SPI2_OUT_PIN] == EXT_COM_RESERVED) error("In use for COM2");

    { // start a new block for getargs()
    	int mode,bits=8;
    	getargs(&p, 5, ",");
        if(argc < 3) error("Incorrect argument count");
        mode=getinteger(argv[2]);
        speed = getinteger(argv[0]);
        if(argc==5)bits=getint(argv[4],4,32);
        hspi2.Init.DataSize = bits-1;
        hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
        if(speed<390625)
		   hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_256;
        else if(speed<781250)
		   hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_128;
        else if(speed<1562500)
		   hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_64;
        else if(speed<3125000)
		   hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_32;
        else if(speed<6250000)
		   hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_16;
        else if(speed<12500000)
		   hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_8;
        else if(speed<25000000)
		   hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_4;
        else
           hspi2.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_2;
           /* you can have any number of case statements */
        hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
        if(mode &2)hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
        hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
        if(mode &1)hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;

        ExtCfg(SPI2_OUT_PIN, EXT_COM_RESERVED, 0);
        ExtCfg(SPI2_INP_PIN, EXT_COM_RESERVED, 0);
        ExtCfg(SPI2_CLK_PIN, EXT_COM_RESERVED, 0);
        HAL_SPI_DeInit(&hspi2);
        HAL_SPI_Init(&hspi2);
    }
}





// output and get a byte via SPI
void fun_spi2(void) {
	union car
	{
		uint64_t aTxBuffer;
		uint8_t cTxBuffer[8];
	}mybufft;
	union car1
	{
		uint64_t aRxBuffer;
		uint8_t cRxBuffer[8];
	}mybuffr;
	mybuffr.aRxBuffer=0;
	mybufft.aTxBuffer=getinteger(ep);
    if(ExtCurrentConfig[SPI2_OUT_PIN] != EXT_COM_RESERVED) error("Not open");
	HAL_SPI_TransmitReceive(&hspi2, mybufft.cTxBuffer, mybuffr.cRxBuffer, 1, 5000);
	iret=mybuffr.aRxBuffer;
    targ = T_INT;

}



void SPI2Close(void){
	HAL_SPI_DeInit(&hspi2);
	if(ExtCurrentConfig[SPI2_OUT_PIN] != EXT_BOOT_RESERVED) ExtCfg(SPI2_OUT_PIN, EXT_NOT_CONFIG, 0);
	if(ExtCurrentConfig[SPI2_INP_PIN] != EXT_BOOT_RESERVED)  ExtCfg(SPI2_INP_PIN, EXT_NOT_CONFIG, 0);      // reset to not in use
	if(ExtCurrentConfig[SPI2_CLK_PIN] != EXT_BOOT_RESERVED)  ExtCfg(SPI2_CLK_PIN, EXT_NOT_CONFIG, 0);

}
unsigned int *GetSendDataList(char *p, unsigned int *nbr) {
    unsigned int *buf;
    int i;
    void *ptr;

    getargs(&p, MAX_ARG_COUNT, ",");
    if(!(argc & 1)) error("Invalid syntax");
    *nbr = getint(argv[0], 0, 9999999);
    if(!*nbr) return NULL;
    buf = GetTempMemory(*nbr * sizeof(unsigned int));

    // first check if this is the situation with just two arguments where the second argument could be a string or a simple variable or an array
    // check the correct arg count AND that the second argument looks like a variable AND it is not a function
    if(argc == 3 && isnamestart(*argv[2]) && *skipvar(argv[2], false) == 0 && !(FindSubFun(argv[2], 1) >= 0 && strchr(argv[2], '(') != NULL)) {
    	ptr = findvar(argv[2], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");

        // now check if it is a non array string
		if(vartbl[VarIndex].type & T_STR) {
            if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
            if(*((char *)ptr) < *nbr) error("Insufficient data");
            ptr += sizeof(char);                                    // skip the length byte in a MMBasic string
            for (i = 0; i < *nbr; i++) {
                buf[i] = *(char *)ptr;
                ptr += sizeof(char);
            }
            return buf;
		}

        // if it is a MMFLOAT or integer do some sanity checks
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(*nbr > 1) {
            if(vartbl[VarIndex].dims[0] == 0) error("Invalid variable");
            if(*nbr > (vartbl[VarIndex].dims[0] + 1 - OptionBase)) error("Insufficient data");
        }

        // now check if it is a MMFLOAT
        if(vartbl[VarIndex].type & T_NBR) {
            for (i = 0; i < *nbr; i++) {
                buf[i] = FloatToInt32(*(MMFLOAT *)ptr);
                ptr += sizeof(MMFLOAT);
            }
            return buf;
        }

        // try for an integer
        if(vartbl[VarIndex].type & T_INT)  {
            for (i = 0; i < *nbr; i++) {
                buf[i] = *(unsigned int *)ptr;
                ptr += sizeof(long long int);
            }
            return buf;
        }

    }

    // if we got to here we must have a simple list of expressions to send (phew!)
    if(*nbr != ((argc - 1) >> 1)) error("Incorrect argument count");
    for (i = 0; i < *nbr; i++) {
        buf[i] = getinteger(argv[i + i + 2]);
    }
    return buf;
}
//void cmd_spi(void){
//	char *p;
//	if((p=checkstring(cmdline,"1")))cmd_spi1(p);
//	else if((p=checkstring(cmdline,"2")))cmd_spi2(p);
//	else error("Syntax");
//}


long long int *GetReceiveDataBuffer(char *p, unsigned int *nbr) {
    void *ptr;

    getargs(&p, 3, ",");
    if(argc != 3) error("Invalid syntax");
    *nbr = getinteger(argv[0]);
    ptr = findvar(argv[2], V_NOFIND_NULL | V_EMPTY_OK);
    if(ptr == NULL) error("Invalid variable");
	if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
        if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + *nbr) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
            error("Insufficient array size");
	}
        else error("Invalid variable");
    return ptr;
}
