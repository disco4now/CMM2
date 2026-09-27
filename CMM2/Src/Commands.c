/***************************************************************************

CMM2 MMBasic
commands.c

Handles all the commands in MMBasic

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
#include "MMBasic_Includes.h"

#include "Hardware_Includes.h"
#include "help.h"
#include "Turtle.h"
#ifdef STRUCTENABLED
#include "re.h"
#endif

/* Stride-aware array access macros for struct member arrays */
#define STRIDE_FLOAT(ptr, idx, stride) (*(MMFLOAT *)((char *)(ptr) + (idx) * (stride)))
#define STRIDE_INT(ptr, idx, stride) (*(long long int *)((char *)(ptr) + (idx) * (stride)))

void flist(int, int, int);
void clearprog(void);
void execute_one_command(char *p);
void MIPS16 ListProgramFlash(char *p, int all);
void MIPS16 ListFile(char *pp, int all);
// stack to keep track of nested FOR/NEXT loops
struct s_forstack forstack[MAXFORLOOPS + 1];
int forindex;
extern unsigned long long int statement_time;
extern char *firststmt;
extern volatile uint64_t FastTimer;
// stack to keep track of nested DO/LOOP loops
struct s_dostack dostack[MAXDOLOOPS];
int doindex;                                						// counts the number of nested DO/LOOP loops
char *KeyInterrupt=NULL;
volatile int Keycomplete=0;
int keyselect=0;
void ListNewLine(int *ListCnt, int all);
// stack to keep track of GOSUBs, SUBs and FUNCTIONs
char *gosubstack[MAXGOSUB];
char *errorstack[MAXGOSUB];
int gosubindex;
char runcmd[STRINGSIZE];

char DimUsed = false;						// used to catch OPTION BASE after DIM has been used
struct sa_data datastore[MAXRESTORE];
int restorepointer = 0;

#ifdef STRUCTENABLED
// Structure type definition table
struct s_structdef *g_structtbl[MAX_STRUCT_TYPES]; // Array of pointers, allocated per-type
int g_structcnt = 0;							   // Number of defined structure types
int g_StructArg = -1;							   // Struct index for pending DIM AS structtype (-1 if none)
int g_StructMemberType = 0;						   // Type of struct member being accessed (0 if not a member access)
int g_StructMemberOffset = 0;					   // Offset of member within struct (for EXTRACT/INSERT/SORT)
int g_StructMemberSize = 0;						   // Size of the member (for EXTRACT/INSERT/SORT)
int g_ExprStructType = -1;						   // Struct type index from expression evaluation (-1 if not a struct)
#endif



int TraceOn;                                // used to track the state of TRON/TROFF
  char *TraceBuff[TRACE_BUFF_SIZE];
  int TraceBuffIndex;                       // used for listing the contents of the trace buffer

int OptionErrorSkip;                                               // how to handle an error
int MMerrno=0;                                                        // the error number
char *MMErrMsg = "";                                                // the error message
uint64_t g_flag = 0;
static inline CommandToken commandtbl_decode(const char *p)
{
#ifdef CMD16BIT	
	return ((CommandToken)(p[0] & 0x7f)) | ((CommandToken)(p[1] & 0x7f) << 7);
#else
    return ((CommandToken)(p[0])) ;
#endif
}
void erase(char *p, char nofree);
void command_print(char *cmd);
void execute(char *mycmd);
void cmd_null(void) {
	// do nothing (this is just a placeholder for commands that have no action)
}
void display_help(int current, char *buff){
	   MMPrintString("\r\n");
	   CurrentX=3;
	   int k;
	   for(k=0;k<Option.Width-1;k++)putConsole(k<strlen(help[current+1])? help[current+1][k] : ' ');
	   PRet();
	   CurrentX=3;
	   k=0;
	   for(int j=0;j<strlen(buff);j++)if(toupper(buff[j])!=help[current+2][j])k=1;
	   if(k==0){
		   for(k=0;k<Option.Width-1;k++)putConsole(k<strlen(help[current+3])? help[current+3][k] : ' ');
	   } else {
		 for(k=0;k<Option.Width-1;k++)putConsole(' ');
	   }
	   PRet();
	   CurrentX=3;
	   k=0;
	   for(int j=0;j<strlen(buff);j++)if(toupper(buff[j])!=help[current+4][j])k=1;
	   if(k==0){
		   for(k=0;k<Option.Width-1;k++)putConsole(k<strlen(help[current+5])? help[current+5][k] : ' ');
	   } else {
		 for(k=0;k<Option.Width-1;k++)putConsole(' ');
	   }
}
void do_help(char *p){
    if(CurrentLinePtr) error("Invalid in a program");
    char buff[STRINGSIZE]={0};
    strcpy(buff,p);
    int i,currsave=CurrentY, up=1;
    int current=0;
    int firstin=1;
 	MMPrintString("\r\n\n\n");
    while(1){
    	if(up)SerUSBPutS("\033[3A");
		ShowCursor(false);
		putConsole('\r');
    	CurrentY=currsave;CurrentX=3;
        putConsole('?');putConsole(' ');putConsole(' ');
        CurrentX-=(FontTable[gui_font >> 4][0] * (gui_font & 0b1111));
        MMPrintString(buff);
    	int c=0;
    	if(firstin==0){
			do {
				ShowCursor(true);
				c=MMInkey();

			}while(c==-1);
    	}
    	firstin=0;
    	if(c==0x1b || c==F12){
    		ShowCursor(false);
    		MMPrintString("\r\n\n\n\n");
    		strcpy(p,help[current+1]);
    		break;
    	}
    	if(c=='\b' && strlen(buff)){
    		ShowCursor(false);
    		buff[strlen(buff)-1]=0;
    		CurrentX-=(FontTable[gui_font >> 4][0] * (gui_font & 0b1111));
    		SerUSBPutS("\033[1K");
    		DisplayPutC(' ');
    		putConsole('\r');
    	} else if(c==UP){
    		int k;
    	    current-=2;
    	    if(current==-2)current=0;
		    k=0;
		    for(int j=0;j<strlen(buff);j++)if(toupper(buff[j])!=help[current][j])k=1;
		    if(k==0){
			   display_help(current, buff);
			   up=1;
			   continue;
		    } else {
			   current+=2;
		    }
    	} else if(c==DOWN){
    		int k;
    	    current+=2;
    	    if(*help[current]=='Z')current-=2;
 		    k=0;
 		    for(int j=0;j<strlen(buff);j++)if(toupper(buff[j])!=help[current][j])k=1;
 		    if(k==0){
 			   display_help(current, buff);
 			   up=1;
 			   continue;
 		    } else {
 			   current-=2;
 		    }
      	} else if(IsPrint(c)){
    		ShowCursor(false);
    		buff[strlen(buff)]=c;
    		buff[strlen(buff)]=0;
    	}
    	routinechecks(1);
		ShowCursor(false);
    	for(i=0;*help[i]!='Z';i+=2){
    	   int k=0;
    	   for(int j=0;j<strlen(buff);j++)if(toupper(buff[j])!=help[i][j])k=1;
  	       if(k==0 && strlen(buff)){
  	    	   current=i;
  	    	   display_help(current, buff);
  	    	   up=1;
  	    	   break;
  	       } else {
  	    	   up=0;
  	       }
    	}
    }
    ShowCursor(false);
}
/*
void cmd_helpx(void){
	  do_help(cmdline);
	  memset(cmdline,0,STRINGSIZE);
	  memset(inpbuf,0,STRINGSIZE);
}
*/

int printWrappedText(const char *text, int screenWidth, int listcnt, int all)
{
	int length = strlen(text);
	int start = 0; // Start index of the current line
	char buff[STRINGSIZE];
	while (start < length)
	{
		int end = start + screenWidth; // Calculate the end index for the current line
		if (end >= length)
		{
			// If end is beyond the text length, just print the remaining text
			memset(buff, 0, STRINGSIZE);
			sprintf(buff, "%s", text + start);
			MMPrintString(buff);
			ListNewLine(&listcnt, all);
			break;
		}

		// Find the last space within the current screen width
		int lastSpace = -1;
		for (int i = start; i < end; i++)
		{
			if (text[i] == ' ')
			{
				lastSpace = i;
			}
		}

		if (lastSpace != -1)
		{
			// If a space is found, break at the space
			memset(buff, 0, STRINGSIZE);
			sprintf(buff, "%.*s", lastSpace - start, text + start);
			MMPrintString(buff);
			ListNewLine(&listcnt, all);
			start = lastSpace + 1; // Skip the space
		}
		else
		{
			// If no space is found, truncate at screen width
			memset(buff, 0, STRINGSIZE);
			sprintf(buff, "%.*s", screenWidth, text + start);
			MMPrintString(buff);
			ListNewLine(&listcnt, all);
			start += screenWidth;
		}
	}
	return listcnt;
}






void cmd_help(void){

	if(CurrentLinePtr) error("Invalid in a program");
	if (!ExistsFile("/help.txt")){
		  helpquotes=0;
		  do_help(cmdline);
		  memset(cmdline,0,STRINGSIZE);
		  memset(inpbuf,0,STRINGSIZE);
	}else{


		getcsargs(&cmdline, 1);
		//if (!ExistsFile("help.txt"))
		// 	error("help.txt not found");
		if (!argc)
		{
			helpquotes=1;
			MMPrintString("Enter help and the name of the command or function\r\nUse * for multicharacter wildcard or ? for single character wildcard\r\n");
		}
		else
		{
			if (helpquotes==0){        //Set helpquotes=1 and reissue command so it tokenises with quotes.
				 helpquotes=1;
				 strcpy(inpbuf,"HELP \"");
				 strcat(inpbuf,argv[0]);
			     strcat(inpbuf,"\"\r\n");
			    // MMPrintString(inpbuf);
			     tokenise(true);                                             // turn into executable code
			     ExecuteProgram(tknbuf);                                     // execute the line straight away
				// mymemset(inpbuf,0,STRINGSIZE);
				return;
			}
			int fnbr = FindFreeFileNbr();
			char *buff = GetTempStrMemory();
			BasicFileOpen("/help.txt", fnbr, FA_READ);
			//BasicFileOpen("help.txt", fnbr, FA_READ);
			int ListCnt = CurrentY / (FontTable[gui_font >> 4][1] * (gui_font & 0b1111)) + 2;
			//MMPrintString("1");
			char *p = (char *)getCstring(argv[0]);
			//MMPrintString(p);
			bool end = false;
			while (!FileEOF(fnbr))
			{ // while waiting for the end of file
				memset(buff, 0, STRINGSIZE);
				char *in = buff;
				while (1)
				{
					if (FileEOF(fnbr))
					{
						end = true;
						break;
					}
					char c = FileGetChar(fnbr);
					if (c == '\n')
						break;
					if (c == '\r')
						continue;
					*in++ = c;
				}
				if (end)
					break;
				skipspace(p);
				if (buff[0] == '~')
				{
					if (pattern_matching(p, &buff[1], 0, 0))
					{
						while (1)
						{ // loop through all lines for the command
							memset(buff, 0, STRINGSIZE);
							char *in = buff;
							while (1)
							{ // get this line
								if (FileEOF(fnbr))
								{
									end = true;
									break;
								}
								char c = FileGetChar(fnbr);
								if (c == '\n')
									break;
								if (c == '\r')
									continue;
								*in++ = c;
							}
							if (end)
								break;
							if (buff[0] == '~')
							{ // now we need to rewind the file to check this line
								ListNewLine(&ListCnt, false);
								//lfs_file_seek(&lfs, FileTable[fnbr].lfsptr, -(strlen(buff) + 2), LFS_SEEK_CUR);
								break;
							}
							else
							{
								ListCnt = printWrappedText(buff, Option.Width - 1, ListCnt, false);
							}
						}
					}
				}
			}
			FileClose(fnbr);
		}
    }
}



#ifdef CMD16BIT

#ifndef REDIM
void parse_and_strip(char *string, int *dims)
{
	// Initialize dims to zero
	for (int i = 0; i < MAXDIM; i++)
	{
		dims[i] = 0;
	}
	char *open = strchr(string, '(');
	char *close = strchr(string, ')');

	if (open && close && close > open)
	{
		// Parse numbers inside parentheses
		char buffer[256];
		strncpy(buffer, open + 1, close - open - 1);
		buffer[close - open - 1] = '\0';

		char *token = strtok(buffer, ",");
		int idx = 0;
		while (token && idx < MAXDIM)
		{
			dims[idx++] = atoi(token);
			token = strtok(NULL, ",");
		}

		// Replace with name()
		*(open + 1) = '\0';	   // truncate after '('
		strcpy(open + 1, ")"); // append ')'
	}
}

bool array_comp(int in[MAXDIM], int out[MAXDIM])

{
	int last_in = -1, last_out = -1;

	// Find last non-zero index in each array
	for (int i = MAXDIM - 1; i >= 0; i--)
	{
		if (last_in == -1 && in[i] != 0)
			last_in = i;
		if (last_out == -1 && out[i] != 0)
			last_out = i;
	}

	// If positions of last non-zero differ, not allowed
	if (last_in != last_out)
		return false;

	// If both arrays are all zeros, they are identical
	if (last_in == -1)
		return true;

	// Compare all elements except at the last non-zero index
	for (int i = 0; i < MAXDIM; i++)
	{
		if (i == last_in)
			continue; // allow difference at the last non-zero
		if (in[i] != out[i])
			return false;
	}

	return true;
}


void cmd_redim(void)
{
	int dims[MAXDIM] = {0}, newdims[MAXDIM] = {0};
	char *oldmemory = NULL, *newmemory;
	int oldsize = 0, newsize = 0;
	int length = -1;
#ifdef STRUCTENABLED
	int structIdx = -1;
#endif
	char *tp;
	char old[MAXVARLEN + 1];
	int preserve = ((tp = checkstring(cmdline, "PRESERVE")) ? 1 : 0);
	if (tp == NULL)
		tp = cmdline;
	{
		getcsargs(&tp, MAX_ARG_COUNT);
		for (int i = 0; i < argc; i += 2)
		{ // step through the arguments
			strncpy((char *)old, (char *)argv[i], MAXVARLEN);
			parse_and_strip((char *)old, newdims);
			findvar(old, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
			if (vartbl[VarIndex].type & T_STR)
				length = vartbl[VarIndex].size;
#ifdef STRUCTENABLED
			if (vartbl[VarIndex].type & T_STRUCT)
				structIdx = (int)vartbl[VarIndex].size; // Save struct type index
#endif
			if (!vartbl[VarIndex].dims[0])
				error("$ is not an array ", argv[i]);
			int type = TypeMask(vartbl[VarIndex].type);
			if (vartbl[VarIndex].dims[0] > 0)
			{
				oldmemory = vartbl[VarIndex].val.s;
				oldsize = MemSize(oldmemory);
				for (int i = 0; i < MAXDIM; i++)
				{
					dims[i] = vartbl[VarIndex].dims[i];
				}
				if (!array_comp(dims, newdims) && preserve)
					error("Only the last array index can be changed");
			}
			//uint32_t addr = erase((char *)old, (preserve ? true : false));
			erase((char *)old, (preserve ? true : false));
			if (type & T_STR)
			{
				char *newstring = GetTempStrMemory();
				strcpy((char *)newstring, (char *)argv[i]);
				strcat((char *)newstring, " LENGTH ");
				IntToStr((char *)&newstring[strlen((char *)newstring)], length, 10);
				findvar(newstring, type | V_FIND | V_DIM_VAR);
			}
#ifdef STRUCTENABLED
			else if (type & T_STRUCT)
			{
				// Strip "AS structtype" from the argument - find and terminate at AS token
				char *vararg = GetTempStrMemory();
				strcpy((char *)vararg, (char *)argv[i]);
				char *asp = skipvar(vararg, false);
				while (*asp && *asp != tokenAS)
					asp++;
				if (*asp == tokenAS)
					*asp = 0;			 // Terminate string before AS
				g_StructArg = structIdx; // Set struct type for findvar
				findvar(vararg, type | T_IMPLIED | V_FIND | V_DIM_VAR);
				g_StructArg = -1; // Reset
			}
#endif
			else
				findvar(argv[i], type | V_FIND | V_DIM_VAR);
			newmemory = vartbl[VarIndex].val.s;
			newsize = MemSize(vartbl[VarIndex].val.s);
			if (preserve)
			{
				if (newsize < oldsize)
					oldsize = newsize;
				memcpy(newmemory, oldmemory, oldsize);
				// Check if in heap
			//	if (addr > (uint32_t)MMHeap && addr < (uint32_t)MMHeap + heap_memory_size)
			//	{
					//FreeMemorySafe((void **)&addr);
					//FreeMemory((void **)&oldmemory);
					FreeMemory(oldmemory);
					//FreeMemory(vartbl[j].val.s);
			//	}
//#ifdef rp2350
//#ifndef PICOMITEWEB
//				else if (addr > (uint32_t)PSRAMbase && addr < (uint32_t)PSRAMbase + PSRAMsize)
//				{
//					FreeMemorySafe((void **)&addr);
//				}
//#endif
//#endif
			}
		}
	}
}

#endif

void cmd_arrayset(void)
{
	array_set(cmdline);
}
void array_set(char *tp)
{
	MMFLOAT f;
	long long int i64;
	char *s;
	int dims[MAXDIM] = {0};
	int i, t, copy, card1 = 1;
	unsigned char size = 0;
	MMFLOAT *a1float = NULL;
	int64_t *a1int = NULL;
	char *a1str = NULL;
	int s1; // stride for numeric arrays
	getcsargs(&tp, 3);
	if (!(argc == 3))
		StandardError(2);
	findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	t = vartbl[VarIndex].type;
	evaluate(argv[0], &f, &i64, &s, &t, false);
	if (t & T_STR)
	{
		card1 = parsestringarray(argv[2], &a1str, 2, 0, dims, true, &size);
		copy = (int)size + 1;
		memset(a1str, 0, copy * card1);
		if (*s)
		{
			for (i = 0; i < card1; i++)
			{
				Mstrcpy(&a1str[i * copy], s);
			}
		}
	}
	else
	{
		card1 = parsenumberarray(argv[2], &a1float, &a1int, 2, 0, dims, true,&s1);
		if (t & T_STR)
			SyntaxError();
		;

		if (a1float != NULL)
		{
			for (i = 0; i < card1; i++)
				STRIDE_FLOAT(a1float, i, s1) = ((t & T_INT) ? (MMFLOAT)i64 : f);
		}
		else
		{
			for (i = 0; i < card1; i++)
				STRIDE_INT(a1int, i, s1) = ((t & T_INT) ? i64 : FloatToInt64(f));
		}
	}
}
void cmd_add(void)
{
	array_add(cmdline);
}

void array_add(char *tp)
{
	MMFLOAT f;
	long long int i64;
	char *s;
	int dims[MAXDIM] = {0};
	int i, t, card1 = 1, card2 = 1;
	MMFLOAT *a1float = NULL, *a2float = NULL, scale;
	int64_t *a1int = NULL, *a2int = NULL;
	char *a1str = NULL, *a2str = NULL;
	int s1, s2;
	getcsargs(&tp, 5);
	if (!(argc == 5))
		StandardError(2);
	findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	t = vartbl[VarIndex].type;
	if (t & T_STR)
	{
		unsigned char size = 0, size2 = 0;
		char *toadd;
		card1 = parsestringarray(argv[0], &a1str, 1, 0, dims, false, &size);
		evaluate(argv[2], &f, &i64, &s, &t, false);
		if (!(t & T_STR))
			SyntaxError();
		;
		toadd = getstring(argv[2]);
		card2 = parsestringarray(argv[4], &a2str, 3, 0, dims, true, &size2);
		if (card1 != card2)
			StandardError(16);
		char *buff = GetTempStrMemory(); // this will last for the life of the command
		int copy = size + 1;
		int copy2 = size2 + 1;
		for (i = 0; i < card1; i++)
		{
			char *sarg1 = a1str + i * copy;
			char *sarg2 = a2str + i * copy2;
			if (*sarg1 + *toadd > size2)
				error("String too long");
			Mstrcpy(buff, sarg1);
			Mstrcat(buff, toadd);
			Mstrcpy(sarg2, buff);
		}
	}
	else
	{
		card1 = parsenumberarray(argv[0], &a1float, &a1int, 1, 0, dims, false,&s1);
		evaluate(argv[2], &f, &i64, &s, &t, false);
		if (t & T_STR)
			SyntaxError();
		;
		scale = getnumber(argv[2]);
		card2 = parsenumberarray(argv[4], &a2float, &a2int, 3, 0, dims, true,&s2);
		if (card1 != card2)
			StandardError(16);
		if (scale != 0.0)
		{
			if (a2float != NULL && a1float != NULL)
			{
				for (i = 0; i < card1; i++)
					STRIDE_FLOAT(a2float, i, s2) = ((t & T_INT) ? (MMFLOAT)i64 : f) + STRIDE_FLOAT(a1float, i, s1);
					//*a2float++ = ((t & T_INT) ? (MMFLOAT)i64 : f) + (*a1float++);
			}
			else if (a2float != NULL && a1float == NULL)
			{
				for (i = 0; i < card1; i++)
					STRIDE_FLOAT(a2float, i, s2) = ((t & T_INT) ? (MMFLOAT)i64 : f) + ((MMFLOAT)STRIDE_INT(a1int, i, s1));
					//(*a2float++) = ((t & T_INT) ? (MMFLOAT)i64 : f) + ((MMFLOAT)*a1int++);
			}
			else if (a2float == NULL && a1float != NULL)
			{
				for (i = 0; i < card1; i++)
					STRIDE_INT(a2int, i, s2) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) + STRIDE_FLOAT(a1float, i, s1));
					//(*a2int++) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1float++));
			}
			else
			{
				for (i = 0; i < card1; i++)
					STRIDE_INT(a2int, i, s2) = ((t & T_INT) ? i64 : FloatToInt64(f)) + STRIDE_INT(a1int, i, s1);
					//(*a2int++) = ((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1int++);
			}
		}
		else
		{
			if (a2float != NULL && a1float != NULL)
			{
				for (i = 0; i < card1; i++)
					STRIDE_FLOAT(a2float, i, s2) = STRIDE_FLOAT(a1float, i, s1);
					//*a2float++ = *a1float++;
			}
			else if (a2float != NULL && a1float == NULL)
			{
				for (i = 0; i < card1; i++)
					STRIDE_FLOAT(a2float, i, s2) = ((MMFLOAT)STRIDE_INT(a1int, i, s1));
					//(*a2float++) = ((MMFLOAT)*a1int++);
			}
			else if (a2float == NULL && a1float != NULL)
			{
				for (i = 0; i < card1; i++)
					STRIDE_INT(a2int, i, s2) = FloatToInt64(STRIDE_FLOAT(a1float, i, s1));
					//(*a2int++) = FloatToInt64(*a1float++);
			}
			else
			{
				for (i = 0; i < card1; i++)
					STRIDE_INT(a2int, i, s2) = STRIDE_INT(a1int, i, s1);
					//*a2int++ = *a1int++;
			}
		}
	}
}
void cmd_insert(void)
{
	array_insert(cmdline);
}
void array_insert(char *tp)
{
	int i, j, t, start, increment, dim[MAXDIM], pos[MAXDIM], off[MAXDIM], dimcount = 0, target = -1;
	int64_t *a1int = NULL, *a2int = NULL;
	MMFLOAT *afloat = NULL;
	char *a1str = NULL, *a2str = NULL;
	unsigned char size = 0, size2 = 0;
	int dims[MAXDIM] = {0};
	getcsargs(&tp, 15);
	if (argc < 7)
		StandardError(2);
	findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	t = vartbl[VarIndex].type;
	if (t & T_STR)
	{
		parsestringarray(argv[0], &a1str, 1, 0, dims, false, &size);
	}
	else
	{
		parsenumberarray(argv[0], &afloat, &a1int, 1, 0, dims, false,NULL);
		if (!a1int)
			a1int = (int64_t *)afloat;
	}
	if (dims[1] <= 0)
		error("Argument 1 must be a 2D or more array");
	for (i = 0; i < MAXDIM; i++)
	{
		if (dims[i] - OptionBase > 0)
		{
			dimcount++;
			dim[i] = dims[i] - OptionBase;
		}
		else
			dim[i] = 0;
	}
	if (((argc - 1) / 2 - 1) != dimcount)
		StandardError(2);
	for (i = 0; i < dimcount; i++)
	{
		if (*argv[i * 2 + 2])
			pos[i] = getint(argv[i * 2 + 2], OptionBase, dim[i] + OptionBase) - OptionBase;
		else
		{
			if (target != -1)
				error("Only one index can be omitted");
			target = i;
			pos[i] = 1;
		}
	}
	if (t & T_STR)
	{
		parsestringarray(argv[i * 2 + 2], &a2str, i + 1, 1, dims, true, &size2);
	}
	else
	{
		parsenumberarray(argv[i * 2 + 2], &afloat, &a2int, i + 1, 1, dims, true,NULL);
		if (!a2int)
			a2int = (int64_t *)afloat;
	}
	if (target == -1)
		return;
	if (dim[target] + OptionBase != dims[0])
		error("Size mismatch between insert and target array");
	if (size != size2)
		error("String arrays differ in string length");
	i = dimcount - 1;
	while (i >= 0)
	{
		off[i] = 1;
		for (j = 0; j < i; j++)
			off[i] *= (dim[j] + 1);
		i--;
	}
	start = 1;
	for (i = 0; i < dimcount; i++)
	{
		start += (pos[i] * off[i]);
	}
	start--;
	increment = off[target];
	start -= increment;
	if (t & T_STR)
	{
		int copy = (int)size + 1;
		for (i = 0; i <= dim[target]; i++)
		{
			char *p = a2str + i * copy;
			char *q = &a1str[(start + i * increment) * copy];
			memcpy(q, p, copy);
		}
	}
	else
	{
		for (i = 0; i <= dim[target]; i++)
			a1int[start + i * increment] = *a2int++;
	}
	return;
}
void cmd_slice(void)
{
	array_slice(cmdline);
}
void array_slice(char *tp)
{
	int i, j, t, start, increment, dim[MAXDIM], pos[MAXDIM], off[MAXDIM], dimcount = 0, target = -1, toarray = 0;
	int64_t *a1int = NULL, *a2int = NULL;
	MMFLOAT *afloat = NULL;
	char *a1str = NULL, *a2str = NULL;
	unsigned char size = 0, size2 = 0;
	int dims[MAXDIM] = {0};
	getcsargs(&tp, 15);
	if (argc < 7)
		StandardError(2);
	findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	t = vartbl[VarIndex].type;
	if (t & T_STR)
	{
		parsestringarray(argv[0], &a1str, 1, 0, dims, false, &size);
	}
	else
	{
		parsenumberarray(argv[0], &afloat, &a1int, 1, 0, dims, false,NULL);
		if (!a1int)
			a1int = (int64_t *)afloat;
	}
	if (dims[1] <= 0)
		error("Argument 1 must be a 2D or more array");
	for (i = 0; i < MAXDIM; i++)
	{
		if (dims[i] - OptionBase > 0)
		{
			dimcount++;
			dim[i] = dims[i] - OptionBase;
		}
		else
			dim[i] = 0;
	}
	if (((argc - 1) / 2 - 1) != dimcount)
		StandardError(2);
	for (i = 0; i < dimcount; i++)
	{
		if (*argv[i * 2 + 2])
			pos[i] = getint(argv[i * 2 + 2], OptionBase, dim[i] + OptionBase) - OptionBase;
		else
		{
			if (target != -1)
				error("Only one index can be omitted");
			target = i;
			pos[i] = 1;
		}
	}
	if (t & T_STR)
	{
		toarray = parsestringarray(argv[i * 2 + 2], &a2str, i + 1, 1, dims, true, &size2) - 1;
	}
	else
	{
		toarray = parsenumberarray(argv[i * 2 + 2], &afloat, &a2int, i + 1, 1, dims, true,NULL) - 1;
		if (!a2int)
			a2int = (int64_t *)afloat;
	}
	if (dim[target] != toarray)
		error("Size mismatch between slice and target array");
	if (size != size2)
		error("String arrays differ in string length");
	i = dimcount - 1;
	while (i >= 0)
	{
		off[i] = 1;
		for (j = 0; j < i; j++)
			off[i] *= (dim[j] + 1);
		i--;
	}
	start = 1;
	for (i = 0; i < dimcount; i++)
	{
		start += (pos[i] * off[i]);
	}
	start--;
	increment = off[target];
	start -= increment;
	if (t & T_STR)
	{
		int copy = (int)size + 1; // allow for the length character of the string
		for (i = 0; i <= dim[target]; i++)
		{
			char *p = a2str + i * copy;
			char *q = &a1str[(start + i * increment) * copy];
			memcpy(p, q, copy);
		}
	}
	else
	{
		for (i = 0; i <= dim[target]; i++)
			*a2int++ = a1int[start + i * increment];
	}
	return;
}

