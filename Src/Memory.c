/***********************************************************************************************************************
Maximite

timers.c

This module manages all RAM memory allocation for MMBasic running on the Micromite.

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

**************************************************************************************************************************

NOTE:
  In the PIC32 the following variables are set by the linker:

      (unsigned char *)&_stack         This is the virtual address of the top of the stack and unless some RAM functions are
                                       defined it is also the top of the RAM.  In this case its value is 0xA0020000.

      (unsigned char *)&_splim         This is the virtual address of the top of the heap and represents the start of free
                                       and unallocated memory.

      (unsigned char *)&_heap          This is the virtual address of the top of the memory allocated by the compiler (static
                                       variables, etc)

      (unsigned int)&_min_stack_size   This is the number of bytes allocated to the stack.  No run time checking is performed
                                       and this value is only used by the linker to warn if memory is over allocated.


  The RAM memory map looks like this:

  |--------------------|    <<<   0xA0002000  (Top of RAM)
  |  Functions in RAM  |
  |--------------------|    <<<   (unsigned char *)&_stack
  |                    |
  | Stack (grows down) |
  |                    |
  |--------------------|    <<<   (unsigned char *)&_stack - (unsigned int)&_min_stack_size
  |                    |
  |                    |
  |     Free RAM       |
  |                    |
  |                    |
  |--------------------|   <<<   (unsigned char *)&_splim
  |                    |
  | Heap (if allocated)|
  |                    |
  |--------------------|   <<<   (unsigned char *)&_heap
  |                    |
  |                    |
  |                    |
  |  Static RAM Vars   |
  |                    |
  |                    |
  |                    |
  |--------------------|    <<<   0xA0000000


  The variables must be defined to the C Compiler before using them.  Eg:
      extern unsigned int _stack;
      extern unsigned int _splim;
      extern unsigned int _heap;
      extern unsigned int _min_stack_size;


************************************************************************************************************************/



				// the pre Harmony peripheral libraries

#define INCLUDE_FUNCTION_DEFINES

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"



// memory parameters for this chip
// ===============================
// The following settings will allow MMBasic to use all the free memory on the PIC32.  If you need some RAM for
// other purposes you can declare the space needed as a static variable -or- allocate space to the heap (which
// will reduce the memory available to MMBasic) -or- change the definition of RAMEND.
// NOTE: MMBasic does not use the heap.  It has its own heap which is allocated out of its own memory space.

// The virtual address that MMBasic can start using memory.  This must be rounded up to RAMPAGESIZE.
// MMBasic uses just over 5K for static variables so, in the simple case, RAMBASE could be set to 0xA001800.
// However, the PIC32 C compiler provides us with a convenient marker (see diagram above).


volatile char *StrTmp[MAXTEMPSTRINGS];                                       // used to track temporary string space on the heap
volatile char StrTmpLocalIndex[MAXTEMPSTRINGS];                              // used to track the LocalIndex for each temporary string space on the heap

//unsigned char *VarTableTop;                                         // this is the top of the RAM table

unsigned int MBitsGet(void *addr);
void MBitsSet(void *addr, int bits);
//void *getheap(int size);
//unsigned int SBitsGet(void *addr);
//void SBitsSet(void *addr, int bits);
//void *getSheap(int size);
unsigned int SDBitsGet(void *addr);
void SDBitsSet(void *addr, int bits);
//void *getSDRAM(int size);
unsigned int UsedHeap(void);
void *GetStringMemory(void);
//void heapstats(char *m1);
int TempMemoryIsChanged = false;						            // used to prevent unnecessary scanning of strtmp[]
int StrTmpIndex = 0;                                                // index to the next unallocated slot in strtmp[]
//unsigned int   __attribute__ ((aligned (8))) smap[16]; //16*16*256 = 64Kbyte of memory
unsigned int *SDmap=(unsigned int *)0x30000000; //space for 24Mbyte of memory
unsigned int __attribute__ ((aligned (8))) mmap[65];// 64*16*256 = 256Kbyes of memory
extern unsigned int GetPokeAddr(char *p);
extern unsigned int GetPeekAddr(char *p);
#define ASMMAX 6400 // maximum number of bytes that can be copied or set by assembler routines
#define MAXCPY 3200 // tuned maximum number of bytes to copy using ZCOPY
uint8_t *tbuff=(uint8_t *)FUNEND; // buffer in fast memory for SDRAM to SDRAM copies
void *minMMheap;
uint32_t RAMBASE = 0x30008000;
extern int CheckEmpty(char *p);
/***********************************************************************************************************************
 MMBasic commands
************************************************************************************************************************/
//__attribute__((section(".fastcode")))
//void cmd_memory(void)
//{
// PIntH(&cmd_memory);PRet();
  //chprintf(chp, "addr of fun2 is 0x%x", (uint32_t) &testITCMfun2);
//}


