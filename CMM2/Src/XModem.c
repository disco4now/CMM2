/***************************************************************************
CMM2 MMBasic
xmodem.c

Implements the xmodem command and protocol for the MMBasic.
This includes the ability to send and receive a file directly from the SD card.

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

#include <stdio.h>

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"

void xmodemTransmit(char *p, int fnbr, int commport);
void xmodemReceive(char *sp, int maxbytes, int fnbr, int crunch, int commport);
int FindFreeFileNbr(void);

void cmd_xmodem(void) {
	char BreakKeySave;
    int rcv = 0, fnbr;
    int commport=0;
	char *fname;
    if(toupper(*cmdline) == 'R')
        rcv = true;
    else if(toupper(*cmdline) == 'S')
        rcv = false;
    else
        error("Syntax");
    while(IsAlpha(*cmdline)) cmdline++ ;                           // find the filename (if it is there)
    skipspace(cmdline);

    BreakKeySave = BreakKey;
    BreakKey = 0;
        
    if((*cmdline == 0 || *cmdline == '\'')) {
    	error("Syntax");
    } else {
        // this is a transfer to/from the SD card
        if(!InitSDCard()) return;
        fnbr = FindFreeFileNbr();
        getargs(&cmdline,3,",");
        fname = getFstring(argv[0]);                               // get the file name
        if(argc==3){
        	commport=getint(argv[2],1,3);
        	if(commport==1 && com1 !=true)error("COM1 not open");
        	if(commport==2 && com2 !=true)error("COM2 not open");
           	if(commport==3 && com3 !=true)error("COM3 not open");
        }
    	if(!(OptionConsole & 1) && commport==0)error("Serial Console Disabled");
        if(rcv) {
            if(!BasicFileOpen(fname, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
            xmodemReceive(NULL, 0, fnbr, false, commport);
        } else {
            if(!BasicFileOpen(fname, fnbr, FA_READ)) return;
            xmodemTransmit(NULL, fnbr, commport);
        }
        FileClose(fnbr);
    }
    BreakKey = BreakKeySave;
}


int _inbyte(int timeout, int commport) {
	int c;
	
	PauseTimer = 0;
	while(PauseTimer < timeout) {
		c = (commport==0 ? getConsole() : SerialGetchar(commport));
		if(c != -1) {
			return c;
		}
		routinechecks(1);
	}
	return -1;	
}

void XmodemPutC(int commport, char c){
	if(commport==0) SerUSBPutC(c);
	else SerialPutchar(commport,c);
}
// for the MX470 we don't want any XModem data echoed to the LCD panel


/***********************************************************************************************
the xmodem protocol
************************************************************************************************/

/* derived from the work of Georges Menie (www.menie.org) Copyright 2001-2010 Georges Menie
 * very much debugged and changed
 *
 * this is just the basic XModem protocol (no 1K blocks, crc, etc).  It has been tested on
 * Terra Term and is intended for use with that software.
 */


#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define PAD  0x1a

#define DLY_1S 1000
#define MAXRETRANS 25

#define X_BLOCK_SIZE	128
#define X_BUF_SIZE	X_BLOCK_SIZE + 6								// 128 for XModem + 3 head chars + 2 crc + nul


static int check(const unsigned char *buf, int sz)
{
	int i;
	unsigned char cks = 0;
	for (i = 0; i < sz; ++i) {
		cks += buf[i];
	}
	if (cks == buf[sz])
		return 1;

	return 0;
}


static void flushinput(int commport)
{
	while (_inbyte(((DLY_1S)*3)>>1,commport) >= 0){
		routinechecks(1);
	}
}


