/***********************************************************************************************************************
MMBasic

MM_Custom.h

Copyright 2011 - 2021 Geoff Graham.  All Rights Reserved.
Copyright 2016 - 2021 Peter Mather.  All Rights Reserved.

Include file that contains the globals and defines for MMCustom.c in the Maximite version of MMBasic.

Note:  When you add extra functions you may get an error during the link phase indicating that there is insufficient
       flash space.  This is because the spare flash memory is allocated to the internal flash drive A:  As a result
       you will have to reduce the amount of flash allocated to drive A: to make space for your added functions.
       To do this reduce the number of pages allocated in files.h.  Look for this line:
            #define MONOCHROME_NBR_PAGES        nn
       and reduce the amount allocated by one and try recompiling.  If you still get an error reduce the number again
       and try another recompile until the error goes away.

************************************************************************************************************************/


