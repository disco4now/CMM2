********** STMCUBEIDE settings the compile the CMM2 firtware  *********************************
These build options are used to overcome a problem where the USBKeyboard would only connect intermittently and in some cases not at all.
This first occurred when the code size expanded to use Flash Slot 7 or with an upgrade of the compiler. If compiled with Optimisation (Os) 
i.e. for size the issue disappeared. Compiling for (Os) reduced the performance of MMBasic.
While the actual root cause is still unknown they following build options resolve the issue and restore the MMBasic performance.

1. Use STMCubeIDE 2.0 with GCC 13.3.1 (The default installed with STMCubeIDE 2.0
2. Compile with Opimisation (02) at the project level.
3. Give the following files a individual Optimisation setting.


Middlewares/ST/STM32_USB_Host_Library
-------------------------------------
all .c files set por Optimisation (0s) Size


Drivers/STM32H7xx_HAL_Driver/HAL Drivers at Drivers/Src
------------------------------------------------------- 
All set for Opimistaion (0s)  size
EXCEPT for these files that have no individual setting i.e. default to project settings.
stm32_hal_adc.c
stm32_hal_adc_ex.c

AND this one set for  (O3) More optimisation
stmh7xx_hal_gpio.c    (03) More optimisation.  

/Src project files
------------------
All have no individual setting i.e. default to project settings. 
EXCEPT FOR THESE
stm32h7xx_hal_msp.c         (Os)
stmh7xx_it.c                (Os)
system_stm32h7xx.c          (Os)

AND these with Optimisation (0z) More size
fm.c
kilo.c


----------------------------------------------------------------------------------------------------------------------------
These configurations setup when troubleshooting this issue are maintained, but now dont affected the location of code or .text
----------------------------------------------------------------------------------------------------------------------------
Note the .extra psect and the extra FLASH2 section

All the functions in OtherDisplays.c and Maths.c to use the new psect .extra
e.g.
void __attribute__((section(".extra"))) DrawBitmap8(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){

This means they can be relocated to another flash slot is required. This is not required and the new psect .extra is location with the normal code.
The configuration is kept incase it is needed in future.

The STM32H743II_FLASH.ld file has comments showing the potention relocation of the .text and .extra sections to FLASH7.
-------------------------------------------------------------------------------------------------------------------------------

These two #define statements in flash.h determine whether MMBasic ProgMemory uses Sectors 1-6 or 0-6

    /* Setup the flash used by MMBasic ProgMemory
    #define PROGRAMSTARTCODE 0                                /* Start at BANK2 Sector 0        */
 	#define FLASH_PROGRAM_ADDR    ADDR_FLASH_SECTOR_0_BANK2   /* Start Basic Program flash area */
   //#define PROGRAMSTARTCODE 1                               /* Start at BANK2 Sector 1        */ 
   //#define FLASH_PROGRAM_ADDR   ADDR_FLASH_SECTOR_1_BANK2   /* Start Basic Program flash area */

