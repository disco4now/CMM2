/***********************************************************************************************************************
MMBasic

FileIO.c

Does all the SD Card related file I/O in MMBasic.

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


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"

//** SD CARD INCLUDES ***********************************************************
#include "ff.h"
#include "diskio.h"
//#include "integer.h"
#include "decode_polling.h"
#include "gifdec.h"
#include "ltdc.h"
#include "picojpeg.h"

int OptionFileErrorAbort = true;
extern void Display_Refresh(void);
extern BYTE MDD_SDSPI_CardDetectState(void);
// FatFs definitions
FATFS FatFs;
//static JPEG_HandleTypeDef     JPEG_Handle;
JPEG_ConfTypeDef       JPEG_Info;
//static DMA2D_HandleTypeDef    DMA2D_Handle;
//static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize, uint32_t ChromaSampling);
char FileGetChar(int fnbr);
char FilePutChar(char c, int fnbr);
int FileEOF(int fnbr);
void File_CloseAll(void);
int InitSDCard(void);
char *ChangeToDir(char *p);
void LoadImage(char *p);
void LoadJPG(char *p);
void LoadPNG(char *p);
void LoadGIF(char *p);
void LoadFont(char *p);
void cmd_LoadJPGImage(char *p);
int ImageHeight, ImageWidth, ChromaSubsampling;
int dirflags;
FRESULT FSerror;
volatile BYTE SDCardStat = STA_NOINIT | STA_NODISK;
volatile int diskcheckrate = 0;
extern 	volatile unsigned int checkSD;
#define SDbufferSize 512
extern unsigned int __attribute__((section(".my_section"))) _excep_cause;
extern int as_strcmpi (const char *s1, const char *s2);
int jxpos=0, jypos=0;
extern uint8_t *FontBuffer;
extern RTC_HandleTypeDef hrtc;
gd_GIF *gif=NULL;
unsigned char *frame;
int giffnbr=0,gifxOrigin, gifyOrigin;
extern volatile uint8_t pagesetdone;
char filepath[FF_MAX_LFN]="A:/";
extern int docheck;
extern LTDC_LayerCfgTypeDef pLayerCfg;
char fullpathname[FF_MAX_LFN];
char fullfilepathname[FF_MAX_LFN];
typedef struct sa_dlist {
    char from[STRINGSIZE];
    char to[STRINGSIZE];
} a_dlist;
a_dlist *dlist;

int nDefines;
int LineCount=0;
// 8*8*4 bytes * 3 = 768
int16_t *gCoeffBuf;

// 8*8*4 bytes * 3 = 768
uint8_t *gMCUBufR;
uint8_t *gMCUBufG;
uint8_t *gMCUBufB;

// 256 bytes
int16_t *gQuant0;
int16_t *gQuant1;
uint8_t *gHuffVal2;
uint8_t *gHuffVal3;
uint8_t *gInBuf;
#define BLOCK_SIZE 4096
/*****************************************************************************************
Mapping of errors reported by the file system to MMBasic file errors
*****************************************************************************************/
const int ErrorMap[] = {        0,                                  // 0
                                1,                                  // Assertion failed
                                2,                                  // Low level I/O error
                                3,                                  // No response from SDcard
                                4,                                  // Could not find the file
                                5,                                  // Could not find the path
                                6,                                  // The path name format is invalid
                                7,                                  // Prohibited access or directory full
                                8,                                  // Directory exists or path to it cannot be found
                                9,                                  // The file/directory object is invalid
                               10,                                  // SD card is write protected
                               11,                                  // The logical drive number is invalid
                               12,                                  // The volume has no work area
                               13,                                  // Not a FAT volume
                               14,                                  // Format aborted
                               15,                                  // Could not access volume
                               16,                                  // File sharing policy
                               17,                                  // Buffer could not be allocated
                               18,                                  // Too many open files
                               19,                                  // Parameter is invalid
							   20									// Not present
                            };

/******************************************************************************************
Text for the file related error messages reported by MMBasic
******************************************************************************************/
const char *FErrorMsg[] = {	"",
		"A hard error occurred in the low level disk I/O layer",
		"Assertion failed",
		"SD Card not found",
		"Could not find the file",
		"Could not find the path",
		"The path name format is invalid",
		"FAccess denied due to prohibited access or directory full",
		"Access denied due to prohibited access",
		"The file/directory object is invalid",
		"The physical drive is write protected",
		"The logical drive number is invalid",
		"The volume has no work area",
		"There is no valid FAT volume",
		"The f_mkfs() aborted due to any problem",
		"Could not get a grant to access the volume within defined period",
		"The operation is rejected according to the file sharing policy",
		"LFN working buffer could not be allocated",
		"Number of open files > FF_FS_LOCK",
		"Given parameter is invalid",
		"SD card not present"
};




//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// MMBASIC COMMANDS & FUNCTIONS FOR THE SDCARD /////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup the SD Card based on the settings saved in flash

/*static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize, uint32_t ChromaSampling)
{
    int maxW=PageTable[WritePage].xmax;

  uint32_t cssMode = DMA2D_CSS_420, inputLineOffset = 0;
  uint32_t destination = 0;

  if(ChromaSampling == JPEG_420_SUBSAMPLING)
  {
    cssMode = DMA2D_CSS_420;

    inputLineOffset = xsize % 16;
    if(inputLineOffset != 0)
    {
      inputLineOffset = 16 - inputLineOffset;
    }
  }
  else if(ChromaSampling == JPEG_444_SUBSAMPLING)
  {
    cssMode = DMA2D_NO_CSS;

    inputLineOffset = xsize % 8;
    if(inputLineOffset != 0)
    {
      inputLineOffset = 8 - inputLineOffset;
    }
  }
  else if(ChromaSampling == JPEG_422_SUBSAMPLING)
  {
    cssMode = DMA2D_CSS_422;

    inputLineOffset = xsize % 16;
    if(inputLineOffset != 0)
    {
      inputLineOffset = 16 - inputLineOffset;
    }
  }

  //##-1- Configure the DMA2D Mode, Color Mode and output offset #############
  DMA2D_Handle.Init.Mode         = DMA2D_M2M_PFC;
  DMA2D_Handle.Init.ColorMode    = (VideoColour==12  ? DMA2D_OUTPUT_ARGB4444 : (VideoColour==32 ? DMA2D_OUTPUT_ARGB8888 : DMA2D_OUTPUT_RGB565 ));
  DMA2D_Handle.Init.OutputOffset = maxW - xsize;
  DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  // No Output Alpha Inversion
  DMA2D_Handle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     // No Output Red & Blue swap

  //##-2- DMA2D Callbacks Configuration ######################################
  DMA2D_Handle.XferCpltCallback  = NULL;

  //##-3- Foreground Configuration ###########################################
  DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_YCBCR;
  DMA2D_Handle.LayerCfg[1].ChromaSubSampling = cssMode;
  DMA2D_Handle.LayerCfg[1].InputOffset = inputLineOffset;
  DMA2D_Handle.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; // No ForeGround Red/Blue swap
  DMA2D_Handle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; // No ForeGround Alpha inversion

  DMA2D_Handle.Instance          = DMA2D;

  //##-4- DMA2D Initialization     ###########################################
  HAL_DMA2D_Init(&DMA2D_Handle);
  HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1);

  //##-5-  copy the new decoded frame to the LCD Frame buffer ################
  destination = (uint32_t)pDst + ((y * hltdc.LayerCfg->ImageWidth) + x) * 4;

  HAL_DMA2D_Start(&DMA2D_Handle, (uint32_t)pSrc, destination, xsize, ysize);
  HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 25);  // wait for the previous DMA2D transfer to ends
}*/