#endif


void cmd_execute(void){
	execute(cmdline);
}
void execute(char *mycmd){
//    char *temp_tknbuf;
	char *ttp;
	int i=0,toggle=0;
//    temp_tknbuf = GetTempStrMemory();
//    strcpy(temp_tknbuf, tknbuf);
    // first save the current token buffer in case we are in immediate mode
    // we have to fool the tokeniser into thinking that it is processing a program line entered at the console
    skipspace(mycmd);
	strcpy(inpbuf, getCstring(mycmd));                                      // then copy the argument
	if(!(toupper(inpbuf[0])=='R' && toupper(inpbuf[1])=='U' && toupper(inpbuf[2])=='N')){ //convert the string to upper case
		 while(inpbuf[i]){
			if(inpbuf[i]==34){
				if(toggle==0)toggle=1;
				else toggle=0;
			}
			if(!toggle){
				if(inpbuf[i]==':')error("Only single statements allowed");
				inpbuf[i]=toupper(inpbuf[i]);
			}
			i++;
		  }
		 tokenise(true);                                                 // and tokenise it (the result is in tknbuf)
		 mymemset(inpbuf,0,STRINGSIZE);
		 tknbuf[strlen(tknbuf)]=0;
		 tknbuf[strlen(tknbuf)+1]=0;
		 ttp = nextstmt;                                                 // save the globals used by commands
		 ScrewUpTimer=1000;
		 ExecuteProgram(tknbuf);                                              // execute the function's code
		 ScrewUpTimer=0;
// TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
		 nextstmt = ttp;
		 return;
    } else {
    	char *p=inpbuf;
		char *q, *s;
		char fn[FF_MAX_LFN];
		FRESULT fr;
		FILINFO fno;
#ifndef CMD16BIT
		p[0]=GetCommandValue("RUN");
		memmove(&p[1],&p[4],strlen(p)-4);
		if((q=strchr(p,':'))){
			q--;
			*q='0';
		}
		p[strlen(p)-3]=0;
#else
		CommandToken tkn = GetCommandValue("RUN");
		//tknbuf[0] = (tkn & 0x7f) + C_BASETOKEN;
		//tknbuf[1] = (tkn >> 7) + C_BASETOKEN; // tokens can be 14-bit
		p[0] = (tkn & 0x7f) + C_BASETOKEN;
		p[1] = (tkn >> 7) + C_BASETOKEN; // tokens can be 14-bit
		memmove(&p[2], &p[4], strlen(p) - 4);
		if((q=strchr(p,':'))){
			q--;
			*q='0';
		}
		p[strlen(p)-2]=0;
#endif

		if((q=strchr(p,'\"')) != 0){
			if((s=strchr(&q[1],'\"')) != 0)*s=0;
			strcpy(fn,&q[1]);
			strcpy(fn,q);
			if(strchr(fn, '.') == NULL){
			  strcat(fn, ".BAS");
			}
			if(!InitSDCard()) {}; //make sure the SDcard is there
			fr = f_stat(&fn[1], &fno); //check for the file in the current directory
			*s='\"';
			if(fr != FR_OK || (fno.fattrib & AM_DIR)){ // file was not in the current directory so insert the path
			  int slen=strlen(p) - (uint32_t)p + (uint32_t)&q[1];
			  memmove(&q[1+strlen((char *)Option.path)],&q[1],slen+1);
			  memcpy(&q[1],(char *)Option.path,strlen((char *)Option.path));
			}
		}
		CloseAudio(1);
		strcpy(tknbuf,inpbuf);
		longjmp(run,1);
    }
}
// length of string now checked
void cmd_inc(void){
	char *p,*q;
    int vtype;
	getargs(&cmdline,3,",");
	if(argc==1){
		p = findvar(argv[0], V_FIND);
		if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
#ifdef STRUCTENABLED
		if (g_StructMemberType != 0)
			vtype = TypeMask(g_StructMemberType);
#endif
        if(vtype & T_STR) error("Invalid variable");                // sanity check
		if(vtype & T_NBR)
            (*(MMFLOAT *)p) = (*(MMFLOAT *)p) + 1.0;
		else if(vtype & T_INT)*(long long int *)p = *(long long int *)p + 1;
		else error("Syntax");
	} else {
		p = findvar(argv[0], V_FIND);
		if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
#ifdef STRUCTENABLED
		if (g_StructMemberType != 0)
			vtype = TypeMask(g_StructMemberType);
#endif
        if(vtype & T_STR){
        	int size=vartbl[VarIndex].size;
#ifdef STRUCTENABLED
			if (g_StructMemberType & T_STR)
				size = g_StructMemberSize;
#endif
        	q=getstring(argv[2]);
        	//if(*p + *q > MAXSTRLEN) error("String too long");
        	if(*p + *q > size) error("String too long");
			//Mstrcat(p, getstring(argv[2]));
        	Mstrcat(p, q);
        } else if(vtype & T_NBR){
        	 (*(MMFLOAT *)p) = (*(MMFLOAT *)p)+getnumber(argv[2]);
        } else if(vtype & T_INT){
        	*(long long int *)p = *(long long int *)p+getinteger(argv[2]);
        } else error("syntax");
 	}
}
/*
void cmd_inc(void){
	char *p, *q;
    int vtype;
	getargs(&cmdline,3,",");
	if(argc==1){
		p = findvar(argv[0], V_FIND);
		if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
        if(vtype & T_STR) error("Invalid variable");                // sanity check
		if(vtype & T_NBR)
            (*(MMFLOAT *)p) = (*(MMFLOAT *)p) + 1.0;
		else if(vtype & T_INT)*(long long int *)p = *(long long int *)p + 1;
		else error("Syntax");
	} else {
		p = findvar(argv[0], V_FIND);
		if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
        if(vtype & T_STR){
        	q=getstring(argv[2]);
    		if(*p + *q > MAXSTRLEN) error("String too long");
			Mstrcat(p, q);
        } else if(vtype & T_NBR){
        	 (*(MMFLOAT *)p) = (*(MMFLOAT *)p)+getnumber(argv[2]);
        } else if(vtype & T_INT){
        	*(long long int *)p = *(long long int *)p+getinteger(argv[2]);
        } else error("syntax");
 	}
}
*/
void cmd_debug(void){
	char *p;
		if((p = checkstring(cmdline, "BREAK"))) {
		char *q=GetStringMemory();
		while(1){
			MMPrintString("DEBUG> ");
			MMgetline(0, &q[1]);
			if(strncasecmp(&q[1], "CONTINUE",8)==0)break;
			*q='\"';
			strcat(q,"\"");
			execute(q);
			memset(inpbuf,0,STRINGSIZE);
		}
		FreeMemorySafe((void *)&q);
		return;
		}
		command_print(cmdline);
}
// the PRINT command
void cmd_print(void) {
	command_print(cmdline);
}
void command_print(char *cmd) {
	char *s, *p;
    unsigned char *ss;
	MMFLOAT f;
    long long int i64;
	int i, t, fnbr;
	int docrlf;														// this is used to suppress the cr/lf if needed

	getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ";,");				// this is a macro and must be the first executable stmt

//    s = 0; *s = 56;											    // for testing the exception handler

	docrlf = true;

	if(argc > 0 && *argv[0] == '#') {								// check if the first arg is a file number
		argv[0]++;
         if((*argv[0] == 'G') || (*argv[0] == 'g')){
            argv[0]++;
            if(!((*argv[0] == 'P') || (*argv[0] == 'p')))error("Syntax");
            argv[0]++;
            if(!((*argv[0] == 'S') || (*argv[0] == 's')))error("Syntax");
            if(!GPSchannel) error("GPS not activated");
            if(argc!=3) error("Only a single string parameter allowed");
            p = argv[2];
			t = T_NOTYPE;
			p = evaluate(p, &f, &i64, &s, &t, true);			// get the value and type of the argument
            ss=(unsigned char *)s;
            if(!(t & T_STR)) error("Only a single string parameter allowed");
            int i,xsum=0;
            if(ss[1]!='$' || ss[ss[0]]!='*')error("GPS command must start with dollar and end with star");
            for(i=1;i<=ss[0];i++){
                SerialPutchar(GPSchannel, s[i]);
                if(s[i]=='$')xsum=0;
                if(s[i]!='*')xsum ^=s[i];
            }
            i=xsum/16;
            i=i+'0';
            if(i>'9')i=i-'0'+'A';
            SerialPutchar(GPSchannel, i);
            i=xsum % 16;
            i=i+'0';
            if(i>'9')i=i-'0'+'A';
            SerialPutchar(GPSchannel, i);
            SerialPutchar(GPSchannel, 13);
            SerialPutchar(GPSchannel, 10);
            return;
        } else {
            fnbr = getinteger(argv[0]);									// get the number
            i = 1;
            if(argc >= 2 && *argv[1] == ',') i = 2;						// and set the next argument to be looked at
        }
	} else {
		fnbr = 0;													// no file number so default to the standard output
		i = 0;
	}

	for(; i < argc; i++) {											// step through the arguments
		if(*argv[i] == ',') {
			MMfputc('\t', fnbr);									// print a tab for a comma
			docrlf = false;                                         // a trailing comma should suppress CR/LF
		}
		else if(*argv[i] == ';') {
			docrlf = false;											// other than suppress cr/lf do nothing for a semicolon
		}
		else {														// we have a normal expression
			p = argv[i];
			while(*p) {
				t = T_NOTYPE;
				p = evaluate(p, &f, &i64, &s, &t, true);			// get the value and type of the argument
                if(t & T_NBR) {
                    *inpbuf = ' ';                                  // preload a space
                    FloatToStr(inpbuf + ((f >= 0) ? 1:0), f, 0, STR_AUTO_PRECISION, ' ');// if positive output a space instead of the sign
					MMfputs(CtoM(inpbuf), fnbr);					// convert to a MMBasic string and output
				} else if(t & T_INT) {
                    *inpbuf = ' ';                                  // preload a space
                    IntToStr(inpbuf + ((i64 >= 0) ? 1:0), i64, 10); // if positive output a space instead of the sign
					MMfputs(CtoM(inpbuf), fnbr);					// convert to a MMBasic string and output
				} else if(t & T_STR) {
					MMfputs(s, fnbr);								// print if a string (s is a MMBasic string)
				} else error("Attempt to print reserved word");								// trap things like PRINT AS
			}
			docrlf = true;
		}
	}
	if(docrlf && AutoLineWrap) MMfputs("\2\r\n", fnbr);								// print the terminating cr/lf unless it has been suppressed
	if(PrintPixelMode!=0)SerUSBPutS("\033[m");
	PrintPixelMode=0;
	AutoLineWrap=true;
}



// the LET command
// because the LET is implied (ie, line does not have a recognisable command)
// it ends up as the place where mistyped commands are discovered.  This is why
// the error message is "Unknown command"
void cmd_let(void) {
	int t, size;
	MMFLOAT f;
    long long int i64;
	char *s;
	char *p1, *p2;
	int vartype; // effective type (may differ for struct members)

	p1 = cmdline;

	// search through the line looking for the equals sign
	while(*p1 && tokenfunction(*p1) != op_equal) p1++;
	if(!*p1) error("Unknown command");

	// check that we have a straight forward variable
	p2 = skipvar(cmdline, false);
	skipspace(p2);
	if(p1 != p2) error("Syntax");

	// create the variable and get the length if it is a string
	p2 = findvar(cmdline, V_FIND);
    size = vartbl[VarIndex].size;
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
#ifdef STRUCTENABLED
	// For struct member access, use the member type instead of the base variable type
	if (g_StructMemberType != 0)
	{
		vartype = g_StructMemberType;
		// For string members, use the size set by ResolveStructMember
		if (vartype & T_STR)
		{
			size = g_StructMemberSize;
		}
	}
	else
	{
		vartype = vartbl[VarIndex].type;
	}
#else
	vartype = vartbl[VarIndex].type;
#endif

	// step over the equals sign, evaluate the rest of the command and save in the variable
	p1++;
	if (vartype & T_STR) {
		t = T_STR;
		p1 = evaluate(p1, &f, &i64, &s, &t, false);
		if(*s > size) error("String too long");
		Mstrcpy(p2, s);
	}
	else if (vartype & T_NBR) {
		t = T_NBR;
		p1 = evaluate(p1, &f, &i64, &s, &t, false);
		if(t & T_NBR)
            (*(MMFLOAT *)p2) = f;
        else
            (*(MMFLOAT *)p2) = (MMFLOAT)i64;
	}
#ifdef STRUCTENABLED
	else if (vartype & T_STRUCT)
	{
		// Struct assignment - evaluate the right side and copy struct data
		// Save destination info BEFORE evaluate changes g_VarIndex
		int dest_struct_idx = (int)vartbl[VarIndex].size;
		int dest_struct_size = g_structtbl[dest_struct_idx]->total_size;

		t = T_NOTYPE; // Let evaluate determine the actual type
		p1 = evaluate(p1, &f, &i64, &s, &t, false);

		// Check that RHS is a struct
		if (!(t & T_STRUCT))
		{
			error("Expected a structure value");
		}

		// Validate struct types match (g_ExprStructType set by getvalue or function return)
		if (g_ExprStructType >= 0 && g_ExprStructType != dest_struct_idx)
		{
			error("Structure types must match");
		}

		if (s != NULL)
		{
			if (dest_struct_idx >= 0 && dest_struct_idx < g_structcnt)
			{
				memcpy(p2, s, dest_struct_size);
			}
			else
			{
				error("Invalid struct type");
			}
		}
		else
		{
			error("No struct value");
		}
	}
#endif

	else {
		t = T_INT;
		p1 = evaluate(p1, &f, &i64, &s, &t, false);
		if(t & T_INT)
            (*(long long int *)p2) = i64;
        else
            (*(long long int *)p2) = FloatToInt64(f);
	}
	checkend(p1);
}
int as_strcmpi (const char *s1, const char *s2)
{
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2)
    return 0;

  do
    {
      c1 = tolower (*p1++);
      c2 = tolower (*p2++);
      if (c1 == '\0')
	break;
    }
  while (c1 == c2);

  return c1 - c2;
}

void sortStrings(char **arr, int n)
{
    char temp[16];
    int i,j;
    // Sorting strings using bubble sort
    for (j=0; j<n-1; j++)
    {
        for (i=j+1; i<n; i++)
        {
            if (as_strcmpi(arr[j], arr[i]) > 0)
            {
                strcpy(temp, arr[j]);
                strcpy(arr[j], arr[i]);
                strcpy(arr[i], temp);
            }
        }
    }
}

void do_listfiles(char *ccmdline){
	int i, j, dirs, ListCnt, currentsize;
	uint32_t currentdate;
	char *p, /**q,*/ extension[8];
	int fcnt, sortorder=0;
	char ts[FF_MAX_LFN] = {0};
    char pp[FF_MAX_LFN] = {0};
    char q[FF_MAX_LFN]={0};
	s_flist *flist;
    static DIR djd;
    static FILINFO fnod;
	mymemset(&djd,0,sizeof(DIR));
	mymemset(&fnod,0,sizeof(FILINFO));
    fcnt = 0;
    if(*ccmdline){
    	getargs(&ccmdline,3,",");
    	if(!(argc==1 || argc==3))error("Syntax");
    	p = getCstring(argv[0]);
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
    	if(argc==3){
    		if(checkstring(argv[2], "NAME"))sortorder=0;
    		else if(checkstring(argv[2], "TIME"))sortorder=1;
    		else if(checkstring(argv[2], "SIZE"))sortorder=2;
    		else if(checkstring(argv[2], "TYPE"))sortorder=3;
    		else error("Syntax");
    	}
    }
    if(pp[0]==0)strcpy(pp,"*");
    if(CurrentLinePtr) error("Invalid in a program");
    if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
    flist=GetMemory(sizeof(s_flist)*MAXFILES);
//    	    q = GetCWD();
   	fullpath(q);
	MMPrintString("A:");
	MMPrintString(fullpathname);
    MMPrintString("\r\n");

    // search for the first file/dir
    FSerror = f_findfirst(&djd, &fnod, fullpathname, pp);
    ErrorCheck(0);
    // add the file to the list, search for the next and keep looping until no more files
    while(FSerror == FR_OK && fnod.fname[0]) {
        if(fcnt >= MAXFILES) {
        		FreeMemorySafe((void *)&flist);
            	f_closedir(&djd);
                error("Too many files to list");
        }
        if(!(fnod.fattrib & (AM_SYS | AM_HID))){
            // add a prefix to each line so that directories will sort ahead of files
            if(fnod.fattrib & AM_DIR){
                ts[0] = 'D';
                currentdate=0xFFFFFFFF;
                fnod.fdate=0xFFFF;
                fnod.ftime=0xFFFF;
                mymemset(extension,'+',sizeof(extension));
            	extension[sizeof(extension)-1]=0;
            } else {
                ts[0] = 'F';
                currentdate=(fnod.fdate<<16) | fnod.ftime;
                if(fnod.fname[strlen(fnod.fname)-1]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-1]);
                else if(fnod.fname[strlen(fnod.fname)-2]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-2]);
                else if(fnod.fname[strlen(fnod.fname)-3]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-3]);
                else if(fnod.fname[strlen(fnod.fname)-4]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-4]);
                else if(fnod.fname[strlen(fnod.fname)-5]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-5]);
                else {
                	mymemset(extension,'.',sizeof(extension));
                	extension[sizeof(extension)-1]=0;
                }
           }
            currentsize=fnod.fsize;
            // and concatenate the filename found
            strcpy(&ts[1], fnod.fname);
            // sort the file name into place in the array
            if(sortorder==0){
            	for(i = fcnt; i > 0; i--) {
            		if( strcicmp((flist[i - 1].fn), (ts)) > 0)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
            } else if(sortorder==2){
            	for(i = fcnt; i > 0; i--) {
            		if( (flist[i - 1].fs) > currentsize)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
            } else if(sortorder==3){
            	for(i = fcnt; i > 0; i--) {
            		char e2[8];
                    if(flist[i - 1].fn[strlen(flist[i - 1].fn)-1]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-1]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-2]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-2]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-3]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-3]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-4]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-4]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-5]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-5]);
                    else {
                    	if(flist[i - 1].fn[0]=='D'){
                    		mymemset(e2,'+',sizeof(e2));
                        	e2[sizeof(e2)-1]=0;
                    	} else {
                    		mymemset(e2,'.',sizeof(e2));
                        	e2[sizeof(e2)-1]=0;
                    	}
                    }
            		if( strcicmp((e2), (extension)) > 0)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
        	} else {
            	for(i = fcnt; i > 0; i--) {
            		if( ((flist[i - 1].fd<<16) |flist[i - 1].ft)  < currentdate)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}

        	}
            strcpy(flist[i].fn, ts);
            flist[i].fs = fnod.fsize;
            flist[i].fd = fnod.fdate;
            flist[i].ft = fnod.ftime;
            fcnt++;
        }
        FSerror = f_findnext(&djd, &fnod);
   }
    // list the files with a pause every screen full
	ListCnt = 2;
	for(i = dirs = 0; i < fcnt; i++) {
    	routinechecks(1);
		if(MMAbort) {
        	FreeMemorySafe((void *)&flist);
            f_closedir(&djd);
            WDTimer = 0;                                                // turn off the watchdog timer
            mymemset(inpbuf,0,STRINGSIZE);
        	SCB_CleanInvalidateDCache();
            longjmp(mark, 1);
        }
		if(flist[i].fn[0] == 'D') {
    		dirs++;
            MMPrintString("   <DIR>  ");
		}
		else {
		    IntToStrPad(ts, (flist[i].ft>>11)&0x1F, '0', 2, 10);
		    ts[2] = ':'; IntToStrPad(ts + 3, (flist[i].ft >>5)&0x3F, '0', 2, 10);
		    ts[5]=' ';
		    IntToStrPad(ts + 6, flist[i].fd & 0x1F, '0', 2, 10);
		    ts[8] = '-'; IntToStrPad(ts + 9, (flist[i].fd >> 5)&0xF, '0', 2, 10);
		    ts[11] = '-'; IntToStr(ts + 12 , ((flist[i].fd >> 9)& 0x7F )+1980, 10);
		    ts[16] =' ';
		    IntToStrPad(ts+17, flist[i].fs, ' ', 10, 10); MMPrintString(ts);
            MMPrintString("  ");
        }
        MMPrintString(flist[i].fn + 1);
		MMPrintString("\r\n");
		// check if it is more than a screen full
		if(++ListCnt >= Option.Height && i < fcnt) {
			MMPrintString("PRESS ANY KEY ...");
			MMgetchar(1);
			MMPrintString("\r                 \r");
			ListCnt = 1;
		}
	}
    // display the summary
    IntToStr(ts, dirs, 10); MMPrintString(ts);
    MMPrintString(" director"); MMPrintString(dirs==1?"y, ":"ies, ");
    IntToStr(ts, fcnt - dirs, 10); MMPrintString(ts);
    MMPrintString(" file"); MMPrintString((fcnt-dirs)==1?"":"s");
	MMPrintString("\r\n");
	FreeMemorySafe((void *)&flist);
    f_closedir(&djd);
    mymemset(inpbuf,0,STRINGSIZE);
	SCB_CleanInvalidateDCache();
    longjmp(mark, 1);
}
void cmd_listfiles(void){
	do_listfiles(cmdline);
}
void PIntFixed(int64_t n, int m) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    for(int i=strlen(s); i<m; i++)putConsole(' ');
    MMPrintString(s);
    MMPrintString(",");
}
void PFltFixed(MMFLOAT flt){
	   char s[20];
	   FloatToStr(s, flt, 10,1, ' ');
	    MMPrintString(s);
	    MMPrintString(",");
}
void PIntFixedFile(int64_t n, int m, int fnbr) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    for(int i=strlen(s); i<m; i++)FilePutChar(' ',fnbr);
    FilePutStr(strlen(s), s, fnbr);
    FilePutStr(1, ",", fnbr);
}
void PFltFixedFile(MMFLOAT flt, int fnbr){
	   char s[20];
	   FloatToStr(s, flt, 10,1, ' ');
	   FilePutStr(strlen(s), s, fnbr);
	   FilePutStr(1, ",", fnbr);
}


