/***********************************************************************************************************************
MMBasic

Misc.c

Handles all the miscelaneous commands and functions in MMBasic.  These are commands and functions that do not
comfortably fit anywhere else.

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
#include <time.h>
#include "upng.h"
#include "CMMFontZero.h"
#include "CMMFontOne.h"
#include "CMMFontTwo.h"
#include <stdio.h>
#include "ffconf.h"
#include "hxcmod.h"
#include "cJSON.h"
//#include "xregex.h"
#include "re.h"
#include "aes.h"

extern const unsigned char  Misc_12x20_LE[2854];
extern const unsigned char  Hom_16x24_LE[4564];
extern const unsigned char font1[];
extern int colourmap[8];
extern modcontext mcontext;
extern volatile uint64_t FastTimer;
extern int resolve_path(char *path,char *result,char *pos);
struct s_inttbl inttbl[NBRINTERRUPTS];
extern char *InterruptReturn;
extern void cmd_fasttick(char *p);
int TickPeriod[NBRSETTICKS];
volatile int TickTimer[NBRSETTICKS+1];
volatile unsigned char TickActive[NBRSETTICKS];
char *TickInt[NBRSETTICKS+1];
char *OnKeyGOSUB = NULL;
const char *daystrings[] = {"dummy","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
int CMM1=false;
char EchoOption = true;
unsigned long long int __attribute__((section(".my_section"))) saved_variable;  //  __attribute__ ((persistent));  // and this is the address
int sendCRLF=3;
unsigned int CurrentCpuSpeed;
MMFLOAT optionangle=1.0;
int optiony=0;
unsigned int PeripheralBusSpeed;
int SaveOptionErrorSkip=0;
int SaveMMerrno;           // save the error number
char SaveMMErrMsg[MAXERRMSG];  // save the error message

extern RTC_HandleTypeDef hrtc;
extern volatile int ADCcomplete;
extern volatile int Keycomplete;
extern volatile int DACcomplete;
extern volatile uint64_t * volatile a1point, * volatile a2point, * volatile a3point;
extern volatile MMFLOAT  * volatile a1float, * volatile a2float, * volatile a3float;
extern int ADCmax;
extern volatile int ADCchannelA;
extern volatile int ADCchannelB;
extern volatile int ADCchannelC;
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
extern char *LCDList[];
extern volatile BYTE SDCardStat;
extern volatile int keyboardseen;
extern BYTE MDD_SDSPI_CardDetectState(void);
extern volatile uint64_t uSecTimer;
extern TIM_HandleTypeDef htim16;
extern a_flist *alist;
extern int StartEditLine,StartEditCharacter;
extern const char *FErrorMsg[];
extern char FunKey[NBRPROGKEYS][MAXKEYLEN + 1];
extern int overrun; // value of MM.OW
extern char bootcause[12];
extern int mmOWvalue;
extern volatile int TouchState;
extern volatile struct s_nunstruct nunstruct[4];
extern volatile struct s_nunstruct mousestruct[4];
extern const char *KBrdList[];
extern void MX470Cursor(int x, int y);
extern void MX470Display(int fn);
extern void setterminal(void);
extern char *firststmt;
extern char *nextstmt;
extern volatile uint64_t Count5High;
extern uint64_t lastCount5High;

extern uint8_t getrnd(void);
//extern uint32_t restart_reason;
extern unsigned int b64d_size(unsigned int in_size);
extern unsigned int b64e_size(unsigned int in_size);
extern unsigned int b64_encode(const unsigned char* in, unsigned int in_len, unsigned char* out);
extern unsigned int b64_decode(const unsigned char* in, unsigned int in_len, unsigned char* out);
void parselongAES(char *p, int ivadd, uint8_t *keyx, uint8_t *ivx, int64_t **inint, int64_t **outint);

char *CSubInterrupt;
volatile int CSubComplete=0;

#define getcsargs(x, y)                                \
    char argbuf[STRINGSIZE + STRINGSIZE / 2]; \
    char *argv[y];                            \
    int argc;                                          \
    makeargs(x, y, argbuf, argv, &argc, ( char *)",")
static inline CommandToken commandtbl_decode(const char *p)
{
#ifdef CMD16BIT	
	return ((CommandToken)(p[0] & 0x7f)) | ((CommandToken)(p[1] & 0x7f) << 7);
#else
    return ((CommandToken)(p[0])) ;
#endif
}

void integersort(int64_t *iarray, int n, long long *index, int flags, int startpoint){
    int i, j = n, s = 1;
    int64_t t;
    if((flags & 1) == 0){
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (iarray[i] < iarray[i - 1]) {
					t = iarray[i];
					iarray[i] = iarray[i - 1];
					iarray[i - 1] = t;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    } else {
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (iarray[i] > iarray[i - 1]) {
					t = iarray[i];
					iarray[i] = iarray[i - 1];
					iarray[i - 1] = t;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    }
}
void floatsort(MMFLOAT *farray, int n, long long *index, int flags, int startpoint){
    int i, j = n, s = 1;
    int64_t t;
    MMFLOAT f;
    if((flags & 1) == 0){
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (farray[i] < farray[i - 1]) {
					f = farray[i];
					farray[i] = farray[i - 1];
					farray[i - 1] = f;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    } else {
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (farray[i] > farray[i - 1]) {
					f = farray[i];
					farray[i] = farray[i - 1];
					farray[i - 1] = f;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    }
}
/*
void stringsort(unsigned char *sarray, int n, int offset, long long *index, int flags, int startpoint){
	int ii,i, s = 1,isave;
	int k;
	unsigned char *s1,*s2,*p1,*p2;
	unsigned char temp;
	int reverse= 1-((flags & 1)<<1);
    while (s){
      s=0;
      for(i=1;i<n;i++){
        s2=i*offset+sarray;
        s1=(i-1)*offset+sarray;
        ii = *s1 < *s2 ? *s1 : *s2; //get the smaller  length
        p1 = s1 + 1; p2 = s2 + 1;
        k=0; //assume the strings match
        while((ii--) && (k==0)) {
          if(flags & 2){
			  if(toupper(*p1) > toupper(*p2)){
				k=reverse; //earlier in the array is bigger
			  }
			  if(toupper(*p1) < toupper(*p2)){
				 k=-reverse; //later in the array is bigger
			  }
          } else {
			  if(*p1 > *p2){
				k=reverse; //earlier in the array is bigger
			  }
			  if(*p1 < *p2){
				 k=-reverse; //later in the array is bigger
			  }
          }
          p1++; p2++;
        }
      // if up to this point the strings match
      // make the decision based on which one is shorter
      if(k==0){
        if(*s1 > *s2) k=reverse;
        if(*s1 < *s2) k=-reverse;
      }
      if (k==1){ // if earlier is bigger swap them round
        ii = *s1 > *s2 ? *s1 : *s2; //get the bigger length
        ii++;
        p1=s1;p2=s2;
        while(ii--){
          temp=*p1;
          *p1=*p2;
          *p2=temp;
          p1++; p2++;
        }
        s=1;
        if(index!=NULL){
        	isave=index[i-1+startpoint];
        	index[i-1+startpoint]=index[i+startpoint];
        	index[i+startpoint]=isave;
        }
      }
      routinechecks(1);
    }
  }
}
*/
/* enhance string sort from pico */
void stringsort(unsigned char *sarray, int n, int offset, long long *index, int flags, int startpoint){
	int ii,i, s = 1,isave;
	int k;
	unsigned char *s1,*s2,*p1,*p2;
	unsigned char temp;
	int reverse= 1-((flags & 1)<<1);
    while (s){
        s=0;
        for(i=1;i<n;i++){
            s2=i*offset+sarray;
            s1=(i-1)*offset+sarray;
            ii = *s1 < *s2 ? *s1 : *s2; //get the smaller  length
            p1 = s1 + 1; p2 = s2 + 1;
            k=0; //assume the strings match
            while((ii--) && (k==0)) {
            if(flags & 2){
                if(toupper(*p1) > toupper(*p2)){
                    k=reverse; //earlier in the array is bigger
                }
                if(toupper(*p1) < toupper(*p2)){
                    k=-reverse; //later in the array is bigger
                }
            } else {
                if(*p1 > *p2){
                    k=reverse; //earlier in the array is bigger
                }
                if(*p1 < *p2){
                    k=-reverse; //later in the array is bigger
                }
            }
            p1++; p2++;
            }
        // if up to this point the strings match
        // make the decision based on which one is shorter
            if(k==0){
                if(*s1 > *s2) k=reverse;
                if(*s1 < *s2) k=-reverse;
            }
            if (k==1){ // if earlier is bigger swap them round
                ii = *s1 > *s2 ? *s1 : *s2; //get the bigger length
                ii++;
                p1=s1;p2=s2;
                while(ii--){
                temp=*p1;
                *p1=*p2;
                *p2=temp;
                p1++; p2++;
                }
                s=1;
                if(index!=NULL){
                    isave=index[i-1+startpoint];
                    index[i-1+startpoint]=index[i+startpoint];
                    index[i+startpoint]=isave;
                }
            }
        }
    }
    if((flags & 5) == 5){
        for(i=n-1;i>=0;i--){
            s2=i*offset+sarray;
            if(*s2 !=0)break;
        }
        i++;
        if(i){
            s2=(n-i)*offset+sarray;
            memmove(s2,sarray,offset*i);
            memset(sarray,0,offset*(n-i));
            if(index!=NULL){
                long long int *newindex=(long long int *)GetTempMemory(n* sizeof(long long int));
                memmove(&newindex[n-i],&index[startpoint],i*sizeof(long long int));
                memmove(newindex,&index[startpoint+i],(n-i)*sizeof(long long int));
                memmove(&index[startpoint],newindex,n*sizeof(long long int));
            }
        }
    } else if(flags & 4){
        for(i=0;i<n;i++){
            s2=i*offset+sarray;
            if(*s2 !=0)break;
        }
        if(i){
            s2=i*offset+sarray;
            memmove(sarray,s2,offset*(n-i));
            s2=(n-i)*offset+sarray;
            memset(s2,0,offset*i);
            if(index!=NULL){
                long long int *newindex=(long long int *)GetTempMemory(n* sizeof(long long int));
                memmove(newindex,&index[startpoint+i],(n-i)*sizeof(long long int));
                memmove(&newindex[n-i],&index[startpoint],i*sizeof(long long int));
                memmove(&index[startpoint],newindex,n*sizeof(long long int));
            }
        }
    }
}

/*enhanced sort from pico */
void cmd_sort(void){
    MMFLOAT *a3float=NULL;
    int64_t *a3int=NULL,*a4int=NULL;
    unsigned char *a3str=NULL;
    int i, size=0, truesize,flags=0, maxsize=0, startpoint=0;
	getargs(&cmdline,9,",");
    size=parseany(argv[0],&a3float,&a3int,&a3str,&maxsize,true)-1;
    truesize=size;
    if(argc>=3 && *argv[2]){
        int card=parseintegerarray(argv[2],&a4int,2,1,NULL,true,NULL)-1;
    	if(card !=size)error("Array size mismatch");
    }
    if(argc>=5 && *argv[4])flags=getint(argv[4],0,7);
    if(argc>=7 && *argv[6])startpoint=getint(argv[6],OptionBase,size+OptionBase);
    size-=startpoint;
    if(argc==9)size=getint(argv[8],1,size+1+OptionBase)-1;
    if(startpoint)startpoint-=OptionBase;
    if(a3float!=NULL){
    	a3float+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	floatsort(a3float, size+1, a4int, flags, startpoint);
    } else if(a3int!=NULL){
    	a3int+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	integersort(a3int,  size+1, a4int, flags, startpoint);
    } else if(a3str!=NULL){
    	a3str+=((startpoint)*(maxsize+1));
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	stringsort(a3str, size+1,maxsize+1, a4int, flags, startpoint);
    }
}
/*
void cmd_sort(void){
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    MMFLOAT *a3float=NULL;
    int64_t *a3int=NULL,*a4int=NULL;
    unsigned char *a3str=NULL;
    int i, size, truesize,flags=0, maxsize=0, startpoint=0;
	getargs(&cmdline,9,",");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be array");
        }
        a3float = (MMFLOAT *)ptr1;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be array");
        }
        a3int = (int64_t *)ptr1;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be array");
        }
        a3str = (unsigned char *)ptr1;
        maxsize=vartbl[VarIndex].size;
    } else error("Argument 1 must be array");
	if((uint32_t)ptr1!=(uint32_t)vartbl[VarIndex].val.s)error("Argument 1 must be array");
    truesize=size=(vartbl[VarIndex].dims[0] - OptionBase);
    if(argc>=3 && *argv[2]){
    	ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    	if(vartbl[VarIndex].type & T_INT) {
    		if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    		if(vartbl[VarIndex].dims[0] <= 0 ) {		// Not an array
    			error("Argument 2 must be integer array");
    		}
    		a4int = (int64_t *)ptr2;
    	} else error("Argument 2 must be integer array");
    	if((vartbl[VarIndex].dims[0] - OptionBase) !=size)error("Arrays should be the same size");
		if((uint32_t)ptr2!=(uint32_t)vartbl[VarIndex].val.s)error("Argument 2 must be array");
    }
    if(argc>=5 && *argv[4])flags=getint(argv[4],0,3);
    if(argc>=7 && *argv[6])startpoint=getint(argv[6],OptionBase,size+OptionBase);
    size-=startpoint;
    if(argc==9)size=getint(argv[8],1,size+1+OptionBase)-1;
    if(startpoint)startpoint-=OptionBase;
    if(a3float!=NULL){
    	a3float+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	floatsort(a3float, size+1, a4int, flags, startpoint);
    } else if(a3int!=NULL){
    	a3int+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	integersort(a3int,  size+1, a4int, flags, startpoint);
    } else if(a3str!=NULL){
    	a3str+=((startpoint)*(maxsize+1));
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	stringsort(a3str,  size+1,maxsize+1, a4int, flags, startpoint);
    }
}
*/
void fun_datetime(void){
    sret = GetTempStrMemory();                                    // this will last for the life of the command
	if(checkstring(ep, "NOW")){
	    RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
	    IntToStrPad(sret, day, '0', 2, 10);
	    sret[2] = '-'; IntToStrPad(sret + 3, month, '0', 2, 10);
	    sret[5] = '-'; IntToStr(sret + 6, year, 10);
	    sret[10] = ' ';
	    IntToStrPad(sret+11, hour, '0', 2, 10);
	    sret[13] = ':'; IntToStrPad(sret + 14, minute, '0', 2, 10);
	    sret[16] = ':'; IntToStrPad(sret + 17, second, '0', 2, 10);
	} else {
		struct tm  *tm;
		struct tm tma;
		tm=&tma;
	    time_t timestamp = getint(ep, 0x80000000, 0x7FFFFFFF); /* See README.md if your system lacks timegm(). */
	    tm=gmtime(&timestamp);
	    IntToStrPad(sret, tm->tm_mday, '0', 2, 10);
	    sret[2] = '-'; IntToStrPad(sret + 3, tm->tm_mon+1, '0', 2, 10);
	    sret[5] = '-'; IntToStr(sret + 6, tm->tm_year+1900, 10);
	    sret[10] = ' ';
	    IntToStrPad(sret+11, tm->tm_hour, '0', 2, 10);
	    sret[13] = ':'; IntToStrPad(sret + 14, tm->tm_min, '0', 2, 10);
	    sret[16] = ':'; IntToStrPad(sret + 17, tm->tm_sec, '0', 2, 10);
	}
    CtoM(sret);
    targ = T_STR;
}
void fun_keydown(void) {
	int i,n=getint(ep,0,8);
	iret=0;
	while(MMInkey() != -1); // clear anything in the input buffer
	if(n==8){
		iret=(caps_lock ? 1: 0) |
				(num_lock ? 2: 0) |
				(scroll_lock ? 4: 0);
	} else if(n){
		iret = KeyDown[n-1];											        // this is the character
	} else {
		for(i=0;i<6;i++){
			if(KeyDown[i])iret++;
		}
	}
	targ=T_INT;
}
void fun_mouse(void){
	iret=-1;
	int chan=2;
	char *p;
	getargs(&ep,3,",");
	if(argc==3)chan=getint(argv[2],0,3);
	if(!((chan==3 && mouse3) || (chan==2 && mouse2) || (chan==1 && mouse1) || (chan==0 && mouse0) ))error("Not open");
	p=argv[0];
	if(toupper(*p)=='X')iret=mousestruct[chan].ax;
	else if(toupper(*p)=='Y')iret=mousestruct[chan].ay;
	else if(toupper(*p)=='L')iret=mousestruct[chan].Z;
	else if(toupper(*p)=='R')iret=mousestruct[chan].C;
	else if(toupper(*p)=='W')iret=mousestruct[chan].L;
	else if(toupper(*p)=='S')iret=mousestruct[chan].az;
	else if(toupper(*p)=='T')iret=mousestruct[chan].classic[0];
	else if(toupper(*p)=='D'){
		iret=mousestruct[chan].R;
		mousestruct[chan].R=0;
	}
	else if(toupper(*p)=='Z'){
		iret=mousestruct[chan].az;
		mousestruct[chan].az=0;
	}
	else error("Syntax");
    targ = T_INT;
}

