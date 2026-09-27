/***************************************************************************
CMM2 MMBasic
Audio.h

Include file that contains the globals and defines for Audio.c in MMBasic.

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

**********************************************************************************/

#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// General definitions used by other modules

#ifndef AUDIO_HEADER
#define AUDIO_HEADER
typedef enum { P_NOTHING, P_PAUSE_TONE, P_TONE, P_PAUSE_SOUND, P_SOUND, P_WAV, P_PAUSE_WAV, P_FLAC, P_MP3, P_MOD, P_PAUSE_MOD, P_PAUSE_FLAC, P_TTS, P_DAC, P_PAUSE_MP3} e_CurrentlyPlaying;
extern volatile e_CurrentlyPlaying CurrentlyPlaying; 
extern volatile e_CurrentlyPlaying CurrentlyPlayinge;
extern int PWM_FREQ;
extern char *sbuff1, *sbuff2;
extern unsigned char *stress; //numbers from 0 to 8
extern unsigned char *phonemeLength; //tab40160
extern unsigned char *phonemeindex;
extern volatile int playreadcomplete;
extern volatile uint64_t bcount[3];
extern unsigned char *phonemeIndexOutput; //tab47296
extern unsigned char *stressOutput; //tab47365
extern unsigned char *phonemeLengthOutput; //tab47416
extern unsigned char speed;
extern unsigned char pitch;
extern unsigned char mouth;
extern unsigned char throat;
extern unsigned char *pitches; // tab43008
extern unsigned char *frequency1;
extern unsigned char *frequency2;
extern unsigned char *frequency3;
extern void Audio_Interrupt(void);
extern unsigned char *amplitude1;
extern unsigned char *amplitude2;
extern unsigned char *amplitude3;
extern volatile int wav_filesize;                                    // head and tail of the ring buffer for com1
extern int trackplaying, trackstoplay;
extern const int mapping[101];

;
typedef struct sa_flist {
    char fn[FF_MAX_LFN];
} a_flist;

extern unsigned char *sampledConsonantFlag; // tab44800
#endif
#endif
