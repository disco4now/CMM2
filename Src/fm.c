/* Code loosely based on Kilo -- A very simple editor in less than 1-kilo lines of code (as counted
 *         by "cloc"). Does not depend on libcurses, directly emits VT100
 *         escapes on the terminal.
 *
 * -----------------------------------------------------------------------
 *
 * Copyright (C) 2016 Salvatore Sanfilippo <antirez at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGF.
 */
/* Massively modified and enhanced by Peter Mather. Copyright 2019-2020
 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#define GUI_C_NORMAL            WHITE
#define GUI_C_BCOLOUR           BLACK
#define GUI_C_COMMENT           RGB(192, 192, 0,	0)
#define GUI_C_KEYWORD           CYAN
#define GUI_C_QUOTE             RGB(0, 200, 0,	0)
#define GUI_C_NUMBER            RGB(255, 128, 128,	0)
#define GUI_C_LINE              GRAY
#define GUI_C_STATUS            (Option.colourmode==1 ? BROWN : gui_fcolour)
   #define MX470PutC(c)        {DisplayPutC(c,0);}
    #define MX470Scroll(n)      if((OptionConsole & 2))ScrollLCD(n, 1)
	#define PaintOn 189
	#define PaintOff 188
//    #define dx(...) {char s[140];sprintf(s,  __VA_ARGS__); SerUSBPutS(s); SerUSBPutS("\r\n");}

/** defines **/
#define KILO_VERSION "0.1b1"
#define KILO_TAB_STOP Option.Tab
#define KILO_QUIT_TIMES 3

#define CTRLKEY(a) (a & 0x1f)
#define MMPrintString       SerUSBPutS
#define BACKSPACE CTRLKEY('H')
extern volatile int ConsoleRxBufHead;
extern volatile int ConsoleRxBufTail;
extern void MX470PutS(char *s, int fc, int bc);
extern void MX470Cursor(int x, int y);
extern void MX470Display(int fn);
extern char *fstrstr (const char *s1, const char *s2);
extern char *strcasechr(const char *p, int ch);
extern char *bstrstr (const char *s1, const char *s3, const char *s2);
extern volatile uint8_t pagesetdone;
extern void setterminal(void);
extern volatile struct s_nunstruct mousestruct[4];

int FMStartEditLine=0, FMStartEditCharacter=0;
int FMnewmarkmode=0;
//char *FMcbuff=NULL;
int FMshowdefault=0;
extern int docheck;

static void list_files(int sortorder);
void close_FM(int cs);
static void FMInit(void);
const char FMdefaultpromptsmall[]="ESC=Exit, F2=Run, F3=List, F4=Edit, F5=MkDir, ^C=Copy, ^F=Find, ^K=Del, ^R=Rename, ^S=Sort";
const char FMdefaultpromptbig[]="ESC=Exit, F2=Run, F4=Edit, ^C=Copy, ^F=Find, ^K=Del";
const char FMdefaultpromptverybig[]="ESC=Exit, F2=Run, F4=Edit, ^C=Copy";
#define FMdefaultprompt ((Option.Width < 52) ? FMdefaultpromptverybig : (Option.Width < 100) ? FMdefaultpromptbig : FMdefaultpromptsmall)
enum editorHighlight {
  HL_NORMAL = 0,
  HL_COMMENT,
  HL_MLCOMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_STRING,
  HL_NUMBER,
  HL_MATCH,
  HL_MARK
};

#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)

/*** data ***/

struct editorSyntax {
  char *filetype;
  char **filematch;
  char **keywords;
  char *singleline_comment_start;
  char *rem_comment_start;
  int flags;
};

typedef struct erow {
  int idx;
  short size;
  short modified;
  short markstart;
  short markend;
  char *chars;
  char *selectbuff;
  unsigned char *hl;
  int hl_open_comment;
} erow;

struct FMConfig {
  int cx, cy;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  int insert;
  erow *row;
  short BreakKeySave;
  short sortorder;
  char *filename;
  char statusmsg[106];
  struct editorSyntax *syntax;
};
void (*FMcallback)(char *, int);

struct FMConfig F;

struct abuf {
  char *b;
  int len;
};
char mydir[256];
#define ABUF_INIT                                                              \
  { NULL, 0 }

struct abuf FMab = ABUF_INIT;

#define FMHLDB_ENTRIES (sizeof(FMHLDB) / sizeof(FMHLDB[0]))

/** prototypes **/
static void editorSetStatusMessage(const char *fmt, ...);
static void editorRefreshScreen();
static char *editorPrompt(char *prompt, int offset);
static void editorMoveCursor(int key);
/*** terminal ***/

static void parseanddisplay(char *p, int len){
	int i=0,j;
	char seq[12];
	while(i<len){
		while(p[i]!='\033'){
			DisplayPutC(p[i]);
			i++;
		}
		i++;
		if(p[i]=='['){ //escape sequence found
			mymemset(seq,0,12);
			j=0;
			seq[j++]=p[i];
			do {
				i++;
				seq[j++]=p[i];
			} while (!(IsAlpha(p[i])));
			if(seq[strlen(seq)-1]=='H'){
				if(strlen(seq)==2){
					MX470Cursor(0,0);
				}
				else {
					int rows,cols;
					sscanf(&seq[1], "%d;%d", &cols, &rows);
					MX470Cursor((rows-1)*gui_font_width,(cols-1)*gui_font_height);
				}
			}
			else if(strncmp(seq,"[K",2)==0){ // erase to end of line
				int i=F.screencols-(CurrentX/gui_font_width);
				while(i--)DisplayPutC(' ');
			}
			else if(strncmp(seq,"[?25l",5)==0)ShowCursor(false);
			else if(strncmp(seq,"[?25h",5)==0)ShowCursor(true);
			else if(strncmp(seq,"[36m",4)==0){gui_fcolour=CYAN;gui_bcolour=BLACK;}
			else if(strncmp(seq,"[37m",4)==0){gui_fcolour=WHITE;gui_bcolour=BLACK;}
			else if(strncmp(seq,"[32m",4)==0){gui_fcolour=GREEN;gui_bcolour=BLACK;}
			else if(strncmp(seq,"[39m",4)==0){gui_fcolour=WHITE;gui_bcolour=BLACK;}
			else if(strncmp(seq,"[35m",4)==0){gui_fcolour=MAGENTA;gui_bcolour=BLACK;}
			else if(strncmp(seq,"[31m",4)==0){gui_fcolour=RED;gui_bcolour=BLACK;}
			else if(strncmp(seq,"[34m",4)==0)gui_bcolour=BLUE;
			else if(strncmp(seq,"[33m",4)==0){gui_fcolour=YELLOW;gui_bcolour=BLACK;}
			else if(strncmp(seq,"[m",2)==0){
				gui_fcolour=WHITE;
				gui_bcolour=BLACK;
			}
			else if(strncmp(seq,"[7m",3)==0){
				int i=gui_fcolour;
				gui_fcolour=gui_bcolour;
				gui_bcolour=i;
			}
			i++;
		}
	}
}