void fun_format(void) {
	char *p, *fmt;
	int inspec;
	getargs(&ep, 3, ",");
	if(argc%2 == 0) error("Invalid syntax");
	if(argc == 3)
		fmt = getCstring(argv[2]);
	else
		fmt = "%g";

	// check the format string for errors that might crash the CPU
	for(inspec = 0, p = fmt; *p; p++) {
		if(*p == '%') {
			inspec++;
			if(inspec > 1) error("Only one format specifier (%) allowed");
			continue;
		}

		if(inspec == 1 && (*p == 'g' || *p == 'G' || *p == 'f' || *p == 'e' || *p == 'E'|| *p == 'l'))
			inspec++;


		if(inspec == 1 && !(IsDigitinline(*p) || *p == '+' || *p == '-' || *p == '.' || *p == ' '))
			error("Illegal character in format specification");
	}
	if(inspec != 2) error("Format specification not found");
	sret = GetTempStrMemory();									// this will last for the life of the command
	sprintf(sret, fmt, getnumber(argv[0]));
	CtoM(sret);
	targ=T_STR;
}

void cmd_guiMX170(void) {
    char *p;
    if((p = checkstring(cmdline, "CURSOR"))){
    	cmd_cursor(p);
    	return;
    }
    if((p = checkstring(cmdline, "BITMAP"))){
        int x, y, fc, bc, h, w, scale, t, bytes;
        char *s;
        MMFLOAT f;
        long long int i64;
        int cursorhidden=0;

        getargs(&p, 15, ",");
        if(!(argc & 1) || argc < 5) error("Argument count");

        // set the defaults
        h = 8; w = 8; scale = 1; bytes = 8; fc = gui_fcolour; bc = gui_bcolour;

        x = getinteger(argv[0]);
        y = getinteger(argv[2]);

        // get the type of argument 3 (the bitmap) and its value (integer or string)
        t = T_NOTYPE;
        evaluate(argv[4], &f, &i64, &s, &t, true);
        if(t & T_NBR)
            error("Invalid argument");
        else if(t & T_INT)
            s = (char *)&i64;
        else if(t & T_STR)
            bytes = *s++;

        if(argc > 5 && *argv[6]) w = getint(argv[6], 1, HRes);
        if(argc > 7 && *argv[8]) h = getint(argv[8], 1, VRes);
        if(argc > 9 && *argv[10]) scale = getint(argv[10], 1, 15);
        if(argc > 11 && *argv[12]) fc = getint(argv[12], 0, WHITE);
        if(argc == 15) bc = getint(argv[14], -1, WHITE);
        if(h * w > bytes * 8) error("Not enough data");
		if(cursoron)
			if( !(xcursor + wcursor < x||
				xcursor > x + w * scale ||
				ycursor + hcursor < y ||
				ycursor > y + h * scale)){
				hidecursor(0);
				cursorhidden=1;
			}

        DrawBitmap(x, y, w, h, scale, fc, bc, (unsigned char *)s);
        if(cursorhidden)showcursor(0, xcursor, ycursor);
    	return;
    }
    error("Syntax");
}
long long int GetuSec(void){
	uint64_t fifty,gettime;
	do {
		fifty=uSecTimer;
		gettime= (uint64_t) __HAL_TIM_GET_COUNTER(&htim16) + uSecTimer*50000;
	} while(uSecTimer!=fifty);
	return gettime ;											        // this is the character
}
long long int Getuptime(void){
	uint64_t fifty,gettime;
	do {
		fifty=g_uptime;
		gettime= (uint64_t) __HAL_TIM_GET_COUNTER(&htim16) + g_uptime*50000;
	} while(g_uptime!=fifty);
	return gettime ;											        // this is the character
}
void fun_uSec(void) {
	fret=(MMFLOAT)GetuSec()/1000.0;
	targ=T_NBR;
}
void cmd_uSec(void){
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	uint64_t t = getinteger(++cmdline)*1000;
	HAL_NVIC_DisableIRQ(TIM16_IRQn);
	TIM16->CR1 &= ~1;
	uSecTimer=t/50000;
	__HAL_TIM_SET_COUNTER(&htim16, (uint16_t)(t % 50000));
	TIM16->CR1 |= 1;
	HAL_NVIC_EnableIRQ(TIM16_IRQn);

}

void fun_epoch(void){
	char *arg;
	struct tm  *tm;
	struct tm tma;
	tm=&tma;
	int d, m, y, h, min, s;
	if(!checkstring(ep, "NOW"))
	{
		arg = getCstring(ep);
		getargs(&arg, 11, "-/ :");										// this is a macro and must be the first executable stmt in a block
		if(!(argc == 11)) error("Syntax");
			d = atoi(argv[0]);
			m = atoi(argv[2]);
			y = atoi(argv[4]);
			if(d>1000){
				int tmp=d;
				d=y;
				y=tmp;
			}
			if(y >= 0 && y < 100) y += 2000;
			if(d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) error("Invalid date");
			h = atoi(argv[6]);
			min  = atoi(argv[8]);
			s = atoi(argv[10]);
			if(h < 0 || h > 23 || min < 0 || m > 59 || s < 0 || s > 59) error("Invalid time");
			day = d;
			month = m;
			year = y;
			tm->tm_year = y - 1900;
			tm->tm_mon = m - 1;
			tm->tm_mday = d;
			tm->tm_hour = h;
			tm->tm_min = min;
			tm->tm_sec = s;
	} else {
		RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
		tm->tm_year = year - 1900;
		tm->tm_mon = month - 1;
		tm->tm_mday = day;
		tm->tm_hour = hour;
		tm->tm_min = minute;
		tm->tm_sec = second;
	}
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    iret=timestamp;
	    targ = T_INT;
}
void cmd_pause(void) {
	static int interrupted = false;
    MMFLOAT f;
    static int64_t end,count;
    int64_t start, stop, tick;
    f = getnumber(cmdline);                                         // get the pulse width
    if(f < 0) error("Number out of bounds");
    if(f < 0.05) return;

	if(f < 1.5) {
		uSec(f * 1000);                                             // if less than 1.5mS do the pause right now
		return;                                                     // and exit straight away
    }
	if(!interrupted){
		count=(int64_t)(f*1000);
		start=GetuSec();
		tick=PauseTimer;
		while(PauseTimer==tick){}  //wait for the next clock tick
		stop=GetuSec();
		count-=(stop-start);
		end = (count % 1000); //get the number of ticks remaining
		count/=1000;
		PauseTimer=0;
	}
    if(count){
		if(InterruptReturn == NULL) {
			// we are running pause in a normal program
			// first check if we have reentered (from an interrupt) and only zero the timer if we have NOT been interrupted.
			// This means an interrupted pause will resume from where it was when interrupted
			if(!interrupted) PauseTimer = 0;
			interrupted = false;

			while(PauseTimer < count) {
				CheckAbort();
				if(check_interrupt()) {
					// if there is an interrupt fake the return point to the start of this stmt
					// and return immediately to the program processor so that it can send us off
					// to the interrupt routine.  When the interrupt routine finishes we should reexecute
					// this stmt and because the variable interrupted is static we can see that we need to
					// resume pausing rather than start a new pause time.
					while(*cmdline && *cmdline != cmdtoken) cmdline--;	// step back to find the command token
					InterruptReturn = cmdline;							// point to it
					interrupted = true;								    // show that this stmt was interrupted
					return;											    // and let the interrupt run
				}
			}
			interrupted = false;
		}
		else {
			// we are running pause in an interrupt, this is much simpler but note that
			// we use a different timer from the main pause code (above)
			IntPauseTimer = 0;
			while(IntPauseTimer < FloatToInt32(f)) CheckAbort();
		}
    }
	uSec(end);
}

void cmd_lmid(void)
{
    unsigned char *p;
    int num = -1;
    char *lsStart = NULL;
    int64_t *dest = NULL;
    getcsargs(&cmdline, 5);
    if (!(argc == 5 || argc == 3))
    	error("Argument count");
    int totalsize = (parseintegerarray(argv[0], &dest, 1, 1, NULL, true,NULL) - 1) * 8; // size of the longsting in bytes
    lsStart = (char *)&dest[1];
    int currentlength = dest[0];
    int start = getint(argv[2], 1, currentlength); // pick a starting point in the string
    if (argc == 5)
        num = getint(argv[4], 0, currentlength);
    if (start + (num < 0 ? 0 : num - 1) - 1 > currentlength)
        error("Selection exceeds length of string");
    start--; // position 1 is the array position 0
    while (*cmdline && tokenfunction(*cmdline) != op_equal)
        cmdline++;
    if (!*cmdline)
        SyntaxError();
    ;
    ++cmdline;
    if (!*cmdline)
        SyntaxError();
    ;
    char *value = (char *)getstring(cmdline);
    if (num == -1)
        num = value[0];
    p = (unsigned char *)&value[1];
    if (num == value[0])
        memcpy(&lsStart[start], p, num); // simple substitution
    else
    {
        int change = value[0] - num;
        if (currentlength + change > totalsize)
            error("String too long");
        // first move the original after the insertion to it's new position
        memmove(&lsStart[start + value[0]], &lsStart[start + num], totalsize - (start + num - 1));
        dest[0] += change;
        memcpy(&lsStart[start], p, value[0]);
    }
}

void cmd_longString(void){
    char *tp;
    tp = checkstring(cmdline, (char *)"SETBYTE");
    if(tp){
        int64_t *dest=NULL;
        int p=0;
        uint8_t *q=NULL;
        int nbr;
        int j=0;
    	getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=(parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1)*8-1;
        q=(uint8_t *)&dest[1];
        p = getint(argv[2],OptionBase,j-OptionBase);
        nbr=getint(argv[4],0,255);
        q[p-OptionBase]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"APPEND");
    if(tp){
        int64_t *dest=NULL;
        char *p= NULL;
        char *q= NULL;
        int i,j,nbr;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1;
        q=(char *)&dest[1];
        q+=dest[0];
        p=(char *)getstring(argv[2]);
        nbr = i = *p++;
        if(j*8 < dest[0]+i)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"TRIM");
    if(tp){
        int64_t *dest=NULL;
        uint32_t trim;
        char *p, *q=NULL;
        int i;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL);
        q=(char *)&dest[1];
        trim=getint(argv[2],1,dest[0]);
        i = dest[0]-trim;
        p=q+trim;
        while(i--)*q++=*p++;
        dest[0]-=trim;
        return;
    }
    tp = checkstring(cmdline, (char *)"REPLACE");
    if(tp){
        int64_t *dest=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL);
        q=(char *)&dest[1];
        p=(char *)getstring(argv[2]);
        nbr=getint(argv[4],1,dest[0]-*p+1);
        q+=nbr-1;
        i = *p++;
        while(i--)*q++=*p++;
        return;
    }
    tp = checkstring(cmdline, (char *)"LOAD");
    if(tp){
        int64_t *dest=NULL;
        char *p;
        char *q=NULL;
        int i,j;
        getargs(&tp, 5, ( char *)",");
        if(argc != 5)error("Argument count");
        int64_t nbr=getinteger(argv[2]);
        i=nbr;
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1;
        q=(char *)&dest[1];
        dest[0]=0;
        p=(char *)getstring(argv[4]);
        if(nbr> *p)nbr=*p;
        p++;
        if(j*8 < dest[0]+nbr)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"LEFT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false,NULL);
        p=(char *)&src[1];
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0])nbr=i=src[0];
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"RIGHT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false,NULL);
        p=(char *)&src[1];
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0]){
            nbr=i=src[0];
        } else p+=(src[0]-nbr);
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"MID");
    if(tp){
       int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr,start;
        getargs(&tp, 7,(char *)",");
        if(argc < 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false,NULL);
        p=(char *)&src[1];
        start=getint(argv[4],1,src[0]);
        if(argc==7)nbr=getinteger(argv[6]);
        else nbr=src[0];
        p+=start-1;
        if(nbr+start>src[0]){
            nbr=src[0]-start+1;
        }
        i=nbr;
        if(j*8 < nbr)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }

    tp = checkstring(cmdline, (char *)"CLEAR");
    if(tp){
        int64_t *dest=NULL;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL);
        dest[0]=0;
        return;
    }
    tp = checkstring(cmdline, (char *)"RESIZE");
    if(tp){
        int64_t *dest=NULL;
        int j=0;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=(parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1)*8;
        dest[0] = getint(argv[2], 0, j);
        return;
    }
    tp = checkstring(cmdline, (char *)"UCASE");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL);
        q=(char *)&dest[1];
        i=dest[0];
        while(i--){
        if(*q >= 'a' && *q <= 'z')
            *q -= 0x20;
        q++;
        }
        return;
    }
    /* Picomite version  */
    tp = checkstring(cmdline, "PRINT");
    if (tp)
    {
        int64_t *dest = NULL;
        char *q = NULL;
        int j, fnbr = 0;
        bool docrlf = true;
        getargs(&tp, 5, ",;");
        if (argc == 5)
            error("Syntax");
        if (argc >= 3)
        {
            if (*argv[0] == '#')
                argv[0]++;              // check if the first arg is a file number
            fnbr = getinteger(argv[0]); // get the number
            parseintegerarray(argv[2], &dest, 2, 1, NULL, true,NULL);
            if (argc == 4)
                if (*argv[3] == ';')
                    docrlf = false;
        }
        else
        {
            parseintegerarray(argv[0], &dest, 1, 1, NULL, true,NULL);
            if (argc == 2)
                if (*argv[1] == ';')
                    docrlf = false;
        }
        q = (char *)&dest[1];
        j = dest[0];
        while (j--)
        {
            MMfputc(*q++, fnbr);
        }
        if (docrlf)
            MMfputs("\2\r\n", fnbr);
        return;
    }


    tp = checkstring(cmdline, (char *)"LCASE");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL);
        q=(char *)&dest[1];
        i=dest[0];
        while(i--){
            if(*q >= 'A' && *q <= 'Z')
                *q += 0x20;
            q++;
        }
        return;
    }
    tp = checkstring(cmdline, (char *)"COPY");
    if(tp){
       int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL);
        q=(char *)&dest[1];
        dest[0]=0;
        parseintegerarray(argv[2],&src,2,1,NULL,false,NULL);
        p=(char *)&src[1];
        if((j-i)*8 < src[0])error("Destination array too small");
        i=src[0];
        while(i--)*q++=*p++;
        dest[0]=src[0];
        return;
    }
    tp = checkstring(cmdline, (char *)"CONCAT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j,d=0,s=0;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true,NULL)-1;
        q=(char *)&dest[1];
        d=dest[0];
        parseintegerarray(argv[2],&src,2,1,NULL,false,NULL);
        p=(char *)&src[1];
        i = s = src[0];
        if(j*8 < (d+s))error("Destination array too small");
        q+=d;
        while(i--)*q++=*p++;
        dest[0]+=src[0];
        return;
    }
    //unsigned char * parselongAES(uint8_t *p, int ivadd, uint8_t *keyx, uint8_t *ivx, int64_t **inint, int64_t **outint)
        tp = checkstring(cmdline, "AES128");
        if(tp) {
            struct AES_ctx ctx;
            unsigned char keyx[16];
            char * p;
            int64_t *dest=NULL, *src=NULL;
            char *qq=NULL;
            char *q=NULL;
    //void parselongAES(uint8_t *p, int ivadd, uint8_t *keyx, uint8_t *ivx, int64_t **inint, int64_t **outint){
            if((p=checkstring(tp, "ENCRYPT CBC"))){
                uint8_t iv[16];
                for(int i=0;i<16;i++)iv[i]=getrnd();
                parselongAES(p, 16, &keyx[0], &iv[0], &src, &dest);
                qq=(char *)&src[1];
                q=(char *)&dest[1];
                dest[0]=src[0]+16;
                memcpy(&q[16],qq,src[0]);
                memcpy(q,iv,16);
                AES_init_ctx_iv(&ctx, keyx, iv);
                AES_CBC_encrypt_buffer(&ctx, (unsigned char *)&q[16], src[0]);
                return;
            } else if((p=checkstring(tp,"DECRYPT CBC"))){
                uint8_t iv[16];
                parselongAES(p, -16, &keyx[0], NULL, &src, &dest);
                dest[0]=src[0]-16;
                qq=(char *)&src[1];
                q=(char *)&dest[1];
                memcpy(iv,qq,16); //restore the IV
                memcpy(q,&qq[16],dest[0]);
                AES_init_ctx_iv(&ctx, keyx, iv);
                AES_CBC_decrypt_buffer(&ctx, (unsigned char *)q, dest[0]);
                return;
            } else if((p=checkstring(tp,"ENCRYPT ECB"))){
                struct AES_ctx ctxcopy;
                parselongAES(p, 0, &keyx[0], NULL, &src, &dest);
                qq=(char *)&src[1];
                q=(char *)&dest[1];
                dest[0]=src[0];
                memcpy(q,qq,src[0]);
                AES_init_ctx(&ctxcopy, keyx);
                for(int i=0;i<src[0];i+=16){
                    memcpy(&ctx,&ctxcopy,sizeof(ctx));
                    AES_ECB_encrypt(&ctx, (unsigned char *)&q[i]);
                }
                return;
            } else if((p=checkstring(tp, "DECRYPT ECB"))){
                struct AES_ctx ctxcopy;
                parselongAES(p, 0, &keyx[0], NULL, &src, &dest);
                qq=(char *)&src[1];
                q=(char *)&dest[1];
                dest[0]=src[0];
                memcpy(q,qq,src[0]);
                AES_init_ctx(&ctxcopy, keyx);
                for(int i=0;i<src[0];i+=16){
                    memcpy(&ctx,&ctxcopy,sizeof(ctx));
                    AES_ECB_decrypt(&ctx, (unsigned char *)&q[i]);
                }
                return;
            } else if((p=checkstring(tp, "ENCRYPT CTR"))){
                uint8_t iv[16];
                for(int i=0;i<16;i++)iv[i]=getrnd();
                parselongAES(p, 16, &keyx[0], &iv[0], &src, &dest);
                qq=(char *)&src[1];
                q=(char *)&dest[1];
                dest[0]=src[0]+16;
                memcpy(&q[16],qq,src[0]);
                memcpy(q,iv,16);
                AES_init_ctx_iv(&ctx, keyx, iv);
                AES_CTR_xcrypt_buffer(&ctx, (unsigned char *)&q[16], src[0]);
                return;
            } else if((p=checkstring(tp, "DECRYPT CTR"))){
                uint8_t iv[16];
                parselongAES(p, -16, &keyx[0], NULL, &src, &dest);
                dest[0]=src[0]-16;
                qq=(char *)&src[1];
                q=(char *)&dest[1];
                memcpy(iv,qq,16); //restore the IV
                memcpy(q,&qq[16],dest[0]);
                AES_init_ctx_iv(&ctx, keyx, iv);
                AES_CTR_xcrypt_buffer(&ctx, (unsigned char *)q, dest[0]);
                return;
            } else error("Syntax");
        }
        tp = checkstring(cmdline,"BASE64");
        if(tp) {
            char * p;
            if((p=checkstring(tp, "ENCODE"))){
                int64_t *dest=NULL, *src=NULL;
                unsigned char *qq=NULL;
                unsigned char *q=NULL;
                int j;
                getargs(&p, 3, ",");
                if(argc != 3)error("Argument count");
                j=parseintegerarray(argv[2],&dest,2,1,NULL,true,NULL)-1;
                q=(unsigned char *)&dest[1];
                parseintegerarray(argv[0],&src,1,1,NULL,false,NULL);
                qq=(unsigned char *)&src[1];
                if(j*8 < b64e_size(src[0]))error("Destination array too small");
                dest[0]=b64_encode(qq, src[0], q);
                return;
            } else if((p=checkstring(tp, "DECODE"))){
                int64_t *dest=NULL, *src=NULL;
                unsigned char *qq=NULL;
                unsigned char *q=NULL;
                int j;
                getargs(&p, 3,",");
                if(argc != 3)error("Argument count");
                j=parseintegerarray(argv[2],&dest,2,1,NULL,true,NULL)-1;
                q=(unsigned char *)&dest[1];
                parseintegerarray(argv[0],&src,1,1,NULL,false,NULL);
                qq=(unsigned char *)&src[1];
                if(j*8 < b64d_size(src[0]))error("Destination array too small");
                dest[0]=b64_decode(qq, src[0], q);
                return;
            } else error("Syntax");
        }
    error("Invalid option");
}

