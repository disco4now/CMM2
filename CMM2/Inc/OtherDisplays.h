/***********************************************************************************************************************
MMBasic

Serial.h

Include file that contains the globals and defines for serial.c in MMBasic.

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