void MIPS16 cmd_list(void) {
	char *p, *tp;
	int i,j,k,m, step=5;
	hidecursor(0);
	FreeMemorySafe((void *)&cursorsave);
	cursorenable=0;
    int maxW=PageTable[WritePage].xmax;
    if((p = checkstring(cmdline, "ALL"))) {
        if(!(*p == 0 || *p == '\'')) {
        	getargs(&p,1,",");
			char *qq=GetTempStrMemory();
        	char *buff=GetTempStrMemory();
        	strcpy(buff,getCstring(argv[0]));
			getfullfilepath(buff,qq);
    		if(strchr(qq, '.') == NULL) strcat(buff, ".BAS");
            ListFile(qq, true);
        } else {
        	ListProgram((char *)ProgMemory, true);
        	checkend(p);
        }
        cleanend();
    } else if((p = checkstring(cmdline, "FLASH ALL"))) {
        ListProgramFlash((char *)ProgMemory, true);
        checkend(p);
    } else if(checkstring(cmdline, "PROFILE CSV")) {
    	if(Option.profile==0)error("Profiling not enabled");
        static uint32_t *lastprofile=NULL;
    	char b[STRINGSIZE],fn[FF_MAX_LFN]={0};
    	char *pp;
    	p=ProgMemory;
    	if(*p != 1)return;
    	if(p[1] != 39)return;
        int fnbr=0;
    	while(!(*p == 0 || *p == 0xff)) {                               // normally a LIST ends at the break so this is a safety precaution
            if(*p == T_NEWLINE) {
				uint32_t *profileadd=(uint32_t *)((uint32_t)(p-firststmt+(G1Hardware? 0xD0300000 : 0xD0700000)) & (~3));
				if((uint32_t)profileadd-(uint32_t)lastprofile==4)profileadd++;
				lastprofile=profileadd;
				if(p!=firststmt){
					PIntFixedFile(profileadd[0],10, fnbr);
					PFltFixedFile((MMFLOAT)profileadd[1]/(MMFLOAT)profileadd[0], fnbr);
					FilePutStr(3,"  \"",fnbr);
				} else {
					strcpy(fn,&p[2]);
					fn[strlen(fn)-3]='C';
					fn[strlen(fn)-2]='S';
					fn[strlen(fn)-1]='V';
					if(!CurrentLinePtr){
						MMPrintString("Creating ");
						MMPrintString(fn);
						PRet();
					}
				    fnbr = FindFreeFileNbr();
			        if(!BasicFileOpen(fn, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
				}
    			p = llist(b, p);                                        // otherwise expand the line
    			pp = b;
				while(*pp) {
					if((*pp=='\'' && pp[1]=='/')){
						pp++;
					}
					else if(!(*pp=='\'' && pp[1]=='|')){
						FilePutChar(*pp++,fnbr);
					}
					else {
						FilePutChar('"',fnbr);
						FilePutChar(',',fnbr);
						pp+=2;
						if(IsDigit(*pp))FilePutChar(',',fnbr);
					}
				}
				FilePutStr(2,"\r\n",fnbr);
				routinechecks(1);
    			if(p[0] == 0 && p[1] == 0) break;                       // end of the listing ?
    		}
    	}
        FileClose(fnbr);
        checkend(p);
    } else if(checkstring(cmdline, "PROFILE")) {
        if(CurrentLinePtr) error("Invalid in a program");
    	if(Option.profile==0)error("Profiling not enabled");
        static uint32_t *lastprofile=NULL;
    	char b[STRINGSIZE];
    	char *pp;
        int ListCnt = 1;
    	p=ProgMemory;
    	if(*p != 1)return;
    	if(p[1] != 39)return;
    	while(!(*p == 0 || *p == 0xff)) {                               // normally a LIST ends at the break so this is a safety precaution
            if(*p == T_NEWLINE) {
				uint32_t *profileadd=(uint32_t *)((uint32_t)(p-firststmt+(G1Hardware? 0xD0300000 : 0xD0700000)) & (~3));
				if((uint32_t)profileadd-(uint32_t)lastprofile==4)profileadd++;
				lastprofile=profileadd;
				if(p!=firststmt){
					PIntFixed(profileadd[0],10);
					PFltFixed((MMFLOAT)profileadd[1]/(MMFLOAT)profileadd[0]);
					MMPrintString("  \"");
				}
    			p = llist(b, p);                                        // otherwise expand the line
    			pp = b;
				while(*pp) {
					if(MMCharPos >= Option.Width) ListNewLine(&ListCnt, 0);
					if((*pp=='\'' && pp[1]=='/')){
						pp++;
					}
					else if(!(*pp=='\'' && pp[1]=='|')){
						MMputchar(*pp++);
					}
					else {
						MMputchar('"');
						MMputchar(',');
						pp+=2;
						if(IsDigit(*pp))MMputchar(',');
					}
				}
				routinechecks(1);
                ListNewLine(&ListCnt, 0);
    			if(p[0] == 0 && p[1] == 0) break;                       // end of the listing ?
    		}
    	}
        checkend(p);
    } else if((tp = checkstring(cmdline,"FILES"))){
    	do_listfiles(tp);
    } else if((tp = checkstring(cmdline,"PAGES"))){
    	PO("MODE ",3);PInt(VideoMode);putConsole(',');PInt(VideoColour);MMPrintString(" has ");PInt(LastPage+1);MMPrintString(" pages\r\n");
    	MMPrintString("Page no.   Page Address   Width   Height   Size   Lines");PRet();
    	for(int i=0;i<=LastPage;i++){
    		putConsole(' ');
    		if(i<10)putConsole(' ');
    		PInt(i);
    		MMPrintString("         &H");
    		PIntH((uint32_t)PageTable[i].address);
    		if(PageTable[i].xmax<1000)MMPrintString("     ");
    		else MMPrintString("    ");
    		PInt((uint32_t)PageTable[i].xmax);
    		MMPrintString("     ");
    		PInt((uint32_t)PageTable[i].ymax);
    		if(PageTable[i].size<0xFFFF)putConsole(' ');
    		MMPrintString("    &H");
    		PIntH((uint32_t)PageTable[i].size);
    		if(PageTable[i].size<0xFFFFF)MMPrintString("   ");
    		else MMPrintString("  ");
    		putConsole(PageTable[i].expand ? '2' : '1');
    		PRet();
    	}
    } else if((p = checkstring(cmdline, "FLASH"))) {
        ListProgramFlash((char *)ProgMemory, false);
        checkend(p);
    } else if((p = checkstring(cmdline, "COMMANDS"))) {
    	step=maxW/gui_font_width/20;
    	m=0;
    	//int x=6;
    	int x=8;
		char** c=GetTempMemory((CommandTableSize+x)*sizeof(*c)+(CommandTableSize+x)*20);
		for(i=0;i<CommandTableSize+x;i++){
				c[m]= (char *)((int)c + sizeof(char *) * (CommandTableSize+x) + m*20);
    			if(m<CommandTableSize)strcpy(c[m],commandtbl[i].name);
    			//else if(m==CommandTableSize)strcpy(c[m],"HUMID");
    			else if(m==CommandTableSize)strcpy(c[m],"ELSE IF");
    			else if(m==CommandTableSize+1)strcpy(c[m],"END IF");
    			else if(m==CommandTableSize+2)strcpy(c[m],"Exit Do");
    			else if(m==CommandTableSize+3)strcpy(c[m],"Cat");
    			else if(m==CommandTableSize+4)strcpy(c[m],"Bit(");
    			else if(m==CommandTableSize+5)strcpy(c[m],"Byte(");
    			else if(m==CommandTableSize+6)strcpy(c[m],"Flag(");
    			//else if(m==CommandTableSize+5)strcpy(c[m],"I2C2");
    			//else if(m==CommandTableSize+6)strcpy(c[m],"I2C3");
    			//else if(m==CommandTableSize+7)strcpy(c[m],"SPI2");
    			//else if(m==CommandTableSize+8)strcpy(c[m],"Erase");
    			else strcpy(c[m],"Sprite");
    			m++;
		}
    	sortStrings(c,m);
    	for(i=1;i<m-1;i+=step){   //hide last command
    		for(k=0;k<step;k++){
        		if(i+k<m-1){  //hide last command
        			MMPrintString(c[i+k]);
        			if(k!=(step-1))for(j=strlen(c[i+k]);j<19;j++)putConsole(' ');
        		}
    		}
    		MMPrintString("\r\n");
    	}
		MMPrintString("Total of ");PInt(m-1);MMPrintString(" commands\r\n");
		if (checkstring(cmdline, "COMMANDS V")){
			 MMPrintString("Total of ");PInt(m-1-x);MMPrintString(" tokens\r\n");
		}

      } else if((p = checkstring(cmdline, "FUNCTIONS"))) {
       	step=maxW/gui_font_width/20;
     	m=0;
     	//int x=2+MMEND;
     	int x=6+MMEND;

		char** c=GetTempMemory((TokenTableSize+x)*sizeof(*c)+(TokenTableSize+x)*20);
		for(i=0;i<TokenTableSize+x;i++){
				c[m]= (char *)((int)c + sizeof(char *) * (TokenTableSize+x) + m*20);
    			if(m<TokenTableSize)strcpy(c[m],tokentbl[i].name);
    			else if(m<TokenTableSize+MMEND && m>=TokenTableSize)strcpy(c[m],overlaid_functions[i-TokenTableSize]);
    			//       			else if(m==TokenTableSize+MMEND)strcpy(c[m],"=<");
    			//       			else if(m==TokenTableSize+MMEND+1)strcpy(c[m],"=>");
    			//       			else strcpy(c[m],"MM.Info$(");
    			//else if(m==TokenTableSize+MMEND)strcpy(c[m],"MM.Errno");
    			//else if(m==TokenTableSize+MMEND+1)strcpy(c[m],"MM.Onewire");
    			else if(m==TokenTableSize+MMEND)strcpy(c[m],"OCT$(");
    			else if(m==TokenTableSize+MMEND+1)strcpy(c[m],"HEX$(");
    			else if(m==TokenTableSize+MMEND+2)strcpy(c[m],"BIN$(");
    			else if(m==TokenTableSize+MMEND+3)strcpy(c[m],"Bit(");
    			else if(m==TokenTableSize+MMEND+4)strcpy(c[m],"Byte(");
    			else if(m==TokenTableSize+MMEND+5)strcpy(c[m],"Flag(");
    			//else if(m==TokenTableSize+MMEND+3)strcpy(c[m],"MM.I2C");
    			//else strcpy(c[m],"MM.Errmsg$");
				m++;
		}
    	sortStrings(c,m);
    	for(i=1;i<m-2;i+=step){
    	//for(i=1;i<m-0;i+=step){
    		for(k=0;k<step;k++){
        		if(i+k<m-2){
        		//if(i+k<m-0){
        			MMPrintString(c[i+k]);
        			if(k!=(step-1))for(j=strlen(c[i+k]);j<19;j++)putConsole(' ');
        		}
    		}
    		MMPrintString("\r\n");
    	}
		MMPrintString("Total of ");PInt(m-2);MMPrintString(" functions and operators\r\n");
		if (checkstring(cmdline, "FUNCTIONS V")){
			  MMPrintString("Total of ");PInt(m-x-1);MMPrintString(" tokens\r\n");
		}

    } else {
        if(!(*cmdline == 0 || *cmdline == '\'')) {
        	getargs(&cmdline,1,",");
			char *qq=GetTempStrMemory();
        	char *buff=GetTempStrMemory();
        	strcpy(buff,getCstring(argv[0]));
			getfullfilepath(buff,qq);
    		if(strchr(qq, '.') == NULL) strcat(buff, ".BAS");
			ListFile(qq, false);
        } else {
        	ListProgram((char *)ProgMemory, false);
        	checkend(cmdline);
        }
        cleanend();
    }
}


void ListNewLine(int *ListCnt, int all) {
	MMPrintString("\r\n");
	(*ListCnt)++;
    if(!all && *ListCnt >= Option.Height-(Option.showstatus ? 2:0)) {
    	MMPrintString("PRESS ANY KEY ...");
    	MMgetchar(1);
    	MMPrintString("\r                 \r");
    	*ListCnt = 1;
    }
}
void MIPS16 ListProgramFlash(char *p, int all) {
	char b[STRINGSIZE];
	char *pp;
    int ListCnt = 1;

	while(!(*p == 0 || *p == 0xff)) {                               // normally a LIST ends at the break so this is a safety precaution
        if(*p == T_NEWLINE) {
			p = llist(b, p);                                        // otherwise expand the line
			pp = b;
			while(*pp) {
				if(MMCharPos >= Option.Width) ListNewLine(&ListCnt, all);
				MMputchar(*pp++);
			}
        	routinechecks(1);
            ListNewLine(&ListCnt, all);
			if(p[0] == 0 && p[1] == 0) break;                       // end of the listing ?
		}
	}
}



void MIPS16 ListProgram(char *pp, int all) {
	char buff[STRINGSIZE];
    FRESULT fr;
    FILINFO fno;
    int fnbr;
    int i,ListCnt = 1;
	if(*pp++ != 1)return;
	if(*pp++ != 39)return;
    fr = f_stat(pp, &fno);
    if(fr == FR_OK  && !(fno.fattrib & AM_DIR)){
    	fnbr = FindFreeFileNbr();
    	if(!BasicFileOpen(pp, fnbr, FA_READ)) return;
    	while(!FileEOF(fnbr)) {                                     // while waiting for the end of file
        	routinechecks(1);
    		mymemset(buff,0,256);
    		MMgetline(fnbr, (char *)buff);									    // get the input line
    		for(i=0;i<strlen(buff);i++)if(buff[i] == TAB) buff[i] = ' ';
    		MMPrintString(buff);
    		ListCnt+=strlen(buff)/Option.Width;
            ListNewLine(&ListCnt, all);
    	}
    	FileClose(fnbr);
    } else error("File not found");
}

void MIPS16 ListFile(char *pp, int all) {
	char buff[STRINGSIZE];
    FRESULT fr;
    FILINFO fno;
    int fnbr;
    int i,ListCnt = 1;
    fr = f_stat(pp, &fno);
    if(fr == FR_OK  && !(fno.fattrib & AM_DIR)){
    	fnbr = FindFreeFileNbr();
    	if(!BasicFileOpen(pp, fnbr, FA_READ)) return;
    	while(!FileEOF(fnbr)) {                                     // while waiting for the end of file
    		mymemset(buff,0,256);
    		MMgetline(fnbr, (char *)buff);									    // get the input line
    		for(i=0;i<strlen(buff);i++)if(buff[i] == TAB) buff[i] = ' ';
    		MMPrintString(buff);
    		ListCnt+=strlen(buff)/Option.Width;
        	routinechecks(1);
            ListNewLine(&ListCnt, all);
    	}
    	FileClose(fnbr);
    } else error("File not found");
}



#ifdef H7RUN
/* From H7 takes a filename    */
void MIPS16 cmd_run(void) {
    // RUN [ filename$ ] [, cmd_args$ ]
    char *filename = "", *cmd_args = "";
    getargs(&cmdline, 3, ",");
    switch (argc) {
        case 0:
            break;
        case 1:
            filename = getCstring(argv[0]);
            break;
        case 2:
            cmd_args = getCstring(argv[1]);
            break;
        default:
            filename = getCstring(argv[0]);
            cmd_args = getCstring(argv[2]);
            break;
    }

    // The memory allocated by getCstring() is not preserved across
    // a call to FileLoadProgram() so we need to cache 'filename' and
    // 'cmd_args' on the stack.
    char buf[MAXSTRLEN + 1];
    if (snprintf(buf, MAXSTRLEN + 1, "\"%s\",%s", filename, cmd_args) > MAXSTRLEN) {
        error("RUN command line too long");
    }
    //char *pcmd_args = buf + strlen(filename) + 2;
    char *pcmd_args = buf + strlen(filename) + 3; // *** THW 16/4/23

    if (*filename && !FileLoadProgram(buf)) return;
    ScrewUpTimer=0;
    ClearRuntime();
    WatchdogSet = false;
	PrepareProgram(true);
    // Create a global constant MM.CMDLINE$ containing 'cmd_args'.
   // void *ptr = findvar("MM.CMDLINE$", V_FIND | V_DIM_VAR | T_CONST);
    CtoM(pcmd_args);
    //memcpy(ptr, pcmd_args + 1, *pcmd_args);
    // memcpy(ptr, pcmd_args, *pcmd_args + 1); // *** THW 16/4/23
    Mstrcpy(cmdlinebuff, pcmd_args);
   // MMPrintString(cmdlinebuff);PRet();
    IgnorePIN = false;
    if(Option.ProgFlashSize != PROG_FLASH_SIZEMAX) ExecuteProgram(ProgMemory + Option.ProgFlashSize);       // run anything that might be in the library
    if(*ProgMemory != T_NEWLINE) return;                            // no program to run
    nextstmt = ProgMemory;
}
#endif
#define OLDRUN
#ifdef NEWRUN

void MIPS16 cmd_run(void) {
	 // RUN [ filename$ ] [, cmd_args$ ]
    char *filename = "", *cmd_args = "";
    getargs(&cmdline, 3, ",");

	char *c=(char *)ProgMemory;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
	SerUSBPutS("\033[?25h");
	SerUSBPutS("\033[37m");
	SerUSBPutS("\033[m");
	MM_Delay(3);
    ScrewUpTimer=0;
	while(MMInkey()!=-1){}
	if(Option.showstatus){
	    ShortScroll=0;
		ShowCursor(false);
		int lastx=CurrentX,lasty=CurrentY;
		DrawRectangle( 0,maxH-gui_font_height-1,maxW-1,maxH-1,BLACK);
		CurrentY=lasty;
		CurrentX=lastx;
	}

	//
	   // RUN [ filename$ ] [, cmd_args$ ]
	  //  char *filename = "", *cmd_args = "";
	  //  getargs(&cmdline, 3, ",");
	    switch (argc) {
	        case 0:
	            break;
	        case 1:
	            filename = getCstring(argv[0]);
	            break;
	        case 2:
	            cmd_args = getCstring(argv[1]);
	            break;
	        default:
	            filename = getCstring(argv[0]);
	            cmd_args = getCstring(argv[2]);
	            break;
	    }

	    // The memory allocated by getCstring() is not preserved across
	    // a call to FileLoadProgram() so we need to cache 'filename' and
	    // 'cmd_args' on the stack.
	    char buf[MAXSTRLEN + 1];
	    if (snprintf(buf, MAXSTRLEN + 1, "\"%s\",%s", filename, cmd_args) > MAXSTRLEN) {
	        error("RUN command line too long");
	    }
	    //char *pcmd_args = buf + strlen(filename) + 2;
	    char *pcmd_args = buf + strlen(filename) + 3; // *** THW 16/4/23
	    MMPrintString(pcmd_args);PRet();

	// CtoM(pcmd_args);
	//skipspace(cmdline);
	mymemset(runcmd,0,STRINGSIZE);
	//memcpy(runcmd,cmdline,strlen(cmdline));
	//memcpy(runcmd,pcmd_args,strlen(pcmd_args));
	memcpy(runcmd,buf,strlen(buf));
	//if(*cmdline == 34){
	MMPrintString(buf);PRet();
	MMPrintString(runcmd);PRet();
	if(*filename){
		if(!FileLoadProgram(runcmd,1)) return;
	} else {
		if(*c++ != 1)error("Nothing to run");
		if(*c++ != 39)error("Nothing to run");
		if(!FileLoadProgram((char *)&ProgMemory[2],0)) return;
	}
	ClearRuntime();

	CtoM(pcmd_args);
	MMPrintString(pcmd_args);PRet();
	//Mstrcpy(cmdlinebuff, pcmd_args);
	memcpy(runcmd,pcmd_args,strlen(pcmd_args));

    WatchdogSet = false;
	PrepareProgram(true);
    IgnorePIN = false;
    if(*ProgMemory != T_NEWLINE) return;                             // no program to run
    setmode(Option.mode,DEFCOLOUR,0,Option.mode==12?1:0);
    if((uint32_t)PageTable[0].address==0xD0000000){
    	mymemset((char *)0x24000000,0,512*1024);
    	mymemset((char *)0xD0000000+PageTable[0].size,0,512*6*1024-PageTable[0].size);
    } else {
    	mymemset((char *)0xD0000000,0,512*6*1024);
    	mymemset((char *)0x24000000+PageTable[0].size,0,512*1024-PageTable[0].size);
    }
    if(Option.profile){
    	if(G1Hardware)mymemset((char *)0xD0300000, 0, 512*1024);
    	else mymemset((char *)0xD0700000, 0, 512*1024);
    }
    turtle_init();  //turtle
    statement_time= ReadCoreTimer() + FastTimer;
    firststmt= (char *)ProgMemory;
    nextstmt = (char *)ProgMemory;
}


#endif

#ifdef OLDRUN
void MIPS16 cmd_run(void) {
	char *c=(char *)ProgMemory;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
	SerUSBPutS("\033[?25h");
	SerUSBPutS("\033[37m");
	SerUSBPutS("\033[m");
	MM_Delay(3);
    ScrewUpTimer=0;
	while(MMInkey()!=-1){}
	if(Option.showstatus){
	    ShortScroll=0;
		ShowCursor(false);
		int lastx=CurrentX,lasty=CurrentY;
		DrawRectangle( 0,maxH-gui_font_height-1,maxW-1,maxH-1,BLACK);
		CurrentY=lasty;
		CurrentX=lastx;
	}

	skipspace(cmdline);
	mymemset(runcmd,0,STRINGSIZE);
	memcpy(runcmd,cmdline,strlen(cmdline));
	if(*cmdline == 34){
		if(!FileLoadProgram(cmdline,1)) return;
	} else {
		if(*c++ != 1)error("Nothing to run");
		if(*c++ != 39)error("Nothing to run");
		if(!FileLoadProgram((char *)&ProgMemory[2],0)) return;
	}
	ClearRuntime();
    WatchdogSet = false;
	PrepareProgram(true);
    IgnorePIN = false;
    if(*ProgMemory != T_NEWLINE) return;                             // no program to run
    setmode(Option.mode,DEFCOLOUR,0,Option.mode==12?1:0);
    if((uint32_t)PageTable[0].address==0xD0000000){
    	mymemset((char *)0x24000000,0,512*1024);
    	mymemset((char *)0xD0000000+PageTable[0].size,0,512*6*1024-PageTable[0].size);
    } else {
    	mymemset((char *)0xD0000000,0,512*6*1024);
    	mymemset((char *)0x24000000+PageTable[0].size,0,512*1024-PageTable[0].size);
    }
    if(Option.profile){
    	if(G1Hardware)mymemset((char *)0xD0300000, 0, 512*1024);
    	else mymemset((char *)0xD0700000, 0, 512*1024);
    }
    turtle_init();  //turtle
    statement_time= ReadCoreTimer() + FastTimer;
    firststmt= (char *)ProgMemory;
    nextstmt = (char *)ProgMemory;
}
#endif

void MIPS16 cmd_continue(void) {
    if(*cmdline == tokenFOR) {
        if(forindex == 0) error("No FOR loop is in effect");
        nextstmt = forstack[forindex - 1].nextptr;
        return;
    }
    if(checkstring(cmdline, "DO")) {
        if(doindex == 0) error("No DO loop is in effect");
        nextstmt = dostack[doindex - 1].loopptr;
        return;
    }
    // must be a normal CONTINUE
	checkend(cmdline);
	if(CurrentLinePtr) error("Invalid in a program");
	if(ContinuePoint == NULL) error("Cannot continue");
    IgnorePIN = false;
	nextstmt = ContinuePoint;
}

void cmd_goto(void) {
	if(isnamestart(*cmdline))
		nextstmt = findlabel(cmdline);								// must be a label
	else
		nextstmt = findline(getinteger(cmdline), true);				// try for a line number
    IgnorePIN = false;
}

void cmd_call(void){
	int i;
	char *q;
	char *p=getCstring(cmdline); //get the command we want to call
	q=p;
	while(*q){ //convert to upper case for the match
		*q=toupper(*q);
		q++;
	}
	q=cmdline;
	while(*q){
		if(*q==',' || *q=='\'')break;
		q++;
	}
	if(*q==',')q++;
	i = FindSubFun(p, false);                   // it could be a defined command
	strcat(p," ");
	strcat(p,q);
//	MMPrintString(p);PRet();
	if(i >= 0) {                                // >= 0 means it is a user defined command
		DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
	}
	else
		error("Unknown user subroutine");
}
// Single byte tokens.
// char tokenTHEN, tokenELSE, tokenGOTO, tokenEQUAL, tokenTO, tokenSTEP;
// char tokenWHILE, tokenUNTIL, tokenGOSUB, tokenAS, tokenFOR;
void cmd_if(void) {
	int r, i, testgoto, testelseif;
	char ss[3];														// this will be used to split up the argument line
	char *p, *tp;
	char *rp = NULL;

	ss[0] = tokenTHEN;
	ss[1] = tokenELSE;
	ss[2] = 0;

	testgoto = false;
	testelseif = false;

retest_an_if:
	{																// start a new block
		getargs(&cmdline, 20, ss);									// getargs macro must be the first executable stmt in a block

		if(testelseif && argc > 2) error("Unexpected text");

		// if there is no THEN token retry the test with a GOTO.  If that fails flag an error
		if(argc < 2 || *argv[1] != ss[0]) {
			if(testgoto) error("IF without THEN");
			ss[0] = tokenGOTO;
			testgoto = true;
			goto retest_an_if;
		}


		// allow for IF statements embedded inside this IF

		//if(argc >= 3 && *argv[2] == cmdIF) argc = 3;                // this is IF xx=yy THEN IF ... so we want to evaluate only the first 3
		if (argc >= 3 && commandtbl_decode(argv[2]) == cmdIF)         // CMD16BIT
			argc = 3; // this is IF xx=yy THEN IF ... so we want to evaluate only the first 3

		//if(argc >= 5 && *argv[4] == cmdIF) argc = 5;                // this is IF xx=yy THEN cmd ELSE IF ... so we want to evaluate only the first 5
		if (argc >= 5 && commandtbl_decode(argv[4]) == cmdIF)         // CMD16BIT
			argc = 5; // this is IF xx=yy THEN cmd ELSE IF ... so we want to evaluate only the first 5

		if(argc == 4 || (argc == 5 && *argv[3] != ss[1])) error("Syntax");  //i.e. tokenELSE

		r = (getnumber(argv[0]) != 0);								// evaluate the expression controlling the if statement

		if(r) {
			// the test returned TRUE
			// first check if it is a multiline IF (ie, only 2 args)
			if(argc == 2) {
				// if multiline do nothing, control will fall through to the next line (which is what we want to execute next)
				;
			}
			else {
				// This is a standard single line IF statement
				// Because the test was TRUE we are just interested in the THEN cmd stage.
				if(*argv[1] == tokenGOTO) {
					cmdline = argv[2];
					cmd_goto();
					return;
				} else if(IsDigitinline(*argv[2])) {
					nextstmt = findline(getinteger(argv[2]), true);
				} else {
					if(argc == 5) {
						// this is a full IF THEN ELSE and the statement we want to execute is between the THEN & ELSE
						// this is handled by a special routine
						execute_one_command(argv[2]);
					} else {
						// easy - there is no ELSE clause so just point the next statement pointer to the byte after the THEN token
						for(p = cmdline; *p && *p != ss[0]; p++);	// search for the token
						nextstmt = p + 1;							// and point to the byte after
					}
				}
			}
		} else {
			// the test returned FALSE so we are just interested in the ELSE stage (if present)
			// first check if it is a multiline IF (ie, only 2 args)
			if(argc == 2) {
				// search for the next ELSE, or ENDIF and pass control to the following line
				// if an ELSEIF is found re execute this function to evaluate the condition following the ELSEIF
				i = 1; p = nextstmt;
				while(1) {
                    p = GetNextCommand(p, &rp, "No matching ENDIF");
                    CommandToken tkn = commandtbl_decode(p);
					if(tkn == cmdtoken) {
						// found a nested IF command, we now need to determine if it is a single or multiline IF
						// search for a THEN, then check if only white space follows.  If so, it is multiline.
						tp = p + sizeof(CommandToken);
						while(*tp && *tp != ss[0]) tp++;
						if(*tp) tp++;								// step over the THEN
						skipspace(tp);
						if(*tp == 0 || *tp == '\'')					// yes, only whitespace follows
							i++;									// count it as a nested IF
						else										// no, it is a single line IF
							skipelement(p);							// skip to the end so that we avoid an ELSE
						continue;
					}

					if(tkn == cmdELSE && i == 1) {
						// found an ELSE at the same level as this IF.  Step over it and continue with the statement after it
						skipelement(p);
						nextstmt = p;
						break;
					}

					if((tkn == cmdELSEIF/* || *p == cmdELSE_IF*/) && i == 1) {
						// we have found an ELSEIF statement at the same level as our IF statement
						// setup the environment to make this function evaluate the test following ELSEIF and jump back
						// to the start of the function.  This is not very clean (it uses the dreaded goto for a start) but it works
						p += sizeof(CommandToken); // step over the token
						skipspace(p);
						CurrentLinePtr = rp;
						if(*p == 0) error("Syntax");                // there must be a test after the elseif
						cmdline = p;
						skipelement(p);
						nextstmt = p;
						testgoto = false;
						testelseif = true;
						goto retest_an_if;
					}

					if(tkn == cmdENDIF/* || *p == cmdEND_IF*/) i--;						// found an ENDIF so decrement our nested counter
					if(i == 0) {
						// found our matching ENDIF stmt.  Step over it and continue with the statement after it
						skipelement(p);
						nextstmt = p;
						break;
					}
				}
			}
			else {
				// this must be a single line IF statement
				// check if there is an ELSE on the same line
				if(argc == 5) {
					// there is an ELSE command
					if(IsDigitinline(*argv[4]))
						// and it is just a number, so get it and find the line
						nextstmt = findline(getinteger(argv[4]), true);
					else
					{
#ifndef CMD16BIT
						// there is a statement after the ELSE clause  so just point to it (the byte after the ELSE token)
						for(p = cmdline; *p && *p != ss[1]; p++);	// search for the token
						nextstmt = p + 1;							// and point to the byte after

#else
						// IF <condition> THEN <statement1> ELSE <statement2>
						// Find and read the THEN function token.
						for (p = cmdline; *p && *p != ss[0]; p++)
						{
						}
						// Skip the command that <statement1> must start with.
						p++;
						skipspace(p);
						p += sizeof(CommandToken);
						// Find and read the ELSE function token.
						for (; *p && *p != ss[1]; p++)
							;
						nextstmt = p + 1; // The statement after the ELSE token.
#endif
					}
                } else {
                    // no ELSE on a single line IF statement, so just continue with the next statement
                    skipline(cmdline);
                    nextstmt = cmdline;
                }
			}
		}
	}
}



void cmd_else(void) {
	int i;
	char *p, *tp;

	// search for the next ENDIF and pass control to the following line
	i = 1; p = nextstmt;

	if(cmdtoken ==  cmdELSE) checkend(cmdline);

	while(1) {
        p = GetNextCommand(p, NULL, "No matching ENDIF");
        CommandToken tkn = commandtbl_decode(p);
		if(tkn == cmdIF) {
			// found a nested IF command, we now need to determine if it is a single or multiline IF
			// search for a THEN, then check if only white space follows.  If so, it is multiline.
			tp = p + sizeof(CommandToken);
			while(*tp && *tp != tokenTHEN) tp++;
			if(*tp) tp++;											// step over the THEN
			skipspace(tp);
			if(*tp == 0 || *tp == '\'')								// yes, only whitespace follows
				i++;												// count it as a nested IF
		}
		if(tkn == cmdENDIF/* || tkn == cmdEND_IF*/) i--;				    // found an ENDIF so decrement our nested counter
		if(i == 0) break;											// found our matching ENDIF stmt
	}
	// found a matching ENDIF.  Step over it and continue with the statement after it
	skipelement(p);
	nextstmt = p;
}



void cmd_end(void) {
	checkend(cmdline);
	cleanend();
}



void cmd_select(void) {
    int i, type;
    char *p, *rp = NULL, *SaveCurrentLinePtr;
    void *v;
    MMFLOAT f = 0;
    long long int i64 = 0;
    char s[STRINGSIZE];

    // these are the tokens that we will be searching for
    // they are cached the first time this command is called

    type = T_NOTYPE;
    v = DoExpression(cmdline, &type);                               // evaluate the select case value
    type = TypeMask(type);
    if(type & T_NBR) f = *(MMFLOAT *)v;
    if(type & T_INT) i64 = *(long long int *)v;
    if(type & T_STR) Mstrcpy(s, (char *)v);

    // now search through the program looking for a matching CASE statement
    // i tracks the nesting level of any nested SELECT CASE commands
    SaveCurrentLinePtr = CurrentLinePtr;                            // save where we are because we will have to fake CurrentLinePtr to get errors reported correctly
    i = 1; p = nextstmt;
    while(1) {
        p = GetNextCommand(p, &rp, "No matching END SELECT");
        CommandToken tkn = commandtbl_decode(p);
        if(tkn == cmdSELECT_CASE) i++;                               // found a nested SELECT CASE command, increase the nested count and carry on searching

        // is this a CASE stmt at the same level as this SELECT CASE.
        if(tkn == cmdCASE && i == 1) {
            int t;
            MMFLOAT ft, ftt;
            long long int i64t, i64tt;
            char *st, *stt;

            CurrentLinePtr = rp;                                    // and report errors at the line we are on
#ifdef CMD16BIT
            p++;				 // step past rest of command token
#endif
            // loop through the comparison elements on the CASE line.  Each element is separated by a comma
            do {
                p++;
                skipspace(p);
                t = type;
                // check for CASE IS,  eg  CASE IS > 5  -or-  CASE > 5  and process it if it is
                // an operator can be >, <>, etc but it can also be a prefix + or - so we must not catch them
                if((SaveCurrentLinePtr = checkstring(p, "IS")) || ((tokentype(*p) & T_OPER) && !(*p == GetTokenValue("+") || *p == GetTokenValue("-")))) {
                    int o;
                    if(SaveCurrentLinePtr) p += 2;
                    skipspace(p);
                    if(tokentype(*p) & T_OPER)
                        o = *p++ - C_BASETOKEN;                     // get the operator
                    else
                        error("Syntax");
                    if(type & T_NBR) ft = f;
                    if(type & T_INT) i64t = i64;
                    if(type & T_STR) st = s;
                    while(o != E_END) p = doexpr(p, &ft, &i64t, &st, &o, &t); // get the right hand side of the expression and evaluate the operator in o
                    if(!(t & T_INT)) error("Syntax");     			// comparisons must always return an integer
                    if(i64t) {                                      // evaluates to true
                        skipelement(p);
                        nextstmt = p;
                        CurrentLinePtr = SaveCurrentLinePtr;
                        return;                                     // if we have a match just return to the interpreter and let it execute the code
                    } else {                                        // evaluates to false
                        skipspace(p);
                        continue;
                    }
                }

                // it must be either a single value (eg, "foo") or a range (eg, "foo" TO "zoo")
                // evaluate the first value
                p = evaluate(p, &ft, &i64t, &st, &t, true);
                skipspace(p);
                if(*p == tokenTO) {                      			// is there is a TO keyword?
                    p++;
                    t = type;
                    p = evaluate(p, &ftt, &i64tt, &stt, &t, false); // evaluate the right hand side of the TO expression
                    if(((type & T_NBR) && f >= ft && f <= ftt) || ((type & T_INT) && i64 >= i64t && i64 <= i64tt) || (((type & T_STR) && Mstrcmp(s, st) >= 0) && (Mstrcmp(s, stt) <= 0))) {
                        skipelement(p);
                        nextstmt = p;
                        CurrentLinePtr = SaveCurrentLinePtr;
                        return;                                     // if we have a match just return to the interpreter and let it execute the code
                    } else {
                        skipspace(p);
                        continue;                                   // otherwise continue searching
                    }
                }

                // if we got to here the element must be just a single match.  So make the test
                if(((type & T_NBR) && f == ft) ||  ((type & T_INT) && i64 == i64t) ||  ((type & T_STR) && Mstrcmp(s, st) == 0)) {
                    skipelement(p);
                    nextstmt = p;
                    CurrentLinePtr = SaveCurrentLinePtr;
                    return;                                         // if we have a match just return to the interpreter and let it execute the code
                }
                skipspace(p);
            } while(*p == ',');                                     // keep looping through the elements on the CASE line
            checkend(p);
            CurrentLinePtr = SaveCurrentLinePtr;
        }

        // test if we have found a CASE ELSE statement at the same level as this SELECT CASE
        // if true it means that we did not find a matching CASE - so execute this code
        if(tkn == cmdCASE_ELSE && i == 1) {
			p += sizeof(CommandToken); // step over the token
            checkend(p);
            skipelement(p);
            nextstmt = p;
            CurrentLinePtr = SaveCurrentLinePtr;
            return;
        }

        if(tkn == cmdEND_SELECT){
        	i--;                                // found an END SELECT so decrement our nested counter
#ifdef CMD16BIT
           p++;
#endif
        }

        if(i == 0) {
            // found our matching END SELECT stmt.  Step over it and continue with the statement after it
            skipelement(p);
            nextstmt = p;
            CurrentLinePtr = SaveCurrentLinePtr;
            return;
        }
    }
}


// if we have hit a CASE or CASE ELSE we must search for a END SELECT at this level and resume at that point
void cmd_case(void) {
    int i;
    char *p;

    // search through the program looking for a END SELECT statement
    // i tracks the nesting level of any nested SELECT CASE commands
    i = 1; p = nextstmt;
    while(1) {
        p = GetNextCommand(p, NULL, "No matching END SELECT");
		CommandToken tkn = commandtbl_decode(p);
        if(tkn == cmdSELECT_CASE) i++;                               // found a nested SELECT CASE command, we now need to search for its END CASE

        if(tkn == cmdEND_SELECT) i--;                                // found an END SELECT so decrement our nested counter
        if(i == 0) {
            // found our matching END SELECT stmt.  Step over it and continue with the statement after it
            skipelement(p);
            nextstmt = p;
            break;
        }
    }
}



void cmd_input(void) {
	char s[STRINGSIZE];
	char *p, *sp, *tp;
	int i, fnbr;
	getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",;");				// this is a macro and must be the first executable stmt

	// is the first argument a file number specifier?  If so, get it
	if(argc >= 3 && *argv[0] == '#') {
		argv[0]++;
		fnbr = getinteger(argv[0]);
		i = 2;
	}
	else {
		fnbr = 0;
		// is the first argument a prompt?
		// if so, print it followed by an optional question mark
		if(argc >= 3 && *argv[0] == '"' && (*argv[1] == ',' || *argv[1] == ';')) {
			*(argv[0] + strlen(argv[0]) - 1) = 0;
			argv[0]++;
			MMPrintString(argv[0]);
			if(*argv[1] == ';') MMPrintString("? ");
			i = 2;
		} else {
			MMPrintString("? ");									// no prompt?  then just print the question mark
			i = 0;
		}
	}

	if(argc - i < 1) error("Syntax");						        // no variable to input to

	MMgetline(fnbr, inpbuf);									    // get the line
	p = inpbuf;

	// step through the variables listed for the input statement
	// and find the next item on the line and assign it to the variable
	for(; i < argc; i++) {
		sp = s;														// sp is a temp pointer into s[]
		if(*argv[i] == ',' || *argv[i] == ';') continue;
		skipspace(p);
		if(*p != 0) {
			if(*p == '"') {											// if it is a quoted string
				p++;												// step over the quote
				while(*p && *p != '"')  *sp++ = *p++;				// and copy everything upto the next quote
				while(*p && *p != ',') p++;							// then find the next comma
			} else {												// otherwise it is a normal string of characters
				while(*p && *p != ',') *sp++ = *p++;				// copy up to the comma
				while(sp > s && sp[-1] == ' ') sp--;				// and trim trailing whitespace
			}
		}
		*sp = 0;													// terminate the string
		tp = findvar(argv[i], V_FIND);								// get the variable and save its new value
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
		int inp_vtype = vartbl[VarIndex].type;  // STRUCTENABLED
		int inp_size = vartbl[VarIndex].size;   // STRUCTENABLED
#ifdef STRUCTENABLED
		if (g_StructMemberType != 0)
		{
		inp_vtype = g_StructMemberType;
		if (g_StructMemberType & T_STR)
			 inp_size = g_StructMemberSize;
		}
#endif
		if (inp_vtype & T_STR) {                // STRUCTENABLED
			if (strlen((char *)s) > inp_size) error("String too long");
			strcpy(tp, s);
			CtoM(tp);												// convert to a MMBasic string
		} else if (inp_vtype & T_INT) {         // STRUCTENABLED
    		*((long long int *)tp) = strtoll(s, &sp, 10);			// convert to an integer
		}
		else
			*((MMFLOAT *)tp) = (MMFLOAT)atof(s);
		if(*p == ',') p++;
	}
}


void cmd_trace(void) {
    if(checkstring(cmdline, "ON"))
    	TraceOn = true;
    else if(checkstring(cmdline, "OFF"))
        TraceOn = false;
    else if(checkstring(cmdline, "LIST")) {
        int i;
        cmdline += 4;
        skipspace(cmdline);
        if(*cmdline == 0 || *cmdline =='\'')  //'
        	i = TRACE_BUFF_SIZE - 1;
        else
        	i = getint(cmdline, 0, TRACE_BUFF_SIZE - 1);
        i = TraceBuffIndex - i;
        if(i < 0) i += TRACE_BUFF_SIZE;
        while(i != TraceBuffIndex) {
        	TraceLines(TraceBuff[i]);
            if(++i >= TRACE_BUFF_SIZE) i = 0;
        }
    }
    else
        error("Unknown command");
}



// FOR command
void cmd_for(void) {
    int i, t, vlen, test;
    char ss[4];                                                     // this will be used to split up the argument line
    char *p, *tp, *xp;
    void *vptr;
    char *vname, vtype;
//    static char fortoken, nexttoken;

      // cache these tokens for speed
//    if(!fortoken) fortoken = cmdFOR;
//    if(!nexttoken) nexttoken = cmdNEXT;

    ss[0] = tokenEQUAL;
    ss[1] = tokenTO;
    ss[2] = tokenSTEP;
    ss[3] = 0;

    {                                                               // start a new block
        getargs(&cmdline, 7, ss);                                   // getargs macro must be the first executable stmt in a block
        if(argc < 5 || argc == 6 || *argv[1] != ss[0] || *argv[3] != ss[1]) error("FOR with misplaced = or TO");
        if(argc == 6 || (argc == 7 && *argv[5] != ss[2])) error("Syntax");

        // get the variable name and trim any spaces
        vname = argv[0];
        if(*vname && *vname == ' ') vname++;
        while(*vname && vname[strlen(vname) - 1] == ' ') vname[strlen(vname) - 1] = 0;
        vlen = strlen(vname);
        vptr = findvar(argv[0], V_FIND);                            // create the variable
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
#ifdef STRUCTENABLED
		if (g_StructMemberType != 0)
			vtype = TypeMask(g_StructMemberType);
#endif
        if(vtype & T_STR) error("Invalid variable");                // sanity check

        // check if the FOR variable is already in the stack and remove it if it is
        // this is necessary as the program can jump out of the loop without hitting
        // the NEXT statement and this will eventually result in a stack overflow
        for(i = 0; i < forindex ;i++) {
            if(forstack[i].var == vptr && forstack[i].level == LocalIndex) {
                while(i < forindex - 1) {
                    forstack[i].forptr = forstack[i+1].forptr;
                    forstack[i].nextptr = forstack[i+1].nextptr;
                    forstack[i].var = forstack[i+1].var;
                    forstack[i].vartype = forstack[i+1].vartype;
                    forstack[i].level = forstack[i+1].level;
                    forstack[i].tovalue.i = forstack[i+1].tovalue.i;
                    forstack[i].stepvalue.i = forstack[i+1].stepvalue.i;
                    i++;
                }
                forindex--;
                break;
            }
        }

        if(forindex == MAXFORLOOPS) error("Too many nested FOR loops");

        forstack[forindex].var = vptr;                              // save the variable index
        forstack[forindex].vartype = vtype;                         // save the type of the variable
        forstack[forindex].level = LocalIndex;                      // save the level of the variable in terms of sub/funs
        forindex++;                                                 // incase functions use for loops
        if(vtype & T_NBR) {
            *(MMFLOAT *)vptr = getnumber(argv[2]);                  // get the starting value for a float and save
            forstack[forindex - 1].tovalue.f = getnumber(argv[4]);  // get the to value and save
            if(argc == 7)
                forstack[forindex - 1].stepvalue.f = getnumber(argv[6]);// get the step value for a float and save
            else
                forstack[forindex - 1].stepvalue.f = 1.0;           // default is +1
        } else {
            *(long long int *)vptr = getinteger(argv[2]);           // get the starting value for an integer and save
            forstack[forindex - 1].tovalue.i = getinteger(argv[4]); // get the to value and save
            if(argc == 7)
                forstack[forindex - 1].stepvalue.i = getinteger(argv[6]);// get the step value for an integer and save
            else
                forstack[forindex - 1].stepvalue.i = 1;             // default is +1
        }
        forindex--;

        forstack[forindex].forptr = nextstmt + 1;                   // return to here when looping

        // now find the matching NEXT command
        t = 1; p = nextstmt;
        while(1) {
              p = GetNextCommand(p, &tp, "No matching NEXT");
            CommandToken tkn = commandtbl_decode(p);
            if(tkn == cmdFOR) t++;                                 // count the FOR
            if(tkn == cmdNEXT) {                                   // is it NEXT
            	xp = p + sizeof(CommandToken); // point to after the NEXT token
                while(*xp && strncasecmp(xp, vname, vlen)) xp++;    // step through looking for our variable
                if(*xp && !isnamechar(xp[vlen]))                    // is it terminated correctly?
                    t = 0;                                          // yes, found the matching NEXT
                else
                    t--;                                            // no luck, just decrement our stack counter
            }
            if(t == 0) {                                            // found the matching NEXT
                forstack[forindex].nextptr = p;                     // pointer to the start of the NEXT command
                break;
            }
        }

          // test the loop value at the start
          if(forstack[forindex].vartype & T_INT)
              test = (forstack[forindex].stepvalue.i >= 0 && *(long long int *)vptr > forstack[forindex].tovalue.i) || (forstack[forindex].stepvalue.i < 0 && *(long long int *)vptr < forstack[forindex].tovalue.i) ;
          else
              test = (forstack[forindex].stepvalue.f >= 0 && *(MMFLOAT *)vptr > forstack[forindex].tovalue.f) || (forstack[forindex].stepvalue.f < 0 && *(MMFLOAT *)vptr < forstack[forindex].tovalue.f) ;

          if(test) {
            // loop is invalid at the start, so go to the end of the NEXT command
            skipelement(p);                                         // find the command after the NEXT command
            nextstmt = p;                                           // this is where we will continue
        } else {
            forindex++;                                             // save the loop data and continue on with the command after the FOR statement
          }
    }
}

void cmd_new(void){
	int i=3;
	LineCount=0;
    if(CurrentLinePtr) error("Invalid in a program");
	ClearVars(0);
    ClearSavedVars();                                               // clear any saved variables
    while(FlashWriteInit((uint32_t)ProgMemory) && i)i--;                     // erase program memory
    if(i==0)error("Failed to erase flash memory");
    FlashWriteByte(0); FlashWriteByte(0); FlashWriteByte(0);    // terminate the program in flash
    FlashWriteClose();
	ClearProgram();
    WatchdogSet = false;
    Option.Autorun = false;
    mymemset(inpbuf,0,STRINGSIZE);
    mymemset(lastcmd,0,STRINGSIZE*4);
	SCB_CleanInvalidateDCache();
    longjmp(mark, 1);
}


void cmd_next(void) {
    int i, vindex, test;
    void *vtbl[MAXFORLOOPS];
    int vcnt;
    char *p;
    getargs(&cmdline, MAXFORLOOPS * 2, ",");                        // getargs macro must be the first executable stmt in a block

    vindex = 0;                                                     // keep lint happy

    for(vcnt = i = 0; i < argc; i++) {
        if(i & 0x01) {
            if(*argv[i] != ',') error("Syntax");
        } else
            vtbl[vcnt++] = findvar(argv[i], V_FIND | V_NOFIND_ERR); // find the variable and error if not found
    }

    loopback:
    // first search the for stack for a loop with the same variable specified on the NEXT's line
    if(vcnt) {
        for(i = forindex - 1; i >= 0; i--)
            for(vindex = vcnt - 1; vindex >= 0 ; vindex--)
                if(forstack[i].var == vtbl[vindex])
                    goto breakout;
    } else {
        // if no variables specified search the for stack looking for an entry with the same program position as
        // this NEXT statement. This cheats by using the cmdline as an identifier and may not work inside an IF THEN ELSE
        for(i = 0; i < forindex; i++) {
            p = forstack[i].nextptr + sizeof(CommandToken);
            skipspace(p);
            if(p == cmdline) goto breakout;
        }
    }

    error("Cannot find a matching FOR");

    breakout:

    // found a match
    // apply the STEP value to the variable and test against the TO value
    if(forstack[i].vartype & T_INT) {
        *(long long int *)forstack[i].var += forstack[i].stepvalue.i;
        test = (forstack[i].stepvalue.i >= 0 && *(long long int *)forstack[i].var > forstack[i].tovalue.i) || (forstack[i].stepvalue.i < 0 && *(long long int *)forstack[i].var < forstack[i].tovalue.i) ;
    } else {
        *(MMFLOAT *)forstack[i].var += forstack[i].stepvalue.f;
        test = (forstack[i].stepvalue.f >= 0 && *(MMFLOAT *)forstack[i].var > forstack[i].tovalue.f) || (forstack[i].stepvalue.f < 0 && *(MMFLOAT *)forstack[i].var < forstack[i].tovalue.f) ;
    }

    if(test) {
        // the loop has terminated
        // remove the entry in the table, then skip forward to the next element and continue on from there
        while(i < forindex - 1) {
            forstack[i].forptr = forstack[i+1].forptr;
            forstack[i].nextptr = forstack[i+1].nextptr;
            forstack[i].var = forstack[i+1].var;
            forstack[i].vartype = forstack[i+1].vartype;
            forstack[i].level = forstack[i+1].level;
            forstack[i].tovalue.i = forstack[i+1].tovalue.i;
            forstack[i].stepvalue.i = forstack[i+1].stepvalue.i;
            i++;
        }
        forindex--;
        if(vcnt > 0) {
            // remove that entry from our FOR stack
            for(; vindex < vcnt - 1; vindex++) vtbl[vindex] = vtbl[vindex + 1];
            vcnt--;
            if(vcnt > 0)
                goto loopback;
            else
                return;
        }

    } else {
        // we have not reached the terminal value yet, so go back and loop again
        nextstmt = forstack[i].forptr;
    }
}




void cmd_do(void) {
    int i;
    char *p, *tp, *evalp;
    if(cmdtoken==cmdWHILE)error("Unknown command");
	// if it is a DO loop find the WHILE token and (if found) get a pointer to its expression
	while(*cmdline && *cmdline != tokenWHILE) cmdline++;
	if(*cmdline == tokenWHILE) {
		evalp = ++cmdline;
	}
	else
		evalp = NULL;
    // check if this loop is already in the stack and remove it if it is
    // this is necessary as the program can jump out of the loop without hitting
    // the LOOP or WEND stmt and this will eventually result in a stack overflow
    for(i = 0; i < doindex ;i++) {
        if(dostack[i].doptr == nextstmt) {
            while(i < doindex - 1) {
                dostack[i].evalptr = dostack[i+1].evalptr;
                dostack[i].loopptr = dostack[i+1].loopptr;
                dostack[i].doptr = dostack[i+1].doptr;
                dostack[i].level = dostack[i+1].level;
                i++;
            }
            doindex--;
            break;
        }
    }

    // add our pointers to the top of the stack
    if(doindex == MAXDOLOOPS) error("Too many nested DO or WHILE loops");
    dostack[doindex].evalptr = evalp;
    dostack[doindex].doptr = nextstmt;
    dostack[doindex].level = LocalIndex;

    // now find the matching LOOP command
    i = 1; p = nextstmt;
    while(1) {
        p = GetNextCommand(p, &tp, "No matching LOOP");
        CommandToken tkn = commandtbl_decode(p);
        if(tkn == cmdtoken) i++;                                     // entered a nested DO or WHILE loop
        if(tkn == cmdLOOP) i--;                                    // exited a nested loop

        if(i == 0) {                                                // found our matching LOOP or WEND stmt
            dostack[doindex].loopptr = p;
            break;
        }
    }

    if(dostack[doindex].evalptr != NULL) {
        // if this is a DO WHILE ... LOOP statement
        // search the LOOP statement for a WHILE or UNTIL token (p is pointing to the matching LOOP statement)
    	p += sizeof(CommandToken);
        while(*p && *p < 0x80) p++;
        if(*p == tokenWHILE) error("LOOP has a WHILE test");
        if(*p == tokenUNTIL) error("LOOP has an UNTIL test");
    }

    doindex++;

    // do the evaluation (if there is something to evaluate) and if false go straight to the command after the LOOP or WEND statement
    if(dostack[doindex - 1].evalptr != NULL && getnumber(dostack[doindex - 1].evalptr) == 0) {
        doindex--;                                                  // remove the entry in the table
        nextstmt = dostack[doindex].loopptr;                        // point to the LOOP or WEND statement
        skipelement(nextstmt);                                      // skip to the next command
    }

}




void cmd_loop(void) {
    char *p;
    int tst = 0;                                                    // initialise tst to stop the compiler from complaining
    int i;

    // search the do table looking for an entry with the same program position as this LOOP statement
    for(i = 0; i < doindex ;i++) {
        p = dostack[i].loopptr + sizeof(CommandToken);
        skipspace(p);
        if(p == cmdline) {
            // found a match
            // first check if the DO statement had a WHILE component
            // if not find the WHILE statement here and evaluate it
            if(dostack[i].evalptr == NULL) {                        // if it was a DO without a WHILE
                if(*cmdline >= 0x80) {                              // if there is something
                    if(*cmdline == tokenWHILE)
                        tst = (getnumber(++cmdline) != 0);          // evaluate the expression
                    else if(*cmdline == tokenUNTIL)
                        tst = (getnumber(++cmdline) == 0);          // evaluate the expression
                    else
                        error("Syntax");
                }
                else {
                    tst = 1;                                        // and loop forever
                    checkend(cmdline);                              // make sure that there is nothing else
                }
            }
            else {                                                  // if was DO WHILE
                tst = (getnumber(dostack[i].evalptr) != 0);         // evaluate its expression
                checkend(cmdline);                                  // make sure that there is nothing else
            }

            // test the expression value and reset the program pointer if we are still looping
            // otherwise remove this entry from the do stack
            if(tst)
                nextstmt = dostack[i].doptr;                        // loop again
            else {
                // the loop has terminated
                // remove the entry in the table, then just let the default nextstmt run and continue on from there
                  doindex = i;
                // just let the default nextstmt run
            }
            return;
        }
    }
    error("LOOP without a matching DO");
}



void cmd_exitfor(void) {
	if(forindex == 0) error("No FOR loop is in effect");
	nextstmt = forstack[--forindex].nextptr;
	checkend(cmdline);
	skipelement(nextstmt);
}



void cmd_exit(void) {
	if(doindex == 0) error("No DO loop is in effect");
	nextstmt = dostack[--doindex].loopptr;
	checkend(cmdline);
	skipelement(nextstmt);
}



void cmd_error(void) {
	char *s, p[STRINGSIZE];
	if(*cmdline && *cmdline != '\'') {
		s = getCstring(cmdline);

		strcpy(p,s);
		error(p);
	}
	else
		error("");
}




// this is the Sub or Fun command
// it simply skips over text until it finds the end of it
void cmd_subfun(void) {
	char *p, returntoken, errtoken;

    if(gosubindex != 0) error("No matching END declaration");       // we have hit a SUB/FUN while in another SUB or FUN
	if(cmdtoken == cmdSUB) {
	    returntoken = cmdENDSUB;
	    errtoken = cmdENDFUNCTION;
	} else {
	    returntoken = cmdENDFUNCTION;
	    errtoken = cmdENDSUB;
    }
	p = nextstmt;
	while(1) {
        p = GetNextCommand(p, NULL, "No matching END declaration");
        CommandToken tkn = commandtbl_decode(p);                     //CMD16BIT
        if(tkn == cmdSUB || tkn == cmdFUN || tkn == errtoken) error("No matching END declaration");
		if(tkn == returntoken) {                                     // found the next return
    		skipelement(p);
    		nextstmt = p;                                           // point to the next command
    		break;
        }
    }
}


/* Fixed as suggested by tom ********************/
//https://www.thebackshed.com/forum/ViewTopic.php?FID=16&TID=15486
void cmd_gosub(void) {
	if(gosubindex >= MAXGOSUB) error("Too many nested GOSUB");
	char *return_to = nextstmt;
	routinechecks(1);
	if(isnamestart(*cmdline))
		nextstmt = findlabel(cmdline);								// must be a label
	else
		nextstmt = findline(getinteger(cmdline), true);				// try for a line number
    IgnorePIN = false;

    errorstack[gosubindex] = CurrentLinePtr;
   	gosubstack[gosubindex++] = return_to;
   	LocalIndex++;
    CurrentLinePtr = nextstmt;           //fix for error line no
}
/* original

void cmd_gosub(void) {
	if(gosubindex >= MAXGOSUB) error("Too many nested GOSUB");
    errorstack[gosubindex] = CurrentLinePtr;
	gosubstack[gosubindex++] = nextstmt;
	LocalIndex++;
	routinechecks(1);
	if(isnamestart(*cmdline))
		nextstmt = findlabel(cmdline);								// must be a label
	else
		nextstmt = findline(getinteger(cmdline), true);				// try for a line number
    IgnorePIN = false;
}

*/

//Changes MID$ command so the length to be replaced can be 0 - i.e. insert the new string at the position specified
void cmd_mid(void)
{
	char *p;
	int mid_vtype;
	getargs(&cmdline,5,",");
	findvar(argv[0], V_NOFIND_ERR);
	if (vartbl[VarIndex].type & T_CONST)error("Cannot change a constant");
	mid_vtype = vartbl[VarIndex].type;
	int size =vartbl[VarIndex].size;
#ifdef STRUCTENABLED
	if (g_StructMemberType != 0)
	{
		mid_vtype = g_StructMemberType;
		if (g_StructMemberType & T_STR)
			size = g_StructMemberSize;
	}
#endif

	if (!(mid_vtype & T_STR))error("Not a string");
	char *sourcestring = (char *)getstring(argv[0]);
	int start = getint(argv[2], 1, sourcestring[0]);
	int num = -1;
	if (argc == 5)num = getint(argv[4], 0, sourcestring[0]);
	if (start + (num < 0 ? 0 : num - 1) > sourcestring[0])
		error("Selection exceeds length of string");
	while (*cmdline && tokenfunction(*cmdline) != op_equal)	cmdline++;
	if (!*cmdline)error("Syntax");
	++cmdline;
	if (!*cmdline)error("Syntax");
	char *value = (char *)getstring(cmdline);
	if (num == -1)num = value[0];
	p = (char *)&value[1];
	if (num == value[0])
		memcpy(&sourcestring[start], p, num);
	else
	{
		int change = value[0] - num;
		if (sourcestring[0] + change > size)
			error("String too long");
		memmove(&sourcestring[start + value[0]], &sourcestring[start + num], sourcestring[0] - (start + num - 1));
		sourcestring[0] += change;
		memcpy(&sourcestring[start], p, value[0]);
	}
}
/*
void cmd_mid(void){
	char *p;
	getargs(&cmdline,5,",");
	findvar(argv[0], V_NOFIND_ERR);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(!(vartbl[VarIndex].type & T_STR)) error("Not a string");
	int size=vartbl[VarIndex].size;
	char *sourcestring=getstring(argv[0]);
	int start=getint(argv[2],1,sourcestring[0]);
	int num=0;
	if(argc==5)num=getint(argv[4],1,sourcestring[0]);
	if(start+num-1>sourcestring[0])error("Selection exceeds length of string");
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
	if(!*cmdline) error("Syntax");
	char *value = getstring(cmdline);
	if(num==0)num=value[0];
	p=&value[1];
	if(num==value[0]) memcpy(&sourcestring[start],p,num);
	else {
		int change=value[0]-num;
		if(sourcestring[0]+change>size)error("String too long");
		memmove(&sourcestring[start+value[0]],&sourcestring[start+num],sourcestring[0]-(start+num-1));
		sourcestring[0]+=change;
		memcpy(&sourcestring[start],p,value[0]);
    }
}
*/
/*
void cmd_mid(void){
	unsigned char *p;
	getargs(&cmdline,5,",");
	findvar(argv[0], V_NOFIND_ERR);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(!(vartbl[VarIndex].type & T_STR)) error("Not a string");
	char *sourcestring=getstring(argv[0]);
	int start=getint(argv[2],1,sourcestring[0]);
	int num=0;
	if(argc==5)num=getint(argv[4],1,sourcestring[0]);
	if(start+num-1>sourcestring[0])error("Selection exceeds length of string");
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
	if(!*cmdline) error("Syntax");
	char *value = getstring(cmdline);
	if(num==0)num=value[0];
	p=(unsigned char *)&value[1];
	if(num==value[0]) memcpy(&sourcestring[start],p,num);
	else {
		int change=value[0]-num;
		if(sourcestring[0]+change>255)error("String too long");
		memmove(&sourcestring[start+value[0]],&sourcestring[start+num],sourcestring[0]-(start+num-1));
		sourcestring[0]+=change;
		memcpy(&sourcestring[start],p,value[0]);
	}
}
*/
#ifdef CMD16BIT
void cmd_byte(void)
{
	int byte_vtype;
	getcsargs(&cmdline, 3);
	findvar(argv[0], V_NOFIND_ERR);
	if (vartbl[VarIndex].type & T_CONST)
		StandardError(22);
	byte_vtype = vartbl[VarIndex].type;
#ifdef STRUCTENABLED
	if (g_StructMemberType != 0)
		byte_vtype = g_StructMemberType;
#endif
	if (!(byte_vtype & T_STR))
		error("Not a string");
	unsigned char *sourcestring = (unsigned char *)getstring(argv[0]);
	int start = getint(argv[2], 1, sourcestring[0]);
	while (*cmdline && tokenfunction(*cmdline) != op_equal)
		cmdline++;
	if (!*cmdline)
		SyntaxError();
	;
	++cmdline;
	if (!*cmdline)
		SyntaxError();
	;
	int value = getint(cmdline, 0, 255);
	sourcestring[start] = value;
}
void cmd_bitbyteflag(void)
{
	int i;
	getcsargs(&cmdline, 5);
	i = getint(argv[0], 0, 4); //
	if (i==4)  //BYTE
	{
		int byte_vtype;
		//getcsargs(&cmdline, 5);
		findvar(argv[2], V_NOFIND_ERR);
		if (vartbl[VarIndex].type & T_CONST)
			StandardError(22);
		byte_vtype = vartbl[VarIndex].type;
	#ifdef STRUCTENABLED
		if (g_StructMemberType != 0)
			byte_vtype = g_StructMemberType;
	#endif
		if (!(byte_vtype & T_STR))
			error("Not a string");
		unsigned char *sourcestring = (unsigned char *)getstring(argv[2]);
		int start = getint(argv[4], 1, sourcestring[0]);
		while (*cmdline && tokenfunction(*cmdline) != op_equal)
			cmdline++;
		if (!*cmdline)
			SyntaxError();
		;
		++cmdline;
		if (!*cmdline)
			SyntaxError();
		;
		int value = getint(cmdline, 0, 255);
		sourcestring[start] = value;
	}
	else if (i==1)  //BIT
	{
		int bit_vtype;
		//getcsargs(&cmdline, 5);
		uint64_t *source = (uint64_t *)findvar(argv[2], V_NOFIND_ERR);
		if (vartbl[VarIndex].type & T_CONST)
			StandardError(22);
		bit_vtype = vartbl[VarIndex].type;
	#ifdef STRUCTENABLED
		if (g_StructMemberType != 0)
			bit_vtype = g_StructMemberType;
	#endif
		if (!(bit_vtype & T_INT))
			error("Not an integer");
		uint64_t bit = (uint64_t)1 << (uint64_t)getint(argv[4], 0, 63);
		while (*cmdline && tokenfunction(*cmdline) != op_equal)
			cmdline++;
		if (!*cmdline)
			SyntaxError();
		;
		++cmdline;
		if (!*cmdline)
			SyntaxError();
		;
		int value = getint(cmdline, 0, 1);
		if (value)
			*source |= bit;
		else
			*source &= (~bit);
	}
	else if (i==0)  //FLAG
	{
		//getcsargs(&cmdline, 3);
		uint64_t bit = (uint64_t)1 << (uint64_t)getint(argv[2], 0, 63);
		while (*cmdline && tokenfunction(*cmdline) != op_equal)
			cmdline++;
		if (!*cmdline)
			SyntaxError();
		;
		++cmdline;
		if (!*cmdline)
			SyntaxError();
		;
		int value = getint(cmdline, 0, 1);
		if (value)
			g_flag |= bit;
		else
			g_flag &= ~bit;
	}

}
void cmd_bit(void)
{
	int bit_vtype;
	getcsargs(&cmdline, 3);
	uint64_t *source = (uint64_t *)findvar(argv[0], V_NOFIND_ERR);
	if (vartbl[VarIndex].type & T_CONST)
		StandardError(22);
	bit_vtype = vartbl[VarIndex].type;
#ifdef STRUCTENABLED
	if (g_StructMemberType != 0)
		bit_vtype = g_StructMemberType;
#endif
	if (!(bit_vtype & T_INT))
		error("Not an integer");
	uint64_t bit = (uint64_t)1 << (uint64_t)getint(argv[2], 0, 63);
	while (*cmdline && tokenfunction(*cmdline) != op_equal)
		cmdline++;
	if (!*cmdline)
		SyntaxError();
	;
	++cmdline;
	if (!*cmdline)
		SyntaxError();
	;
	int value = getint(cmdline, 0, 1);
	if (value)
		*source |= bit;
	else
		*source &= (~bit);
}
void cmd_flags(void)
{
	while (*cmdline && tokenfunction(*cmdline) != op_equal)
		cmdline++;
	if (!*cmdline)
		SyntaxError();
	;
	g_flag = getinteger(++cmdline);
}

void cmd_flag(void)
{
	getcsargs(&cmdline, 1);
	uint64_t bit = (uint64_t)1 << (uint64_t)getint(argv[0], 0, 63);
	while (*cmdline && tokenfunction(*cmdline) != op_equal)
		cmdline++;
	if (!*cmdline)
		SyntaxError();
	;
	++cmdline;
	if (!*cmdline)
		SyntaxError();
	;
	int value = getint(cmdline, 0, 1);
	if (value)
		g_flag |= bit;
	else
		g_flag &= ~bit;
}







#endif


void cmd_return(void) {
 	checkend(cmdline);
	if(gosubindex == 0 || gosubstack[gosubindex - 1] == NULL) error("Nothing to return to");
    ClearVars(LocalIndex--);                                        // delete any local variables
    TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
	nextstmt = gosubstack[--gosubindex];                            // return to the caller
    CurrentLinePtr = errorstack[gosubindex];
}

void cmd_endfun(void) {
 	checkend(cmdline);
	if(gosubindex == 0 || gosubstack[gosubindex - 1] != NULL) error("Nothing to return to");
	nextstmt = "\0\0\0";                                            // now terminate this run of ExecuteProgram()
}

// Updated for Option Escape
void cmd_read(void) {
    int i, j, k, len, card;
    char *p,  *lineptr = NULL, *ptr;
//#ifndef CMD16BIT
//    char datatoken;
//#else
//    unsigned short datatoken;
//#endif
    CommandToken datatoken;
    int vcnt, vidx, num_to_read=0;
    //Add READ SAVE and READ RESTORE
	if (checkstring(cmdline, (char*)"SAVE")) {
		if(restorepointer== MAXRESTORE - 1)error((char*)"Too many saves");
		datastore[restorepointer].SaveNextDataLine = NextDataLine;
		datastore[restorepointer].SaveNextData = NextData;
		restorepointer++;
		return;
	}
	if (checkstring(cmdline, (char*)"RESTORE")) {
		if (!restorepointer)error((char*)"Nothing to restore");
		restorepointer--;
		NextDataLine = datastore[restorepointer].SaveNextDataLine;
		NextData = datastore[restorepointer].SaveNextData;
		return;
	}

    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");                // getargs macro must be the first executable stmt in a block

    if(argc == 0) error("Syntax");
	// first count the elements and do the syntax checking
    for(vcnt = i = 0; i < argc; i++) {
        if(i & 0x01) {
            if(*argv[i] != ',') error("Syntax");
        } else {
			findvar(argv[i], V_FIND | V_EMPTY_OK);
			if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
			card=1;
			if(emptyarray){ //empty array
				for(k=0;k<MAXDIM;k++){
					j=(vartbl[VarIndex].dims[k] - OptionBase + 1);
					if(j)card *= j;
				}
			}
			num_to_read+=card;
		}
	}
    char **vtbl=GetTempMemory(num_to_read * sizeof (char *));
    int *vtype=GetTempMemory(num_to_read * sizeof (int));
    int *vsize=GetTempMemory(num_to_read * sizeof (int));
    // step through the arguments and save the pointer and type
    for(vcnt = i = 0; i < argc; i+=2) {
		ptr = vtbl[vcnt] = findvar(argv[i], V_FIND | V_EMPTY_OK);
		card=1;
		if(emptyarray){
			for(k=0;k<MAXDIM;k++){
				j=(vartbl[VarIndex].dims[k] - OptionBase + 1);
				if(j)card *= j;
			}
		}
		for(k=0;k<card;k++){
			if(k){
				if(vartbl[VarIndex].type & (T_INT | T_NBR))ptr+=8;
				else ptr+=vartbl[VarIndex].size+1;
				vtbl[vcnt]=ptr;
			}
#ifdef STRUCTENABLED
			if (g_StructMemberType != 0)
			{
				vtype[vcnt] = TypeMask(g_StructMemberType);
				vsize[vcnt] = (g_StructMemberType & T_STR) ? g_StructMemberSize : vartbl[VarIndex].size;
			}
			else
			{
				vtype[vcnt] = TypeMask(vartbl[VarIndex].type);
				vsize[vcnt] = vartbl[VarIndex].size;
			}
#else
			vtype[vcnt] = TypeMask(vartbl[VarIndex].type);
			vsize[vcnt] = vartbl[VarIndex].size;
#endif
			vcnt++;
		}
    }

    // setup for a search through the whole memory
    vidx = 0;
    datatoken = GetCommandValue("Data");
    p = lineptr = NextDataLine;
    if(*p == 0xff) error("No DATA to read");                        // error if there is no program

  // search looking for a DATA statement.  We keep returning to this point until all the data is found
search_again:
    while(1) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0 /*|| *p == 0xff*/) error("No DATA to read");     // 2nd 0 so end of the program and we still need more data
        if(*p == T_NEWLINE) lineptr = p++;                          // fix as per picomite if token 255 in use
        if(*p == T_LINENBR) p += 3;
        skipspace(p);
        if(*p == T_LABEL) {                                         // if there is a label here
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        CommandToken tkn = commandtbl_decode(p);
        if(tkn == datatoken)
        	break;                                                  // found a DATA statement
        while(*p) p++;                                              // look for the zero marking the start of the next element
    }
    NextDataLine = lineptr;
    //p++;                                                            // step over the token
    p += sizeof(CommandToken);                                      // step over the 8/16bit token
    skipspace(p);
    if(!*p || *p == '\'') { CurrentLinePtr = lineptr; error("No DATA to read"); }

        // we have a DATA statement, first split the line into arguments
        {                                                           // new block, the getargs macro must be the first executable stmt in a block
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");
        if((argc & 1) == 0) { CurrentLinePtr = lineptr; error("Syntax"); }
        // now step through the variables on the READ line and get their new values from the argument list
        // we set the line number to the number of the DATA stmt so that any errors are reported correctly
        while(vidx < vcnt) {
            // check that there is some data to read if not look for another DATA stmt
            if(NextData > argc) {
                skipline(p);
                NextData = 0;
                goto search_again;
            }
            CurrentLinePtr = lineptr;
            if(vtype[vidx] & T_STR) {
                char *p1, *p2;
                if(*argv[NextData] == '"') {                               // if quoted string
                  	int toggle=0;
                    for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData] + 1; *p2 && *p2 != '"'; len++) {
                    	if(*p2=='\\' && p2[1]!='"' && OptionEscape)toggle^=1;
	                    if(toggle){
	                        if(*p2=='\\' && IsDigit(p2[1]) && IsDigit(p2[2]) && IsDigit(p2[3])){
	                            p2++;
	                            i=(*p2++)-48;
	                            i*=10;
	                            i+=(*p2++)-48;
	                            i*=10;
	                            i+=(*p2++)-48;
	                            if(i==0)error("Illegal escape sequence, use CHR$(0) for the Null character","$");
	                            *p1++=i;
	                        } else {
	                            p2++;
	                            switch(*p2){
	                                case '\\':
	                                    *p1++='\\';
	                                    p2++;
	                                    break;
	                                case 'a':
	                                    *p1++='\a';
	                                    p2++;
	                                    break;
	                                case 'b':
	                                    *p1++='\b';
	                                    p2++;
	                                    break;
	                                case 'e':
	                                    *p1++='\e';
	                                    p2++;
	                                    break;
	                                case 'f':
	                                    *p1++='\f';
	                                    p2++;
	                                    break;
	                                case 'n':
	                                    *p1++='\n';
	                                    p2++;
	                                    break;
	                                case 'q':
	                                    *p1++='\"';
	                                    p2++;
	                                    break;
	                                case 'r':
	                                    *p1++='\r';
	                                    p2++;
	                                    break;
	                                case 't':
	                                    *p1++='\t';
	                                    p2++;
	                                    break;
	                                case 'v':
	                                    *p1++='\v';
	                                    p2++;
	                                    break;
	                                case '&':
	                                    p2++;
	                                    if(IsxDigit(*p2) && IsxDigit(p2[1])){
	                                        i=0;
	                                        i = (i << 4) | ((toupper(*p2) >= 'A') ? toupper(*p2) - 'A' + 10 : *p2 - '0');
	                                        p++;
	                                        i = (i << 4) | ((toupper(*p2) >= 'A') ? toupper(*p2) - 'A' + 10 : *p2 - '0');
	                                        if(i==0)error("Illegal escape sequence, use CHR$(0) for the Null character","$");
	                                        p2++;
	                                        *p1++=i;
	                                    } else *p1++='x';
	                                    break;
	                                default:
	                                    *p1++=*p2++;
	                            }
	                        }
	                        toggle=0;
	                    } else *p1++ = *p2++;
                    }
                } else {                                            // else if not quoted
                	for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData]; *p2 && *p2 != '\'' ; len++, p1++, p2++) {
                        if(*p2 < 0x20 || *p2 >= 0x7f) error("Invalid character");
                        *p1 = *p2;                                  // copy up to the comma
                    }
                }
                if(len > vsize[vidx]) error("String too long");
                *p1 = 0;                                            // terminate the string
                //MMPrintString("before");PRet();
                CtoM(vtbl[vidx]);                                   // convert to a MMBasic string
               // MMPrintString(vtbl[vidx]);PRet();
            }
            else if(vtype[vidx] & T_INT){
            	char *p = argv[NextData];
                while(*p)  *p++ = toupper(*p);                               // all expressions must be in uppercase
            	*((long long int *)vtbl[vidx]) = getinteger(argv[NextData]); // much easier if integer variable
            }
            else{
            	char *p = argv[NextData];
            	while(*p)  *p++ = toupper(*p);                               // all expressions must be in uppercase
                *((MMFLOAT *)vtbl[vidx]) = getnumber(argv[NextData]);      // same for numeric variable
            }
            vidx++;
            NextData += 2;
        }
    }
}