void parselongAES(char *p, int ivadd, uint8_t *keyx, uint8_t *ivx, int64_t **inint, int64_t **outint){
	int64_t *a1int=NULL, *a2int=NULL, *a3int=NULL, *a4int=NULL;
	unsigned char *a1str=NULL,*a4str=NULL;
	MMFLOAT *a1float=NULL, *a4float=NULL;
	int card1, card3;
	getargs(&p,7,",");
	if(ivx==NULL){
		if(argc!=5)error("Syntax");
	} else {
		if(argc<5)error("Syntax");
	}
	*outint=NULL;
// first process the key
	int length=0;
	card1= parseany(argv[0], &a1float, &a1int, &a1str, &length, false);
	if(card1!=16)error("Key must be 16 elements long");
	if(a1int!=NULL){
		for(int i=0;i<16;i++){
			if(a1int[i]<0 || a1int[i]>255)error("Key number out of bounds 0-255");
			keyx[i]=a1int[i];
		}
	} else if (a1float!=NULL){
		for(int i=0;i<16;i++){
			if(a1float[i]<0 || a1float[i]>255)error("Key number out of bounds 0-255");
			keyx[i]=a1float[i];
		}
	} else if(a1str!=NULL){
		for(int i=0;i<16;i++){
			keyx[i]=a1str[i+1];
		}
	}
//next process the initialisation vector if any
	if(argc==7){
		length=0;
		card1= parseany(argv[6], &a4float, &a4int, &a4str, &length, false);
		if(card1!=16)error("Initialisation vector must be 16 elements long");
		if(a4int!=NULL){
			for(int i=0;i<16;i++){
				if(a4int[i]<0 || a4int[i]>255)error("Key number out of bounds 0-255");
				ivx[i]=a4int[i];
			}
		} else if (a4float!=NULL){
			for(int i=0;i<16;i++){
				if(a4float[i]<0 || a4float[i]>255)error("Key number out of bounds 0-255");
				ivx[i]=a4float[i];
			}
		} else if(a4str!=NULL){
			for(int i=0;i<16;i++){
				ivx[i]=a4str[i+1];
			}
		}
	}
//now process the longstring used for input
	parseintegerarray(argv[2],&a2int,2,1,NULL,false,NULL);
	if(*a2int % 16)error("input must be multiple of 16 elements long");
    *inint=a2int;
	card3=parseintegerarray(argv[4],&a3int,3,1,NULL,false,NULL);
	if((card3-1)*8<*a2int + ivadd)error("Output array too small");
    *outint=a3int;
}


void fun_LGetStr(void){
        char *p;
        char *s=NULL;
        int64_t *src=NULL;
        int start,nbr,j;
        getargs(&ep, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=(parseintegerarray(argv[0],&src,2,1,NULL,false,NULL)-1)*8;
        start = getint(argv[2],1,j);
        nbr = getinteger(argv[4]);
        if(nbr < 1 || nbr > MAXSTRLEN) error("Number out of bounds");
        if(start+nbr>src[0])nbr=src[0]-start+1;
        sret = GetTempMemory(STRINGSIZE);                                       // this will last for the life of the command
        s=(char *)&src[1];
        s+=(start-1);
        p=(char *)sret+1;
        *sret=nbr;
        while(nbr--)*p++=*s++;
        *p=0;
        targ = T_STR;
}

void fun_linputstr(void)
{
    int j, nbr, fnbr;
    int64_t *dest = NULL;
    uint8_t *q = NULL;
    unsigned int read = 0;
    getcsargs(&ep, 5);
    if (argc != 5)
        SyntaxError();
    j = (parseintegerarray(argv[0], &dest, 1, 1, NULL, true,NULL) - 1) * 8;
    q = (uint8_t *)&dest[1];
    nbr = getint(argv[4], OptionBase, j - OptionBase);
    if (*argv[2] == '#')
        argv[2]++;
    fnbr = getinteger(argv[2]);
    if (fnbr == 0)
        SyntaxError();
    if (fnbr < 1 || fnbr > MAXOPENFILES)
        StandardError(18);
    if (FileTable[fnbr].com == 0)
        StandardError(19);
    if (FileTable[fnbr].com <= MAXCOMPORTS)
        error("Input from file only");
    FileGetData(fnbr, q, nbr, &read);
    dest[0] = read; // update the length of the string
    iret = dest[0];
    targ = T_INT;
}

void fun_LGetByte(void){
        uint8_t *s=NULL;
        int64_t *src=NULL;
        int start,j;
    	getargs(&ep, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=(parseintegerarray(argv[0],&src,2,1,NULL,false,NULL)-1)*8;
        s=(uint8_t *)&src[1];
        start = getint(argv[2],OptionBase,j-OptionBase);
        iret=s[start-OptionBase];
        targ = T_INT;
}

// Updated from Picomite to use latest REGEX library.
// Caters for OPTION ESCAPE being on.
// Allows alternate form which evaluates the search string as a regular expression
//see fun_Instr
void fun_LInstr(void){
    int64_t *src=NULL;
    char srch[STRINGSIZE];
    char *str=NULL;
    int slen,found=0,i,j,n;
    getargs(&ep, 7, ",");
    if(argc <3  || argc > 7)error("Argument count");
    int64_t start;
    if(argc>=5 && *argv[4])
        start=getinteger(argv[4])-1;
    else
         start=0;
    j=(parseintegerarray(argv[0],&src,2,1,NULL,false,NULL)-1);
    str=(char *)&src[0];
    if(argc<7){
        strcpy((char *)srch,( char *)getstring(argv[2]));
        slen=*srch;
        iret=0;
        if(start>src[0] || start<0 || slen==0 || src[0]==0 || slen>src[0]-start)
           found=1;
        if(!found){
           n=src[0]- slen - start;

            for(i = start; i <= n + start; i++) {
                if(str[i + 8] == srch[1]) {
                    for(j = 0; j < slen; j++)
                        if(str[j + i + 8] != srch[j + 1])
                            break;
                    if(j == slen) {iret= i + 1; break;}
                }
            }
        }
     } else {  //search string is a regular expression
           // Uses new regex library by Peter
            int tmp = OptionEscape;
            OptionEscape = 0;
        	strcpy((char *)srch,( char *)getstring(argv[2]));
            OptionEscape = tmp;
            int match_length;
            void *temp;
            MMFLOAT *tempf = NULL;
            int64_t *tempi = NULL;
            MtoC(( char *)srch);
            temp = findvar(argv[6], V_FIND);
            if(!(vartbl[VarIndex].type & (T_NBR |T_INT)))
                error("Invalid variable");
            if (vartbl[VarIndex].type & T_INT)
               tempi = temp;
            else
               tempf = temp;
           	int match_idx = re_match(srch, &str[start+8], &match_length);
         	if (match_idx != -1){
              if(tempf)
                 *tempf=(MMFLOAT)(match_length);
              else
                *tempi =(int64_t)(match_length);
           	  iret=match_idx+1+start;
           	 }else{
                if (tempf)
                   *tempf = 0.0;
                else
                    *tempi = 0;
          	}
	 }
     targ = T_INT;
}

void fun_LCompare(void){
    int64_t *dest, *src;
    char *p=NULL;
    char *q=NULL;
    int d=0,s=0,found=0;
    getargs(&ep, 3, (char *)",");
    if(argc != 3)error("Argument count");
    parseintegerarray(argv[0],&dest,1,1,NULL,false,NULL);
    q=(char *)&dest[1];
    d=dest[0];
    parseintegerarray(argv[2],&src,1,1,NULL,false,NULL);
    p=(char *)&src[1];
    s=src[0];
    while(!found) {
        if(d == 0 && s == 0) {found=1;iret=0;}
        if(d == 0 && !found) {found=1;iret=-1;}
        if(s == 0 && !found) {found=1;iret=1;}
        if(*q < *p && !found) {found=1;iret=-1;}
        if(*q > *p && !found) {found=1;iret=1;}
        q++;  p++;  d--; s--;
    }
    targ = T_INT;
}

void fun_LLen(void) {
    int64_t *dest=NULL;
    getargs(&ep, 1, (char *)",");
    if(argc != 1)error("Argument count");
    parseintegerarray(argv[0],&dest,1,1,NULL,false,NULL);
    iret=dest[0];
    targ = T_INT;
}


#ifdef PRFEPARSE
void cmd_longString(void){
    char *tp;
    tp = checkstring(cmdline, "SETBYTE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        int p=0;
        uint8_t *q=NULL;
        int nbr;
        int j=0;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            j=(vartbl[VarIndex].dims[0] - OptionBase)*8-1;
            dest = (long long int *)ptr1;
            q=(uint8_t *)&dest[1];
        } else error("Argument 1 must be integer array");
        p = getint(argv[2],OptionBase,j-OptionBase);
        nbr=getint(argv[4],0,255);
        q[p-OptionBase]=nbr;
         return;
    }
    tp = checkstring(cmdline, "APPEND");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *p= NULL;
        char *q= NULL;
        int i,j,nbr;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
            q+=dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        p=getstring(argv[2]);
        nbr = i = *p++;
         if(j*8 < dest[0]+i)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, "TRIM");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        uint32_t trim;
        char *p, *q=NULL;
        int i;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        trim=getint(argv[2],1,dest[0]-1);
        i = dest[0]-trim;
        p=q+trim;
        while(i--)*q++=*p++;
        dest[0]-=trim;
        return;
    }
    tp = checkstring(cmdline, "REPLACE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,nbr;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        p=getstring(argv[2]);
        nbr=getint(argv[4],1,dest[0]-*p+1);
        q+=nbr-1;
        i = *p++;
        while(i--)*q++=*p++;
        return;
    }
    tp = checkstring(cmdline, "LOAD");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *p;
        char *q=NULL;
        int i,j;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        int64_t nbr=getinteger(argv[2]);
        i=nbr;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            dest[0]=0;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        p=getstring(argv[4]);
        if(nbr> *p)nbr=*p;
        p++;
        if(j*8 < dest[0]+nbr)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, "LEFT");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
        } else error("Argument 2 must be integer array");
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0])nbr=i=src[0];
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, "RIGHT");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
        } else error("Argument 2 must be integer array");
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0]){
            nbr=i=src[0];
        } else p+=(src[0]-nbr);
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, "MID");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr,start;
    	getargs(&tp, 7, ",");
        if(argc != 7)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
        } else error("Argument 2 must be integer array");
        start=getint(argv[4],1,src[0]);
        nbr=getinteger(argv[6]);
        p+=start-1;
        if(nbr+start>src[0]){
            nbr=src[0]-start+1;
        }
        i=nbr;
        if(j*8 < nbr)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, "CLEAR");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        getargs(&tp, 1, ",");
        if(argc != 1)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
        } else error("Argument 1 must be integer array");
        dest[0]=0;
        return;
    }
    tp = checkstring(cmdline, "RESIZE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        int j=0;
        getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            j=(vartbl[VarIndex].dims[0] - OptionBase)*8;
            dest = (long long int *)ptr1;
        } else error("Argument 1 must be integer array");
        dest[0] = getint(argv[2], 0, j);
        return;
    }
    tp = checkstring(cmdline, "UCASE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
    	getargs(&tp, 1, ",");
        if(argc != 1)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        i=dest[0];
        while(i--){
        if(*q >= 'a' && *q <= 'z')
            *q -= 0x20;
        q++;
        }
        return;
    }
    tp = checkstring(cmdline, "PRINT");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *q=NULL;
        int i, j, fnbr;
    	getargs(&tp, 5, ",;");
        if(argc < 1 || argc > 4)error("Argument count");
        if(argc > 0 && *argv[0] == '#') {								// check if the first arg is a file number
            argv[0]++;
            fnbr = getinteger(argv[0]);									// get the number
            i = 1;
            if(argc >= 2 && *argv[1] == ',') i = 2;						// and set the next argument to be looked at
        }
        else {
            fnbr = 0;													// no file number so default to the standard output
            i = 0;
        }
        if(argc>=1){
            ptr1 = findvar(argv[i], V_FIND | V_EMPTY_OK);
            if(vartbl[VarIndex].type & T_INT) {
                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                    error("Argument must be integer array");
                }
                dest = (long long int *)ptr1;
                q=(char *)&dest[1];
            } else error("Argument must be integer array");
            j=dest[0];
            while(j--){
                MMfputc(*q++, fnbr);
            }
            i++;
        }
        if(argc > i){
            if(*argv[i] == ';') return;
        }
        MMfputs("\2\r\n", fnbr);
        return;
    }
    tp = checkstring(cmdline, "LCASE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
    	getargs(&tp, 1, ",");
        if(argc != 1)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        i=dest[0];
        while(i--){
            if(*q >= 'A' && *q <= 'Z')
                *q += 0x20;
            q++;
        }
        return;
    }
    tp = checkstring(cmdline, "COPY");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            dest[0]=0;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
            i=src[0];
        } else error("Argument 2 must be integer array");
        if(j*8 <i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=src[0];
        return;
    }
    tp = checkstring(cmdline, "CONCAT");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j,d=0,s=0;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            d=dest[0];
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
            i = s = src[0];
        } else error("Argument 2 must be integer array");
        if(j*8 < (d+s))error("Destination array too small");
        q+=d;
        while(i--)*q++=*p++;
        dest[0]+=src[0];
        return;
    }
    error("Invalid option");
}

