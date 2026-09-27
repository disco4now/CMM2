/***************************************************************************
CMM2 MMBasic
FileIO.h

Supporting header file for FileIO.c which does all the SD Card related file I/O in MMBasic.

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

//** SD CARD INCLUDES ***********************************************************
#include "ff.h"
#include "diskio.h"
#include <stdbool.h>




#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
    extern void GetSdFileDescriptor(int fnbr);
    extern void FileOpen(char *fname, char *fmode, char *ffnbr);
    extern int BasicFileOpen(char *fname, int fnbr, int mode);
    extern void FileClose(int fnbr);
    extern void ForceFileClose(int fnbr);

    extern int FileGetData(int fnbr, void *buff, int count, unsigned int *read);
    extern void FilePutStr(int count, char *c, int fnbr);
    extern int  FileEOF(int fnbr);
    extern char *ChangeToDir(char *p);
    extern int InitSDCard(void);
    extern void ErrorCheck(int fnbr);
    extern void ErrorThrow(int e);
    extern void CheckSDCard(void);
    extern int FileLoadProgram(char *fname,int mode);
    extern int FindFreeFileNbr(void);

    /* ============================================================================
    * Function declarations - Character and string I/O
    * ============================================================================ */
    extern char FileGetChar(int fnbr);
    extern char FilePutChar(char c, int fnbr);
    void FilePutData(char *c, int fnbr, int n);

    extern const int ErrorMap[21];
    extern int SDCardPresent;
    extern volatile int tickspersample;
    extern void checkWAVinput(void);
    extern char *WAVInterrupt;
    extern int WAVcomplete;
    extern int WAV_fnbr;
    extern FRESULT FSerror;
    extern int BMPfnbr;

    #define WAV_BUFFER_SIZE 8192
	#define EFFECT_BUFFER_SIZE 4096
    extern int OptionFileErrorAbort;
    extern const char *FErrorMsg[];
    extern char *GetCWD(void);
    extern int strcicmp(char const *a, char const *b);
    extern gd_GIF *gif;
    extern unsigned char *frame;
    extern int giffnbr;
    extern char filepath[FF_MAX_LFN];
    extern char fullpathname[FF_MAX_LFN];
    extern void InitFileIO(void);
    extern void fullpath(char *q);
    extern void getfullfilepath(char *p, char *q);
    extern int LineCount;
    extern int ExistsFile(char *p);



#define MAXFILES 2048
typedef struct ss_flist {
    char fn[FF_MAX_LFN];
    int fs; //file size
    int fd; //file date
    int ft; //file time
} s_flist;

typedef struct
{
        int width;
        int height;
        int bitsPerPixel;
        int linesProcessed;
        bool success;
        char errorMsg[256];
} BMP_Result;

extern bool (*linecallback)(int *imagewidth, int *imageheight, uint32_t *linedata, int *linenumber);
extern BMP_Result decodeBMP(bool topdown);
void decodeBMPheader(int *width, int *height);

#endif
