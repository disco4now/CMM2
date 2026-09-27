/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

CAN.c

Handles the CAN command.

Copyright 2011-2026 Geoff Graham and  Peter Mather.
Copyright 2024-2026      Gerry Allardice.

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
/********** CAN implementation MMBasic on Armmite H7 *********************************

 *
 *  The FDCAN clock is 40Mhz derived from PLL2
    i.e. PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;   //CAN Added
    https://kvaser.com/support/calculators/can-fd-bit-timing-calculator/
 *
 * Commands to interface the CAN
 * CAN OPEN canopen,speed,mode
 * CAN CLOSE
 * CAN START
 * CAN STOP
 * CAN FILTER index,idtype,type,config,id1,id2
 * CAN GLOBAL id,eid,idr,eidr
 * CAN SEND id,eid,dlc,msg,ret
 * CAN READ fifo,id,eid,rtr,dlc,msg,filterno,ret
 * **************************************************************
 * canopen   HAS_144PINS Pin allocations  FDCANx  Shares pins with
 *           CANL        CANH             Used
 * -------   ---------- ----------------  -------- --------------
 * 1         PD0/114     PD1/115          FDCAN1      -
 * 2         PB8/139     PB9/140          FDCAN1    I2C
 * 3         PB12/73     PB13/74          FDCAN2    COM3
 * 4
 * 5
 * 6
 * 7         LOOPBACK                     FDCAN1    n/a
 * 8         LOOPBACK                     FDCAN1    n/a
 * 9         LOOPBACK                     FDCAN2    n/a
 * **************************************************************
 * canopen   HAS_100PINS Pin allocations  FDCANx  Shares pins with
 *           CANL        CANH             Used
 * -------   ---------- ----------------  -------- --------------
 * 1         PD0/81      PD1/82           FDCAN1    COUNT1/2
 * 2         PB8/95      PB9/96           FDCAN1    I2C
 * 3         PB12/51     PB6/92           FDCAN2    COM3
 * 4
 * 5
 * 6
 * 7         LOOPBACK                     FDCAN1    n/a
 * 8         LOOPBACK                     FDCAN1    n/a
 * 9         LOOPBACK                     FDCAN2    n/a
 *
 ***************************************************************
 * canopen   CMM2      Pin allocations    FDCANx  Shares pins with
 *           CAN RX      CAN TX           Used
 * -------   ---------- ----------------  -------- --------------
 * 1         PB8/5       PB9/3            FDCAN1    I2C SCK/SDA
 * 2         PB5/19      PB13/40          FDCAN2    SPI MOSI/SPI2 CLK
 * 3
 * 4
 * 5
 * 6
 * 7         LOOPBACK                     FDCAN1    n/a
 * 8         LOOPBACK                     FDCAN2    n/a
 * 9
 **********************************************************************************/

  /* Message RAM configuration:
    ***************************
                ________
    0x4000AC00 |________| Standard_filter[0]
    0x4000AC04 |________| Standard_filter[1]
    0x4000AC08 |||||||||| Rx_buffer[0]
               ||||||||||
               ||||||||||
               ||||||||||
    0x4000AC18 |________| Rx_buffer[1]
               |________|
               |________|
               |________|
    0x4000AC28 |||||||||| Tx_buffer[0]
               ||||||||||
               ||||||||||
    0x4000AC34 ||||||||||
  */

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "CAN.h"


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern FDCAN_HandleTypeDef hfdcan;
FDCAN_ClkCalUnitTypeDef sCcuConfig;
FDCAN_FilterTypeDef sFilterConfig;
FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_RxHeaderTypeDef RxHeader;