//Pre Option Explicit
#ifdef OLD
void cmd_read(void) {
    int i, j, k, len, card;
    char *p, datatoken, *lineptr = NULL, *ptr;
    int vcnt, vidx, num_to_read=0;
	if (checkstring(cmdline, "SAVE")) {
		if(restorepointer== MAXRESTORE - 1)error((char*)"Too many saves");
		datastore[restorepointer].SaveNextDataLine = NextDataLine;
		datastore[restorepointer].SaveNextData = NextData;
		restorepointer++;
		return;
	}
	if (checkstring(cmdline, "RESTORE")) {
		if (!restorepointer)error((char*)"Nothing to restore");
		restorepointer--;
		NextDataLine = datastore[restorepointer].SaveNextDataLine;
		NextData = datastore[restorepointer].SaveNextData;
		return;
	}

    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");                // getargs macro must be the first executable stmt in a block
    if(argc == 0) error("Syntax");
	// first count the elements and do the syntax checking
    for(vcnt = i = 0; i < argc; i++) {
        if(i & 0x01) {
            if(*argv[i] != ',') error("Syntax");
        } else {
			findvar(argv[i], V_FIND | V_EMPTY_OK);
			if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
			card=1;
//			PInt((uint32_t)ptr);PIntComma((uint32_t)vartbl[VarIndex].val.s);PIntComma((uint32_t)vartbl[VarIndex].dims[0]);PRet();
			if(emptyarray){ //empty array
				for(k=0;k<MAXDIM;k++){
					j=(vartbl[VarIndex].dims[k] - OptionBase + 1);
					if(j)card *= j;
				}
			}
			num_to_read+=card;
		}
	}
    char **vtbl=GetTempMemory(num_to_read * sizeof (char *));
    int *vtype=GetTempMemory(num_to_read * sizeof (int));
    int *vsize=GetTempMemory(num_to_read * sizeof (int));

    // step through the arguments and save the pointer and type
    for(vcnt = i = 0; i < argc; i+=2) {
		ptr = vtbl[vcnt] = findvar(argv[i], V_FIND | V_EMPTY_OK);
		card=1;
		if(emptyarray){
			for(k=0;k<MAXDIM;k++){
				j=(vartbl[VarIndex].dims[k] - OptionBase + 1);
				if(j)card *= j;
			}
		}
		for(k=0;k<card;k++){
			if(k){
				if(vartbl[VarIndex].type & (T_INT | T_NBR))ptr+=8;
				else ptr+=vartbl[VarIndex].size+1;
				vtbl[vcnt]=ptr;
			}
			vtype[vcnt] = TypeMask(vartbl[VarIndex].type);
			vsize[vcnt] = vartbl[VarIndex].size;
			vcnt++;
		}
    }

    // setup for a search through the whole memory
    vidx = 0;
    datatoken = GetCommandValue("Data");
    p = lineptr = NextDataLine;
    if(*p == 0xff) error("No DATA to read");                        // error if there is no program

  // search looking for a DATA statement.  We keep returning to this point until all the data is found
search_again:
    while(1) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0 || *p == 0xff) error("No DATA to read");         // end of the program and we still need more data
        if(*p == T_NEWLINE) lineptr = p++;
        if(*p == T_LINENBR) p += 3;
        skipspace(p);
        if(*p == T_LABEL) {                                         // if there is a label here
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        if(*p == datatoken) break;                                  // found a DATA statement
        while(*p) p++;                                              // look for the zero marking the start of the next element
    }
    NextDataLine = lineptr;
    p++;                                                            // step over the token
    skipspace(p);
    if(!*p || *p == '\'') { CurrentLinePtr = lineptr; error("No DATA to read"); }

        // we have a DATA statement, first split the line into arguments
        {                                                           // new block, the getargs macro must be the first executable stmt in a block
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");
        if((argc & 1) == 0) { CurrentLinePtr = lineptr; error("Syntax"); }
        // now step through the variables on the READ line and get their new values from the argument list
        // we set the line number to the number of the DATA stmt so that any errors are reported correctly
        while(vidx < vcnt) {
            // check that there is some data to read if not look for another DATA stmt
            if(NextData > argc) {
                skipline(p);
                NextData = 0;
                goto search_again;
            }
            CurrentLinePtr = lineptr;
            if(vtype[vidx] & T_STR) {
                char *p1, *p2;
                if(*argv[NextData] == '"') {                               // if quoted string
                  	int toggle=0;
                    for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData] + 1; *p2 && *p2 != '"'; len++) {
						if(*p2=='|')toggle^=1;
						if(*p2=='|' && isdigit((unsigned char)p2[1])){
							if(toggle){
							p2++;
							i=(*p2++)-48;
							while(isdigit((unsigned char)*p2)){
								i*=10;
								i+=(*p2++)-48;
							}
							*p1++=i;
							if(*p2=='|')p2++;
							} else {
								p2++;
							}
							toggle=0;
						} else *p1++ = *p2++;
					}
                } else {
					int toggle=0;                                            // else if not quoted
                    for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData]; *p2 && *p2 != '\'' ; len++) {
                        if(*p2 < 0x20 || *p2 >= 0x7f) error("Invalid character");
						if(*p2=='|')toggle^=1;
						if(*p2=='|' && isdigit((unsigned char)p2[1])){
							if(toggle){
							p2++;
							i=(*p2++)-48;
							while(isdigit((unsigned char)*p2)){
								i*=10;
								i+=(*p2++)-48;
							}
							*p1++=i;
							if(*p2=='|')p2++;
							} else {
								p2++;
							}
							toggle=0;
						} else *p1++ = *p2++;
                    }
                }
                if(len > vsize[vidx]) error("String too long");
                *p1 = 0;                                            // terminate the string
                CtoM(vtbl[vidx]);                                   // convert to a MMBasic string
            }
            else {
                char *p = argv[NextData];
                while(*p)  *p++ = toupper(*p);                               // all expressions must be in uppercase
				if(vtype[vidx] & T_INT)
					*((long long int *)vtbl[vidx]) = getinteger(argv[NextData]); // much easier if integer variable
				else
					*((MMFLOAT *)vtbl[vidx]) = getnumber(argv[NextData]);      // same for numeric variable
				}
            vidx++;
            NextData += 2;
        }
    }
}
#endif