//__attribute__((section(".fastcode")))
//void  cmd_memory(void) {
void MIPS16 cmd_memory(void) {
#if !defined(LITE)
	char *p,*tp;
    tp = checkstring(cmdline, "PACK");
    if(tp){
        getargs(&tp,7,",");
        if(argc!=7)error("Syntax");
        int i,n=getinteger(argv[4]);
        if(n<=0)return;
        int size=getint(argv[6],1,32);
        if(!(size==1 || size==4 || size==8 || size==16 || size==32))error((char *)"Invalid size");
        int sourcesize,destinationsize;
        void *top=NULL;
        uint64_t *from=NULL;
        if(CheckEmpty((char *)argv[0])){
            sourcesize=parseintegerarray(argv[0],(int64_t **)&from, 1,1,NULL,false,NULL);
            if(sourcesize<n)error("Source array too small");
        } else from=(uint64_t *)GetPokeAddr(argv[0]);
        if(CheckEmpty((char *)argv[2])){
            destinationsize=parseintegerarray(argv[2],(int64_t **)&top, 2,1,NULL,true,NULL);
            if(destinationsize*64/size<n)error("Destination array too small");
        } else top=(void *)GetPokeAddr(argv[2]);
        if((uint32_t)from % 8)error("Source address not divisible by 8");
        if(size==1){
            uint8_t *to=(uint8_t *)top;
            for(i=0;i<n;i++){
                int s= i % 8;
                if(s==0)*to=0;
                *to |= ((*from++) & 0x1)<<s;
                if(s==7)to++;
           }
        } else if(size==4){
            uint8_t *to=(uint8_t *)top;
            for(i=0;i<n;i++){
                if((i & 1) == 0){
                    *to=(*from++) & 0xF;
                } else {
                    *to |= ((*from++) & 0xF)<<4;
                    to++;
                }
           }
        } else if(size==8){
            uint8_t *to=(uint8_t *)top;
            while(n--){
            *to++=(uint8_t)*from++;
            }
        } else if(size==16){
            uint16_t *to=(uint16_t *)top;
            if((uint32_t)to % 2)error("Destination address not divisible by 2");
            while(n--){
            *to++=(uint16_t)*from++;
            }
        } else if(size==32){
            uint32_t *to=(uint32_t *)top;
            if((uint32_t)to % 4)error("Destination address not divisible by 4");
            while(n--){
            *to++=(uint32_t)*from++;
            }
        }
        return;
    }
    tp = checkstring(cmdline, "PRINT");
    if(tp){
        char *fromp=NULL;
        int sourcesize;
        int64_t *aint;
        getargs(&tp,5,",");
        if(!(argc==5))error("Syntax");
	    if(*argv[0] == '#') argv[0]++;
		int fnbr = getint(argv[0],1,MAXOPENFILES);	// get the number
        int n=getinteger(argv[2]);
        if(CheckEmpty((char *)argv[4])){
            sourcesize=parseintegerarray(argv[4],&aint,3,1,NULL,false,NULL);
            if(sourcesize*8<n)error("Source array too small");
            fromp=(char *)aint;
        } else {
            fromp=(char *)GetPeekAddr(argv[4]);
        }
        if (FileTable[fnbr].com > MAXCOMPORTS)
        {
            FilePutStr(n, fromp, fnbr);
        }
        else error("File % not open",fnbr);
        return;
    }
    tp = checkstring(cmdline, "INPUT");
    if(tp){
        char *fromp=NULL;
        int sourcesize;
        int64_t *aint;
        getargs(&tp,5,",");
        if(!(argc==5))error("Syntax");
	    if(*argv[0] == '#') argv[0]++;
		int fnbr = getint(argv[0],1,MAXOPENFILES);	// get the number
        int n=getinteger(argv[2]);
        if(CheckEmpty((char *)argv[4])){
            sourcesize=parseintegerarray(argv[4],&aint,3,1,NULL,false,NULL);
            if(sourcesize*8<n)error("Source array too small");
            fromp=(char *)aint;
        } else {
            fromp=(char *)GetPokeAddr(argv[4]);
        }
        if (FileTable[fnbr].com > MAXCOMPORTS)
        {
            while(!(MMfeof(fnbr)) && n--) *fromp++=FileGetChar(fnbr);
            if(n)error("End of file");
        }
        else error("File % not open",fnbr);
        return;
    }
    tp = checkstring(cmdline, "UNPACK");
    if(tp){
        getargs(&tp,7,",");
        if(argc!=7)error("Syntax");
        int i,n=getinteger(argv[4]);
        if(n<=0)return;
        int size=getint(argv[6],1,32);
        if(!(size==1 || size==4 || size==8 || size==16 || size==32))error((char *)"Invalid size");
        int sourcesize,destinationsize;
        uint64_t *to=NULL;
        void *fromp=NULL;
        if(CheckEmpty((char *)argv[0])){
            sourcesize=parseintegerarray(argv[0],(int64_t **)&fromp, 1,1,NULL,false,NULL);
            if(sourcesize*64/size<n)error("Source array too small");
        } else {
            fromp=(void*)GetPokeAddr(argv[0]);
        }
        if(CheckEmpty((char *)argv[2])){
            destinationsize=parseintegerarray(argv[2],(int64_t **)&to, 2,1,NULL,true,NULL);
            if(n>destinationsize)error("Destination array too small");
        } else to=(uint64_t *)GetPokeAddr(argv[2]);
        if((uint32_t)to % 8)error("Source address not divisible by 8");
        if(size==1){
            uint8_t *from=(uint8_t *)fromp;
            for(i=0;i<n;i++){
                int s= i % 8;
                *to++ = ((*from & (1<<s)) ? 1 : 0);
                if(s==7)from++;
           }

        } else if(size==4){
            uint8_t *from=(uint8_t *)fromp;
            for(i=0;i<n;i++){
                if((i & 1) == 0){
                    *to++=(*from) & 0xF;
                } else {
                    *to++ = (*from) >> 4;
                    from++;
                }
           }
        } else if(size==8){
            uint8_t *from=(uint8_t *)fromp;
            while(n--){
            *to++=(uint64_t)*from++;
            }
        } else if(size==16){
            uint16_t *from=(uint16_t *)fromp;
            if((uint32_t)from % 2)error("Source address not divisible by 2");
            while(n--){
            *to++=(uint64_t)*from++;
            }
        } else if(size==32){
            uint32_t *from=(uint32_t *)fromp;
            if((uint32_t)from % 4)error("Source address not divisible by 4");
            while(n--){
            *to++=(uint64_t)*from++;
            }
        }
        return;
    }
    tp = checkstring(cmdline, "COPY");
    if(tp){
    	if((p = checkstring(tp, "INTEGER"))) {
    		int stepin=1, stepout=1;
        	getargs(&p,9,",");
        	if(argc<5)error("Syntax");
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
         	uint64_t *from=(uint64_t *)GetPokeAddr(argv[0]);
         	uint64_t *to=(uint64_t *)GetPokeAddr(argv[2]);
        	if((uint32_t)from % 8)error("Address not divisible by 8");
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	if(argc>=7 && *argv[6])stepin=getint(argv[6],0,0xFFFF);
        	if(argc==9)stepout=getint(argv[8],0,0xFFFF);
        	if(stepin==1 && stepout==1)mycopy(to, from, n*8);
        	else{
            	while(n--){
            		*to=*from;
            		to+=stepout;
            		from+=stepin;
            	}
        	}
    		return;
    	}
    	if((p = checkstring(tp, "FLOAT"))) {
    		int stepin=1, stepout=1;
        	getargs(&p,9,","); //assume byte
        	if(argc<5)error("Syntax");
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	MMFLOAT *from=(MMFLOAT *)GetPokeAddr(argv[0]);
        	MMFLOAT *to=(MMFLOAT *)GetPokeAddr(argv[2]);
        	if((uint32_t)from % 8)error("Address not divisible by 8");
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	if(argc>=7 && *argv[6])stepin=getint(argv[6],0,0xFFFF);
        	if(argc==9)stepout=getint(argv[8],0,0xFFFF);
        	if(n<=0)return;
        	if(stepin==1 && stepout==1)mycopy(to, from, n*8);
        	else{
            	while(n--){
            		*to=*from;
            		to+=stepout;
            		from+=stepin;
            	}
        	}
    		return;
    	}
    	getargs(&tp,5,",");
    	if(argc!=5)error("Syntax");
    	char *from=(char *)GetPeekAddr(argv[0]);
    	char *to=(char *)GetPokeAddr(argv[2]);
    	int n=getinteger(argv[4]);
    	mycopy(to, from, n);
    	return;
    }
    tp = checkstring(cmdline, "SET");
    if(tp){
    	char *p;
    	if((p = checkstring(tp, "BYTE"))) {
        	getargs(&p,5,","); //assume byte
        	if(argc!=5)error("Syntax");
         	char *to=(char *)GetPokeAddr(argv[0]);
         	int val=getint(argv[2],0,255);
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	mymemset(to, val, n);
    		return;
    	}
    	if((p = checkstring(tp, "SHORT"))) {
        	getargs(&p,5,","); //assume byte
        	if(argc!=5)error("Syntax");
         	char *to=(char *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 2)error("Address not divisible by 2");
        	char *q=(char *)to;
        	union {
        		uint8_t c[4];
    			uint16_t a[2];
    			uint32_t b;
    		}data;
    		data.a[0]=getint(argv[2],0,65535);
    		data.a[1]=data.a[0];
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	if((uint32_t)to & 3){ //get to word boundary
        		*q++=data.c[0];
        		*q++=data.c[1];
        		n--;
        	}
        	while(n>ASMMAX/2){
        		myset(q,data.b,ASMMAX);
        		n-=ASMMAX/2;
        		q+=ASMMAX;
        	}
        	if(n>1){
        		myset(q,data.b,n<<1);
        	}
        	if(n & 1){
        		q+=((n>>1)<<1);
        		*q++=data.c[0];
        		*q++=data.c[1];
        	}
    		return;
    	}
    	if((p = checkstring(tp, "WORD"))) {
        	getargs(&p,5,",");
        	if(argc!=5)error("Syntax");
         	char *to=(char *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 4)error("Address not divisible by 4");
        	char *q=(char *)to;
        	uint32_t data;
    		data=getinteger(argv[2]) & 0xFFFFFFFF;
        	int n=getinteger(argv[4]);
        	if(n<=0)return;
        	while(n>ASMMAX/4){
        		myset(q,data,ASMMAX);
        		n-=ASMMAX/4;
        		q+=ASMMAX;
        	}
        	if(n)myset(q,data,n<<2);
    		return;
    	}
    	if((p = checkstring(tp, "INTEGER"))) {
    		int stepin=1;
        	getargs(&p,7,",");
        	if(argc<5)error("Syntax");
         	uint64_t *to=(uint64_t *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	int64_t data;
    		data=getinteger(argv[2]);
        	int n=getinteger(argv[4]);
        	if(argc==7)stepin=getint(argv[6],0,0xFFFF);
        	if(n<=0)return;
        	if(stepin==1)while(n--)*to++=data;
        	else{
            	while(n--){
            		*to=data;
            		to+=stepin;
            	}
        	}
    		return;
    	}
    	if((p = checkstring(tp, "FLOAT"))) {
    		int stepin=1;
        	getargs(&p,7,","); //assume byte
        	if(argc<5)error("Syntax");
        	MMFLOAT *to=(MMFLOAT *)GetPokeAddr(argv[0]);
        	if((uint32_t)to % 8)error("Address not divisible by 8");
        	MMFLOAT data;
    		data=getnumber(argv[2]);
        	int n=getinteger(argv[4]);
           	if(argc==7)stepin=getint(argv[6],0,0xFFFF);
        	if(n<=0)return;
        	if(stepin==1)while(n--)*to++=data;
        	else{
            	while(n--){
            		*to=data;
            		to+=stepin;
            	}
        	}
    		return;
    	}
    	getargs(&tp,5,","); //assume byte
    	if(argc!=5)error("Syntax");
     	char *to=(char *)GetPokeAddr(argv[0]);
     	int val=getint(argv[2],0,255);
    	int n=getinteger(argv[4]);
    	if(n<=0)return;
    	mymemset(to, val, n);
    	return;
    }
    int i, j, var, nbr, vsize, VarCnt;
    int ProgramSize, ProgramPercent, VarSize, VarPercent, GeneralSize, GeneralPercent, SavedVarSize, SavedVarSizeK, SavedVarPercent, SavedVarCnt;
    int CFunctSize, CFunctSizeK, CFunctNbr, CFunctPercent, FontSize, FontSizeK, FontNbr, FontPercent;
    unsigned int CurrentRAM, *pint;
	SCB_CleanInvalidateDCache();
    CurrentRAM = ((unsigned int)RAMEND - (unsigned int)RAMBASE) + ((unsigned int)SDRAMEND - (unsigned int)SDRAMBASE) ;

    // calculate the space allocated to variables on the heap
    for(i = VarCnt = vsize = var = 0; var < MAXVARS; var++) {
        if(vartbl[var].type == T_NOTYPE) continue;
        VarCnt++;  vsize += sizeof(struct s_vartbl);
        if(vartbl[var].val.s == NULL) continue;
        if(vartbl[var].type & T_PTR) continue;
        nbr = vartbl[var].dims[0] + 1 - OptionBase;
        if(vartbl[var].dims[0]) {
            for(j = 1; j < MAXDIM && vartbl[var].dims[j]; j++)
                nbr *= (vartbl[var].dims[j] + 1 - OptionBase);
            if(vartbl[var].type & T_NBR)
                i += MRoundUp(nbr * sizeof(MMFLOAT));
            else if(vartbl[var].type & T_INT)
                i += MRoundUp(nbr * sizeof(long long int));
            else
                i += MRoundUp(nbr * (vartbl[var].size + 1));
        } else
            if(vartbl[var].type & T_STR)
                i += STRINGSIZE;
    }
    VarSize = (vsize + i + 512)/1024;                               // this is the memory allocated to variables
    VarPercent = ((vsize + i) * 100)/CurrentRAM;
    if(VarCnt && VarSize == 0) VarPercent = VarSize = 1;            // adjust if it is zero and we have some variables
    i = UsedHeap() - i;
    if(i < 0) i = 0;
    GeneralSize = (i + 512)/1024; GeneralPercent = (i * 100)/CurrentRAM;
    // count the space used by saved variables (in flash)
    p = (char *)SAVED_VAR_RAM_ADDR;
    SavedVarCnt = 0;
    while(!(*p == 0 || *p == 0xff)) {
        unsigned char type, array;
        SavedVarCnt++;
        type = *p++;
        array = type & 0x80;  type &= 0x7f;                         // set array to true if it is an array
        if(!(type==T_INT || type==T_STR || type==T_NBR)){
        	ClearSavedVars();
        	SavedVarCnt=0;
        	break;
        }
        p += strlen(p) + 1;
        if(array)
            p += (p[0] | p[1] << 8 | p[2] << 16| p[3] << 24) + 4;
        else {
            if(type &  T_NBR)
                p += sizeof(MMFLOAT);
            else if(type &  T_INT)
                p += sizeof(long long int);
            else
                p += *p + 1;
        }
    }
    SavedVarSize = p - (char *)SAVED_VAR_RAM_ADDR;
    SavedVarSizeK = (SavedVarSize + 512) / 1024;
    SavedVarPercent = (SavedVarSize * 100) / (PROG_FLASH_SIZE + SAVED_VAR_RAM_SIZE);
    if(SavedVarCnt && SavedVarSizeK == 0) SavedVarPercent = SavedVarSizeK = 1;        // adjust if it is zero and we have some variables

    // count the space used by CFunctions, CSubs and fonts
    CFunctSize = CFunctNbr = FontSize = FontNbr = 0;
    pint = (unsigned int *)CFunctionFlash;
    while(*pint != 0xffffffff) {
        if(*pint <FONT_TABLE_SIZE) {
            pint++;
            FontNbr++;
            FontSize += *pint + 8;
        } else {
            pint++;
            CFunctNbr++;
            CFunctSize += *pint + 8;
        }
        pint += (*pint + 4) / sizeof(unsigned int);
    }
    CFunctPercent = (CFunctSize * 100) /  (PROG_FLASH_SIZE + SAVED_VAR_RAM_SIZE);
    CFunctSizeK = (CFunctSize + 512) / 1024;
    if(CFunctNbr && CFunctSizeK == 0) CFunctPercent = CFunctSizeK = 1;              // adjust if it is zero and we have some functions
    FontPercent = (FontSize * 100) /  (PROG_FLASH_SIZE + SAVED_VAR_RAM_SIZE);
    FontSizeK = (FontSize + 512) / 1024;
    if(FontNbr && FontSizeK == 0) FontPercent = FontSizeK = 1;                      // adjust if it is zero and we have some functions

    // count the number of lines in the program
    p = (char *)ProgMemory;
    i = 0;
    while(*p != 0xff) {                                             // skip if program memory is erased
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) break;                                          // end of the program or module
        if(*p == T_NEWLINE) {
            i++;                                                    // count the line
            p++;                                                    // skip over the newline token
        }
        if(*p == T_LINENBR) p += 3;                                 // skip over the line number
        skipspace(p);
        if(p[0] == T_LABEL) p += p[1] + 2;                          // skip over the label
        while(*p) p++;                                              // look for the zero marking the start of an element
    }
    ProgramSize = ((p - ProgMemory) + 512)/1024;
    ProgramPercent = ((p - ProgMemory) * 100)/(PROG_FLASH_SIZE + SAVED_VAR_RAM_SIZE);
    if(ProgramPercent > 100) ProgramPercent = 100;
    if(i && ProgramSize == 0) ProgramPercent = ProgramSize = 1;                                        // adjust if it is zero and we have some lines

    MMPrintString("Program:\r\n");
    IntToStrPad(inpbuf, ProgramSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), ProgramPercent, ' ', 2, 10); strcat(inpbuf, "%) Program (");
    IntToStr(inpbuf + strlen(inpbuf), i, 10); strcat(inpbuf, " lines)\r\n");
	MMPrintString(inpbuf);

    if(CFunctNbr) {
        IntToStrPad(inpbuf, CFunctSizeK, ' ', 4, 10); strcat(inpbuf, "K (");
        IntToStrPad(inpbuf + strlen(inpbuf), CFunctPercent, ' ', 2, 10); strcat(inpbuf, "%) "); MMPrintString(inpbuf);
        IntToStr(inpbuf, CFunctNbr, 10); strcat(inpbuf, " Embedded C Routine"); strcat(inpbuf, CFunctNbr == 1 ? "\r\n":"s\r\n");
        MMPrintString(inpbuf);
    }

    if(FontNbr) {
        IntToStrPad(inpbuf, FontSizeK, ' ', 4, 10); strcat(inpbuf, "K (");
        IntToStrPad(inpbuf + strlen(inpbuf), FontPercent, ' ', 2, 10); strcat(inpbuf, "%) "); MMPrintString(inpbuf);
        IntToStr(inpbuf, FontNbr, 10); strcat(inpbuf, " Embedded Fonts"); strcat(inpbuf, FontNbr == 1 ? "\r\n":"s\r\n");
        MMPrintString(inpbuf);
    }

    if(SavedVarCnt) {
        IntToStrPad(inpbuf, SavedVarSizeK, ' ', 4, 10); strcat(inpbuf, "K (");
        IntToStrPad(inpbuf + strlen(inpbuf), SavedVarPercent, ' ', 2, 10); strcat(inpbuf, "%)");
        IntToStrPad(inpbuf + strlen(inpbuf), SavedVarCnt, ' ', 2, 10); strcat(inpbuf, " Saved Variable"); strcat(inpbuf, SavedVarCnt == 1 ? " (":"s (");
        IntToStr(inpbuf + strlen(inpbuf), SavedVarSize, 10); strcat(inpbuf, " bytes)\r\n");
        MMPrintString(inpbuf);
    }

    IntToStrPad(inpbuf, ((PROG_FLASH_SIZE + SAVED_VAR_RAM_SIZE) + 512)/1024 - ProgramSize - CFunctSizeK - FontSizeK - SavedVarSizeK , ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), 100 - ProgramPercent - CFunctPercent - FontPercent - SavedVarPercent , ' ', 2, 10); strcat(inpbuf, "%) Free\r\n");
	MMPrintString(inpbuf);

    MMPrintString("\r\nData:\r\n");
    IntToStrPad(inpbuf, VarSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), VarPercent, ' ', 2, 10); strcat(inpbuf, "%) ");
    IntToStr(inpbuf + strlen(inpbuf), VarCnt, 10); strcat(inpbuf, " Variable"); strcat(inpbuf, VarCnt == 1 ? "\r\n":"s\r\n");
	MMPrintString(inpbuf);

    IntToStrPad(inpbuf, GeneralSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), GeneralPercent, ' ', 2, 10); strcat(inpbuf, "%) General\r\n");
	MMPrintString(inpbuf);

    IntToStrPad(inpbuf, (CurrentRAM + 512)/1024 - VarSize - GeneralSize, ' ', 4, 10); strcat(inpbuf, "K (");
    IntToStrPad(inpbuf + strlen(inpbuf), 100 - VarPercent - GeneralPercent, ' ', 2, 10); strcat(inpbuf, "%) Free\r\n");
	MMPrintString(inpbuf);