#endif

#ifdef PRFEPARSE
void fun_LGetStr(void){
        void *ptr1 = NULL;
        char *p;
        char *s=NULL;
        int64_t *src=NULL;
        int start,nbr,j;
    	getargs(&ep, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            src = (int64_t *)ptr1;
            s=(char *)&src[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase)*8;
        start = getint(argv[2],1,j);
	nbr = getinteger(argv[4]);
	if(nbr < 1 || nbr > MAXSTRLEN) error("Number out of bounds");
        if(start+nbr>src[0])nbr=src[0]-start+1;
	sret = GetTempStrMemory();                                       // this will last for the life of the command
        s+=(start-1);
        p=sret+1;
        *sret=nbr;
        while(nbr--)*p++=*s++;
        *p=0;
        targ = T_STR;
}

void fun_LGetByte(void){
        void *ptr1 = NULL;
        uint8_t *s=NULL;
        int64_t *src=NULL;
        int start,j;
    	getargs(&ep, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            src = (int64_t *)ptr1;
            s=(uint8_t *)&src[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase)*8-1;
        start = getint(argv[2],OptionBase,j-OptionBase);
        iret=s[start-OptionBase];
        targ = T_INT;
}


void fun_LInstr(void){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *srch;
        char *str=NULL;
        int slen,found=0,i,j,n;
        getargs(&ep, 7, ",");
        if(argc <3  || argc > 7)error("Argument count");
        int64_t start;
        if(argc>=5 && *argv[4])start=getinteger(argv[4])-1;
        else start=0;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            str=(char *)&dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        srch=getstring(argv[2]);
        if(argc<7){
            slen=*srch;
            iret=0;
            if(start>dest[0] || start<0 || slen==0 || dest[0]==0 || slen>dest[0]-start)found=1;
            if(!found){
                n=dest[0]- slen - start;

                for(i = start; i <= n + start; i++) {
                    if(str[i + 8] == srch[1]) {
                        for(j = 0; j < slen; j++)
                            if(str[j + i + 8] != srch[j + 1])
                                break;
                        if(j == slen) {iret= i + 1; break;}
                    }
                }
            }
        } else { //search string is a regular expression
            regex_t regex;
            int reti;
            regmatch_t pmatch;
            MMFLOAT *temp=NULL;
            MtoC(srch);
            temp = findvar(argv[6], V_FIND);
            if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");
            reti = regcomp(&regex, srch, 0);
            if( reti ) error("Could not compile regex");
	        reti = regexec(&regex, &str[start+8], 1, &pmatch, 0);
            if( !reti ){
                iret=pmatch.rm_so+1+start;
                if(temp)*temp=(MMFLOAT)(pmatch.rm_eo-pmatch.rm_so);
            }
            else if( reti == REG_NOMATCH ){
                iret=0;
                if(temp)*temp=0.0;
            }
            else{
                error("Regex execution error");
            }
        }
        targ = T_INT;
}


void fun_LCompare(void){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest, *src;
        char *p=NULL;
        char *q=NULL;
        int d=0,s=0,found=0;
    	getargs(&ep, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
            d=dest[0];
        } else error("Argument 1 must be integer array");
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
            s=src[0];
        } else error("Argument 2 must be integer array");
    while(!found) {
        if(d == 0 && s == 0) {found=1;iret=0;}
        if(d == 0 && !found) {found=1;iret=-1;}
        if(s == 0 && !found) {found=1;iret=1;}
        if(*q < *p && !found) {found=1;iret=-1;}
        if(*q > *p && !found) {found=1;iret=1;}
        q++;  p++;  d--; s--;
    }
        targ = T_INT;
}

void fun_LLen(void) {
    void *ptr1 = NULL;
    int64_t *dest=NULL;
    getargs(&ep, 1, ",");
    if(argc != 1)error("Argument count");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be integer array");
        }
        dest = (long long int *)ptr1;
    } else error("Argument 1 must be integer array");
    iret=dest[0];
    targ = T_INT;
}
#endif

void copy_clock(void){
	if(Option.RTCinstalled==0)return;
	uint8_t c[7];
	DS3231_RD_Reg(0,c,7);
	hour=((c[2] & 0x30)>>4)*10 + (c[2] & 0x0f);
	minute=((c[1] & 0x70)>>4)*10 + (c[1] & 0x0f);
	second=((c[0] & 0x70)>>4)*10 + (c[0] & 0x0f);
	year=((c[6] & 0xf0)>>4)*10 + (c[6] & 0x0f);
	month=((c[5] & 0x10)>>4)*10 + (c[5] & 0x0f);
	day=((c[4] & 0x30)>>4)*10 + (c[4] & 0x0f);
	day_of_week=c[3];
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	sTime.Hours = hour;
	sTime.Minutes = minute;
	sTime.Seconds = second;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	sDate.WeekDay = day_of_week;
	sDate.Month = month;
	sDate.Date = day;
	sDate.Year = year;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
}



void update_clock(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	sTime.Hours = hour;
	sTime.Minutes = minute;
	sTime.Seconds = second;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	sDate.WeekDay = day_of_week;
	sDate.Month = month;
	sDate.Date = day;
	sDate.Year = year-2000;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
}


// this is invoked as a command (ie, date$ = "6/7/2010")
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command, split it up and save in the system counters
void cmd_date(void) {
	char *arg;
	struct tm  *tm;
	struct tm tma;
	tm=&tma;
	int dd, mm, yy;
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
	arg = getCstring(cmdline);
	{
		getargs(&arg, 5, "-/");										// this is a macro and must be the first executable stmt in a block
		if(argc != 5) error("Syntax");
		dd = atoi(argv[0]);
		mm = atoi(argv[2]);
		yy = atoi(argv[4]);
		if(yy >= 0 && yy < 100) yy += 2000;
	    //check year
	    if(yy>=1900 && yy<=9999)
	    {
	        //check month
	        if(mm>=1 && mm<=12)
	        {
	            //check days
	            if((dd>=1 && dd<=31) && (mm==1 || mm==3 || mm==5 || mm==7 || mm==8 || mm==10 || mm==12))
	                {}
	            else if((dd>=1 && dd<=30) && (mm==4 || mm==6 || mm==9 || mm==11))
	                {}
	            else if((dd>=1 && dd<=28) && (mm==2))
	                {}
	            else if(dd==29 && mm==2 && (yy%400==0 ||(yy%4==0 && yy%100!=0)))
	                {}
	            else
	                error("Day is invalid");
	        }
	        else
	        {
	            error("Month is not valid");
	        }
	    }
	    else
	    {
	        error("Year is not valid");
	    }

		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		day = dd;
		month = mm;
		year = yy;
	    tm->tm_year = year - 1900;
	    tm->tm_mon = month - 1;
	    tm->tm_mday = day;
	    tm->tm_hour = hour;
	    tm->tm_min = minute;
	    tm->tm_sec = second;
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    tm=gmtime(&timestamp);
	    day_of_week=tm->tm_wday;
	    if(day_of_week==0)day_of_week=7;
		update_clock();
		mT4IntEnable(1);       										// enable interrupt
		if(Option.RTCinstalled){
			uint8_t c[4];
			c[0]=day_of_week;
			c[1]=((day/10)<<4) + (day % 10);
			c[2]=((month/10)<<4) + (month % 10);
			c[3]=(((year-2000)/10)<<4) + ((year-2000) % 10);
			DS3231_WR_Reg(3,c,4);
		}
	}
}

// this is invoked as a function
void fun_date(void) {
	uint8_t c[3];
    sret = GetTempStrMemory();                                    // this will last for the life of the command
	if(Option.RTCinstalled==0){
		RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
		IntToStrPad(sret, day, '0', 2, 10);
		sret[2] = '-'; IntToStrPad(sret + 3, month, '0', 2, 10);
		sret[5] = '-'; IntToStr(sret + 6, year, 10);
	} else {
		DS3231_RD_Reg(4,c,3);
		year=((c[2] & 0xf0)>>4)*10 + (c[2] & 0x0f);
		month=((c[1] & 0x10)>>4)*10 + (c[1] & 0x0f);
		day=((c[0] & 0x30)>>4)*10 + (c[0] & 0x0f);
		IntToStrPad(sret, hour, '0', 2, 10);
		IntToStrPad(sret, day, '0', 2, 10);
		sret[2] = '-'; IntToStrPad(sret + 3, month, '0', 2, 10);
		sret[5] = '-'; IntToStr(sret + 6, year+2000, 10);
	}
	CtoM(sret);
    targ = T_STR;
}

// this is invoked as a function
void fun_day(void) {
    char *arg;
    struct tm  *tm;
    struct tm tma;
    tm=&tma;
    time_t time_of_day;
    int i;
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    int d, m, y;
    if(!checkstring(ep, "NOW"))
    {
        arg = getCstring(ep);
        getargs(&arg, 5, "-/");										// this is a macro and must be the first executable stmt in a block
        if(!(argc == 5))error("Syntax");
        d = atoi(argv[0]);
        m = atoi(argv[2]);
        y = atoi(argv[4]);
		if(d>1000){
			int tmp=d;
			d=y;
			y=tmp;
		}
        if(y >= 0 && y < 100) y += 2000;
        if(d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) error("Invalid date");
        tm->tm_year = y - 1900;
        tm->tm_mon = m - 1;
        tm->tm_mday = d;
        tm->tm_hour = 0;
        tm->tm_min = 0;
        tm->tm_sec = 0;
        time_of_day = timegm(tm);
        tm=gmtime(&time_of_day);
        i=tm->tm_wday;
        if(i==0)i=7;
    	strcpy(sret,daystrings[i]);
    } else {
        RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
    	strcpy(sret,daystrings[day_of_week]);
    }
    CtoM(sret);
    targ = T_STR;
}

// this is invoked as a command (ie, time$ = "6:10:45")
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command, split it up and save in the system counters
void cmd_time(void) {
	char *arg;
	int h = 0;
	int m = 0;
	int s = 0;
    MMFLOAT f;
    long long int i64;
    char *ss;
    int t=0;
    int offset;
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
    evaluate(cmdline, &f, &i64, &ss, &t, false);
	if(t & T_STR){
		arg = getCstring(cmdline);
		{
			getargs(&arg, 5, ":");								// this is a macro and must be the first executable stmt in a block
			if(argc%2 == 0) error("Syntax");
			h = atoi(argv[0]);
			if(argc >= 3) m = atoi(argv[2]);
			if(argc == 5) s = atoi(argv[4]);
			if(h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) error("Invalid time");
			mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
			hour = h;
			minute = m;
			second = s;
			SecondsTimer = 0;
			update_clock();
	    	mT4IntEnable(1);       										// enable interrupt
	    }
	} else {
		struct tm  *tm;
		struct tm tma;
		tm=&tma;
		offset=getinteger(cmdline);
		RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
		tm->tm_year = year - 1900;
		tm->tm_mon = month - 1;
		tm->tm_mday = day;
		tm->tm_hour = hour;
		tm->tm_min = minute;
		tm->tm_sec = second;
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    timestamp+=offset;
	    tm=gmtime(&timestamp);
		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		hour = tm->tm_hour;
		minute = tm->tm_min;
		second = tm->tm_sec;
		SecondsTimer = 0;
		update_clock();
    	mT4IntEnable(1);       										// enable interrupt

	}
	if(Option.RTCinstalled){
		uint8_t c[3];
		c[0]=((second/10)<<4) + (second % 10);
		c[1]=((minute/10)<<4) + (minute % 10);
		c[2]=((hour/10)<<4) + (hour % 10);
		DS3231_WR_Reg(0,c,3);
	}
}




// this is invoked as a function
void fun_time(void) {
	uint8_t c[3];
	sret = GetTempStrMemory();									// this will last for the life of the command
    if(Option.fulltime || Option.RTCinstalled==0){
		RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
		IntToStrPad(sret, hour, '0', 2, 10);
		sret[2] = ':'; IntToStrPad(sret + 3, minute, '0', 2, 10);
		sret[5] = ':'; IntToStrPad(sret + 6, second, '0', 2, 10);
	    if(Option.fulltime){
	    	sret[8] = '.'; IntToStrPad(sret + 9, milliseconds, '0', 3, 10);
	    }
    } else {
		DS3231_RD_Reg(0,c,3);
		hour=((c[2] & 0x30)>>4)*10 + (c[2] & 0x0f);
		minute=((c[1] & 0x70)>>4)*10 + (c[1] & 0x0f);
		second=((c[0] & 0x70)>>4)*10 + (c[0] & 0x0f);
		IntToStrPad(sret, hour, '0', 2, 10);
		sret[2] = ':'; IntToStrPad(sret + 3, minute, '0', 2, 10);
		sret[5] = ':'; IntToStrPad(sret + 6, second, '0', 2, 10);
    }
	CtoM(sret);
    targ = T_STR;
}



void cmd_ireturn(void){
  if(InterruptReturn == NULL) error("Not in interrupt");
  checkend(cmdline);
  nextstmt = InterruptReturn;
  if(LocalIndex)    ClearVars(LocalIndex--);                        // delete any local variables
    TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
    *CurrentInterruptName = 0;                                        // for static vars we are not in an interrupt
    InterruptReturn = NULL;
    if(DelayedDrawKeyboard) {
        DelayedDrawKeyboard = false;
        DrawKeyboard(1);                                            // the pop-up GUI keyboard should be drawn AFTER the pen down interrupt
    }
    if(DelayedDrawFmtBox) {
        DelayedDrawFmtBox = false;
        DrawFmtBox(1);                                              // the pop-up GUI keyboard should be drawn AFTER the pen down interrupt
    }
    if(SaveOptionErrorSkip>0)OptionErrorSkip=SaveOptionErrorSkip+1;
    strcpy( MMErrMsg, SaveMMErrMsg);   //restore saved error messages
    MMerrno=SaveMMerrno;              //restore saved MMerrno

}


// set up the tick interrupt
void cmd_settick(void){
    int period;
    int irq=0;;
    char s[STRINGSIZE];
    getargs(&cmdline, 5, ",");
    strcpy(s,argv[0]);
    if(!(argc == 3 || argc == 5)) error("Argument count");
    if(argc == 5) irq = getint(argv[4], 1, NBRSETTICKS) - 1;
    if(checkstring(argv[0],"PAUSE")){
        TickActive[irq]=0;
        return;
    } else if(checkstring(argv[0],"RESUME")){
        TickActive[irq]=1;
        return;
    } else period = getint(argv[0], -1, INT_MAX);
    if(period == 0) {
        TickInt[irq] = NULL;                                        // turn off the interrupt
    } else {
        TickPeriod[irq] = period;
        TickInt[irq] = GetIntAddress(argv[2]);                      // get a pointer to the interrupt routine
        TickTimer[irq] = 0;                                         // set the timer running
        InterruptUsed = true;
        TickActive[irq]=1;

    }
}