void cmd_restore(void) {
	if(*cmdline == 0 || *cmdline == '\'') {
        NextDataLine = ProgMemory;
		NextData = 0;
	} else {
		skipspace(cmdline);
		if(*cmdline=='"') {
			NextDataLine = findlabel(getCstring(cmdline));
			NextData = 0;
		}
		else if(IsDigit(*cmdline) || *cmdline==GetTokenValue( (char *)"+") || *cmdline==GetTokenValue( (char *)"-")  || *cmdline=='.'){
				NextDataLine = findline(getinteger(cmdline), true);		// try for a line number
				NextData = 0;
		} else {
			void *ptr=findvar(cmdline,V_NOFIND_NULL);
			if(ptr){
				if(vartbl[VarIndex].type & T_NBR) {
					if(vartbl[VarIndex].dims[0] > 0) {		// Not an array
						error("Syntax");
					}
					NextDataLine = findline(getinteger(cmdline), true);
				} else if(vartbl[VarIndex].type & T_INT) {
					if(vartbl[VarIndex].dims[0] > 0) {		// Not an array
						error("Syntax");
					}
					NextDataLine = findline(getinteger(cmdline), true);
				} else {
					char *c=getCstring(cmdline);
					char b[STRINGSIZE]={0};
					char *d=b;
					while(*c){
						*d++=toupper(*c++);
					}
					NextDataLine = findlabel(b);					    // must be a label
				}
			} else if(isnamestart(*cmdline)) {
				NextDataLine = findlabel(cmdline);					    // must be a label
			}
			NextData = 0;
		}
	}
}


void cmd_lineinput(void) {
	char *vp;
	int i, fnbr;
	getargs(&cmdline, 3, ",;");										// this is a macro and must be the first executable stmt
	if(argc == 0 || argc == 2) error("Syntax");

	i = 0;
	fnbr = 0;
	if(argc == 3) {
		// is the first argument a file number specifier?  If so, get it
		if(*argv[0] == '#' && *argv[1] == ',') {
			argv[0]++;
			fnbr = getinteger(argv[0]);
		}
		else {
			// is the first argument a prompt?  if so, print it otherwise there are too many arguments
			if(*argv[1] != ',' && *argv[1] != ';') error("Syntax");
			MMfputs(getstring(argv[0]), 0);
		}
	i = 2;
	}

	if(argc - i != 1) error("Syntax");
	vp = findvar(argv[i], V_FIND);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	int linp_vtype = vartbl[VarIndex].type;
	int linp_size = vartbl[VarIndex].size;
#ifdef STRUCTENABLED
	if (g_StructMemberType != 0)
	{
		linp_vtype = g_StructMemberType;
		if (g_StructMemberType & T_STR)
			linp_size = g_StructMemberSize;
	}
#endif
	if (!(linp_vtype & T_STR)) error("Invalid variable");
	MMgetline(fnbr, inpbuf);									    // get the input line
	if(strlen(inpbuf) > linp_size) error("String too long");
	strcpy(vp, inpbuf);
	CtoM(vp);														// convert to a MMBasic string
}



void cmd_on(void) {
	int r;
	char ss[4];													    // this will be used to split up the argument line
    char *p;

	// first check if this is:   ON KEY location
	if((p = checkstring(cmdline, "KEY")) != NULL) {
		getargs(&p,3,",");
		if(argc==1){
			if(*argv[0] == '0' && !IsDigitinline(*(argv[0]+1))){
				OnKeyGOSUB = NULL;                                      // the program wants to turn the interrupt off
			} else {
				OnKeyGOSUB = GetIntAddress(argv[0]);						    // get a pointer to the interrupt routine
				InterruptUsed = true;
			}
			return;
		} else {
			keyselect=getint(argv[0],0,255);
			if(keyselect==0){
				KeyInterrupt = NULL;                                      // the program wants to turn the interrupt off
			} else {
				if(*argv[2] == '0' && !IsDigitinline(*(argv[2]+1))){
					KeyInterrupt = NULL;                                      // the program wants to turn the interrupt off
				} else {
					KeyInterrupt = GetIntAddress(argv[2]);						    // get a pointer to the interrupt routine
					InterruptUsed = true;
				}
			}
			return;
		}
	}
    p = checkstring(cmdline, "ERROR");
    if(p) {
        if(checkstring(p, "ABORT")) {
            OptionErrorSkip = 0;
            return;
        }
        MMerrno = 0;                                                // clear the error flags
        *MMErrMsg = 0;
        if(checkstring(p, "CLEAR")) return;
        if(checkstring(p, "IGNORE")) {
            OptionErrorSkip = -1;
            return;
        }
        if((p = checkstring(p, "SKIP"))) {
            if(*p == 0 || *p == '\'')
                OptionErrorSkip = 1;
            else
                OptionErrorSkip = getint(p, 1, 10000) ;
            return;
        }
        error("Syntax");
	}

	// if we got here the command must be the traditional:  ON nbr GOTO|GOSUB line1, line2,... etc

	ss[0] = tokenGOTO;
	ss[1] = tokenGOSUB;
	ss[2] = ',';
	ss[3] = 0;
	{																// start a new block
		getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ss);				// getargs macro must be the first executable stmt in a block
		if(argc < 3 || !(*argv[1] == ss[0] || *argv[1] == ss[1])) error("Syntax");
		if(argc%2 == 0) error("Syntax");

		r = getint(argv[0], 0, 255);									// evaluate the expression controlling the statement
		if(r == 0 || r > argc/2) return;							// microsoft say that we just go on to the next line

		if(*argv[1] == ss[1]) {
			// this is a GOSUB, same as a GOTO but we need to first push the return pointer
			if(gosubindex >= MAXGOSUB) error("Too many nested GOSUB");
            errorstack[gosubindex] = CurrentLinePtr;
			gosubstack[gosubindex++] = nextstmt;
        	LocalIndex++;
		}

		if(isnamestart(*argv[r*2]))
			nextstmt = findlabel(argv[r*2]);						// must be a label
		else
			nextstmt = findline(getinteger(argv[r*2]), true);		// try for a line number
	}
    IgnorePIN = false;
}