static int editorReadKey(int cursor, int prompt, int buflen) {
	int c;
	static int lastZ=0, lastC=0;;
	do {
        if(FMcallback==NULL && prompt)MX470Cursor(buflen*gui_font_width, (F.screenrows+1)*gui_font_height-2);
	    if(cursor && Option.colourmode!=1)ShowCursor(true);
		c=MMInkey();
		if(Option.Mouse>=0){
			if(mousestruct[Option.Mouse].ax!=xcursor || mousestruct[Option.Mouse].ay!=ycursor){
				hidecursor(1);
				showcursor(1, mousestruct[Option.Mouse].ax, mousestruct[Option.Mouse].ay);
			}
			if(mousestruct[Option.Mouse].R){
				mousestruct[Option.Mouse].R=0;
				while(mousestruct[Option.Mouse].L){}
				mouse0foundz=0;
				mouse0foundc=0;
				mouse1foundz=0;
				mouse1foundc=0;
				mouse2foundz=0;
				mouse2foundc=0;
				mouse3foundz=0;
				mouse3foundc=0;
				return '\n';
			}
			if(mousestruct[Option.Mouse].az<0){
				mousestruct[Option.Mouse].az=0;
				return UP;
			}
			if(mousestruct[Option.Mouse].az>0){
				mousestruct[Option.Mouse].az=0;
				return DOWN;
			}
			if(mousestruct[Option.Mouse].Z != lastZ){
				lastZ=mousestruct[Option.Mouse].Z;
				if(lastZ){
					int line=mousestruct[Option.Mouse].ay/gui_font_height;
					if(line<=F.screenrows){
						F.cy=F.rowoff+line;
						editorRefreshScreen();
					}
				}
			}
			if(mousestruct[Option.Mouse].C != lastC){
				lastC=mousestruct[Option.Mouse].C;
				if(lastC){
					int line=mousestruct[Option.Mouse].ay/gui_font_height;
					if(line>=F.screenrows/2) return PDOWN;
					else return PUP;
				}
			}
		}
	} while(c==-1);

	return c;
}

/** syntax highlighting **/

static void editorUpdateSyntax(erow *row) {
  row->hl = ReAllocMemory(row->hl, row->size);
  mymemset(row->hl, HL_NORMAL, row->size);
  if(Option.colourmode!=1)return;
  char *q=&row->chars[strlen(row->chars)-4];
  if(strcasecmp(q,".bas")==0)mymemset(row->hl, HL_COMMENT, row->size);
  else {
	  char directory[6]={0};
	  memcpy(directory,row->chars,5);
	  if(strcasecmp(directory,"<DIR>")==0)mymemset(row->hl, HL_KEYWORD1, row->size);
  }
  return;
}

static int editorSyntaxToColor(int hl) {
  switch (hl) {
  case HL_COMMENT:
  case HL_MLCOMMENT:
    return 33;
  case HL_KEYWORD1:
    return 36;
  case HL_KEYWORD2:
    return 32;
  case HL_STRING:
    return 35;
  case HL_NUMBER:
    return 32;
  case HL_MATCH:
    return 34;
  default:
    return 37;
  }
}


static void editorUpdateRow(erow *row) {
  row->modified=1;
  editorUpdateSyntax(row);
  routinechecks(1);
}

static void editorInsertRow(int at, char *s, size_t len, int update) {
  int i;
  if (at < 0 || at > F.numrows)
    return;

  F.row = ReAllocMemory(F.row, sizeof(erow) * (F.numrows + 1));
  memmove(&F.row[at + 1], &F.row[at], sizeof(erow) * (F.numrows - at));
  for (int j = at + 1; j <= F.numrows; j++)
    F.row[j].idx++;

  F.row[at].idx = at;

  F.row[at].size = len;
  F.row[at].chars = GetMemory(len + 1);
  mycpy(F.row[at].chars, s, len);
  F.row[at].chars[len] = '\0';

  F.row[at].hl = GetMemory(len + 1);
  F.row[at].hl_open_comment = 0;
  F.row[at].markstart = 0;
  F.row[at].markend = -1;
  F.row[at].modified = 1;
  if(update)editorUpdateRow(&F.row[at]);
  for(i=at-1;i<F.numrows;i++)F.row[i].modified=1;
  F.numrows++;
}

static void editorFreeRow(erow *row) {
  FreeMemorySafe((void *)&row->chars);
  FreeMemorySafe((void *)&row->hl);
}

static void editorDelRow(int at) {
  int i;
  if (at < 0 || at >= F.numrows)
    return;
  editorFreeRow(&F.row[at]);
  memmove(&F.row[at], &F.row[at + 1], sizeof(erow) * (F.numrows - at - 1));
  for (int j = at; j < F.numrows - 1; j++)
    F.row[j].idx--;

  F.numrows--;
  for(i=at-1;i<F.numrows;i++)F.row[i].modified=1;
}

static int FMRun(int c) {
  	SerUSBPutS("\033[?25h");
  	SerUSBPutS("\033[37m");
  	SerUSBPutS("\033[m");
    if(Option.Mouse>=0){
    	strcpy(inpbuf,"CONTROLLER MOUSE CLOSE n:GUI CURSOR OFF");
    	inpbuf[23]=Option.Mouse+48;
	      tokenise(true);                                             // turn into executable code
  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
    }
	if(c){
	  char *q=&F.filename[strlen(F.filename)-4];
	  if(strcasecmp(q,".bas")!=0)return 0;
	  MMPrintString("\033[2J");
	  MMPrintString("\033[H");
	  MX470Display(DISPLAY_CLS);                          // clear screen on the MX470 display only
	  MX470Cursor(0, 0);
	  FileLoadProgram(F.filename,0);
	  if(c==F2){
		 CloseAudio(1);
		 close_FM(0);
		 ClearVars(0);
		 strcpy(inpbuf,"RUN\r\n");
		 tokenise(true);                                             // turn into executable code
		 InitHeap();
		 ExecuteProgram(tknbuf);                                     // execute the line straight away
		 cleanend();
	  }
	  return 1;
	}
	return 0;
}
static void FMCopy(int c) {
  char p[255];
  strcpy(p,F.filename);
  F.filename = editorPrompt("Copy as: %s (ESC to cancel)\033[K", 9);
  if (F.filename == NULL) {
    editorSetStatusMessage("Copy aborted\033[K");
    strcpy(F.filename,p);
    FMshowdefault=2;
    return;
  }
  strcpy(inpbuf,"COPY \"");
  strcat(inpbuf,p);
  strcat(inpbuf,"\" TO \"");
  strcat(inpbuf,F.filename);
  strcat(inpbuf,"\"\r\n");
  tokenise(true);                                             // turn into executable code
  ExecuteProgram(tknbuf);                                     // execute the line straight away
  mymemset(inpbuf,0,STRINGSIZE);
  FMshowdefault=1;
}

