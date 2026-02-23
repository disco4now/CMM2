/************************************************************************************************************************
Micromite

I2C.c

Routines to handle I2C access.

Copyright 2011 Gerard Sexton
Copyright 2011-2020 Geoff Graham
Copyright 2019-2020 Peter Mather

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

  *  Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  *  Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
extern I2C_HandleTypeDef hi2c1, hi2c2, hi2c4;

// Declare functions
void i2cEnable(char *p);
void i2cDisable(char *p);
void i2cSend(char *p);
void i2cReceive(char *p);
void i2c_disable(void);
void i2c_enable(int bps);
void i2c_masterCommand(int timer);
void i2cCheck(char *p);
//
void i2c2Enable(char *p);
void i2c2Disable(char *p);
void i2c2Send(char *p);
void i2c2Receive(char *p);
void i2c2_disable(void);
void i2c2_enable(int bps);
void i2c2_masterCommand(int timer);
void i2c2Check(char *p);
//
void i2c3Enable(char *p);
void i2c3Disable(char *p);
void i2c3Send(char *p);
void i2c3Receive(char *p);
void i2c3_disable(void);
void i2c3_enable(int bps);
void i2c3_masterCommand(int timer);
void i2c3Check(char *p);
//
static MMFLOAT *I2C_Rcvbuf_Float;										// pointer to the master receive buffer for a MMFLOAT
static long long int *I2C_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
static char *I2C_Rcvbuf_String;										// pointer to the master receive buffer for a string
static unsigned int I2C_Addr;										// I2C device address
static volatile unsigned int I2C_Sendlen;							// length of the master send buffer
static volatile unsigned int I2C_Rcvlen;							// length of the master receive buffer
static unsigned char I2C_Send_Buffer[256];                                   // I2C send buffer
unsigned int I2C1_enabled;									// I2C enable marker
unsigned int I2C_Timeout;									// master timeout value
static unsigned int I2C_Status;										// status flags
int mmI2Cvalue;
	// value of MM.I2C
	static MMFLOAT *I2C2_Rcvbuf_Float;										// pointer to the master receive buffer for a MMFLOAT
	static long long int *I2C2_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
	static char *I2C2_Rcvbuf_String;										// pointer to the master receive buffer for a string
	static unsigned int I2C2_Addr;										// I2C device address
	static volatile unsigned int I2C2_Sendlen;							// length of the master send buffer
	static volatile unsigned int I2C2_Rcvlen;							// length of the master receive buffer
	unsigned int I2C2_enabled;									// I2C enable marker
	unsigned int I2C2_Timeout;									// master timeout value
	static unsigned int I2C2_Status;										// status flags
	// value of MM.I2C
	static MMFLOAT *I2C3_Rcvbuf_Float;										// pointer to the master receive buffer for a MMFLOAT
	static long long int *I2C3_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
	static char *I2C3_Rcvbuf_String;										// pointer to the master receive buffer for a string
	static unsigned int I2C3_Addr;										// I2C device address
	static volatile unsigned int I2C3_Sendlen;							// length of the master send buffer
	static volatile unsigned int I2C3_Rcvlen;							// length of the master receive buffer
	unsigned int I2C3_enabled;									// I2C enable marker
	unsigned int I2C3_Timeout;									// master timeout value
	static unsigned int I2C3_Status;										// status flags
extern RTC_HandleTypeDef hrtc;
//void cmd_i2c1(char *i2c);
//void cmd_i2c2(char *i2c);
//void cmd_i2c3(char *i2c);
/*******************************************************************************************
							  I2C related commands in MMBasic
                              ===============================
These are the functions responsible for executing the I2C related commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

********************************************************************************************/
//void cmd_i2c(void){
//	char *p;
//	if((p=checkstring(cmdline,"1")))cmd_i2c1(p);
//	else if((p=checkstring(cmdline,"2")))cmd_i2c2(p);
//	else if((p=checkstring(cmdline,"3")))cmd_i2c3(p);
//	else error("Syntax");
//}
void cmd_i2c(void) {
    char *p;//, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2cEnable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2cDisable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
        i2cSend(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2cReceive(p);
    else if((p = checkstring(cmdline, "CHECK")) != NULL)
        i2cCheck(p);
    else
        error("Unknown command");
}
void cmd_i2c2(void) {
    char *p;//, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2c2Enable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2c2Disable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
        i2c2Send(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2c2Receive(p);
    else if((p = checkstring(cmdline, "CHECK")) != NULL)
        i2c2Check(p);
    else
        error("Unknown command");
}

void cmd_i2c3(void) {
    char *p;//, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2c3Enable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2c3Disable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
        i2c3Send(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2c3Receive(p);
    else if((p = checkstring(cmdline, "CHECK")) != NULL)
        i2c3Check(p);
    else
        error("Unknown command");
}
/*
 * *Initialize RTC and set the Time and Date
*/
void RtcGetTime(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	int localsecs;
	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	localsecs=sTime.Seconds;
	if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	milliseconds=1000-((sTime.SubSeconds*1000)/sTime.SecondFraction);
	if(sTime.SubSeconds<5){
		do {
			if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
			{
				error("RTC hardware error");
			}
			if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
			{
				error("RTC hardware error");
			}
			milliseconds=1000-((sTime.SubSeconds*1000)/sTime.SecondFraction);
		} while(localsecs==sTime.Seconds);
	}
	second = sTime.Seconds;
	minute = sTime.Minutes;
	hour = sTime.Hours;
	day = sDate.Date;
	month = sDate.Month;
	year = sDate.Year+2000;
	day_of_week=sDate.WeekDay;
}

// enable the I2C1 module - master mode
void i2cEnable(char *p) {
	int speed, timeout;
	getargs(&p, 3, ",");
	if(argc != 3) error("Invalid syntax");
	speed = getinteger(argv[0]);
	if(!(speed ==100 || speed == 400 || speed==1000)) error("Valid speeds 100, 400, 1000");
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C1_enabled || nun1 || classic1 || mouse1) error("I2C already OPEN");
	I2C_Timeout = timeout;
	i2c_enable(speed);

}
// enable the I2C2 module - master mode
void i2c2Enable(char *p) {
	int speed, timeout;
	getargs(&p, 3, ",");
	if(argc != 3) error("Invalid syntax");
	speed = getinteger(argv[0]);
	if(!(speed ==100 || speed == 400 || speed==1000)) error("Valid speeds 100, 400, 1000");
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C2_enabled || nun2 || classic2 || mouse2) error("I2C already OPEN");
	I2C2_Timeout = timeout;
	i2c2_enable(speed);

}
// enable the I2C3 module - master mode
void i2c3Enable(char *p) {
	int speed, timeout;
	getargs(&p, 3, ",");
	if(argc != 3) error("Invalid syntax");
	speed = getinteger(argv[0]);
	if(!(speed ==100 || speed == 400 || speed==1000)) error("Valid speeds 100, 400, 1000");
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C3_enabled || nun3 || classic3 || mouse3) error("I2C already OPEN");
	I2C3_Timeout = timeout;
	i2c3_enable(speed);

}