// utility routine used by DoDim() below and other places in the interpreter
// checks if the type has been explicitly specified as in DIM FLOAT A, B, ... etc
char *CheckIfTypeSpecified(char *p, int *type, int AllowDefaultType) {
    char *tp;
#ifdef STRUCTENABLED
	g_StructArg = -1; // Reset struct index
#endif
    if((tp = checkstring(p, "INTEGER")) != NULL)
        *type = T_INT | T_IMPLIED;
    else if((tp = checkstring(p, "STRING")) != NULL)
        *type = T_STR | T_IMPLIED;
    else if((tp = checkstring(p, "FLOAT")) != NULL)
        *type = T_NBR | T_IMPLIED;
#ifdef STRUCTENABLED
	else
	{
		// Check if it's a structure type name
		skipspace(p); // Skip any leading whitespace before type name
		int structidx = FindStructType(p);
		if (structidx >= 0)
		{
			*type = T_STRUCT | T_IMPLIED;
			g_StructArg = structidx; // Store struct index in global
			// Advance past the type name
			tp = p;
			while (isnamechar(*tp))
				tp++;
			skipspace(tp);
		}
		else
		{
			if (!AllowDefaultType)
				error("Variable type");
			tp = p;
			*type = DefaultType; // if the type is not specified use the default
		}
	}
#else
    else {
        if(!AllowDefaultType) error("Variable type");
        tp = p;
        *type = DefaultType;                                        // if the type is not specified use the default
    }
#endif
    return tp;
}



char *SetValue(char *p, int t, void *v) {
    MMFLOAT f;
    long long int i64;
    char *s;
    char __attribute__ ((aligned (4))) TempCurrentSubFunName[MAXVARLEN + 1];
    varnamecopy(TempCurrentSubFunName, CurrentSubFunName);			    // save the current sub/fun name
	if(t & T_STR) {
		p = evaluate(p, &f, &i64, &s, &t, true);
		Mstrcpy(v, s);
	}
	else if(t & T_NBR) {
		p = evaluate(p, &f, &i64, &s, &t, false);
		if(t & T_NBR)
            (*(MMFLOAT *)v) = f;
        else
            (*(MMFLOAT *)v) = (MMFLOAT)i64;
	} else {
		p = evaluate(p, &f, &i64, &s, &t, false);
		if(t & T_INT)
            (*(long long int *)v) = i64;
        else
            (*(long long int *)v) = FloatToInt64(f);
	}
	varnamecopy(CurrentSubFunName, TempCurrentSubFunName);			    // restore the current sub/fun name
    return p;
}