static void FMRenameAs(int c) {
  char p[255];
  strcpy(p,F.filename);
  F.filename = editorPrompt("Rename as: %s (ESC to cancel)\033[K", 11);
  if (F.filename == NULL) {
    editorSetStatusMessage("Rename aborted\033[K");
    strcpy(F.filename,p);
    FMshowdefault=2;
    return;
  }
  strcpy(inpbuf,"RENAME \"");
  strcat(inpbuf,p);
  strcat(inpbuf,"\" AS \"");
  strcat(inpbuf,F.filename);
  strcat(inpbuf,"\"\r\n");
  tokenise(true);                                             // turn into executable code
  ExecuteProgram(tknbuf);                                     // execute the line straight away
  mymemset(inpbuf,0,STRINGSIZE);
  FMshowdefault=1;
}
void FMMakeDir(int c){
	  char p[255];
	  strcpy(p,F.filename);
	  F.filename = editorPrompt("Directory name: %s (ESC to cancel)\033[K", 16);
	  if (F.filename == NULL) {
	    editorSetStatusMessage("Make directory aborted\033[K");
	    strcpy(F.filename,p);
	    FMshowdefault=2;
	    return;
	  }
	  strcpy(inpbuf,"MKDIR \"");
	  strcat(inpbuf,F.filename);
	  strcat(inpbuf,"\"\r\n");
	  tokenise(true);                                             // turn into executable code
	  ExecuteProgram(tknbuf);                                     // execute the line straight away
	  mymemset(inpbuf,0,STRINGSIZE);
	  FMshowdefault=1;
}
static int DeleteFile(void) {
  char *confirm;
  confirm = editorPrompt("Are you sure (Y/N) ? %s\033[K", 21);
  if (toupper(confirm[0])!='Y') {
    editorSetStatusMessage("Delete aborted\033[K");
    FMshowdefault=2;
    FreeMemorySafe((void *)&confirm);
    return 0;
  }
  FreeMemorySafe((void *)&confirm);
  strcpy(inpbuf,"KILL \"");
  strcat(inpbuf,F.filename);
  strcat(inpbuf,"\"\r\n");
  tokenise(true);                                             // turn into executable code
  ExecuteProgram(tknbuf);                                     // execute the line straight away
  mymemset(inpbuf,0,STRINGSIZE);
  FMshowdefault=1;
  return 1;
}


/** find **/
static void editorFindFMcallback(char *query, int key) {
  static int last_match = -1;
  static int direction = 1;
  static char *last_match_pos = NULL;
  static char *last_match_actual = NULL;
  static int saved_hl_line =-1;
//  static char *saved_hl = NULL;
  char *match;
  int current=0;
  if(saved_hl_line!=-1){
     F.row[saved_hl_line].markstart=0;
     F.row[saved_hl_line].markend=-1;
     editorUpdateRow(&F.row[saved_hl_line]);
  }
  if (key == '\n' || key == 0x1b) {
    last_match = -1;
    direction = 1;
    FMcallback=NULL;
    editorSetStatusMessage(FMdefaultprompt);
    int i;
    for(i=0;i<F.numrows;i++)F.row[i].modified=1;
    F.cx=0;
    return;
  } else if (key == RIGHT || key == DOWN) {
	  if(direction==-1)last_match_pos=last_match_pos+strlen(query);
    direction = 1;
  } else if (key == LEFT || key == UP) {
    direction = -1;
  } else {
    last_match = -1;
    direction = 1;
  }

  if (last_match == -1){
    direction = 1;
    current=F.cy;
    last_match_pos=F.row[current].chars+F.cx;
  } else {
	  current = last_match;
  }
  int i;
  for (i = 0; i < F.numrows; i++) {
    erow *row = &F.row[current];
    if(direction==1)match = fstrstr(last_match_pos, query);
    else match=bstrstr(last_match_pos, F.row[current].chars, query);
    if (match) {
      last_match_actual=match;
      int k=(int)((uint32_t)last_match_actual-(uint32_t)F.row[current].chars);
      F.row[current].markstart=k;
      F.row[current].markend=k+strlen(query);
      editorUpdateRow(&F.row[current]);
      if(direction==1)last_match_pos=match+strlen(query);
      else last_match_pos=match-1;
      last_match = current;
      saved_hl_line = current;
      F.cy = current;
      F.cx = match- row->chars;
      break;
    } else {
    	last_match_actual=NULL;
    	current += direction;
        if (current == -1)
          current = F.numrows - 1;
        else if (current == F.numrows)
          current = 0;
    	if(direction==1)last_match_pos=F.row[current].chars;
    	else last_match_pos=F.row[current].chars+F.row[current].size-1;
    }

  }
}
static void editorFind(void) {
  int saved_cx = F.cx;
  int saved_cy = F.cy;
  int saved_coloff = F.coloff;
  int saved_rowoff = F.rowoff;
  FMcallback=editorFindFMcallback;
  char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)\033[K",0);
  if (query) {
    FreeMemorySafe((void *)&query);
  } else {
    F.cx = saved_cx;
    F.cy = saved_cy;
    F.coloff = saved_coloff;
    F.rowoff = saved_rowoff;
  }
}

/*** append buffer ***/


