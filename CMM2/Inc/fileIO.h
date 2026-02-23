/***********************************************************************************************************************
MMBasic

FileIO.h

Supporting header file for FileIO.c which does all the SD Card related file I/O in MMBasic.

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

//** SD CARD INCLUDES ***********************************************************
#include "ff.h"
#include "diskio.h"




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
#define MAXFILES 2048
typedef struct ss_flist {
    char fn[FF_MAX_LFN];
    int fs; //file size
    int fd; //file date
    int ft; //file time
} s_flist;

#endif
