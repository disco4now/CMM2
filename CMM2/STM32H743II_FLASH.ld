/*
*****************************************************************************
**

**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32H743ZITx Device with
**                2048KByte FLASH, 128KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
**  (c)Copyright Ac6.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Ac6 permit registered System Workbench for MCU users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the System Workbench for MCU toolchain.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20020000-0x10;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x100;      /* required amount of heap  */
_Min_Stack_Size = 0x4800; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
DTCMRAM (rw)    : ORIGIN = 0x20000000, LENGTH = 128K
AXIRAM (rw)     : ORIGIN = 0x24000000, LENGTH = 512K
RAM_D2 (rw)     : ORIGIN = 0x30000000, LENGTH = 288K
RAM_D3 (rw)     : ORIGIN = 0x38000000, LENGTH = 64K
ITCMRAM (xrw)   : ORIGIN = 0x00000000, LENGTH = 64K
/*FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 896K */    /*FLASH restricted to 0-6 */
FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K        /*FLASH uses 0-7          */
/*FLASH7 (rx)	    : ORIGIN = 0x080E0000, LENGTH = 128K */
/*FLASH7 (rx)	    : ORIGIN = 0x08100000, LENGTH = 128K */
/*FLASH2 (r)      : ORIGIN = 0x08120000, LENGTH = 896K  */   /* MMBasic programs use 1-6   */
FLASH2 (r)      : ORIGIN = 0x08100000, LENGTH = 1024K       /*  MMbasic programs use 0-6    */
/*RAM_D5 (rw)     : ORIGIN = 0x38800000, LENGTH = 4K*/
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >ITCMRAM AT> FLASH
  
   /* used by the startup to initialize data */
  _siDTCMData = LOADADDR(.DTCMData);

.DTCMData : {
  . = ALIGN(4);
  _sDTCMData = .;        /* create a global symbol at DTCMData start */
   . = ALIGN(4);
  *stm32h7xx_it.o (.data .data*)
   _eDTCMData = .;        /* create a global symbol at DTCMData end */
   } >DTCMRAM AT> FLASH
 
 /* Uninitialized DTCM data section */
. = ALIGN(4);

.DTCMbss : {
  . = ALIGN(4);
  _sDTCMbss = .;        /* create a global symbol at DTCMbss start */
   . = ALIGN(4);
   _eDTCMbss = .;        /* create a global symbol at DTCMbss end */
   } >DTCMRAM
   
/* used by the startup to initialize data */
  _siITCM = LOADADDR(.ITCMSection);


  /* The program code and other data goes into FLASH */
  .text :
  {
    /*. = 0x28000;      sector 1 is skipped. Was used for Options but could be now used */
    . = ALIGN(4); 
  
    *(.cspace)
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

.extra :
{
    . = ALIGN(4);
    KEEP(*(.extra))     /* <-- explicitly pull in .extra */
    . = ALIGN(4);
    _etext7 = .;
} >FLASH    /*                       FLASH7 */
 
  .ITCMSection : {
 
   . = ALIGN(4);
   _sITCM = .;        /* create a global symbol at ITCM start */
   
    *stm32h7xx_it.o (.text .text*) 
   *stm32h7xx_it.o (.rodata .rodata*)  
   *Timers.o (.text .text*) 
   *Timers.o (.rodata .rodata*)  
   *stm32h7xx_hal_ltdc.o (.text .text*) 
   *stm32h7xx_hal_hcd.o (.text .text*) 
      
 
    _eITCM = .;        /* create a global symbol at ITCM end */
   
 } >ITCMRAM AT> FLASH
  
  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> FLASH  /*                     FLASH7*/

   .my_section (NOLOAD):
  {
    . = ALIGN(4);
    _s_my_section = .;       
    *(.my_section)
    . = ALIGN(4) ;
    _e_my_section = .; 
  } > DTCMRAM
  
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >DTCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
    _e_endheap = .; 
  } >DTCMRAM

/*
  .backup (NOLOAD):
  {
    . = ALIGN(4);
    _s_backup = .;       
    *(.backup)
    . = ALIGN(4) ;
    _e_backup = .; 
  } > RAM_D5*/

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}

