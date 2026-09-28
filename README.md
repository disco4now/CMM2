# CMM2
Colour Maximite 2

Files to build the CMM2 MMBasic running on the STM32H743II



The STM32CubeIDE project is directory CMM2. This should be placed in your STM32CubeIDE workspace. e.g. workspace/CMM2 and opened and compiled using STM32CubeIDE.  
A compiled binary version is under the the binaries directory.  
A user manual for MMBasic on the CMM2 is under the docs directory.  


Changes since V5.07.01  Release  
V6.00.00b14  
Licence updated.  
16 bit tokens etc.  

V6.00.00b6    
Added CAN etc  

To Compile the source.  
Using STM32CubeIDE v2.0.0 and GCC 13.3.x  
 
Just put the source in a directory in the STM32CubeIDE workspace   
then open the .project file in notepad and adjust the <name>xxxxx</name> entry to match the directory used in the workspace.    

   <?xml version="1.0" encoding="UTF-8"?>  i.e.    
   <projectDescription>  
   <name>CMM2</name>  
   <comment></comment>  
   <projects>  
   </projects>  
      
Then use STM32CubeIDE menu, File->Open Projects From File System    
and select the folder and it should import.  
It will import to STM32CubeIDE as that name.  
Once its imported into STM32CUBEIDE it is set to compile as Debug. Go to menu  
Project-->Build Configurations-->Set Active and selected Release. This should compile.  




*****************************************************************************   
CMM2 Colour Maximite 2 MMBasic   
MMBasic for CMM2 hardware based on STM32H743II

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

*******************************************************************************  

 In addition the software components from STMicroelectronics are provided   
 subject to the license as detailed below:   
   
  ******************************************************************************   
  * @attention   
  *
  * <center>&copy; Copyright (c) 2019 STMicroelectronics.   
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license   
  * SLA0044, the "License"; You may not use this file except in compliance with   
  * the License. You may obtain a copy of the License at:  
  *                             www.st.com/SLA0044  
  *