void cmd_option(void) {
	char *tp;
	int maxH=PageTable[WritePage].ymax;

	tp = checkstring(cmdline, "BASE");
	if(tp) {
		if(DimUsed) error("Must be before DIM or LOCAL");
		OptionBase = getint(tp, 0, 1);
		return;
	}

	tp = checkstring(cmdline, "ESCAPE OFF");
	    if(tp) {
	        OptionEscape = false;
	        return;
	}

	tp = checkstring(cmdline, "ESCAPE");
	    if(tp) {
	        OptionEscape = true;
	        return;
	}

	tp = checkstring(cmdline, "EXPLICIT");
	if(tp) {
//        if(varcnt != 0) error("Variables already defined");
		OptionExplicit = true;
		return;
	}

    tp = checkstring(cmdline, "DEFAULT");
	if(tp) {
		if(checkstring(tp, "INTEGER"))	{ DefaultType = T_INT; 	return; }
		if(checkstring(tp, "FLOAT"))	{ DefaultType = T_NBR; 	return; }
		if(checkstring(tp, "STRING"))	{ DefaultType = T_STR; 	return; }
		if(checkstring(tp, "NONE"))	    { DefaultType = T_NOTYPE; 	return; }
	}

	tp = checkstring(cmdline, "DISPLAY");
	if(tp){
	     setterminal();
	     return;
    }

    tp = checkstring(cmdline, "F11");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F11Key))error("Maximum 63 characters");
		else strcpy((char *)Option.F11Key, p);
		SaveOptions(1);
		//strcpy(FunKey[10],(char *)Option.F11Key);
		return;
	}
    tp = checkstring(cmdline, "F12");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F12Key))error("Maximum 63 characters");
		else strcpy((char *)Option.F12Key, p);
		SaveOptions(1);
		//strcpy(FunKey[11],(char *)Option.F12Key);
		return;
	}
    tp = checkstring(cmdline, "F15");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F15Key))error("Maximum 63 characters");
		else strcpy((char *)Option.F15Key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F16");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F16Key))error("Maximum 63 characters");
		else strcpy((char *)Option.F16Key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F19");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F19Key))error("Maximum 63 characters");
		else strcpy((char *)Option.F19Key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F20");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F20Key))error("Maximum 63 characters");
		else strcpy((char *)Option.F20Key, p);
		SaveOptions(1);
		return;
	}


    tp = checkstring(cmdline, "SEARCH PATH");
	if(tp) {
		char p[STRINGSIZE];
		if(checkstring(tp, "DISABLE"))	mymemset((char *)Option.path,0,sizeof(Option.path));
		else {
			strcpy(p,getCstring(tp));
			if(strlen(p)>=sizeof(Option.path))error("Maximum 127 characters");
			else {
				if(p[0]!='/')error("Path must start and end with /");
				if(p[strlen(p)-1]!='/')error("Path must start and end with /");
				strcpy((char *)Option.path, p);
			}
		}
		SaveOptions(1);
		return;
	}

	tp = checkstring(cmdline, "CRLF");
	if(tp) {
	   	if(CurrentLinePtr==NULL) error("Only valid in a program");
		if(checkstring(tp, "CRLF"))	{ sendCRLF = 3; 	return; }
		if(checkstring(tp, "CR"))	{ sendCRLF = 2; 	return; }
		if(checkstring(tp, "LF"))	{ sendCRLF = 1; 	return; }
	}

	tp = checkstring(cmdline, "BREAK");
	if(tp) {
		BreakKey = getinteger(tp);
		return;
	}
    tp = checkstring(cmdline, "MILLISECONDS");
	if(tp) {
		if(checkstring(tp, "ON"))		{ Option.fulltime = true; return; }
		if(checkstring(tp, "OFF"))		{ Option.fulltime = false; return;  }
	}

    tp = checkstring(cmdline, "LEGACY");
	if(tp) {
	   	if(CurrentLinePtr==NULL) error("Only valid in a program");
	   	if(VideoColour!=8)error("Display must be in 8-bit mode for legacy use");
		if(checkstring(tp, "ON"))		{
			FontTable[0]=(unsigned char *)CMMfontZero;
			FontTable[1]=(unsigned char *)CMMfontOne;
			FontTable[2]=(unsigned char *)CMMfontTwo;
			colourmap[0]=CLUT[0];
			colourmap[1]=CLUT[3];
			colourmap[2]=CLUT[28];
			colourmap[3]=CLUT[31];
			colourmap[4]=CLUT[224];
			colourmap[5]=CLUT[227];
			colourmap[6]=CLUT[252];
			colourmap[7]=CLUT[255];
			CMM1 = true;
			return;
		}
		if(checkstring(tp, "OFF"))		{
			FontTable[0]=(unsigned char *)font1;
			FontTable[1]=(unsigned char *)Misc_12x20_LE;
			FontTable[2]=(unsigned char *)Hom_16x24_LE;
			CMM1 = false;
			return;
		}
	}

	tp = checkstring(cmdline, "AUTORUN");
	if(tp) {
		if(checkstring(tp, "ON"))		{ Option.Autorun = true; SaveOptions(1); return; }
		if(checkstring(tp, "OFF"))		{ Option.Autorun = false; SaveOptions(1); return;  }
	}
	tp = checkstring(cmdline, "STATUS");
	if(tp) {
    	if(CurrentLinePtr) error("Invalid in a program");
		if(checkstring(tp, "ON")){
			if(Option.showstatus == true)return;
	    	ShowCursor(false);
			Option.showstatus = true;
			SaveOptions(1);
			cleanend();
			return;
		}
		if(checkstring(tp, "OFF"))		{
		    if((OptionConsole & 2) && !CurrentLinePtr) {                 // if we are at the command prompt on the LCD
		    	ShowCursor(false);
		    	int lastx=CurrentX,lasty=CurrentY;
		    	char buff[255];
		    	int i;
		    	for(i=0;i<Option.Width;i++)buff[i]=' ';
		    	GUIPrintString(0, maxH-gui_font_height-1, gui_font, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, BROWN, BLACK, buff);
		    	CurrentY=lasty;
		    	CurrentX=lastx;
		    }
			Option.showstatus = false;
			SaveOptions(1);
			return;
		}
		error("Syntax");
	}

	tp = checkstring(cmdline, "CASE");
	if(tp) {
		if(checkstring(tp, "LOWER"))	{ Option.Listcase = CONFIG_LOWER; SaveOptions(1); return; }
		if(checkstring(tp, "UPPER"))	{ Option.Listcase = CONFIG_UPPER; SaveOptions(1); return; }
		if(checkstring(tp, "TITLE"))	{ Option.Listcase = CONFIG_TITLE; SaveOptions(1); return; }
	}
	tp = checkstring(cmdline, "ANGLE");
	if(tp) {
		if(checkstring(tp, "DEGREES"))	{ optionangle=RADCONV; return; }
		if(checkstring(tp, "RADIANS"))	{ optionangle=1.0; return; }
	}
	tp = checkstring(cmdline, "Y_AXIS");
	if(tp) {
	   	if(CurrentLinePtr==NULL) error("Only valid in a program");
		if(checkstring(tp, "UP"))	{ optiony=1; return; }
		if(checkstring(tp, "DOWN"))	{ optiony=0; return; }
	}


    tp = checkstring(cmdline, "TAB");
	if(tp) {
		if(checkstring(tp, "2"))		{ Option.Tab = 2; SaveOptions(1); return; }
		if(checkstring(tp, "3"))		{ Option.Tab = 3; SaveOptions(1); return; }
		if(checkstring(tp, "4"))		{ Option.Tab = 4; SaveOptions(1); return; }
		if(checkstring(tp, "8"))		{ Option.Tab = 8; SaveOptions(1); return; }
	}
    tp = checkstring(cmdline, "BAUDRATE");
	if(tp) {
    	if(CurrentLinePtr) error("Invalid in a program");
    	ShortScroll=Option.showstatus;
        int i;
		i = getinteger(tp);
        if(i > PeripheralBusSpeed/17) error("Baud rate too high");
        if(i < 100) error("Number out of bounds");
        Option.Baudrate = i;
        SaveOptions(1);
        MMPrintString("Restart to activate");                // set the console baud rate
		return;
	}
    tp = checkstring(cmdline, "VCC");
	if(tp) {
        MMFLOAT f;
		f = getnumber(tp);
        if(f > 3.6) error("VCC too high");
        if(f < 1.8) error("VCC too low");
        VCC=f;
		return;
	}

	tp = checkstring(cmdline, "PIN");
	if(tp) {
    	int i;
		i = getint(tp, 0, 99999999);
        Option.PIN = i;
        SaveOptions(1);
		return;
	}


    tp = checkstring(cmdline, "COLOURCODE");
    if(tp == NULL) tp = checkstring(cmdline, "COLORCODE");
	if(tp) {
		if(checkstring(tp, "ON"))		{ Option.colourmode = 1; SaveOptions(1); return; }
		if(checkstring(tp, "OFF"))		{ Option.colourmode = 0; SaveOptions(1); return;  }
		if(checkstring(tp, "REVERSE"))		{ Option.colourmode = -1; SaveOptions(1); return;  }
	}

	OtherOptions();
}
void cmd_JumpToBootloader(void){
    _excep_code = RESTART_BOOT0;                            // otherwise do an automatic reset
	while(ConsoleTxBufTail != ConsoleTxBufHead);
	MM_Delay(10);
    SoftReset();                                                // this will restart the processor

}

void JumpToBootloader(void)
{
  uint32_t i=0;
  void (*SysMemBootJump)(void);

  /* Set the address of the entry point to bootloader */
     volatile uint32_t BootAddr = 0x1FF09800;
  /* Disable all interrupts */
     __disable_irq();

  /* Disable Systick timer */
     SysTick->CTRL = 0;

  /* Set the clock to the default state */
     HAL_RCC_DeInit();

  /* Clear Interrupt Enable Register & Interrupt Pending Register */
     for (i=0;i<5;i++)
     {
	  NVIC->ICER[i]=0xFFFFFFFF;
	  NVIC->ICPR[i]=0xFFFFFFFF;
     }
  /* Re-enable all interrupts */
     __enable_irq();

  /* Set up the jump to booloader address + 4 */
     SysMemBootJump = (void (*)(void)) (*((uint32_t *) ((BootAddr + 4))));

  /* Set the main stack pointer to the bootloader stack */
     __set_MSP(*(uint32_t *)BootAddr);

  /* Call the function to jump to bootloader location */
     SysMemBootJump();
  /* Jump is done successfully */
     while (1)
     {
      /* Code should never reach this loop */
     }
}

#define STM32_UUID       ((uint8_t*)0x1FF1E800)
// board SN 48 bit
static inline uint64_t mix(uint64_t h)
{
    h ^= h >> 23;
    h *= 0x2127599bf4325c37ULL;
    h ^= h >> 47;
    //
    return h;
}

uint64_t fastHash64(const void * buf, size_t len, uint64_t seed)
{
    const uint64_t m = 0x880355f21e6d1965ULL;   //Always as unsigned long long
    const uint64_t * pos = (const uint64_t*)buf;
    const uint64_t * end = pos + (len / 8);
    const unsigned char * pos2;
    uint64_t h = seed ^ (len * m);
    uint64_t v;

    while(pos != end)
    {
        v  = *pos++;
        h ^= mix(v);
        h *= m;
    }

    pos2 = (const unsigned char*)pos;
    v = 0;

    switch(len & 7)
    {
        case 7: v ^= (uint64_t)pos2[6] << 48;
        case 6: v ^= (uint64_t)pos2[5] << 40;
        case 5: v ^= (uint64_t)pos2[4] << 32;
        case 4: v ^= (uint64_t)pos2[3] << 24;
        case 3: v ^= (uint64_t)pos2[2] << 16;
        case 2: v ^= (uint64_t)pos2[1] << 8;
        case 1: v ^= (uint64_t)pos2[0];
                h ^= mix(v);
                h *= m;
    }

    return mix(h);
}

int64_t getBoardSerial(void)
{
  uint64_t hash = fastHash64(STM32_UUID, 12, 1234554321) & 0xFFFFFFFFFFFF;
  return hash;
}


// function (which looks like a pre defined variable) to return the type of platform
void fun_device(void){
	sret = GetTempStrMemory();									// this will last for the life of the command
    strcpy(sret, (G1Hardware ? "Colour Maximite 2" :"Colour Maximite 2 G2"));
    CtoM(sret);
    targ = T_STR;
}
#define RoundUptoPage(a)     ((((uint64_t)a) + (uint64_t)(128*1024 - 1)) & (uint64_t)(~(128*1024 - 1)))// round up to the nearest whole integer

void fun_info(void){
	char *tp;
	sret = GetTempStrMemory();									// this will last for the life of the command
	char rettype='S';
	tp=checkstring(ep, "PIN");
	if(tp){
		int pin;
		pin = getint(tp, 1, NBRPINS);
		if(IsInvalidPin(pin))strcpy(sret,"Invalid");
		else if(ExtCurrentConfig[pin] & CP_IGNORE_RESERVED)strcpy(sret,"Reserved");
		else if(ExtCurrentConfig[pin]) strcpy(sret,"In Use");
		else strcpy(sret,"Unused");
		CtoM(sret);
		targ=T_STR;
		return;
	}
	tp=checkstring(ep, "PAGE ADDRESS");
	if(tp){
		iret = (int64_t)((uint32_t)GetPageAddress(getint(tp, 0, LastPage)));
		targ=T_INT;
		return;
	}
    tp=checkstring(ep, "FONT ADDRESS");
		if(tp){
		iret=(int64_t)((uint32_t)FontTable[getint(tp,1,FONT_TABLE_SIZE)-1]);
		targ=T_INT;
		return;
	}
	tp=checkstring(ep, "FONT POINTER");
		if(tp){
		iret=(int64_t)((uint32_t)&FontTable[getint(tp,1,FONT_TABLE_SIZE)-1]);
		targ=T_INT;
		return;
	}
	tp=checkstring(ep, "FRAMEBUFFER");
	if(tp){
		iret = (int64_t)((uint32_t)PageTable[WPN].address);
		targ=T_INT;
		return;
	}
	tp=checkstring(ep, "SAMPLE PLAYING");
	if(tp){
		iret = (int64_t)((uint32_t)hxcmod_effectplaying(&mcontext,getint(tp, 1, NUMMAXSEFFECTS)-1));
		targ=T_INT;
		return;
	}
	tp=checkstring(ep, "UPTIME");
	if(tp){
        fret = (MMFLOAT)Getuptime()/1000000.0;
        targ = T_NBR;
		return;
	}
	tp=checkstring(ep, "FILESIZE");
	if(tp){
		int i,j;
		char pp[FF_MAX_LFN] = {0};
		char q[FF_MAX_LFN]={0};
		static DIR djd;
		static FILINFO fnod;
		mymemset(&djd,0,sizeof(DIR));
		mymemset(&fnod,0,sizeof(FILINFO));
		char *p = getFstring(tp);
		if(p[strlen(p)-1]=='\\' || p[strlen(p)-1]=='/' || p[strlen(p)-1]==':')error("Invalid file specification");
		i=strlen(p)-1;
		while(i>0 && !(p[i] == 92 || p[i]==47))i--;
		if(i>0){
			memcpy(q,p,i);
			 for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
			if(q[1]==':')q[0]='0';
			i++;
		}
		strcpy(pp,&p[i]);
		if((pp[0]==47 || pp[0]==92) && i==0){
			strcpy(q,&pp[1]);
			strcpy(pp,q);
			strcpy(q,"0:/");
		}
		if(pp[0]==0)strcpy(pp,"*");
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		fullpath(q);
		strcpy(q,fullpathname);
		if(q[strlen(q)-1]!=47)strcat(q,"/");
		strcat(q,pp);
		ErrorCheck(0);
//		if(strcmp(q,"/*")==0 || strcmp(q,"/A:")==0 || strcmp(q,"/a:")==0){ iret=-2; targ=T_INT; MMErrMsg = (char *)FErrorMsg[4]; return;}
		FSerror = f_stat(q, &fnod);
		if(FSerror != FR_OK){ iret=-1; targ=T_INT; MMErrMsg = (char *)FErrorMsg[4]; return;}
		if((fnod.fattrib & AM_DIR)){ iret=-2; targ=T_INT; MMErrMsg = (char *)FErrorMsg[4]; return;}
		iret=fnod.fsize;
		targ=T_INT;
		return;
	}
	tp=checkstring(ep, "EXISTS DIR");
	if(tp){
		int i;
	    char *p;
		targ=T_INT;
	    DIR dir;
	    char rp[STRINGSIZE],oldfilepath[STRINGSIZE];
	    p = strupr(getFstring(tp));  // get the directory name and convert to a standard C string
	    for(i=0;i<strlen(p);i++)if(p[i]=='\\')p[i]='/';  //allow backslash for the DOS oldies
	    if(strcmp(p,".")==0){iret=1;return;} //nothing to do
	    if(strlen(p)==0){iret=1;return;};//nothing to do
	    strcpy(oldfilepath,filepath); //save the path in case the change of directory fails
	    if(p[1]==':'){ //modify the requested path so that if the disk is specified the pathname is absolute and starts with /
	    	if(p[2]=='/')p+=2;
	    	else {
	    		p[1]='/';
	    		p++;
	    	}
	    }
	    if (*p=='/'){ //absolute path specified
	    	strcpy(rp,"A:");
	    	strcat(rp,p);
	    } else { // relative path specified
	    	strcpy(rp,filepath); //copy the current pathname
	        if(rp[strlen(rp)-1]!='/')  strcat(rp,"/"); //make sure the previous pathname ends in slash, will only be the case at root
	    	strcat(rp,p); //append the new pathname
	    }
		strcpy(filepath,rp); //set the new pathname
		resolve_path(filepath,rp,rp); //resolve to single absolute path
		if(strcmp(rp,"A:")==0)strcat(rp,"/"); //if root append the slash
		strcpy(filepath,rp); //store this back to the filepath variable
	    if(!InitSDCard()) { //If no disk restore the old path and return
	    	strcpy(filepath,oldfilepath);
	    	return;
	    }
		FSerror = f_opendir(&dir, &filepath[2]); //finally change directory always using an absolute pathname
		if(FSerror != FR_OK)iret=0;
		else iret=1;
		targ=T_INT;
		strcpy(filepath,oldfilepath); //if it didn't work restore the original path
//		ErrorCheck(0); // error if the pathname was invalid
		return;
	}
	tp=checkstring(ep, "EXISTS FILE");
	if(tp){
		int i,j;
		char pp[FF_MAX_LFN] = {0};
		char q[FF_MAX_LFN]={0};
		static DIR djd;
		static FILINFO fnod;
		mymemset(&djd,0,sizeof(DIR));
		mymemset(&fnod,0,sizeof(FILINFO));
		char *p = getFstring(tp);
//		if(p[strlen(p)-1]=='\\' || p[strlen(p)-1]=='/' || p[strlen(p)-1]==':')error("Invalid file specification");
		i=strlen(p)-1;
		while(i>0 && !(p[i] == 92 || p[i]==47))i--;
		if(i>0){
			memcpy(q,p,i);
			 for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
			if(q[1]==':')q[0]='0';
			i++;
		}
		strcpy(pp,&p[i]);
		if((pp[0]==47 || pp[0]==92) && i==0){
			strcpy(q,&pp[1]);
			strcpy(pp,q);
			strcpy(q,"0:/");
		}
		if(pp[0]==0)strcpy(pp,"*");
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		fullpath(q);
		strcpy(q,fullpathname);
		if(q[strlen(q)-1]!=47)strcat(q,"/");
		strcat(q,pp);
		ErrorCheck(0);
//		if(strcmp(q,"/*")==0 || strcmp(q,"/A:")==0 || strcmp(q,"/a:")==0){ iret=-2; targ=T_INT; MMErrMsg = (char *)FErrorMsg[4]; return;}
		FSerror = f_stat(q, &fnod);
		if(FSerror != FR_OK){ iret=0; targ=T_INT; MMErrMsg = (char *)FErrorMsg[4]; return;}
		if((fnod.fattrib & AM_DIR)){ iret=0; targ=T_INT; MMErrMsg = (char *)FErrorMsg[4]; return;}
		iret=1;
		targ=T_INT;
		return;
	}
	tp=checkstring(ep,"DIRECTORY");
	if(tp){
		strcpy(sret,filepath);
		if(sret[strlen(sret)-1]!='/')strcat(sret,"/");
		CtoM(sret);
	    targ=T_STR;
		return;
	}

	tp=checkstring(ep, "MODIFIED");
	if(tp){
		int i,j;
	    char pp[FF_MAX_LFN] = {0};
	    char q[FF_MAX_LFN]={0};
	    static DIR djd;
	    static FILINFO fnod;
		mymemset(&djd,0,sizeof(DIR));
		mymemset(&fnod,0,sizeof(FILINFO));
		char *p = getFstring(tp);
		if(p[strlen(p)-1]=='\\' || p[strlen(p)-1]=='/')p[strlen(p)-1]=0;
		i=strlen(p)-1;
		while(i>0 && !(p[i] == 92 || p[i]==47))i--;
		if(i>0){
			memcpy(q,p,i);
			 for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
			if(q[1]==':')q[0]='0';
			i++;
		}
		strcpy(pp,&p[i]);
		if((pp[0]==47 || pp[0]==92) && i==0){
			strcpy(q,&pp[1]);
			strcpy(pp,q);
			strcpy(q,"0:/");
		}
		if(pp[0]==0)strcpy(pp,"*");
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		fullpath(q);
		strcpy(q,fullpathname);
		if(q[strlen(q)-1]!=47)strcat(q,"/");
		strcat(q,pp);
		ErrorCheck(0);
		FSerror = f_stat(q, &fnod);
		if(FSerror != FR_OK){ iret=-1; targ=T_STR; MMErrMsg = (char *)FErrorMsg[4]; return;}
//		if((fnod.fattrib & AM_DIR)){ iret=-2; targ=T_INT; MMErrMsg = (char *)FErrorMsg[4]; return;}
	    IntToStr(sret , ((fnod.fdate>>9)&0x7F)+1980, 10);
	    sret[4] = '-'; IntToStrPad(sret + 5, (fnod.fdate>>5)&0xF, '0', 2, 10);
	    sret[7] = '-'; IntToStrPad(sret + 8, fnod.fdate&0x1F, '0', 2, 10);
	    sret[10] = ' ';
	    IntToStrPad(sret+11, (fnod.ftime>>11)&0x1F, '0', 2, 10);
	    sret[13] = ':'; IntToStrPad(sret + 14, (fnod.ftime>>5)&0x3F, '0', 2, 10);
	    sret[16] = ':'; IntToStrPad(sret + 17, (fnod.ftime&0x1F)*2, '0', 2, 10);
		CtoM(sret);
	    targ=T_STR;
		return;
	}
	tp=checkstring(ep, "OPTION");
	if(tp){
		if(checkstring(tp, "ANGLE")){
			if(optionangle==1.0)strcpy(sret,"RADIANS");
			else strcpy(sret,"DEGREES");
		} else if(checkstring(tp, "Y_AXIS")){
			if(optiony==1)strcpy(sret,"UP");
			else strcpy(sret,"DOWN");
		} else if(checkstring(tp, "AUTORUN")){
			if(Option.Autorun == false)strcpy(sret,"Off");
			else strcpy(sret,"On");
		} else if(checkstring(tp, "CONSOLE")){
			if(OptionConsole == 3)strcpy(sret,"Both");
			else if(OptionConsole == 2)strcpy(sret,"Screen");
			else strcpy(sret,"Serial");
		} else if(checkstring(tp, "EXPLICIT")){
			if(OptionExplicit == false)strcpy(sret,"Off");
			else strcpy(sret,"On");
		} else if(checkstring(tp, "LEGACY")){
			if(CMM1)strcpy(sret,"On");
			else strcpy(sret,"Off");
		} else if(checkstring(tp, "PROFILING")){
			if(Option.profile)strcpy(sret,"On");
			else strcpy(sret,"Off");
		} else if(checkstring(tp, "USBKEYBOARD")){
			strcpy(sret,(char *)KBrdList[(int)Option.USBKeyboard]);
		} else if(checkstring(tp, "DEFAULT")){
			if(DefaultType == T_INT)strcpy(sret,"Integer");
			else if(DefaultType == T_NBR)strcpy(sret,"Float");
			else if(DefaultType == T_STR)strcpy(sret,"String");
			else strcpy(sret,"None");
		} else if(checkstring(tp, "BASE")){
			if(OptionBase==1)iret=1;
			else iret=0;
			targ=T_INT;
			return;
		} else if(checkstring(tp, "MOUSE")){
			iret=Option.Mouse;
			targ=T_INT;
			return;
		} else if(checkstring(tp, "CONSOLE PORT")){
			iret=Option.ConsolePort;
			targ=T_INT;
			return;
		} else if(checkstring(tp, "BREAK")){
			iret=BreakKey;
			targ=T_INT;
			return;
		} else error("Syntax");
		CtoM(sret);
	    targ=T_STR;
		return;
	}
	if(checkstring(ep, "CPUSPEED")){
		iret=SystemCoreClock;
		rettype='I';
	} else if(checkstring(ep, "MODE")){
		fret = VideoMode+(VideoColour==8? 0.8: (VideoColour==12? 0.12: (VideoColour==16 ? 0.16 : 0.32)));
	    rettype='N';
	} else if(checkstring(ep, "DEVICE")){
	    fun_device();
	    return;
	} else if(checkstring(ep, "UPTIME")){
		fret=(MMFLOAT)(ReadCoreTimer() + FastTimer)/(MMFLOAT)ticks_per_microsecond/1000000.0;
		rettype='N';
	} else if(checkstring(ep, "SEARCH PATH")){
	    strcpy(sret, (char *)Option.path);
	} else if(checkstring(ep, "VERSION")){
		//char *p;
	    //fret = (MMFLOAT)strtol(VERSION, &p, 10);
	    //fret += (MMFLOAT)strtol(p + 1, &p, 10) / (MMFLOAT)100.0;
	    //fret += (MMFLOAT)strtol(p + 1, &p, 10) / (MMFLOAT)10000.0;
		fun_version();
		rettype='N';
	} else if(checkstring(ep, "DISK SIZE")){
	    if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
	    FATFS *fs;
	    DWORD fre_clust;
	    /* Get volume information and free clusters of drive 1 */
	    f_getfree("0:", &fre_clust, &fs);
	    /* Get total sectors and free sectors */
	    iret= (uint64_t)(fs->n_fatent - 2) * (uint64_t)fs->csize *(uint64_t)FF_MAX_SS;
		rettype='I';
	} else if(checkstring(ep, "FREE SPACE")){
	    if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
	    FATFS *fs;
	    DWORD fre_clust;
	    /* Get volume information and free clusters of drive 1 */
	    f_getfree("0:", &fre_clust, &fs);
	    /* Get total sectors and free sectors */
	     iret = (uint64_t)fre_clust * (uint64_t)fs->csize  *(uint64_t)FF_MAX_SS;
		rettype='I';
	} else if(checkstring(ep, "FONTWIDTH")){
	    iret = FontTable[gui_font >> 4][0] * (gui_font & 0b1111);
		rettype='I';
	} else if(checkstring(ep, "FONTHEIGHT")){
	    iret = FontTable[gui_font >> 4][1] * (gui_font & 0b1111);
		rettype='I';
	} else if(checkstring(ep, "HPOS")){
	    iret = CurrentX;
		rettype='I';
	} else if(checkstring(ep, "VPOS")){
	    iret = CurrentY;
		rettype='I';
	} else if(checkstring(ep, "OVERRUN")){
	    iret = overrun;
		rettype='I';
	} else if(checkstring(ep, "FRAMEH")){
	    iret = PageTable[WPN].xmax;
		rettype='I';
	} else if(checkstring(ep, "ONEWIRE")){
		iret = mmOWvalue;
		rettype='I';
	} else if(checkstring(ep, "I2C")){
	    iret = mmI2Cvalue;
		rettype='I';
	} else if(checkstring(ep, "FCOLOUR") || checkstring(ep, "FCOLOR") ){
		iret=gui_fcolour;
		rettype='I';
	} else if(checkstring(ep, "BCOLOUR") || checkstring(ep, "BCOLOR")){
		iret=gui_bcolour;
		rettype='I';
	} else if(checkstring(ep, "FONT")){
		iret=(gui_font >> 4)+1;
		rettype='I';
	} else if(checkstring(ep, "RTC")){
	    iret = Option.RTCinstalled;
		rettype='I';
	} else if(checkstring(ep, "LINES")){
	    iret = LineCount;
		rettype='I';

	//}else if (checkstring(ep, "LINE")) {
		    //if (!CurrentLinePtr) {
		    //    strcpy(sret, "UNKNOWN");
		    //} else  {
		    //} else if(CurrentLinePtr < ProgMemory + Option.ProgFlashSize) {
		    //	sprintf(sret, "%d", CountLines(CurrentLinePtr));
		    //}// else {
		    //	strcpy(sret, "LIBRARY");
		  //  }
		  //  CtoM(sret);
		   // targ=T_STR;
		  //  return;
		  //  rettype='S';

	} else if(checkstring(ep, "HARDWARE")){
	    iret = G1Hardware;
		rettype='I';
	} else if(checkstring(ep, "ERRNO")){
	    iret = MMerrno;
		rettype='I';
	} else if(checkstring(ep, "ERRMSG")){
		if(MMErrMsg[0]==13)strcpy(sret, &MMErrMsg[2]);
		else strcpy(sret, MMErrMsg);
	} else if(checkstring(ep, "FRAMEV")){
	    iret = PageTable[WPN].ymax;
		rettype='I';
	} else if(checkstring(ep, "STACK")){
		iret=(int64_t)((uint32_t)__get_MSP());
		rettype='I';
	} else if(checkstring(ep, "WRITE PAGE")){
		iret=(int64_t)((uint32_t)PageTable[WritePage].address);
		rettype='I';
	} else if(checkstring(ep, "PROGRAM")){
		iret=(int64_t)((uint32_t)ProgMemory);
		rettype='I';
	} else if(checkstring(ep, "MAX PAGES")){
		iret= LastPage;
		rettype='I';
	} else if(checkstring(ep, "CURRENT")){
		tp=(char *)ProgMemory;
		if(*tp++ == 1 && *tp++ == 39){
			strcpy(sret,"A:/");
			strcat(sret,&tp[1]);
		} else strcpy(sret,"NONE");
	} else if(checkstring(ep, "PATH")){
		tp=(char *)ProgMemory;
		if(*tp++ == 1 && *tp++ == 39){
			strcpy(sret,"A:/");
			strcat(sret,&tp[1]);
			int linelen = strlen(sret);
		    while (linelen > 0 && (sret[linelen - 1] !='/')) linelen--;
		    sret[linelen]=0;
		} else strcpy(sret,"NONE");
	} else if(checkstring(ep, "CODE SIZE")){
		char *p=(char *)ProgMemory;
		int i=0;
		int onezero=0;
		while(1){
			if(*p == 0)onezero++;
			else onezero=0;
			if(onezero==2)break;
			p++;
			i++;
		}
		iret=i;
		rettype='I';
	} else if(checkstring(ep, "VARCNT")){
		iret=(int64_t)((uint32_t)varcnt);
		rettype='I';
	} else if(checkstring(ep, "MINHEAP")){
		iret=(int64_t)((uint32_t)minMMheap);
		rettype='I';
	} else if(checkstring(ep, "HEAP")){
	    int nbr = 0;
	    unsigned char *addr;
	    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBASE; addr -= RAMPAGESIZE){
	        if(!(MBitsGet(addr) & PUSED)) nbr++;
	    }
	    for(addr = (unsigned char *)SDRAMEND -  RAMPAGESIZE; addr > (unsigned char *)SDRAMBASE; addr -= RAMPAGESIZE){
	        if(!(SDBitsGet(addr) & PUSED)) nbr++;
	    }
		iret=(int64_t)nbr * RAMPAGESIZE;
		rettype='I';
	} else if(checkstring(ep, "HEAPI")){
	    int nbr = 0;
	    unsigned char *addr;
	    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBASE; addr -= RAMPAGESIZE){
	        if(!(MBitsGet(addr) & PUSED)) nbr++;
	    }
	   	iret=(int64_t)nbr * RAMPAGESIZE;
		rettype='I';
	} else if(checkstring(ep, "HEAPS")){
	    int nbr = 0;
	    unsigned char *addr;
	    for(addr = (unsigned char *)SDRAMEND -  RAMPAGESIZE; addr > (unsigned char *)SDRAMBASE; addr -= RAMPAGESIZE){
	        if(!(SDBitsGet(addr) & PUSED)) nbr++;
	    }
		iret=(int64_t)nbr * RAMPAGESIZE;
		rettype='I';
	} else if(checkstring(ep, "RESET")){
		strcpy(sret,bootcause);
		mymemset(bootcause,0,sizeof(bootcause));
	} else if(checkstring(ep, "TRACK")){
		if(CurrentlyPlaying == P_MP3 || CurrentlyPlaying == P_FLAC || CurrentlyPlaying == P_WAV) strcpy(sret,alist[trackplaying].fn);
		else strcpy(sret,"OFF");
	} else if(checkstring(ep, "SOUND")){
		switch(CurrentlyPlaying){
		case P_NOTHING:strcpy(sret,"OFF");break;
		case P_PAUSE_TONE:
		case P_PAUSE_MP3:
		case P_PAUSE_WAV:
		case P_PAUSE_MOD:
		case P_PAUSE_SOUND:
		case P_PAUSE_FLAC:strcpy(sret,"PAUSED");break;
		case P_TONE:strcpy(sret,"TONE");break;
		case P_WAV:strcpy(sret,"WAV");break;
		case P_MP3:strcpy(sret,"MP3");break;
		case P_MOD:strcpy(sret,"MODFILE");break;
		case P_TTS:strcpy(sret,"TTS");break;
		case P_FLAC:strcpy(sret,"FLAC");break;
		case P_DAC:strcpy(sret,"DAC");break;
		case P_SOUND:strcpy(sret,"SOUND");break;
		}
	} else if(checkstring(ep, "KEYBOARD")){
		strcpy(sret,(keyboardseen? "Connected":"Not Connected"));

	} else if(checkstring(ep, "ID")){  //Unique ID 12 bytes
     	int i;
       char id_out[25];
       // Generate hex one nibble at a time
       for (i = 0; i<24; i++) {
          int nibble = (TM_ID_GetUnique8(i/2) >> (4 - 4*(i%2)) ) & 0xf;
          id_out[i] = (char)(nibble < 10 ? nibble + '0' : nibble + 'A' - 10);
       }
       id_out[i] = 0;
       strcpy(sret,id_out);

    } else if(checkstring(ep, "ID48")){  //Unique ID  48 bits
     	iret=(int64_t)getBoardSerial();
		targ=T_INT;
		return;

	} else if(checkstring(ep, "SDCARD")){
		if(!MDD_SDSPI_CardDetectState()) {
			int i=OptionFileErrorAbort;
			OptionFileErrorAbort=0;
			if(!InitSDCard())strcpy(sret,"Not present");
			else  strcpy(sret,"Ready");
			OptionFileErrorAbort=i;
		}
		else if((SDCardStat & (STA_NODISK | STA_NOINIT))==(STA_NODISK | STA_NOINIT)) strcpy(sret,"Not present");
		else if(!(SDCardStat & STA_NOINIT)) strcpy(sret,"Ready");
		else strcpy(sret,"Unused");
	} else error("Syntax");

    targ=T_INT;
	if(rettype=='S'){
		CtoM(sret);
		targ=T_STR;
	}
	if(rettype=='N')targ=T_NBR;
}

