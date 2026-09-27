/***************************************************************************
CMM2 MMBasic
Memory.h

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

*******************************************************************************/



#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef MEMORY_HEADER
#define MEMORY_HEADER

extern volatile char *StrTmp[];                                              // used to track temporary string space on the heap
extern int TempMemoryTop;                                           // this is the last index used for allocating temp memory
extern int TempMemoryIsChanged;						                // used to prevent unnecessary scanning of strtmp[]

typedef enum _M_Req {M_PROG, M_VAR} M_Req;

extern void m_alloc(int size);
//extern void *GetMemory(size_t msize);
extern void *GetTempMemory(int NbrBytes);
extern void *GetTempStrMemory(void);
extern void *GetMemory(size_t NbrBytes);
extern void *GetInternalMemory(size_t NbrBytes);
//extern void *GetSystemMemory(int  msize);
extern void ClearTempMemory(void);
extern void ClearSpecificTempMemory(void *addr);
extern void FreeMemory(void *addr);
extern void *GetFastTempMemory(size_t size);//extern void *GetSMemory(size_t msize);
extern void InitHeap(void);
extern char *HeapBottom(void);
extern int FreeSpaceOnHeap(void);
extern int FreeSpaceOnSHeap(void);
extern void *ReAllocMemory(void *addr, size_t msize);
extern void *GetStringMemory(void);
extern void *GetStringMemoryBottom(void);
//extern void *SDReAllocMemory(void *addr, size_t msize);
extern void FreeMemorySafe(void **addr);
extern void mycopysafe(void *out, const void *in, int n);
extern void *ReAllocInternalMemory(void *addr, size_t msize);
extern int G1Hardware;
//extern unsigned int _stack;
//extern unsigned int _splim;
//extern unsigned int _heap;
//extern unsigned int _min_stack_size;
//extern unsigned int _text_begin;
extern int MemSize(void *addr);
//extern int SDMemSize(void *addr);
extern void FreeMemorySafe(void **addr);
extern unsigned int MBitsGet(void *addr);
extern unsigned int SDBitsGet(void *addr);
extern uint32_t MinHeap(void);
extern void *minMMheap;
//extern int FreeSpaceOnSDHeap(void);
// RAM parameters
// ==============
// The following settings will allow MMBasic to use all the free memory on the STM32.  If you need some RAM for
// other purposes you can declare the space needed as a static variable -or- allocate space to the heap (which
// will reduce the memory available to MMBasic) -or- change the definition of RAMEND.
// NOTE: MMBasic does not use the heap.  It has its own heap which is allocated out of its own memory space.
extern uint32_t RAMBASE;
#define RAMEND          	0x30040000
#define FUNBASE				0x30040000
#define FUNEND				FUNBASE+sizeof(struct s_funtbl)*(MAXSUBFUN+1)
#define SRAMBASE         	0x38000000
#define SRAMEND         	0x38010000
#define SDRAMBASE         	(G1Hardware ? ((Option.ProgramStartCode<0 || Option.profile) ? 0xD0380000: 0xD0300000) : 0xD0800000)
#define SDRAMEND         	(G1Hardware ? 0xD0800000-0x100 : 0xD2000000-0x100)
// other (minor) memory management parameters
#define RAMPAGESIZE        256                                         // the allocation granuality
#define SDRAMPAGESIZE        256                                      // the allocation granuality
#define PAGEBITS        2                                           // nbr of status bits per page of allocated memory, must be a power of 2

#define PUSED           0b01                                        // flag that indicates that the page is in use
#define PLAST           0b10                                        // flag to show that this is the last page in a single allocation

#define PAGESPERWORD    ((sizeof(unsigned int) * 8)/PAGEBITS)
#define MRoundUp(a)     (((a) + (RAMPAGESIZE - 1)) & (~(RAMPAGESIZE - 1)))// round up to the nearest page size
#define TBUFFSIZE	3200
#define LBUFFSIZE	6400
#define LBUFFSTART FUNEND+TBUFFSIZE+0x100
#define LBUFFEND LBUFFSTART+LBUFFSIZE
#endif
#endif