#else
    // count the number of lines in the program
    char *p = ProgMemory;
	while(*p != 0xff) {                                             // skip if program memory is erased
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) break;                                          // end of the program or module
        if(*p == T_NEWLINE) p++;                                    // skip over the newline token
        if(*p == T_LINENBR) p += 3;                                 // skip over the line number
		skipspace(p);
		if(p[0] == T_LABEL) p += p[1] + 2;							// skip over the label
		while(*p) p++;												// look for the zero marking the start of an element
    }
    IntToStr(inpbuf, ((Option.ProgFlashSize - (p - ProgMemory)) + 512)/1024, 10);
    MMPrintString(inpbuf); MMPrintString("K free flash");
#endif
}



/***********************************************************************************************************************
 Public memory management functions
************************************************************************************************************************/

/* all memory allocation (except for the heap) is made by m_alloc()
   memory layout used by MMBasic:

          |------MMBasic Heap -|    <<<   This is the end of the RAM allocated to MMBasic (defined as RAMEND)
          |                    |
          |    Variable Data   |
          |(allocated downward)|
          |         \/         |
          | -----------------  |  vartbl is hashed and not in heap.
          |         /\         |
          |   Temporary Memory |
          | (allocated upwards)|
          |--------------------|   <<<   RAMBASE
                                         This is the start of the RAM allocated to MMBasic (defined as RAMBASE)

   m_alloc(size) is called when the program is running and whenever the variable table needs to be expanded

   Separately calls are made to GetMemory() and FreeMemory() to allocate or free space on the heap (which grows downward
   towards the variable table).  While the program is running an out of memory situation will occur when the space between
   the heap (growing downwards) and the variable table (growing up) reaches zero.

*/
void mymemset(void *out, uint32_t in, int n){
	if(!n)return;
	uint8_t d=(char)in;
	uint32_t dd=d | (d<<8) | (d<<16) | (d<<24);
	char *p=(char *)out;
	while(((uint32_t)p & 3) && n){ //get to word boundary
		*p++=d;
		n--;
	}
	while(n>ASMMAX){
		myset(p,dd,ASMMAX);
		n-=ASMMAX;
		p+=ASMMAX;
	}
	if(n)myset(p,dd,n);
}
void zcopy(void *out, const void *in, int n){
	if(((uint32_t)out & 0x80000000) && ((uint32_t)in & 0x80000000)){ //copy is to and from SDRAM
		uint32_t *t= (uint32_t *) tbuff;
		_Z10copy_wordsPKmPmm((uint32_t *)in, t, n>>2);
		_Z10copy_wordsPKmPmm(t, (uint32_t *)out, n>>2);
	} else {
		_Z10copy_wordsPKmPmm((uint32_t *)in, (uint32_t *)out, n>>2);
	}
	routinechecks(1);
}

