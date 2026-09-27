/***************************************************************************
CMM2 MMBasic
MMBasic.h

Include file that contains the globals and defines for MMBasic.c in MMBasic.

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

#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "Configuration.h"

/* ============================================================================
 * Type definitions for data items
 * Used in tokens, variables and arguments to functions
 * ============================================================================ */
#define T_NOTYPE       0                            // type not set or discovered
#define T_NBR       0x01                            // number (or float) type
#define T_STR       0x02                            // string type
#define T_INT       0x04                            // 64 bit integer type
#define T_PTR       0x08                            // the variable points to another variable's data
#define T_IMPLIED   0x10                            // the variables type does not have to be specified with a suffix
#define T_CONST     0x20                            // the contents of this variable cannot be changed
#define T_BLOCKED   0x40                            // Hash table entry blocked after ERASE
#ifdef STRUCTENABLED
#define T_STRUCT 0x80 // variable is a structure type
#define TypeMask(a) ((a) & (T_NBR | T_INT | T_STR | T_STRUCT))
#else
#define TypeMask(a) ((a) & (T_NBR | T_INT | T_STR)) // macro to isolate the variable type bits
#endif

// namelen field in vartbl is used exclusively for flag bits (length no longer stored)
#define NAMELEN_EXPLICIT 0x80 // Bit 7: variable had explicit type suffix ($, %, !)
#define NAMELEN_STATIC 0x40   // Bit 6: static variable (dot in name is naming convention)

/* ============================================================================
 * Token types - These are or'ed with the data types above to fully define a token
 * ============================================================================ */
#define T_INV       0                               // an invalid token
#define T_NA        0                               // an invalid token
#define T_CMD       0x10                            // a command
#define T_OPER      0x20                            // an operator
#define T_FUN       0x40                            // a function (also used for a function that can operate as a command)
#define T_FNA       0x80                            // a function that has no arguments

#define C_BASETOKEN 0x80                            // the base of the token numbers

/* ============================================================================
 * Program line flags
 * ============================================================================ */
#define T_CMDEND    0                               // end of a command
#define T_NEWLINE   1                               // Single byte indicating the start of a new line
#define T_LINENBR   2                               // three bytes for a line number
#define T_LABEL     4                               // variable length indicating a label

#define E_END       255                             // dummy last operator in an expression

/* ============================================================================
 * Variable finding flags (used in findvar() function) they should be or'd together
 * ============================================================================ */
#define V_FIND              0x0000                    // a straight forward find, if the variable is not found it is created and set to zero
#define V_NOFIND_ERR        0x0200                    // throw an error if not found
#define V_NOFIND_NULL       0x0400                    // return a null pointer if not found
#define V_DIM_VAR           0x0800                    // dimension an array
#define V_LOCAL             0x1000                    // create a local variable
#define V_EMPTY_OK          0x2000                    // allow an empty array variable.  ie, var()
#define V_FUNCT             0x4000                    // we are defining the name of a function

/* ============================================================================
 * Struct member array protection macro
 * Use after findvar() to error if struct member arrays are used where not supported
 * ============================================================================ */
#ifdef STRUCTENABLED
#define CHECK_STRUCT_MEMBER_ARRAY()                                              \
    do                                                                           \
    {                                                                            \
        if ((vartbl[g_VarIndex].type & T_STRUCT) && g_StructMemberType != 0 && \
            vartbl[VarIndex].dims[0] > 0)                                    \
            StandardError(47);                                                   \
    } while (0)
#else
#define CHECK_STRUCT_MEMBER_ARRAY() ((void)0)
#endif

/* ============================================================================
 * Expression evaluation flags - used in the last argument in expression()
 * ============================================================================ */
#define E_NOERROR           true
#define E_ERROR             0
#define E_DONE_GETVAL       0b10

// this flag is used to signal that automatic precision is to be used in FloatToStr()
#define STR_AUTO_PRECISION  999

/* ============================================================================
     * Type definitions
  ============================================================================ */
#ifdef CMD16BIT
    typedef uint16_t CommandToken;   //16 bit cmd tokens
#else
    typedef char CommandToken;    //8 bit cmd tokens
#endif

    /* Variable table structure */
typedef struct s_vartbl {                               // structure of the variable table
	char name[MAXVARLEN];                       // variable's name
    char type;                                  // its type (T_NUM, T_INT or T_STR)
    char level;                                 // its subroutine or function level (used to track local variables)
    unsigned char size;                         // the number of chars to allocate for each element in a string array
    char namelen;                               // Used if STRUCTENABLED
    int __attribute__ ((aligned (4))) dims[MAXDIM];                     // the dimensions. it is an array if the first dimension is NOT zero
    union u_val{
        MMFLOAT f;                              // the value if it is a float
        long long int i;                        // the value if it is an integer
        MMFLOAT *fa;                            // pointer to the allocated memory if it is an array of floats
        long long int *ia;                      // pointer to the allocated memory if it is an array of integers
        char *s;                                // pointer to the allocated memory if it is a string
    }  __attribute__ ((aligned (8))) val;
} __attribute__ ((aligned (8))) xval;

