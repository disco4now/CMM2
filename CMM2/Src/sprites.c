/***************************************************************************

CMM2 MMBasic
sprites.c

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
int layer_in_use[MAXLAYER+1];
unsigned char LIFO[MAXBLITBUF];
unsigned char zeroLIFO[MAXBLITBUF];
#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a < b) ? b : a)
int LIFOpointer=0;
int zeroLIFOpointer=0;
int sprites_in_use=0;
char *COLLISIONInterrupt=NULL;
int CollisionFound=false;
int sprite_which_collided = -1;
static int hideall=0;
struct blitbuffer blitbuff[MAXBLITBUF];											//Buffer pointers for the BLIT command
const int colours[16]={0x00,0xFF,0x4000,0x40ff,0x8000,0x80ff,0xff00,0xffff,0xff0000,0xff00FF,0xff4000,0xff40ff,0xff8000,0xff80ff,0xffff00,0xffffff};
void LIFOadd(int n){
    int i, j=0;
    for(i=0; i<LIFOpointer; i++){
        if(LIFO[i]!=n){
            LIFO[j]=LIFO[i];
            j++;
        }
    }
    LIFO[j]=n;
    LIFOpointer=j+1;
}
void LIFOremove(int n){
    int i, j=0;
    for(i=0; i<LIFOpointer; i++){
        if(LIFO[i]!=n){
            LIFO[j]=LIFO[i];
            j++;
        }
    }
    LIFOpointer=j;
}
void LIFOswap(int n, int m){
    int i;
    for(i=0; i<LIFOpointer; i++){
        if(LIFO[i]==n)LIFO[i]=m;
    }
}
void zeroLIFOadd(int n){
    int i, j=0;
    for(i=0; i<zeroLIFOpointer; i++){
        if(zeroLIFO[i]!=n){
            zeroLIFO[j]=zeroLIFO[i];
            j++;
        }
    }
    zeroLIFO[j]=n;
    zeroLIFOpointer=j+1;
}
void zeroLIFOremove(int n){
    int i, j=0;
    for(i=0; i<zeroLIFOpointer; i++){
        if(zeroLIFO[i]!=n){
            zeroLIFO[j]=zeroLIFO[i];
            j++;
        }
    }
    zeroLIFOpointer=j;
}
void zeroLIFOswap(int n, int m){
    int i;
    for(i=0; i<zeroLIFOpointer; i++){
        if(zeroLIFO[i]==n)zeroLIFO[i]=m;
    }
}
uint8_t convert_8bit(uint32_t c){
    return ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
}
uint16_t convert_12bit(uint32_t c){
	uint32_t red,green,blue;
	red=BIT_4[((c & 0xFF0000)>>20)]<<8;
    green=BIT_4[((c & 0xFF00)>>12)]<<4;
    blue=BIT_4[((c & 0xFF)>>4)];
    return red|green|blue;
}

uint16_t convert_16bit(uint32_t c){
	uint32_t red,green,blue;
    red=BIT_5[((c & 0xFF0000)>>19)]<<11;
    green=BIT_6[((c & 0xFF00)>>10)]<<5;
    blue=BIT_5[((c & 0xFF)>>3)];
    return red|green|blue;
}


void closeallsprites(void){
    int i;
    for(i = 0; i < MAXBLITBUF; i++) {
        if(i<=MAXLAYER)layer_in_use[i]=0;
        if(i){
        	if(blitbuff[i].mymaster==-1)FreeMemorySafe((void *)&blitbuff[i].blitbuffptr);
        	FreeMemorySafe((void *)&blitbuff[i].blitstoreptr);
        }
        blitbuff[i].blitbuffptr = NULL;
        blitbuff[i].blitstoreptr = NULL;
        blitbuff[i].master=-1;
        blitbuff[i].mymaster=-1;
        blitbuff[i].x=10000;
        blitbuff[i].y=10000;
        blitbuff[i].w=0;
        blitbuff[i].h=0;
        blitbuff[i].next_x = 10000;
        blitbuff[i].next_y = 10000;
        blitbuff[i].bc=0;
        blitbuff[i].layer=-1;
		blitbuff[i].active=false;
        blitbuff[i].edges=0;
    }
    LIFOpointer=0;
    zeroLIFOpointer=0;
    sprites_in_use=0;
    hideall=0;
}
void fun_sprite(void){
    int bnbr=0, w=-1, h=-1,t=0, x=10000, y=10000, l=0, n, c=0;
    getargs(&ep, 5,",");
    if(checkstring(argv[0], "W")) t=1;
    else if(checkstring(argv[0], "H")) t=2;
    else if(checkstring(argv[0], "X")) t=3;
    else if(checkstring(argv[0], "Y")) t=4;
    else if(checkstring(argv[0], "L")) t=5;
    else if(checkstring(argv[0], "C")) t=6;
    else if(checkstring(argv[0], "V")) t=7;
    else if(checkstring(argv[0], "T")) t=8;
    else if(checkstring(argv[0], "E")) t=9;
    else if(checkstring(argv[0], "D")) t=10;
    else if(checkstring(argv[0], "A")) t=11;
    else if(checkstring(argv[0], "N")) t=12;
    else if(checkstring(argv[0], "S")) t=13;
    else error("Syntax");
    if(t<12){
        if(argc<3)error("Syntax");
        if(*argv[2] == '#') argv[2]++;
        bnbr = getint(argv[2],0,MAXBLITBUF-1);
        if(bnbr==0){
            if(argc==5 && !(t==7 || t==10)){
                n=getint(argv[4],1,blitbuff[0].collisions[0]);
                c=blitbuff[0].collisions[n];
            } else c=blitbuff[0].collisions[0];
        }
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            w=blitbuff[bnbr].w;
            h=blitbuff[bnbr].h;
        } 
        if(blitbuff[bnbr].active){
            x=blitbuff[bnbr].x;
            y=blitbuff[bnbr].y;
            l=blitbuff[bnbr].layer;
            if(argc==5 && !(t==7 || t==10)){
                 n=getint(argv[4],1,blitbuff[bnbr].collisions[0]);
                 c=blitbuff[bnbr].collisions[n];
            } else c=blitbuff[bnbr].collisions[0];
        }
    }
    if(t==1)iret=w;
    else if(t==2)iret=h;
    else if(t==3) {if(blitbuff[bnbr].active)iret=x; else iret=10000;}
    else if(t==4) {if(blitbuff[bnbr].active)iret=y; else iret=10000;}
    else if(t==5) {if(blitbuff[bnbr].active)iret=l; else iret=-1;}
    else if(t==8) {if(blitbuff[bnbr].active)iret=blitbuff[bnbr].lastcollisions; else iret=0;}
    else if(t==9) {if(blitbuff[bnbr].active)iret=blitbuff[bnbr].edges; else iret=0;}
    else if(t==6) {if(blitbuff[bnbr].collisions[0])iret=c; else iret=-1;}
	else if(t==11) iret=(int64_t)((uint32_t)blitbuff[bnbr].blitbuffptr);
    else if(t==7){
    	int rbnbr=0;
    	int x1=0,y1=0,h1=0,w1=0;
    	double vector;
    	if(argc<5)error("Syntax");
        if(*argv[4] == '#') argv[4]++;
        rbnbr = getint(argv[4],1,MAXBLITBUF-1);
        if(blitbuff[rbnbr].blitbuffptr!=NULL){
            w1=blitbuff[rbnbr].w;
            h1=blitbuff[rbnbr].h;
        }
        if(blitbuff[rbnbr].active){
            x1=blitbuff[rbnbr].x;
            y1=blitbuff[rbnbr].y;
        }
        if(!(blitbuff[bnbr].active && blitbuff[rbnbr].active))fret=-1.0;
        else {
        	x+=w/2;
        	y+=h/2;
        	x1+=w1/2;
        	y1+=h1/2;
        	y1-=y;
        	x1-=x;
        	vector=atan2(y1,x1);
        	vector+=PI_VALUE/2.0;
        	if(vector<0)vector+=PI_VALUE*2.0;
        	fret=vector;
        }
        targ=T_NBR;
        return;
    } else if(t==10){
        	int rbnbr=0;
        	int x1=0,y1=0,h1=0,w1=0;
        	if(argc<5)error("Syntax");
            if(*argv[4] == '#') argv[4]++;
            rbnbr = getint(argv[4],1,MAXBLITBUF-1);
            if(blitbuff[rbnbr].blitbuffptr!=NULL){
                w1=blitbuff[rbnbr].w;
                h1=blitbuff[rbnbr].h;
            }
            if(blitbuff[rbnbr].active){
                x1=blitbuff[rbnbr].x;
                y1=blitbuff[rbnbr].y;
            }
            if(!(blitbuff[bnbr].active && blitbuff[rbnbr].active))fret=-1.0;
            else {
            	x+=w/2;
            	y+=h/2;
            	x1+=w1/2;
            	y1+=h1/2;
            	fret=sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y));
            }
            targ=T_NBR;
            return;
    } else if(t==12) {
            if(argc==3){
                n=getint(argv[2],0,MAXLAYER);
                iret=layer_in_use[n];
            } else iret=sprites_in_use;
    } else if(t==13) iret = sprite_which_collided;
    else {
    }
    targ = T_INT;   
}
void checklimits(int bnbr, int *n){
    int maxW=PageTable[WritePage].xmax;
    int maxH=PageTable[WritePage].ymax;
		blitbuff[bnbr].collisions[*n]=0;
		if(blitbuff[bnbr].x<0){
        	if(!(blitbuff[bnbr].edges & 1)){
        		blitbuff[bnbr].edges|=1;
        		blitbuff[bnbr].collisions[*n]=0xF1;
        		(*n)++;
        	}
        } else blitbuff[bnbr].edges &= ~1;

        if(blitbuff[bnbr].y<0){
        	if(!(blitbuff[bnbr].edges &2)){
        		blitbuff[bnbr].edges|=2;
        		if(blitbuff[bnbr].collisions[*n] & 0xF0)blitbuff[bnbr].collisions[*n]|=0xF2;
        		else {
        			blitbuff[bnbr].collisions[*n]=0xF2;
        			(*n)++;
        		}
        	}
        }  else blitbuff[bnbr].edges &= ~2;

        if(blitbuff[bnbr].x + blitbuff[bnbr].w > maxW){
        	if(!(blitbuff[bnbr].edges &4)){
        		blitbuff[bnbr].edges|=4;
         		if(blitbuff[bnbr].collisions[*n] & 0xF0)blitbuff[bnbr].collisions[*n]|=0xF4;
        		else {
        			blitbuff[bnbr].collisions[*n]=0xF4;
        			(*n)++;
        		}
            }
        }  else blitbuff[bnbr].edges &= ~4;

        if(blitbuff[bnbr].y + blitbuff[bnbr].h > maxH){
        	if(!(blitbuff[bnbr].edges & 8)){
        		blitbuff[bnbr].edges|=8;
         		if(blitbuff[bnbr].collisions[*n] & 0xF0)blitbuff[bnbr].collisions[*n]|=0xF8;
        		else {
        			blitbuff[bnbr].collisions[*n]=0xF8;
        			(*n)++;
        		}
        	}
        }  else blitbuff[bnbr].edges &= ~8;
}

void ProcessCollisions(int bnbr){
    int k, j=1, n=1, bcol=1;
    //We know that any collision is caused by movement of sprite bnbr
    // a value of zero indicates that we are processing movement of layer 0 and any
    // sprites on that layer
    CollisionFound=false;
    sprite_which_collided=-1;
    uint64_t mask, mymask=(uint64_t)1<<((uint64_t)bnbr-(uint64_t)1);
    mymemset(blitbuff[0].collisions, 0, MAXCOLLISIONS);
    if(bnbr!=0){ // a specific sprite has moved
    	mymemset( blitbuff[bnbr].collisions, 0, MAXCOLLISIONS); //clear our previous collisions
        if(blitbuff[bnbr].layer!=0){
            if(layer_in_use[blitbuff[bnbr].layer]+layer_in_use[0]>1){ //other sprites in this layer
                for(k=1;k<MAXBLITBUF;k++){
        			mask=(uint64_t)1<<((uint64_t)k-(uint64_t)1);
                	if(!(blitbuff[k].active)){
                		blitbuff[bnbr].lastcollisions &= ~mask;
                		continue;
                	}
                	if(k==bnbr) continue;
                    if(j == layer_in_use[blitbuff[bnbr].layer]+layer_in_use[0]) break; //nothing left to process
                    if((blitbuff[k].layer == blitbuff[bnbr].layer || blitbuff[k].layer == 0)){
                        j++;
                        if( !(blitbuff[k].x + blitbuff[k].w < blitbuff[bnbr].x ||
                                blitbuff[k].x > blitbuff[bnbr].x+blitbuff[bnbr].w ||
                                blitbuff[k].y + blitbuff[k].h < blitbuff[bnbr].y ||
                                blitbuff[k].y > blitbuff[bnbr].y + blitbuff[bnbr].h)){
                					if(n<MAXCOLLISIONS && !(blitbuff[bnbr].lastcollisions & mask))blitbuff[bnbr].collisions[n++]=k;
                					blitbuff[bnbr].lastcollisions |= mask;
                					blitbuff[k].lastcollisions |= mymask;
                        }	else {
                        	blitbuff[bnbr].lastcollisions &= ~mask;
        					blitbuff[k].lastcollisions &= ~mymask;
                        }
                    }
                }
            }
        } else {
            for(k=1;k<MAXBLITBUF;k++){
                if(j == sprites_in_use) break; //nothing left to process
                if(k == bnbr) continue;
    			mask=(uint64_t)1<<((uint64_t)k-(uint64_t)1);
            	if(!(blitbuff[k].active)){
            		blitbuff[bnbr].lastcollisions &= ~mask;
            		continue;
            	} else j++;
                if( !(blitbuff[k].x + blitbuff[k].w < blitbuff[bnbr].x ||
                        blitbuff[k].x > blitbuff[bnbr].x+blitbuff[bnbr].w ||
                        blitbuff[k].y + blitbuff[k].h < blitbuff[bnbr].y ||
                        blitbuff[k].y > blitbuff[bnbr].y + blitbuff[bnbr].h)){
							if(n<MAXCOLLISIONS && !(blitbuff[bnbr].lastcollisions & mask))blitbuff[bnbr].collisions[n++]=k;
								blitbuff[bnbr].lastcollisions |= mask;
            					blitbuff[k].lastcollisions |= mymask;
                			}	else {
                				blitbuff[bnbr].lastcollisions &= ~mask;
            					blitbuff[k].lastcollisions &= ~mymask;
                			}
            }

        }
// now look for collisions with the edge of the screen
        checklimits(bnbr, &n);
        if(n>1){
            CollisionFound=true;
            sprite_which_collided=bnbr;
            blitbuff[bnbr].collisions[0]=n-1;
        }
    } else { //the background layer has moved
        j=0;
        for(k=1;k<MAXBLITBUF;k++){ //loop through all sprites
			mask=(uint64_t)1<<((uint64_t)k-(uint64_t)1);
            n=1;
            int kk, jj=1;
            if(j == sprites_in_use) break; //nothing left to process
            if(blitbuff[k].active){ //sprite found
                mymemset( blitbuff[k].collisions, 0, MAXCOLLISIONS);
                j++;
                if(layer_in_use[blitbuff[k].layer]+layer_in_use[0]>1){ //other sprites in this layer
                    for(kk=1;kk<MAXBLITBUF;kk++){
                        if(kk == k) continue;
                        if(jj == layer_in_use[blitbuff[k].layer]+layer_in_use[0]) break; //nothing left to process
                        if((blitbuff[kk].layer == blitbuff[k].layer || blitbuff[kk].layer == 0)){
                            jj++;
                            if( !(blitbuff[kk].x + blitbuff[kk].w < blitbuff[k].x ||
                                blitbuff[kk].x > blitbuff[k].x+blitbuff[k].w ||
                                blitbuff[kk].y + blitbuff[kk].h < blitbuff[k].y ||
                                blitbuff[kk].y > blitbuff[k].y + blitbuff[k].h)){
    							if(n<MAXCOLLISIONS && !(blitbuff[k].lastcollisions & mask))blitbuff[k].collisions[n++]=kk;
    								blitbuff[k].lastcollisions |= mask;
                    			}	else {
                    				blitbuff[k].lastcollisions &= ~mask;
                    			}
                        }
                    }
                }
                checklimits(k, &n);
                if(n>1 && n<MAXCOLLISIONS && bcol<MAXCOLLISIONS){
                    blitbuff[0].collisions[bcol]=k;
                    bcol++;
                    blitbuff[k].collisions[0]=n-1;
                }
            }
        }
        if(bcol>1){
            CollisionFound=true;
            sprite_which_collided=0;
            blitbuff[0].collisions[0]=bcol-1;
        }
    }
}

void blithide(int bnbr, int free){
    int w, h, x1, y1;
    w=blitbuff[bnbr].w;
    h=blitbuff[bnbr].h;
    x1 = blitbuff[bnbr].x;
    y1 = blitbuff[bnbr].y;
	blitbuff[bnbr].active=0;
    DrawBufferFast(x1,y1,x1+w-1,y1+h-1,blitbuff[bnbr].blitstoreptr);
}

void BlitShowBuff32(int bnbr, int x1, int y1, int mode){
    char *current, *r, *rr, *q, *qq;
    int x, y, rotation, linelength;
    rotation = blitbuff[bnbr].rotation;
    ReadPage=WritePage;
    union colourmap
    {
        char rgbbytes[4];
        short rgb[2];
        int r;
    } a, b __attribute((unused));
    int w, h;
    if(blitbuff[bnbr].blitbuffptr!=NULL){
        qq = q = blitbuff[bnbr].blitbuffptr;
        w=blitbuff[bnbr].w;
        h=blitbuff[bnbr].h;
        linelength=w*4;
        current = blitbuff[bnbr].blitstoreptr;
        if(!(mode==0 || mode & 4) && blitbuff[bnbr].active){
            DrawBufferFast(blitbuff[bnbr].x, blitbuff[bnbr].y, blitbuff[bnbr].x + w - 1, blitbuff[bnbr].y + h - 1,current);
        }
        blitbuff[bnbr].x=x1;
        blitbuff[bnbr].y=y1;
        if(!(mode==2))ReadBufferFast(x1,y1,x1+w-1,y1+h-1,current);
        // we now have the old screen image stored together with the coordinates
        rr = r = GetMemory(w*h*4);
        a.r=0;
        b.r=0;
        for(y=0; y<h; y++){
            if(rotation<2)qq=q+linelength*y;
            else qq=q+(h-y-1)*linelength;
            if(rotation==1 || rotation==3)qq+=linelength;
            for(x=0; x<w; x++){
                if(rotation==1 || rotation==3){
                    a.rgbbytes[3]=*--qq;
                    a.rgbbytes[2]=*--qq;
                    a.rgbbytes[1]=*--qq;
                    a.rgbbytes[0]=*--qq;
                } else {
                    a.rgbbytes[0]=*qq++;
                    a.rgbbytes[1]=*qq++;
                    a.rgbbytes[2]=*qq++;
                    a.rgbbytes[3]=*qq++;
                }
                b.rgbbytes[0]=*current++;
                b.rgbbytes[1]=*current++;
                b.rgbbytes[2]=*current++;
                b.rgbbytes[3]=*current++;
                if(a.r!=blitbuff[bnbr].bc || mode & 8){
                    *r++=a.rgbbytes[0];
                    *r++=a.rgbbytes[1];
                    *r++=a.rgbbytes[2];
                    *r++=a.rgbbytes[3];
                } else {
                    *r++=b.rgbbytes[0];
                    *r++=b.rgbbytes[1];
                    *r++=b.rgbbytes[2];
                    *r++=b.rgbbytes[3];
                }
            }
        }
        DrawBufferFast(x1,y1,x1+w-1,y1+h-1,rr);
        if(!(mode & 4))blitbuff[bnbr].active=1;
        FreeMemory(rr);
    }
}

void BlitShowBuff16(int bnbr, int x1, int y1, int mode){
    char *current, *r, *rr, *q, *qq;
    int x, y, rotation, linelength;
    rotation = blitbuff[bnbr].rotation;
    ReadPage=WritePage;
    union colourmap
    {
        char rgbbytes[4];
        short rgb[2];
        int r;
    } a, b __attribute((unused));
    int w, h;
    if(blitbuff[bnbr].blitbuffptr!=NULL){
        qq = q = blitbuff[bnbr].blitbuffptr;
        w=blitbuff[bnbr].w;
        h=blitbuff[bnbr].h;
        linelength=w*2;
        current = blitbuff[bnbr].blitstoreptr;
        if(!(mode==0 || mode & 4) && blitbuff[bnbr].active){
            DrawBufferFast(blitbuff[bnbr].x, blitbuff[bnbr].y, blitbuff[bnbr].x + w - 1, blitbuff[bnbr].y + h - 1,current);
        }
        blitbuff[bnbr].x=x1;
        blitbuff[bnbr].y=y1;
        if(!(mode==2))ReadBufferFast(x1,y1,x1+w-1,y1+h-1,current);
        // we now have the old screen image stored together with the coordinates
        rr = r = GetMemory(w*h*2);
        a.r=0;
        b.r=0;
        for(y=0; y<h; y++){
            if(rotation<2)qq=q+linelength*y;
            else qq=q+(h-y-1)*linelength;
            if(rotation==1 || rotation==3)qq+=linelength;
            for(x=0; x<w; x++){
                if(rotation==1 || rotation==3){
                    a.rgbbytes[1]=*--qq;
                    a.rgbbytes[0]=*--qq;
                } else {
                    a.rgbbytes[0]=*qq++;
                    a.rgbbytes[1]=*qq++;
                }
                b.rgbbytes[0]=*current++;
                b.rgbbytes[1]=*current++;
                if(a.r!=blitbuff[bnbr].bc || mode & 8){
                    *r++=a.rgbbytes[0];
                    *r++=a.rgbbytes[1];
                } else {
                    *r++=b.rgbbytes[0];
                    *r++=b.rgbbytes[1];
                }
            }
        }
        DrawBufferFast(x1,y1,x1+w-1,y1+h-1,rr);
        if(!(mode & 4))blitbuff[bnbr].active=1;
        FreeMemory(rr);
    }
}
void BlitShowBuff8(int bnbr, int x1, int y1, int mode){
    char *current, *r, *rr, *q, *qq;
    int x, y, rotation, linelength;
    rotation = blitbuff[bnbr].rotation;
    ReadPage=WritePage;
    char a, b ;
    int w, h;
        qq = q = blitbuff[bnbr].blitbuffptr;
        w=blitbuff[bnbr].w;
        h=blitbuff[bnbr].h;
        linelength=w;
        current = blitbuff[bnbr].blitstoreptr;
        if(!(mode==0 || mode & 4) && blitbuff[bnbr].active){
            DrawBufferFast(blitbuff[bnbr].x, blitbuff[bnbr].y, blitbuff[bnbr].x + w - 1, blitbuff[bnbr].y + h - 1,current);
        }
        blitbuff[bnbr].x=x1;
        blitbuff[bnbr].y=y1;
        if(!(mode==2))ReadBufferFast(x1,y1,x1+w-1,y1+h-1,current);
        // we now have the old screen image stored together with the coordinates
        rr = r = GetMemory(w*h);
        a=0;
        b=0;
        for(y=0; y<h; y++){
            if(rotation<2)qq=q+linelength*y;
            else qq=q+(h-y-1)*linelength;
            if(rotation==1 || rotation==3)qq+=linelength;
            for(x=0; x<w; x++){
                if(rotation==1 || rotation==3){
                    a=*--qq;
                } else {
                    a=*qq++;
                }
                b=*current++;
                if(a!=blitbuff[bnbr].bc || mode & 8){
                    *r++=a;

                } else {
                    *r++=b;
                }
            }
        }
        DrawBufferFast(x1,y1,x1+w-1,y1+h-1,rr);
        if(!(mode & 4))blitbuff[bnbr].active=1;
        FreeMemory(rr);
}

int sumlayer(void){
    int i,j=0;
    for(i=0; i<=MAXLAYER ;i++)j+=layer_in_use[i];
    return j;
}
void loadarray(char *p){
    int bnbr,w,h, size,i;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;
    void *ptr1 = NULL;
    MMFLOAT *a3float=NULL;
    int64_t *a3int=NULL;
    char *q;
    uint16_t *qq;
    uint32_t *qqq;
    getargs(&p, 7,",");
    if(*argv[0] == '#') argv[0]++;
    bnbr=getint(argv[0],1,MAXBLITBUF-1);
    if(blitbuff[bnbr].blitbuffptr==NULL){
    	w=getint(argv[2],1,maxW);
    	h=getint(argv[4],1,maxH);
        ptr1 = findvar(argv[6], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_NBR) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be array");
            }
            a3float = (MMFLOAT *)ptr1;
        } else if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be array");
            }
            a3int = (int64_t *)ptr1;
        } else error("Argument 1 must be array");
        size=(vartbl[VarIndex].dims[0] - OptionBase);
        if(size<w*h-1)error("Array Dimensions");
        blitbuff[bnbr].blitbuffptr = GetMemory(w*h*PageTable[WritePage].nbytes);
        blitbuff[bnbr].blitstoreptr = GetMemory(w*h*PageTable[WritePage].nbytes);
        blitbuff[bnbr].bc=0;
        blitbuff[bnbr].w=w;
        blitbuff[bnbr].h=h;
        blitbuff[bnbr].master=0;
        blitbuff[bnbr].mymaster=-1;
        blitbuff[bnbr].x=10000;
        blitbuff[bnbr].y=10000;
        blitbuff[bnbr].layer=-1;
        blitbuff[bnbr].next_x = 10000;
        blitbuff[bnbr].next_y = 10000;
    	blitbuff[bnbr].active=false;
    	blitbuff[bnbr].lastcollisions=0;
        blitbuff[bnbr].edges=0;
    	q=blitbuff[bnbr].blitbuffptr;
        qq=(uint16_t *)q;
        qqq=(uint32_t *)q;
		if (VideoColour==8) {
			int c;
			for(i=0; i<w*h;i++){
				if(a3float)c=(int)a3float[i];
				else c=(int)a3int[i];
                *q++ = ((c & 0b111000000000000000000000)>>16) | ((c & 0b1110000000000000)>>11) | ((c & 0b11000000)>>6);
			}
        } else if (VideoColour==16) {
			int c;
			for(i=0; i<w*h;i++){
				if(a3float)c=(int)a3float[i];
				else c=(int)a3int[i];
                *qq++=convert_16bit(c);
			}
        } else if (VideoColour==12) {
			int c;
			for(i=0; i<w*h;i++){
				if(a3float)c=(int)a3float[i];
				else c=(int)a3int[i];
                *qq++=convert_12bit(c);
			}
        } else if (VideoColour==32){
			int c;
			for(i=0; i<w*h;i++){
				if(a3float)c=(int)a3float[i];
				else c=(int)a3int[i];
                *qqq++=c;
			}
        }
    } else error("Buffer already in use");
}
void loadpng(char *p){
	upng_t* upng;
    union colourmap
    {
        unsigned char rgbbytes[4];
        unsigned short rgb[2];
        unsigned int r;
    } b __attribute((unused)),c;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
    int bnbr, i, w, h, deftrans=8;
    char *q;
    uint16_t *qq;
    uint32_t *qqq;
    getargs(&p, 5,",");
    if(*argv[0] == '#') argv[0]++;
    bnbr=getint(argv[0],1,MAXBLITBUF-1);
    if(blitbuff[bnbr].blitbuffptr==NULL){
        char *p=getCstring(argv[2]);
        if(argc==5)deftrans=getint(argv[4],1,15);
        upng = upng_new_from_file(p);
        upng_header(upng);
        if(upng_get_width(upng) >= maxW || upng_get_height(upng) >= maxH){
            upng_free(upng);
            error("Image too large");
        }
        if(!(upng_get_format(upng)==1 || upng_get_format(upng)==3)){
            upng_free(upng);
            error("Invalid format");
        }
        upng_decode(upng);
        w=upng_get_width(upng);
        h=upng_get_height(upng);
//        PInt(w*h*PageTable[WritePage].nbytes);PRet();MM_Delay(500);
        blitbuff[bnbr].blitbuffptr = GetMemory(w*h*PageTable[WritePage].nbytes);
        blitbuff[bnbr].blitstoreptr = GetMemory(w*h*PageTable[WritePage].nbytes);
        blitbuff[bnbr].bc=0;
        blitbuff[bnbr].w=w;
        blitbuff[bnbr].h=h;
        blitbuff[bnbr].master=0;
        blitbuff[bnbr].mymaster=-1;
        blitbuff[bnbr].x=10000;
        blitbuff[bnbr].y=10000;
        blitbuff[bnbr].layer=-1;
        blitbuff[bnbr].next_x = 10000;
        blitbuff[bnbr].next_y = 10000;
		blitbuff[bnbr].active=false;
		blitbuff[bnbr].lastcollisions=0;
        blitbuff[bnbr].edges=0;
		q=blitbuff[bnbr].blitbuffptr;
        qq=(uint16_t *)q;
        qqq=(uint32_t *)q;
		if (VideoColour==8) {
            const unsigned char *rr;
            rr=upng_get_buffer(upng);
            i=upng_get_width(upng)*upng_get_height(upng)*4;
            c.rgbbytes[3]=0;
            while(i){
                c.rgbbytes[2]=*rr++;
                c.rgbbytes[1]=*rr++;
                c.rgbbytes[0]=*rr++;
                if(upng_get_format(upng)==3){
                	c.rgbbytes[3]=*rr++;
                	if((c.rgbbytes[3]>>4) < deftrans) c.rgbbytes[3]=0;
                	else c.rgbbytes[3]=0xF0;
                }
                else c.rgbbytes[3]=1;
                if(c.rgbbytes[3]==0)*q=0;
                else {
                    *q = ((c.r & 0b111000000000000000000000)>>16) | ((c.r & 0b1110000000000000)>>11) | ((c.r & 0b11000000)>>6);
                    if(*q==0 && c.r!=0){ //problem
                    	*q=convert_8bit(NOTBLACK);
                    }
                }
                q++;
                i-=4;
            }
        } else if (VideoColour==16) {
        	uint32_t sc;
            const unsigned char *rr;
            rr=upng_get_buffer(upng);
            i=upng_get_width(upng)*upng_get_height(upng)*4;
            c.rgbbytes[3]=0;
            while(i){
                c.rgbbytes[2]=*rr++;
                c.rgbbytes[1]=*rr++;
                c.rgbbytes[0]=*rr++;
                if(upng_get_format(upng)==3){
                	c.rgbbytes[3]=*rr++;
                	if((c.rgbbytes[3]>>4) < deftrans) c.rgbbytes[3]=0;
                	else c.rgbbytes[3]=0xF0;
                }
                else c.rgbbytes[3]=1;
                if(c.rgbbytes[3]==0)sc=0;
                else {
                	sc=convert_16bit(c.r);
                    if(c.r!=0 && sc==0){
                    	sc=convert_16bit(NOTBLACK);
                    }
                }
       			*qq++=sc;
       			i-=4;
            }
        } else if (VideoColour==12) {
        	uint32_t sc;
            const unsigned char *rr;
            rr=upng_get_buffer(upng);
            i=upng_get_width(upng)*upng_get_height(upng)*4;
            c.rgbbytes[3]=0;
            while(i){
            	c.rgbbytes[3]=0;
                c.rgbbytes[2]=*rr++;
                c.rgbbytes[1]=*rr++;
                c.rgbbytes[0]=*rr++;
                if(upng_get_format(upng)==3){
                	c.rgbbytes[3]=*rr++;
                	if((c.rgbbytes[3]>>4) < deftrans){
                		c.r=0;
                	}
                	else c.rgbbytes[3]=0xF0;
                }
            	sc=convert_12bit(c.r);
                if(c.r!=0 && sc==0){
                	sc=convert_12bit(NOTBLACK);
                }
                if(sc)sc|=0xF000;
     			*qq++=sc;
     			i-=4;
            }
        } else if (VideoColour==32){
            const unsigned char *rr;
            rr=upng_get_buffer(upng);
            i=upng_get_width(upng)*upng_get_height(upng)*4;
            c.rgbbytes[3]=0;
            while(i){
            	c.rgbbytes[3]=0;
                c.rgbbytes[2]=*rr++;
                c.rgbbytes[1]=*rr++;
                c.rgbbytes[0]=*rr++;
                if(upng_get_format(upng)==3)c.rgbbytes[3]=*rr++;
                else c.rgbbytes[3]=0xFF;
     			*qqq++=c.r;
     			i-=4;
            }
        }
        upng_free(upng);
    } else error("Buffer already in use");
}
void hidesafe(int bnbr){
    int found=0;
    int zerofound=0;
	int i;
    for(i=LIFOpointer-1; i>= 0; i--) {
    	if(LIFO[i]==bnbr){
    		blithide(LIFO[i],0);
    		found=i;
    		break;
    	}
    	blithide(LIFO[i],0);
    }
    if(!found){
		for(i=zeroLIFOpointer-1; i>= 0; i--){
			if(zeroLIFO[i]==bnbr){
				blithide(zeroLIFO[i],0);
				found=i;
				zerofound=1;
				break;
			}
			blithide(zeroLIFO[i],0);
		}
    }
    sprites_in_use--;
    layer_in_use[blitbuff[bnbr].layer]--;
    blitbuff[bnbr].x=10000;
    blitbuff[bnbr].y=10000;
    if(blitbuff[bnbr].layer==0)zeroLIFOremove(bnbr);
    else LIFOremove(bnbr);
    blitbuff[bnbr].layer=-1;
    blitbuff[bnbr].next_x = 10000;
    blitbuff[bnbr].next_y = 10000;
	blitbuff[bnbr].lastcollisions=0;
    blitbuff[bnbr].edges=0;
    if(zerofound){
        for(i=found; i< zeroLIFOpointer; i++){
        	BlitShowBuffer(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        }
        for(i=0; i< LIFOpointer; i++){
            BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
        }
    } else{
        for(i=found; i< LIFOpointer; i++){
            BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
        }
    }
}

void showsafe(int bnbr,int x,int y){
    int found=0;
	int i;
    for(i=LIFOpointer-1; i>= 0; i--) {
    	if(LIFO[i]==bnbr){
    		blithide(LIFO[i],0);
    		found=i;
    		break;
    	}
    	blithide(LIFO[i],0);
    }
    if(!found){
		for(i=zeroLIFOpointer-1; i>= 0; i--){
			if(zeroLIFO[i]==bnbr){
				blithide(zeroLIFO[i],0);
				found=-i;
				break;
			}
			blithide(zeroLIFO[i],0);
		}
    }
	BlitShowBuffer(bnbr, x, y, 1);
    if(found<0){
    	found=-found;
        for(i=found+1; i< zeroLIFOpointer; i++){
        	BlitShowBuffer(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        }
        for(i=0; i< LIFOpointer; i++){
            BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
        }
    } else{
        for(i=found+1; i< LIFOpointer; i++){
            BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
        }
    }

}
void loadsprite(char *p){
    int fnbr, width, number,height=0, newsprite=1, startsprite=1, bnbr, lc, i;
    char *q, *fname;
    short *qq;
	uint32_t *qqq;
    char buff[256];
    getargs(&p, 3,",");
    fnbr = FindFreeFileNbr();
    fname = getFstring(argv[0]);
    if(argc==3)startsprite=getint(argv[2],1,64);
    if(strchr(fname, '.') == NULL) strcat(p, ".SPR");
    if(!BasicFileOpen(fname, fnbr, FA_READ)) error("File not found");
	MMgetline(fnbr, (char *)buff);							    // get the input line
	while(buff[0]==39)MMgetline(fnbr, (char *)buff);
	sscanf((char *)buff, "%d,%d, %d", &width, &number, &height);
	if(height==0)height=width;
	bnbr=startsprite;
	if(number+startsprite>MAXBLITBUF){
		FileClose(fnbr);
		error("Maximum of 64 sprites");
	}
	while(!FileEOF(fnbr) && bnbr<=number+startsprite) {                                     // while waiting for the end of file
		if(newsprite){
			newsprite=0;
	        if(blitbuff[bnbr].blitbuffptr==NULL)blitbuff[bnbr].blitbuffptr = GetMemory(width*height* (VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
	        if(blitbuff[bnbr].blitstoreptr==NULL)blitbuff[bnbr].blitstoreptr = GetMemory(width*height* (VideoColour==8 ? 1 : (VideoColour<=16 ? 2 : 4)));
	        blitbuff[bnbr].bc=0;
	        blitbuff[bnbr].w=width;
	        blitbuff[bnbr].h=height;
	        blitbuff[bnbr].master=0;
	        blitbuff[bnbr].mymaster=-1;
	        blitbuff[bnbr].x=10000;
	        blitbuff[bnbr].y=10000;
	        blitbuff[bnbr].layer=-1;
	        blitbuff[bnbr].next_x = 10000;
	        blitbuff[bnbr].next_y = 10000;
	        blitbuff[bnbr].active=false;
	        blitbuff[bnbr].lastcollisions=0;
	        blitbuff[bnbr].edges=0;
	        q=blitbuff[bnbr].blitbuffptr;
	        qq=(short *)blitbuff[bnbr].blitbuffptr;
	        qqq=(uint32_t *)blitbuff[bnbr].blitbuffptr;
	        lc=height;
		}
		while(lc--){
			MMgetline(fnbr, (char *)buff);									    // get the input line
			while(buff[0]==39)MMgetline(fnbr, (char *)buff);
			if(strlen(buff)<width)mymemset(&buff[strlen(buff)],32,width-strlen(buff));
			if(VideoColour==8){
				for(i=0;i<width;i++){
					if(buff[i]==' ')*q++=0;
					else if(buff[i]=='0')*q++=convert_8bit(NOTBLACK);
					else if(buff[i]=='1')*q++=convert_8bit(BLUE);
					else if(buff[i]=='2')*q++=convert_8bit(GREEN);
					else if(buff[i]=='3')*q++=convert_8bit(CYAN);
					else if(buff[i]=='4')*q++=convert_8bit(RED);
					else if(buff[i]=='5')*q++=convert_8bit(MAGENTA);
					else if(buff[i]=='6')*q++=convert_8bit(YELLOW);
					else if(buff[i]=='7')*q++=convert_8bit(WHITE);
					else if(buff[i]=='8')*q++=convert_8bit(LITEGRAY);
					else if(buff[i]=='9')*q++=convert_8bit(GRAY);
					else if(buff[i]=='A' || buff[i]=='a')*q++=convert_8bit(ORANGE);
					else if(buff[i]=='B' || buff[i]=='b')*q++=convert_8bit(PINK);
					else if(buff[i]=='C' || buff[i]=='c')*q++=convert_8bit(GOLD);
					else if(buff[i]=='D' || buff[i]=='d')*q++=convert_8bit(SALMON);
					else if(buff[i]=='E' || buff[i]=='e')*q++=convert_8bit(BEIGE);
					else if(buff[i]=='F' || buff[i]=='f')*q++=convert_8bit(BROWN);
					else *q++=0;
				}
			}
			if(VideoColour==12){
				for(i=0;i<width;i++){
					if(buff[i]==' ')*qq++=0;
					else if(buff[i]=='0')*qq++=0xF000;
					else if(buff[i]=='1')*qq++=convert_12bit(BLUE)|0xF000;
					else if(buff[i]=='2')*qq++=convert_12bit(GREEN)|0xF000;
					else if(buff[i]=='3')*qq++=convert_12bit(CYAN)|0xF000;
					else if(buff[i]=='4')*qq++=convert_12bit(RED)|0xF000;
					else if(buff[i]=='5')*qq++=convert_12bit(MAGENTA)|0xF000;
					else if(buff[i]=='6')*qq++=convert_12bit(YELLOW)|0xF000;
					else if(buff[i]=='7')*qq++=convert_12bit(WHITE)|0xF000;
					else if(buff[i]=='8')*qq++=convert_12bit(LITEGRAY)|0xF000;
					else if(buff[i]=='9')*qq++=convert_12bit(GRAY)|0xF000;
					else if(buff[i]=='A' || buff[i]=='a')*qq++=convert_12bit(ORANGE)|0xF000;
					else if(buff[i]=='B' || buff[i]=='b')*qq++=convert_12bit(PINK)|0xF000;
					else if(buff[i]=='C' || buff[i]=='c')*qq++=convert_12bit(GOLD)|0xF000;
					else if(buff[i]=='D' || buff[i]=='d')*qq++=convert_12bit(SALMON)|0xF000;
					else if(buff[i]=='E' || buff[i]=='e')*qq++=convert_12bit(BEIGE)|0xF000;
					else if(buff[i]=='F' || buff[i]=='f')*qq++=convert_12bit(BROWN)|0xF000;
					else *qq++=0;
				}
			}
			if(VideoColour==16){
				for(i=0;i<width;i++){
					if(buff[i]==' ')*qq++=0;
					else if(buff[i]=='0')*qq++=convert_16bit(NOTBLACK);
					else if(buff[i]=='1')*qq++=convert_16bit(BLUE);
					else if(buff[i]=='2')*qq++=convert_16bit(GREEN);
					else if(buff[i]=='3')*qq++=convert_16bit(CYAN);
					else if(buff[i]=='4')*qq++=convert_16bit(RED);
					else if(buff[i]=='5')*qq++=convert_16bit(MAGENTA);
					else if(buff[i]=='6')*qq++=convert_16bit(YELLOW);
					else if(buff[i]=='7')*qq++=convert_16bit(WHITE);
					else if(buff[i]=='8')*qq++=convert_16bit(LITEGRAY);
					else if(buff[i]=='9')*qq++=convert_16bit(GRAY);
					else if(buff[i]=='A' || buff[i]=='a')*qq++=convert_16bit(ORANGE);
					else if(buff[i]=='B' || buff[i]=='b')*qq++=convert_16bit(PINK);
					else if(buff[i]=='C' || buff[i]=='c')*qq++=convert_16bit(GOLD);
					else if(buff[i]=='D' || buff[i]=='d')*qq++=convert_16bit(SALMON);
					else if(buff[i]=='E' || buff[i]=='e')*qq++=convert_16bit(BEIGE);
					else if(buff[i]=='F' || buff[i]=='f')*qq++=convert_16bit(BROWN);
					else *qq++=0;
				}
			}
			if(VideoColour==32){
				for(i=0;i<width;i++){
					if(buff[i]==' ')*qqq++=0;
					else if(buff[i]=='0')*qqq++=NOTBLACK;
					else if(buff[i]=='1')*qqq++=BLUE;
					else if(buff[i]=='2')*qqq++=GREEN;
					else if(buff[i]=='3')*qqq++=CYAN;
					else if(buff[i]=='4')*qqq++=RED;
					else if(buff[i]=='5')*qqq++=MAGENTA;
					else if(buff[i]=='6')*qqq++=YELLOW;
					else if(buff[i]=='7')*qqq++=WHITE;
					else if(buff[i]=='8')*qqq++=LITEGRAY;
					else if(buff[i]=='9')*qqq++=convert_16bit(GRAY);
					else if(buff[i]=='A' || buff[i]=='a')*qqq++=ORANGE;
					else if(buff[i]=='B' || buff[i]=='b')*qqq++=PINK;
					else if(buff[i]=='C' || buff[i]=='c')*qqq++=GOLD;
					else if(buff[i]=='D' || buff[i]=='d')*qqq++=SALMON;
					else if(buff[i]=='E' || buff[i]=='e')*qqq++=BEIGE;
					else if(buff[i]=='F' || buff[i]=='f')*qqq++=BROWN;
					else *qqq++=0;
				}
			}
		}
		bnbr++;
		newsprite=1;
	}
	FileClose(fnbr);
}
char getnextuncompressednibble(char **s, int reset){
    static int toggle=0;
    if(reset){
        toggle=reset-1;
        return 0;
    }
    if(!toggle){
        toggle ^=1;
        return **s & 0x0f;
    } else {
        toggle ^=1;
        char r=(**s & 0xf0)>>4;
        (*s)++;
        return r;
    }

}
static inline char getnextnibble(char **fc, int reset){
    static uint8_t available;
    static char out;
    if(reset){
        available=0;
    }
    if(available==0){
        available=**fc & 0xF; //number of identical pixels
        out=(**fc)>>4;
        (*fc)++;
    }
    if(!reset)available--;
    return out;
}
void copyframetoscreen(uint8_t *s,int xstart, int xend, int ystart, int yend, int odd){
    int c, i=(xend-xstart+1)*(yend-ystart+1)*3;
    char *buff=GetTempMemory(i);
    char *p=buff;
	if(odd){
		c=colours[(*s & 0xF0)>>4];
		*p++=c & 0xFF;
		*p++= (c>>8) & 0xff;
		*p++= c>>16;
		s++;
		i-=3;
	}
	while(i>0){
		c=colours[*s & 0xF];
		*p++=c & 0xFF;
		*p++= (c>>8) & 0xff;
		*p++= c>>16;
		if(i>3){
			c=colours[(*s & 0xF0)>>4];
			*p++=c & 0xFF;
			*p++= (c>>8) & 0xff;
			*p++= c>>16;
		}
		s++;
		i-=6;
	}
	DrawBuffer(xstart,ystart,xend,yend,buff,0);
}
void docompressed(char *fc,int x1, int y1, int w, int h, int8_t blank){
    if(blank==-1){
        char tobuff[w/2], *to;
        int ww=w;
        int xx1=x1;
        if(x1<0){
            ww+=x1;
            xx1=0;
        }
        if(x1+w>HRes){
            ww=HRes-x1;
        }
        getnextnibble(&fc,1); //reset the decoder
        for(int y=y1;y<y1+h;y++){
            to=tobuff;
            int otoggle=0;
            for(int x=x1;x<x1+w;x++){
                if(y<0 || y>=VRes){
                    getnextnibble(&fc,0);
                    continue;
                }
                if(x>=0 && x<HRes){
                    if(otoggle==0){
                        *to=getnextnibble(&fc,0);
                        otoggle ^=1;
                    } else {
                        *to|=(getnextnibble(&fc,0)<<4);
                        otoggle^=1;
                        to++;
                    }
                } else getnextnibble(&fc,0);
            }
            if(ww>0 && xx1<HRes)copyframetoscreen((unsigned char *)tobuff,xx1, xx1+ww-1, y, y, 0);
        }
    } else {
        char tobuff[w/2], *to;
        getnextnibble(&fc,1); //reset the decoder
        for(int y=y1;y<y1+h;y++){
            int x=x1;
            while(1){
                to=tobuff;
                int otoggle=0;
                char c;
                int ww=0;
                int xx=-1;
                while((c=getnextnibble(&fc,0))==blank){
                    x++;
                    if(x==x1+w)break;
                }
                if(x==x1+w)break; //nothing found so exit
                *to=c;
                otoggle ^=1;
                xx=x;
                x++;
                ww=1;
                if(xx!=x1+w-1){
                    while((c=getnextnibble(&fc,0))!=blank){
                        x++;
                        ww++;
                        if(otoggle==0){
                            *to=c;
                            otoggle ^=1;
                        } else {
                            *to|=(c<<4);
                            otoggle^=1;
                            to++;
                        }
                        if(x==x1+w)break;
                    }
                }
                x++;
                if(xx+ww>HRes){
                    ww=HRes-xx;
                }
                if(xx>=0 && ww>0 && y>=0 && y<VRes)copyframetoscreen((unsigned char *)tobuff,xx, xx+ww-1, y, y, 0);
                if(xx<0 && xx+ww>=0){
                    char *t=tobuff-(xx/2)-(xx&1);
                    ww+=xx;
                    if(ww>0)copyframetoscreen((unsigned char *)t,0, ww-1, y, y, xx&1);
                }
                if(x>=x1+w)break;
            }
        }
    }
}


int blitother(void){
    int x1, y1, w, h;
    char *p;
    if ((p = checkstring(cmdline, "MEMORY"))) {
        int8_t blank=-1;
        getargs(&p, 7, ",");
        if(argc<5)error("Syntax");
        char *from=(char *)GetPeekAddr(argv[0]);
        x1 = (int)getinteger(argv[2]);
        y1 = (int)getinteger(argv[4]);
        uint16_t *size=(uint16_t *)from;
        w=(size[0] & 0x7FFF);
        h=(size[1] & 0x7FFF);
        from+=4;
        if(argc==7)blank=getint(argv[6],-1,15);
        if(size[0] & 0x8000 || size[1] &  0x8000) {
            docompressed(from, x1, y1, w, h, blank);
        } else {
            if(blank==-1){
                 char *fc=from;
                 char tobuff[w/2], *to;
                 int ww=w;
                 int xx1=x1;
                 if(x1<0){
                     ww+=x1;
                     xx1=0;
                 }
                 if(x1+w>HRes){
                     ww=HRes-x1;
                 }
                 getnextuncompressednibble(&fc,1); //reset the decoder
                 for(int y=y1;y<y1+h;y++){
                     to=tobuff;
                     int otoggle=0;
                     for(int x=x1;x<x1+w;x++){
                         if(y<0 || y>=VRes){
                             getnextuncompressednibble(&fc,0);
                             continue;
                         }
                         if(x>=0 && x<HRes){
                             if(otoggle==0){
                                 *to=getnextuncompressednibble(&fc,0);
                                 otoggle ^=1;
                             } else {
                                 *to|=(getnextuncompressednibble(&fc,0)<<4);
                                 otoggle^=1;
                                 to++;
                             }
                         } else getnextuncompressednibble(&fc,0);
                     }
                     if(ww>0 && xx1<HRes)copyframetoscreen((unsigned char *)tobuff,xx1, xx1+ww-1, y, y, 0);
                 }
             } else {
                 char *fc=from;
                 char tobuff[w/2], *to;
                 getnextuncompressednibble(&fc,1); //reset the decoder
                 for(int y=y1;y<y1+h;y++){
                     int x=x1;
                     while(1){
                         to=tobuff;
                         int otoggle=0;
                         char c;
                         int ww=0;
                         int xx=-1;
                         while((c=getnextuncompressednibble(&fc,0))==blank){
                             x++;
                             if(x==x1+w)break;
                         }
                         if(x==x1+w)break; //nothing found so exit
                         *to=c;
                         otoggle ^=1;
                         xx=x;
                         x++;
                         ww=1;
                         if(xx!=x1+w-1){
                             while((c=getnextuncompressednibble(&fc,0))!=blank){
                                 x++;
                                 ww++;
                                 if(otoggle==0){
                                     *to=c;
                                     otoggle ^=1;
                                 } else {
                                     *to|=(c<<4);
                                     otoggle^=1;
                                     to++;
                                 }
                                 if(x==x1+w)break;
                             }
                         }
                         x++;
                         if(xx+ww>HRes){
                             ww=HRes-xx;
                         }
                         if(xx>=0 && ww>0 && y>=0 && y<VRes)copyframetoscreen((unsigned char *)tobuff,xx, xx+ww-1, y, y, 0);
                         if(xx<0 && xx+ww>=0){
                             char *t=tobuff-(xx/2)-(xx&1);
                             ww+=xx;
                             if(ww>0)copyframetoscreen((unsigned char *)t,0, ww-1, y, y, xx&1);
                         }
                         if(x>=x1+w)break;
                    }
                 }
             }
        }
        return 1;
    }
    return 0;
}

void cmd_blit(void) {
    int x1, y1, x2, y2, w, h, bnbr;
    int new=0;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
    char *p, *q;
    if(blitother())return;
    if((p = checkstring(cmdline, "SHOW SAFE"))) {
		int layer;
		getargs(&p, 11,",");
		if(!(argc ==7 || argc==9 || argc==11)) error("Syntax");
		if(hideall)error("Sprites are hidden");
		if(*argv[0] == '#') argv[0]++;
		bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
		if(blitbuff[bnbr].blitbuffptr!=NULL){
			x1 = getint(argv[2],-blitbuff[bnbr].w+1,maxW-1);
			y1 = getint(argv[4],-blitbuff[bnbr].h+1,maxH-1);
			layer=getint(argv[6],0,MAXLAYER);
			if(argc>=9 && *argv[8])blitbuff[bnbr].rotation=getint(argv[8],0,3);
			else blitbuff[bnbr].rotation = 0;
			if(argc==11 && *argv[10]){
				new=getint(argv[10],0,1);
			}
			q=blitbuff[bnbr].blitbuffptr;
			w=blitbuff[bnbr].w;
			h=blitbuff[bnbr].h;
		    int cursorhidden=0;
		    if(cursoron){
				hidecursor(0);
				cursorhidden=1;
		    }
			if(blitbuff[bnbr].active){
				if(new){
					hidesafe(bnbr);
					blitbuff[bnbr].layer=layer;
					layer_in_use[blitbuff[bnbr].layer]++;
					if(blitbuff[bnbr].layer==0) zeroLIFOadd(bnbr);
					else LIFOadd(bnbr);
					sprites_in_use++;
					BlitShowBuffer(bnbr, x1, y1, 1);
				} else {
					showsafe(bnbr,x1, y1);
				}
			} else {
				blitbuff[bnbr].layer=layer;
				layer_in_use[blitbuff[bnbr].layer]++;
				if(blitbuff[bnbr].layer==0) zeroLIFOadd(bnbr);
				else LIFOadd(bnbr);
				sprites_in_use++;
				BlitShowBuffer(bnbr, x1, y1, 1);
			}
		    if(cursorhidden)showcursor(0, xcursor,ycursor);
			ProcessCollisions(bnbr);
	        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
		} else error("Buffer not in use");
    } else if((p = checkstring(cmdline, "SHOW"))) {
		int layer;
		getargs(&p, 9,",");
		if(!(argc ==7 || argc==9)) error("Syntax");
		if(hideall)error("Sprites are hidden");
		if(*argv[0] == '#') argv[0]++;
		bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
		if(blitbuff[bnbr].blitbuffptr!=NULL){
			x1 = getint(argv[2],-blitbuff[bnbr].w+1,maxW-1);
			y1 = getint(argv[4],-blitbuff[bnbr].h+1,maxH-1);
			layer=getint(argv[6],0,MAXLAYER);
			if(argc==9)blitbuff[bnbr].rotation=getint(argv[8],0,3);
			else blitbuff[bnbr].rotation = 0;
			q=blitbuff[bnbr].blitbuffptr;
			w=blitbuff[bnbr].w;
			h=blitbuff[bnbr].h;
			if(blitbuff[bnbr].active){
				layer_in_use[blitbuff[bnbr].layer]--;
				if(blitbuff[bnbr].layer==0)zeroLIFOremove(bnbr);
				else LIFOremove(bnbr);
				sprites_in_use--;
			}
			blitbuff[bnbr].layer=layer;
			layer_in_use[blitbuff[bnbr].layer]++;
			if(blitbuff[bnbr].layer==0) zeroLIFOadd(bnbr);
			else LIFOadd(bnbr);
			sprites_in_use++;
		    int cursorhidden=0;
		    if(cursoron){
				hidecursor(0);
				cursorhidden=1;
		    }
			BlitShowBuffer(bnbr, x1, y1, 1);
		    if(cursorhidden)showcursor(0, xcursor,ycursor);
			ProcessCollisions(bnbr);
	        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
		} else error("Buffer not in use");
    } else if((p = checkstring(cmdline, "HIDE ALL"))) {
		if(hideall)error("Sprites are hidden");
    	int i;
	    int cursorhidden=0;
	    if(cursoron){
			hidecursor(0);
			cursorhidden=1;
	    }
        for(i=LIFOpointer-1; i>= 0; i--) {
            blithide(LIFO[i],0);
        }
        for(i=zeroLIFOpointer-1; i>= 0; i--){
            blithide(zeroLIFO[i],0);
        }
	    if(cursorhidden)showcursor(0, xcursor,ycursor);
        hideall=1;
    } else if((p = checkstring(cmdline, "RESTORE"))) {
		if(!hideall)error("Sprites are not hidden");
    	int i;
	    int cursorhidden=0;
	    if(cursoron){
			hidecursor(0);
			cursorhidden=1;
	    }
        for(i=0; i< zeroLIFOpointer; i++){
        	BlitShowBuffer(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        }
        for(i=0; i< LIFOpointer; i++){
        	    if(blitbuff[LIFO[i]].next_x != 10000){
                blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                blitbuff[LIFO[i]].next_x = 10000;
            }
            if(blitbuff[LIFO[i]].next_y != 10000){
                blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                blitbuff[LIFO[i]].next_y = 10000;
            }
            BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
        }
	    if(cursorhidden)showcursor(0, xcursor,ycursor);
        hideall=0;
        ProcessCollisions(0);
    } else if((p = checkstring(cmdline, "HIDE SAFE"))) {
        getargs(&p, 1,",");
        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
        if(argc !=1) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
		if(hideall)error("Sprites are hidden");
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            if(blitbuff[bnbr].active){
				int cursorhidden=0;
				if(cursoron){
					hidecursor(0);
					cursorhidden=1;
				}
				hidesafe(bnbr);
				if(cursorhidden)showcursor(0, xcursor,ycursor);
				if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
            } else error("Not Showing");
        } else error("Buffer not in use");
    } else if((p = checkstring(cmdline, "HIDE"))) {
        getargs(&p, 1,",");
        if(argc !=1) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            if(blitbuff[bnbr].active){
                sprites_in_use--;
        	    int cursorhidden=0;
        	    if(cursoron){
        			hidecursor(0);
        			cursorhidden=1;
        	    }
                blithide(bnbr, 0);
        	    if(cursorhidden)showcursor(0, xcursor,ycursor);
                layer_in_use[blitbuff[bnbr].layer]--;
                blitbuff[bnbr].x=10000;
                blitbuff[bnbr].y=10000;
                if(blitbuff[bnbr].layer==0)zeroLIFOremove(bnbr);
                else LIFOremove(bnbr);
                blitbuff[bnbr].layer=-1;
                blitbuff[bnbr].next_x = 10000;
                blitbuff[bnbr].next_y = 10000;
        		blitbuff[bnbr].lastcollisions=0;
                blitbuff[bnbr].edges=0;
            } else error("Not Showing");
        } else error("Buffer not in use");
        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
//
    } else if((p = checkstring(cmdline, "SWAP"))) {
        int rbnbr;
        getargs(&p, 5,",");
        if(argc < 3) error("Syntax");
		if(hideall)error("Sprites are hidden");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        if(*argv[2] == '#') argv[0]++;
        rbnbr = getint(argv[2],1,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr==NULL || blitbuff[bnbr].active==false) error("Original buffer not displayed");
        if(!blitbuff[bnbr].active)error("Original buffer not displayed");
        if(blitbuff[rbnbr].active) error("New buffer already displayed");
        if(!(blitbuff[rbnbr].w ==blitbuff[bnbr].w && blitbuff[rbnbr].h ==blitbuff[bnbr].h)) error("Size mismatch");
// copy the relevant data
        blitbuff[rbnbr].blitstoreptr=blitbuff[bnbr].blitstoreptr;
        blitbuff[rbnbr].x=blitbuff[bnbr].x;
        blitbuff[rbnbr].y=blitbuff[bnbr].y;
        blitbuff[rbnbr].layer=blitbuff[bnbr].layer;
        blitbuff[rbnbr].lastcollisions=blitbuff[bnbr].lastcollisions;
        if(blitbuff[rbnbr].layer==0)zeroLIFOswap(bnbr,rbnbr);
        else LIFOswap(bnbr,rbnbr);
// "Hide" the old sprite
        blitbuff[bnbr].x=10000;
        blitbuff[bnbr].y=10000;
        blitbuff[bnbr].layer=-1;
        blitbuff[bnbr].next_x = 10000;
        blitbuff[bnbr].next_y = 10000;
        blitbuff[bnbr].active=0;
        blitbuff[bnbr].lastcollisions=0;
        if(argc==5)blitbuff[rbnbr].rotation=getint(argv[4],0,3);
        else blitbuff[rbnbr].rotation = 0;
	    int cursorhidden=0;
	    if(cursoron){
			hidecursor(0);
			cursorhidden=1;
	    }
        BlitShowBuffer(rbnbr, blitbuff[rbnbr].x, blitbuff[rbnbr].y, 2);
	    if(cursorhidden)showcursor(0, xcursor,ycursor);
        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");

    } else if((p = checkstring(cmdline, "READ"))) {
        getargs(&p, 11,",");
        if(!(argc == 9 || argc == 11)) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        x1 = getinteger(argv[2]);
        y1 = getinteger(argv[4]);
        w = getinteger(argv[6]);
        h = getinteger(argv[8]);
        if(w < 1 || h < 1) return;
        blitbuff[bnbr].bc=0;
        if(blitbuff[bnbr].blitbuffptr==NULL){
            blitbuff[bnbr].blitbuffptr = GetMemory(w*h*PageTable[WritePage].nbytes);
            blitbuff[bnbr].blitstoreptr = GetMemory(w*h*PageTable[WritePage].nbytes);
            blitbuff[bnbr].bc=0;
            blitbuff[bnbr].w=w;
            blitbuff[bnbr].h=h;
            blitbuff[bnbr].master=0;
            blitbuff[bnbr].mymaster=-1;
            blitbuff[bnbr].x=10000;
            blitbuff[bnbr].y=10000;
            blitbuff[bnbr].layer=-1;
            blitbuff[bnbr].next_x = 10000;
            blitbuff[bnbr].next_y = 10000;
    		blitbuff[bnbr].active=false;
    		blitbuff[bnbr].lastcollisions=0;
            blitbuff[bnbr].edges=0;
    		q=blitbuff[bnbr].blitbuffptr;
        } else {
            if(blitbuff[bnbr].mymaster != -1) error("Can't read into a copy", bnbr);
            if(blitbuff[bnbr].master >0) error("Copies exist", bnbr);
            if(!(blitbuff[bnbr].w==w && blitbuff[bnbr].h==h))error("Existing buffer is incorrect size");
            q=blitbuff[bnbr].blitbuffptr;
        } 
        if(argc==11){
        	if(checkstring(argv[10], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[10],0,LastPage);
        	}
        }
	    int cursorhidden=0;
	    if(cursoron){
			hidecursor(0);
			cursorhidden=1;
	    }
        ReadBufferFast(x1,y1,x1+w-1,y1+h-1,q);
	    if(cursorhidden)showcursor(0, xcursor,ycursor);
        ReadPage=WritePage;
    } else if((p = checkstring(cmdline, "COPY"))) {
        int cpy,nbr,c1,n1;
        getargs(&p, 5,",");
        if(argc !=5) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            if(*argv[2] == '#') argv[2]++;
            c1 = cpy = getint(argv[2], 1, MAXBLITBUF-1);
            n1 = nbr = getint(argv[4], 1, MAXBLITBUF-2);

            while(n1) {
                if(blitbuff[c1].blitbuffptr!=NULL)error("Buffer already in use %",c1);
                if(blitbuff[bnbr].master==-1)error("Can't copy a copy");;
                n1--;
                c1++;
            }
            while(nbr) {
                blitbuff[cpy].blitbuffptr=blitbuff[bnbr].blitbuffptr;
                blitbuff[cpy].w=blitbuff[bnbr].w;
                blitbuff[cpy].h=blitbuff[bnbr].h;
                blitbuff[cpy].blitstoreptr = GetMemory(blitbuff[cpy].w * blitbuff[cpy].h * PageTable[WritePage].nbytes);
                blitbuff[cpy].bc=blitbuff[bnbr].bc;
                blitbuff[cpy].x=10000;
                blitbuff[cpy].y=10000;
                blitbuff[cpy].next_x = 10000;
                blitbuff[cpy].next_y = 10000;
                blitbuff[cpy].layer=-1;
                blitbuff[cpy].mymaster=bnbr;
                blitbuff[cpy].master=-1;
                blitbuff[cpy].edges=0;
                blitbuff[bnbr].master |= (1<<cpy);
        		blitbuff[bnbr].lastcollisions=0;
    			blitbuff[cpy].active=false;
                nbr--;
                cpy++;
            }
        } else error("Buffer not in use");

    } else if((p = checkstring(cmdline, "LOADARRAY"))) {
        loadarray(p);

    } else if((p = checkstring(cmdline, "LOADPNG"))) {
        loadpng(p);

    } else if((p = checkstring(cmdline, "LOAD"))) {
        loadsprite(p);

    } else if((p = checkstring(cmdline, "INTERRUPT"))) {
        getargs(&p, 1,",");
        COLLISIONInterrupt = GetIntAddress(argv[0]);					// get the interrupt location
        InterruptUsed = true;
        return;

    } else if((p = checkstring(cmdline, "NOINTERRUPT"))) {
        COLLISIONInterrupt = NULL;					// get the interrupt location
        return;

    } else if((p = checkstring(cmdline, "CLOSE ALL"))) {
        closeallsprites();
        
    } else if((p = checkstring(cmdline, "CLOSE"))) {
        getargs(&p, 1,",");
		if(hideall)error("Sprites are hidden");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);
        if(blitbuff[bnbr].master>0) error("Copies still open");
        if(blitbuff[bnbr].blitbuffptr!=NULL){
     	   if(blitbuff[bnbr].active){
     		   int cursorhidden=0;
     		   if(cursoron){
     			   hidecursor(0);
     			   cursorhidden=1;
     		   }
     		   blithide(bnbr, 1);
     		   if(cursorhidden)showcursor(0, xcursor,ycursor);
     		   if(blitbuff[bnbr].layer==0)zeroLIFOremove(bnbr);
     		   else LIFOremove(bnbr);
     		   layer_in_use[blitbuff[bnbr].layer]--;
     		   sprites_in_use--;
     	   }
           if(blitbuff[bnbr].mymaster==-1)FreeMemorySafe((void *)&blitbuff[bnbr].blitbuffptr);
           else blitbuff[blitbuff[bnbr].mymaster].master &= ~(1<<bnbr);
   		    FreeMemorySafe((void *)&blitbuff[bnbr].blitstoreptr);
   	        blitbuff[bnbr].blitbuffptr = NULL;
   	        blitbuff[bnbr].blitstoreptr = NULL;
   	        blitbuff[bnbr].master=-1;
   	        blitbuff[bnbr].mymaster=-1;
   	        blitbuff[bnbr].x=10000;
   	        blitbuff[bnbr].y=10000;
   	        blitbuff[bnbr].w=0;
   	        blitbuff[bnbr].h=0;
   	        blitbuff[bnbr].next_x = 10000;
   	        blitbuff[bnbr].next_y = 10000;
   	        blitbuff[bnbr].bc=0;
   	        blitbuff[bnbr].layer=-1;
   			blitbuff[bnbr].active=false;
   	        blitbuff[bnbr].edges=0;
        } else error("Buffer not in use");
        if(sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error("sprite internal error");
    } else if((p = checkstring(cmdline, "TRANSPARENCY"))) {
    	int x,y, trans;
        getargs(&p, 3,",");
        if(!(VideoColour==12))error("Invalid for this display mode");
        if(argc !=3) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        trans=getint(argv[2],1,15);
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            w=blitbuff[bnbr].w;
            h=blitbuff[bnbr].h;
            if(VideoColour==12){
            	uint16_t *q=(uint16_t *)blitbuff[bnbr].blitbuffptr;
            	for(y=0;y<h;y++){
            		for(x=0;x<w;x++){
            			if(*q & 0xF000){
            				*q &= 0xFFF;
            				*q |= (trans<<12);
            			}
            			q++;
            		}
            	}
            } else {
            	uint8_t *q=(uint8_t *)blitbuff[bnbr].blitbuffptr;
            	for(y=0;y<h;y++){
            		for(x=0;x<w;x++){
            			if(*q & 0xF0){
            				*q &= 0xF;
            				*q |= (trans<<4);
            			}
            			q++;
            		}
            	}
            }

        } else error("Buffer not in use");

    } else if((p = checkstring(cmdline, "NEXT"))) {
        getargs(&p, 5,",");
        if(!(argc ==5)) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        blitbuff[bnbr].next_x = getint(argv[2],-blitbuff[bnbr].w+1,maxW-1);
        blitbuff[bnbr].next_y = getint(argv[4],-blitbuff[bnbr].h+1,maxH-1);
//
    } else if((p = checkstring(cmdline, "WRITE"))) {
    	int mode=4;
        getargs(&p, 7,",");
        if(!(argc ==5 || argc==7)) error("Syntax");
        if(*argv[0] == '#') argv[0]++;
        bnbr = getint(argv[0],1,MAXBLITBUF-1);									// get the number
        if(blitbuff[bnbr].blitbuffptr!=NULL){
            x1 = getint(argv[2], -blitbuff[bnbr].w+1, maxW);
            y1 = getint(argv[4], -blitbuff[bnbr].h+1, maxH);
            if(argc==7)blitbuff[bnbr].rotation=getint(argv[6],0,7);
            else blitbuff[bnbr].rotation = 4;
            if((blitbuff[bnbr].rotation & 4) ==0 )mode |=8;
            blitbuff[bnbr].rotation &= 3;
            q=blitbuff[bnbr].blitbuffptr;
            w=blitbuff[bnbr].w;
            h=blitbuff[bnbr].h;
  		    int cursorhidden=0;
  		    if(cursoron){
  			   hidecursor(0);
  			   cursorhidden=1;
  		    }
            BlitShowBuffer(bnbr, x1, y1, mode);
  		    if(cursorhidden)showcursor(0, xcursor,ycursor);
        } else error("Buffer not in use");
    } else if((p = checkstring(cmdline, "MOVE"))) {
		if(hideall)error("Sprites are hidden");
        int i;
		int cursorhidden=0;
		if(cursoron){
		   hidecursor(0);
		   cursorhidden=1;
		}
        for(i=LIFOpointer-1; i>= 0; i--) blithide(LIFO[i],0);
        for(i=zeroLIFOpointer-1; i>= 0; i--)blithide(zeroLIFO[i],0);
//
        for(i=0; i< zeroLIFOpointer; i++){
            if(blitbuff[zeroLIFO[i]].next_x != 10000){
                blitbuff[zeroLIFO[i]].x=blitbuff[zeroLIFO[i]].next_x;
                blitbuff[zeroLIFO[i]].next_x = 10000;
            }
            if(blitbuff[zeroLIFO[i]].next_y != 10000){
                blitbuff[zeroLIFO[i]].y=blitbuff[zeroLIFO[i]].next_y;
                blitbuff[zeroLIFO[i]].next_y = 10000;
            }
            BlitShowBuffer(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        }
        for(i=0; i< LIFOpointer; i++){
            if(blitbuff[LIFO[i]].next_x != 10000){
                blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                blitbuff[LIFO[i]].next_x = 10000;
            }
            if(blitbuff[LIFO[i]].next_y != 10000){
                blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                blitbuff[LIFO[i]].next_y = 10000;
            }
            BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);

        }
		if(cursorhidden)showcursor(0, xcursor,ycursor);
        ProcessCollisions(0);
    } else if((p = checkstring(cmdline, "SCROLLR"))) {
        int i,xmin,xmax,ymin,ymax, blank=-1, sx=0, sy=0, b1, b2, xs=10000, ys=10000;
    	int flip=0;
        char *current3=NULL, *current4=NULL;
        getargs(&p, 13,",");
		if(hideall)error("Sprites are hidden");
        if(!(argc ==11 || (argc==13))) error("Syntax");
        x1 = getint(argv[0],0,maxW-2);
        y1 = getint(argv[2],0,maxH-2);
        w = getint(argv[4],1,maxW-x1);
        h = getint(argv[6],1,maxH-y1);
        sx = getint(argv[8],1-w, w-1);
        sy = getint(argv[10],1-h, h-1);
        if(sx>=0){ //convert coordinates for use with BLIT
            x2=x1+sx;
            w-=sx;
        } else {
            x2=x1;
            x1-=sx; //sx is negative so this subtracts it
            w+=sx;
        }
        if(sy<0){ //convert coordinates for use with BLIT
            y2=y1-sy;
            h+=sy;
        } else {
            y2=y1;
            y1+=sy; 
            h-=sy;
        }
        if(argc==13)blank=getColour(argv[12], 1);
        xmin=min(x1,x2);
        xmax=max(x1,x2)+w-1;
        ymin=min(y1,y2);
        ymax=max(y1,y2)+h-1;
        if(w < 1 || h < 1) return;
        if(x1 < 0) { x2 -= x1; w += x1; x1 = 0; }
        if(x2 < 0) { x1 -= x2; w += x2; x2 = 0; }
        if(y1 < 0) { y2 -= y1; h += y1; y1 = 0; }
        if(y2 < 0) { y1 -= y2; h += y2; y2 = 0; }
        if(x1 + w > maxW) w = maxW - x1;
        if(x2 + w > maxW) w = maxW - x2;
        if(y1 + h > maxH) h = maxH - y1;
        if(y2 + h > maxH) h = maxH - y2;
        if(w < 1 || h < 1 || x1 < 0 || x1 + w > maxW || x2 < 0 || x2 + w > maxW || y1 < 0 || y1 + h > maxH || y2 < 0 || y2 + h > maxH) return;

        b1=RoundUptoInt(abs(sx))*ymax;
        b2=RoundUptoInt(xmax)*abs(sy);
		if(blank<0){
            if(x1!=x2)current3=GetMemory(b1*PageTable[WritePage].nbytes);
            if(y1!=y2)current4=GetMemory(b2*PageTable[WritePage].nbytes);
        }
		int cursorhidden=0;
		if(cursoron){
		   hidecursor(0);
		   cursorhidden=1;
		}

        for(i=LIFOpointer-1; i>= 0; i--) {
            blithide(LIFO[i],0);

        }
            
        for(i=zeroLIFOpointer-1; i>= 0; i--){
            xs=blitbuff[zeroLIFO[i]].x + (blitbuff[zeroLIFO[i]].w >> 1);
            ys=blitbuff[zeroLIFO[i]].y + (blitbuff[zeroLIFO[i]].h >> 1);
            blithide(zeroLIFO[i],0);
            if((xs <= xmax) && (xs  >= xmin) && (ys <= ymax) && (ys >= ymin)){
                xs+=(x2-x1);
                if(xs>=xmax)xs -=  (xmax-xmin+1);
                if(xs < xmin)xs += (xmax-xmin+1);
                blitbuff[zeroLIFO[i]].x=xs-(blitbuff[zeroLIFO[i]].w >> 1);
                ys+=(y2-y1);
                if(ys>=ymax)ys -=  (ymax-ymin+1);
                if(ys < ymin)ys += (ymax-ymin+1);
                blitbuff[zeroLIFO[i]].y=ys-(blitbuff[zeroLIFO[i]].h >> 1);
            }
        }
        if(blank==-1){
            if(x1!=x2){
                if(sx>0){
                    ReadBufferFast(xmax-sx+1, ymin, xmax, ymax, current3) ;
                } else {
                    ReadBufferFast(xmin, ymin, xmin-sx-1, ymax,  current3) ;
                }
            }
            if(y1!=y2){
                if(sy>0){
                    ReadBufferFast(xmin, ymin, xmax, ymin-1+sy, current4);
                } else {
                    ReadBufferFast(xmin, ymax+sy+1, xmax, ymax, current4);
                }
            }
        }
    	int copymode=PageTable[ReadPage].expand | (PageTable[WritePage].expand<<1);
    	switch(copymode){
    	case 0:
    		MoveBufferNormal( x1, y1, x2, y2, w, h, flip);
    		break;
    	case 1:
        	MoveBufferContract( x1, y1, x2, y2, w, h, flip);
    		break;
    	case 2:
        	MoveBufferExpand( x1, y1, x2, y2, w, h, flip);
    		break;
    	case 3:
        	MoveBufferDup( x1, y1, x2, y2, w, h, flip);
    	}
        if(x1!=x2){
            if(blank!=-1){
                if(sx>0){
                    DrawRectangle(xmin, ymin, xmin+sx-1, ymax,  blank) ;
                } else {
                    DrawRectangle(xmax+sx+1, ymin, xmax, ymax, blank) ;
                }
            } else {
                if(sx>0){
                    DrawBufferFast(xmin, ymin, xmin+sx-1, ymax,  current3) ;
                } else {
                    DrawBufferFast(xmax+sx+1, ymin, xmax, ymax, current3) ;
                }
            }
        }
        if(y1!=y2){
            if(blank!=-1){
                if(sy>0){
                    DrawRectangle(xmin, ymax-sy+1, xmax, ymax, blank);
                } else {
                    DrawRectangle(xmin, ymin, xmax, ymin-1-sy, blank);
                }
            } else {
                if(sy>0){
                    DrawBufferFast(xmin, ymax-sy+1, xmax, ymax, current4);
                } else {
                    DrawBufferFast(xmin, ymin, xmax, ymin-1-sy, current4);
                }
            }
        }
		FreeMemorySafe((void*)&current3);
		FreeMemorySafe((void*)&current4);
        for(i=0; i< zeroLIFOpointer; i++){
        	BlitShowBuffer(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
        }
        for(i=0; i< LIFOpointer; i++){
        	    if(blitbuff[LIFO[i]].next_x != 10000){
                blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                blitbuff[LIFO[i]].next_x = 10000;
            }
            if(blitbuff[LIFO[i]].next_y != 10000){
                blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                blitbuff[LIFO[i]].next_y = 10000;
            }
            BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
        }
		if(cursorhidden)showcursor(0, xcursor,ycursor);
        ProcessCollisions(0);
    } else if((p = checkstring(cmdline, "SCROLL"))) {
        int i, n, m=0, blank=-2,x,y;
        char *current=NULL;
        getargs(&p, 5,",");
		if(hideall)error("Sprites are hidden");
        x = getint(argv[0],-maxW/2-1, maxW);
        y = getint(argv[2],-maxH/2-1, maxH);
        if(argc==5)blank=getColour(argv[2], 1);
        if(!(x==0 && y==0)){
    		int cursorhidden=0;
    		if(cursoron){
    		   hidecursor(0);
    		   cursorhidden=1;
    		}
        	m=((maxW*(y>0?y:-y))*PageTable[WritePage].nbytes);
        	n=((maxH*(x>0?x:-x))*PageTable[WritePage].nbytes);
        	if(n>m)m=n;
    		if(blank==-2)current=GetMemory(m);
            for(i=LIFOpointer-1; i>= 0; i--) blithide(LIFO[i],0);
            for(i=zeroLIFOpointer-1; i>= 0; i--){
                    int xs=blitbuff[zeroLIFO[i]].x+ (blitbuff[zeroLIFO[i]].w >> 1);
                    int ys=blitbuff[zeroLIFO[i]].y+ (blitbuff[zeroLIFO[i]].h >> 1);
                    blithide(zeroLIFO[i],0);
                    xs+=x;
                    if(xs>=maxW)xs-=maxW;
                    if(xs < 0)xs+=maxW;
                    blitbuff[zeroLIFO[i]].x=xs-(blitbuff[zeroLIFO[i]].w >> 1);
                    ys-=y;
                    if(ys>=maxH)ys-=maxH;
                    if(ys < 0)ys+=maxH;
                    blitbuff[zeroLIFO[i]].y=ys-(blitbuff[zeroLIFO[i]].h >> 1);
            }
            if(x>0){
                if(blank==-2)ReadBufferFast(maxW-x,0,maxW-1,maxH-1,current);
                ScrollBufferH(x);
                if(blank==-2)DrawBufferFast(0,0,x-1,maxH-1,current);
                else if(blank!=-1)DrawRectangle(0, 0, x-1, maxH-1,  blank) ;
            } else if(x<0){
                x=-x;
                if(blank==-2)ReadBufferFast(0,0,x-1,maxH-1,current);
                ScrollBufferH(-x);
                if(blank==-2)DrawBufferFast(maxW-x,0,maxW-1,maxH-1, current);
                else if(blank!=-1)DrawRectangle(maxW-x,0,maxW-1,maxH-1, blank);
    		}
    		if(y>0){
    	        if(blank==-2)ReadBufferFast(0,0,maxW-1,y-1,current);
    	        ScrollBufferV(y, 0);
    	        if(blank==-2)DrawBufferFast(0,maxH-y,maxW-1,maxH-1,current);
    	        else if(blank!=-1)DrawRectangle(0,maxH-y,maxW-1,maxH-1, blank) ;
    	    } else if(y<0){
    	        y=-y;
    	        if(blank==-2)ReadBufferFast(0,maxH-y,maxW-1,maxH-1,current);
    	        ScrollBufferV(-y, 0 );
    	        if(blank==-2)DrawBufferFast(0,0,maxW-1,y-1,current);
    	        else if(blank!=-1)DrawRectangle(0,0,maxW-1,y-1, blank) ;
    	    }
			for(i=0; i< zeroLIFOpointer; i++){
            	BlitShowBuffer(zeroLIFO[i], blitbuff[zeroLIFO[i]].x, blitbuff[zeroLIFO[i]].y, 0);
             }
            for(i=0; i< LIFOpointer; i++){
                if(blitbuff[LIFO[i]].next_x != 10000){
                    blitbuff[LIFO[i]].x=blitbuff[LIFO[i]].next_x;
                    blitbuff[LIFO[i]].next_x = 10000;
                }
                if(blitbuff[LIFO[i]].next_y != 10000){
                    blitbuff[LIFO[i]].y=blitbuff[LIFO[i]].next_y;
                    blitbuff[LIFO[i]].next_y = 10000;
                }

                BlitShowBuffer(LIFO[i], blitbuff[LIFO[i]].x, blitbuff[LIFO[i]].y, 0);
             }
    		if(cursorhidden)showcursor(0, xcursor,ycursor);
            ProcessCollisions(0);
            if(current)FreeMemory(current);
        }
     } else {
    	int flip=0;
        getargs(&cmdline,15,",");
        if(argc < 11) error("Syntax");
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        x2 = getinteger(argv[4]);
        y2 = getinteger(argv[6]);
        w = getinteger(argv[8]);
        h = getinteger(argv[10]);
        if(argc>=13 && *argv[12]){
        	if(checkstring(argv[12], "FRAMEBUFFER")){
        		ReadPage=WPN;
        	} else {
				ReadPage=getint(argv[12],0,LastPage);
        	}
        }
        int readlimx=PageTable[ReadPage].xmax;
        int readlimy=PageTable[ReadPage].ymax;
        if(argc==15)flip=getint(argv[14],0,7);
        if(w < 1 || h < 1) {
            ReadPage=WritePage;
        	return;
        }
        if(x1 < 0) {
        	x2 -= x1;
        	w += x1;
        	x1 = 0;
        }
        if(x2 < 0) {
        	if(!(flip & 1))x1 -= x2;
        	w += x2;
        	x2 = 0;
        }
        if(y1 < 0) {
        	y2 -= y1;
        	h += y1;
        	y1 = 0;
        }
        if(y2 < 0) {
        	if(!(flip & 2))y1 -= y2;
        	h += y2;
        	y2 = 0;
        }
        if(x1 + w > readlimx) {
        	w = readlimx - x1;
        }
        if(x2 + w > maxW) {
        	if(flip & 1) x1 += x2 + w - maxW;
        	w = maxW - x2;
        }
        if(y1 + h > readlimy) {
        	h = readlimy - y1;
        }
        if(y2 + h > maxH) {
        	if(flip & 2) y1 += y2 + h - maxH;
        	h = maxH - y2;
        }
        if(w < 1 || h < 1 || x1 < 0 || x1 + w > readlimx || x2 < 0 || x2 + w > maxW || y1 < 0 || y1 + h > readlimy || y2 < 0 || y2 + h > maxH) {
            ReadPage=WritePage;
        	return;
        }
        if(VideoColour==8 && (x1 & 3) == 0 && (x2 & 3) == 0 && (w & 3)==0){
			docopy=zcopy;
        } else if(VideoColour !=8 && (x1 & 1) == 0 && (x2 & 1) == 0 && (w & 1)==0){
			docopy=zcopy;
        } else if(VideoColour==32) docopy=zcopy;
    	int copymode=PageTable[ReadPage].expand | (PageTable[WritePage].expand<<1);
    	switch(copymode){
    	case 0:
//        	MMPrintString("MoveBufferNormal\r\n");
    		MoveBufferNormal( x1, y1, x2, y2, w, h, flip);
    		break;
    	case 1:
//        	MMPrintString("MoveBufferContract\r\n");
        	MoveBufferContract( x1, y1, x2, y2, w, h, flip);
    		break;
    	case 2:
//        	MMPrintString("MoveBufferExpand\r\n");
        	MoveBufferExpand( x1, y1, x2, y2, w, h, flip);

    		break;
    	case 3:
//        	MMPrintString("MoveBufferDup\r\n");
        	MoveBufferDup( x1, y1, x2, y2, w, h, flip);
    	}
        ReadPage=WritePage;
        docopy=mycopy;
    }
}