void mycopy(void *out, const void *in, int n){
	if(n==0 || in==out)return;
	if(n<16){
		while(n--)*(char *)out++=*(char *)in++;
		return;
	}
	uint32_t *t= (uint32_t *) tbuff;
//	PIntH((uint32_t)out);PIntHC((uint32_t)out);PIntComma(n);PRet();MM_Delay(1000);
	if((uint32_t)out & 0x80000000 && (uint32_t)in & 0x80000000){ //copy is to and from SDRAM
		if(((uint32_t)out & 3)==0 && ((uint32_t)in & 3)==0){ // start points both on word boundaries
			uint8_t *d, *s;
			d=(uint8_t *)out; s=(uint8_t *)in;
			while(n>=TBUFFSIZE){
				_Z10copy_wordsPKmPmm((uint32_t *)s, t, TBUFFSIZE/4);
				_Z10copy_wordsPKmPmm(t, (uint32_t *)d, TBUFFSIZE/4);
				d+=TBUFFSIZE;
				s+=TBUFFSIZE;
				n-=TBUFFSIZE;
				routinechecks(1);
			}
			if(n & ~3){
				_Z10copy_wordsPKmPmm((uint32_t *)s, t, n>>2);
				_Z10copy_wordsPKmPmm(t, (uint32_t *)d, n>>2);
				d+=(n & ~3);
				s+=(n & ~3);
				n-=(n & ~3);
			}
			while(n--){
				*d++=*s++;
			}
			routinechecks(1);
		} else if(((uint32_t)out & 1)==0 && ((uint32_t)in & 1)==0){// start points both on half word boundaries
			uint8_t *d, *s;
			d=(uint8_t *)out; s=(uint8_t *)in;
			if(((uint32_t)s & 2) && ((uint32_t)d & 2)){ //special case move 1 halfword and then do the rest
				while((uint32_t)s % 4 && n){ //get to the next word boundary
					*d++=*s++;
					n--;
				}
				while(n>=TBUFFSIZE){
					_Z10copy_wordsPKmPmm((uint32_t *)s, t, TBUFFSIZE/4);
					_Z10copy_wordsPKmPmm(t, (uint32_t *)d, TBUFFSIZE/4);
					d+=TBUFFSIZE;
					s+=TBUFFSIZE;
					n-=TBUFFSIZE;
					routinechecks(1);
				}
				if(n & ~3){
					_Z10copy_wordsPKmPmm((uint32_t *)s, t, n>>2);
					_Z10copy_wordsPKmPmm(t, (uint32_t *)d, n>>2);
					d+=(n & ~3);
					s+=(n & ~3);
					n-=(n & ~3);
				}
				while(n--){
					*d++=*s++;
				}
				routinechecks(1);
			} else {
				uint16_t *dd, *ss;
				dd=(uint16_t *)out; ss=(uint16_t *)in;
				while(n>=64){
					n-=64;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					routinechecks(1);
				}
				while(n & ~3){
					*dd++=*ss++;
					n-=2;
				}
				d=(uint8_t *)dd;
				s=(uint8_t *)ss;
				while(n--){
					*d++=*s++;
				}
			}
		} else { //any byte location for one or both pointers
			char *d, *s;
			d=(char *)out; s=(char *)in;
			if(((uint32_t)s % 4) == ((uint32_t)d % 4)){ //special case same offset into word
				while((uint32_t)s % 4 && n){ //get to the next word boundary
					*d++=*s++;
					n--;
				}
				while(n>=TBUFFSIZE){
					_Z10copy_wordsPKmPmm((uint32_t *)s, t, TBUFFSIZE/4);
					_Z10copy_wordsPKmPmm(t, (uint32_t *)d, TBUFFSIZE/4);
					d+=TBUFFSIZE;
					s+=TBUFFSIZE;
					n-=TBUFFSIZE;
					routinechecks(1);
				}
				if(n & ~3){
					_Z10copy_wordsPKmPmm((uint32_t *)s, t, n>>2);
					_Z10copy_wordsPKmPmm(t, (uint32_t *)d, n>>2);
					d+=(n & (uint32_t)~3);
					s+=(n & ~3);
					n-=(n & ~3);
				}
				while(n--){
					*d++=*s++;
				}
				routinechecks(1);
			} else {
				while(n>=32){
					n-=32;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					routinechecks(1);
				}
				while(n--){
					*d++=*s++;
				}
			}
		}
	} else 	if((uint32_t)out & 0x80000000 || (uint32_t)in & 0x80000000){ //copy is to or from SDRAM
		if(((uint32_t)out & 3)==0 && ((uint32_t)in & 3)==0){ // start points both on word boundaries
			uint8_t *d, *s;
			d=(uint8_t *)out; s=(uint8_t *)in;
			while(n>=MAXCPY){
				_Z10copy_wordsPKmPmm((uint32_t *)s, (uint32_t *)d, MAXCPY/4);
				d+=MAXCPY;
				s+=MAXCPY;
				n-=MAXCPY;
				routinechecks(1);
			}
			if(n & ~3){
				_Z10copy_wordsPKmPmm((uint32_t *)s, (uint32_t *)d, n>>2);
				d+=(n & ~3);
				s+=(n & ~3);
				n-=(n & ~3);
			}
			while(n--){
				*d++=*s++;
			}
			routinechecks(1);
		} else if(((uint32_t)out & 1)==0 && ((uint32_t)in & 1)==0){// start points both on half word boundaries
			uint8_t *d, *s;
			d=(uint8_t *)out; s=(uint8_t *)in;
			if(((uint32_t)s & 2) && ((uint32_t)d & 2)){ //special case move 1 halfword and then do the rest
				while((uint32_t)s % 4 && n){ //get to the next word boundary
					*d++=*s++;
					n--;
				}
				while(n>=MAXCPY){
					_Z10copy_wordsPKmPmm((uint32_t *)s, (uint32_t *)d, MAXCPY/4);
					d+=MAXCPY;
					s+=MAXCPY;
					n-=MAXCPY;
					routinechecks(1);
				}
				if(n & ~3){
					_Z10copy_wordsPKmPmm((uint32_t *)s, (uint32_t *)d, n>>2);
					d+=(n & ~3);
					s+=(n & ~3);
					n-=(n & ~3);
				}
				while(n--){
					*d++=*s++;
				}
				routinechecks(1);
			} else {
				uint16_t *dd, *ss;
				dd=(uint16_t *)out; ss=(uint16_t *)in;
				while(n>=64){
					n-=64;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					*dd++=*ss++;
					routinechecks(1);
				}
				while(n & ~3){
					*dd++=*ss++;
					n-=2;
				}
				d=(uint8_t *)dd;
				s=(uint8_t *)ss;
				while(n--){
					*d++=*s++;
				}
			}
		} else { //any byte location for one or both pointers
			char *d, *s;
			d=(char *)out; s=(char *)in;
			if(((uint32_t)s & 3) == ((uint32_t)d & 3)){ //special case same offset into word
				while((uint32_t)s % 4 && n){ //get to the next word boundary
					*d++=*s++;
					n--;
				}
				while(n>=MAXCPY){
					_Z10copy_wordsPKmPmm((uint32_t *)s, (uint32_t *)d, MAXCPY/4);
					d+=MAXCPY;
					s+=MAXCPY;
					n-=MAXCPY;
					routinechecks(1);
				}
				if(n & ~3){
					_Z10copy_wordsPKmPmm((uint32_t *)s, (uint32_t *)d, n>>2);
					d+=(n & ~3);
					s+=(n & ~3);
					n-=(n & ~3);
				}
				while(n--){
					*d++=*s++;
				}
				routinechecks(1);
			} else {
				while(n>=32){
					n-=32;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					*d++=*s++;
					routinechecks(1);
				}
				while(n--){
					*d++=*s++;
				}
			}
		}
	} else {
		char *d, *s;
		d=out; s=(char *)in;
		if(((uint32_t)s & 3) != ((uint32_t)d & 3)){ //not byte aligned
			while(n>=ASMMAX){
				mycpy(d,s,ASMMAX);
				d+=ASMMAX;
				s+=ASMMAX;
				n-=ASMMAX;
				routinechecks(1);
			}
			if(n)mycpy(d,s,n);
		} else {
			while((uint32_t)s % 4 && n){ //get to the next word boundary
				*d++=*s++;
				n--;
			}
			if(n & ~3){
				_Z10copy_wordsPKmPmm((uint32_t *)s, (uint32_t *)d, n>>2);
				d+=(n & ~3);
				s+=(n & ~3);
				n-=(n & ~3);
			}
			while(n--){
				*d++=*s++;
			}
		}
	routinechecks(1);
	}
//    SCB_CleanDCache_by_Addr((uint32_t *)((uint32_t)out & 0xFFFFFFE0), size+32);

}

