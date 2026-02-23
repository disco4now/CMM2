/***********************************************************************************************************************
MMBasic

sprites.h

Include file that contains the globals and defines for serial.c in MMBasic.

Copyright 2018 Peter Mather.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef SPRITE_HEADER
#define SPRITE_HEADER
struct blitbuffer{
    char * blitbuffptr; //points to the sprite image, set to NULL if not in use
    char * blitstoreptr; //points to the stored background, set to NULL if not in use
    char  collisions[MAXCOLLISIONS+1]; //set to NULL if not in use, otherwise contains current collisions
    int64_t master; //bitmask of which sprites are copies
    uint64_t lastcollisions;
    short x; //set to 1000 if not in use
    short y;
    short next_x; //set to 1000 if not in use
    short next_y;
    short w; 
    short h;
    int bc; //background colour;
    signed char layer; //defaults to 1 if not specified. If zero then scrolls with background
    signed char mymaster; //number of master if this is a copy
    char rotation;
    char active;
    char edges;
};
extern struct blitbuffer blitbuff[MAXBLITBUF];	
extern int layer_in_use[MAXLAYER+1];
extern int sprites_in_use;
extern int LIFOpointer;
extern int zeroLIFOpointer;
extern char *COLLISIONInterrupt;
extern int CollisionFound;
extern void closeallsprites(void);
extern void loadsprite(char *p);
extern void BlitShowBuff8(int bnbr, int x1, int y1, int mode);
extern void BlitShowBuff16(int bnbr, int x1, int y1, int mode);
extern void BlitShowBuff32(int bnbr, int x1, int y1, int mode);
#endif
#endif