static void abAppend(struct abuf *ab, const char *s, int len) {
  char *new = ReAllocMemory(ab->b, ab->len + len);
//  if(new!=ab->b){PInt(ab->len);PIntHC((uint32_t)ab->b);PIntHC((uint32_t)new);PRet();}
  if (new == NULL)
    return;
  mycopy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

static void abfree(void) {
	FreeMemorySafe((void *)&FMab.b);
	FMab.len=0;
}

/*** output ***/

static void editorScroll() {
  int i=0,oldrow=F.rowoff,oldcol=F.coloff;
//  F.rx = 0;
  if (F.cy >= F.numrows) {
	  F.cx=0;
  }

  if (F.cy < F.rowoff) {
    F.rowoff = F.cy;
  }
  if (F.cy >= F.rowoff + F.screenrows) {
    F.rowoff = F.cy - F.screenrows + 1;
  }
  if (F.cx < F.coloff) {
    F.coloff = F.cx;
  }
  if (F.cx >= F.coloff + F.screencols) {
    F.coloff = F.cx - F.screencols + 1;
  }
  if(oldrow!=F.rowoff || oldcol!=F.coloff) {
	  for(i=0;i<F.numrows;i++)F.row[i].modified=1;
	  while(MMInkey()!=-1);
  }
}

static void editorDrawRows(struct abuf *ab) {
  int y;
  char *savecolour;
  for (y = 0; y < (F.numrows<F.screenrows?F.numrows:F.screenrows); y++) {
  	routinechecks(1);
    int filerow = y + F.rowoff;
    if(filerow==F.cy && Option.colourmode==1){
    	savecolour=GetMemory(F.row[filerow].size);
    	memcpy(savecolour,F.row[filerow].hl,F.row[filerow].size);
    	F.row[filerow].modified=1;
    	mymemset(F.row[filerow].hl, HL_MATCH, F.row[filerow].size);
 /*   	int len = F.row[filerow].size - F.coloff;
        if (len < 0)
          len = 0;
        if (len > F.screencols)
          len = F.screencols;
        unsigned char *hl = &F.row[filerow].hl[F.coloff];
        int j;
        for (j = 0; j < len; j++) {
        	hl[j]=HL_NUMBER;
        }*/
   }
	if(F.row[filerow].modified==0 && filerow < F.numrows){
		abAppend(ab, "\r\n", 2);
		continue;
	}
	F.row[filerow].modified=0;
    if (filerow >= F.numrows) {
        abAppend(ab, "\033[K\r\n", 5);
    } else {
      int len = F.row[filerow].size - F.coloff;
      if (len < 0)
        len = 0;
      if (len > F.screencols)
        len = F.screencols;

      char *c = &F.row[filerow].chars[F.coloff];
      unsigned char *hl = &F.row[filerow].hl[F.coloff];
      int current_color = -1;
      int j;
      for (j = 0; j < len; j++) {
        if (IsCntrl(c[j])) {
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          abAppend(ab, "\033[7m", 4);
          abAppend(ab, &sym, 1);
          abAppend(ab, "\033[m", 3);
          if (current_color != -1) {
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\033[%dm", current_color);
            abAppend(ab, buf, clen);
          }
        } else if (hl[j] == HL_NORMAL) {
          if (current_color != -1) {
            abAppend(ab, "\033[39m", 5);
            current_color = -1;
          }
          abAppend(ab, &c[j], 1);
        } else {
          int color = editorSyntaxToColor(hl[j]);
          if (color != current_color) {
            current_color = color;
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\033[%dm", color);
            abAppend(ab, buf, clen);
          }
          abAppend(ab, &c[j], 1);
        }
      }
      abAppend(ab, "\033[39m", 5);
      abAppend(ab, "\033[K", 3);
      abAppend(ab, "\r\n", 2);
      if(filerow==F.cy && Option.colourmode==1){
      	F.row[filerow].modified=1;
      	memcpy(F.row[filerow].hl,savecolour,F.row[filerow].size);
      	FreeMemorySafe((void *)&savecolour);
       }
    }
  }
}

static void editorDrawStatusBar(struct abuf *ab) {
	  char buf[60];
	  snprintf(buf, sizeof(buf), "\033[%d;%dH", F.screenrows+1, 1);
	  strcpy(F.filename,filepath);
	  if(F.filename[strlen(F.filename)-1]!='/')strcat(F.filename,"/"); //prepare the full pathname
	  strcat(F.filename,&F.row[F.cy].chars[29]);
	  abAppend(ab, buf, strlen(buf));
		  abAppend(ab, "\033[7m", 4);
		  char status[80], rstatus[80];
		  int len = snprintf(status, sizeof(status), "%s",
		                     F.filename
		                     );
		  int rlen =
		      snprintf(rstatus, sizeof(rstatus), "%s  %d/%d",
		    		  (F.sortorder==0?"Name":(F.sortorder==1?"Date":(F.sortorder==2?"Size":"Type"))),
		               F.cy + 1, F.numrows);

		  if (len > F.screencols)
		    len = F.screencols;
		  if(status[0]=='0' && status[1]==':')status[0]='A';
		  abAppend(ab, status, len);
		  while (len < F.screencols) {
		    if (F.screencols - len == rlen) {
		      abAppend(ab, rstatus, rlen);
		      break;
		    } else {
		      abAppend(ab, " ", 1);
		      len++;
		    }
		  }
		  abAppend(ab, "\033[m", 3);
}

static void editorDrawMessageBar(struct abuf *ab) {
  abAppend(ab, "\033[K", 3);
  int msglen = strlen(F.statusmsg);
  if (msglen > F.screencols) msglen = F.screencols;
  char buf[32];
  snprintf(buf, sizeof(buf), "\033[%d;%dH", F.screenrows+2, 1);
  abAppend(ab, buf, strlen(buf));
  abAppend(ab, F.statusmsg, msglen);
}

static void editorRefreshScreen(void) {
  abfree();
  editorScroll();
  FMab.len=0;
  abAppend(&FMab, "\033[?25l", 6);
  abAppend(&FMab, "\033[H", 3);

  editorDrawRows(&FMab);
  editorDrawStatusBar(&FMab);
  editorDrawMessageBar(&FMab);
  char buf[32];
  snprintf(buf, sizeof(buf), "\033[%d;%dH", (F.cy - F.rowoff) + 1,
           (F.cx - F.coloff) + 1);
  abAppend(&FMab, buf, strlen(buf));

  if(Option.colourmode!=1)abAppend(&FMab, "\033[?25h", 6);
  if(OptionConsole & 1){
	  int i=FMab.len;
	  char *s=FMab.b;
	  while(i--){
		  SerUSBPutC(*s++);
	  }
  }

  if(OptionConsole & 2)parseanddisplay(FMab.b,FMab.len);
}

static void editorSetStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(F.statusmsg, sizeof(F.statusmsg), fmt, ap);
  va_end(ap);
}

/** input **/

static char *editorPrompt(char *prompt, int offset) {
  size_t bufsize = 128;
  char *buf = GetMemory(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();
    if(FMcallback==NULL){
    	ShowCursor(false);
    	MMPrintString("\033[?25l");
    }
    int c = editorReadKey((FMcallback==NULL? 1 : 1) , (FMcallback==NULL? 1 : 0), buflen+offset);
    if(FMcallback==editorFindFMcallback || FMcallback==NULL){
        if (c == DEL || c == CTRLKEY('H') || c == BACKSPACE) {
          if (buflen != 0)
            buf[--buflen] = '\0';
        } else if (c == 0x1b) {
          editorSetStatusMessage("");
          if (FMcallback)
            FMcallback(buf, c);
          FreeMemorySafe((void *)&buf);
          return NULL;
        } else if (c == '\n') {
          if (buflen != 0) {
            editorSetStatusMessage("");
            if (FMcallback)
              FMcallback(buf, c);
            return buf;
          }
        } else if (!IsCntrl(c) && c < 128) {
          if (buflen == bufsize - 1) {
            bufsize *= 2;
            buf = ReAllocMemory(buf, bufsize);
          }
          buf[buflen++] = c;
          buf[buflen] = '\0';
        }

        if (FMcallback)
          FMcallback(buf, c);
    }
  }
}

static void editorMoveCursor(int key) {
  erow *row = (F.cy >= F.numrows) ? NULL : &F.row[F.cy];
  switch (key) {
  case LEFT:
    if (F.cx != 0) {
      F.cx--;
    } else if (F.cy > 0) {
      F.cy--;
      F.cx = F.row[F.cy].size;
    }
    break;
  case RIGHT:
    if (row && F.cx < row->size) {
      F.cx++;
    } else if (row && F.cx == row->size && F.cy != F.numrows-1) {
      F.cy++;
      F.cx = 0;
    }
    break;
  case UP:
    if (F.cy > 0) {
      F.cy--;
    }
    break;
  case DOWN:
    if (F.cy < F.numrows-1) {
      F.cy++;
    }
    break;
  }

  row = (F.cy >= F.numrows) ? NULL : &F.row[F.cy];
  int rowlen = row ? row->size : 0;
  if (F.cx > rowlen) {
    F.cx = rowlen;
  }
}

static int editorProcessKeypress() {
  static int quit_times = KILO_QUIT_TIMES;
  int c = editorReadKey(1,0, 0);
  int i;
  if(c=='\n'){
  	 if(strncmp(F.row[F.cy].chars,"<DIR>",5) == 0){ //directory found
        if(!(F.row[F.cy].chars[29]=='.')){
        	if(filepath[strlen(filepath)-1]!='/') strcat(filepath,"/");
        	strcat(filepath,&F.row[F.cy].chars[29]);
        } else { //must start with two dots
        	 int linelen = strlen(filepath);
        	 if(strlen(filepath)!=3){
        		 while (linelen > 0 && filepath[linelen - 1] != '/' ) linelen--;
        		 if(linelen)filepath[linelen-1]=0;
        		 if(strlen(filepath)==2)strcat(filepath,"/");
        	 }
        }
        f_chdir(&filepath[2]);
  		for(i=F.numrows-1;i>=0;i--)editorDelRow(i);
  		for(i=0;i<=F.numrows;i++)editorFreeRow(&F.row[i]);
  		FreeMemorySafe((void *)&F.row);
  		FreeMemorySafe((void *)&FMab.b);
//  		FreeMemorySafe((void *)&FMcbuff);
  		list_files(F.sortorder);

  		return 1;
  	 } else {
  		  strcpy(F.filename,filepath);
  		  if(F.filename[strlen(F.filename)-1]!='/')strcat(F.filename,"/"); //prepare the full pathname
  		  strcat(F.filename,&F.row[F.cy].chars[29]);
  		  F.filename[0]='0';
  		  char *q=&F.filename[strlen(F.filename)-4];
  	  	  if(strcasecmp(q,".mp3")==0){
  	  		  CloseAudio(1);
  	  		  strcpy(inpbuf,"PLAY MP3 \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  strcat(inpbuf,"\"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  		  return 1;
  	  	  } else if(strcasecmp(q,"flac")==0){
  	  		  CloseAudio(1);
  	  		  strcpy(inpbuf,"PLAY FLAC \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  strcat(inpbuf,"\"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  		  return 1;
  	  	  } else if(strcasecmp(q,".wav")==0){
  	  		  CloseAudio(1);
  	  		  strcpy(inpbuf,"PLAY WAV \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  strcat(inpbuf,"\"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  		  return 1;
  	  	  } else if(strcasecmp(q,".mod")==0){
  	  		  CloseAudio(1);
  	  		  strcpy(inpbuf,"PLAY MODFILE \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  strcat(inpbuf,"\"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  	  		  return 1;
  	  	  } else if(strcasecmp(q,".jpg")==0/* && Option.Monitor*/){
  	  		  char pagerestore=5;
  	  		  int x=xcursor, y=ycursor;
  	  		  if((uint32_t)PageTable[ReadPage].address==(uint32_t)0xD0000000){
  	  			  strcpy(inpbuf,"PAGE COPY 0 TO 2:MODE x ,16:");
  	  			  pagerestore=2;
  	  		  }
  	  		  else strcpy(inpbuf,"PAGE COPY 0 TO 5:MODE x ,16:");
  	  		  if(Option.mode<10)inpbuf[22]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[23]=(Option.mode % 10)+48;
  	  			  inpbuf[22]=(Option.mode / 10)+48;
  	  		  }
  	  		  hidecursor(1);
  	  		  strcat(inpbuf,"LOAD JPG \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  if(Option.Mouse>=0)strcat(inpbuf,"\": GUI CURSOR OFF\r\n");
  	  		  else strcat(inpbuf,"\"\r\n");
	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  	  	      while(MMInkey()!=-1){}
  	  	      MM_Delay(100);
  	  	      while(1){
  	  	    	  if(MMInkey()!=-1)break;
  	  	    	  if(Option.Mouse>=0 && mousestruct[Option.Mouse].Z )break;
  	  	      }
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  		  if(pagerestore==2)strcpy(inpbuf,"MODE x ,8:PAGE COPY 2 TO 0");
  	  		  else strcpy(inpbuf,"MODE x ,8:PAGE COPY 5 TO 0");
  	  		  if(Option.mode<10)inpbuf[5]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[6]=(Option.mode % 10)+48;
  	  			  inpbuf[5]=(Option.mode / 10)+48;
  	  		  }
  	  		  if(Option.Mouse>=0)strcat(inpbuf,": GUI CURSOR ON\r\n");
  	  		  else strcat(inpbuf,"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  	  		  SetFont(((Option.editfont-1) << 4) | 1);
  		      mymemset(inpbuf,0,STRINGSIZE);
  	    	  hidecursor(1);showcursor(1, x, y);
 	  		  return 1;
  	  	  } else if(strcasecmp(q,".bmp")==0/* && Option.Monitor*/){
  	  		  char pagerestore=5;
  	  		  if((uint32_t)PageTable[ReadPage].address==(uint32_t)0xD0000000){
  	  			  strcpy(inpbuf,"PAGE COPY 0 TO 2:MODE x ,16:");
  	  			  pagerestore=2;
  	  		  }
  	  		  else strcpy(inpbuf,"PAGE COPY 0 TO 5:MODE x ,16:");
  	  		  if(Option.mode<10)inpbuf[22]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[23]=(Option.mode % 10)+48;
  	  			  inpbuf[22]=(Option.mode / 10)+48;
  	  		  }
  	  		  strcat(inpbuf,"LOAD BMP \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  if(Option.Mouse>=0)strcat(inpbuf,"\": GUI CURSOR OFF\r\n");
  	  		  else strcat(inpbuf,"\"\r\n");
	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  	  	      while(MMInkey()!=-1){}
  	  	      MM_Delay(100);
  	  	      while(1){
  	  	    	  if(MMInkey()!=-1)break;
  	  	    	  if(Option.Mouse>=0 && mousestruct[Option.Mouse].Z )break;
  	  	      }
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  		  if(pagerestore==2)strcpy(inpbuf,"MODE x ,8:PAGE COPY 2 TO 0");
  	  		  else strcpy(inpbuf,"MODE x ,8:PAGE COPY 5 TO 0");
  	  		  if(Option.mode<10)inpbuf[5]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[6]=(Option.mode % 10)+48;
  	  			  inpbuf[5]=(Option.mode / 10)+48;
  	  		  }
  	  		  if(Option.Mouse>=0)strcat(inpbuf,": GUI CURSOR ON\r\n");
  	  		  else strcat(inpbuf,"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  		      mymemset(inpbuf,0,STRINGSIZE);
  			  SetFont(((Option.editfont-1) << 4) | 1);
  	  		  return 1;
   	  	  } else if(strcasecmp(q,".png")==0/* && Option.Monitor*/){
  	  		  char pagerestore=5;
  	  		  if((uint32_t)PageTable[ReadPage].address==(uint32_t)0xD0000000){
  	  			  strcpy(inpbuf,"PAGE COPY 0 TO 2:MODE x ,16:");
  	  			  pagerestore=2;
  	  		  }
  	  		  else strcpy(inpbuf,"PAGE COPY 0 TO 5:MODE x ,16:");
  	  			  if(Option.mode<10)inpbuf[22]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[23]=(Option.mode % 10)+48;
  	  			  inpbuf[22]=(Option.mode / 10)+48;
  	  		  }
  	  		  strcat(inpbuf,"LOAD PNG \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  if(Option.Mouse>=0)strcat(inpbuf,"\": GUI CURSOR OFF\r\n");
  	  		  else strcat(inpbuf,"\"\r\n");
	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  	  	      while(MMInkey()!=-1){}
  	  	      MM_Delay(100);
  	  	      while(1){
  	  	    	  if(MMInkey()!=-1)break;
  	  	    	  if(Option.Mouse>=0 && mousestruct[Option.Mouse].Z )break;
  	  	      }
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  		  if(pagerestore==2)strcpy(inpbuf,"MODE x ,8:PAGE COPY 2 TO 0");
  	  		  else strcpy(inpbuf,"MODE x ,8:PAGE COPY 5 TO 0");
  	  		  if(Option.mode<10)inpbuf[5]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[6]=(Option.mode % 10)+48;
  	  			  inpbuf[5]=(Option.mode / 10)+48;
  	  		  }
  	  		  if(Option.Mouse>=0)strcat(inpbuf,": GUI CURSOR ON\r\n");
  	  		  else strcat(inpbuf,"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  		      mymemset(inpbuf,0,STRINGSIZE);
  			  SetFont(((Option.editfont-1) << 4) | 1);
  	  		  return 1;
   	  	  } else if(strcasecmp(q,".gif")==0/* && Option.Monitor*/){
  	  		  char pagerestore=5;
  	  		  if((uint32_t)PageTable[ReadPage].address==(uint32_t)0xD0000000){
  	  			  strcpy(inpbuf,"PAGE COPY 0 TO 2:MODE x ,16:");
  	  			  pagerestore=2;
  	  		  }
 	  		  else strcpy(inpbuf,"PAGE COPY 0 TO 5:MODE x ,16:");
  	  		  if(Option.mode<10)inpbuf[22]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[23]=(Option.mode % 10)+48;
  	  			  inpbuf[22]=(Option.mode / 10)+48;
  	  		  }
  	  		  strcat(inpbuf,"LOAD GIF \"");
  	  		  strcat(inpbuf,F.filename);
  	  		  if(Option.Mouse>=0)strcat(inpbuf,"\": GUI CURSOR OFF\r\n");
  	  		  else strcat(inpbuf,"\"\r\n");
	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  	  	      while(MMInkey()!=-1){}
  	  	      MM_Delay(100);
  	  	      while(1){
  	  	    	  if(MMInkey()!=-1)break;
  	  	    	  if(Option.Mouse>=0 && mousestruct[Option.Mouse].Z )break;
  	  	      }
  		      mymemset(inpbuf,0,STRINGSIZE);
  	  		  if(pagerestore==2)strcpy(inpbuf,"MODE x ,8:PAGE COPY 2 TO 0");
  	  		  else strcpy(inpbuf,"MODE x ,8:PAGE COPY 5 TO 0");
  	  		  if(Option.mode<10)inpbuf[5]=Option.mode+48;
  	  		  else {
  	  			  inpbuf[6]=(Option.mode % 10)+48;
  	  			  inpbuf[5]=(Option.mode / 10)+48;
  	  		  }
  	  		  if(Option.Mouse>=0)strcat(inpbuf,": GUI CURSOR ON\r\n");
  	  		  else strcat(inpbuf,"\r\n");
  	  	      tokenise(true);                                             // turn into executable code
  	  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
  		      mymemset(inpbuf,0,STRINGSIZE);
  			  if(gif!=NULL){
  				GifTimer=5000;
  				FileClose(giffnbr);
  				FreeMemorySafe((void*)&frame);
  				gd_close_gif(gif);
  				gif=NULL;
  				GifTimer=0;
  				giffnbr=0;
  			  }
  			  SetFont(((Option.editfont-1) << 4) | 1);
  	  		  return 1;
   	  	  } else if(strcasecmp(q,".bas")==0){
  	  		  c=F2;
  	  	  }
  	 }
  }
  char *f;
//  case F1:	         // Save and exit
  switch (c) {
  case F2:		     // run
	if(strncmp(F.row[F.cy].chars,"<DIR>",5) == 0)break;
	SerUSBPutS("\033[37m");
	strcpy(F.filename,&F.row[F.cy].chars[29]);
	SetFont(Option.DefaultFont);
	setterminal();
    reset_CLUT();
	FMRun(c);
    break;
  case F3:		     // List
	  if(strncmp(F.row[F.cy].chars,"<DIR>",5) == 0)break;
	  f=GetTempStrMemory();
	  strcpy(f,&F.row[F.cy].chars[29]);
	  close_FM(0);
	  strcpy(inpbuf,"LIST \"");
	  strcat(inpbuf,f);
	  strcat(inpbuf,"\"\r\n");
      tokenise(true);                                             // turn into executable code
      MX470Display(DISPLAY_CLS);                          // clear screen on the MX470 display only
  	  SerUSBPutS("\033[37m");
      ExecuteProgram(tknbuf);                                     // execute the line straight away
	  mymemset(inpbuf,0,STRINGSIZE);
	  return -1;
  case F4:		     // Edit
	  if(strncmp(F.row[F.cy].chars,"<DIR>",5) == 0)break;
	  f=GetTempStrMemory();
	  strcpy(f,&F.row[F.cy].chars[29]);
	  close_FM(0);
	  strcpy(inpbuf,"EDIT \"");
	  strcat(inpbuf,f);
	  strcat(inpbuf,"\"\r\n");
      tokenise(true);                                             // turn into executable code
      ExecuteProgram(tknbuf);                                     // execute the line straight away
	  mymemset(inpbuf,0,STRINGSIZE);
	  return 0;
  case CTRLKEY('C'): //copy file
	  if(strncmp(F.row[F.cy].chars,"<DIR>",5) == 0)break;
	  strcpy(F.filename,filepath);
	  if(F.filename[strlen(F.filename)-1]!='/')strcat(F.filename,"/"); //prepare the full pathname
	  strcat(F.filename,&F.row[F.cy].chars[29]);
	  F.filename[0]='0';
	  FMCopy(c);
		for(i=F.numrows-1;i>=0;i--)editorDelRow(i);
		for(i=0;i<=F.numrows;i++)editorFreeRow(&F.row[i]);
		FreeMemorySafe((void *)&F.row);
		FreeMemorySafe((void *)&FMab.b);
//		FreeMemorySafe((void *)&FMcbuff);
		list_files(F.sortorder);
	  break;
  case F5: //Make directory
	  strcpy(F.filename,filepath);
	  if(F.filename[strlen(F.filename)-1]!='/')strcat(F.filename,"/"); //prepare the full pathname
	  strcat(F.filename,&F.row[F.cy].chars[29]);
	  F.filename[0]='0';
	  FMMakeDir(c);
		for(i=F.numrows-1;i>=0;i--)editorDelRow(i);
		for(i=0;i<=F.numrows;i++)editorFreeRow(&F.row[i]);
		FreeMemorySafe((void *)&F.row);
		FreeMemorySafe((void *)&FMab.b);
//		FreeMemorySafe((void *)&FMcbuff);
		list_files(F.sortorder);
	  break;
  case CTRLKEY('K'): //Delete file
	  strcpy(F.filename,filepath);
	  if(F.filename[strlen(F.filename)-1]!='/')strcat(F.filename,"/"); //prepare the full pathname
	  strcat(F.filename,&F.row[F.cy].chars[29]);
	  F.filename[0]='0';
 	  if(DeleteFile()) editorDelRow(F.cy) ;
		for(i=F.numrows-1;i>=0;i--)editorDelRow(i);
		for(i=0;i<=F.numrows;i++)editorFreeRow(&F.row[i]);
		FreeMemorySafe((void *)&F.row);
		FreeMemorySafe((void *)&FMab.b);
//		FreeMemorySafe((void *)&FMcbuff);
		list_files(F.sortorder);
  	  break;
  case CTRLKEY('R'): //rename file
	  strcpy(F.filename,filepath);
	  if(F.filename[strlen(F.filename)-1]!='/')strcat(F.filename,"/"); //prepare the full pathname
	  strcat(F.filename,&F.row[F.cy].chars[29]);
	  F.filename[0]='0';
	  FMRenameAs(c);
		for(i=F.numrows-1;i>=0;i--)editorDelRow(i);
		for(i=0;i<=F.numrows;i++)editorFreeRow(&F.row[i]);
		FreeMemorySafe((void *)&F.row);
		FreeMemorySafe((void *)&FMab.b);
//		FreeMemorySafe((void *)&FMcbuff);
		list_files(F.sortorder);
	  break;
  case CTRLKEY('S'):
  	  	F.sortorder++;
  	  	if(F.sortorder==4)F.sortorder=0;
  	  	i=F.sortorder;
  	  	close_FM(0);
  	  	FMInit();
  	  	F.sortorder=i;
  	  	list_files(F.sortorder);
  	    if(Option.Mouse>=0){
  	    	strcpy(inpbuf,"GUI CURSOR ON:CONTROLLER MOUSE OPEN n,,,m");
  	    	inpbuf[36]=Option.Mouse+48;
  	    	if(Option.Sensitivity<10)inpbuf[40]=Option.Sensitivity+48;
  	    	else {
  	    		inpbuf[40]='1';
  	    		inpbuf[41]='0';
  	    	}
  		    tokenise(true);                                             // turn into executable code
  	  	    ExecuteProgram(tknbuf);                                     // execute the line straight away
  	    }
     break;
  case HOME:
  {    		int times=F.rowoff + F.cy;
            while (times--) editorMoveCursor(UP);}
    break;

  case END:
  {		int times=F.numrows-F.rowoff - F.cy + 1;
    		while (times-->=1) editorMoveCursor(DOWN);}
    break;
  case CTRLKEY('F'):
    editorFind();
    break;

  case PUP:
  case PDOWN: {
    if (c == PUP) {
    	if(F.cy!=F.rowoff){
    		F.cy = F.rowoff;
    	} else {
    		int times = F.screenrows;
    		while (times--) editorMoveCursor((c == PUP || c==CTRLKEY('P')) ? UP : DOWN);
    	}
    } else if (c == PDOWN) {
    	if(F.cy!=F.rowoff + F.screenrows - 1){
    	     F.cy = F.rowoff + F.screenrows - 1;
    	     if(F.cy>F.numrows)F.cy=F.numrows - 1;
    	} else {
		  F.cy = F.rowoff + F.screenrows - 1;
		  if (F.cy > F.numrows)
			F.cy = F.numrows;

		  int times = F.screenrows;
		  while (times--) editorMoveCursor((c == PUP || c==CTRLKEY('P')) ? UP : DOWN);
    	}
    }
  } break;
  case UP:
  case DOWN:
    editorMoveCursor(c);
    break;

  case 0x1b:
		MMPrintString("\033[?25h\0337\033[2J\033[H");									// vt100 clear screen and home cursor
//    	SetFont(Option.DefaultFont);
//    	setterminal();
//        reset_CLUT();
		return 0;

  default:
    break;
  }
  if(FMshowdefault){
	  FMshowdefault--;
	  if(!FMshowdefault)editorSetStatusMessage(FMdefaultprompt);
  }
  quit_times = KILO_QUIT_TIMES;
  return quit_times;
}

/*** init ***/

static void FMInit(void) {
  F.cx = 0;
  F.cy = 0;
  F.rowoff = 0;
  F.coloff = 0;
  F.numrows = 0;
  F.row = NULL;
  F.filename = NULL;
  F.statusmsg[0] = '\0';
  F.syntax = NULL;
  F.sortorder=0;;
  F.BreakKeySave = BreakKey;
  BreakKey = 0;
  F.insert=1;
  char sp[50]={0};
  strcpy(sp,"\033[8;");
  IntToStr(&sp[strlen(sp)],Option.Height,10);
  strcat(sp,";");
  IntToStr(&sp[strlen(sp)],Option.Width+1,10);
  strcat(sp,"t");
  SerUSBPutS(sp);						//
  F.screenrows=Option.Height;
  F.screencols=Option.Width;
  F.screenrows -= 2;
}
static void list_files(int sortorder)    {
    char line[STRINGSIZE];
    char extension[STRINGSIZE];
    ssize_t linelen;
	int i, dirs;
	uint32_t currentsize=0;
	uint32_t currentdate;
	char *p;
	int fcnt;
	char ts[STRINGSIZE] = "";
	s_flist *flist;
    DIR djd;
    FILINFO fnod;
	mymemset(&djd,0,sizeof(DIR));
	mymemset(&fnod,0,sizeof(FILINFO));
    fcnt = 0;
	p = "*";
	MMPrintString("\033[2J");
	MMPrintString("\033[H");
	MX470Display(DISPLAY_CLS);                          // clear screen on the MX470 display only
	MX470Cursor(0, 0);
	if(CurrentLinePtr) error("Invalid in a program");
    if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
    flist=GetMemory(sizeof(s_flist)*MAXFILES);
    if(strlen(filepath)!=3){ //not in top level directory
        editorInsertRow(F.numrows, "<DIR>                        ..", 31, 1);
        editorUpdateRow(&F.row[F.numrows-1]);
    }
    // search for the first file/dir
    FSerror = f_findfirst(&djd, &fnod, "", p);
    ErrorCheck(0);
    // add the file to the list, search for the next and keep looping until no more files
    while(FSerror == FR_OK && fnod.fname[0]) {
    	routinechecks(1);
        mymemset(extension,0,sizeof(extension));
        if(fcnt >= MAXFILES-1) {
        		FreeMemorySafe((void *)&flist);
            	f_closedir(&djd);
                error("Too many files to list");
        }
        if(!(fnod.fattrib & (AM_SYS | AM_HID))){
            // add a prefix to each line so that directories will sort ahead of files
            if(fnod.fattrib & AM_DIR){
                ts[0] = 'D';
                currentdate=0xFFFFFFFF;
                fnod.fdate=0xFFFF;
                fnod.ftime=0xFFFF;
                extension[0]='+';
                extension[1]='+';
                extension[2]='+';
                strcat(extension,fnod.fname);
            } else {
                ts[0] = 'F';
                currentdate=(fnod.fdate<<16) | fnod.ftime;
                if(fnod.fname[strlen(fnod.fname)-1]=='.') {strcpy(extension,&fnod.fname[strlen(fnod.fname)-1]);strcat(extension,fnod.fname);}
                else if(fnod.fname[strlen(fnod.fname)-2]=='.') {strcpy(extension,&fnod.fname[strlen(fnod.fname)-2]);strcat(extension,fnod.fname);}
                else if(fnod.fname[strlen(fnod.fname)-3]=='.') {strcpy(extension,&fnod.fname[strlen(fnod.fname)-3]);strcat(extension,fnod.fname);}
                else if(fnod.fname[strlen(fnod.fname)-4]=='.') {strcpy(extension,&fnod.fname[strlen(fnod.fname)-4]);strcat(extension,fnod.fname);}
                else if(fnod.fname[strlen(fnod.fname)-5]=='.') {strcpy(extension,&fnod.fname[strlen(fnod.fname)-5]);strcat(extension,fnod.fname);}
                else {
                	extension[0]='.';
                	extension[1]='.';
                	extension[2]='.';
                	strcat(extension,fnod.fname);
                }
           }
            currentsize=fnod.fsize;
            // and concatenate the filename found
            strcpy(&ts[1], fnod.fname);
            // sort the file name into place in the array
            if(sortorder==0){
            	for(i = fcnt; i > 0; i--) {
            		if( strcicmp((flist[i - 1].fn), (ts)) > 0)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
            } else if(sortorder==2){
            	for(i = fcnt; i > 0; i--) {
            		if( (flist[i - 1].fs) > currentsize)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
            } else if(sortorder==3){
        		char e2[STRINGSIZE];
            	for(i = fcnt; i > 0; i--) {
            		mymemset(e2,0,sizeof(e2));
                    if(flist[i - 1].fn[strlen(flist[i - 1].fn)-1]=='.') {strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-1]);strcat(e2,&flist[i - 1].fn[1]);}
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-2]=='.') {strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-2]);strcat(e2,&flist[i - 1].fn[1]);}
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-3]=='.') {strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-3]);strcat(e2,&flist[i - 1].fn[1]);}
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-4]=='.') {strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-4]);strcat(e2,&flist[i - 1].fn[1]);}
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-5]=='.') {strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-5]);strcat(e2,&flist[i - 1].fn[1]);}
                    else {
                    	if(flist[i - 1].fn[0]=='D'){
                    		mymemset(e2,0,sizeof(e2));
                        	e2[0]='+';
                        	e2[1]='+';
                        	e2[2]='+';
                        	strcat(e2,&flist[i - 1].fn[1]);
                    	} else {
                        	e2[0]='.';
                        	e2[1]='.';
                        	e2[2]='.';
                        	strcat(e2,&flist[i - 1].fn[1]);
                    	}
                    }
            		if( strcicmp(e2, extension) > 0)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
        	} else {
            	for(i = fcnt; i > 0; i--) {
            		if( ((flist[i - 1].fd<<16) |flist[i - 1].ft)  < currentdate)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}

        	}
            strcpy(flist[i].fn, ts);
            flist[i].fs = fnod.fsize;
            flist[i].fd = fnod.fdate;
            flist[i].ft = fnod.ftime;
            fcnt++;
        }
        FSerror = f_findnext(&djd, &fnod);
        if(FSerror==0 && fcnt==0 && fnod.fname[0]==0){
        	MMPrintString("Blank SDcard\r\n");
        	FreeMemorySafe((void *)&flist);
        	SCB_CleanInvalidateDCache();
            mymemset(inpbuf,0,STRINGSIZE);
            longjmp(mark, 1);
        }
   }
    // list the files
	for(i = dirs = 0; i < fcnt; i++) {
    	routinechecks(1);
		if(MMAbort) {
        	FreeMemorySafe((void *)&flist);
            f_closedir(&djd);
            WDTimer = 0;                                                // turn off the watchdog timer
            mymemset(inpbuf,0,STRINGSIZE);
        	SCB_CleanInvalidateDCache();
            longjmp(mark, 1);
        }
		if(flist[i].fn[0] == 'D') {
    		dirs++;
            strcpy(ts,"<DIR>                        ");
		}
		else {
		    IntToStrPad(ts, (flist[i].ft>>11)&0x1F, '0', 2, 10);
		    ts[2] = ':'; IntToStrPad(ts + 3, (flist[i].ft >>5)&0x3F, '0', 2, 10);
		    ts[5]=' ';
		    IntToStrPad(ts + 6, flist[i].fd & 0x1F, '0', 2, 10);
		    ts[8] = '-'; IntToStrPad(ts + 9, (flist[i].fd >> 5)&0xF, '0', 2, 10);
		    ts[11] = '-'; IntToStr(ts + 12 , ((flist[i].fd >> 9)& 0x7F )+1980, 10);
		    ts[16] =' ';
		    IntToStrPad(ts+17, flist[i].fs, ' ', 10, 10);
            strcat(ts,"  ");
        }
        strcat(ts, flist[i].fn + 1);
		// check if it is more than a screen full
      	mymemset(line,0,256);
      	  linelen = strlen(ts);
          editorInsertRow(F.numrows, ts, linelen, 1);
          editorUpdateRow(&F.row[F.numrows-1]);
          routinechecks(1);
        }
    f_closedir(&djd);
	FreeMemorySafe((void *)&flist);
	F.cy = F.cx = 0;
	clearrepeat();
	}