void mycopysafe(void *out, const void *in, int n){
	if(n==0 || in==out)return;
	if((uint32_t)out & 0x80000000 || (uint32_t)in & 0x80000000){ //copy is to and/or from SDRAM
		char *d, *s;
		d=(char *)out; s=(char *)in;
		if(((uint32_t)out & 3)==0 && ((uint32_t)in & 3)==0 && (n & 3)==0){
			while(n>=ASMMAX){
				mycpy(d,s,ASMMAX);
				d+=ASMMAX;
				s+=ASMMAX;
				n-=ASMMAX;
			}
			if(n)mycpy(d,s,n);
		} else if(((uint32_t)out & 1)==0 && ((uint32_t)in & 1)==0 && (n & 1)==0){
			short *d, *s;
			n>>=1;
			d=(short *)out; s=(short *)in;
			while(n>=32){
				n-=32;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
			}
			while(n--){
				*d++=*s++;
			}
		} else {
			char *d, *s;
			d=(char *)out; s=(char *)in;
			while(n>=32){
				n-=32;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
				*d++=*s++;
			}
			while(n--){
				*d++=*s++;
			}
		}
	} else {
		char *d, *s;
		d=out; s=(char *)in;
		while(n>=ASMMAX){
			mycpy(d,s,ASMMAX);
			d+=ASMMAX;
			s+=ASMMAX;
			n-=ASMMAX;
		}
		if(n)mycpy(d,s,n);
	}
}