void cmd_save(void) {
    int fnbr;
    unsigned int nbr;
    char *p, *pp, *flinebuf;
    int x,y,w,h, filesize;
    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};
    int i,j;
    if(!InitSDCard()) return;
	int maxH=PageTable[ReadPage].ymax;
    int maxW=PageTable[ReadPage].xmax;
    fnbr = FindFreeFileNbr();
    if((p = checkstring(cmdline, "IMAGE")) !=NULL){
    	getargs(&p,9,",");
        if((void *)ReadBuffer == (void *)DisplayNotSet) error("SAVE IMAGE not available on this display");
        pp = getFstring(argv[0]);
        if(argc!=1 && argc!=9)error("Syntax");
        if(strchr(pp, '.') == NULL) strcat(pp, ".BMP");
        if(!BasicFileOpen(pp, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
        if(argc==1){
        	x=0; y=0; h=maxH; w=maxW;
        } else {
        	x=getint(argv[2],0,maxW-1);
    		if(optiony) x=maxW-1-x;
        	y=getint(argv[4],0,maxH-1);
    		if(optiony) y=maxH-1-y;
        	w=getint(argv[6],1,maxW-x);
        	h=getint(argv[8],1,maxH-y);
        }
    	int savey=optiony;
    	optiony=0;
        filesize=54 + 3*w*h;
        bmpfileheader[ 2] = (unsigned char)(filesize    );
        bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
        bmpfileheader[ 4] = (unsigned char)(filesize>>16);
        bmpfileheader[ 5] = (unsigned char)(filesize>>24);

        bmpinfoheader[ 4] = (unsigned char)(       w    );
        bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
        bmpinfoheader[ 6] = (unsigned char)(       w>>16);
        bmpinfoheader[ 7] = (unsigned char)(       w>>24);
        bmpinfoheader[ 8] = (unsigned char)(       h    );
        bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
        bmpinfoheader[10] = (unsigned char)(       h>>16);
        bmpinfoheader[11] = (unsigned char)(       h>>24);
		f_write(FileTable[fnbr].fptr, bmpfileheader, 14, &nbr);
		f_write(FileTable[fnbr].fptr, bmpinfoheader, 40, &nbr);
        flinebuf = GetTempMemory(maxW * 4);
        for(i = y+h-1; i >= y; i--){
           ReadBuffer(x, i, x+w-1, i, flinebuf);
           pp=flinebuf;
           if(VideoColour==12){
        	   for(j=0;j<maxW*4;j++){
        		   if(j % 4 != 3)*pp++=flinebuf[j];
        	   }
           }
           f_write(FileTable[fnbr].fptr, flinebuf, w*3, &nbr);
           if((w*3) % 4 !=0) f_write(FileTable[fnbr].fptr, bmppad, 4-((w*3) % 4) , &nbr);
           if(CurrentlyPlaying != P_NOTHING){
			   f_sync(FileTable[fnbr].fptr);
			   MM_Delay(1);
           }
        }
        optiony=savey;
    } else if((p = checkstring(cmdline, "DATA")) !=NULL){
		getargs(&p,5,",");
		if(argc!=5)error("Syntax");
		pp = getFstring(argv[0]);
		if(strchr(pp, '.') == NULL) strcat(pp, ".DAT");
		uint32_t address=(GetPeekAddr(argv[2]) & 0b11111111111111111111111111111100);
		uint32_t size=getint(argv[4],1,0x7FFFFFFF);
		for(uint32_t i=address;i<address+size;i++)if(!PEEKRANGE(i)) error("Address");
		if(!BasicFileOpen(pp, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
		f_write(FileTable[fnbr].fptr,  (char *)address, size, &nbr);
		if(nbr!=size)error("File write error");
    } else {
    	error("Syntax");
    }

    FileClose(fnbr);
}
int cmpstr(char *s1,char *s2)
{
  unsigned char *p1 = (unsigned char *) s1;
  unsigned char *p2 = (unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2)
    return 0;

  do
    {
      c1 = tolower (*p1++);
      c2 = tolower (*p2++);
      if (c1 == '\0') return 0;
    }
  while (c1 == c2);

  return c1 - c2;
}

int massage(char *buff){
	int i=nDefines;
	while(i--){
		char *p=dlist[i].from;
		while(*p){
			*p=toupper(*p);
			p++;
		}
		p=dlist[i].to;
		while(*p){
			*p=toupper(*p);
			p++;
		}
		STR_REPLACE(buff,dlist[i].from,dlist[i].to,false);
	}
	STR_REPLACE(buff,"=<","<=",3);
	STR_REPLACE(buff,"=>",">=",3);
	STR_REPLACE(buff," ,",",",3);
	STR_REPLACE(buff,", ",",",3);
	STR_REPLACE(buff," *","*",3);
	STR_REPLACE(buff,"* ","*",3);
	STR_REPLACE(buff,"- ","-",3);
	STR_REPLACE(buff," /","/",3);
	STR_REPLACE(buff,"/ ","/",3);
	STR_REPLACE(buff,"= ","=",3);
	STR_REPLACE(buff,"+ ","+",3);
	STR_REPLACE(buff," )",")",3);
	STR_REPLACE(buff,") ",")",3);
	STR_REPLACE(buff,"( ","(",3);
	STR_REPLACE(buff,"> ",">",3);
	STR_REPLACE(buff,"< ","<",3);
	STR_REPLACE(buff," '","'",3);
	return strlen(buff);
}
void importfile(char *pp, char *tp, char **p, uint32_t buf, int convertdebug){
    int fnbr;
    char buff[256];
    char qq[FF_MAX_LFN] = {0};
    char num[10];
    int importlines=0;
    int ignore=0;
	char *fname, *sbuff, *op, *ip;
    int c, f, slen, data;
    fnbr = FindFreeFileNbr();
    char  *q;
    if((q=strchr(tp,34)) == 0) error("Syntax");
    q++;
    if((q=strchr(q,34)) == 0) error("Syntax");
    fname = getFstring(tp);
	if(strchr(&fname[strlen(fname)-4], '.') == NULL) strcat(fname, ".INC");
    f=strlen(fname);
	q=&fname[strlen(fname)-4];
	if(strcasecmp(q,".inc")!=0)error("must be a .inc file");
	if(!(fname[1]==':' || fname[0] == 92 || fname[0]==47)) {strcpy(qq,pp);strcat(qq,fname);}
	else strcpy(qq,fname);
	BasicFileOpen(qq, fnbr, FA_READ);
    while(!FileEOF(fnbr)) {
    	int toggle=0, len=0;// while waiting for the end of file
    	sbuff=buff;
        if(((uint32_t)*p - buf) >= EDIT_BUFFER_SIZE - 256*6) error("Not enough memory");
        mymemset(buff,0,256);
		MMgetline(fnbr, (char *)buff);									    // get the input line
		data=0;
		importlines++;
		LineCount++;
		routinechecks(1);
		len=strlen(buff);
		toggle=0;
		for(c=0;c<strlen(buff);c++){
			if(buff[c] == TAB) buff[c] = ' ';
		}
		while(*sbuff==' '){
			sbuff++;
			len--;
		}
		if(ignore && sbuff[0]!='#')*sbuff='\'';
		if(strncasecmp(sbuff,"rem ",4)==0 || (len==3 && strncasecmp(sbuff,"rem",3)==0 )){
			sbuff+=2;
			*sbuff='\'';
			continue;
		}
		if(strncasecmp(sbuff,"data ",5)==0)data=1;
		slen=len;
		op=sbuff;
		ip=sbuff;
		while(*ip){
			if(*ip==34){
				if(toggle==0)toggle=1;
				else toggle=0;
			}
			if(!toggle && (*ip==' ' || *ip==':')){
				*op++=*ip++; //copy the first space
				while(*ip==' '){
					ip++;
					len--;
				}
			}
			else *op++=*ip++;
		}
		slen=len;
		if(!(toupper(sbuff[0])=='R' && toupper(sbuff[1])=='U' && toupper(sbuff[2])=='N' && (strlen(sbuff)==3 || sbuff[3]==' '))){
			toggle=0;
			for(c=0;c<slen;c++){
				if(!(toggle || data))sbuff[c]=toupper(sbuff[c]);
				if(sbuff[c]==34){
					if(toggle==0)toggle=1;
					else toggle=0;
				}
			}
		}
		toggle=0;
		for(c=0;c<slen;c++){
			if(sbuff[c]==34){
				if(toggle==0)toggle=1;
				else toggle=0;
			}
			if(!toggle && sbuff[c]==39 && len==slen){
				len=c;//get rid of comments
				break;
			}
		}
		if(sbuff[0]=='#'){
			char *tp=checkstring(&sbuff[1], "DEFINE");
			if(tp){
				getargs(&tp,3,",");
				if(nDefines>=MAXDEFINES){
				    FreeMemorySafe((void *)&buf);
				    FreeMemorySafe((void *)&dlist);
					error("Too many #DEFINE statements");
				}
				strcpy(dlist[nDefines].from,getFstring(argv[0]));
				strcpy(dlist[nDefines].to,getFstring(argv[2]));
				nDefines++;
			} else {
				if(cmpstr("COMMENT END",&sbuff[1])==0)ignore=0;
				if(cmpstr("COMMENT START",&sbuff[1])==0)ignore=1;
				if(cmpstr("MMDEBUG ON",&sbuff[1])==0)convertdebug=0;
				if(cmpstr("MMDEBUG OFF",&sbuff[1])==0)convertdebug=1;
				if(cmpstr("INCLUDE ",&sbuff[1])==0){
					error("Can't import from an import");
				}
			}
		} else {
			if(toggle)sbuff[len++]=34;
			sbuff[len++]=39;
			sbuff[len++]='|';
			memcpy(&sbuff[len],fname,f);
			len+=strlen(fname);
			sbuff[len++]=',';
			IntToStr(num,importlines,10);
			strcpy(&sbuff[len],num);
			len+=strlen(num);
			if(len>254){
				FreeMemorySafe((void *)&buf);
				FreeMemorySafe((void *)&dlist);
				error("Line too long");
			}
			sbuff[len]=0;
			len=massage(sbuff); //can't risk crushing lines with a quote in them
			if((sbuff[0]!=39) || (sbuff[0]==39 && sbuff[1]==39)){
				if(Option.profile){
					while(strlen(sbuff)<9){
						strcat(sbuff," ");
						len++;
					}
				}
				memcpy(*p,sbuff,len);
				*p+=len;
				**p='\n';
				*p+=1;
			}
		}
    }
    FileClose(fnbr);
    return ;
}

// loads a file from the SDCARD into temporary SDRAM memory
// It then writes the tokenised version to either FLASH program memory or
// SDRAM program memory dependent on the setting of OPTION [RAM|FLASH]
// FLASH is only written if the program has changed.
// mode=1 if filename passed. mode=0 for no filename, i.e. run the same program
// return true if success.
int FileLoadProgram(char *fname, int mode) {
    int fnbr, size=0;
    char *p,*op, *ip, *buf, *sbuff, name[FF_MAX_LFN]={0},buff[STRINGSIZE];
    char pp[FF_MAX_LFN] = {0};
    char num[10];
    int c;
    int convertdebug=1;
    int ignore=0;
    nDefines=0;
    LineCount=0;
    int i,importlines=0, data;
	if(mode){
		strcpy(buff,getFstring(fname));
	} else strcpy(buff,fname);
	if(strchr(&buff[strlen(buff)-4], '.') == NULL) strcat(buff, ".BAS");
    ClearProgram();												    // clear any leftovers from the previous program
    if(!InitSDCard()) return false;
    fnbr = FindFreeFileNbr();
	if(!BasicFileOpen(buff, fnbr, FA_READ)) return false;
	strcpy(name,fullfilepathname);
    i=strlen(name)-1;
    while(i>0 && !(name[i] == 92 || name[i]==47))i--;
    memcpy(pp,name,i+1);
    p = buf = GetMemory(EDIT_BUFFER_SIZE);
    dlist=GetMemory(sizeof(a_dlist)*MAXDEFINES);

    while(!FileEOF(fnbr)) {                                     // while waiting for the end of file
    	int toggle=0, len=0, slen;// while waiting for the end of file
    	sbuff=buff;
        if((p - buf) >= EDIT_BUFFER_SIZE - 256*6) error("Not enough memory");
        mymemset(buff,0,256);
		MMgetline(fnbr, (char *)buff);									    // get the input line
		data=0;
		importlines++;
		LineCount++;
    	routinechecks(1);
		len=strlen(buff);
		toggle=0;
		for(c=0;c<strlen(buff);c++){
			if(buff[c] == TAB) buff[c] = ' ';
		}
		while(sbuff[0]==' '){ //strip leading spaces
			sbuff++;
			len--;
		}
		if(ignore && sbuff[0]!='#')*sbuff='\'';
		if(strncasecmp(sbuff,"rem ",4)==0 || (len==3 && strncasecmp(sbuff,"rem",3)==0 )){
			sbuff+=2;
			*sbuff='\'';
			continue;
		}
		if(strncasecmp(sbuff,"mmdebug ",7)==0 && convertdebug==1){
			sbuff+=6;
			*sbuff='\'';
			continue;
		}
		if(strncasecmp(sbuff,"data ",5)==0)data=1;
		slen=len;
		op=sbuff;
		ip=sbuff;
		while(*ip){
			if(*ip==34){
				if(toggle==0)toggle=1;
				else toggle=0;
			}
			if(!toggle && (*ip==' ' || *ip==':')){
				*op++=*ip++; //copy the first space
				while(*ip==' '){
					ip++;
					len--;
				}
			} else *op++=*ip++;
		}
		slen=len;
		if(sbuff[0]=='#'){
			char *tp=checkstring(&sbuff[1], "DEFINE");
			if(tp){
				getargs(&tp,3,",");
				if(nDefines>=MAXDEFINES){
				    FreeMemorySafe((void *)&buf);
				    FreeMemorySafe((void *)&dlist);
					error("Too many #DEFINE statements");
				}
				strcpy(dlist[nDefines].from,getFstring(argv[0]));
				strcpy(dlist[nDefines].to,getFstring(argv[2]));
				nDefines++;
			} else {
				if(cmpstr("COMMENT END",&sbuff[1])==0)ignore=0;
				if(cmpstr("COMMENT START",&sbuff[1])==0)ignore=1;
				if(cmpstr("MMDEBUG ON",&sbuff[1])==0)convertdebug=0;
				if(cmpstr("MMDEBUG OFF",&sbuff[1])==0)convertdebug=1;
				if(cmpstr("INCLUDE",&sbuff[1])==0){
					importfile(pp, &sbuff[8],&p, (uint32_t)buf, convertdebug);
				}
			}
		} else {
			if(!(toupper(sbuff[0])=='R' && toupper(sbuff[1])=='U' && toupper(sbuff[2])=='N' && (strlen(sbuff)==3 || sbuff[3]==' '))){
				toggle=0;
				for(c=0;c<slen;c++){
					if(!(toggle || data))sbuff[c]=toupper(sbuff[c]);
					if(sbuff[c]==34){
						if(toggle==0)toggle=1;
						else toggle=0;
					}
				}
			}
			toggle=0;
			for(c=0;c<slen;c++){
				if(sbuff[c]==34){
					if(toggle==0)toggle=1;
					else toggle=0;
				}
				if(!toggle && sbuff[c]==39 && len==slen){
					len=c;//get rid of comments
					break;
				}
			}
			if(toggle)sbuff[len++]=34;
			sbuff[len++]=39;
			sbuff[len++]='|';
			IntToStr(num,importlines,10);
			strcpy(&sbuff[len],num);
			len+=strlen(num);
			if(len>254){
			    FreeMemorySafe((void *)&buf);
			    FreeMemorySafe((void *)&dlist);
				error("Line too long");
			}
			sbuff[len]=0;
			len=massage(sbuff); //can't risk crushing lines with a quote in them
			if((sbuff[0]!=39) || (sbuff[0]==39 && sbuff[1]==39)){
				if(Option.profile){
					while(strlen(sbuff)<9){
						strcat(sbuff," ");
						len++;
					}
				}
				mycpy(p,sbuff,len);
				p+=len;
				*p++='\n';
			}
		}

    }
    *p = 0;                                                         // terminate the string in RAM
    FileClose(fnbr);
	int load=0;
    if(Option.ProgramStartCode>=0){
		size=SaveProgramToMemory(buf, false, name);
    	uint32_t *top=(uint32_t *)((uint32_t)SDMemory+512*1024-4);
    	top--;
		i=0;
		ip=(char *)SDMemory;
		op=(char *)ProgMemory;
		while(i<size){
			i++;
			if(*ip != *op){
				load=1;
				break;
			}
			op++;
			ip++;
		}
		while(*top-- == 0xFFFFFFFF){};
		size=(uint32_t)top - (uint32_t)SDMemory+256;
		FreeMemorySafe((void *)&SDMemory);
    }
    if(load || Option.ProgramStartCode<0)SaveProgramToFlash(buf, false, name, size);
    FreeMemorySafe((void *)&buf);
    FreeMemorySafe((void *)&dlist);
    return true;
}
#define skipzero(x)    while(*x == 0) x++
void LoadFONT(char *p){
    char buff[256], *pp;
    int i, f, n, y, x, fnbr, bcount;
    uint32_t c;
	getargs(&p, 1, ",");                                            // this MUST be the first executable line in the function
    if(argc == 0) error("Argument count");
    if(!InitSDCard()) return;

    p = getFstring(argv[0]);                                        // get the file name

	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".FNT");
	fnbr = FindFreeFileNbr();
    if(!BasicFileOpen(p, fnbr, FA_READ)) return;
	MMgetline(fnbr, (char *)buff);									    // get the input line
	for(c=0;c<strlen(buff);c++){
		if(buff[c] == TAB) buff[c] = 0;
		if(buff[c] == ' ') buff[c] = 0;
	}
	pp=buff+10;
	if(!mem_equal(buff,"DefineFont",10))error("Syntax");
	skipzero(pp);
	if(*pp!='#')error("Syntax");
	pp++;
	f = (strtoul(pp, NULL, 10)-1);
	if(f!=7)error("Fonts load into #8");
	MMgetline(fnbr, (char *)buff);									    // get the input line
	for(c=0;c<strlen(buff);c++){
		if(buff[c] == TAB) buff[c] = ' ';
	}
	pp=buff;
	c = strtoul(pp, NULL, 16);
	n=c>>24;
	if(((c>>16) & 0xff) < 0x20)error("Can't define non-printing characters");
	y=(c>>8)&0xff;
	x=c & 0xff;
	if((x*y)%8)error("Height x width not divisible by 8");
	bcount=(n*x*y)>>3;
	FontTable[7]=FontBuffer=GetMemory(bcount+4); //don't forget the header!!!
	uint32_t *dcopy=(uint32_t *)FontBuffer;
	*dcopy++=c; //store the font header;
    while(!FileEOF(fnbr)) {
		MMgetline(fnbr, (char *)buff);									    // get the input line
		for(c=0;c<strlen(buff);c++){
			if(buff[c] == TAB) buff[c] = ' ';
		}
		pp=buff;
		for(i=0; i<8; i++){
			if(bcount>0){
				skipspace(pp);
				*dcopy++ = strtoul(pp, &pp, 16);
				bcount-=4;
			}
		}
    }
    FileClose(fnbr);
}

void cmd_load(void) {
    char *p;

    p = checkstring(cmdline, "BMP");
	if(p) {
        LoadImage(p);
       return;
    }
    p = checkstring(cmdline, "JPG");
	if(p) {
		cmd_LoadJPGImage(p);
       return;
    }
    p = checkstring(cmdline, "GIF");
	if(p) {
        LoadGIF(p);
       return;
    }
    p = checkstring(cmdline, "PNG");
	if(p) {
        LoadPNG(p);
       return;
    }
    p = checkstring(cmdline, "FONT");
	if(p) {
        LoadFONT(p);
       return;
    }
	p = checkstring(cmdline, "DATA");
	if(p) {
	    int fnbr;
	    unsigned int nbr;
		static FILINFO fnod;
	    char *pp;
		getargs(&p,3,",");
		if(argc!=3)error("Syntax");
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		pp = getFstring(argv[0]);
		if(strchr(pp, '.') == NULL) strcat(pp, ".DAT");
		uint32_t address=(GetPokeAddr(argv[2]) & 0b11111111111111111111111111111100);
		FSerror = f_stat(pp, &fnod);
		if(FSerror != FR_OK)error((char *)FErrorMsg[4]);
		if((fnod.fattrib & AM_DIR))error((char *)FErrorMsg[4]);
		uint32_t size=fnod.fsize;
		for(uint32_t i=address;i<address+size;i++)if(!POKERANGE(i)) error("Address");
	    fnbr = FindFreeFileNbr();
		if(!BasicFileOpen(pp, fnbr, FA_READ)) return;
		f_read(FileTable[fnbr].fptr,  (char *)address, size, &nbr);
		if(nbr!=size)error("File read error");
	    FileClose(fnbr);
	    return;
	}
	error("Syntax");
}



// search for a volume label, directory or file
// s$ = DIR$(fspec, DIR|FILE|ALL)       will return the first entry
// s$ = DIR$()                          will return the next
// If s$ is empty then no (more) files found
void fun_dir(void) {
    static DIR djd;
    char *p;
    static FILINFO fnod;
    static char pp[FF_MAX_LFN];
    char q[FF_MAX_LFN]={0};
    getargs(&ep, 3, ",");
    if(argc != 0) dirflags = -1;
    if(!(argc <= 3)) error("Syntax");

    if(argc == 3) {
        if(checkstring(argv[2], "DIR"))
            dirflags = AM_DIR;
        else if(checkstring(argv[2], "FILE"))
            dirflags = -1;
        else if(checkstring(argv[2], "ALL"))
            dirflags = 0;
        else
            error("Invalid flag specification");
    }


    if(argc != 0) {
    	int i, j;
        // this must be the first call eg:  DIR$("*.*", FILE)
        p = getFstring(argv[0]);
        i=strlen(p)-1;
        while(i>0 && !(p[i] == 92 || p[i]==47))i--;
        if(i>0){
        	memcpy(q,p,i);
             for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
        	if(q[1]==':')q[0]='0';
        	i++;
        }
        strcpy(pp,&p[i]);
        if((pp[0]==47 || pp[0]==92) && i==0){
        	strcpy(q,&pp[1]);
        	strcpy(pp,q);
        	strcpy(q,"0:/");
        }
		if(pp[0]==0)strcpy(pp,"*");
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		fullpath(q);
        if(pp[0]==0)strcpy(pp,"*");
        djd.pat = pp;
        if(!InitSDCard()) return;                                   // setup the SD card
        FSerror = f_opendir(&djd, fullpathname);
        ErrorCheck(0);
    }
        if(disk_status(0) & STA_NOINIT){
           f_closedir(&djd);
            error("SD card not found");
        }
        if(dirflags == AM_DIR){
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && (fnod.fattrib & AM_DIR) && !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }
        else if(dirflags == -1){
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && !(fnod.fattrib & AM_DIR)&& !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }
        else {
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }

    if (FSerror != FR_OK || !fnod.fname[0])f_closedir(&djd);
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    strcpy(sret, fnod.fname);
    CtoM(sret);                                                     // convert to a MMBasic style string
    targ = T_STR;
}
void getfullpath(char *p, char *q){
	int j;
    strcpy(q,p);
    for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
    if(q[1]==':')q[0]='0';
    fullpath(q);
    strcpy(q,fullpathname);
}

void cmd_copy(void){
	char ss[2];														// this will be used to split up the argument line
	char *fromfile, *tofile;
	ss[0] = tokenTO;
	ss[1] = 0;
	char buff[512];
	unsigned int nbr=0, bw;
	int fnbr1, fnbr2;
	getargs(&cmdline,3,ss);
	if(argc!=3)error("Syntax");
    fnbr1 = FindFreeFileNbr();
	fromfile = getFstring(argv[0]);
	BasicFileOpen(fromfile, fnbr1, FA_READ);
    fnbr2 = FindFreeFileNbr();
	tofile = getFstring(argv[2]);
    if(!BasicFileOpen(tofile, fnbr2, FA_WRITE | FA_CREATE_ALWAYS)) {
    	FileClose(fnbr1);
    }
    while(!f_eof(FileTable[fnbr1].fptr)) {
		FSerror = f_read(FileTable[fnbr1].fptr, buff,512, &nbr);
		ErrorCheck(fnbr1);
	    FSerror = f_write(FileTable[fnbr2].fptr, buff, nbr, &bw);
	    ErrorCheck(fnbr2);
		routinechecks(1);
    }
	FileClose(fnbr1);
	FileClose(fnbr2);
}

void cmd_mkdir(void) {
    char *p;
    char q[FF_MAX_LFN]={0};
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    getfullpath(p,q);
    if(!InitSDCard()) return;
    FSerror = f_mkdir(q);
    ErrorCheck(0);
}



void cmd_rmdir(void){
    char *p;
    char q[FF_MAX_LFN]={0};
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    getfullpath(p,q);
    if(!InitSDCard()) return;
    FSerror = f_unlink(q);
    ErrorCheck(0);
}

void rmSubstr(char *str, const char *toRemove)
{
    size_t length = strlen(toRemove);
    char *found,
         *next = strstr(str, toRemove);

    for (size_t bytesRemoved = 0; (found = next); bytesRemoved += length)
    {
        char *rest = found + length;
        next = strstr(rest, toRemove);
        memmove(found - bytesRemoved,
                rest,
                next ? next - rest: strlen(rest) + 1);
    }
}
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
int resolve_path(char *path,char *result,char *pos)
{
    if (*path == '/') {
	*result = '/';
	pos = result+1;
	path++;
    }
    *pos = 0;
    if (!*path) return 0;
    while (1) {
	char *slash;
	struct stat st;
	st.st_mode=0;
	slash = *path ? strchr(path,'/') : NULL;
	if (slash) *slash = 0;
	if (!path[0] || (path[0] == '.' &&
	  (!path[1] || (path[1] == '.' && !path[2])))) {
	    pos--;
	    if (pos != result && path[0] && path[1])
		while (*--pos != '/');
	}
	else {
	    strcpy(pos,path);
//	    if (lstat(result,&st) < 0) return -1;
	    if (S_ISLNK(st.st_mode)) {
		char buf[PATH_MAX];
//		if (readlink(result,buf,sizeof(buf)) < 0) return -1;
		*pos = 0;
		if (slash) {
		    *slash = '/';
		    strcat(buf,slash);
		}
		strcpy(path,buf);
		if (*path == '/') result[1] = 0;
		pos = strchr(result,0);
		continue;
	    }
	    pos = strchr(result,0);
	}
	if (slash) {
	    *pos++ = '/';
	    path = slash+1;
	}
	*pos = 0;
	if (!slash) break;
    }
    return 0;
}
void fullpath(char *q){
	char *p=GetTempStrMemory();
	char *rp=GetTempStrMemory();
	int i;
	strcpy(p,q);
	mymemset(fullpathname,0,sizeof(fullpathname));
	strcpy(fullpathname,filepath);
    for(i=0;i<strlen(p);i++)if(p[i]=='\\')p[i]='/';  //allow backslash for the DOS oldies
    if(strcmp(p,".")==0 || strlen(p)==0){
    	memmove(fullpathname, &fullpathname[2],strlen(fullpathname));
//    	MMPrintString("Now: ");MMPrintString(fullpathname);PRet();
    	return; //nothing to do
    }
    if(p[1]==':'){ //modify the requested path so that if the disk is specified the pathname is absolute and starts with /
    	if(p[2]=='/')p+=2;
    	else {
    		p[1]='/';
    		p++;
    	}
    }
    if (*p=='/'){ //absolute path specified
    	strcpy(rp,"A:");
    	strcat(rp,p);
    } else { // relative path specified
    	strcpy(rp,fullpathname); //copy the current pathname
        if(rp[strlen(rp)-1]!='/')  strcat(rp,"/"); //make sure the previous pathname ends in slash, will only be the case at root
    	strcat(rp,p); //append the new pathname
    }
	strcpy(fullpathname,rp); //set the new pathname
	resolve_path(fullpathname,rp,rp); //resolve to single absolute path
	if(strcmp(rp,"A:")==0 || strcmp(rp,"0:")==0 )strcat(rp,"/"); //if root append the slash
	strcpy(fullpathname,rp); //store this back to the filepath variable
	memmove(fullpathname, &fullpathname[2],strlen(fullpathname));
//	MMPrintString("Now: ");MMPrintString(fullpathname);PRet();
}
void cmd_chdir(void){
	int i;
    char *p;
    char rp[STRINGSIZE],oldfilepath[STRINGSIZE];
    p = strupr(getFstring(cmdline));  // get the directory name and convert to a standard C string
    for(i=0;i<strlen(p);i++)if(p[i]=='\\')p[i]='/';  //allow backslash for the DOS oldies
    if(strcmp(p,".")==0)return; //nothing to do
    if(strlen(p)==0)return;//nothing to do
    strcpy(oldfilepath,filepath); //save the path in case the change of directory fails
    if(p[1]==':'){ //modify the requested path so that if the disk is specified the pathname is absolute and starts with /
    	if(p[2]=='/')p+=2;
    	else {
    		p[1]='/';
    		p++;
    	}
    }
    if (*p=='/'){ //absolute path specified
    	strcpy(rp,"A:");
    	strcat(rp,p);
    } else { // relative path specified
    	strcpy(rp,filepath); //copy the current pathname
        if(rp[strlen(rp)-1]!='/')  strcat(rp,"/"); //make sure the previous pathname ends in slash, will only be the case at root
    	strcat(rp,p); //append the new pathname
    }
	strcpy(filepath,rp); //set the new pathname
	resolve_path(filepath,rp,rp); //resolve to single absolute path
	if(strcmp(rp,"A:")==0)strcat(rp,"/"); //if root append the slash
	strcpy(filepath,rp); //store this back to the filepath variable
    if(!InitSDCard()) { //If no disk restore the old path and return
    	strcpy(filepath,oldfilepath);
    	return;
    }
	FSerror = f_chdir(&filepath[2]); //finally change directory always using an absolute pathname
	if(FSerror)strcpy(filepath,oldfilepath); //if it didn't work restore the original path
	ErrorCheck(0); // error if the pathname was invalid

}


void fun_cwd(void) {
    sret = GetTempStrMemory();                                    // this will last for the life of the command
	strcpy(sret,filepath);
    sret = CtoM(sret);
    targ = T_STR;
}

void getfullfilepath(char *p, char *q){
	int i,j;
    char pp[FF_MAX_LFN] = {0};
    i=strlen(p)-1;
    while(i>0 && !(p[i] == 92 || p[i]==47))i--;
    if(i>0){
    	memcpy(q,p,i);
    	 for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
    	if(q[1]==':')q[0]='0';
    	i++;
    }
    strcpy(pp,&p[i]);
    if((pp[0]==47 || pp[0]==92) && i==0){
    	strcpy(q,&pp[1]);
    	strcpy(pp,q);
    	strcpy(q,"0:/");
    }
    fullpath(q);
    strcpy(q,fullpathname);
    if(q[strlen(q)-1]!=47)strcat(q,"/");
    strcat(q,pp);
}

void cmd_kill(void){
    char q[FF_MAX_LFN]={0};
    char *p = getFstring(cmdline);
    getfullfilepath(p,q);
    FSerror = f_unlink(q);
    ErrorCheck(0);
    docheck=1;
}



void cmd_seek(void) {
    int fnbr, idx;
    getargs(&cmdline, 5, ",");
    if(argc != 3) error("Syntax");
    if(*argv[0] == '#') argv[0]++;
    fnbr = getinteger(argv[0]);
    if(fnbr < 1 || fnbr > MAXOPENFILES || FileTable[fnbr].com <= MAXCOMPORTS) error("File number");
    if(FileTable[fnbr].com == 0) error("File number #% is not open", fnbr);
    if(!InitSDCard()) return;
    idx = getint(argv[2],1,0x7FFFFFFF) - 1;
    if(idx < 0) idx = 0;
        FSerror = f_lseek(FileTable[fnbr].fptr,idx);
        ErrorCheck(fnbr);

}


void cmd_name(void) {
    char *old, *new, ss[2];
    ss[0] = tokenAS;                                                // this will be used to split up the argument line
    ss[1] = 0;
    char qold[FF_MAX_LFN]={0};
    char qnew[FF_MAX_LFN]={0};
    {                                                               // start a new block
        getargs(&cmdline, 3, ss);                                   // getargs macro must be the first executable stmt in a block
        if(argc != 3) error("Syntax");
        old = getFstring(argv[0]);                                  // get the old name
        getfullfilepath(old,qold);
        new = getFstring(argv[2]);                                  // get the new name
        getfullfilepath(new,qnew);
        if(!InitSDCard()) return;
        FSerror = f_rename(qold, qnew);
        ErrorCheck(0);
    }
}


extern int BMP_bDecode(int x, int y, int fnbr);

void LoadImage(char *p) {
	int fnbr;
	int xOrigin, yOrigin;

	// get the command line arguments
	getargs(&p, 5, ",");                                            // this MUST be the first executable line in the function
    if(argc == 0) error("Argument count");
    if(!InitSDCard()) return;

    p = getFstring(argv[0]);                                        // get the file name
	int maxH=PageTable[WritePage].ymax;

    xOrigin = yOrigin = 0;
	if(argc >= 3) xOrigin = getinteger(argv[2]);                    // get the x origin (optional) argument
	if(argc == 5) {
		yOrigin = getinteger(argv[4]);                    // get the y origin (optional) argument
		if(optiony) yOrigin=maxH-1-yOrigin;
	}

	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".BMP");
	fnbr = FindFreeFileNbr();
    if(!BasicFileOpen(p, fnbr, FA_READ)) return;
	int savey=optiony;
	optiony=0;
    BMP_bDecode(xOrigin, yOrigin, fnbr);
    optiony=savey;
    FileClose(fnbr);
}
void LoadPNG(char *p) {
//	int fnbr;
	int xOrigin, yOrigin,w,h, transparent=0, force=0;
    int maxW=PageTable[WritePage].xmax;
	int maxH=PageTable[WritePage].ymax;
	upng_t* upng;
	// get the command line arguments
	getargs(&p, 7, ",");                                            // this MUST be the first executable line in the function
    if(argc == 0) error("Argument count");
    if(!InitSDCard()) return;

    p = getFstring(argv[0]);                                        // get the file name

    xOrigin = yOrigin = 0;
	if(argc >= 3 && *argv[2]) xOrigin = getinteger(argv[2]);                    // get the x origin (optional) argument
	if(argc >= 5 && *argv[4]){
		yOrigin = getinteger(argv[4]);                    // get the y origin (optional) argument
		if(optiony) yOrigin=maxH-1-yOrigin;
	}
	if(argc ==7)transparent=getint(argv[6],0,15);
	if(transparent){
		if(transparent>1)force=transparent<<4;
		transparent=4;
	}
	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".PNG");
//	fnbr = FindFreeFileNbr();
//    if(!BasicFileOpen(p, fnbr, FA_READ)) return;
    upng = upng_new_from_file(p);
	routinechecks(1);
    upng_header(upng);
    w=upng_get_width(upng);
    h= upng_get_height(upng);
    if(w+xOrigin >maxW || h+yOrigin >maxH){
        upng_free(upng);
        error("Image too large");
    }
    if(!(upng_get_format(upng)==1 || upng_get_format(upng)==3)){
        upng_free(upng);
        error("Invalid format");
    }
	routinechecks(1);
    upng_decode(upng);
    unsigned char *rr;
	routinechecks(1);
    rr=(unsigned char *)upng_get_buffer(upng);
	int savey=optiony;
	optiony=0;
    if(upng_get_format(upng)==3){
    	DrawBuffer(xOrigin, yOrigin, xOrigin+w-1, yOrigin+h-1,(char *)rr,3 | transparent | force);
    } else {
    	DrawBuffer(xOrigin, yOrigin, xOrigin+w-1, yOrigin+h-1,(char *)rr,2 | transparent | force);
    }
    optiony=savey;
    upng_free(upng);
//    FileClose(fnbr);
	clearrepeat();
}
void GIFcallback(void){
	int ret;
	int savey=optiony;
	optiony=0;
	gd_render_frame(gif,frame);
	DrawBuffer(gifxOrigin, gifyOrigin, gifxOrigin+gif->width-1, gifyOrigin+gif->height-1,(char *)frame,2);
	ret=gd_get_frame(gif);
    GifTimer=gif->gce.delay*10;
    if (ret == 0) {
    	gd_rewind(gif);
//    	ret=gd_get_frame(gif);
//        GifTimer=gif->gce.delay*10;

    }
    optiony=savey;
}
void LoadGIF(char *p) {
	int ret;
	// get the command line arguments
	getargs(&p, 5, ",");                                            // this MUST be the first executable line in the function
    if(argc == 0 || gif!=NULL) {
    	if(gif!=NULL){
    		GifTimer=5000;
    		FileClose(giffnbr);
    		FreeMemorySafe((void*)&frame);
    		gd_close_gif(gif);
    		gif=NULL;
    		GifTimer=0;
    		giffnbr=0;
    	}
		if(argc==0)return;
    }
    if(!InitSDCard()) return;
	int maxH=PageTable[WritePage].ymax;

    p = getFstring(argv[0]);                                        // get the file name

    gifxOrigin = gifyOrigin = 0;
	if(argc >= 3) gifxOrigin = getinteger(argv[2]);                    // get the x origin (optional) argument
	if(argc == 5) {
		gifyOrigin = getinteger(argv[4]);                    // get the y origin (optional) argument
		if(optiony) gifyOrigin=maxH-1-gifyOrigin;
	}

	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".GIF");
	giffnbr = FindFreeFileNbr();
    if(!BasicFileOpen(p, giffnbr, FA_READ)) return;
    GifTimer=0x5000;
    gif = gd_open_gif(giffnbr);
    frame = GetMemory(gif->width * gif->height * 3);
	ret=gd_get_frame(gif);
	routinechecks(1);
	int savey=optiony;
	optiony=0;
	if(ret!=-1){
		gd_render_frame(gif,frame);
    	routinechecks(1);
		DrawBuffer(gifxOrigin, gifyOrigin, gifxOrigin+gif->width-1, gifyOrigin+gif->height-1,(char *)frame,2);
    	routinechecks(1);
    	ret=gd_get_frame(gif);
	}
	if(ret<=0){
		FileClose(giffnbr);
		FreeMemorySafe((void*)&frame);
		gd_close_gif(gif);
		gif=NULL;
		GifTimer=0;
		giffnbr=0;
		return;
	}
    optiony=savey;
    GifTimer=gif->gce.delay*10;
}
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
static uint g_nInFileSize;
static uint g_nInFileOfs;
static int jpgfnbr;
unsigned char pjpeg_need_bytes_callback(unsigned char *pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
    uint n, n_read;
//    pCallback_data;

    n = min(g_nInFileSize - g_nInFileOfs, buf_size);
    f_read(FileTable[jpgfnbr].fptr, pBuf, n, &n_read);
    if (n != n_read)
        return PJPG_STREAM_READ_ERROR;
    *pBytes_actually_read = (unsigned char)(n);
    g_nInFileOfs += n;
    return 0;
}
void cmd_LoadJPGImage(char *p)
{
    pjpeg_image_info_t image_info;
    int mcu_x = 0;
    int mcu_y = 0;
    uint row_pitch;
    uint8_t status;
    gCoeffBuf = (int16_t *)GetTempMemory(8 * 8 * sizeof(int16_t));
    gMCUBufR = (uint8_t *)GetTempMemory(256);
    gMCUBufG = (uint8_t *)GetTempMemory(256);
    gMCUBufB = (uint8_t *)GetTempMemory(256);
    gQuant0 = (int16_t *)GetTempMemory(8 * 8 * sizeof(int16_t));
    gQuant1 = (int16_t *)GetTempMemory(8 * 8 * sizeof(int16_t));
    gHuffVal2 = (uint8_t *)GetTempMemory(256);
    gHuffVal3 = (uint8_t *)GetTempMemory(256);
    gInBuf = (uint8_t *)GetTempMemory(PJPG_MAX_IN_BUF_SIZE);
    g_nInFileSize = g_nInFileOfs = 0;

//    uint decoded_width, decoded_height;
    int xOrigin, yOrigin;

    // get the command line arguments
    getargs(&p, 5, ","); // this MUST be the first executable line in the function
    if (argc == 0)
        error("Argument count");
    if (!InitSDCard())
        return;

    p = getFstring(argv[0]); // get the file name

    xOrigin = yOrigin = 0;
    if (argc >= 3)
        xOrigin = getint(argv[2], 0, HRes - 1); // get the x origin (optional) argument
    if (argc == 5)
        yOrigin = getint(argv[4], 0, VRes - 1); // get the y origin (optional) argument

    // open the file
    if (strchr((char *)p, '.') == NULL)
        strcat((char *)p, ".jpg");
    jpgfnbr = FindFreeFileNbr();
    if (!BasicFileOpen((char *)p, jpgfnbr, FA_READ))
        return;

    g_nInFileSize = f_size(FileTable[jpgfnbr].fptr);
    status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, 0);

    if (status)
    {
        if (status == PJPG_UNSUPPORTED_MODE)
        {
            FileClose(jpgfnbr);
            error("Progressive JPEG files are not supported");
        }
        FileClose(jpgfnbr);
        error("pjpeg_decode_init() failed with status %", status);
    }