// disable the I2C1 module - master mode
void i2cDisable(char *p) {
	if(!I2C1_enabled)error("Not open");
	if(nun1 || classic1 || mouse1)error("In use for Controller");
	i2c_disable();
}

// disable the I2C2 module - master mode
void i2c2Disable(char *p) {
	if(!I2C2_enabled)error("Not open");
	if(nun2 || classic2 || mouse2)error("In use for Controller");
	i2c2_disable();
}

// disable the I2C2 module - master mode
void i2c3Disable(char *p) {
	if(!I2C3_enabled)error("Not open");
	if(nun3 || classic3 || mouse3)error("In use for Controller");
	i2c3_disable();
}
// send data to an I2C slave - master mode
void i2cSend(char *p) {
	int addr, i2c_options, sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;

	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Invalid syntax");
	if(!(I2C1_enabled || nun1 || classic1 || mouse1))error("I2C not open");
	addr = getinteger(argv[0]);
	i2c_options = getint(argv[2],0,1);
	I2C_Status = 0;
	if(i2c_options & 0x01) I2C_Status = I2C_Status_BusHold;
	I2C_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
		if(sendlen != ((argc - 5) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C_Sendlen = sendlen;
	I2C_Rcvlen = 0;

	i2c_masterCommand(1);
}
// send data to an I2C slave - master mode
void i2c2Send(char *p) {
	int addr, i2c2_options, sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;

	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Invalid syntax");
	if(!(I2C2_enabled || nun2 || classic2 || mouse2))error("I2C not open");
	addr = getinteger(argv[0]);
	i2c2_options = getint(argv[2],0,1);
	I2C2_Status = 0;
	if(i2c2_options & 0x01) I2C2_Status = I2C_Status_BusHold;
	I2C2_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
		if(sendlen != ((argc - 5) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C2_Sendlen = sendlen;
	I2C2_Rcvlen = 0;

	i2c2_masterCommand(1);
}
// send data to an I2C slave - master mode
void i2c3Send(char *p) {
	int addr, i2c3_options, sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;

	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Invalid syntax");
	if(!(I2C3_enabled || nun3 || classic3 || mouse3))error("I2C not open");
	addr = getinteger(argv[0]);
	i2c3_options = getint(argv[2],0,1);
	I2C3_Status = 0;
	if(i2c3_options & 0x01) I2C3_Status = I2C_Status_BusHold;
	I2C3_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
		if(sendlen != ((argc - 5) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C3_Sendlen = sendlen;
	I2C3_Rcvlen = 0;

	i2c3_masterCommand(1);
}

void i2cCheck(char *p) {
	int addr;
	getargs(&p, 1, ",");
	if(!(I2C1_enabled || nun1 || classic1 || mouse1))error("I2C not open");
	addr = getinteger(argv[0]);
    if(addr<1 || addr>0x7F)error("Invalid I2C address");
    addr<<=1;
	mmI2Cvalue = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)addr, 2, 10);
}
void i2c2Check(char *p) {
	int addr;
	getargs(&p, 1, ",");
	if(!(I2C2_enabled || nun2 || classic2 || mouse2))error("I2C not open");
	addr = getinteger(argv[0]);
    if(addr<1 || addr>0x7F)error("Invalid I2C address");
    addr<<=1;
	mmI2Cvalue = HAL_I2C_IsDeviceReady(&hi2c4, (uint16_t)addr, 2, 10);
}
void i2c3Check(char *p) {
	int addr;
	getargs(&p, 1, ",");
	if(!(I2C3_enabled || nun3 || classic3 || mouse3))error("I2C not open");
	addr = getinteger(argv[0]);
    if(addr<1 || addr>0x7F)error("Invalid I2C address");
    addr<<=1;
	mmI2Cvalue = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)addr, 2, 10);
}
// receive data from an I2C slave - master mode
void i2cReceive(char *p) {
	int addr, i2c_options, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
	if(argc != 7) error("Invalid syntax");
	if(!(I2C1_enabled || nun1 || classic1 || mouse1))error("I2C not open");
	addr = getinteger(argv[0]);
	i2c_options = getint(argv[2],0,1);
	I2C_Status = 0;
	if(i2c_options & 0x01) I2C_Status = I2C_Status_BusHold;
	I2C_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Int = (long long int *)ptr;
        I2C_Rcvbuf_Float = NULL;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C_Rcvbuf_String = (char *)ptr + 1;
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
    } else error("Invalid variable");
	I2C_Rcvlen = rcvlen;

	I2C_Sendlen = 0;

	i2c_masterCommand(1);
}
// receive data from an I2C slave - master mode
void i2c2Receive(char *p) {
	int addr, i2c2_options, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
	if(argc != 7) error("Invalid syntax");
	if(!(I2C2_enabled || nun2 || classic2 || mouse2))error("I2C not open");
	addr = getinteger(argv[0]);
	i2c2_options = getint(argv[2],0,1);
	I2C2_Status = 0;
	if(i2c2_options & 0x01) I2C2_Status = I2C_Status_BusHold;
	I2C2_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Int = (long long int *)ptr;
        I2C2_Rcvbuf_Float = NULL;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C2_Rcvbuf_String = (char *)ptr + 1;
        I2C2_Rcvbuf_Float = NULL;
        I2C2_Rcvbuf_Int = NULL;
    } else error("Invalid variable");
	I2C2_Rcvlen = rcvlen;

	I2C2_Sendlen = 0;

	i2c2_masterCommand(1);
}
// receive data from an I2C slave - master mode
void i2c3Receive(char *p) {
	int addr, i2c3_options, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
	if(argc != 7) error("Invalid syntax");
	if(!(I2C3_enabled || nun3 || classic3 || mouse3))error("I2C not open");
	addr = getinteger(argv[0]);
	i2c3_options = getint(argv[2],0,1);
	I2C3_Status = 0;
	if(i2c3_options & 0x01) I2C3_Status = I2C_Status_BusHold;
	I2C3_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C3_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C3_Rcvbuf_Int = (long long int *)ptr;
        I2C3_Rcvbuf_Float = NULL;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C3_Rcvbuf_String = (char *)ptr + 1;
        I2C3_Rcvbuf_Float = NULL;
        I2C3_Rcvbuf_Int = NULL;
    } else error("Invalid variable");
	I2C3_Rcvlen = rcvlen;

	I2C3_Sendlen = 0;

	i2c3_masterCommand(1);
}