void m_alloc(int size) {
    // every time the variable table is increased this must be called to verify that enough memory is free
    vartbl = (struct s_vartbl *)SRAMBASE;
    memset(vartbl,0,SRAMEND-SRAMBASE);
    funtbl = (struct s_funtbl *)FUNBASE;
    mymemset(funtbl,0,sizeof(struct s_funtbl)*MAXSUBFUN);
    /*    VarTableTop = (unsigned char *)vartbl + MRoundUp(size);
    if(SBitsGet(VarTableTop) & PUSED) {
        LocalIndex = 0;
        ClearTempMemory();                                          // hopefully this will give us enough memory to print the prompt
        error("Not enough variable memory");
    }*/
}


// Get a temporary buffer of any size, returns a pointer to the buffer
// The space only lasts for the length of the command or in the case of a sub/fun until it has exited.
// A pointer to the space is also saved in strtmp[] so that the memory can be automatically freed at the end of the command
// StrTmpLocalIndex[] is used to track the sub/fun nesting level at which it was created
void *GetTempMemory(int NbrBytes) {
    if(StrTmpIndex >= MAXTEMPSTRINGS) error("Not enough temporary memory");
    StrTmpLocalIndex[StrTmpIndex] = LocalIndex;
    StrTmp[StrTmpIndex] = (NbrBytes > 8192 ? GetMemory(NbrBytes) :GetInternalMemory(NbrBytes));
   // StrTmp[StrTmpIndex] = (NbrBytes > 8192 ? GetMemory(NbrBytes) :GetSystemMemory(NbrBytes)); //Get from bottom up

    TempMemoryIsChanged = true;
    return (void *)StrTmp[StrTmpIndex++];
}


// get a temporary string buffer
// this is used by many BASIC string functions.  The space only lasts for the length of the command.
void *GetTempStrMemory(void) {
    if(StrTmpIndex >= MAXTEMPSTRINGS) error("Not enough temporary string memory");
    StrTmpLocalIndex[StrTmpIndex] = LocalIndex;
    StrTmp[StrTmpIndex] = GetStringMemory();
    TempMemoryIsChanged = true;
    return (void *)StrTmp[StrTmpIndex++];
}

// clear any temporary string spaces (these last for just the life of a command) and return the memory to the heap
// this will not clear memory allocated with a local index less than LocalIndex, sub/funs will increment LocalIndex
// and this prevents the automatic use of ClearTempMemory from clearing memory allocated before calling the sub/fun
void ClearTempMemory(void) {
    TempMemoryIsChanged = false;
    while(StrTmpIndex > 0) {
        if(StrTmpLocalIndex[StrTmpIndex - 1] >= LocalIndex) {
            StrTmpIndex--;
            FreeMemory((char *)StrTmp[StrTmpIndex]);
            StrTmp[StrTmpIndex] = NULL;
        } else
            break;
    }
}



