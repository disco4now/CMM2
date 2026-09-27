/***************************************************************************
CMM2 MMBasic
Draw.h

Supporting header file for Draw.c which does the basic LCD display commands and related I/O in MMBasic.


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
  #ifndef DRAW_H_INCL
    #define DRAW_H_INCL

    extern void GUIPrintString(int x, int y, int font, int jh, int jv, int jo, int fc, int bc, char *str);
    extern void GUIPrintChar(int font, int fc, int bc, char c, int jo);
    extern int GetJustification(char *p, int *jh, int *jv, int *jo);
    extern void cmd_guiBasic(void);
    extern void DrawLine(int x1, int y1, int x2, int y2, int w, int c);
    extern void DrawBox(int x1, int y1, int x2, int y2, int w, int c, int fill);
    extern void DrawRBox(int x1, int y1, int x2, int y2, int radius, int c, int fill);
    extern void DrawCircle(int x, int y, int radius, int w, int c, int fill, MMFLOAT aspect);
//    extern void DrawPixel(int x, int y, int c);
    extern void ClearScreen(int c);
    extern void SetFont(int fnt);
    extern void ResetDisplay(void);
    extern int GetFontWidth(int fnt);
    extern int GetFontHeight(int fnt);
    extern int rgb(int r, int g, int b, int t);
    extern void closeall3d(void);
    extern void WindowFrame(int x, int y);
    extern void (*DrawPixel)(int x1, int y1, int c);
    extern void (*DrawRectangle)(int x1, int y1, int x2, int y2, int c);
    extern void (*DrawBitmap)(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
    extern void (*ScrollLCD) (int lines, int blank);
    extern void (*DrawBuffer)(int x1, int y1, int x2, int y2, char *c, int skip);
    extern void (*ReadBuffer)(int x1, int y1, int x2, int y2, char *c);
    extern void (*DrawBufferFast)(int x1, int y1, int x2, int y2, char *c);
    extern void (*ReadBufferFast)(int x1, int y1, int x2, int y2, char *c);
    extern void (*ScrollBufferV)(int lines, int blank);
    extern void (*ScrollBufferH)(int lines);
    extern void (*BlitShowBuffer)(int bnbr, int x1, int y1, int mode);
    extern void DrawRectangleUser(int x1, int y1, int x2, int y2, int c);
    extern void DrawBitmapUser(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
  //  extern void  getargaddress (volatile char *p, long long int **ip, MMFLOAT **fp, volatile int *n);
    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill);
    #define RGB(red, green, blue, trans) (unsigned int) (((trans & 0b1111) << 24) | ((red & 0b11111111) << 16) | ((green  & 0b11111111) << 8) | (blue & 0b11111111))
    #define swap(a, b) {int t = a; a = b; b = t;}

    #define BLACK               RGB(0,    0,      0,	0)
    #define BLUE                RGB(0,    0,      255,	255)
    #define GREEN               RGB(0,    255,    0,	255)
    #define CYAN                RGB(0,    255,    255,	255)
    #define RED                 RGB(255,  0,      0,	255)
    #define MAGENTA             RGB(255,  0,      192,	255)
    #define YELLOW              RGB(255,  255,    0,	255)
    #define BROWN               RGB(0xA5,	0x2a,	0x2a,	255)
    #define GRAY                RGB(64,   64,     64,	255)
    #define LITEGRAY            RGB(128,   128,     128,	255)
    #define WHITE               RGB(255,  255,    255,	255)
    #define ORANGE            	RGB(0xff,	0xA5,	0,	255)
	#define PINK				RGB(0xFF,	0xA0,	0xAB,	255)
	#define GOLD				RGB(0xFF,	0xD7,	0x00,	255)
	#define SALMON				RGB(0xFA,	0x80,	0x72,	255)
	#define BEIGE				RGB(0xF5,	0xF5,	0xDC,	255)
	#define LILAC               RGB(255,  128,  255, 255) //0b1101
	#define FUCHSIA             RGB(255,  64,   255, 255) //0b1011
	#define RUST                RGB(255,  64,     0, 255) //0b1010
	#define CERULEAN            RGB(0,    128,   255, 255) //0b0101
	#define MIDGREEN            RGB(0,    128,    0, 255) //0b0100
	#define COBALT              RGB(0,    64,   255, 255) //0b0011
	#define MYRTLE              RGB(0,    64,     0, 255) //0b0010
	#define NOTBLACK            (VideoColour==8? RGB(0,32, 0,15): (VideoColour==12? RGB(16,16,16,15): (VideoColour==16 ? RGB(0,4,0,15) : RGB(0,0,0,255))))
    #define JUSTIFY_LEFT        0
    #define JUSTIFY_CENTER      1
    #define JUSTIFY_RIGHT       2

    #define JUSTIFY_TOP         0
    #define JUSTIFY_MIDDLE      1
    #define JUSTIFY_BOTTOM      2

    #define ORIENT_NORMAL       0
    #define ORIENT_VERT         1
    #define ORIENT_INVERTED     2
    #define ORIENT_CCW90DEG     3
    #define ORIENT_CW90DEG      4
	#define CURSOR_OFF		350				    // cursor off time in mS
	#define CURSOR_ON		650				    // cursor on time in mS
    extern volatile int CurrentX, CurrentY;             // and the current default position
    
    extern volatile int gui_font;
    extern volatile int gui_font_width, gui_font_height;
    extern volatile int CursorTimer;                                // used to time the flashing cursor

    extern int gui_fcolour;
    extern int gui_bcolour;
    

    #define FONT_BUILTIN_NBR     8              // the number of built in fonts
    #define FONT_TABLE_SIZE     16              // the total size of the font table (builtin + loadable)
    extern unsigned char *FontTable[];
    extern uint32_t GetPageAddress(int page);
    extern int WritePage;
    extern int ReadPage;
    extern void setmode(int mode, int colour, int bc, int force);
    extern uint32_t CLUT[256];
	extern int PrintPixelMode;
	extern int AutoLineWrap;
	extern volatile int deferredcopy;
	extern uint32_t deferredfadd, deferredtadd, deferredtransparent;
	extern void PageCopy(uint32_t p1, uint32_t p2, int transparent);
	extern int getColour(char *c, int minus);
	extern int xcursor;
	extern int ycursor;
	extern int cursoron;
	extern int cursorenable;
	extern void hidecursor(int override);
	extern void showcursor(int override, int x, int y);
	extern const uint8_t cursor8[];
	extern const uint16_t cursor16[];
	extern void cmd_cursor(char *p);
	extern int autocursor;
	extern volatile int mouseupdated;
	extern uint8_t *loadcursordata;
	typedef struct SVD{
		FLOAT3D x;
		FLOAT3D y;
		FLOAT3D z;
	}s_vector;
	typedef struct t_quaternion{
		FLOAT3D w;
		FLOAT3D x;
		FLOAT3D y;
		FLOAT3D z;
		FLOAT3D m;
	}s_quaternion;

	struct D3D{
		s_quaternion *q_vertices;//array of original vertices
		s_quaternion *r_vertices; //array of rotated vertices
		s_quaternion *q_centroids;//array of original vertices
		s_quaternion *r_centroids; //array of rotated vertices
		s_vector *normals;
		uint8_t *facecount; //number of vertices for each face
		uint16_t *facestart; //index into the face_x_vert table of the start of a given face
	    uint32_t *fill; //fill colours
	    uint32_t *line; //line colours
	    uint32_t *colours;
		uint16_t *face_x_vert; //list of vertices for each face
		uint8_t *flags;
	    FLOAT3D *dots;
		FLOAT3D *depth;
		FLOAT3D distance;
		FLOAT3D ambient;
		int *depthindex;
		s_vector light;
		s_vector current;
		short tot_face_x_vert;
	    short xmin,xmax,ymin,ymax;
		uint16_t nv;	//number of vertices to describe the object
		uint16_t nf; // number of faces in the object
	    uint8_t dummy;
	    uint8_t vmax;  // maximum verticies for any face on the object
	    uint8_t camera; // camera to use for the object
		uint8_t nonormals;
		uint8_t depthmode;
	};
	typedef struct {
		FLOAT3D x;
		FLOAT3D y;
		FLOAT3D z;
		FLOAT3D viewplane;
		FLOAT3D panx;
		FLOAT3D pany;
	}s_camera;
	extern struct D3D *struct3d[MAX3D+1];
	extern s_camera camera[MAXCAM+1];
  #endif
#endif

    