void cmd_watchdog(void) {
    int i;

    if(checkstring(cmdline, "OFF") != NULL) {
        WDTimer = 0;
    } else {
        i = getinteger(cmdline);
        if(i < 1) error("Invalid argument");
        WDTimer = i;
    }
}


void fun_restart(void) {
    iret = WatchdogSet;
    targ = T_INT;
}

void cmd_cpu(void) {
    char *p;

//    while(!UARTTransmissionHasCompleted(UART1));                    // wait for the console UART to send whatever is in its buffer

    if((p = checkstring(cmdline, "RESTART"))) {
    	MMPrintString("\r\n");
        _excep_code = RESET_COMMAND;
    	while(ConsoleTxBufTail != ConsoleTxBufHead);
    	MM_Delay(10);
        SoftReset();                                                // this will restart the processor ? only works when not in debug
    } else error("Syntax");
}

void cmd_csubinterrupt(void){
    getargs(&cmdline,1,",");
    if(argc != 0){
        if(checkstring(argv[0],"0")){
            CSubInterrupt = NULL;
            CSubComplete=0;
        } else {
            CSubInterrupt = GetIntAddress(argv[0]);
            CSubComplete=0;
            InterruptUsed = true;
        }
    } else CSubComplete=1;
}


void cmd_cfunction(void) {
    char *p;
    CommandToken tkn;
    CommandToken EndToken;


    EndToken = GetCommandValue("End DefineFont");           // this terminates a DefineFont
    if(cmdtoken == cmdCSUB) EndToken = GetCommandValue("End CSub");                 // this terminates a CSUB
    if(cmdtoken == cmdCFUN) EndToken = GetCommandValue("End CFunction");                 // this terminates a CSUB
    p = cmdline;
    while(*p != 0xff) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) error("Missing END declaration");               // end of the program
        if(*p == T_NEWLINE) p++;                                    // skip over the newline token
        if(*p == T_LINENBR) p += 3;                                 // skip over the line number
        skipspace(p);
        if(*p == T_LABEL) {
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        tkn = commandtbl_decode(p);
        if (tkn == EndToken){
           //if(*p == EndToken) {                                        // found an END token
            nextstmt = p;
            skipelement(nextstmt);
            return;
        }
        p++;
    }
}