void ClearSpecificTempMemory(void *addr) {
    int i;
    for(i = 0; i < StrTmpIndex; i++) {
        if(StrTmp[i] == addr) {
            FreeMemorySafe((void *)&addr);
            StrTmp[i] = NULL;
            StrTmpIndex--;
            while(i < StrTmpIndex) {
                StrTmp[i] = StrTmp[i + 1];
                StrTmpLocalIndex[i] = StrTmpLocalIndex[i + 1];
                i++;
            }
            return;
        }
    }
}


int MemSize(void *addr){ //returns the amount of heap memory allocated to an address
    int i=0;
    int bits;
        if(addr >= (void *)RAMBASE && addr < (void *)RAMEND){
            do {
            	bits = MBitsGet(addr);
            	addr += RAMPAGESIZE;
            	i+=RAMPAGESIZE;
            } while(bits != (PUSED | PLAST));
        } else if(addr >= (void *)SDRAMBASE && addr < (void *)SDRAMEND){
            do {
            	bits = SDBitsGet(addr);
            	addr += RAMPAGESIZE;
            	i+=RAMPAGESIZE;
            } while(bits != (PUSED | PLAST));
        }
    return i;
}

void FreeSDMemory(void *addr) {
    int bits;
    do {
        bits = SDBitsGet(addr);
        SDBitsSet(addr, 0);
        addr += RAMPAGESIZE;
    } while(bits != (PUSED | PLAST));
}

void FreeOMemory(void *addr) {
    int bits;
    do {
        bits = MBitsGet(addr);
        MBitsSet(addr, 0);
        addr += RAMPAGESIZE;
    } while(bits != (PUSED | PLAST));
}

void FreeMemory(void *addr) {
        if(addr >= (void *)RAMBASE && addr < (void *)RAMEND) {FreeOMemory(addr);return;}
        if(addr >= (void *)SDRAMBASE && addr < (void *)SDRAMEND){FreeSDMemory(addr);return;}
}
void FreeMemorySafe(void **addr){
	if(*addr!=NULL){
        if(*addr >= (void *)RAMBASE && *addr < (void *)RAMEND) {FreeOMemory(*addr);*addr=NULL;}
        if(*addr >= (void *)SDRAMBASE && *addr < (void *)SDRAMEND){FreeSDMemory(*addr);*addr=NULL;}
	}
}


void InitHeap(void) {
    int i;
    memset(SDmap,0x00,0x8000);
    memset(mmap,0x00,sizeof(mmap));
    for(i = 0; i < MAXTEMPSTRINGS; i++) StrTmp[i] = NULL;
    MBitsSet((unsigned char *)RAMEND, PUSED | PLAST);
//    SBitsSet((unsigned char *)SRAMEND, PUSED | PLAST);
    SDBitsSet((unsigned char *)SDRAMEND, PUSED | PLAST);
    StrTmpIndex = TempMemoryIsChanged = 0;
    minMMheap=(void *)SDRAMEND;
    m_alloc(0);
}

void *ReAllocMemory(void *addr, size_t msize){
	int size=MemSize(addr);
	if(msize<=size)return addr;
	void *newaddr=GetMemory(msize);
	if(addr!=NULL && size!=0){
//		MMPrintString("Increase SDRAM Heap ");PInt(size);PIntComma(msize);PRet();
		mycopysafe(newaddr,addr,MemSize(addr));
		FreeMemorySafe((void *)&addr);

	}
	return newaddr;
}
void *ReAllocInternalMemory(void *addr, size_t msize){
	int size=MemSize(addr);
	if(msize<=size)return addr;
	void *newaddr=GetInternalMemory(msize);
	if(addr!=NULL && size!=0){
//		MMPrintString("Increase Internal Heap ");PInt(size);PIntComma(msize);PRet();MM_Delay(1000);
		mycopysafe(newaddr,addr,MemSize(addr));
		FreeMemorySafe((void *)&addr);

	}
	return newaddr;
}

/***********************************************************************************************************************
 Private memory management functions
************************************************************************************************************************/