/**************************************************************************************************
Enable the I2C1 module - master mode
***************************************************************************************************/
void i2c_enable(int bps) {
    HAL_I2C_DeInit(&hi2c1);
    ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
    PinSetBit(P_I2C_SDA, TRISSET);
    if(PinRead(P_I2C_SDA) == 0) {
    	int i;
        // it appears as if something is holding SLA low
        // try pulsing the clock to get rid of it
        PinSetBit(P_I2C_SCL, TRISCLR);
        PinSetBit(P_I2C_SCL, LATCLR);
        for(i = 0; i < 20; i++) {
            uSec(25);
            PinSetBit(P_I2C_SCL, LATINV);
            if(PinRead(P_I2C_SDA) == 0) break;
        }
        PinSetBit(P_I2C_SCL, TRISSET);
    }
    ExtCfg(P_I2C_SDA, EXT_COM_RESERVED, 0);
    ExtCfg(P_I2C_SCL, EXT_COM_RESERVED, 0);
    if(bps==100) hi2c1.Init.Timing = 0x10707DBC;
    if(bps==400) hi2c1.Init.Timing = 0x00602173;
    if(bps==1000)hi2c1.Init.Timing = 0x00300B29;
    HAL_I2C_Init(&hi2c1);
	I2C1_enabled=1;
}
void i2c2_enable(int bps) {
    HAL_I2C_DeInit(&hi2c4);
    ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
    PinSetBit(P_I2C2_SDA, TRISSET);
    if(PinRead(P_I2C2_SDA) == 0) {
        int i;
        // it appears as if something is holding SLA low
        // try pulsing the clock to get rid of it
        PinSetBit(P_I2C2_SCL, TRISCLR);
        PinSetBit(P_I2C2_SCL, LATCLR);
        for(i = 0; i < 20; i++) {
           uSec(25);
           PinSetBit(P_I2C2_SCL, LATINV);
           if(PinRead(P_I2C2_SDA) == 0) break;
        }
        PinSetBit(P_I2C2_SCL, TRISSET);
    }
    ExtCfg(P_I2C2_SDA, EXT_COM_RESERVED, 0);
    ExtCfg(P_I2C2_SDA, EXT_COM_RESERVED, 0);
    if(bps==100) hi2c4.Init.Timing = 0x10707DBC;
    if(bps==400) hi2c4.Init.Timing = 0x00602173;
    if(bps==1000)hi2c4.Init.Timing = 0x00300B29;
    HAL_I2C_Init(&hi2c4);
	I2C2_enabled=1;
}
void i2c3_enable(int bps) {
    HAL_I2C_DeInit(&hi2c2);
    ExtCfg(P_I2C3_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C3_SCL, EXT_NOT_CONFIG, 0);
    PinSetBit(P_I2C3_SDA, TRISSET);
    if(PinRead(P_I2C3_SDA) == 0) {
        int i;
        // it appears as if something is holding SLA low
        // try pulsing the clock to get rid of it
        PinSetBit(P_I2C3_SCL, TRISCLR);
        PinSetBit(P_I2C3_SCL, LATCLR);
        for(i = 0; i < 20; i++) {
           uSec(25);
           PinSetBit(P_I2C3_SCL, LATINV);
           if(PinRead(P_I2C3_SDA) == 0) break;
        }
        PinSetBit(P_I2C3_SCL, TRISSET);
    }
    ExtCfg(P_I2C3_SDA, EXT_COM_RESERVED, 0);
    ExtCfg(P_I2C3_SDA, EXT_COM_RESERVED, 0);
    if(bps==100) hi2c2.Init.Timing = 0x10707DBC;
    if(bps==400) hi2c2.Init.Timing = 0x00602173;
    if(bps==1000)hi2c2.Init.Timing = 0x00300B29;
    HAL_I2C_Init(&hi2c2);
	I2C3_enabled=1;
}