#ifdef STRUCTENABLED
    /* Structure member definition */
    typedef struct s_structmember
    {
        unsigned char name[MAXVARLEN]; // Member name
        unsigned char type;            // T_NBR, T_STR, T_INT, or T_STRUCT
        unsigned char size;            // For strings: max length; for nested struct: struct index
        int offset;                    // Byte offset within structure
        short dims[MAXDIM];            // Array dimensions (0 = not an array)
    } structmember_val;

    /* Structure type definition */
    typedef struct s_structdef
    {
        unsigned char name[MAXVARLEN];                     // Structure type name
        int num_members;                                   // Number of members
        struct s_structmember members[MAX_STRUCT_MEMBERS]; // Member definitions
        int total_size;                                    // Total size in bytes
    } structdef_val;
#endif

struct s_funtbl {
	char name[MAXVARLEN];                       // variable's name
	uint32_t index;
};

extern struct s_funtbl *funtbl;
extern struct s_vartbl *vartbl;

#ifdef STRUCTENABLED
    extern struct s_structdef *g_structtbl[MAX_STRUCT_TYPES]; // Array of pointers, allocated per-type
    extern int g_structcnt;
    extern int g_StructArg;          // Struct index for pending DIM AS structtype
    extern int g_StructMemberType;   // Type of struct member being accessed (0 if not a member access)
    extern int g_StructMemberOffset; // Offset of member within struct (for EXTRACT/INSERT/SORT)
    extern int g_StructMemberSize;   // Size of the member (for EXTRACT/INSERT/SORT)
    extern int g_ExprStructType;     // Struct type index from expression evaluation
#endif

extern int varcnt;
extern int Localvarcnt;                              // number of LOCAL variables defined (eg, largest index into the variable table)
extern int Globalvarcnt;                              // number of GLOBAL variables defined (eg, largest index into the variable table)
extern int VarIndex;                            // index of the current variable.  set after the findvar() function has found/created a variable
extern int LocalIndex;                          // used to track the level of local variables

extern int OptionBase;                          // value of OPTION BASE
extern char OptionExplicit,OptionEscape ;       // true if OPTION EXPLICIT has been used
extern char DefaultType;                        // the default type if a variable is not specifically typed
extern char syscheck;
extern int emptyarray;

//#if !defined(BOOL_ALREADY_DEFINED)
//    #define BOOL_ALREADY_DEFINED
//    typedef enum _BOOL { FALSE = 0, TRUE } BOOL;    // Undefined size
//#endif

#ifndef true
    #define true        1
#endif

#ifndef false
    #define false       0
#endif

#define MAXLINENBR          65001                                   // maximim acceptable line number

// skip whitespace
// finishes with x pointing to the next non space char
#define skipspace(x)    while(*x == ' ') x++

// skip to the next element
// finishes pointing to the zero char that preceeds an element
#define skipelement(x)  while(*x) x++

// skip to the next line
// skips text and and element separators until it is pointing to the zero char marking the start of a new line.
// the next byte will be either the newline token or zero char if end of program
#define skipline(x)     while(!(x[-1] == 0 && (x[0] == T_NEWLINE || x[0] == 0)))x++
extern const char namestart[], namein[], nameend[];
// find a token
// finishes pointing to the token or zero char if not found in the line
#define findtoken(x)    while(*x != (tkn) && *x)x++
#define IsDigitinline(a)	( a >= '0' && a <= '9' )

#define isnamestart(c)  (namestart[(uint8_t)c])                    // true if valid start of a variable name
#define isnamechar(c)   (namein[(uint8_t)c])                       // true if valid part of a variable name  (0x1e uadded fior structures)
#define isnameend(c)    (nameend[(uint8_t)c])                      // true if valid at the end of a variable name

//#define isnamestart(c)  (((c & 0xDF)>='A' && (c & 0xDF)<='Z') || c == '_')         // true if valid start of a variable name
//#define isnamechar(c)   (isnamestart(c) || (c>=48 && c<=57) || c == '.')           // true if valid part of a variable name
//#define isnameend(c)    (isnamechar(c) || c == '$' || c == '!' || c == '%')        // true if valid at the end of a variable name