void close_FM(int cs){
	int i;
	FreeMemorySafe((void *)&F.filename);
	for(i=0;i<=F.numrows;i++)editorFreeRow(&F.row[i]);
	FreeMemorySafe((void *)&F.row);
	FreeMemorySafe((void *)&FMab.b);
//	FreeMemorySafe((void *)&FMcbuff);
    gui_fcolour = PromptFC;
    gui_bcolour = PromptBC;
    if(Option.Mouse>=0){
    	strcpy(inpbuf,"CONTROLLER MOUSE CLOSE n:GUI CURSOR OFF");
    	inpbuf[23]=Option.Mouse+48;
	      tokenise(true);                                             // turn into executable code
  	      ExecuteProgram(tknbuf);                                     // execute the line straight away
    }
    if(cs==0){
    	MMPrintString("\033[2J");
    	MMPrintString("\033[H");
    	MX470Display(DISPLAY_CLS);                          // clear screen on the MX470 display only
    	MX470Cursor(0, 0);
    }// home the cursor
    BreakKey = F.BreakKeySave;
}
void cmd_files(void) {
    if(CurrentLinePtr) error("Invalid in a program");
	int cs=1;
    sendCRLF=3;
    docheck=1;
    i2c_disable();                                                  // close I2C
    i2c2_disable();                                                  // close I2C
    i2c3_disable();                                                  // close I2C
    mouse0close();
	ClearVars(0);
	closeallsprites();
	closeall3d();
	hidecursor(0);
	cursorenable=0;
	FreeMemorySafe((void *)&cursorsave);
//	FreeMemorySafe((void *)&main_turtle_polyX);
//	FreeMemorySafe((void *)&main_turtle_polyY);
//	FreeMemorySafe((void *)&PageTable[WPN].address);
//	FreeMemorySafe((void *)&PageTable[BPN].address);
	if(CurrentlyPlaying==P_NOTHING){
		InitHeap();
	}
    setmode(Option.mode,DEFCOLOUR,0,Option.mode==12?1:0);
	SetFont(((Option.editfont-1) << 4) | 1);
	reset_CLUT();
	if(Option.colourmode==-1){
		CLUT[0]=0xFFFFFF;
		CLUT[255]=0;
		CLUT[3]=0xFFFF;
		while(!pagesetdone)CheckAbort();
		HAL_LTDC_ConfigCLUT(&hltdc, CLUT, 256, 0);
	}
    MX470Display(DISPLAY_CLS);                          // clear screen on the MX470 display only
    MX470Cursor(0, 0);                                  // home the cursor
	InitSDCard();
    MM_Delay(3);
	while(MMInkey()!=-1){}
	FMInit();
    editorSetStatusMessage(FMdefaultprompt);
    list_files(F.sortorder);
    editorRefreshScreen();
    if(Option.Mouse>=0){
    	strcpy(inpbuf,"GUI CURSOR ON:CONTROLLER MOUSE OPEN n,,,m");
    	inpbuf[36]=Option.Mouse+48;
    	if(Option.Sensitivity<10)inpbuf[40]=Option.Sensitivity+48;
    	else {
    		inpbuf[40]='1';
    		inpbuf[41]='0';
    	}
	    tokenise(true);                                             // turn into executable code
  	    ExecuteProgram(tknbuf);                                     // execute the line straight away
    }
    while (1) {
    	if((cs=editorProcessKeypress())<=0){
    		close_FM(cs);
        	SetFont(Option.DefaultFont);
        	setterminal();
            setmode(Option.mode,DEFCOLOUR,0,Option.mode==12?1:0);
            reset_CLUT();
    		return;
    	}
    editorRefreshScreen();
  }
}
