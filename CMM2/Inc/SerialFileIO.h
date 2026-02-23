/***********************************************************************************************************************
MMBasic

SerialFileIO.h

Include file that contains the functions for handling serial file I/O in MMBasic.

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

#include "ff.h"


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef FILES_HEADER
#define FILES_HEADER

extern void MMgetline(int filenbr, char *p);
//extern void MMfputs(char *p, int filenbr);
//extern void MMfopen(char *fname, char *mode, int fnbr);
//extern void MMfclose(int fnbr);
extern int MMfgetc(int fnbr);
extern char MMfputc(char c, int fnbr);
extern void CloseAllFiles(void);
extern int MMfeof(int fnbr);

#define MAXCOMPORTS 3

#define MAXOPENFILES  10
union uFileTable {
    unsigned int com;
    FIL *fptr;
};

extern union uFileTable FileTable[MAXOPENFILES + 1];

#endif
#endif


