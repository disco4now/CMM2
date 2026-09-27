/***************************************************************************
CMM2 MMBasic
Configuration.h
Include file that contains the configuration details for MMBasic.

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
#ifndef CONFIG
#define CONFIG
#include <math.h>
// The main clock frequency for the chip at bootup, it can be changed by the CPU command
#define	CLOCKFREQ		(400000000L)			    // This is set in in Configuration Bits.h
#define CONSOLE_BAUDRATE    115200
#define MMFLOAT double
#define TFLOAT float
#define FLOAT3D float
#define sqrt3d sqrtf
#define round3d roundf
#define fabs3d fabsf
#define tcos cosf
#define tsin sinf
#define tfloor floorf
#define tceil ceilf
#define tround roundf
#define tlog10 log10f
#define STR_SIG_DIGITS 9                            // number of significant digits to use when converting MMFLOAT to a string

#define nop	__asm__ ("NOP")

#define forever 1
#define true	1
#define false	0

#define MIPS16
//#define dp(...) {char s[140];sprintf(s,  __VA_ARGS__); MMPrintString(s); MMPrintString("\r\n");}
#define RoundUptoInt(a)     (((a) + (32 - 1)) & (~(32 - 1)))// round up to the nearest whole integer


#define BOOL_ALREADY_DEFINED

/* ============================================================================
 * Operating characteristics - Limits and maximums
 * ============================================================================ */

#define MAXVARLEN           32                      // maximum length of a variable name
#define MAXSTRLEN           255                     // maximum length of a string
#define STRINGSIZE          256                     // must be 1 more than MAXSTRLEN.  3 of these buffers are staticaly created
#define MAXERRMSG           64                      // max error msg size (MM.ErrMsg$ is truncated to this)
#define MAXDIM              5                       // maximum nbr of dimensions to an array
#define CLIPBOARDSIZE		8192					// maximum size of clipboard for edit
#define MAXFORLOOPS         100                     // maximum nbr of nested for-next loops, each entry uses 17 bytes
#define MAXDOLOOPS          100                     // maximum nbr of nested do-loops, each entry uses 12 bytes
#define MAXGOSUB            100                      // maximum nbr of nested gosubs and defined subs/functs, each entry uses 8 bytes
#define MAX_MULTILINE_IF    20                      // maximum nbr of nested multiline IFs, each entry uses 8 bytes
#define MAXTEMPSTRINGS      1024                    // maximum nbr of temporary strings allowed, each entry takes up 4 bytes
#define NBRSETTICKS         4                       // the number of SETTICK interrupts available
#define MAXBLITBUF          65                      // the maximum number of BLIT buffers
#define MAXCOLLISIONS		8						// maximum number of collisions tested
#define MAXLAYER            10                      // maximum number of sprite layers
#define BREAK_KEY           3                       // the default value (CTRL-C) for the break key.  Reset at the command prompt.
#define MAX_MODES           18						// maximum number of video modes
#define MAXKEYLEN           64 //24					// Maximum length of programmable function keys
#define NBRPROGKEYS         12                      // number of programmable function keys
#define MAXSOUNDS         	4                       // number of simultaneous sounds
#define DEFCOLOUR			8						// Default colour depth in bits
#define MAX3D				128						// Maximum number of 3D objects
#define MAXCAM				6						// Maximum number of cameras
#define MAXDEFINES			256						// Maximum number of #define in file
#define MAXVARS				1024
#define MAXHASH				MAXVARS/2-1
#define MAXSUBFUN           MAXVARS/2               // maximum nbr of defined subroutines or functions in a program. each entry takes up 4 bytes
// define the maximum number of arguments to PRINT, INPUT, WRITE, ON, DIM, ERASE, DATA and READ
// each entry uses zero bytes.  The number is limited by the length of a command line
#define MAX_ARG_COUNT       32
#define MAXCFUNCTION	20
// size of the console terminal emulator's screen
#define SCREENWIDTH     80
#define SCREENHEIGHT    24                          // this is the default and it can be changed using the OPTION command
#define NO_KEYBOARD             0
#define CONFIG_UK		1
#define CONFIG_US		2
#define CONFIG_DE		3
#define CONFIG_FR		4
#define CONFIG_ES		5
#define CONFIG_BE		6
#define CONFIG_IT		7
#define MAXKEYBOARDS    6
#define FNV_prime 16777619
#define FNV_offset_basis 2166136261
#define MAXRESTORE          50
#define MAX_POLYGON_VERTICES 128

/* ============================================================================
 * Operating characteristics - Structures
 * Enable structures on platforms with sufficient memory (currently RP2350)
 * ============================================================================ */
#ifdef STRUCTENABLED
#define MAX_STRUCT_TYPES 32     // Maximum number of structure type definitions
#define MAX_STRUCT_MEMBERS 16   // Maximum members per structure
#define MAX_STRUCT_NEST_DEPTH 8 // Maximum nesting depth for nested structures
#endif

/* ============================================================================
 * Operating characteristics - Files and I/O
 * ============================================================================ */


/* ============================================================================
 * Operating characteristics - Display configuration
 * ============================================================================ */
//#define CONFIG_TITLE 0
//#define CONFIG_LOWER 1
//#define CONFIG_UPPER 2

#define silly_low 2000
#define silly_high -1


#endif