// define a variable
// DIM [AS INTEGER|FLOAT|STRING] var[(d1 [,d2,...]] [AS INTEGER|FLOAT|STRING] [, ..., ...]
// LOCAL also uses this function the routines only differ in that LOCAL can only be used in a sub/fun
void MIPS16 cmd_dim(void) {
    int i, j, k, type, typeSave, ImpliedType = 0, VIndexSave, StaticVar = false;
    char *p, chSave, *chPosit;
    char VarName[(MAXVARLEN * 2) + 1];
    void *v, *tv;

    if(*cmdline == tokenAS) cmdline++;                              // this means that we can use DIM AS INTEGER a, b, etc
    p = CheckIfTypeSpecified(cmdline, &type, true);                 // check for DIM FLOAT A, B, ...
    ImpliedType = type;
    {                                                               // getargs macro must be the first executable stmt in a block
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");
        if((argc & 0x01) == 0) error("Syntax");

        for(i = 0; i < argc; i += 2) {
            p = skipvar(argv[i], false);                            // point to after the variable
            while(!(*p == 0 || *p == tokenAS || *p == '\'' || *p == tokenEQUAL))
                p++;                                                // skip over a LENGTH keyword if there and see if we can find "AS"
            chSave = *p; chPosit = p; *p = 0;                       // save the char then terminate the string so that LENGTH is evaluated correctly
            if(chSave == tokenAS) {                                 // are we using Microsoft syntax (eg, AS INTEGER)?
                if(ImpliedType & T_IMPLIED) error("Type specified twice");
                p++;                  // step over the AS token
#ifdef STRUCTENABLED
				skipspace(p);							           // skip any whitespace after AS
#endif
                p = CheckIfTypeSpecified(p, &type, true);           // and get the type
                if(!(type & T_IMPLIED)) error("Variable type");
            }

            if(cmdtoken == cmdLOCAL) {
                if(LocalIndex == 0) error("Invalid here");
                type |= V_LOCAL;                                    // local if defined in a sub/fun
            }

            if(cmdtoken == cmdSTATIC) {
                if(LocalIndex == 0) error("Invalid here");
                // create a unique global name
                if(*CurrentInterruptName)
                    strcpy(VarName, CurrentInterruptName);          // we must be in an interrupt sub
                else
                    strcpy(VarName, CurrentSubFunName);             // normal sub/fun
                for(k = 1; k <= MAXVARLEN; k++)
                	if(!isnamechar(VarName[k]))
                	{
                    VarName[k] = 0;                                 // terminate the string on a non valid char
                    break;
                    }
#ifdef STRUCTENABLED
    			strcat((char *)VarName, "\x1e");		  // use 0x1E (record separator) to avoid conflict with struct member syntax
    			strcat((char *)VarName, (char *)argv[i]); // by prefixing the var name with the sub/fun name
    			StaticVar = NAMELEN_STATIC;				  // flag for marking the variable as static
#else
                strcat(VarName, argv[i]);                           // by prefixing the var name with the sub/fun name
                StaticVar = true;
#endif
            } else
                strcpy(VarName, argv[i]);

            v = findvar(VarName, type | V_NOFIND_NULL);             // check if the variable exists
            typeSave = type;
            VIndexSave = VarIndex;
            if(v == NULL) {                                         // if not found
                v = findvar(VarName, type | V_FIND | V_DIM_VAR);    // create the variable
                type = TypeMask(vartbl[VarIndex].type);
                VIndexSave = VarIndex;
#ifdef STRUCTENABLED
				// Mark static variables with NAMELEN_STATIC so struct member lookup skips them
				if (StaticVar)
					vartbl[VIndexSave].namelen |= NAMELEN_STATIC;
#endif
                *chPosit = chSave;                                  // restore the char previously removed
                if(vartbl[VarIndex].dims[0] == -1) error("Array dimensions");
                if(vartbl[VarIndex].dims[0] > 0) {
                    DimUsed = true;                                 // prevent OPTION BASE from being used
                    v = vartbl[VarIndex].val.s;
                }
                while(*p && *p != '\'' && tokenfunction(*p) != op_equal) p++;   // search through the line looking for the equals sign
                if(tokenfunction(*p) == op_equal) {
                    p++;                                            // step over the equals sign
                    skipspace(p);
#ifdef STRUCTENABLED
					// Handle struct initialization: DIM var AS StructType = (val1, val2, ...)
					if (vartbl[VarIndex].type & T_STRUCT)
					{
						if (*p != '(')
							error("Expected '(' for structure initialisation");

						int struct_idx = (int)vartbl[VIndexSave].size;
						struct s_structdef *sd = g_structtbl[struct_idx];
						int struct_size = sd->total_size;
						char *struct_ptr = (char *)v;

						// Calculate number of struct elements (1 for simple, more for array)
						int num_elements = 1;
						if (vartbl[VIndexSave].dims[0] > 0)
						{
							for (j = 1, k = 0; k < MAXDIM && vartbl[VIndexSave].dims[k]; k++)
							{
								num_elements *= (vartbl[VIndexSave].dims[k] + 1 - OptionBase);
							}
						}

						p++; // step over opening '('
						skipspace(p);

						// Process each struct element
						for (int elem = 0; elem < num_elements; elem++)
						{
							// Process each member of the struct
							for (int m = 0; m < sd->num_members; m++)
							{
								struct s_structmember *member = &sd->members[m];
								char *member_ptr = struct_ptr + member->offset;

								// Calculate number of array elements for this member (1 if not array)
								int member_elements = 1;
								if (member->dims[0] != 0)
								{
									for (k = 0; k < MAXDIM && member->dims[k]; k++)
									{
										member_elements *= (member->dims[k] + 1 - OptionBase);
									}
								}

								// Process each element of member array (or just 1 if not array)
								for (int me = 0; me < member_elements; me++)
								{
									skipspace(p);
									if (*p == ')' || *p == 0)
										error("Not enough initialisation values");

									// Determine member size for pointer advancement
									int elem_size = 0;
									if (member->type & T_STR)
										elem_size = member->size + 1;
									else if (member->type & T_NBR)
										elem_size = sizeof(MMFLOAT);
									else if (member->type & T_INT)
										elem_size = sizeof(long long int);
									else
										error("Unsupported member type in initialisation");

									// Use SetValue to parse and assign the value
									p = SetValue(p, member->type, member_ptr);
									member_ptr += elem_size;

									skipspace(p);
									// Check for comma (more values) or closing paren
									if (*p == ',')
									{
										p++; // skip comma
									}
									else if (*p != ')')
									{
										error("Expected ',' or ')' in structure initialisation");
									}
								}
							}
							// Move to next struct element in array
							struct_ptr += struct_size;
						}

						skipspace(p);
						if (*p != ')')
							error("Expected ')' at end of structure initialisation");
					}
					else
#endif
                    if(vartbl[VarIndex].dims[0] > 0 && *p == '(') {
                        // calculate the overall size of the array
                        for(j = 1, k = 0; k < MAXDIM && vartbl[VIndexSave].dims[k]; k++) {
                            j *= (vartbl[VIndexSave].dims[k] + 1 - OptionBase);
                        }
                        do {
                            p++;                                    // step over the opening bracket or terminating comma
                            p = SetValue(p, type, v);
                            if(type & T_STR) v = (char *)v + vartbl[VIndexSave].size + 1;
                            if(type & T_NBR) v = (char *)v + sizeof(MMFLOAT);
                            if(type & T_INT) v = (char *)v + sizeof(long long int);
                            skipspace(p); j--;
                        } while(j > 0 && *p == ',');
                        if(*p != ')') error("Number of initialising values");
                        if(j != 0) error("Number of initialising values");
                    } else
                        SetValue(p, type, v);
                }
                type = ImpliedType;
            } else {
                if(!StaticVar) error("$ already declared", VarName);
            }


            // if it is a STATIC var create a local var pointing to the global var
            if(StaticVar) {
                tv = findvar(argv[i], typeSave | V_LOCAL | V_NOFIND_NULL);                        // check if the local variable exists
                if(tv != NULL) error("$ already declared", argv[i]);
                tv = findvar(argv[i], typeSave | V_LOCAL | V_FIND | V_DIM_VAR);                   // create the variable
#ifdef STRUCTENABLED
                if(vartbl[VIndexSave].dims[0] > 0 || (vartbl[VIndexSave].type & (T_STR | T_STRUCT))) {
#else
                if(vartbl[VIndexSave].dims[0] > 0 || (vartbl[VIndexSave].type & T_STR)) {
#endif
                	FreeMemory(tv);                                                               // we don't need the memory allocated to the local
                    vartbl[VarIndex].val.s = vartbl[VIndexSave].val.s;                            // point to the memory of the global variable
                } else
                    vartbl[VarIndex].val.ia = &(vartbl[VIndexSave].val.i);                        // point to the data of the variable
                vartbl[VarIndex].type = vartbl[VIndexSave].type | T_PTR;                          // set the type to a pointer
                vartbl[VarIndex].size = vartbl[VIndexSave].size;                                  // just in case it is a string copy the size
                for(j = 0; j < MAXDIM; j++) vartbl[VarIndex].dims[j] = vartbl[VIndexSave].dims[j];// just in case it is an array copy the dimensions
            }
        }
    }
}

void MIPS16 cmd_const(void) {
    char *p;
    void *v;
    int i, type;

	getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
	if((argc & 0x01) == 0) error("Syntax");

    for(i = 0; i < argc; i += 2) {
        p = skipvar(argv[i], false);                                // point to after the variable
        skipspace(p);
        if(tokenfunction(*p) != op_equal) error("Syntax");  // must be followed by an equals sign
        p++;                                                        // step over the equals sign
        type = T_NOTYPE;
        v = DoExpression(p, &type);                                 // evaluate the constant's value
        type = TypeMask(type);
        type |= V_FIND | V_DIM_VAR | T_CONST | T_IMPLIED;
        if(LocalIndex != 0) type |= V_LOCAL;                        // local if defined in a sub/fun
        findvar(argv[i], type);                                     // create the variable
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid constant");
        if(TypeMask(vartbl[VarIndex].type) != TypeMask(type)) error("Invalid constant");
        else {
            if(type & T_NBR) vartbl[VarIndex].val.f = *(MMFLOAT *)v;           // and set its value
            if(type & T_INT) vartbl[VarIndex].val.i = *(long long int *)v;
            //if(type & T_STR) Mstrcpy(vartbl[VarIndex].val.s, (char *)v);
            // CONST string from Picomite 6.00.02RC5
            if(type & T_STR) {
 				if((char)*(char *)v<(MAXDIM-1)*sizeof(vartbl[VarIndex].dims[1])){
 					FreeMemorySafe((void **)&vartbl[VarIndex].val.s);
 					vartbl[VarIndex].val.s=(void *)&vartbl[VarIndex].dims[1];
 			   }
 			   Mstrcpy(vartbl[VarIndex].val.s, (char *)v);
            }
        }
    }
}


#ifdef STRUCTENABLED
// TYPE typename - At runtime, just skip to END TYPE (like SUB/FUN)
// Structure definition is processed in PrepareProgramExt
void cmd_type(void)
{
	 char *p;

	// At runtime, we just skip past the TYPE block
	// The structure definition was already built during PrepareProgram
	p = nextstmt;
	while (1)
	{
		p = GetNextCommand(p, NULL, (char *)"No matching END TYPE");
		CommandToken tkn = commandtbl_decode(p);
		if (tkn == cmdTYPE)
			error("Nested TYPE not allowed");
		if (tkn == cmdEND_TYPE)
		{
			skipelement(p);
			nextstmt = p;
			break;
		}
	}
}

// END TYPE - should never be executed directly (only reached via cmd_type skip)

void cmd_endtype(void)
{
	error("END TYPE without TYPE");
}

// STRUCT command - operations on structure variables
// Syntax:
//   STRUCT COPY source TO destination
//   STRUCT COPY source() TO destination()  - copy entire array
//   (future: STRUCT PRINT var, STRUCT CLEAR var, etc.)

void cmd_struct(void)

{
	 char *p;

	if ((p = checkstring(cmdline, ( char *)"COPY")) != NULL)
	{
		// STRUCT COPY source TO destination
		// STRUCT COPY source() TO destination()  - copy entire array
		 char *src_ptr, *dst_ptr;
		int src_idx, dst_idx, src_struct_type, dst_struct_type;
		char *tp;
		int src_is_array = 0, dst_is_array = 0;
		int src_num_elements = 1, dst_num_elements = 1;

		skipspace(p);

		// Get source variable - V_EMPTY_OK allows empty () for arrays
		src_ptr = findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		src_idx = VarIndex;

		// Check source is a struct (not a member access)
		if (!(vartbl[src_idx].type & T_STRUCT))
			error("Source must be a structure variable");

		// For struct arrays, the pointer returned is to the specific element
		// but we need to verify this is a whole struct, not a member
		if (g_StructMemberType != 0)
			error("Cannot copy structure member, use whole structure");

		src_struct_type = (int)vartbl[src_idx].size; // struct type index stored in size field

		// Check if source is an array with empty parentheses (whole array copy)
		// V_EMPTY_OK returns base pointer when () is empty
		if (vartbl[src_idx].dims[0] != 0)
		{
			// It's an array - check if empty parentheses were used
			char *paren = (char *)strchr((char *)p, '(');
			if (paren)
			{
				paren++;
				skipspace(paren);
				if (*paren == ')')
				{
					// Empty parentheses - whole array copy
					src_is_array = 1;
					for (int d = 0; d < MAXDIM && vartbl[src_idx].dims[d] != 0; d++)
					{
						src_num_elements *= (vartbl[src_idx].dims[d] + 1 - OptionBase);
					}
				}
			}
		}

		// Skip past the source variable to find TO
		tp = skipvar(p, false);
		skipspace(tp);

		// Check for TO keyword (tokenized)
		if (*tp != tokenTO)
			error("Expected TO");
		tp++; // skip TO token
		skipspace(tp);

		// Get destination variable
		dst_ptr = findvar(tp, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		dst_idx = VarIndex;

		// Check destination is a struct
		if (!(vartbl[dst_idx].type & T_STRUCT))
			error("Destination must be a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot copy to structure member, use whole structure");

		dst_struct_type = (int)vartbl[dst_idx].size;

		// Check if destination is an array with empty parentheses
		if (vartbl[dst_idx].dims[0] != 0)
		{
			char *paren = (char *)strchr((char *)tp, '(');
			if (paren)
			{
				paren++;
				skipspace(paren);
				if (*paren == ')')
				{
					dst_is_array = 1;
					for (int d = 0; d < MAXDIM && vartbl[dst_idx].dims[d] != 0; d++)
					{
						dst_num_elements *= (vartbl[dst_idx].dims[d] + 1 - OptionBase);
					}
				}
			}
		}

		// Validate same struct type
		if (src_struct_type != dst_struct_type)
			error("Structure types must match");

		// Validate array copy consistency
		if (src_is_array != dst_is_array)
			error("Both source and destination must be arrays or both must be single structs");

		// For array copy, destination must be at least as large as source
		if (src_is_array && dst_num_elements < src_num_elements)
			error("Destination array too small");

		// Perform the copy
		int struct_size = g_structtbl[src_struct_type]->total_size;
		int copy_size = struct_size * src_num_elements;
		memcpy(dst_ptr, src_ptr, copy_size);
	}
	else if ((p = checkstring(cmdline, ( char *)"SORT")) != NULL)
	{
		// STRUCT SORT array().membername [, flags]
		// flags: bit0=reverse, bit1=case insensitive (strings), bit2=empty strings at end (strings)
		int arr_idx, struct_type;
		char *tp;
		int flags = 0;
		int member_type, member_offset, member_size;
		int num_elements;
		int struct_size;

		skipspace(p);

		// Get array variable with member access: array().membername
		// findvar will resolve the member access and set g_StructMemberType/Offset/Size
		findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		arr_idx = VarIndex;

		// Check it's a struct array with member access
		if (!(vartbl[arr_idx].type & T_STRUCT))
			error("Expected a structure array");
		if (vartbl[arr_idx].dims[0] == 0)
			error("Expected a structure array");
		if (g_StructMemberType == 0)
			error("Expected structarray().membername syntax");

		// Get member info from globals set by findvar
		member_type = g_StructMemberType;
		member_offset = g_StructMemberOffset;
		member_size = g_StructMemberSize;
		(void)member_size; // Used for validation, suppress unused warning

		// Member cannot be a nested struct
		if (member_type == T_STRUCT)
			error("Cannot sort by nested structure member");

		struct_type = (int)vartbl[arr_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;

		// Calculate number of elements
		num_elements = 1;
		for (int d = 0; d < MAXDIM && vartbl[arr_idx].dims[d] != 0; d++)
		{
			num_elements *= (vartbl[arr_idx].dims[d] + 1 - OptionBase);
		}

		// Skip past array().membername to find optional flags
		tp = skipvar(p, false);
		skipspace(tp);

		// Check for optional flags parameter
		if (*tp == ',')
		{
			tp++;
			skipspace(tp);
			flags = (int)getint(tp, 0, 7);
		}

		// Get pointer to array data
		char *base_ptr = vartbl[arr_idx].val.s;

		// Allocate temporary buffer for swap
		char *temp = GetTempMemory(struct_size);

		// Shell sort implementation (efficient for medium arrays, in-place)
		int gap, i, j;
		int reverse = flags & 1;
		int case_insensitive = flags & 2;
		int empty_at_end = flags & 4;

		for (gap = num_elements / 2; gap > 0; gap /= 2)
		{
			for (i = gap; i < num_elements; i++)
			{
				memcpy(temp, base_ptr + i * struct_size, struct_size);

				for (j = i; j >= gap; j -= gap)
				{
					char *elem_j_gap = base_ptr + (j - gap) * struct_size;
					char *val_a = elem_j_gap + member_offset;
					char *val_b = temp + member_offset;
					int cmp = 0;

					// Compare based on member type
					if (member_type & T_INT)
					{
						long long int a = *(long long int *)val_a;
						long long int b = *(long long int *)val_b;
						if (a < b)
							cmp = -1;
						else if (a > b)
							cmp = 1;
						else
							cmp = 0;
					}
					else if (member_type & T_NBR)
					{
						MMFLOAT a = *(MMFLOAT *)val_a;
						MMFLOAT b = *(MMFLOAT *)val_b;
						if (a < b)
							cmp = -1;
						else if (a > b)
							cmp = 1;
						else
							cmp = 0;
					}
					else if (member_type & T_STR)
					{
						// MMBasic strings: first byte is length
						int len_a = *val_a;
						int len_b = *val_b;

						// Handle empty strings at end option
						if (empty_at_end)
						{
							if (len_a == 0 && len_b != 0)
							{
								cmp = 1; // Empty string a goes after b
							}
							else if (len_a != 0 && len_b == 0)
							{
								cmp = -1; // Non-empty a goes before empty b
							}
							else if (len_a == 0 && len_b == 0)
							{
								cmp = 0; // Both empty, equal
							}
							else
							{
								// Both non-empty, compare normally
								int minlen = (len_a < len_b) ? len_a : len_b;
								if (case_insensitive)
								{
									for (int k = 1; k <= minlen; k++)
									{
										int ca = toupper(val_a[k]);
										int cb = toupper(val_b[k]);
										if (ca < cb)
										{
											cmp = -1;
											break;
										}
										if (ca > cb)
										{
											cmp = 1;
											break;
										}
									}
								}
								else
								{
									for (int k = 1; k <= minlen; k++)
									{
										if (val_a[k] < val_b[k])
										{
											cmp = -1;
											break;
										}
										if (val_a[k] > val_b[k])
										{
											cmp = 1;
											break;
										}
									}
								}
								if (cmp == 0)
								{
									if (len_a < len_b)
										cmp = -1;
									else if (len_a > len_b)
										cmp = 1;
								}
							}
						}
						else
						{
							// Normal string comparison
							int minlen = (len_a < len_b) ? len_a : len_b;
							if (case_insensitive)
							{
								for (int k = 1; k <= minlen; k++)
								{
									int ca = toupper(val_a[k]);
									int cb = toupper(val_b[k]);
									if (ca < cb)
									{
										cmp = -1;
										break;
									}
									if (ca > cb)
									{
										cmp = 1;
										break;
									}
								}
							}
							else
							{
								for (int k = 1; k <= minlen; k++)
								{
									if (val_a[k] < val_b[k])
									{
										cmp = -1;
										break;
									}
									if (val_a[k] > val_b[k])
									{
										cmp = 1;
										break;
									}
								}
							}
							if (cmp == 0)
							{
								if (len_a < len_b)
									cmp = -1;
								else if (len_a > len_b)
									cmp = 1;
							}
						}
					}

					// Apply reverse flag
					if (reverse)
						cmp = -cmp;

					// If elem[j-gap] > temp, shift it up
					if (cmp > 0)
					{
						memcpy(base_ptr + j * struct_size, elem_j_gap, struct_size);
					}
					else
					{
						break;
					}
				}
				memcpy(base_ptr + j * struct_size, temp, struct_size);
			}
		}
	}
	else if ((p = checkstring(cmdline, (char *)"CLEAR")) != NULL)
	{
		// STRUCT CLEAR var or STRUCT CLEAR array()
		// Resets all members to defaults (0 for numbers, "" for strings)
		int var_idx, struct_type;
		char *var_ptr;
		int struct_size;

		skipspace(p);

		// Get variable - V_EMPTY_OK allows empty () for arrays
		var_ptr = findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		var_idx = VarIndex;

		// Check it's a struct
		if (!(vartbl[var_idx].type & T_STRUCT))
			error("Expected a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot clear a structure member, use whole structure");

		struct_type = (int)vartbl[var_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;

		// Calculate total size if array
		int num_elements = 1;
		if (vartbl[var_idx].dims[0] != 0)
		{
			for (int d = 0; d < MAXDIM && vartbl[var_idx].dims[d] != 0; d++)
			{
				num_elements *= (vartbl[var_idx].dims[d] + 1 - OptionBase);
			}
		}

		// Zero the memory
		memset(var_ptr, 0, struct_size * num_elements);
	}
	else if ((p = checkstring(cmdline, (char *)"SWAP")) != NULL)
	{
		// STRUCT SWAP var1, var2
		// Swaps two struct variables (must be same type)
		char *src_ptr, *dst_ptr;
		int src_idx, dst_idx;
		int src_struct_type, dst_struct_type;
		char *tp;

		skipspace(p);

		// Get first variable
		src_ptr = findvar(p, V_FIND | V_NOFIND_ERR);
		src_idx = VarIndex;

		if (!(vartbl[src_idx].type & T_STRUCT))
			error("First argument must be a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot swap structure member, use whole structure");

		src_struct_type = (int)vartbl[src_idx].size;

		// Skip past first variable to find comma
		tp = skipvar(p, false);
		skipspace(tp);

		if (*tp != ',')
			error("Expected comma");
		tp++;
		skipspace(tp);

		// Get second variable
		dst_ptr = findvar(tp, V_FIND | V_NOFIND_ERR);
		dst_idx = VarIndex;

		if (!(vartbl[dst_idx].type & T_STRUCT))
			error("Second argument must be a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot swap structure member, use whole structure");

		dst_struct_type = (int)vartbl[dst_idx].size;

		// Validate same struct type
		if (src_struct_type != dst_struct_type)
			error("Structure types must match");

		// Perform the swap using temp memory
		int swap_size = g_structtbl[src_struct_type]->total_size;
		char *temp = GetTempMemory(swap_size);
		memcpy(temp, src_ptr, swap_size);
		memcpy(src_ptr, dst_ptr, swap_size);
		memcpy(dst_ptr, temp, swap_size);
	}

	else if ((p = checkstring(cmdline, (char *)"SAVE")) != NULL)
	{
		// STRUCT SAVE #n, var or STRUCT SAVE #n, array() or STRUCT SAVE #n, array(i)
		// Writes struct data as binary to open file
		int fnbr;
		int var_idx, struct_type, struct_size;
		unsigned char *var_ptr;

		skipspace(p);

		// Get file number
		if (*p != '#')
			error("Expected #filenumber");
		p++;
		fnbr = getinteger(p);

		// Validate file number and that it's a disk file
		if (fnbr < 1 || fnbr > MAXOPENFILES)
			error("Invalid file number");
		if (FileTable[fnbr].com == 0)
			error("File not open");
		if (FileTable[fnbr].com <= MAXCOMPORTS)
			error("Not a disk file");

		// Skip past file number to comma
		while (*p && *p != ',')
			p++;
		if (*p != ',')
			error("Expected comma");
		p++;
		skipspace(p);

		// Save pointer to variable name for parenthesis check
		char *varname = p;

		// Get variable - V_EMPTY_OK allows empty () for arrays
		var_ptr = findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		var_idx = VarIndex;

		if (!(vartbl[var_idx].type & T_STRUCT))
			error("Expected a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot save a structure member, use whole structure");

		struct_type = (int)vartbl[var_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;

		// Determine if it's an array and how to handle it
		int num_elements = 1;
		int is_array = (vartbl[var_idx].dims[0] != 0);

		if (is_array)
		{
			// Check for parentheses
			char *paren = (char *)strchr((char *)varname, '(');
			if (!paren)
				error("Array variable requires () or (index)");

			paren++;
			skipspace(paren);
			if (*paren == ')')
			{
				// Empty brackets - save entire array
				for (int d = 0; d < MAXDIM && vartbl[var_idx].dims[d] != 0; d++)
				{
					num_elements *= (vartbl[var_idx].dims[d] + 1 - OptionBase);
				}
			}
			else
			{
				// Has index - findvar already resolved to the correct element
				// var_ptr points to the specific element, save just one
				num_elements = 1;
			}
		}

		// Write struct data to file
		FilePutData((char *)var_ptr, fnbr, struct_size * num_elements);
	}

	else if ((p = checkstring(cmdline, (char *)"LOAD")) != NULL)
	{
		// STRUCT LOAD #n, var or STRUCT LOAD #n, array() or STRUCT LOAD #n, array(i)
		// Reads struct data as binary from open file
		int fnbr;
		int var_idx, struct_type, struct_size;
		char *var_ptr;
		unsigned int bytes_read;

		skipspace(p);

		// Get file number
		if (*p != '#')
			error("Expected #filenumber");
		p++;
		fnbr = getinteger(p);

		// Validate file number and that it's a disk file
		if (fnbr < 1 || fnbr > MAXOPENFILES)
			error("Invalid file number");
		if (FileTable[fnbr].com == 0)
			error("File not open");
		if (FileTable[fnbr].com <= MAXCOMPORTS)
			error("Not a disk file");

		// Skip past file number to comma
		while (*p && *p != ',')
			p++;
		if (*p != ',')
			error("Expected comma");
		p++;
		skipspace(p);

		// Save pointer to variable name for parenthesis check
		char *varname = p;

		// Get variable - V_EMPTY_OK allows empty () for arrays
		var_ptr = findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		var_idx = VarIndex;

		if (!(vartbl[var_idx].type & T_STRUCT))
			error("Expected a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot load into a structure member, use whole structure");

		struct_type = (int)vartbl[var_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;

		// Determine if it's an array and how to handle it
		int num_elements = 1;
		int is_array = (vartbl[var_idx].dims[0] != 0);

		if (is_array)
		{
			// Check for parentheses
			char *paren = (char *)strchr((char *)varname, '(');
			if (!paren)
				error("Array variable requires () or (index)");

			paren++;
			skipspace(paren);
			if (*paren == ')')
			{
				// Empty brackets - load entire array
				for (int d = 0; d < MAXDIM && vartbl[var_idx].dims[d] != 0; d++)
				{
					num_elements *= (vartbl[var_idx].dims[d] + 1 - OptionBase);
				}
			}
			else
			{
				// Has index - findvar already resolved to the correct element
				// var_ptr points to the specific element, load just one
				num_elements = 1;
			}
		}

		// Read struct data from file
		FileGetData(fnbr, var_ptr, struct_size * num_elements, &bytes_read);
	}
	else if ((p = checkstring(cmdline, (char *)"PRINT")) != NULL)
	{
		// STRUCT PRINT var or STRUCT PRINT array() or STRUCT PRINT array(n)
		// Prints all members of a structure for debugging
		int var_idx, struct_type, struct_size;
		unsigned char *var_ptr;
		struct s_structdef *sd;

		skipspace(p);

		// Get variable - V_EMPTY_OK allows empty () for arrays
		var_ptr = findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		var_idx = VarIndex;

		if (!(vartbl[var_idx].type & T_STRUCT))
			error("Expected a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot print a structure member, use whole structure");

		struct_type = (int)vartbl[var_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;
		sd = g_structtbl[struct_type];

		// Calculate number of elements to print
		int num_elements = 1;
		int is_array = (vartbl[var_idx].dims[0] != 0);
		int single_element = 0;

		// Check if this is an indexed array access (e.g., arr(2)) vs whole array (arr())
		// If findvar resolved to a specific element, var_ptr points to that element
		// We detect this by checking if parentheses contain a value
		unsigned char *paren = (unsigned char *)strchr((char *)p, '(');
		if (paren && is_array)
		{
			paren++;
			skipspace(paren);
			if (*paren != ')')
			{
				// Has an index - print single element
				single_element = 1;
				num_elements = 1;
			}
		}

		if (is_array && !single_element)
		{
			for (int d = 0; d < MAXDIM && vartbl[var_idx].dims[d] != 0; d++)
			{
				num_elements *= (vartbl[var_idx].dims[d] + 1 - OptionBase);
			}
		}

		// Print structure type name
		MMPrintString((char *)sd->name);
		if (is_array && !single_element)
		{
			char buf[32];
			sprintf(buf, " array (%d elements):\r\n", num_elements);
			MMPrintString(buf);
		}
		else if (single_element)
		{
			MMPrintString(":\r\n");
		}
		else
		{
			MMPrintString(":\r\n");
		}

		// Print each element
		for (int elem = 0; elem < num_elements; elem++)
		{
			unsigned char *elem_ptr = var_ptr + (elem * struct_size);

			if (is_array && !single_element)
			{
				char buf[32];
				sprintf(buf, "[%d]:\r\n", elem + OptionBase);
				MMPrintString(buf);
			}

			// Print each member
			for (int m = 0; m < sd->num_members; m++)
			{
				struct s_structmember *sm = &sd->members[m];
				unsigned char *member_ptr = elem_ptr + sm->offset;
				char buf[STRINGSIZE];

				// Calculate array elements for this member
				int member_elements = 1;
				for (int d = 0; d < MAXDIM && sm->dims[d] != 0; d++)
				{
					member_elements *= (sm->dims[d] + 1 - OptionBase);
				}

				if (sm->type == T_STRUCT)
				{
					// Nested structure - print recursively with indentation
					struct s_structdef *nested_sd = g_structtbl[sm->size];
					sprintf(buf, "  .%s = %s:\r\n", sm->name, nested_sd->name);
					MMPrintString(buf);

					// Print nested members with extra indent
					for (int nm = 0; nm < nested_sd->num_members; nm++)
					{
						struct s_structmember *nsm = &nested_sd->members[nm];
						unsigned char *nested_ptr = member_ptr + nsm->offset;

						sprintf(buf, "    .%s = ", nsm->name);
						MMPrintString(buf);

						if (nsm->type == T_INT)
						{
							long long int val = *(long long int *)nested_ptr;
							//sprintf(buf, "%lld", val);
							//MMPrintString(buf);
							PInt(val);
						}
						else if (nsm->type == T_NBR)
						{
							MMFLOAT val = *(MMFLOAT *)nested_ptr;
							sprintf(buf, "%g", val);
							MMPrintString(buf);
						}
						else if (nsm->type == T_STR)
						{
							MMPrintString("\"");
							int len = *nested_ptr;
							for (int c = 0; c < len; c++)
							{
								char ch[2] = {nested_ptr[c + 1], 0};
								MMPrintString(ch);
							}
							MMPrintString("\"");
						}
						else if (nsm->type == T_STRUCT)
						{
							MMPrintString("(nested struct - use deeper access)");
						}
						MMPrintString("\r\n");
					}
				}
				else if (member_elements == 1)
				{
					// Simple member (not an array)
					sprintf(buf, "  .%s = ", sm->name);
					MMPrintString(buf);

					if (sm->type == T_INT)
					{
						long long int val = *(long long int *)member_ptr;
						//sprintf(buf, "%lld", val);
						//MMPrintString(buf);
						PInt(val);
					}
					else if (sm->type == T_NBR)
					{
						MMFLOAT val = *(MMFLOAT *)member_ptr;
						sprintf(buf, "%g", val);
						MMPrintString(buf);
					}
					else if (sm->type == T_STR)
					{
						MMPrintString("\"");
						// String: first byte is length
						int len = *member_ptr;
						for (int c = 0; c < len; c++)
						{
							char ch[2] = {member_ptr[c + 1], 0};
							MMPrintString(ch);
						}
						MMPrintString("\"");
					}
					MMPrintString("\r\n");
				}
				else
				{
					// Array member
					sprintf(buf, "  .%s() = ", sm->name);
					MMPrintString(buf);

					int elem_size;
					if (sm->type == T_STR)
						elem_size = sm->size + 1; // +1 for length byte
					else
						elem_size = sm->size;

					for (int ai = 0; ai < member_elements; ai++)
					{
						unsigned char *arr_ptr = member_ptr + (ai * elem_size);

						if (ai > 0)
							MMPrintString(", ");

						if (sm->type == T_INT)
						{
							long long int val = *(long long int *)arr_ptr;
							//sprintf(buf, "%lld", val);
							//MMPrintString(buf);
							PInt(val);
						}
						else if (sm->type == T_NBR)
						{
							MMFLOAT val = *(MMFLOAT *)arr_ptr;
							sprintf(buf, "%g", val);
							MMPrintString(buf);
						}
						else if (sm->type == T_STR)
						{
							MMPrintString("\"");
							int len = *arr_ptr;
							for (int c = 0; c < len; c++)
							{
								char ch[2] = {arr_ptr[c + 1], 0};
								MMPrintString(ch);
							}
							MMPrintString("\"");
						}
					}
					MMPrintString("\r\n");
				}
			}
		}
	}
	/*
	else if ((p = checkstring(cmdline, (char *)"PRINT")) != NULL)
	{
		// STRUCT PRINT var or STRUCT PRINT array() or STRUCT PRINT array(n)
		// Prints all members of a structure for debugging
		int var_idx, struct_type, struct_size;
		char *var_ptr;
		struct s_structdef *sd;

		skipspace(p);

		// Get variable - V_EMPTY_OK allows empty () for arrays
		var_ptr = findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		var_idx = VarIndex;

		if (!(vartbl[var_idx].type & T_STRUCT))
			error("Expected a structure variable");

		if (g_StructMemberType != 0)
			error("Cannot print a structure member, use whole structure");

		struct_type = (int)vartbl[var_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;
		sd = g_structtbl[struct_type];

		// Calculate number of elements to print
		int num_elements = 1;
		int is_array = (vartbl[var_idx].dims[0] != 0);
		int single_element = 0;

		// Check if this is an indexed array access (e.g., arr(2)) vs whole array (arr())
		// If findvar resolved to a specific element, var_ptr points to that element
		// We detect this by checking if parentheses contain a value
		char *paren = (char *)strchr((char *)p, '(');
		if (paren && is_array)
		{
			paren++;
			skipspace(paren);
			if (*paren != ')')
			{
				// Has an index - print single element
				single_element = 1;
				num_elements = 1;
			}
		}

		if (is_array && !single_element)
		{
			for (int d = 0; d < MAXDIM && vartbl[var_idx].dims[d] != 0; d++)
			{
				num_elements *= (vartbl[var_idx].dims[d] + 1 - OptionBase);
			}
		}

		// Print structure type name
		MMPrintString((char *)sd->name);
		if (is_array && !single_element)
		{
			char buf[32];
			sprintf(buf, " array (%d elements):\r\n", num_elements);
			MMPrintString(buf);
		}
		else if (single_element)
		{
			MMPrintString(":\r\n");
		}
		else
		{
			MMPrintString(":\r\n");
		}

		// Print each element
		for (int elem = 0; elem < num_elements; elem++)
		{
			char *elem_ptr = var_ptr + (elem * struct_size);

			if (is_array && !single_element)
			{
				char buf[32];
				sprintf(buf, "[%d]:\r\n", elem + OptionBase);
				MMPrintString(buf);
			}

			// Print each member
			for (int m = 0; m < sd->num_members; m++)
			{
				struct s_structmember *sm = &sd->members[m];
				char *member_ptr = elem_ptr + sm->offset;
				char buf[STRINGSIZE];

				// Calculate array elements for this member
				int member_elements = 1;
				for (int d = 0; d < MAXDIM && sm->dims[d] != 0; d++)
				{
					member_elements *= (sm->dims[d] + 1 - OptionBase);
				}

				if (sm->type == T_STRUCT)
				{
					// Nested structure - print recursively with indentation
					struct s_structdef *nested_sd = g_structtbl[sm->size];
					sprintf(buf, "  .%s = %s:\r\n", sm->name, nested_sd->name);
					MMPrintString(buf);

					// Print nested members with extra indent
					for (int nm = 0; nm < nested_sd->num_members; nm++)
					{
						struct s_structmember *nsm = &nested_sd->members[nm];
						char *nested_ptr = member_ptr + nsm->offset;

						sprintf(buf, "    .%s = ", nsm->name);
						MMPrintString(buf);

						if (nsm->type == T_INT)
						{
							long long int val = *(long long int *)nested_ptr; //here nested
							sprintf(buf, "%lld", val);
							MMPrintString(buf);

							//IntToStr(buf,(int64_t)val,10);
							//PInt(val);//PRet();

						}
						else if (nsm->type == T_NBR)
						{
							MMFLOAT val = *(MMFLOAT *)nested_ptr;
							sprintf(buf, "%g", val);
							MMPrintString(buf);
						}
						else if (nsm->type == T_STR)
						{
							MMPrintString("\"");
							int len = *nested_ptr;
							for (int c = 0; c < len; c++)
							{
								char ch[2] = {nested_ptr[c + 1], 0};
								MMPrintString(ch);
							}
							MMPrintString("\"");
						}
						else if (nsm->type == T_STRUCT)
						{
							MMPrintString("(nested struct - use deeper access)");
						}
						MMPrintString("\r\n");
					}
				}
				else if (member_elements == 1)
				{
					// Simple member (not an array)
					sprintf(buf, "  .%s = ", sm->name);
					MMPrintString(buf);

					if (sm->type == T_INT)
					{
						long long int val = *(long long int *)member_ptr;  // here single
						//sprintf(buf, "%lld", val);
						//MMPrintString(buf);

						//IntToStr(buf,(int64_t)val,10);
						PInt(val);//PRet();
					}
					else if (sm->type == T_NBR)
					{
						MMFLOAT val = *(MMFLOAT *)member_ptr;
						sprintf(buf, "%g", val);
						MMPrintString(buf);
					}
					else if (sm->type == T_STR)
					{
						MMPrintString("\"");
						// String: first byte is length
						int len = *member_ptr;
						for (int c = 0; c < len; c++)
						{
							char ch[2] = {member_ptr[c + 1], 0};
							MMPrintString(ch);
						}
						MMPrintString("\"");
					}
					MMPrintString("\r\n");
				}
				else
				{
					// Array member
					sprintf(buf, "  .%s() = ", sm->name);
					MMPrintString(buf);

					int elem_size;
					if (sm->type == T_STR)
						elem_size = sm->size + 1; // +1 for length byte
					else
						elem_size = sm->size;

					for (int ai = 0; ai < member_elements; ai++)
					{
						char *arr_ptr = member_ptr + (ai * elem_size);

						if (ai > 0)
							MMPrintString(", ");

						if (sm->type == T_INT)
						{
							long long int val = *(long long int *)arr_ptr;  //here array
							sprintf(buf, "%lld", val);
							MMPrintString(buf);

						}
						else if (sm->type == T_NBR)
						{
							MMFLOAT val = *(MMFLOAT *)arr_ptr;
							sprintf(buf, "%g", val);
							MMPrintString(buf);
						}
						else if (sm->type == T_STR)
						{
							MMPrintString("\"");
							int len = *arr_ptr;
							for (int c = 0; c < len; c++)
							{
								char ch[2] = {arr_ptr[c + 1], 0};
								MMPrintString(ch);
							}
							MMPrintString("\"");
						}
					}
					MMPrintString("\r\n");
				}
			}
		}
	}
	*/
	else if ((p = checkstring(cmdline, (char *)"EXTRACT")) != NULL)
	{
		// STRUCT EXTRACT structarray().membername, destarray()
		// Extracts a single member from each structure in an array into a simple array
		// This allows structure data to be used with commands that expect contiguous arrays
		// (e.g., LINE PLOT, MATH commands)
		int src_idx, dst_idx, struct_type, struct_size;
		char *tp;
		int member_type, member_offset, member_size;
		int src_num_elements, dst_num_elements;
		char *src_base, *dst_base;

		skipspace(p);

		// Get source struct array with member access: structarray().membername
		// findvar will resolve the member access and set g_StructMemberType/Offset/Size
		findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		src_idx = VarIndex;

		// Check it's a struct array with member access
		if (!(vartbl[src_idx].type & T_STRUCT))
			error("Expected a structure array");
		if (vartbl[src_idx].dims[0] == 0)
			error("Expected a structure array, not a single structure");
		if (vartbl[src_idx].dims[1] != 0)
			error("Only 1-dimensional structure arrays are supported");
		if (g_StructMemberType == 0)
			error("Expected structarray().membername syntax");

		// Get member info from globals set by findvar
		member_type = g_StructMemberType;
		member_offset = g_StructMemberOffset;
		member_size = g_StructMemberSize;

		// Member cannot be a nested struct
		if (member_type == T_STRUCT)
			error("Cannot extract nested structure member");

		struct_type = (int)vartbl[src_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;

		// Calculate number of source elements
		src_num_elements = vartbl[src_idx].dims[0] + 1 - OptionBase;
		src_base = vartbl[src_idx].val.s;

		// Skip past structarray().membername to find the comma
		tp = skipvar(p, false);
		skipspace(tp);

		if (*tp != ',')
			error("Expected comma and destination array");
		tp++;
		skipspace(tp);

		// Get destination array with empty ()
		dst_base = findvar(tp, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		dst_idx = VarIndex;

		// Check destination is a simple array (not struct)
		if (vartbl[dst_idx].type & T_STRUCT)
			error("Destination must be a simple array, not a structure");
		if (vartbl[dst_idx].dims[0] == 0)
			error("Destination must be an array");
		if (vartbl[dst_idx].dims[1] != 0)
			error("Destination must be a 1-dimensional array");

		// Calculate destination array size
		dst_num_elements = vartbl[dst_idx].dims[0] + 1 - OptionBase;

		// Check cardinality matches
		if (dst_num_elements != src_num_elements)
			error("Arrays must have the same size (source=%, dest=%)", src_num_elements, dst_num_elements);

		// Check types match
		int dst_type = vartbl[dst_idx].type & (T_INT | T_NBR | T_STR);
		if (dst_type != member_type)
			error("Type mismatch: structure member and destination array must have same type");

		// For strings, check length matches
		if (member_type == T_STR)
		{
			int dst_str_size = vartbl[dst_idx].size; // max string length for dest array
			if (dst_str_size != member_size)
				error("String length mismatch: member length=%, array length=%", member_size, dst_str_size);
		}

		// Perform the extraction
		if (member_type == T_INT)
		{
			long long int *dst = (long long int *)dst_base;
			for (int i = 0; i < src_num_elements; i++)
			{
				char *src_elem = src_base + (i * struct_size) + member_offset;
				dst[i] = *(long long int *)src_elem;
			}
		}
		else if (member_type == T_NBR)
		{
			MMFLOAT *dst = (MMFLOAT *)dst_base;
			for (int i = 0; i < src_num_elements; i++)
			{
				char *src_elem = src_base + (i * struct_size) + member_offset;
				dst[i] = *(MMFLOAT *)src_elem;
			}
		}
		else if (member_type == T_STR)
		{
			int str_size = member_size + 1; // +1 for length byte
			for (int i = 0; i < src_num_elements; i++)
			{
				char *src_elem = src_base + (i * struct_size) + member_offset;
				char *dst_elem = dst_base + (i * str_size);
				memcpy(dst_elem, src_elem, str_size);
			}
		}
	}
	else if ((p = checkstring(cmdline, (char *)"INSERT")) != NULL)
	{
		// STRUCT INSERT srcarray(), structarray().membername
		// Inserts values from a simple array into the specified member of each structure element
		// This is the reverse of STRUCT EXTRACT
		int src_idx, dst_idx, struct_type, struct_size;
		char *tp;
		int member_type, member_offset, member_size;
		int src_num_elements, dst_num_elements;
		char *src_base, *dst_base;

		skipspace(p);

		// Get source simple array variable with empty ()
		src_base = findvar(p, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		src_idx = VarIndex;

		// Check source is a simple array (not struct)
		if (vartbl[src_idx].type & T_STRUCT)
			error("Source must be a simple array, not a structure");
		if (vartbl[src_idx].dims[0] == 0)
			error("Source must be an array");
		if (vartbl[src_idx].dims[1] != 0)
			error("Source must be a 1-dimensional array");

		// Calculate source array size
		src_num_elements = vartbl[src_idx].dims[0] + 1 - OptionBase;
		int src_type = vartbl[src_idx].type & (T_INT | T_NBR | T_STR);
		int src_str_size = vartbl[src_idx].size; // for strings

		// Skip past source array to find comma
		tp = skipvar(p, false);
		skipspace(tp);

		if (*tp != ',')
			error("Expected comma after source array");
		tp++;
		skipspace(tp);

		// Get destination struct array with member access: structarray().membername
		// findvar will resolve the member access and set g_StructMemberType/Offset/Size
		dst_base = findvar(tp, V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		dst_idx = VarIndex;

		// Check it's a struct array with member access
		if (!(vartbl[dst_idx].type & T_STRUCT))
			error("Expected a structure array");
		if (vartbl[dst_idx].dims[0] == 0)
			error("Expected a structure array, not a single structure");
		if (vartbl[dst_idx].dims[1] != 0)
			error("Only 1-dimensional structure arrays are supported");
		if (g_StructMemberType == 0)
			error("Expected structarray().membername syntax");

		// Get member info from globals set by findvar
		member_type = g_StructMemberType;
		member_offset = g_StructMemberOffset;
		member_size = g_StructMemberSize;

		// Member cannot be a nested struct
		if (member_type == T_STRUCT)
			error("Cannot insert into nested structure member");

		struct_type = (int)vartbl[dst_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;

		// Calculate number of destination elements
		dst_num_elements = vartbl[dst_idx].dims[0] + 1 - OptionBase;

		// Use dst_base from the struct variable, not from member resolution
		dst_base = vartbl[dst_idx].val.s;

		// Check cardinality matches
		if (src_num_elements != dst_num_elements)
			error("Arrays must have the same size (source=%, dest=%)", src_num_elements, dst_num_elements);

		// Check types match
		if (src_type != member_type)
			error("Type mismatch: source array and structure member must have same type");

		// For strings, check length matches
		if (member_type == T_STR)
		{
			if (src_str_size != member_size)
				error("String length mismatch: array length=%, member length=%", src_str_size, member_size);
		}

		// Perform the insertion
		if (member_type == T_INT)
		{
			long long int *src = (long long int *)src_base;
			for (int i = 0; i < dst_num_elements; i++)
			{
				 char *dst_elem = dst_base + (i * struct_size) + member_offset;
				*(long long int *)dst_elem = src[i];
			}
		}
		else if (member_type == T_NBR)
		{
			MMFLOAT *src = (MMFLOAT *)src_base;
			for (int i = 0; i < dst_num_elements; i++)
			{
				char *dst_elem = dst_base + (i * struct_size) + member_offset;
				*(MMFLOAT *)dst_elem = src[i];
			}
		}
		else if (member_type == T_STR)
		{
			int str_size = member_size + 1; // +1 for length byte
			for (int i = 0; i < dst_num_elements; i++)
			{
				char *src_elem = src_base + (i * str_size);
				char *dst_elem = dst_base + (i * struct_size) + member_offset;
				memcpy(dst_elem, src_elem, str_size);
			}
		}
	}
	else
	{
		error("Unknown STRUCT subcommand");
	}
}

// Parse a structure member definition line (called from PrepareProgramExt)
// Line format: membername[(dim1[,dim2,...])] AS type [LENGTH n]
// Returns: NULL if valid member parsed, error message string if error
const char *ParseStructMember(char *p, struct s_structdef *sd)
{
	char name[MAXVARLEN + 1];
	int namelen = 0;
	int type = T_NOTYPE;
	int size = 0;
	int offset;
	struct s_structmember *sm;
	short dims[MAXDIM] = {0};
	int ndims = 0;
	int array_elements = 1;

	if (sd->num_members >= MAX_STRUCT_MEMBERS)
		return "Too many members in TYPE";

	skipspace(p);

	// Parse member name
	if (!isnamestart(*p))
		return "Invalid member definition in TYPE"; // Not a valid member definition

	while (isnamechar(*p) && *p != '(' && namelen < MAXVARLEN)
	{
		//name[namelen++] = mytoupper(*p++);
		name[namelen++] = toupper(*p++);
	}
	name[namelen] = 0;

	skipspace(p);

	// Check for array dimensions
	if (*p == '(')
	{
		p++; // Skip opening parenthesis
		skipspace(p);

		while (*p && *p != ')' && ndims < MAXDIM)
		{
			// Parse dimension value manually (can't use getint during preprocess)
			int dim = 0;
			int have_digits = 0;
			while (*p >= '0' && *p <= '9')
			{
				dim = dim * 10 + (*p - '0');
				have_digits = 1;
				p++;
			}
			//if (dim < 1)
			//	dim = 1;
			if (!have_digits)   //Fix from Picomite 6.02.01RC8
				return "Dimensions";
			if (dim <= OptionBase)
				return "Dimensions";

			dims[ndims++] = dim;
			array_elements *= (dim + 1 - OptionBase); // Account for OPTION BASE

			skipspace(p);
			if (*p == ',')
			{
				p++;
				skipspace(p);
			}
		}

		if (*p == ')')
			p++; // Skip closing parenthesis
		skipspace(p);
	}

	skipspace(p);

	// Expect AS keyword (tokenized or literal)
	if (*p == tokenAS)
	{
		p++; // Skip past token
	}
	else if ((p[0] == 'A' || p[0] == 'a') && (p[1] == 'S' || p[1] == 's') && !isnamechar(p[2]))
	{
		p += 2; // Skip past literal AS
	}
	else
	{
		return "Invalid member definition in TYPE"; // Not a valid member definition (no AS keyword)
	}
	skipspace(p);

	// Parse type (checkstring handles both tokenized and literal forms)
	char *tp;
	if ((tp = checkstring(p, (char *)"INTEGER")) != NULL ||
		(tp = checkstring(p, (char *)"INT")) != NULL)
	{
		type = T_INT;
		size = sizeof(long long int);
		p = tp;
	}
	else if ((tp = checkstring(p, (char *)"FLOAT")) != NULL)
	{
		type = T_NBR;
		size = sizeof(MMFLOAT);
		p = tp;
	}
	else if ((tp = checkstring(p, (char *)"STRING")) != NULL)
	{
		type = T_STR;
		p = tp;
		skipspace(p);
		// Check for STRING LENGTH n (consistent with DIM syntax)
		if ((tp = checkstring(p, (char *)"LENGTH")) != NULL)
		{
			p = tp;
			skipspace(p);
			// Parse the size number manually (can't use getint during preprocess)
			size = 0;
			while (*p >= '0' && *p <= '9')
			{
				size = size * 10 + (*p - '0');
				p++;
			}
			if (size < 1)
				size = 1;
			if (size > MAXSTRLEN)
				size = MAXSTRLEN;
		}
		else
		{
			size = MAXSTRLEN; // Default string length
		}
	}
	else
	{
		// Check if it's a previously defined structure type
		char typename[MAXVARLEN + 1];
		int typenamelen = 0;
		char *tp2 = p;

		// Parse the type name
		while (isnamechar(*tp2) && typenamelen < MAXVARLEN)
		{
			typename[typenamelen++] = toupper(*tp2++);
		}
		typename[typenamelen] = 0;

		if (typenamelen > 0)
		{
			// Search for this type name in already-defined struct types
			int nested_idx = -1;
			for (int i = 0; i < g_structcnt; i++)
			{
				if (g_structtbl[i] != NULL && strcmp((char *)typename, (char *)g_structtbl[i]->name) == 0)
				{
					nested_idx = i;
					break;
				}
			}

			if (nested_idx >= 0)
			{
				// Found a nested structure type
				type = T_STRUCT;
				size = nested_idx; // Store struct type index in size field
				p = tp2;
			}
			else
			{
				return "Unknown type in TYPE definition";
			}
		}
		else
		{
			return "Unknown type in TYPE definition";
		}
	}

	// Calculate offset (align to natural boundary)
	offset = sd->total_size;
	// Align integers, floats, and nested structures to 8-byte boundary
	if ((type == T_INT || type == T_NBR || type == T_STRUCT) && (offset % 8) != 0)
	{
		offset = ((offset / 8) + 1) * 8;
	}

	// Add the member
	sm = &sd->members[sd->num_members];
	memcpy(sm->name, name, namelen + 1);
	sm->type = type;
	sm->size = size;
	sm->offset = offset;

	// Store array dimensions
	for (int i = 0; i < MAXDIM; i++)
	{
		sm->dims[i] = dims[i];
	}

	sd->num_members++;

	// Update total size (accounting for array elements)
	if (type == T_STR)
		sd->total_size = offset + (size + 1) * array_elements; // +1 for length byte per element
	else if (type == T_STRUCT)
	{
		// For nested structures, size field contains the struct type index
		int nested_size = g_structtbl[size]->total_size;
		sd->total_size = offset + nested_size * array_elements;
	}
	else
		sd->total_size = offset + size * array_elements;

	return NULL; // Successfully parsed a member (NULL means no error)
}

// Helper function to find a structure type by name (RP2350 only)

int FindStructType(char *name)
{
	int i, namelen = 0;
	char uname[MAXVARLEN + 1];

	// Convert to uppercase for comparison
	while (isnamechar(*name) && *name != '.' && namelen < MAXVARLEN)
	{
		uname[namelen++] = toupper(*name++);
	}
	uname[namelen] = 0;

	for (i = 0; i < g_structcnt; i++)
	{
		if (strcmp((char *)uname, (char *)g_structtbl[i]->name) == 0)
			return i;
	}
	return -1;
}

// Helper function to find a member within a structure definition (RP2350 only)
// Returns member index or -1 if not found
// Also sets *member_type, *member_offset, *member_size if found
// member_dims should point to array of MAXDIM shorts, will be filled with dimensions

int FindStructMember(int struct_idx, char *membername, int *member_type, int *member_offset, int *member_size, short *member_dims)
{
	int i, namelen = 0;
	unsigned char uname[MAXVARLEN + 1];
	struct s_structdef *sd;

	if (struct_idx < 0 || struct_idx >= g_structcnt)
		return -1;

	sd = g_structtbl[struct_idx];

	// Convert member name to uppercase (stop at '.', '(' or end of name)
	while (isnamechar(*membername) && *membername != '.' && *membername != '(' && namelen < MAXVARLEN)
	{
		uname[namelen++] = toupper(*membername++);
	}
	uname[namelen] = 0;

	// Search for member
	for (i = 0; i < sd->num_members; i++)
	{
		if (strcmp((char *)uname, (char *)sd->members[i].name) == 0)
		{
			if (member_type)
				*member_type = sd->members[i].type;
			if (member_offset)
				*member_offset = sd->members[i].offset;
			if (member_size)
				*member_size = sd->members[i].size;
			if (member_dims)
			{
				for (int j = 0; j < MAXDIM; j++)
				{
					member_dims[j] = sd->members[i].dims[j];
				}
			}
			return i;
		}
	}
	return -1;
}

// STRUCT(FIND array(), membername$, value [, start])
// General structure function with subfunctions
// FIND: Searches a struct array for an element where the specified member equals the value
//       Returns the index of the first match (starting from 'start'), or -1 if not found
//       Optional 'start' parameter allows iteration through multiple matches

void fun_struct(void)
{
	char *p;

	// Check for FIND subfunction: STRUCT(FIND array().member, value [, start] [, size])
	// argc==3: array().member, value (simple match)
	// argc==5: array().member, value, start (simple match with start index)
	// argc==7: array().member, value [, start], size (regex match, start optional)
	if ((p = checkstring(ep, (char *)"FIND")) != NULL)
	{
		unsigned char *member_base;
		int var_idx, struct_type, struct_size;
		int member_type, member_offset, member_size;
		int num_elements, i, start_idx;
		int use_regex = 0;
		void *size_var = NULL;
		int64_t *size_var_int = NULL;
		MMFLOAT *size_var_float = NULL;

		// Parse remaining arguments: array().member, value [, start] [, size]
		getcsargs(&p, 7); // array().member, value [, start] [, size] = up to 7 tokens
		if (argc != 3 && argc != 5 && argc != 7)
			error("Syntax: STRUCT(FIND array().member, value [, start] [, size])");

		// Determine if regex mode (argc==7 means regex with size variable)
		if (argc == 7)
			use_regex = 1;

		// Get the struct array member variable (argv[0])
		// This uses findvar which will populate g_StructMemberOffset and g_StructMemberSize
		member_base = findvar(argv[0], V_FIND | V_NOFIND_ERR | V_EMPTY_OK);
		var_idx = VarIndex;

		if (!(vartbl[var_idx].type & T_STRUCT))
			error("Expected a structure array");

		if (vartbl[var_idx].dims[0] == 0)
			error("Expected an array of structures");

		// Check that a member was specified (findvar sets g_StructMemberOffset)
		if (g_StructMemberOffset < 0)
			error("Must specify a member (e.g., array().membername)");

		struct_type = (int)vartbl[var_idx].size;
		struct_size = g_structtbl[struct_type]->total_size;
		member_offset = g_StructMemberOffset;
		member_size = g_StructMemberSize;

		// Get array base pointer (member_base points to member in element 0)
		unsigned char *array_base = member_base - member_offset;

		// Calculate number of elements
		num_elements = 1;
		for (int d = 0; d < MAXDIM && vartbl[var_idx].dims[d] != 0; d++)
		{
			num_elements *= (vartbl[var_idx].dims[d] + 1 - OptionBase);
		}

		// Determine member type from member_size and structure definition
		// member_size > 8 means string (includes length byte)
		if (member_size > 8)
		{
			member_type = T_STR;
		}
		else
		{
			// Check if the member is integer or float by looking at the structure definition
			struct s_structdef *sd = g_structtbl[struct_type];
			member_type = T_NBR; // default
			for (int m = 0; m < sd->num_members; m++)
			{
				if (sd->members[m].offset == member_offset)
				{
					member_type = sd->members[m].type & (T_INT | T_NBR | T_STR);
					break;
				}
			}
		}

		// Get search value and determine its type (argv[2])
		MMFLOAT f;
		long long int i64;
		char *s = NULL;
		int t = T_NOTYPE;
		evaluate(argv[2], &f, &i64, &s, &t, false);

		// Get optional start index (argv[4] if provided and non-empty)
		// For regex mode (argc==7): start is in argv[4] (optional), size var is in argv[6]
		// For simple mode (argc==5): start is in argv[4]
		start_idx = 0;
		if (argc >= 5 && *argv[4])
		{
			start_idx = getint(argv[4], OptionBase, OptionBase + num_elements) - OptionBase;
			// If start is past end of array, return -1 immediately
			if (start_idx >= num_elements)
			{
				targ = T_INT;
				iret = -1;
				return;
			}
		}

		// For regex mode, get the size variable (argv[6])
		if (use_regex)
		{
			if (member_type != T_STR)
				error("Regex search only works with string members");
			if (!(t & T_STR))
				error("Regex pattern must be a string");

			int size_vtype;
			size_var = findvar(argv[6], V_FIND);
			size_vtype = vartbl[VarIndex].type;
#ifdef STRUCTENABLED
			if (g_StructMemberType != 0)
				size_vtype = g_StructMemberType;
#endif
			if (!(size_vtype & (T_NBR | T_INT)))
				error("Size variable must be numeric");
			if (size_vtype & T_INT)
				size_var_int = size_var;
			else
				size_var_float = size_var;
		}

		// Search the array starting from start_idx
		targ = T_INT;
		for (i = start_idx; i < num_elements; i++)
		{
			unsigned char *elem_ptr = array_base + (i * struct_size);
			unsigned char *member_ptr = elem_ptr + member_offset;

			if (member_type == T_INT)
			{
				long long int val = *(long long int *)member_ptr;
				long long int search_val = 0;
				if (t & T_INT)
					search_val = i64;
				else if (t & T_NBR)
					search_val = (long long int)f;
				else
					error("Type mismatch: expected numeric value");

				if (val == search_val)
				{
					iret = i + OptionBase;
					return;
				}
			}
			else if (member_type == T_NBR)
			{
				MMFLOAT val = *(MMFLOAT *)member_ptr;
				MMFLOAT search_val = 0;
				if (t & T_NBR)
					search_val = f;
				else if (t & T_INT)
					search_val = (MMFLOAT)i64;
				else
					error("Type mismatch: expected numeric value");

				if (val == search_val)
				{
					iret = i + OptionBase;
					return;
				}
			}
			else if (member_type == T_STR)
			{
				unsigned char *val = member_ptr; // Points to length byte

				if (use_regex)
				{
					// Regex search mode
					int match_length;
					char *text_cstr = GetTempMemory(STRINGSIZE);
					char *pattern_cstr = GetTempMemory(STRINGSIZE);

					// Convert MMBasic string to C string
					memcpy(text_cstr, val + 1, *val);
					text_cstr[*val] = '\0';

					// Convert pattern (s is MMBasic string)
					memcpy(pattern_cstr, s + 1, *s);
					pattern_cstr[(unsigned char)*s] = '\0';

					int tmp = OptionEscape;
					OptionEscape = 0;
					int match_idx = re_match(pattern_cstr, text_cstr, &match_length);
					OptionEscape = tmp;

					if (match_idx != -1)
					{
						// Found a match - update size variable with match length
						if (size_var_float)
							*size_var_float = (MMFLOAT)match_length;
						else
							*size_var_int = (int64_t)match_length;
						iret = i + OptionBase;
						return;
					}
				}
				else
				{
					// Simple string match
					if (!(t & T_STR))
						error("Type mismatch: expected string value");

					// Compare strings (MMBasic string format: first byte is length)
					if (*val == *s && memcmp(val + 1, s + 1, *s) == 0)
					{
						iret = i + OptionBase;
						return;
					}
				}
			}
		}

		// Not found - set size to 0 for regex mode
		if (use_regex)
		{
			if (size_var_float)
				*size_var_float = 0.0;
			else
				*size_var_int = 0;
		}
		iret = -1;
		return;
	}

	// Check for OFFSET subfunction: STRUCT(OFFSET typename$, element$)
	if ((p = checkstring(ep, (char *)"OFFSET")) != NULL)
	{
		char *typename_str, *element_str;
		int i, member_type, member_offset, member_size;

		// Parse remaining arguments: typename$, element$
		getcsargs(&p, 3); // typename$, element$ = 3 tokens
		if (argc != 3)
			error("Syntax: STRUCT(OFFSET typename$, element$)");

		// Get the structure type name (argv[0])
		typename_str = getstring(argv[0]);

		// Get the element/member name (argv[2])
		element_str = getstring(argv[2]);

		// Search for the structure type by name
		for (i = 0; i < g_structcnt; i++)
		{
			if (g_structtbl[i] != NULL &&
				strlen((char *)g_structtbl[i]->name) == *typename_str &&
				strncasecmp((char *)g_structtbl[i]->name, (char *)(typename_str + 1), *typename_str) == 0)
			{
				// Found the structure type, now find the member
				int member_idx = FindStructMember(i, element_str + 1, &member_type, &member_offset, &member_size, NULL);
				if (member_idx < 0)
					error("Member not found in structure");
				targ = T_INT;
				iret = member_offset;
				return;
			}
		}

		error("Structure type not found");
	}

	// Check for SIZEOF subfunction: STRUCT(SIZEOF typename$)
	if ((p = checkstring(ep, (char *)"SIZEOF")) != NULL)
	{
		char *typename_str;
		int i;

		// Parse remaining argument: typename$
		getcsargs(&p, 1); // typename$ = 1 token
		if (argc != 1)
			error("Syntax: STRUCT(SIZEOF typename$)");

		// Get the structure type name (argv[0])
		typename_str = getstring(argv[0]);

		// Search for the structure type by name
		for (i = 0; i < g_structcnt; i++)
		{
			if (g_structtbl[i] != NULL &&
				strlen((char *)g_structtbl[i]->name) == *typename_str &&
				strncasecmp((char *)g_structtbl[i]->name, (char *)(typename_str + 1), *typename_str) == 0)
			{
				targ = T_INT;
				iret = g_structtbl[i]->total_size;
				return;
			}
		}

		error("Structure type not found");
	}

	// Check for TYPE subfunction: STRUCT(TYPE typename$, element$)
	// Returns T_INT, T_NBR or T_STR for the base type of the element
	if ((p = checkstring(ep, (char *)"TYPE")) != NULL)
	{
		char *typename_str, *element_str;
		int i, member_type, member_offset, member_size;

		// Parse remaining arguments: typename$, element$
		getcsargs(&p, 3); // typename$, element$ = 3 tokens
		if (argc != 3)
			error("Syntax: STRUCT(TYPE typename$, element$)");

		// Get the structure type name (argv[0])
		typename_str = getstring(argv[0]);

		// Get the element/member name (argv[2])
		element_str = getstring(argv[2]);

		// Search for the structure type by name
		for (i = 0; i < g_structcnt; i++)
		{
			if (g_structtbl[i] != NULL &&
				strlen((char *)g_structtbl[i]->name) == *typename_str &&
				strncasecmp((char *)g_structtbl[i]->name, (char *)(typename_str + 1), *typename_str) == 0)
			{
				// Found the structure type, now find the member
				int member_idx = FindStructMember(i, element_str + 1, &member_type, &member_offset, &member_size, NULL);
				if (member_idx < 0)
					error("Member not found in structure");
				targ = T_INT;
				// Return only the base type (T_INT, T_NBR or T_STR)
				iret = member_type & (T_INT | T_NBR | T_STR);
				return;
			}
		}

		error("Structure type not found");
	}

	error("Unknown STRUCT subfunction");
}
#endif // structure


/***********************************************************************************************
 * FUNCTION: erase(char *p) - Erase named global variables
 *
 * Bug fixes from original:
 * 1. Use isnameend() instead of isnamechar() to preserve type suffixes
 * 2. Add memory bounds checking before freeing (heap and PSRAM)
 * 3. Add PSRAM support for RP2350
 * 4. Cache strlen() result to avoid redundant calls
 *
 * Changes for differential split:
 * - Search range changed from MAXVARS/2..MAXVARS to maxlocalvars..MAXVARS
 * - Wrap calculation changed from MAXVARS/2 to maxlocalvars
 ***********************************************************************************************/
/* Picomite
uint32_t erase(char *p, bool nofree)
{
    int j, k, len;
    char *s, *x;
    uint32_t addr = 0;
    len = strlen(p);
    while (len > 0 && !isnamechar(p[strlen(p) - 1])) // CHANGED: was !isnamechar(p[strlen(p)-1])
    {
        p[--len] = 0; // CHANGED: decrement len and use it
    }

    makeupper((unsigned char *)p);
    for (j = maxlocalvars; j < MAXVARS; j++) // CHANGED: was MAXVARS/2
    {
        s = p;
        x = (char *)g_vartbl[j].name;
        len = strlen(p);
        while (len > 0 && *s == *x)
        { // compare the variable to the name that we have
            len--;
            s++;
            x++;
        }
        if (!(len == 0 && (*x == 0 || strlen(p) == MAXVARLEN)))
            continue;
        // found the variable

        // BUG FIX: Add bounds checking before freeing memory
        if (((g_vartbl[j].type & T_STR) || g_vartbl[j].dims[0] != 0) && !(g_vartbl[j].type & T_PTR))
        {
            addr = (uint32_t)g_vartbl[j].val.s; // ADDED: get address once

            if (!nofree)
            {
                // Check if in heap
                if (addr > (uint32_t)MMHeap && addr < (uint32_t)MMHeap + heap_memory_size)
                {
                    FreeMemorySafe((void **)&g_vartbl[j].val.s);
                }
            }
            g_vartbl[j].val.s = NULL;
        }

        k = j + 1;
        if (k == MAXVARS)
            k = maxlocalvars; // CHANGED: was MAXVARS/2
        if (g_vartbl[k].type)
        {
            g_vartbl[j].name[0] = '~';
            g_vartbl[j].type = T_BLOCKED;
        }
        else
        {
            g_vartbl[j].name[0] = 0;
            g_vartbl[j].type = T_NOTYPE;
        }
        g_vartbl[j].dims[0] = 0;
        g_vartbl[j].level = 0;
        g_Globalvarcnt--;
        break;
    }
    if (j == MAXVARS)
        error("Cannot find $", p);
    return addr;
}
*/

/***********************************************************************************************
 * FUNCTION: erase(char *p) - Erase named global variables
 *
***********************************************************************************************/
void erase(char *p,char nofree) {
	//int i,j,k, len;
	int j,k, len;
	//char p[MAXVARLEN + 1], *s, *x;
	char  *s, *x;

	//getargs(&erase, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
	//if((argc & 0x01) == 0) error("Argument count");

	//for(i = 0; i < argc; i += 2) {
	//	strcpy((char *)p, argv[i]);
        while(!isnamechar(p[strlen(p) - 1])) p[strlen(p) - 1] = 0;

		makeupper(p);                                               // all variables are stored as uppercase
		for(j = MAXVARS/2; j < MAXVARS; j++) {
            s = p;  x = vartbl[j].name; len = strlen(p);
            while(len > 0 && *s == *x) {                            // compare the variable to the name that we have
                len--; s++; x++;
            }
            if(!(len == 0 && (*x == 0 || strlen(p) == MAXVARLEN))) continue;
    		// found the variable
#ifdef STRUCTENABLED
            if (((vartbl[j].type & T_STR) || vartbl[j].dims[0] != 0 || (vartbl[j].type & T_STRUCT)) && !(vartbl[j].type & T_PTR))
#else
			if(((vartbl[j].type & T_STR) || vartbl[j].dims[0] != 0) && !(vartbl[j].type & T_PTR))
#endif
			{
				if(!nofree)FreeMemory(vartbl[j].val.s);                        // free any memory (if allocated)
				vartbl[j].val.s=NULL;
			}
			k=j+1;
			if(k==MAXVARS)k=MAXVARS/2;
			if(vartbl[k].type){
				vartbl[j].name[0]='~';
				vartbl[j].type=T_BLOCKED;
			} else {
				vartbl[j].name[0]=0;
				vartbl[j].type=T_NOTYPE;
			}
			vartbl[j].dims[0] = 0;                                    // and again
			vartbl[j].level = 0;
			Globalvarcnt--;
			break;
		}
		if(j == MAXVARS) error("Cannot find $", p);
	//}
}

void cmd_erase(void) {
	int i;
	char p[MAXVARLEN + 1];
	getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");				// getargs macro must be the first executable stmt in a block
	if((argc & 0x01) == 0) error("Argument count");
	for(i = 0; i < argc; i += 2) {
	  strcpy((char *)p, argv[i]);
	  erase(p,0);
	}
}

void cmd_clear(void) {
	//char *p;
	//if((p=checkstring(cmdline,"VARS"))){
	//	cmd_erasevar(p);
	//	return;
	//}
	checkend(cmdline);
	if(LocalIndex)error("Invalid in a subroutine");
	ClearVars(0);
}


/***********************************************************************************************
utility functions used by the various commands
************************************************************************************************/


// utility function used by llist() below
// it copys a command or function honouring the case selected by the user
void strCopyWithCase(char *d, char *s) {
	if(Option.Listcase == CONFIG_LOWER) {
		while(*s) *d++ = tolower(*s++);
	} else if(Option.Listcase == CONFIG_UPPER) {
		while(*s) *d++ = toupper(*s++);
	} else {
		while(*s) *d++ = *s++;
	}
	*d = 0;
}

void replaceAlpha(char *str, const char *replacements[MMEND]){
    char buffer[STRINGSIZE]; // Buffer to store the modified string
    int bufferIndex = 0;
    int len = strlen(str);
    int i = 0;

    while (i < len) {
        // Check for the pattern "~(X)" where X is an uppercase letter
        if (i<len-3 && str[i] == '~' && str[i + 1] == '(' && isupper((int)str[i + 2]) && str[i + 3] == ')') {
            char alpha = str[i + 2]; // Extract the letter 'alpha'
            const char *replacement = replacements[alpha - 'A']; // Get the replacement string

            // Copy the replacement string into the buffer
            strcpy(&buffer[bufferIndex], replacement);
            bufferIndex += strlen(replacement);

            i += 4; // Move past "~(X)"
        } else {
            // Copy the current character to the buffer
            buffer[bufferIndex++] = str[i++];
        }
    }

    buffer[bufferIndex] = '\0'; // Null-terminate the buffer
    strcpy(str,  buffer); // Copy the buffer back into the original string
}

// list a line into a buffer (b) given a pointer to the beginning of the line (p).
// the returned string is a C style string (terminated with a zero)
// this is used by cmd_list(), cmd_edit() and cmd_xmodem()
char MIPS16 *llist(char *b, char *p) {
    int i, firstnonwhite = true;
    char *b_start = b;

    while(1) {
        if(*p == T_NEWLINE) {
            p++;
            firstnonwhite = true;
            continue;
        }

        if(*p == T_LINENBR) {
            i = (((p[1]) << 8) | (p[2]));                           // get the line number
            p += 3;                                                 // and step over the number
            IntToStr(b, i, 10);
            b += strlen(b);
            if(*p != ' ') *b++ = ' ';
            }

        if(*p == T_LABEL) {                                         // got a label
            for(i = p[1], p += 2; i > 0; i--)
                *b++ = *p++;                                        // copy to the buffer
            *b++ = ':';                                             // terminate with a colon
            if(*p && *p != ' ') *b++ = ' ';                         // and a space if necessary
            firstnonwhite = true;
            }                                                       // this deliberately drops through in case the label is the only thing on the line

        if(*p >= C_BASETOKEN) {
            if(firstnonwhite) {

                //if(*p == GetCommandValue("Let"))
                CommandToken tkn = commandtbl_decode(p);
             	if (tkn == GetCommandValue("Let"))
                    *b = 0;                                         // use nothing if it LET
                else {
                    strCopyWithCase(b, commandname(tkn));            // expand the command (if it is not LET)
                    b += strlen(b);                                 // update pointer to the end of the buffer
                    if(IsAlpha(*(b - 1))) *b++ = ' ';               // add a space to the end of the command name
                }
                firstnonwhite = false;
                p += sizeof(CommandToken);                          // CMD16BIT
            } else {                                                // not a command so must be a token
                strCopyWithCase(b, tokenname(*p));                  // expand the token
                b += strlen(b);                                     // update pointer to the end of the buffer
                if(*p == tokenTHEN || *p == tokenELSE)
                    firstnonwhite = true;
                else
                    firstnonwhite = false;
                p++;                                                // CMD16BIT
            }
            // p++;                                                  // CMD16BIT
            continue;
        }

        // hey, an ordinary char, just copy it to the output
        if(*p) {
            *b = *p;                                                // place the char in the buffer
            if(*p != ' ') firstnonwhite = false;
            p++;  b++;                                              // move the pointers
            continue;
        }

        // at this point the char must be a zero
        // zero char can mean both a separator or end of line
        if(!(p[1] == T_NEWLINE || p[1] == 0)) {
            *b++ = ':';                                             // just a separator
            firstnonwhite = true;
            p++;
            continue;
        }

        // must be the end of a line - so return to the caller
        while(*(b-1) == ' ' && b > b_start) --b;                    // eat any spaces on the end of the line
        *b = 0;                                                     // terminate the output buffer
		replaceAlpha((char *)b_start, overlaid_functions) ;         //replace the user version of all the MM. functions
		/*
		// Replacement of future function compression(needed for H7 )
		STR_REPLACE((char *)b_start, "BASE$(2,", "BIN$(", 3);
		STR_REPLACE((char *)b_start, "BASE$(8,", "OCT$(", 3);
		STR_REPLACE((char *)b_start, "BASE$(16,", "HEX$(", 3);
		STR_REPLACE((char *)b_start, "SCHANGE$(L,", "LCASE$(", 3);
		STR_REPLACE((char *)b_start, "SCHANGE$(U,", "UCASE$(", 3);
		STR_REPLACE((char *)b_start, "TOPBOTTOM(I,", "MIN(", 3);
		STR_REPLACE((char *)b_start, "TOPBOTTOM(A,", "MAX(", 3);
		STR_REPLACE((char *)b_start, "SCHANGE$(E,", "LEFT$(", 3);
		STR_REPLACE((char *)b_start, "SCHANGE$(R,", "RIGHT$(", 3);
		*/
        return ++p;
    } // end while
}


#ifndef CMD16BIT
void execute_one_command(char *p) {
    int cmd, i;

    CheckAbort();
    targ = T_CMD;
    skipspace(p);                                                   // skip any whitespace
    if(*p >= C_BASETOKEN && *p - C_BASETOKEN < CommandTableSize - 1 && (commandtbl[*p - C_BASETOKEN].type & T_CMD)) {
        cmd = *p  - C_BASETOKEN;
        if(*p == cmdWHILE || *p == cmdDO || *p == cmdFOR) error("Invalid inside THEN ... ELSE") ;
        cmdtoken = *p;
        cmdline = p + 1;
        skipspace(cmdline);
        commandtbl[cmd].fptr();                                     // execute the command
    } else {
        if(!isnamestart(*p)) error("Invalid character");
        i = FindSubFun(p, false);                                   // it could be a defined command
        if(i >= 0)                                                  // >= 0 means it is a user defined command
            DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
        else
            error("Unknown command");
    }
    ClearTempMemory();                                              // at the end of each command we need to clear any temporary string vars
}

#else
void execute_one_command(char *p) {
    int i;

   CheckAbort();
   targ = T_CMD;
   skipspace(p);                                                   // skip any whitespace
    // if(*p >= C_BASETOKEN && *p - C_BASETOKEN < CommandTableSize - 1 && (commandtbl[*p - C_BASETOKEN].type & T_CMD)) {
   if (p[0] >= C_BASETOKEN && p[1] >= C_BASETOKEN){
	   CommandToken cmd = commandtbl_decode(p);
	   if (cmd == cmdWHILE || cmd == cmdDO || cmd == cmdFOR)
	   		error("Invalid inside THEN ... ELSE");
	   cmdtoken = cmd;
	   cmdline = p + sizeof(CommandToken);
	   skipspace(cmdline);
	   commandtbl[cmd].fptr(); // execute the command

   } else {
    if(!isnamestart(*p)) error("Invalid character");
      i = FindSubFun(p, false);                                   // it could be a defined command
      if(i >= 0)                                                  // >= 0 means it is a user defined command
          DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
      else
          error("Unknown command");
   }
   ClearTempMemory();                                              // at the end of each command we need to clear any temporary string vars
}

#endif