//    decoded_width = image_info.m_width;
//    decoded_height = image_info.m_height;

    row_pitch = image_info.m_MCUWidth * image_info.m_comps;

    unsigned char *imageblock = GetTempMemory(image_info.m_MCUHeight * image_info.m_MCUWidth * image_info.m_comps);
    for (;;)
    {
        uint8_t *pDst_row = imageblock;
        int y, x;

        status = pjpeg_decode_mcu();

        if (status)
        {
            if (status != PJPG_NO_MORE_BLOCKS)
            {
                FileClose(jpgfnbr);
                error("pjpeg_decode_mcu() failed with status %", status);
            }
            break;
        }

        if (mcu_y >= image_info.m_MCUSPerCol)
        {
            FileClose(jpgfnbr);
            return;
        }
        /*    for(int i=0;i<image_info.m_MCUHeight*image_info.m_MCUWidth ;i++){
                  imageblock[i*3+2]=image_info.m_pMCUBufR[i];
                  imageblock[i*3+1]=image_info.m_pMCUBufG[i];
                  imageblock[i*3]=image_info.m_pMCUBufB[i];
              }*/
        //         pDst_row = pImage + (mcu_y * image_info.m_MCUHeight) * row_pitch + (mcu_x * image_info.m_MCUWidth * image_info.m_comps);

        for (y = 0; y < image_info.m_MCUHeight; y += 8)
        {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));
            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
                uint8_t *pDst_block = pDst_row + x * image_info.m_comps;
                // Compute source byte offset of the block in the decoder's MCU buffer.
                uint src_ofs = (x * 8U) + (y * 16U);
                const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
                const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
                const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

                const int bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

                {
                    int bx, by;
                    for (by = 0; by < by_limit; by++)
                    {
                        uint8_t *pDst = pDst_block;

                        for (bx = 0; bx < bx_limit; bx++)
                        {
                            pDst[2] = *pSrcR++;
                            pDst[1] = *pSrcG++;
                            pDst[0] = *pSrcB++;
                            pDst += 3;
                        }

                        pSrcR += (8 - bx_limit);
                        pSrcG += (8 - bx_limit);
                        pSrcB += (8 - bx_limit);

                        pDst_block += row_pitch;
                    }
                }
            }
            pDst_row += (row_pitch * 8);
        }

        x = mcu_x * image_info.m_MCUWidth + xOrigin;
        y = mcu_y * image_info.m_MCUHeight + yOrigin;
        if (y < VRes && x < HRes)
        {
            int yend = min(VRes - 1, y + image_info.m_MCUHeight - 1);
            int xend = min(HRes - 1, x + image_info.m_MCUWidth - 1);
            if (xend < x + image_info.m_MCUWidth - 1)
            {
                // need to get rid of some pixels to remove artifacts
                xend = HRes - 1;
                unsigned char *s = imageblock;
                unsigned char *d = imageblock;
                for (int yp = 0; yp < image_info.m_MCUHeight; yp++)
                {
                    for (int xp = 0; xp < image_info.m_MCUWidth; xp++)
                    {
                        if (xp < xend - x + 1)
                        {
                            *d++ = *s++;
                            *d++ = *s++;
                            *d++ = *s++;
                        }
                        else
                        {
                            s += 3;
                        }
                    }
                }
            }
            if(yend>=yOrigin+image_info.m_height)yend=yOrigin+image_info.m_height-1;
            if(xend>=xOrigin+image_info.m_width){
                for(int yi=y;yi<yend;yi++){
                    uint8_t *ipoint=imageblock+3*image_info.m_MCUWidth*(yi-y);
                    DrawBuffer(x, yi, xOrigin+image_info.m_width-1, yi, (char *)ipoint,0);
                }
            } else DrawBuffer(x, y, xend, yend, (char *)imageblock,0);
        }

        if (y >= VRes)
        { // nothing useful left to process
            FileClose(jpgfnbr);
//            if (Option.Refresh)
//                Display_Refresh();
            return;
        }
        mcu_x++;
        if (mcu_x == image_info.m_MCUSPerRow)
        {
            mcu_x = 0;
            mcu_y++;
        }
    }
    FileClose(jpgfnbr);