unsigned int MBitsGet(void *addr) {
    unsigned int i, *p;
    addr = (void *)((uint32_t)addr - (uint32_t)RAMBASE);
    p = (void *)&mmap[((unsigned int)addr/RAMPAGESIZE) / PAGESPERWORD];        // point to the word in the memory map
    i = ((((unsigned int)addr/RAMPAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS; // get the position of the bits in the word
    return (*p >> i) & ((1 << PAGEBITS) -1);
}


unsigned int SDBitsGet(void *addr) {
    unsigned int i, *p;
    addr = (void *)((uint32_t)addr - (uint32_t)SDRAMBASE);
    p = (void *)&SDmap[((unsigned int)addr/RAMPAGESIZE) / PAGESPERWORD];        // point to the word in the memory map
    i = ((((unsigned int)addr/RAMPAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS; // get the position of the bits in the word
    return (*p >> i) & ((1 << PAGEBITS) -1);
}

void MBitsSet(void *addr, int bits) {
    unsigned int i, *p;
    addr = (void *)((uint32_t)addr - (uint32_t)RAMBASE);
    p = (void *)&mmap[((unsigned int)addr/RAMPAGESIZE) / PAGESPERWORD];        // point to the word in the memory map
    i = ((((unsigned int)addr/RAMPAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS; // get the position of the bits in the word
    *p = (bits << i) | (*p & (~(((1 << PAGEBITS) -1) << i)));
}

void SDBitsSet(void *addr, int bits) {
    unsigned int i, *p;
    addr = (void *)((uint32_t)addr - (uint32_t)SDRAMBASE);
    p = (void *)&SDmap[((unsigned int)addr/RAMPAGESIZE) / PAGESPERWORD];        // point to the word in the memory map
    i = ((((unsigned int)addr/RAMPAGESIZE)) & (PAGESPERWORD - 1)) * PAGEBITS; // get the position of the bits in the word
    *p = (bits << i) | (*p & (~(((1 << PAGEBITS) -1) << i)));
}

void *GetMemory(size_t size){
    uint64_t *i;
    unsigned int j, n, k;
    unsigned char *addr;
    if(!size)size=1;
    j = n = k = (size + RAMPAGESIZE - 1)/RAMPAGESIZE;                         // nbr of pages rounded up
    for(addr = (unsigned char *)SDRAMEND -  RAMPAGESIZE; addr > (unsigned char *)SDRAMBASE; addr -= RAMPAGESIZE) {
        if(!(SDBitsGet(addr) & PUSED)) {
            if(--n == 0) {                                          // found a free slot
                j--;
                SDBitsSet(addr + (j * RAMPAGESIZE), PUSED | PLAST);     // show that this is used and the last in the chain of pages
                while(j--) SDBitsSet(addr + (j * RAMPAGESIZE), PUSED);  // set the other pages to show that they are used
                i=(uint64_t *)addr;
                while(k--){
                	i=clearpage((void *)i);
                }
                SCB_CleanInvalidateDCache_by_Addr((uint32_t *)addr, size);
                if((uint32_t)addr<(uint32_t)minMMheap)minMMheap=(void *)addr;
                return (void *)addr;
            }
        } else
            n = j;                                                  // not enough space here so reset our count
    }
    // out of memory
    LocalIndex = 0;
    error("Not enough heap memory");
    return NULL;                                                    // keep the compiler happy
}
/*
void *GetInternalMemory(size_t size){
    uint64_t *i;
    unsigned int j, n, k;
    unsigned char *addr;
    if(!size)size=1;
    j = n = k = (size + RAMPAGESIZE - 1)/RAMPAGESIZE;                         // nbr of pages rounded up
    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBASE; addr -= RAMPAGESIZE) {
        if(!(MBitsGet(addr) & PUSED)) {
            if(--n == 0) {                                          // found a free slot
                j--;
                MBitsSet(addr + (j * RAMPAGESIZE), PUSED | PLAST);     // show that this is used and the last in the chain of pages
                while(j--) MBitsSet(addr + (j * RAMPAGESIZE), PUSED);  // set the other pages to show that they are used
                i=(uint64_t *)addr;
                while(k--){
                	i=clearpage((void *)i);
                }
                SCB_CleanInvalidateDCache_by_Addr((uint32_t *)addr, size);
                return (void *)addr;
            }
        } else
            n = j;                                                  // not enough space here so reset our count
    }
    // out of memory
    LocalIndex = 0;
    error("Not enough internal memory");
    return NULL;                                                    // keep the compiler happy
}
*/

// Performance enhancement from Picomites.
// gets memory from the bottom up, does not need to jump any allocated variables.
void *GetInternalMemory(size_t size) {
	//uint64_t *i;
    int n=0, k;
    unsigned char *addr;
   // TestStackOverflow();
    k= (size + RAMPAGESIZE - 1)/RAMPAGESIZE;                         // nbr of pages rounded up
    for(addr = (unsigned char *)RAMBASE; addr < (unsigned char *)RAMEND - RAMPAGESIZE; addr += RAMPAGESIZE) {
        if(!(MBitsGet(addr) & PUSED)) {
            if(++n == k) {                                          // found a free slot
                k--;
                //i=(uint64_t *)addr;
                MBitsSet(addr , PUSED | PLAST);     // show that this is used and the last in the chain of pages
                  while(k--){
                    addr-=RAMPAGESIZE;
                    MBitsSet(addr,PUSED);
                  }
                mymemset(addr , 0, size);                              // zero the memory
                SCB_CleanInvalidateDCache_by_Addr((uint32_t *)addr, size);
                return (void *)addr;
            }
        } else n = 0;                                               // not enough space here so reset our count
    }
    LocalIndex = 0;                                                  // Allows ClearTempMemory to clear all levels
    ClearTempMemory();                                               // hopefully this will give us enough to print the prompt
    error("Not enough internal memory");
    return NULL;                                                    // keep the compiler happy
}

// Performance enhancement from Picomites.
// gets memory from the bottom up, does not need to jump any allocated variables.
/*
void *GetSystemMemory(int size) {
	int n=0, k;
    unsigned char *addr;
   // TestStackOverflow();
    k= (size + RAMPAGESIZE - 1)/RAMPAGESIZE;                         // nbr of pages rounded up
    for(addr = (unsigned char *)RAMBASE; addr < (unsigned char *)RAMEND - RAMPAGESIZE; addr += RAMPAGESIZE) {
        if(!(MBitsGet(addr) & PUSED)) {
            if(++n == k) {                                          // found a free slot
                k--;
                MBitsSet(addr , PUSED | PLAST);     // show that this is used and the last in the chain of pages
                  while(k--){
                	addr-=RAMPAGESIZE;
                    MBitsSet(addr,PUSED);
                  }
                mymemset(addr , 0, size);                              // zero the memory
                return (void *)addr;
            }
        } else n = 0;                                               // not enough space here so reset our count
    }
    LocalIndex = 0;                                                  // Allows ClearTempMemory to clear all levels
    ClearTempMemory();                                               // hopefully this will give us enough to print the prompt
    error("Not enough internal memory");
    return NULL;                                                    // keep the compiler happy
}
*/

//Performance enhancement from Picomites.
//Get temporary memory from Bottom of the HEAP
void *GetStringMemory(void) { //routine to get a single page of memory from Bottom UP !!!
    uint64_t *i;
    unsigned char *addr;
   // for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBASE; addr -= RAMPAGESIZE) {
   	for(addr = (unsigned char *)RAMBASE; addr < (unsigned char *)RAMEND - RAMPAGESIZE; addr += RAMPAGESIZE) {
        if(!(MBitsGet(addr) & PUSED)) {
            MBitsSet(addr , PUSED | PLAST);     // show that this is used and the last in the chain of pages
            i=(uint64_t *)addr;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
            return (void *)addr;
        }
    }
    LocalIndex = 0;
    error("Not enough String memory");
    return NULL;                                                    // keep the compiler happy
}

//Performance enhancement from Picomites.
// Put variable data at the Bottom of the heap,so GetInternalMemory search from Top does not need to jump over it.
void *GetStringMemoryBottom(void) { //routine to get a single page of memory from Top - Used for Variables
    uint64_t *i;
    unsigned char *addr;
    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBASE; addr -= RAMPAGESIZE) {
        if(!(MBitsGet(addr) & PUSED)) {
            MBitsSet(addr , PUSED | PLAST);     // show that this is used and the last in the chain of pages
            i=(uint64_t *)addr;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
        	*i++ = 0;
            return (void *)addr;
        }
    }
    LocalIndex = 0;
    error("Not enough String memory");
    return NULL;                                                    // keep the compiler happy
}

uint32_t MinHeap(void){
	unsigned char *addr;
    for(addr = (unsigned char *)SDRAMBASE ; addr < (unsigned char *)SDRAMEND; addr += RAMPAGESIZE)
        if((SDBitsGet(addr) & PUSED)) break;
    return (uint32_t)addr-RAMPAGESIZE;
}
int FreeSpaceOnHeap(void) {
    unsigned int nbr;
    unsigned char *addr;
    nbr = 0;
//    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBASE; addr -= RAMPAGESIZE)
//        if(!(MBitsGet(addr) & PUSED)) nbr++;
    for(addr = (unsigned char *)SDRAMEND -  RAMPAGESIZE; addr > (unsigned char *)SDRAMBASE; addr -= RAMPAGESIZE)
        if(!(SDBitsGet(addr) & PUSED)) nbr++;
    return nbr * RAMPAGESIZE;
}


unsigned int UsedHeap(void) {
    unsigned int nbr;
    unsigned char *addr;
    nbr = 0;
//    for(addr = (unsigned char *)RAMEND -  RAMPAGESIZE; addr > (unsigned char *)RAMBASE; addr -= RAMPAGESIZE)
//        if((MBitsGet(addr) & PUSED)) nbr++;
    for(addr = (unsigned char *)SDRAMEND -  RAMPAGESIZE; addr > (unsigned char *)SDRAMBASE; addr -= RAMPAGESIZE)
        if((SDBitsGet(addr) & PUSED)) nbr++;
    return nbr * RAMPAGESIZE;
}