#define tokentype(i)    ((i >= C_BASETOKEN && i < TokenTableSize - 1 + C_BASETOKEN) ? (tokentbl[i - C_BASETOKEN].type) : 0)                     // get the type of a token
#define tokenfunction(i)((i >= C_BASETOKEN && i < TokenTableSize - 1 + C_BASETOKEN) ? (tokentbl[i - C_BASETOKEN].fptr) : (tokentbl[0].fptr))    // get the function pointer  of a token
#define tokenname(i)    ((i >= C_BASETOKEN && i < TokenTableSize - 1 + C_BASETOKEN) ? (char *)(tokentbl[i - C_BASETOKEN].name) : "")            // get the name of a token

#ifdef CMD16BIT
#define commandfunction(i)((i < CommandTableSize - 1) ? (commandtbl[i].fptr) : (commandtbl[0].fptr))    // get the function pointer  of a token
#define commandname(i)  ((i < CommandTableSize - 1 ) ? (char *)(commandtbl[i].name) : "")        // get the name of a command

#else

#define commandtype(i)  ((i >= C_BASETOKEN && i < CommandTableSize - 1 + C_BASETOKEN) ? (commandtbl[i - C_BASETOKEN].type) : 0)             // get the type of a token
#define commandfunction(i)((i >= C_BASETOKEN && i < CommandTableSize - 1 + C_BASETOKEN) ? (commandtbl[i - C_BASETOKEN].fptr) : (commandtbl[0].fptr))    // get the function pointer  of a token
#define commandname(i)  ((i >= C_BASETOKEN && i < CommandTableSize - 1 + C_BASETOKEN) ? (char *)(commandtbl[i - C_BASETOKEN].name) : "")        // get the name of a command

#endif
// this macro will allocate temporary memory space and build an argument table in it
// x = pointer to the basic text to be split up (char *)
// y = maximum number of args (will throw an error if exceeded) (int)
// s = a string of characters to be used in detecting where to split the text (char *)
#define getargs(x, y, s) char argbuf[STRINGSIZE + STRINGSIZE/2]; char *argv[y]; int argc; makeargs(x, y, argbuf, argv, &argc, s)
#define getcsargs(x, y)                                \
    char argbuf[STRINGSIZE + STRINGSIZE / 2]; \
    char *argv[y];                            \
    int argc;                                          \
    makeargs(x, y, argbuf, argv, &argc, ( char *)",")

extern int CommandTableSize, TokenTableSize;

extern volatile int MMAbort;
extern jmp_buf mark;                            // longjump to recover from an error
extern char BreakKey;                           // console break key (defaults to CTRL-C)
extern jmp_buf run;                            // longjump to recover from an error

extern int ProgMemSize;

extern int NextData;                            // used to track the next item to read in DATA & READ stmts
extern char *NextDataLine;                      // used to track the next line to read in DATA & READ stmts
extern char *CurrentLinePtr;                    // pointer to the current line being executed
extern char *ContinuePoint;                     // Where to continue from if using the continue statement

extern char inpbuf[];                           // used to store user keystrokes until we have a line
extern char tknbuf[];                           // used to store the tokenised representation of the users input line
extern char lastcmd[];                          // used to store the command history in case the user uses the up arrow at the command prompt

extern MMFLOAT farg1, farg2, fret;              // Global floating point variables used by operators
extern long long int iarg1, iarg2, iret;        // Global integer variables used by operators
extern char *sarg1, *sarg2, *sret;              // Global string pointers used by operators
extern int targ;                                // Global type of argument (string or MMFLOAT) returned by an operator

extern int cmdtoken;                            // Token number of the command
extern char *cmdline;                           // Command line terminated with a zero char and trimmed of spaces
extern char *nextstmt;                          // Pointer to the next statement to be executed.
extern char *ep;                                // Pointer to the argument to a function

extern int OptionErrorSkip;                    // value of OPTION ERROR
extern int MMerrno;
extern char *MMErrMsg;


extern char *subfun[];                          // Table of subroutines and functions built when the program starts running
extern char __attribute__ ((aligned (4))) CurrentSubFunName[MAXVARLEN + 1];   // the name of the current sub or fun
extern char CurrentInterruptName[MAXVARLEN + 1];// the name of the current interrupt function
struct s_tokentbl {                             // structure of the token table
	char name[16];                                 // the string (eg, PRINT, FOR, ASC(, etc)
    char type;                                  // the type returned (T_NBR, T_STR, T_INT)
    char precedence;                            // precedence used by operators only.  operators with equal precedence are processed left to right.
    void (*fptr)(void);                         // pointer to the function that will interpret that token
};
/* Hash table structure */
struct s_hash {                             // structure of the token table
	short hash;                                 // the string (eg, PRINT, FOR, ASC(, etc)
    short level;                                  // the type returned (T_NBR, T_STR, T_INT)
};
extern const struct s_tokentbl tokentbl[];
extern const struct s_tokentbl commandtbl[];