//#ifdef USBKEYBOARD
	clearrepeat();
//#endif
//    if (Option.Refresh)
//        Display_Refresh();
}

/*void LoadJPG(char *p) {
	int fnbr, progressive=2;
	int xPos, yPos, x, y;
	uint8_t *from;
	char c, lastc=0;
	unsigned int nbr;

	// get the command line arguments
	getargs(&p, 5, ",");                                            // this MUST be the first executable line in the function
    if(WritePage==WPN)error("JPG load not available to the framebuffer");
    if(argc == 0) error("Argument count");
    if(!InitSDCard()) return;
	int maxH=PageTable[WritePage].ymax;
    int maxW=PageTable[WritePage].xmax;

    p = getFstring(argv[0]);                                        // get the file name

    xPos = yPos = 0;
	if(argc >= 3) xPos = getinteger(argv[2]);                    // get the x origin (optional) argument
	if(argc == 5){
		yPos = getinteger(argv[4]);                    // get the y origin (optional) argument
		if(optiony) yPos=maxH-1-yPos;
	}

	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".JPG");
	fnbr = FindFreeFileNbr();
    if(!BasicFileOpen(p, fnbr, FA_READ)) return;
    f_read(FileTable[fnbr].fptr, &c,1, &nbr);
    if(c!=0xff)error("Invalid file");
    f_read(FileTable[fnbr].fptr, &c,1, &nbr);
    if(c!=0xd8)error("Invalid file");
    while(progressive==2){
    	f_read(FileTable[fnbr].fptr, &c,1, &nbr);
    	if(lastc==0xFF && c==0xC0)progressive=0;
    	if(lastc==0xFF && c==0xC2)progressive=1;
    	lastc=c;
    }
    if(progressive){
        FileClose(fnbr);
        error("Progressive encoding not supported");
    } else {
    	f_rewind(FileTable[fnbr].fptr);
    }
	routinechecks(1);
    JPEG_Handle.Instance = JPEG;
    HAL_JPEG_Init(&JPEG_Handle);
    char *JPEG_OUTPUT_DATA_BUFFER=GetTempMemory(maxW*maxH*4);
	routinechecks(1);
    JPEG_DecodePolling(&JPEG_Handle, FileTable[fnbr].fptr, (uint32_t)JPEG_OUTPUT_DATA_BUFFER);
	routinechecks(1);

    //##-8- Get JPEG Info  ###############################################
    FileClose(fnbr);
    if(ImageWidth+xPos>maxW){
        FreeMemorySafe((void *)&JPEG_OUTPUT_DATA_BUFFER);
    	error("Horizontal image size %",ImageWidth);
    }
    if(ImageHeight+yPos>maxH){
        FreeMemorySafe((void *)&JPEG_OUTPUT_DATA_BUFFER);
    	error("Vertical image size %",ImageHeight);
    }
	int savey=optiony;
	optiony=0;
    int cursorhidden=0;
    if(cursoron){
		hidecursor(0);
		cursorhidden=1;
    }
    if(VideoColour!=8 && PageTable[WritePage].expand==0)DMA2D_CopyBuffer((uint32_t *)JPEG_OUTPUT_DATA_BUFFER, (uint32_t *)PageTable[WritePage].address, xPos>>1 , yPos>>1, ImageWidth, ImageHeight, ChromaSubsampling);
    else {
    	MPU_Config_nCacheable(1);
        char *LCD_FRAME_BUFFER=GetTempMemory(maxW*ImageHeight*2);
    	DMA2D_CopyBuffer((uint32_t *)JPEG_OUTPUT_DATA_BUFFER, (uint32_t *)LCD_FRAME_BUFFER, 0 , 0, ImageWidth, ImageHeight, ChromaSubsampling);
    	routinechecks(1);
        x=ImageWidth;
        if(xPos+ImageWidth>maxW)x=maxW-xPos;
		pagesetdone=0;
		while(!pagesetdone){
			CheckAbort();
		}
		for(y=0;y<ImageHeight;y++){
			if(y+yPos>=maxH)break;
			routinechecks(1);
			from=(uint8_t *)(LCD_FRAME_BUFFER+2*y*maxW);
			if(VideoColour==8)DrawBuffer(xPos,y+yPos,xPos+x,y+yPos,(char *)from,8);
			else DrawBufferFast(xPos,y+yPos,xPos+x,y+yPos,(char *)from);
		}
    	MPU_Config_nCacheable(0);
    }
    if(cursorhidden)showcursor(0, xcursor,ycursor);
    HAL_JPEG_DeInit(&JPEG_Handle);
    optiony=savey;
	clearrepeat();
}
*/