// receive data
// if sp == NULL we are saving to a file on the SD card (fnbr is the file number)
// otherwise we are saving to RAM which will later be written to program memory
void xmodemReceive(char *sp, int maxbytes, int fnbr, int crunch, int commport) {
    unsigned char xbuff[X_BUF_SIZE];
    unsigned char *p;
    unsigned char trychar = NAK; //'C';
    unsigned char packetno = 1;
    int i, c;
    int retry, retrans = MAXRETRANS;

    CrunchData(&sp, 0);                                         // initialise the crunch subroutine

        // first establish communication with the remote
    while(1) {
        for( retry = 0; retry < 32; ++retry) {
            if(trychar) XmodemPutC(commport,trychar);
            if ((c = _inbyte((DLY_1S)<<1,commport)) >= 0) {
                switch (c) {
                case SOH:
                    goto start_recv;
                case EOT:
                    flushinput(commport);
                    XmodemPutC(commport,ACK);
                    if(sp != NULL) {
                        if(maxbytes <= 0) error("Not enough memory");
                        *sp++ = 0;                                  // terminate the data
                    }
                    return;                                         // no more data
                case CAN:
                    flushinput(commport);
                    XmodemPutC(commport,ACK);
                    error("Cancelled by remote");
                    break;
                default:
                    break;
                }
            }
        }
        flushinput(commport);
        XmodemPutC(commport,CAN);;
        XmodemPutC(commport,CAN);;
        XmodemPutC(commport,CAN);;
        error("Remote did not respond");                            // no sync

    start_recv:
        trychar = 0;
        p = xbuff;
        *p++ = SOH;
        for (i = 0;  i < (X_BLOCK_SIZE+3); ++i) {
            if ((c = _inbyte(DLY_1S,commport)) < 0) goto reject;
            *p++ = c;
        }
        if (xbuff[1] == (unsigned char)(~xbuff[2]) && (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) && check(&xbuff[3], X_BLOCK_SIZE)) {
            if (xbuff[1] == packetno) {
                for(i = 0 ; i < X_BLOCK_SIZE ; i++) {
                    if(sp != NULL) {
                        // save the data to the RAM buffer
                        if(--maxbytes > 0) {
                            if(xbuff[i + 3] == PAD) continue;
//                            if(xbuff[i + 3] == PAD)
//                                *sp++ = 0;                          // replace any EOF's (used to pad out a block) with NUL
//                            else
                            if(xbuff[i + 3] == 0) continue;
                                if(crunch)
                                    CrunchData(&sp, xbuff[i + 3]);
                                else
                                    *sp++ = xbuff[i + 3];           // saving to a memory buffer
                        }
                    } else {
                        // we are saving to a file
                        FilePutChar(xbuff[i + 3], fnbr);
                    }
                }
                ++packetno;
                retrans = MAXRETRANS+1;
            }
            if (--retrans <= 0) {
                flushinput(commport);
                XmodemPutC(commport,CAN);;
                XmodemPutC(commport,CAN);;
                XmodemPutC(commport,CAN);;
                error("Too many errors");
            }
            XmodemPutC(commport,ACK);
            continue;
        }
    reject:
        flushinput(commport);
        XmodemPutC(commport,NAK);
    }
}


// transmit data
// if p == NULL we are reading the data to be sent from a file on the SD card (fnbr is the file number)
// otherwise we are reading from RAM and p points to the start of the data (which is terminated by a zero char)
void xmodemTransmit(char *p, int fnbr, int commport) {
	unsigned char xbuff[X_BUF_SIZE]; 
	unsigned char packetno = 1;
    char prevchar = 0;
	int i, c, len;
	int retry;

	// first establish communication with the remote
	while(1) {
		for( retry = 0; retry < 32; ++retry) {
			if ((c = _inbyte((DLY_1S)<<1,commport)) >= 0) {
				switch (c) {
				case NAK:											// start sending
					goto start_trans;
				case CAN:
					if ((c = _inbyte(DLY_1S,commport)) == CAN) {
						XmodemPutC(commport,ACK);
						flushinput(commport);
						error("Cancelled by remote");
					}
					break;
				default:
					break;
				}
			}
		}
		XmodemPutC(commport,CAN);;
		XmodemPutC(commport,CAN);;
		XmodemPutC(commport,CAN);;
		flushinput(commport);
		error("Remote did not respond");							// no sync

		// send a packet
		while(1) {
		start_trans:
			mymemset (xbuff, 0, X_BUF_SIZE);							// start with an empty buffer
			
			xbuff[0] = SOH;											// copy the header
			xbuff[1] = packetno;
			xbuff[2] = ~packetno;
			
            if(p != NULL) {
                // our data is in RAM
                for(len = 0; len < 128 && *p; len++) {
                    if(*p == '\n' && prevchar != '\r')
                        prevchar = xbuff[len + 3] = '\r';
                    else
                        prevchar = xbuff[len + 3] = *p++;			// copy the data from memory into the packet
                }
            } else {
                // we get the data from a file
                for(len = 0; len < 128 && !FileEOF(fnbr); len++) {
                    xbuff[len + 3] = FileGetChar(fnbr);				// copy the data from the file into the packet
                }
            }
			if (len > 0) {
				unsigned char ccks = 0;
				for (i = 3; i < X_BLOCK_SIZE+3; ++i) {
					ccks += xbuff[i];
				}
				xbuff[X_BLOCK_SIZE+3] = ccks;
				
				// now send the block
				for (retry = 0; retry < MAXRETRANS && !MMAbort; ++retry) {
					// send the block
					for (i = 0; i < X_BLOCK_SIZE+4 && !MMAbort; ++i) {
						XmodemPutC(commport,xbuff[i]);
					}
					// check the response
					if ((c = _inbyte(DLY_1S,commport)) >= 0 ) {
						switch (c) {
						case ACK:
							++packetno;
							goto start_trans;
						case CAN:									// cancelled by remote
							XmodemPutC(commport,ACK);
							flushinput(commport);
							error("Cancelled by remote");
							break;
						case NAK:									// receiver got a corrupt block
						default:
							break;
						}
					}
				}
				// too many retrys... give up
				XmodemPutC(commport,CAN);;
				XmodemPutC(commport,CAN);;
				XmodemPutC(commport,CAN);;
				flushinput(commport);
				error("Too many errors");
			}
			
			// finished sending - send end of text
			else {
				for (retry = 0; retry < 10; ++retry) {
					XmodemPutC(commport,EOT);
					if ((c = _inbyte((DLY_1S)<<1,commport)) == ACK) break;
				}
				flushinput(commport);
				if(c == ACK) return;
				error("Error closing");
			}
		}
	}
}