// utility function used by cmd_poke() to validate an address
unsigned int GetPokeAddr(char *p) {
    unsigned int i;
    i = getinteger(p);
    if(!POKERANGE(i)) error("Address");
    return i;
}



void cmd_poke(void) {
    char *p;
    void *pp;

	getargs(&cmdline, 5, ",");
    if((p = checkstring(argv[0], "BYTE"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	uint8_t *padd=(uint8_t *)(a);
        *padd = getinteger(argv[2]);
        padd = (uint8_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if((p = checkstring(argv[0], "SHORT"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 2)error("Address not divisible by 2");
    	uint16_t *padd=(uint16_t *)(a);
        *padd = getinteger(argv[2]);
        padd = (uint16_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if((p = checkstring(argv[0], "WORD"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 4)error("Address not divisible by 4");
    	uint32_t *padd=(uint32_t *)(a);
        *padd = getinteger(argv[2]);
        padd = (uint32_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if((p = checkstring(argv[0], "INTEGER"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 8)error("Address not divisible by 8");
    	uint64_t *padd=(uint64_t *)(a);
    	*padd = getinteger(argv[2]);
        padd = (uint64_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }
    if((p = checkstring(argv[0], "FLOAT"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 8)error("Address not divisible by 8");
    	MMFLOAT *padd=(MMFLOAT *)(a);
    	*padd = getnumber(argv[2]);
        padd = (MMFLOAT *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if(argc != 5) error("Argument count");

    if(checkstring(argv[0], "VARTBL")) {
        *((char *)vartbl + (unsigned int)getinteger(argv[2])) = getinteger(argv[4]);
        return;
    }
    if((p = checkstring(argv[0], "VAR"))) {
		pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    	*((char *)pp + (unsigned int)getinteger(argv[2])) = getinteger(argv[4]);
        return;
    }
    // the default is the old syntax of:   POKE hiaddr, loaddr, byte
    *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2])) = getinteger(argv[4]);
}



// function to find a CFunction
// only used by fun_peek() below
unsigned int GetCFunAddr(int *ip, int i) {
    while(*ip != 0xffffffff) {
        if(*ip++ == (unsigned int)subfun[i]) {                      // if we have a match
            ip++;                                                   // step over the size word
            i = *ip++;                                              // get the offset
            return (unsigned int)(ip + i);                          // return the entry point
        }
        ip += (*ip + 4) / sizeof(unsigned int);
    }
    return 0;
}




// utility function used by fun_peek() to validate an address
unsigned int GetPeekAddr(char *p) {
    unsigned int i;
    i = getinteger(p);
    if(!PEEKRANGE(i)) error("Address");
    return i;
}


// Will return a byte within the PIC32 virtual memory space.
void fun_peek(void) {
    char *p;
    void *pp;
	getargs(&ep, 3, ",");
    if((p = checkstring(argv[0], "BYTE"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned char *)GetPeekAddr(p);
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "VARADDR"))){
        if(argc != 1) error("Syntax");
        pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = (unsigned int)pp;
        targ = T_INT;
        return;
        }
    if ((p = checkstring(argv[0],"BP"))) {
        if (argc != 1)
            SyntaxError();
        ;
        findvar(p, V_FIND | V_NOFIND_ERR);
        if (!(vartbl[VarIndex].type & T_INT))
            error("Not integer variable");
        iret = *(unsigned char *)(uint32_t)vartbl[VarIndex].val.i;
        vartbl[VarIndex].val.i++;
        targ = T_INT;
        return;
    }
    if ((p = checkstring(argv[0], "WP"))) {
        if (argc != 1)
            SyntaxError();
        ;
        findvar(p, V_FIND | V_NOFIND_ERR);
        if (!(vartbl[VarIndex].type & T_INT))
            error("Not integer variable");
        if (vartbl[VarIndex].val.i & 3)
            error("Not on word boundary");
        iret = *(unsigned int *)(uint32_t)vartbl[VarIndex].val.i;
        vartbl[VarIndex].val.i += 4;
        targ = T_INT;
        return;
    }
    if ((p = checkstring(argv[0], "SP"))) {
        if (argc != 1)
            SyntaxError();
        ;
        findvar(p, V_FIND | V_NOFIND_ERR);
        if (!(vartbl[VarIndex].type & T_INT))
            error("Not integer variable");
        if (vartbl[VarIndex].val.i & 1)
            error("Not on short boundary");
        iret = *(unsigned short *)(uint32_t)vartbl[VarIndex].val.i;
        vartbl[VarIndex].val.i += 2;
        targ = T_INT;
        return;
    }

    if((p = checkstring(argv[0], "VARHEADER"))){
        if(argc != 1) error("Syntax");
        pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = (unsigned int)&vartbl[VarIndex];
        targ = T_INT;
        return;
        }
/*
    if((p = checkstring(argv[0], "CFUNADDR"))){
    	int i,j=0;

        if(argc != 1) error("Syntax");
        i = FindSubFun(p, true);                                    // search for a function first
        if(i == -1) i = FindSubFun(p, false);                       // and if not found try for a subroutine
        //if(i == -1 || !(*subfun[i] == cmdCSUB)) error("Invalid argument");
        if (i == -1 || !(commandtbl_decode(subfun[i]) == cmdCSUB))
        // search through program flash and the library looking for a match to the function being called
        j = GetCFunAddr((int *)CFunctionFlash, i);
        if(!j) error("Internal fault 6(sorry)");
        iret = (unsigned int)j;                                     // return the entry point
        targ = T_INT;
        return;
    }
*/
    if((p = checkstring(argv[0], "CFUNADDR"))){
            if(argc != 1) error("Syntax");
            int i = FindSubFun(p, true);                                    // search for a function first
            if(i == -1) i = FindSubFun(p, false);                       // and if not found try for a subroutine
            //if(i == -1 || !(*subfun[i] == cmdCSUB)) error("Invalid argument");
           // if (i == -1 || !(commandtbl_decode(subfun[i]) == cmdCSUB)) error("Invalid argument");
            if(i == -1 || !(commandtbl_decode(subfun[i]) == cmdCFUN || commandtbl_decode(subfun[i]) == cmdCSUB)) error("Invalid argument");
            // search through program flash and the library looking for a match to the function being called
            int j = GetCFunAddr((int *)CFunctionFlash, i);
            if(!j) error("Internal fault 6(sorry)");
           // if(!j) j = GetCFunAddr((int *)CFunctionLibrary, i);    //Check the library
           // if(!j) error("Internal fault 6(sorry)");
            iret = (unsigned int)j;                                     // return the entry point
            targ = T_INT;
            return;
    }

    if((p = checkstring(argv[0], "WORD"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned int *)(GetPeekAddr(p) & 0b11111111111111111111111111111100);
        targ = T_INT;
        return;
        }
    if((p = checkstring(argv[0], "SHORT"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned short *)(GetPeekAddr(p) & 0b11111111111111111111111111111110);
        targ = T_INT;
        return;
        }
    if((p = checkstring(argv[0], "INTEGER"))){
        if(argc != 1) error("Syntax");
        iret = *(uint64_t *)(GetPeekAddr(p) & 0xFFFFFFF8);
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "FLOAT"))){
        if(argc != 1) error("Syntax");
        fret = *(MMFLOAT *)(GetPeekAddr(p) & 0xFFFFFFF8);
        targ = T_NBR;
        return;
        }

    if(argc != 3) error("Syntax");

    if((checkstring(argv[0], "PROGMEM"))){
        iret = *((char *)ProgMemory + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    if((checkstring(argv[0], "VARTBL"))){
        iret = *((char *)vartbl + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    if((p = checkstring(argv[0], "VAR"))){
		pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = *((char *)pp + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    // default action is the old syntax of  b = PEEK(hiaddr, loaddr)
	iret = *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2]));
    targ = T_INT;
}


// remove unnecessary text
void MIPS16 CrunchData(char **p, int c) {
    static char inquotes, lastch, incomment;

    if(c == '\n') c = '\r';                                         // CR is the end of line terminator
    if(c == 0  || c == '\r' ) {
        inquotes = false; incomment = false;                        // newline so reset our flags
        if(c) {
            if(lastch == '\r') return;                              // remove two newlines in a row (ie, empty lines)
            *((*p)++) = '\r';
        }
        lastch = '\r';
        return;
    }

    if(incomment) return;                                           // discard comments
    if(c == ' ' && lastch == '\r') return;                          // trim all spaces at the start of the line
    if(c == '"') inquotes = !inquotes;
    if(inquotes) {
        *((*p)++) = c;                                              // copy everything within quotes
        return;
    }
    if(c == '\'') {                                                 // skip everything following a comment
        incomment = true;
        return;
    }
    if(c == ' ' && (lastch == ' ' || lastch == ',')) {
        lastch = ' ';
        return;                                                     // remove more than one space or a space after a comma
    }
    *((*p)++) = lastch = c;
}

void MIPS16 cmd_autosave(void) {
    char *buf, *p, *r=NULL;
    char fname[256];
    int c,count=0,prevc = 0;
    int noecho = 0;
    uint64_t timeout;
    sendCRLF=3;
    int changemode=VideoMode;
    int changefont=gui_font;
    if(CurrentLinePtr) error("Invalid in a program");
    if((*cmdline == 0 || *cmdline == '\'')) {
    	error("Syntax");
    } else {
      	if(!InitSDCard()) return;
    	p = checkstring(cmdline, "N");  //G.A.
    	if(p){
    	  noecho=true;
    	  skipspace(p);
    	  r = getFstring(p);
       	}else{
           r = getFstring(cmdline);
    	}
    	// open the file

    	if(strchr(r, '.') == NULL) strcat(r, ".BAS");
    	strcpy(fname,r);
        ClearProgram();                                                 // clear any leftovers from the previous program
        p = buf = GetTempMemory(EDIT_BUFFER_SIZE);
  	    if((uint32_t)PageTable[WritePage].address!=0x24000000){
  	    	setmode(1,DEFCOLOUR,0,0);
  	    }
  	    SetFont((6 << 4) | 1);
        ShowCursor(false);
        /////////////////////////////////////////////////////

        int first = true;
        int skip_initial_lf = (p == buf);
        uint64_t lastchartime = GetuSec(); // Initialize it!
        while((c = MMInkey()) != F1 && c!=F2 && c!=F6 ){                    // while waiting for the end of text char
        	   //////////////////////////////
        	if(p == buf && c == '\n') continue;         // throw away an initial line feed which can follow the command
        	// Check timeout even when no character received
        	if (!first && GetuSec() - lastchartime > 100000)
        	{
        	  if (noecho)
        	  {
        	     MMPrintString("Enter F1 ,F2 or F6 to exit\r\n");
        	     noecho = false;
        	  }
        	}

        	// Early exit for no input
        	if (c == -1)
        	{
        	  if (count && GetuSec() - timeout > 100000)
        	    {
                 count = 0;
                }
                continue;
            }

           // Got a valid character - update timestamp and clear first flag
           lastchartime = GetuSec();
           first = false;

           // Handle initial LF
           if (skip_initial_lf && c == '\n')
           {
              skip_initial_lf = false;
              continue;
           }
           skip_initial_lf = false;

           ShowCursor(true);
           // if(p == buf && c == '\n') continue;                         // throw away an initial line feed which can follow the command

            if((p - buf) >= EDIT_BUFFER_SIZE) error("Not enough memory");
            if(IsPrint(c) || c == '\r' || c == '\n' || c == TAB) {
                if(c == TAB) c = ' ';
               // if(!(c == '\r')){
                    *p++ = c;
               //     if(count++ > 240)error("Line length > 240");
               // }// insert the input into RAM
                {
                    ShowCursor(false);
                    // Echo logic
                     int should_echo = !(c == '\n' && prevc == '\r');
                     if (should_echo)
                      {
                          timeout = GetuSec();
                          if (!noecho)
                               MMputchar(c);
                           count++;
                      }
                      if (c == '\r')
                      {
                         count = 0;
                         if (!noecho)
                         {
                             MMputchar('\n');
                         }
                      }
                	//MMputchar(c);     // and echo it
                	//if(c=='\n'){
                	//	count=0;
                	//	MMputchar('\r');
                	//}
                }
                prevc = c;
            }
        }

        if(VideoMode!=changemode)setmode(changemode,DEFCOLOUR,0,0);
        if(gui_font!=changefont)SetFont(changefont);
        ShowCursor(false);
        *p = 0;                                                         // terminate the string in RAM
        while(getConsole() != -1);                                      // clear any rubbish in the input

//        ClearSavedVars();                                               // clear any saved variables
    	int fnbr;
    	unsigned int nbr;
    	char *pm=buf;
    	if(!InitSDCard()) return;
    	fnbr = FindFreeFileNbr();
    	if(!BasicFileOpen(fname, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
    	while(*pm){
    		routinechecks(1);
    		f_write(FileTable[fnbr].fptr, pm, 1, &nbr);
    		pm++;
    	}
    	FileClose(fnbr);
    	PRet();MMPrintString("> Saved to SDcard");PRet();   //Let MMCC know we are done
	    char *q=&fname[strlen(fname)-4];
	    if(strcasecmp(q,".bas")!=0 || c==F6)return;
	  	SCB_CleanInvalidateDCache();
    	FileLoadProgram(fname,0);
		/*StartEditChar = */StartEditCharacter = 0;
	  	if(Option.profile){
	  		if(G1Hardware)mymemset((char *)0xD0300000, 0, 512*1024);
	  		else mymemset((char *)0xD0700000, 0, 512*1024);
	  	}
    	if(c==F2){
		  	SCB_CleanInvalidateDCache();
 			SCB_CleanInvalidateDCache();
	        gui_fcolour = PromptFC;
	        gui_bcolour = PromptBC;
	        MX470Display(DISPLAY_CLS);                          // clear screen on the MX470 display only
	        MX470Cursor(0, 0);                                  // home the cursor
			ClearVars(0);
			SetFont(Option.DefaultFont);
	    	setterminal();
			reset_CLUT();
	  		strcpy(inpbuf,"RUN\r\n");
	  	    tokenise(true);                                             // turn into executable code
	  	    ExecuteProgram(tknbuf);                                     // execute the line straight away
			cleanend();
    	}
    }
}






/***********************************************************************************************
interrupt check

The priority of interrupts (highest to low) is:
Touch (MM+ only)
CFunction Interrupt
ON KEY
I2C Slave Rx
I2C Slave Tx
COM1
COM2
COM3 (MM+ only)
COM4 (MM+ only)
GUI Int Down (MM+ only)
GUI Int Up (MM+ only)
WAV Finished (MM+ only)
IR Receive
I/O Pin Interrupts in order of definition
Tick Interrupts (1 to 4 in that order)

************************************************************************************************/

// check if an interrupt has occured and if so, set the next command to the interrupt routine
// will return true if interrupt detected or false if not
int check_interrupt(void) {
	int i, v;
	char *intaddr;
	static char rti[2];
	if(GUIactive && GUITimer1>=16 ){
		if(mousestruct[Option.Mouse].ax!=xcursor || mousestruct[Option.Mouse].ay!=ycursor){
			hidecursor(0);
			showcursor(0, mousestruct[Option.Mouse].ax, mousestruct[Option.Mouse].ay);
		}
		GUITimer1=0;
	}
	if(autocursor && mouseupdated){
		mouseupdated=0;
		int mouse=(mouse0?0:mouse1?1:mouse2?2:3);
		if(mousestruct[mouse].ax!=xcursor || mousestruct[mouse].ay!=ycursor){
			hidecursor(0);
			showcursor(0, mousestruct[mouse].ax, mousestruct[mouse].ay);
		}
	}

    ProcessTouch();                                             // check GUI touch
    if(CheckGuiFlag) CheckGui();                                // This implements a LED flash

    if(!InterruptUsed) return 0;                                    // quick exit if there are no interrupts set
	if(InterruptReturn != NULL || CurrentLinePtr == NULL) return 0;	// skip if we are in an interrupt or in immediate mode

//Deal with the fast tick interrupt first
    for(int i=1;i<=MAXPID;i++){
        if(PIDchannels[i].interrupt!=NULL && Getuptime()>PIDchannels[i].timenext && PIDchannels[i].active){
            PIDchannels[i].timenext=Getuptime()+(PIDchannels[i].PIDparams->T * 1000000);
            intaddr=(char *)PIDchannels[i].interrupt;
            goto GotAnInterrupt;
        }
    }
	if(TickInt[NBRSETTICKS] != NULL && TickTimer[NBRSETTICKS]) {
		TickTimer[NBRSETTICKS]--;
		intaddr = TickInt[NBRSETTICKS];
		goto GotAnInterrupt;
	}

    // check for an  ON KEY loc  interrupt
    if(OnKeyGOSUB && kbhitConsole()) {
        intaddr = OnKeyGOSUB;							            // set the next stmt to the interrupt location
		goto GotAnInterrupt;
    }
	// interrupt routines for the serial ports
    if(com1 || com2 || com3){
		if(com1_interrupt != NULL && SerialRxStatus(1) >= com1_ilevel) {// do we need to interrupt?
			intaddr = com1_interrupt;									// set the next stmt to the interrupt location
			goto GotAnInterrupt;
		}
		if(com1_TX_interrupt && com1_TX_complete){
			intaddr=com1_TX_interrupt;
			com1_TX_complete=false;
			goto GotAnInterrupt;
		}
		if(com2_interrupt != NULL && SerialRxStatus(2) >= com2_ilevel) {// do we need to interrupt?
			intaddr = com2_interrupt;									// set the next stmt to the interrupt location
			goto GotAnInterrupt;
		}
		if(com2_TX_interrupt && com2_TX_complete){
			intaddr=com2_TX_interrupt;
			com2_TX_complete=false;
			goto GotAnInterrupt;
		}
		if(com3_interrupt != NULL && SerialRxStatus(3) >= com3_ilevel) {// do we need to interrupt?
			intaddr = com3_interrupt;									// set the next stmt to the interrupt location
			goto GotAnInterrupt;
		}
		if(com3_TX_interrupt && com3_TX_complete){
			intaddr=com3_TX_interrupt;
			com3_TX_complete=false;
			goto GotAnInterrupt;
		}
    }
    if(gui_int_down && GuiIntDownVector) {                          // interrupt on pen down
        intaddr = GuiIntDownVector;                                 // get a pointer to the interrupt routine
        gui_int_down = false;
        goto GotAnInterrupt;
    }

    if(gui_int_up && GuiIntUpVector) {
        intaddr = GuiIntUpVector;                                   // get a pointer to the interrupt routine
        gui_int_up = false;
        goto GotAnInterrupt;
    }
    if(KeyInterrupt != NULL && Keycomplete) {
		Keycomplete=false;
		intaddr = KeyInterrupt;									    // set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
    if(WAVInterrupt != NULL && WAVcomplete) {
        WAVcomplete=false;
		intaddr = WAVInterrupt;									    // set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
    if(COLLISIONInterrupt != NULL && CollisionFound) {
        CollisionFound=false;
		intaddr = COLLISIONInterrupt;									    // set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
    if(ADCInterrupt != NULL && ADCcomplete){
 		ADCcomplete=false;
    	intaddr = ADCInterrupt;									    // set the next stmt to the interrupt location
    	goto GotAnInterrupt;
    }

    if(DACInterrupt != NULL && DACcomplete){
 		DACcomplete=false;
    	intaddr = (char *)DACInterrupt;									    // set the next stmt to the interrupt location
    	goto GotAnInterrupt;
    }

    if(IrGotMsg && IrInterrupt != NULL) {
        IrGotMsg = false;
		intaddr = IrInterrupt;									    // set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}

    if(FrameInterrupt != NULL && Framecomplete){
 		Framecomplete=false;
    	intaddr = FrameInterrupt;									    // set the next stmt to the interrupt location
    	goto GotAnInterrupt;
    }
    if(Count5High!=lastCount5High && CountInterrupt){
    	intaddr=CountInterrupt;
    	lastCount5High=Count5High;
    	goto GotAnInterrupt;
    }
    if(nun1 || nun2 || nun3 || classic1 || classic2 || classic3){
		if(nun1Interruptz !=NULL && nun1foundz){
			nun1foundz=0;
			intaddr=nun1Interruptz;
			goto GotAnInterrupt;
		}

		if(nun1Interruptc !=NULL && nun1foundc){
			nun1foundc=0;
			intaddr=nun1Interruptc;
			goto GotAnInterrupt;
		}
		if(nun2Interruptz !=NULL && nun2foundz){
			nun2foundz=0;
			intaddr=nun2Interruptz;
			goto GotAnInterrupt;
		}

		if(nun2Interruptc !=NULL && nun2foundc){
			nun2foundc=0;
			intaddr=nun2Interruptc;
			goto GotAnInterrupt;
		}

		if(nun3Interruptz !=NULL && nun3foundz){
			nun3foundz=0;
			intaddr=nun3Interruptz;
			goto GotAnInterrupt;
		}

		if(nun3Interruptc !=NULL && nun3foundc){
			nun3foundc=0;
			intaddr=nun3Interruptc;
			goto GotAnInterrupt;
		}
    }

    if(mouse0 || mouse1 || mouse2 || mouse3){
		if(mouse0Interruptz !=NULL && mouse0foundz){
			mouse0foundz=0;
			intaddr=mouse0Interruptz;
			goto GotAnInterrupt;
		}

		if(mouse0Interruptu !=NULL && mouse0leftup){
			mouse0leftup=0;
			intaddr=mouse0Interruptu;
			goto GotAnInterrupt;
		}

		if(mouse0Interruptc !=NULL && mouse0foundc){
			mouse0foundc=0;
			intaddr=mouse0Interruptc;
			goto GotAnInterrupt;
		}

		if(mouse1Interruptz !=NULL && mouse1foundz){
			mouse1foundz=0;
			intaddr=mouse1Interruptz;
			goto GotAnInterrupt;
		}

		if(mouse1Interruptc !=NULL && mouse1foundc){
			mouse1foundc=0;
			intaddr=mouse1Interruptc;
			goto GotAnInterrupt;
		}
		if(mouse1Interruptu !=NULL && mouse1leftup){
			mouse1leftup=0;
			intaddr=mouse1Interruptu;
			goto GotAnInterrupt;
		}
		if(mouse2Interruptz !=NULL && mouse2foundz){
			mouse2foundz=0;
			intaddr=mouse2Interruptz;
			goto GotAnInterrupt;
		}

		if(mouse2Interruptc !=NULL && mouse2foundc){
			mouse2foundc=0;
			intaddr=mouse2Interruptc;
			goto GotAnInterrupt;
		}

		if(mouse2Interruptu !=NULL && mouse2leftup){
			mouse2leftup=0;
			intaddr=mouse2Interruptu;
			goto GotAnInterrupt;
		}

		if(mouse3Interruptz !=NULL && mouse3foundz){
			mouse3foundz=0;
			intaddr=mouse3Interruptz;
			goto GotAnInterrupt;
		}

		if(mouse3Interruptc !=NULL && mouse3foundc){
			mouse3foundc=0;
			intaddr=mouse3Interruptc;
			goto GotAnInterrupt;
		}
		if(mouse3Interruptu !=NULL && mouse3leftup){
			mouse3leftup=0;
			intaddr=mouse3Interruptu;
			goto GotAnInterrupt;
		}

    }

    if(CSubInterrupt != NULL && CSubComplete) {
           	intaddr = CSubInterrupt;                                  // set the next stmt to the interrupt location
        	CSubComplete=0;
        	goto GotAnInterrupt;
    }

    for(i = 0; i < NBRINTERRUPTS; i++) {                            // scan through the interrupt table
		if(inttbl[i].pin != 0) {                                    // if this entry has an interrupt pin set
			v = ExtInp(inttbl[i].pin);								// get the current value of the pin
			// check if interrupt occured
			if((inttbl[i].lohi == T_HILO && v < inttbl[i].last) || (inttbl[i].lohi == T_LOHI && v > inttbl[i].last) || (inttbl[i].lohi == T_BOTH && v != inttbl[i].last)) {
				intaddr = inttbl[i].intp;							// set the next stmt to the interrupt location
				inttbl[i].last = v;									// save the new pin value
				goto GotAnInterrupt;
			} else
				inttbl[i].last = v;									// no interrupt, just update the pin value
		}
	}
	// check if one of the tick interrupts is enabled and if it has occured
	for(i = 0; i < NBRSETTICKS; i++) {
    	if(TickInt[i] != NULL && TickTimer[i] > TickPeriod[i]) {
    		// reset for the next tick but skip any ticks completely missed
    		while(TickTimer[i] > TickPeriod[i]) TickTimer[i] -= TickPeriod[i];
    		intaddr = TickInt[i];
    		goto GotAnInterrupt;
    	}
    }

    // if no interrupt was found then return having done nothing
	return 0;

    // an interrupt was found if we jumped to here
GotAnInterrupt:
    LocalIndex++;                                                   // IRETURN will decrement this
    if(OptionErrorSkip>0)SaveOptionErrorSkip=OptionErrorSkip;
    else SaveOptionErrorSkip = 0;
    OptionErrorSkip=0;
    strcpy( SaveMMErrMsg, MMErrMsg);   //save error message and clear
    *MMErrMsg=0;
    SaveMMerrno=MMerrno;              // saved MMerrno and clear
    MMerrno=0;
    InterruptReturn = nextstmt;                                     // for when IRETURN is executed
    // if the interrupt is pointing to a SUB token we need to call a subroutine
#ifndef CMD16BIT
    if(*intaddr == cmdSUB) {
        strncpy(CurrentInterruptName, intaddr + 1, MAXVARLEN);
        rti[0] = cmdIRET;                                           // setup a dummy IRETURN command
        rti[1] = 0;
#else
    CommandToken tkn = commandtbl_decode((const char *)intaddr);
    if (tkn == cmdSUB){
        strncpy(CurrentInterruptName, intaddr + 2, MAXVARLEN);
        rti[0] = (cmdIRET & 0x7f) + C_BASETOKEN;
        rti[1] = (cmdIRET >> 7) + C_BASETOKEN; // tokens can be 14-bit
#endif
        if(gosubindex >= MAXGOSUB) error("Too many SUBs for interrupt");
        errorstack[gosubindex] = CurrentLinePtr;
    	gosubstack[gosubindex++] = rti;                             // return from the subroutine to the dummy IRETURN command
        LocalIndex++;                                               // return from the subroutine will decrement LocalIndex
        skipelement(intaddr);                                       // point to the body of the subroutine
    }

    nextstmt = intaddr;                                             // the next command will be in the interrupt routine
    return 1;
}


// get the address for a MMBasic interrupt
// this will handle a line number, a label or a subroutine
// all areas of MMBasic that can generate an interrupt use this function
char *GetIntAddress(char *p) {
    int i;
	if(isnamestart(*p)) {                                           // if it starts with a valid name char
    	i = FindSubFun(p, 0);                                       // try to find a matching subroutine
    	if(i == -1)
		    return findlabel(p);					                // if a subroutine was NOT found it must be a label
		else
		    return subfun[i];                                       // if a subroutine was found, return the address of the sub
	}

	return findline(getinteger(p), true);	                        // otherwise try for a line number
}
void fun_json(void){
    char *json_string=NULL;
    const cJSON *root = NULL;
    void *ptr1 = NULL;
    char *p;
	int64_t *dest=NULL;
    MMFLOAT tempd;
    int i,j,k,mode,index;
    char field[32],num[6];
    getargs(&ep, 3, ",");
    char *a=GetTempStrMemory();
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_INT) {
    if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
        error("Argument 1 must be integer array");
    }
    dest = (long long int *)ptr1;
    json_string=(char *)&dest[1];
    } else error("Argument 1 must be integer array");
    cJSON * parse = cJSON_Parse(json_string);
    if(parse==NULL)error("Invalid JSON data");
    root=parse;
    p=getCstring(argv[2]);
    int len = strlen(p);
    memset(field,0,32);
    memset(num,0,6);
    i=0;j=0;k=0;mode=0;
    while(i<len){
        if(p[i]=='['){ //start of index
            mode=1;
            field[j]=0;
            root = cJSON_GetObjectItemCaseSensitive(root, field);
            memset(field,0,32);
            j=0;
        }
        if(p[i]==']'){
            num[k]=0;
            index=atoi(num);
            root = cJSON_GetArrayItem(root, index);
            memset(num,0,6);
            k=0;
        }
        if(p[i]=='.'){ //new field separator
            if(mode==0){
                field[j]=0;
                root = cJSON_GetObjectItemCaseSensitive(root, field);
             memset(field,0,32);
                j=0;
            } else { //step past the dot after a close bracket
                mode=0;
            }
        } else  {
            if(mode==0)field[j++]=p[i];
            else if(p[i]!='[')num[k++]=p[i];
        }
        i++;
    }
    root = cJSON_GetObjectItem(root, field);

    if (cJSON_IsObject(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsInvalid(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsNumber(root))
    {
        tempd = root->valuedouble;

        if((MMFLOAT)((int64_t)tempd)==tempd) IntToStr(a,(int64_t)tempd,10);
        else FloatToStr(a, tempd, 0, STR_AUTO_PRECISION, ' ');   // set the string value to be saved
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsBool(root)){
        int64_t tempint;
        tempint=root->valueint;
        cJSON_Delete(parse);
        if(tempint)strcpy(sret,"true");
        else strcpy(sret,"false");
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsString(root)){
        strcpy(a,root->valuestring);
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    cJSON_Delete(parse);
    targ=T_STR;
    sret=a;
}