int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// ERROR HANDLING ////////////////////////////////////////////


void ErrorThrow(int e) {
    MMerrno = e;
    FSerror = e;
    MMErrMsg = (char *)FErrorMsg[e];
    if(e && OptionFileErrorAbort) error(MMErrMsg);
    return;
}


void ErrorCheck(int fnbr) {                                         //checks for an error, if fnbr is specified frees up the filehandle before sending error
    int e;
    e = (int)FSerror;
    if(fnbr != 0 && e != 0) ForceFileClose(fnbr);
    if(e >= 1 && e <= 19) ErrorThrow(ErrorMap[e]);
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// GENERAL I/O ////////////////////////////////////////////


void FileOpen(char *fname, char *fmode, char *ffnbr) {
    int fnbr;
    BYTE mode = 0;
    if(str_equal(fmode, "OUTPUT"))
        mode = FA_WRITE | FA_CREATE_ALWAYS;
    else if(str_equal(fmode, "APPEND"))
        mode = FA_WRITE | FA_OPEN_APPEND;
    else if(str_equal(fmode, "INPUT"))
        mode = FA_READ;
    else if(str_equal(fmode, "RANDOM"))
        mode = FA_WRITE | FA_OPEN_APPEND | FA_READ;
    else
        error("File access mode");

    if(*ffnbr == '#') ffnbr++;
    fnbr = getinteger(ffnbr);
    BasicFileOpen(fname, fnbr, mode);
}


// this performs the basic duties of opening a file, all file opens in MMBasic should use this
// it will open the file, set the FileTable[] entry and populate the file descriptor
// it returns with true if successful or false if an error
int BasicFileOpen(char *fname, int fnbr, int mode) {
    char pp[FF_MAX_LFN] = {0};
    char q[FF_MAX_LFN]={0};
    char *p=fname;
    int i,j;
    if(fnbr < 1 || fnbr > MAXOPENFILES) error("File number");
    if(FileTable[fnbr].com != 0) error("File number already open");
    if(!InitSDCard()) return false;
    	FileTable[fnbr].fptr = GetInternalMemory(sizeof(FIL));              // allocate the file descriptor
        i=strlen(p)-1;
        while(i>0 && !(p[i] == 92 || p[i]==47))i--;
        if(i>0){
        	memcpy(q,p,i);
             for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
        	if(q[1]==':')q[0]='0';
        	i++;
        }
        strcpy(pp,&p[i]);
        if((pp[0]==47 || pp[0]==92) && i==0){
        	strcpy(q,&pp[1]);
        	strcpy(pp,q);
        	strcpy(q,"0:/");
        }
        fullpath(q);
//       	MMPrintString("Was: ");MMPrintString(fname);PRet();
//       	MMPrintString("Path: ");MMPrintString(fullpathname);PRet();
//       	MMPrintString("File: ");MMPrintString(pp);PRet();
       	strcpy(q,fullpathname);
       	if(fullpathname[strlen(fullpathname)-1]!='/')strcat(q,"/");
       	strcat(q,pp);
//       	MMPrintString("Full: ");MMPrintString(q);PRet();
       	strcpy(fullfilepathname,q);
//       	MMPrintString(fullfilepathname);PRet();
        FSerror = f_open(FileTable[fnbr].fptr, q, mode);        // open it
        ErrorCheck(fnbr);
        clearrepeat();

    if(FSerror) {
        ForceFileClose(fnbr);
        return false;
    } else
        return true;
}


//close the file and free up the file handle
// it will generate an error if needed
void FileClose(int fnbr) {
    ForceFileClose(fnbr);
    ErrorThrow(FSerror);
}


//close the file and free up the file handle
// it will NOT generate an error
void ForceFileClose(int fnbr) {
    if(fnbr && FileTable[fnbr].fptr != NULL){
        FSerror = f_close(FileTable[fnbr].fptr);
        FreeMemorySafe((void *)(char *)&FileTable[fnbr].fptr);
//        FreeMemorySafe((void *)&SDbuffer[fnbr]);
//        buffpointer[fnbr]=0;
//        lastfptr[fnbr]=-1;
//        bw[fnbr]=-1;
//        fmode[fnbr]=0;
    }
}



char FileGetChar(int fnbr) {
    char ch;
    unsigned int i;
    FSerror = f_read(FileTable[fnbr].fptr, &ch,1, &i);
    		ErrorCheck(fnbr);
    return ch;
}



// bulk read data. For Fat file system can only be used if FileGetchar has not been called first
int FileGetData(int fnbr, void *buff, int count, unsigned int *read)
{
   // if (filesource[fnbr] == FATFSFILE)
    {
        FSerror = f_read(FileTable[fnbr].fptr, buff, count, (UINT *)read);
    }
  //  else
  //  {
  //      FSerror = lfs_file_read(&lfs, FileTable[fnbr].lfsptr, buff, count);
  //      *read = FSerror;
  //      if (FSerror > 0)
  //          FSerror = 0;
   //     ErrorCheck(fnbr);
   // }
    return FSerror;
}

void FilePutStr(int count, char *c, int fnbr){
    unsigned int bw;
    InitSDCard();
    FSerror = f_write(FileTable[fnbr].fptr, c, count, &bw);
    ErrorCheck(fnbr);
}


char FilePutChar(char c, int fnbr) {
    static char t;
    unsigned int bw;
    t = c;
    if(!InitSDCard()) return 0;
    FSerror = f_write(FileTable[fnbr].fptr, &t, 1, &bw);
    ErrorCheck(fnbr);
    return t;
}

void FilePutData(char *c, int fnbr, int n){
        unsigned int bw;
        if (!InitSDCard()) return ;
        FSerror = f_write(FileTable[fnbr].fptr, c, n, &bw);
        ErrorCheck(fnbr);

   // }
}

int FileEOF(int fnbr) {
	return f_eof(FileTable[fnbr].fptr);
}

int InitSDCard(void) {
    int i;
    ErrorThrow(0);    // reset mm.errno to zero
    if(!MDD_SDSPI_CardDetectState()) { ErrorThrow(20); return false; }  // error if the card is not present
    if(!(SDCardStat & STA_NOINIT))
    	return 1;                     // if the card is present and has been initialised we have nothing to do
    for(i = 0; i < MAXOPENFILES; i++)
        if(FileTable[i].com > MAXCOMPORTS)
            if(FileTable[i].fptr != NULL)
                ForceFileClose(i);
    i = f_mount(&FatFs, "", 1);
    if(i) { ErrorThrow(ErrorMap[i]); return false; }
    return 2;
}



// finds the first available free file number.  Throws an error if no free file numbers
int FindFreeFileNbr(void) {
    int i;
    for(i = MAXOPENFILES; i > 0; i--)
        if(FileTable[i].com == 0) return i;
    error("Too many files open");
    return 0;
}


// check the SD card to see if it has been removed.  Also check WAV playback
// this is called from cmd_pause(), the main ExecuteProgram() loop and the console's MMgetchar()
void CheckSDCard(void) {
	if(gif==NULL && CurrentlyPlaying == P_NOTHING && CurrentLinePtr)return;
	if(!MDD_SDSPI_CardDetectState()) {
		if(!SDCardStat & STA_NOINIT){
			mymemset(&FatFs,0,sizeof(FATFS));
			SDCardStat = STA_NOINIT | STA_NODISK;
			mymemset(filepath,0,sizeof(filepath));
			strcpy(filepath,"A:/");
		}
	} else {
		if(CurrentlyPlayinge == P_WAV || CurrentlyPlaying == P_WAV || CurrentlyPlaying == P_FLAC || CurrentlyPlaying == P_MP3 || CurrentlyPlaying == P_MOD)
			checkWAVinput();
		if(GifTimer==0 && gif!=NULL){
			GifTimer=5000;
			GIFcallback();
		}
	}
}