// used for the trace function
#define TRACE_BUFF_SIZE  128 //must be a multiple of 2
extern int TraceOn;
extern char *TraceBuff[TRACE_BUFF_SIZE];
extern int TraceBuffIndex;

// used to store commonly used tokens for faster token checking
extern char tokenTHEN, tokenELSE, tokenGOTO, tokenEQUAL, tokenTO, tokenSTEP, tokenWHILE, tokenUNTIL, tokenGOSUB, tokenAS, tokenFOR;
#ifndef CMD16BIT
extern char cmdIF, cmdENDIF, /*cmdEND_IF, */cmdELSEIF, /*cmdELSE_IF, */cmdELSE, cmdLOOP, cmdWHILE, cmdFOR, cmdNEXT, cmdWHILE, cmdENDSUB, cmdLOCAL, cmdSTATIC, cmdENDFUNCTION, cmdDO, cmdSELECT_CASE, cmdCASE, cmdCASE_ELSE, cmdEND_SELECT;
extern char cmdSUB, cmdFUN, cmdCSUB, cmdIRET;
#else
extern unsigned short cmdIF, cmdENDIF, /*cmdEND_IF, */cmdELSEIF, /*cmdELSE_IF, */cmdELSE, cmdLOOP, cmdWHILE, cmdFOR, cmdNEXT, cmdWHILE, cmdENDSUB, cmdLOCAL, cmdSTATIC, cmdENDFUNCTION, cmdDO, cmdSELECT_CASE, cmdCASE, cmdCASE_ELSE, cmdEND_SELECT;
extern unsigned short cmdSUB, cmdFUN, cmdCSUB,cmdCFUN,cmdIRET;
#endif
#ifdef STRUCTENABLED
    extern unsigned short cmdTYPE, cmdEND_TYPE;
#endif

// void error(char *msg) ;
void error(char *, ...);
/* ============================================================================
 * Function declarations - Error handling ( From picomites)
 * ============================================================================ */
///void error(char *msg, ...);
void SyntaxError(void);
void StandardError(int n);
void StandardErrorParam(int n, int m);
void StandardErrorParam2(int n, int m, int l);
void StandardErrorParam3(int n, int m, int l, int h);
void StandardErrorParamS(int n, char *m);


void MIPS16 InitBasic(void);
int FloatToInt32(MMFLOAT);
long long int FloatToInt64(MMFLOAT x);
void makeargs(char **tp, int maxargs, char *argbuf, char *argv[], int *argc, char *delim);
void *findvar(char *, int);
void erasearray(char *n);
void MIPS16 ClearVars(int level);
void MIPS16 ClearStack(void);
void MIPS16 ClearRuntime(void);
void MIPS16 ClearProgram(void);
void *DoExpression(char *p, int *t);
char *evaluate(char *p, MMFLOAT *fa, long long int *ia, char **sa, int *ta, int noerror);
char *doexpr(char *p, MMFLOAT *fa, long long int *ia, char **sa, int *oo, int *t);
MMFLOAT getnumber(char *p);
long long int getinteger(char *p);
//int getint(char *p, int min, int max);
long long int getint(char *p, long long int min, long long int max);
char *getstring(char *p);
void MIPS16 tokenise(int console);
void ExecuteProgram(char *);
//void AddProgramLine(int append);
char *findline(int, int);
char *findlabel(char *labelptr);
char *skipvar(char *p, int noerror);
char *skipexpression(char *p);
char *GetNextCommand(char *p, char **CLine, char *EOFMsg);
int FunctionType(char *p);
char *getclosebracket(char *p);
void makeupper(char *p);
void checkend(char *p);
int GetCommandValue(char *n);
int GetTokenValue(char *n);
char *checkstring(char *p, char *tkn);
char *MtoC(char *p);
char *CtoM(char *p);
void Mstrcpy(char *dest, char *src);
void Mstrcat(char *dest, char *src);
int Mstrcmp(char *s1, char *s2);
char *getCstring(char *p);
char *getFstring(char *p);
void InsertLastcmd(char *s);
int MIPS16 TraceLines(char *target);
void DefinedSubFun(int iscmd, char *cmd, int index, MMFLOAT *fa, long long int *i64, char **sa, int *t);
int FindSubFun(char *p, int type);
void MIPS16 PrepareProgram(int);
void MMPrintString(char* s);
void MMfputs(char *p, int filenbr);
void IntToStrPad(char *p, long long int nbr, signed char padch, int maxch, int radix);
void IntToStr(char *strr, long long int nbr, unsigned int base);
void FloatToStr(char *p, MMFLOAT f, int m, int n, unsigned char ch);
void STR_REPLACE(char *target, const char *needle, const char *replacement, uint8_t ignoresurround);
int str_equal(char *s1, char *s2);
#define mem_equal(a,b,c) !strncasecmp(a,b,c)