/**************************************************************************************************
Disable the I2C1 module - master mode
***************************************************************************************************/
void i2c_disable(void) {
    I2C_Status = I2C_Status_Disable;
	I2C_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
    I2C_Rcvbuf_Float = NULL;
    I2C_Rcvbuf_Int = NULL;
	I2C_Sendlen = 0;												// length of the master send buffer
	I2C_Rcvlen = 0;													// length of the master receive buffer
	I2C_Addr = 0;													// I2C device address
	I2C_Timeout = 0;												// master timeout value
	HAL_I2C_DeInit(&hi2c1);
	I2C1_enabled = 0;
    ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
}
void i2c2_disable(void) {
    I2C2_Status = I2C_Status_Disable;
	I2C2_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
    I2C2_Rcvbuf_Float = NULL;
    I2C2_Rcvbuf_Int = NULL;
	I2C2_Sendlen = 0;												// length of the master send buffer
	I2C2_Rcvlen = 0;													// length of the master receive buffer
	I2C2_Addr = 0;													// I2C device address
	I2C2_Timeout = 0;												// master timeout value
	HAL_I2C_DeInit(&hi2c4);
	I2C2_enabled = 0;
    ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
}
void i2c3_disable(void) {
    I2C3_Status = I2C_Status_Disable;
	I2C3_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
    I2C3_Rcvbuf_Float = NULL;
    I2C3_Rcvbuf_Int = NULL;
	I2C3_Sendlen = 0;												// length of the master send buffer
	I2C3_Rcvlen = 0;													// length of the master receive buffer
	I2C3_Addr = 0;													// I2C device address
	I2C3_Timeout = 0;												// master timeout value
	HAL_I2C_DeInit(&hi2c2);
	I2C3_enabled = 0;
    ExtCfg(P_I2C3_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C3_SCL, EXT_NOT_CONFIG, 0);
}
/**************************************************************************************************
Send and/or Receive data - master mode
***************************************************************************************************/
void i2c_masterCommand(int timer) {
//	unsigned char start_type,
	unsigned char i,i2caddr=I2C_Addr<<1,I2C_Rcv_Buffer[I2C_Rcvlen+2];
	mT4IntEnable(0);
	if(I2C_Sendlen){
		mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)i2caddr, I2C_Send_Buffer, I2C_Sendlen, I2C_Timeout);
	}
	if(I2C_Rcvlen){
		mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, (uint16_t)i2caddr, (uint8_t *)I2C_Rcv_Buffer, I2C_Rcvlen, I2C_Timeout);
		for(i=0;i<I2C_Rcvlen;i++){
			if(I2C_Rcvbuf_String!=NULL){
				*I2C_Rcvbuf_String=I2C_Rcv_Buffer[i];
				I2C_Rcvbuf_String++;
			}
			if(I2C_Rcvbuf_Float!=NULL){
				*I2C_Rcvbuf_Float=I2C_Rcv_Buffer[i];
				I2C_Rcvbuf_Float++;
			}
			if(I2C_Rcvbuf_Int!=NULL){
				*I2C_Rcvbuf_Int=I2C_Rcv_Buffer[i];
				I2C_Rcvbuf_Int++;
			}
		}
	}
	mT4IntEnable(1);
}

