/***************************************************************************
CMM2 MMBasic
OtherDisplays.h


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

/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
void fun_sprite(void);
#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
#endif
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef OTHERDISPLAYS_HEADER
#define OTHERDISPLAYS_HEADER
extern unsigned short colcount[256];
extern void ConfigDisplayOther(char *p);
extern void InitDisplayOther(void);
extern void ReadBufferFast32(int x1, int y1, int x2, int y2, char* p);
extern void DrawBufferFast32(int x1, int y1, int x2, int y2, char* p);
extern void ReadBufferFast16(int x1, int y1, int x2, int y2, char* p);
extern void DrawBufferFast16(int x1, int y1, int x2, int y2, char* p);
extern void ReadBufferFast8(int x1, int y1, int x2, int y2, char* p);
extern void DrawBufferFast8(int x1, int y1, int x2, int y2, char* p);
extern void DrawBitmap32(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
extern void DrawBuffer32(int x1, int y1, int x2, int y2, char* p, int skip);
extern void ReadBuffer32(int x1, int y1, int x2, int y2, char* p);
extern void DrawRectangle32(int x1, int y1, int x2, int y2, int c);
extern void DrawBitmap16(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
extern void DrawBuffer16(int x1, int y1, int x2, int y2, char* p, int skip);
extern void ReadBuffer16(int x1, int y1, int x2, int y2, char* p);
extern void DrawRectangle16(int x1, int y1, int x2, int y2, int c);
extern void DrawBitmap8(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
extern void DrawBuffer8(int x1, int y1, int x2, int y2, char* p, int skip);
extern void ReadBuffer8(int x1, int y1, int x2, int y2, char* p);
extern void DrawRectangle8(int x1, int y1, int x2, int y2, int c);
extern void DrawPixel8(int x1, int y1, int c);
extern void DrawPixel16(int x1, int y1, int c);
extern void DrawPixel32(int x1, int y1, int c);
extern void ScrollBuff32V(int lines, int blank);
extern void ScrollBuff16V(int lines, int blank);
extern void ScrollBuff8V(int lines, int blank);
extern void DrawPixelExternal(int x, int y, int c);
extern void ScrollBuff32H(int lines);
extern void ScrollBuff16H(int lines);
extern void ScrollBuff8H(int lines);
extern void MoveBufferExpand(int x1, int y1, int x2, int y2, int w, int h, int flip);
extern void MoveBufferWorld(int x1, int y1, int x2, int y2, int w, int h, int flip);
extern void MoveBufferContract(int x1, int y1, int x2, int y2, int w, int h, int flip);
extern void Stitch(uint32_t fadd1, uint32_t fadd2, uint32_t tadd, int offset);
extern void PageAnd(uint32_t fadd1, uint32_t fadd2, uint32_t taddt);
extern void PageOr(uint32_t fadd1, uint32_t fadd2, uint32_t taddt);
extern void PageXor(uint32_t fadd1, uint32_t fadd2, uint32_t taddt);
extern void copytofloat(uint8_t ***output, int xs, int ys, int w, int h);
extern void copyfromfloat(uint8_t ***input, int xs, int ys, int w, int h, int dontcopyblack);
extern void MoveBufferDup(int x1, int y1, int x2, int y2, int w, int h, int flip);
extern void MoveBufferNormal(int x1, int y1, int x2, int y2, int w, int h, int flip);
extern void DrawHLineFast(int x1, int y, int x2, int c);
extern char *BIT_4 ;
extern char *BIT_5 ;
extern char *BIT_6 ;

enum {
  TRANSFER_WAIT,
  TRANSFER_COMPLETE,
  TRANSFER_ERROR
};

#endif
#endif
