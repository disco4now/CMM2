/***************************************************************************
CMM2 MMBasic
sprites.h

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