void i2c2_masterCommand(int timer) {
//	unsigned char start_type,
	unsigned char i,i2c2addr=I2C2_Addr<<1,I2C2_Rcv_Buffer[I2C2_Rcvlen+2];
	mT4IntEnable(0);
	if(I2C2_Sendlen){
		mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)i2c2addr, I2C_Send_Buffer, I2C2_Sendlen, I2C2_Timeout);
	}
	if(I2C2_Rcvlen){
		mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c4, (uint16_t)i2c2addr, (uint8_t *)I2C2_Rcv_Buffer, I2C2_Rcvlen, I2C2_Timeout);
		for(i=0;i<I2C2_Rcvlen;i++){
			if(I2C2_Rcvbuf_String!=NULL){
				*I2C2_Rcvbuf_String=I2C2_Rcv_Buffer[i];
				I2C2_Rcvbuf_String++;
			}
			if(I2C2_Rcvbuf_Float!=NULL){
				*I2C2_Rcvbuf_Float=I2C2_Rcv_Buffer[i];
				I2C2_Rcvbuf_Float++;
			}
			if(I2C2_Rcvbuf_Int!=NULL){
				*I2C2_Rcvbuf_Int=I2C2_Rcv_Buffer[i];
				I2C2_Rcvbuf_Int++;
			}
		}
	}
	mT4IntEnable(1);
}
void i2c3_masterCommand(int timer) {
//	unsigned char start_type,
	unsigned char i,i2c3addr=I2C3_Addr<<1,I2C3_Rcv_Buffer[I2C3_Rcvlen+2];
	mT4IntEnable(0);
	if(I2C3_Sendlen){
		mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)i2c3addr, I2C_Send_Buffer, I2C3_Sendlen, I2C3_Timeout);
	}
	if(I2C3_Rcvlen){
		mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, (uint16_t)i2c3addr, (uint8_t *)I2C3_Rcv_Buffer, I2C3_Rcvlen, I2C3_Timeout);
		for(i=0;i<I2C3_Rcvlen;i++){
			if(I2C3_Rcvbuf_String!=NULL){
				*I2C3_Rcvbuf_String=I2C3_Rcv_Buffer[i];
				I2C3_Rcvbuf_String++;
			}
			if(I2C3_Rcvbuf_Float!=NULL){
				*I2C3_Rcvbuf_Float=I2C3_Rcv_Buffer[i];
				I2C3_Rcvbuf_Float++;
			}
			if(I2C3_Rcvbuf_Int!=NULL){
				*I2C3_Rcvbuf_Int=I2C3_Rcv_Buffer[i];
				I2C3_Rcvbuf_Int++;
			}
			}
	}
	mT4IntEnable(1);
}

//void fun_mmi2c(void) {
//	iret = mmI2Cvalue;
//    targ = T_INT;
//}