/*

/3. Start the FDCAN module using HAL_FDCAN_Start function. At this level the node is active on the bus: it can
send and receive messages.
4. The following Tx control functions can only be called when the FDCAN module is started:
– HAL_FDCAN_AddMessageToTxFifoQ
– HAL_FDCAN_EnableTxBufferRequest
– HAL_FDCAN_AbortTxRequest
5. After having submitted a Tx request in Tx Fifo or Queue, it is possible to get Tx buffer location used to place
the Tx request thanks to HAL_FDCAN_GetLatestTxFifoQRequestBuffer API. It is then possible to abort later
on the corresponding Tx Request using HAL_FDCAN_AbortTxRequest API.
6. When a message is received into the FDCAN message RAM, it can be retrieved using the
HAL_FDCAN_GetRxMessage function.
7. Calling the HAL_FDCAN_Stop function stops the FDCAN module by entering it to initialization mode and
re-enabling access to configuration registers through the configuration functions listed here above.
8. All other control functions can be called any time after initialization phase, no matter if the FDCAN module is
started or stopped.
*/
char canmode=0;	  //CAN mode not set.
char datasize=8;
char brs=0;
char fdcan=0;
//static const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
void cmd_can(void) {
	int speed,cansave;
    char *p,*tp;

    if(checkstring(cmdline, "CLOSE")) {

        /* Stop the FDCAN module */
        HAL_FDCAN_Stop(&hfdcan);
    	HAL_FDCAN_DeInit(&hfdcan);


      	if (canopen==1) {
    		if(ExtCurrentConfig[CAN_1A_RX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_1A_RX, EXT_NOT_CONFIG, 0);   // reset to not in use
    		if(ExtCurrentConfig[CAN_1A_TX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_1A_TX, EXT_NOT_CONFIG, 0);
    	}
      	if (canopen==2 ) {
      	   if(ExtCurrentConfig[CAN_2A_RX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_2A_RX, EXT_NOT_CONFIG, 0);   // reset to not in use
      	   if(ExtCurrentConfig[CAN_2A_TX] != EXT_BOOT_RESERVED)  ExtCfg(CAN_2A_TX, EXT_NOT_CONFIG, 0);
      	}


      	canopen=0;
        return;
    }

    if(checkstring(cmdline, "START")) {
    	if (!canopen) error("CAN not open");
        /* Start the FDCAN module */
        HAL_FDCAN_Start(&hfdcan);
        return;
    }

    if(checkstring(cmdline, "STOP")) {
    	if (!canopen) error("CAN not open");
        /* Stop the FDCAN module */
        HAL_FDCAN_Stop(&hfdcan);
        return;
    }

    /* CAN SEND id,eid,rtr,dlc,msg,ret             Classical
     * CAN SEND id,eid,rtr,dlc,msg(),ret           FD CAN
     * Adds one message to the Fifo TXBuffer.
     * Returns 0 if  message is added.
     */
    if((p = checkstring(cmdline, "SEND")) != NULL) {

    	 if (!canopen) error("CAN not open");
    	 ALIGN_32BYTES(uint8_t TxData[datasize]);
    	 int i;
    	 int *ret,eid,rtr,dlc;
    	 uint32_t id;

    	 void *ptr1 = NULL;
    	 uint8_t *msg=NULL;

         getargs(&p, 11, ",");
    	 if(!(argc == 11 )) error("Incorrect argument count");
    	 eid=getint(argv[2],0,1);
    	 if(eid){
    	     id=getint(argv[0],0,0x1FFFFFFF);
    	 }else{
    		 id=getint(argv[0],0,0x7FF);
    		 //MMPrintString("ID=");PIntHC(id);PRet();
    	 }
    	 rtr=getint(argv[4],0,1);
       	 dlc=getint(argv[6],0,64);
       	 if (dlc>datasize)error("dlc exceeds configured datasize");
    	// mybuff.iTxBuffer=getinteger(argv[8]);
    	// msgp=getinteger(argv[8]);
    	// *msg=msgp;
    	 ptr1 = findvar(argv[8], V_FIND | V_EMPTY_OK);
    	 msg = (uint8_t *)ptr1;

    	 ret = findvar(argv[10], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for ret");


    	 //TxHeader.Identifier = 0x111;
    	 TxHeader.Identifier = id;
    	 if (eid){
    		TxHeader.IdType = FDCAN_EXTENDED_ID;
    	 }else{
    		 TxHeader.IdType = FDCAN_STANDARD_ID;
    	 }
    	 if(rtr){
    		 TxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
    	    		// TxHeader.TxFrameType=1;
    	 }else{
    		 TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    	    //		 TxHeader.TxFrameType=0;
    	 }
    	 //FDCAN_REMOTE_FRAME
    	 //static const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

    	 if (dlc==12){
    		 TxHeader.DataLength = FDCAN_DLC_BYTES_12;
    	 }else if (dlc==16){
    		 TxHeader.DataLength = FDCAN_DLC_BYTES_16;
     	 }else if (dlc==20){
    		 TxHeader.DataLength = FDCAN_DLC_BYTES_20;
    	 }else if (dlc==24){
    		 TxHeader.DataLength = FDCAN_DLC_BYTES_24;
       	 }else if (dlc==32){
    		 TxHeader.DataLength = FDCAN_DLC_BYTES_32;
    	     //TxHeader.DataLength =(13<<16);
    	 }else if (dlc==48){
    		 TxHeader.DataLength = FDCAN_DLC_BYTES_48;
     	 }else if (dlc==64){
    		 TxHeader.DataLength = FDCAN_DLC_BYTES_64;
    	 }else{
    		 TxHeader.DataLength =(dlc<<16);
       	 }

    	 TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    	 if (fdcan==1){
    	     TxHeader.FDFormat = FDCAN_FD_CAN;
    	     TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;//FDCAN_STORE_TX_EVENTS;
    	     if (brs){
    	  		 TxHeader.BitRateSwitch = FDCAN_BRS_ON;  //FDCAN_BRS_OFF  FDCAN_BRS_ON
    	     }else{
    	   	     TxHeader.BitRateSwitch = FDCAN_BRS_OFF;  //FDCAN_BRS_OFF  FDCAN_BRS_ON
    	   	 }
    	 }else{
    		 TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    		 TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    	 }




    	 //TxHeader.TxEventFifoControl = FDCAN_STORE_TX_EVENTS;
    	 //TxHeader.MessageMarker = 0x52;
    	 /* reverse the passed in data  */
    	 //MMPrintString("SEND");PRet();
    	 if (fdcan==0){
    		 for (i=0;i<8;i++){TxData[7-i]=msg[i];}
    	 }else{
    	   for (i=0;i<dlc;i++){
    		// TxData[(datasize-1)-i]=0;
    		 TxData[(dlc-1)-i]=msg[i];
    		// PIntHC(msg[i]);
    	   }
    	 }
    	// PRet();
    	 if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &TxHeader, TxData)!=0){;
    	   *ret=1;
    	 }else{
    	   *ret=0;
    	 }
    	 return;
    }



    /* CAN READ fifo,id,eid,rtr,dlc,msg,fmi,ret         Classical
     * CAN READ fifo,id,eid,rtr,dlc,msg(),fmi,ret       FD CAN
     * Reads one message from the nominated Fifo RXBuffer.
     * Returns 0 if no message is available, else returns the number of messages.
     */
    if((p = checkstring(cmdline, "READ")) != NULL) {
    	 if (!canopen) error("CAN not open");
    	 ALIGN_32BYTES( uint8_t RxData[datasize]);
    	 int i,fifo,adj;
    	 int *ret,*eid,*dlc,*rtr,*fmi;
    	 uint32_t *id;
    	 uint8_t *msg;


         getargs(&p, 15, ",");
    	 if(!(argc == 15 )) error("Incorrect argument count");
    	 fifo=getint(argv[0],0,1);
    	 // get the  variables
    	 id = findvar(argv[2], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for id");
    	 eid = findvar(argv[4], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for eid");
    	 rtr = findvar(argv[6], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for rtr");
    	 dlc = findvar(argv[8], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for dl");
    	 msg = findvar(argv[10], V_FIND | V_EMPTY_OK);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for msg");
    	 fmi = findvar(argv[12], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for fmi");
    	 ret = findvar(argv[14], V_FIND);
    	 if(!(vartbl[VarIndex].type & T_INT)) error("Invalid variable for ret");
    	 adj=0;
        if (fifo==0){
    	    /* Retrieve next message from Rx FIFO 0 */
    	    *ret= HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0);
        	if (*ret){
       	          HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData);
       	          // PIntH(RxHeader.Identifier);
       	          *id=RxHeader.Identifier;
       	          if(!RxHeader.IsFilterMatchingFrame){
       	        	//PInt(RxHeader.FilterMatchIndex) ;
       	       		//*fmi=(RxHeader.FilterIndex && 0x7f);
       	        	*fmi=RxHeader.FilterIndex;
       	          }else{
       	       		*fmi=99;
       	          }
				  *rtr=RxHeader.RxFrameType;
				  *dlc=DLCtoBytes[(RxHeader.DataLength>>16)];
				  if(*dlc==12 || *dlc==20)adj=4;
				  if (RxHeader.IdType==FDCAN_EXTENDED_ID){*eid=1;}else{*eid=0;}

			  // if (fdcan==1){
			   if (*dlc>8){
       	 		 for (i=0;i<*dlc;i++){msg[(*dlc-1)-i+adj]=RxData[i];}
       	 	     if (adj==4) for (;i<*dlc+4;i++){msg[(*dlc-1)-i+adj]=0;}
       	 	   }else{
       	 	    //if (*dlc<8)for (i=*dlc;i<8;i++){msg[7-i]=0;}
       	 	    for (i=0;i<8;i++){msg[7-i]=RxData[i];}
       	 	    for (i=*dlc;i<8;i++){msg[7-i]=0;}
       	 	   }
    	    }

        }else{
        	/* Retrieve next message from Rx FIFO 1 */
        	*ret= HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO1);
        	if (*ret){
         	   	   HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO1, &RxHeader, RxData);
        	       //PIntH(RxHeader.Identifier);PRet();
        	   	   *id=RxHeader.Identifier;
        	   	   if(!RxHeader.IsFilterMatchingFrame){
        	   		//PInt(RxHeader.FilterIndex) ;
        	   		*fmi=RxHeader.FilterIndex;
        	   		//*fmi=(RxHeader.FilterIndex && 0x7f);
        	   	   }else{
        	   		*fmi=99;
        	   	   }
        	 	   *rtr=RxHeader.RxFrameType;
        	 	   *dlc=DLCtoBytes[(RxHeader.DataLength>>16)];
        	 	   if(*dlc==12 || *dlc==20)adj=4;
        	 	   if (RxHeader.IdType==FDCAN_EXTENDED_ID){*eid=1;}else{*eid=0;}

        	 	  // if (fdcan==1){
        	 	   if (*dlc>8){
        	 		 for (i=0;i<*dlc;i++){msg[(*dlc-1)-i+adj]=RxData[i];}
        	 	     if (adj==4) for (;i<*dlc+4;i++){msg[(*dlc-1)-i+adj]=0;}
        	 	   }else{
        	 	    //if (*dlc<8)for (i=*dlc;i<8;i++){msg[7-i]=0;}
        	 	    for (i=0;i<8;i++){msg[7-i]=RxData[i];}
        	 	    for (i=*dlc;i<8;i++){msg[7-i]=0;}
        	 	   }
             }

        }
        return;
    }



    /*CAN FILTER idtype,index,type,config,id1,id2
     * type:    FDCAN_FILTER_RANGE
     * config:  FDCAN_FILTER_TO_RXFIFO0 FDCAN_FILTER_TO_RXFIFO1 FDCAN_FILTER_TO_RXFIFO0
     *
     * type values
     * ignored if FilterConfig is set to FDCAN_FILTER_TO_RXBUFFER
     * FDCAN_FILTER_RANGE         ((uint32_t)0x00000000U)  !< Range filter from FilterID1 to FilterID2
     * FDCAN_FILTER_DUAL          ((uint32_t)0x00000001U)  !< Dual ID filter for FilterID1 or FilterID2
     * FDCAN_FILTER_MASK          ((uint32_t)0x00000002U)  !< Classic filter: FilterID1 = filter, FilterID2 = mask
     * FDCAN_FILTER_RANGE_NO_EIDM ((uint32_t)0x00000003U)  !< Range filter from FilterID1 to FilterID2, EIDM mask not applied

     * Config values
     * FDCAN_FILTER_DISABLE       ((uint32_t)0x00000000U)  !< Disable filter element
     * FDCAN_FILTER_TO_RXFIFO0    ((uint32_t)0x00000001U)  !< Store in Rx FIFO 0 if filter matches
     * FDCAN_FILTER_TO_RXFIFO1    ((uint32_t)0x00000002U)  !< Store in Rx FIFO 1 if filter matches
     * FDCAN_FILTER_REJECT        ((uint32_t)0x00000003U)  !< Reject ID if filter matches
     * ----------------These not supported -----------------------------------------------------------
     * FDCAN_FILTER_HP            ((uint32_t)0x00000004U)  !< Set high priority if filter matches
     * FDCAN_FILTER_TO_RXFIFO0_HP ((uint32_t)0x00000005U)  !< Set high priority and store in FIFO 0 if filter matches
     * FDCAN_FILTER_TO_RXFIFO1_HP ((uint32_t)0x00000006U)  !< Set high priority and store in FIFO 1 if filter matches
     * ----------------This one in future ?????? -----------------------------------------------------
     * FDCAN_FILTER_TO_RXBUFFER   ((uint32_t)0x00000007U)  !< Store into Rx Buffer, configuration of FilterType ignored
     *
     * FDCAN_STANDARD_ID ((uint32_t)0x00000000U)  !< Standard ID element
     * FDCAN_EXTENDED_ID ((uint32_t)0x40000000U)  !< Extended ID element

     CAN FILTER idtype,index,type,config,id1,id2
     CAN FILTER GLOBAL id,eid,idr,eidr

       Values for id and eid                values for idr and eidr
       ---------------------                -----------------------
       0=FDCAN_ACCEPT_IN_RX_FIFO0           0=FDCAN_FILTER_REMOTE
       1=FDCAN_ACCEPT_IN_RX_FIFO1		    1=FDCAN_REJECT_REMOTE
       2=FDCAN_REJECT
     */

    if((p = checkstring(cmdline, "FILTER")) != NULL) {
       if (!canopen) error("CAN not open");
       if((tp = checkstring(p, "GLOBAL"))){
    	    uint32_t id,eid,idr,eidr;
        	getargs(&tp, 7, ",");
        	if(!(argc == 7 )) error("Incorrect argument count");
        	id=getint(argv[0],0,2);
        	eid=getint(argv[2],0,2);
        	idr=getint(argv[4],0,1);
        	eidr=getint(argv[6],0,1);
        	HAL_FDCAN_ConfigGlobalFilter(&hfdcan, id, eid, idr, eidr);
            return;

       }

     	uint32_t index,idtype,type,config,id1,id2;
        getargs(&p, 11, ",");
       	if(!(argc == 11 )) error("Incorrect argument count");
     	index=getint(argv[0],0,20);
     	idtype=getint(argv[2],0,1);
       	type=getint(argv[4],0,3);
     	config=getint(argv[6],0,3);
        if(idtype){
     		id1=getint(argv[8],0,0x1FFFFFFF);
     		id2=getint(argv[10],0,0x1FFFFFFF);
      	}else{
     		id1=getint(argv[8],0,0x7FF);
     		id2=getint(argv[10],0,0x7FF);

     	}

    	/* Configure filter */
       	if(idtype){
        	sFilterConfig.IdType = FDCAN_EXTENDED_ID;
       	}else{
       		sFilterConfig.IdType = FDCAN_STANDARD_ID;
       	}
    	sFilterConfig.FilterIndex = index;
       	sFilterConfig.FilterType = type;
    	sFilterConfig.FilterConfig = config;
    	sFilterConfig.FilterID1 = id1;
    	sFilterConfig.FilterID2 = id2;
    	HAL_FDCAN_ConfigFilter(&hfdcan, &sFilterConfig);


    	//HAL_FDCAN_ConfigGlobalFilter(&hfdcan, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    	return;


    }

    /*************************************************************************
     * CAN GLOBAL id,eid,idr,eidr

       Values for id and eid                values for idr and eidr
       ---------------------                -----------------------
       0=FDCAN_ACCEPT_IN_RX_FIFO0           0=FDCAN_FILTER_REMOTE
       1=FDCAN_ACCEPT_IN_RX_FIFO1		    1=FDCAN_REJECT_REMOTE
       2=FDCAN_REJECT

    *************************************************************************/
  /*
    if((p = checkstring(cmdline, "GLOBAL")) != NULL) {
    	if (!canopen) error("CAN not open");
       	uint32_t id,eid,idr,eidr;
       	getargs(&p, 7, ",");
       	if(!(argc == 7 )) error("Incorrect argument count");
       	id=getint(argv[0],0,2);
       	eid=getint(argv[2],0,2);
       	idr=getint(argv[4],0,1);
       	eidr=getint(argv[6],0,1);
       	HAL_FDCAN_ConfigGlobalFilter(&hfdcan, id, eid, idr, eidr);
        return;
    }
*/

/*

 FDCAN_FRAME_CLASSIC   ((uint32_t)0x00000000U)                             Classic mode
 FDCAN_FRAME_FD_NO_BRS ((uint32_t)FDCAN_CCCR_FDOE)                         FD mode without BitRate Switching
 FDCAN_FRAME_FD_BRS    ((uint32_t)(FDCAN_CCCR_FDOE | FDCAN_CCCR_BRSE))     FD mode with BitRate Switching

 FDCAN_MODE_NORMAL               ((uint32_t)0x00000000U)                   Normal mode
 FDCAN_MODE_RESTRICTED_OPERATION ((uint32_t)0x00000001U)                   Restricted Operation mode
 FDCAN_MODE_BUS_MONITORING       ((uint32_t)0x00000002U)                   Bus Monitoring mode
 FDCAN_MODE_INTERNAL_LOOPBACK    ((uint32_t)0x00000003U)                   Internal LoopBack mode
 FDCAN_MODE_EXTERNAL_LOOPBACK    ((uint32_t)0x00000004U)                   External LoopBack mode

*/
/*		                       5                       13                                        25
		CAN OPEN index,speed,mode[,prescale,seg1,seg2,sjw]	[,brs,datamax,dprescale,dseg1,dseg2,dsjw]

*/
 if((p = checkstring(cmdline, "OPEN")) != NULL) {
	    int prescaler,seg1,seg2,sjw,dprescaler,dseg1,dseg2,dsjw;
	    int usecan2=0,i;
	    getargs(&p, 25, ",");
        if (canopen) error("Already open");

        datasize=8; //datasize
        brs=0;     //No BRS
        prescaler=1;seg1=0;seg2=0;sjw=0;dprescaler=1;dseg1=0;dseg2=0;dsjw=0;


        if(argc < 5) error("Incorrect argument count");

        //canopen=getinteger(argv[0]);
        //speed=getint(argv[2],125000,1000000);
        speed = getinteger(argv[2]);
        if (!(speed==125000||speed==250000||speed==500000||speed==1000000||speed==0))error("Valid speeds are 0,125000,250000,5000000,1000000");
        if (speed==0 && argc < 13)error("Incorrect argument count for speed 0");
           if (speed==0){
                prescaler = getinteger(argv[6]);
               	seg1 = getinteger(argv[8]);
               	seg2 = getinteger(argv[10]);
               	sjw = getinteger(argv[12]);
               	if(argc != 13 && argc !=25)error("Incorrect argument count for speed 0");
               if (argc==25){
            	  fdcan=1;
                  brs=getinteger(argv[14]);
                  datasize=getinteger(argv[16]);
               	  dprescaler = getinteger(argv[18]);
               	  dseg1 = getinteger(argv[20]);
               	  dseg2 = getinteger(argv[22]);
                  dsjw = getinteger(argv[24]);

              }


           }
        canmode = getint(argv[4],0,3);
        canopen=getint(argv[0],1,2);

    	    if (canopen==1  ) {
    	       cansave=canopen;canopen=0;
    	       CheckPin(CAN_1A_RX, CP_CHECKALL);
    	       CheckPin(CAN_1A_TX, CP_CHECKALL);
               ExtCfg(CAN_1A_RX, EXT_COM_RESERVED, 0);
               ExtCfg(CAN_1A_TX, EXT_COM_RESERVED, 0);
               canopen=cansave;
            }
    	    if (canopen==2 ) {
    	       usecan2=1;
    	       cansave=canopen;canopen=0;
    	       CheckPin(CAN_2A_RX, CP_CHECKALL);
    	       CheckPin(CAN_2A_TX, CP_CHECKALL);
               ExtCfg(CAN_2A_RX, EXT_COM_RESERVED, 0);
               ExtCfg(CAN_2A_TX, EXT_COM_RESERVED, 0);
               canopen=cansave;
            }



        /* Initializes the FDCAN peripheral */
       if (usecan2){ hfdcan.Instance = FDCAN2;}else{hfdcan.Instance = FDCAN1;}

       if (fdcan==0){
    	   hfdcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC; //FDCAN_FRAME_FD_BRS; FDCAN_FRAME_FD_BNO_BRS
       }else{
          if(brs){ hfdcan.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;}else{hfdcan.Init.FrameFormat = FDCAN_FRAME_FD_BRS;}
       }
       if(canmode==1){
         hfdcan.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;   //FDCAN_MODE_INTERNAL_LOOPBACK
       }else if(canmode==2){
          hfdcan.Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;  //FDCAN_MODE_INTERNAL_LOOPBACK
       }else if(canmode==3){
          hfdcan.Init.Mode =  FDCAN_MODE_BUS_MONITORING;  //FDCAN_MODE_INTERNAL_LOOPBACK
       }else{
         hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
       }
       hfdcan.Init.AutoRetransmission = ENABLE;
       hfdcan.Init.TransmitPause = DISABLE;
       hfdcan.Init.ProtocolException = ENABLE;

       //hfdcan.Init.NominalPrescaler = 0x1; /* tq = NominalPrescaler x (1/fdcan_ker_ck)
/*       The Armmite H7 sets the fdcan_ker_ck at 40MHz.  */

       /* Bit time configuration: 40MHz for CAN 1 Mbit/s
          fdcan_ker_ck               = 40 MHz
          Time_quantum (tq)          = 25 ns
          Synchronization_segment    = 1 tq
          Propagation_segment        = 23 tq
          Phase_segment_1            = 8 tq
          Phase_segment_2            = 8 tq
          Synchronization_Jump_width = 8 tq
          Bit_length                 = 40 tq = 1 µs
          Bit_rate                   = 1 MBit/s
        */


/**** Bus Timing Parameters for FDCAN  Clock Frequency 40MHz on H7 *****************
     *             Nominal sample point 80%               DATA sample point at 60%
       Speed       Pre    Seg1  Seg2 JSW  Sample   Data-> Pre TSeg1 TSeg2 SJW  Sample
		KHz        Scale                  Point          Scale                 point
	   -------     -----  ----  ---  ---  ---             ---  ---   ---  ---  ---
       125/125	 	1     255	 64	  64  80%              1   191	 128  128  60%
       250/250	 	1	  127    32   32  80%              1    95	  64   64  60%
	   500/500	 	1	   63    16   16  80%	           1    47	  32   32  60%
      1000/1000 	1	   31     8	   8  80%              1    23	  16   16  60%
       500/2000 	1	   63    16   16  80%              1    11	   8    8  60%
      1000/2000 	1	   31     8	   8  80%              1    11	   8    8  60%
***********************************************************************************/
  if (speed==125000){
       hfdcan.Init.NominalPrescaler = 1;     /* tq = NominalPrescaler x (1/fdcan_ker_ck) */
       hfdcan.Init.NominalTimeSeg1 = 255;    /* NominalTimeSeg1 = Propagation_segment + Phase_segment_1 */
       hfdcan.Init.NominalTimeSeg2 = 64;
       hfdcan.Init.NominalSyncJumpWidth = 64;
      // MMPrintString("Speed 125000 \r\n");
  }else if (speed==250000){
      hfdcan.Init.NominalPrescaler = 1;     /* tq = NominalPrescaler x (1/fdcan_ker_ck) */
      hfdcan.Init.NominalTimeSeg1 = 127;    /* NominalTimeSeg1 = Propagation_segment + Phase_segment_1 */
      hfdcan.Init.NominalTimeSeg2 = 32;
      hfdcan.Init.NominalSyncJumpWidth =32;
     // MMPrintString("Speed 250000 \r\n");
  }else if (speed==500000){
      hfdcan.Init.NominalPrescaler = 1;     /* tq = NominalPrescaler x (1/fdcan_ker_ck) */
      hfdcan.Init.NominalTimeSeg1 = 63;    /* NominalTimeSeg1 = Propagation_segment + Phase_segment_1 */
      hfdcan.Init.NominalTimeSeg2 = 16;
      hfdcan.Init.NominalSyncJumpWidth =16;
     // MMPrintString("Speed 500000 \r\n");
  }else if (speed==10000000){
      hfdcan.Init.NominalPrescaler = 1;     /* tq = NominalPrescaler x (1/fdcan_ker_ck) */
      hfdcan.Init.NominalTimeSeg1 = 31;    /* NominalTimeSeg1 = Propagation_segment + Phase_segment_1 */
      hfdcan.Init.NominalTimeSeg2 = 8;
      hfdcan.Init.NominalSyncJumpWidth =8;
     // MMPrintString("Speed 1000000 \r\n");
  }else if (speed==0){
       hfdcan.Init.NominalPrescaler = prescaler;     /* tq = NominalPrescaler x (1/fdcan_ker_ck) */
       hfdcan.Init.NominalTimeSeg1 = seg1;    /* NominalTimeSeg1 = Propagation_segment + Phase_segment_1 */
       hfdcan.Init.NominalTimeSeg2 = seg2;
       hfdcan.Init.NominalSyncJumpWidth =sjw;
       if(brs){
          hfdcan.Init.DataPrescaler = dprescaler;
       	  hfdcan.Init.DataTimeSeg1 = dseg1;    /* DataTimeSeg1 = Propagation_segment + Phase_segment_1 */
       	  hfdcan.Init.DataTimeSeg2 = dseg2;
       	  hfdcan.Init.DataSyncJumpWidth = dsjw;
       	 // MMPrintString("BRS=1   \r\n");
       }
      // MMPrintString("Speed 0 \r\n");
  }

//#define FDCAN_DATA_BYTES_8  ((uint32_t)0x00000004U) /*!< 8 bytes data field  */
//#define FDCAN_DATA_BYTES_12 ((uint32_t)0x00000005U) /*!< 12 bytes data field */
//#define FDCAN_DATA_BYTES_16 ((uint32_t)0x00000006U) /*!< 16 bytes data field */
//#define FDCAN_DATA_BYTES_20 ((uint32_t)0x00000007U) /*!< 20 bytes data field */
//#define FDCAN_DATA_BYTES_24 ((uint32_t)0x00000008U) /*!< 24 bytes data field */
//#define FDCAN_DATA_BYTES_32 ((uint32_t)0x0000000AU) /*!< 32 bytes data field */
//#define FDCAN_DATA_BYTES_48 ((uint32_t)0x0000000EU) /*!< 48 bytes data field */
//#define FDCAN_DATA_BYTES_64 ((uint32_t)0x00000012U) /*!< 64 bytes data field */
       hfdcan.Init.MessageRAMOffset = 0;
       hfdcan.Init.StdFiltersNbr = 32;   //max 128  0-128 words         32
       hfdcan.Init.ExtFiltersNbr = 32;   //max  64  0-128 words         64

       hfdcan.Init.RxFifo0ElmtsNbr = 32; //max 64   0-1152 words       576 if dlc=64
       hfdcan.Init.RxFifo1ElmtsNbr = 32; //max 64   0-1152 words       576 if dlc=64

       hfdcan.Init.RxBuffersNbr = 0;     //max 64   0-1152 words

       hfdcan.Init.TxEventsNbr = 0;
       hfdcan.Init.TxBuffersNbr = 0;     //max 32   0-576  words

       hfdcan.Init.TxFifoQueueElmtsNbr = 32; //Max 32 0-576 words      576 if dlc=64
       hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;      // 1824 total of 2560 available
     if( datasize==8){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
     } else if( datasize==12){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_12;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_12;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_12;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_12;
     } else if( datasize==16){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_16;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_16;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_16;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_16;
     } else if( datasize==20){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_20;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_20;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_20;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_20;
     } else if( datasize==24){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_24;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_24;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_24;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_24;
     } else if( datasize==32){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_32;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_32;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_32;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_32;
     } else if( datasize==48){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_48;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_48;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_48;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_48;
     } else if( datasize==64){
       hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_64;
       hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_64;
       hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_64;
       hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_64;
     } else  {
    	error("Invalid datasize");
     }

       /**************************/
       HAL_FDCAN_DeInit(&hfdcan);
       HAL_FDCAN_Init(&hfdcan);
       /* Start the FDCAN module */
       // HAL_FDCAN_Start(&hfdcan);
       /*** Initially set the global filter to accept unmatched messages to RX_FIFO1  and reject remote fromes**/
       HAL_FDCAN_ConfigGlobalFilter(&hfdcan, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

       /* Initially disable all other filters  0 to 31 for each of ID and EID  */
          for(i=0;i<32;i++){

           	sFilterConfig.IdType = FDCAN_STANDARD_ID;
           	sFilterConfig.FilterIndex =i;
           	sFilterConfig.FilterType = 0;
           	sFilterConfig.FilterConfig = FDCAN_FILTER_DISABLE;
           	sFilterConfig.FilterID1 = 0;
           	sFilterConfig.FilterID2 = 0;
           	HAL_FDCAN_ConfigFilter(&hfdcan, &sFilterConfig);

        	sFilterConfig.IdType = FDCAN_EXTENDED_ID;
           	sFilterConfig.FilterIndex =i;
           	sFilterConfig.FilterType = 0;
           	sFilterConfig.FilterConfig = FDCAN_FILTER_DISABLE;
           	sFilterConfig.FilterID1 = 0;
           	sFilterConfig.FilterID2 = 0;
           	HAL_FDCAN_ConfigFilter(&hfdcan, &sFilterConfig);

          }

       return;
   }
     error("Invalid syntax");
 }











