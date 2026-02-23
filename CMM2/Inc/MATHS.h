/***********************************************************************************************************************
MMBasic

Editor.h

Include file that contains the globals and defines for the full screen editor in MMBasic.
  
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

#include <stdbool.h>


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397 /* changes to STATE_VECTOR_LENGTH also require changes to this */
#define MAXPID 16
typedef struct tagMTRand {
  unsigned long mt[STATE_VECTOR_LENGTH];
  int index;
} MTRand;

void seedRand(unsigned long seed);
unsigned long genRandLong(MTRand* rand);
double genRand(MTRand* rand);
extern struct tagMTRand *g_myrand;


// General definitions used by other modules
extern void Q_Mult(MMFLOAT *q1, MMFLOAT *q2, MMFLOAT *n);
extern void Q_Invert(MMFLOAT *q, MMFLOAT *n);
extern void cmd_SensorFusion(char *passcmdline);
void MahonyQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT Ki, MMFLOAT Kp, MMFLOAT deltat, MMFLOAT *yaw, MMFLOAT *pitch, MMFLOAT *roll);
void MadgwickQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT beta, MMFLOAT deltat, MMFLOAT *pitch, MMFLOAT *yaw, MMFLOAT *roll);

//int parsenumberarray(char *tp, MMFLOAT **a1float, int64_t **a1int, int argno, int dimensions, int *dims, bool ConstantNotAllowed);
//int parsefloatarray(char *tp, MMFLOAT **a1float, int argno, int dimensions, int *dims, bool ConstantNotAllowed);
//int parseintegerarray(char *tp, int64_t **a1int, int argno, int dimensions, int *dims, bool ConstantNotAllowed);
// STRUCTURES
int parsenumberarray(char *tp, MMFLOAT **a1float, int64_t **a1int, int argno, int dimensions, int *dims, bool ConstantNotAllowed,int *stride);
int parsefloatarray(char *tp, MMFLOAT **a1float, int argno, int dimensions, int *dims, bool ConstantNotAllowed,int *stride);
int parseintegerarray(char *tp, int64_t **a1int, int argno, int dimensions, int *dims, bool ConstantNotAllowed,int *stride);

int parsestringarray(char *tp,  char **a1str, int argno, int dimensions, int *dims, bool ConstantNotAllowed, unsigned char *length);
int parseany( char *tp, MMFLOAT **a1float, int64_t **a1int, unsigned char ** a1str, int *length, bool stringarray);

extern volatile unsigned int AHRSTimer;
#define CRC4_DEFAULT_POLYNOME       0x03
#define CRC4_ITU                    0x03


// CRC 8
#define CRC8_DEFAULT_POLYNOME       0x07
#define CRC8_DVB_S2                 0xD5
#define CRC8_AUTOSAR                0x2F
#define CRC8_BLUETOOTH              0xA7
#define CRC8_CCITT                  0x07
#define CRC8_DALLAS_MAXIM           0x31                // oneWire
#define CRC8_DARC                   0x39
#define CRC8_GSM_B                  0x49
#define CRC8_SAEJ1850               0x1D
#define CRC8_WCDMA                  0x9B


// CRC 12
#define CRC12_DEFAULT_POLYNOME      0x080D
#define CRC12_CCITT                 0x080F
#define CRC12_CDMA2000              0x0F13
#define CRC12_GSM                   0x0D31


// CRC 16
#define CRC16_DEFAULT_POLYNOME      0x1021
#define CRC16_CHAKRAVARTY           0x2F15
#define CRC16_ARINC                 0xA02B
#define CRC16_CCITT                 0x1021
#define CRC16_CDMA2000              0xC867
#define CRC16_DECT                  0x0589
#define CRC16_T10_DIF               0x8BB7
#define CRC16_DNP                   0x3D65
#define CRC16_IBM                   0x8005
#define CRC16_OPENSAFETY_A          0x5935
#define CRC16_OPENSAFETY_B          0x755B
#define CRC16_PROFIBUS              0x1DCF


// CRC 32
#define CRC32_DEFAULT_POLYNOME      0x04C11DB7
#define CRC32_ISO3309               0x04C11DB7
#define CRC32_CASTAGNOLI            0x1EDC6F41
#define CRC32_KOOPMAN               0x741B8CD7
#define CRC32_KOOPMAN_2             0x32583499
#define CRC32_Q                     0x814141AB


// CRC 64
#define CRC64_DEFAULT_POLYNOME      0x42F0E1EBA9EA3693
#define CRC64_ECMA64                0x42F0E1EBA9EA3693
#define CRC64_ISO64                 0x000000000000001B
typedef struct {

	/* Controller gains */
	MMFLOAT Kp;
	MMFLOAT Ki;
	MMFLOAT Kd;

	/* Derivative low-pass filter time constant */
	MMFLOAT tau;

	/* Output limits */
	MMFLOAT limMin;
	MMFLOAT limMax;
	
	/* Integrator limits */
	MMFLOAT limMinInt;
	MMFLOAT limMaxInt;

	/* Sample time (in seconds) */
	MMFLOAT T;

	/* Controller "memory" */
	MMFLOAT integrator;
	MMFLOAT prevError;			/* Required for integrator */
	MMFLOAT differentiator;
	MMFLOAT prevMeasurement;		/* Required for differentiator */

	/* Controller output */
	MMFLOAT out;

} PIDController;

typedef struct PIDchan {
	char *interrupt;
    int process;
    PIDController *PIDparams;
    uint64_t timenext;
    bool active;
}s_PIDchan;
extern s_PIDchan PIDchannels[MAXPID+1];

MMFLOAT PIDController_Update(PIDController *pid, MMFLOAT setpoint, MMFLOAT measurement);



#endif
