/***********************************************************************************************************************
MMBasic

Onewire.c

Handles all the commands and functions related to One Wire support.

The one wire support is based on code from Dallas Semiconductor Corporation:
 Copyright (C) 1999-2006 Dallas Semiconductor Corporation,
 All Rights Reserved.
 
 Permission is hereby granted, free of charge,
 to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to
 deal in the Software without restriction, including
 without limitation the rights to use, copy, modify,
 merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom
 the Software is furnished to do so, subject to the
 following conditions:

 The above copyright notice and this permission notice
 shall be included in all copies or substantial portions
 of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
 ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.

Modifications: 
   Copyright 2012 Gerard Sexton
   Copyright 2012-2020 Geoff Graham
   Copyright 2019-2020 Peter Mather


************************************************************************************************************************/

// These two together take up about 4K of flash and no one seems to use them !!
//#define INCLUDE_CRC
//#define INCLUDE_1WIRE_SEARCH

/**********************************************************************************
 All other required definitions and global variables should be define here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
#ifndef ONEWIRE_HEADER
#define ONEWIRE_HEADER
extern int mmOWvalue;
extern long long int *ds18b20Timers;
//#define INCLUDE_1WIRE_SEARCH

#endif
#endif
