################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Analog_I2C.c \
../Src/Audio.c \
../Src/BmpDecoder.c \
../Src/CAN.c \
../Src/CFunctions.c \
../Src/Commands.c \
../Src/Custom.c \
../Src/DEV_Config.c \
../Src/Draw.c \
../Src/External.c \
../Src/FileIO.c \
../Src/Functions.c \
../Src/GPS.c \
../Src/GUI.c \
../Src/I2C.c \
../Src/MATHS.c \
../Src/MMBasic.c \
../Src/MM_Custom.c \
../Src/MM_Misc.c \
../Src/Memory.c \
../Src/MiscSTM32.c \
../Src/Onewire.c \
../Src/Operators.c \
../Src/OtherDisplays.c \
../Src/PWM.c \
../Src/SPI-LCD.c \
../Src/SPI.c \
../Src/Serial.c \
../Src/SerialFileIO.c \
../Src/Timers.c \
../Src/XModem.c \
../Src/aes.c \
../Src/cJSON.c \
../Src/debug.c \
../Src/decode_polling.c \
../Src/dma2d.c \
../Src/ff.c \
../Src/ffsystem.c \
../Src/ffunicode.c \
../Src/flash.c \
../Src/fm.c \
../Src/gifdec.c \
../Src/hxcmod.c \
../Src/keyboard.c \
../Src/kilo.c \
../Src/ltdc.c \
../Src/main.c \
../Src/mmc_stm32.c \
../Src/mouse.c \
../Src/picojpeg.c \
../Src/re.c \
../Src/reciter.c \
../Src/render.c \
../Src/sam.c \
../Src/sprites.c \
../Src/stm32h7xx_hal_msp.c \
../Src/stm32h7xx_it.c \
../Src/syscalls.c \
../Src/system_stm32h7xx.c \
../Src/turtle.c \
../Src/upng.c \
../Src/usb_host.c \
../Src/usbh_conf.c \
../Src/usbh_platform.c 

S_UPPER_SRCS += \
../Src/assember.S 

OBJS += \
./Src/Analog_I2C.o \
./Src/Audio.o \
./Src/BmpDecoder.o \
./Src/CAN.o \
./Src/CFunctions.o \
./Src/Commands.o \
./Src/Custom.o \
./Src/DEV_Config.o \
./Src/Draw.o \
./Src/External.o \
./Src/FileIO.o \
./Src/Functions.o \
./Src/GPS.o \
./Src/GUI.o \
./Src/I2C.o \
./Src/MATHS.o \
./Src/MMBasic.o \
./Src/MM_Custom.o \
./Src/MM_Misc.o \
./Src/Memory.o \
./Src/MiscSTM32.o \
./Src/Onewire.o \
./Src/Operators.o \
./Src/OtherDisplays.o \
./Src/PWM.o \
./Src/SPI-LCD.o \
./Src/SPI.o \
./Src/Serial.o \
./Src/SerialFileIO.o \
./Src/Timers.o \
./Src/XModem.o \
./Src/aes.o \
./Src/assember.o \
./Src/cJSON.o \
./Src/debug.o \
./Src/decode_polling.o \
./Src/dma2d.o \
./Src/ff.o \
./Src/ffsystem.o \
./Src/ffunicode.o \
./Src/flash.o \
./Src/fm.o \
./Src/gifdec.o \
./Src/hxcmod.o \
./Src/keyboard.o \
./Src/kilo.o \
./Src/ltdc.o \
./Src/main.o \
./Src/mmc_stm32.o \
./Src/mouse.o \
./Src/picojpeg.o \
./Src/re.o \
./Src/reciter.o \
./Src/render.o \
./Src/sam.o \
./Src/sprites.o \
./Src/stm32h7xx_hal_msp.o \
./Src/stm32h7xx_it.o \
./Src/syscalls.o \
./Src/system_stm32h7xx.o \
./Src/turtle.o \
./Src/upng.o \
./Src/usb_host.o \
./Src/usbh_conf.o \
./Src/usbh_platform.o 

S_UPPER_DEPS += \
./Src/assember.d 

C_DEPS += \
./Src/Analog_I2C.d \
./Src/Audio.d \
./Src/BmpDecoder.d \
./Src/CAN.d \
./Src/CFunctions.d \
./Src/Commands.d \
./Src/Custom.d \
./Src/DEV_Config.d \
./Src/Draw.d \
./Src/External.d \
./Src/FileIO.d \
./Src/Functions.d \
./Src/GPS.d \
./Src/GUI.d \
./Src/I2C.d \
./Src/MATHS.d \
./Src/MMBasic.d \
./Src/MM_Custom.d \
./Src/MM_Misc.d \
./Src/Memory.d \
./Src/MiscSTM32.d \
./Src/Onewire.d \
./Src/Operators.d \
./Src/OtherDisplays.d \
./Src/PWM.d \
./Src/SPI-LCD.d \
./Src/SPI.d \
./Src/Serial.d \
./Src/SerialFileIO.d \
./Src/Timers.d \
./Src/XModem.d \
./Src/aes.d \
./Src/cJSON.d \
./Src/debug.d \
./Src/decode_polling.d \
./Src/dma2d.d \
./Src/ff.d \
./Src/ffsystem.d \
./Src/ffunicode.d \
./Src/flash.d \
./Src/fm.d \
./Src/gifdec.d \
./Src/hxcmod.d \
./Src/keyboard.d \
./Src/kilo.d \
./Src/ltdc.d \
./Src/main.d \
./Src/mmc_stm32.d \
./Src/mouse.d \
./Src/picojpeg.d \
./Src/re.d \
./Src/reciter.d \
./Src/render.d \
./Src/sam.d \
./Src/sprites.d \
./Src/stm32h7xx_hal_msp.d \
./Src/stm32h7xx_it.d \
./Src/syscalls.d \
./Src/system_stm32h7xx.d \
./Src/turtle.d \
./Src/upng.d \
./Src/usb_host.d \
./Src/usbh_conf.d \
./Src/usbh_platform.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc -c "$<" -mcpu=cortex-m7 -std=gnu99 -DUSE_HAL_DRIVER -DSTRUCTENABLED -DCMD16BIT -DSTM32H743xx -c -I"C:/workspacecmm2/CMM2V6.00.00/FATFS" -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/Image -O2 -ffunction-sections -fdata-sections -mslow-flash-data -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/%.o: ../Src/%.S Src/subdir.mk
	arm-none-eabi-gcc -c -mcpu=cortex-m7 -c -Wa,-W -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"
Src/fm.o: ../Src/fm.c Src/subdir.mk
	arm-none-eabi-gcc -c "$<" -mcpu=cortex-m7 -std=gnu99 -DUSE_HAL_DRIVER -DSTRUCTENABLED -DCMD16BIT -DSTM32H743xx -c -I"C:/workspacecmm2/CMM2V6.00.00/FATFS" -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/Image -Oz -ffunction-sections -fdata-sections -mslow-flash-data -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/kilo.o: ../Src/kilo.c Src/subdir.mk
	arm-none-eabi-gcc -c "$<" -mcpu=cortex-m7 -std=gnu99 -DUSE_HAL_DRIVER -DSTRUCTENABLED -DCMD16BIT -DSTM32H743xx -c -I"C:/workspacecmm2/CMM2V6.00.00/FATFS" -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/Image -Oz -ffunction-sections -fdata-sections -mslow-flash-data -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/Analog_I2C.cyclo ./Src/Analog_I2C.d ./Src/Analog_I2C.o ./Src/Analog_I2C.su ./Src/Audio.cyclo ./Src/Audio.d ./Src/Audio.o ./Src/Audio.su ./Src/BmpDecoder.cyclo ./Src/BmpDecoder.d ./Src/BmpDecoder.o ./Src/BmpDecoder.su ./Src/CAN.cyclo ./Src/CAN.d ./Src/CAN.o ./Src/CAN.su ./Src/CFunctions.cyclo ./Src/CFunctions.d ./Src/CFunctions.o ./Src/CFunctions.su ./Src/Commands.cyclo ./Src/Commands.d ./Src/Commands.o ./Src/Commands.su ./Src/Custom.cyclo ./Src/Custom.d ./Src/Custom.o ./Src/Custom.su ./Src/DEV_Config.cyclo ./Src/DEV_Config.d ./Src/DEV_Config.o ./Src/DEV_Config.su ./Src/Draw.cyclo ./Src/Draw.d ./Src/Draw.o ./Src/Draw.su ./Src/External.cyclo ./Src/External.d ./Src/External.o ./Src/External.su ./Src/FileIO.cyclo ./Src/FileIO.d ./Src/FileIO.o ./Src/FileIO.su ./Src/Functions.cyclo ./Src/Functions.d ./Src/Functions.o ./Src/Functions.su ./Src/GPS.cyclo ./Src/GPS.d ./Src/GPS.o ./Src/GPS.su ./Src/GUI.cyclo ./Src/GUI.d ./Src/GUI.o ./Src/GUI.su ./Src/I2C.cyclo ./Src/I2C.d ./Src/I2C.o ./Src/I2C.su ./Src/MATHS.cyclo ./Src/MATHS.d ./Src/MATHS.o ./Src/MATHS.su ./Src/MMBasic.cyclo ./Src/MMBasic.d ./Src/MMBasic.o ./Src/MMBasic.su ./Src/MM_Custom.cyclo ./Src/MM_Custom.d ./Src/MM_Custom.o ./Src/MM_Custom.su ./Src/MM_Misc.cyclo ./Src/MM_Misc.d ./Src/MM_Misc.o ./Src/MM_Misc.su ./Src/Memory.cyclo ./Src/Memory.d ./Src/Memory.o ./Src/Memory.su ./Src/MiscSTM32.cyclo ./Src/MiscSTM32.d ./Src/MiscSTM32.o ./Src/MiscSTM32.su ./Src/Onewire.cyclo ./Src/Onewire.d ./Src/Onewire.o ./Src/Onewire.su ./Src/Operators.cyclo ./Src/Operators.d ./Src/Operators.o ./Src/Operators.su ./Src/OtherDisplays.cyclo ./Src/OtherDisplays.d ./Src/OtherDisplays.o ./Src/OtherDisplays.su ./Src/PWM.cyclo ./Src/PWM.d ./Src/PWM.o ./Src/PWM.su ./Src/SPI-LCD.cyclo ./Src/SPI-LCD.d ./Src/SPI-LCD.o ./Src/SPI-LCD.su ./Src/SPI.cyclo ./Src/SPI.d ./Src/SPI.o ./Src/SPI.su ./Src/Serial.cyclo ./Src/Serial.d ./Src/Serial.o ./Src/Serial.su ./Src/SerialFileIO.cyclo ./Src/SerialFileIO.d ./Src/SerialFileIO.o ./Src/SerialFileIO.su ./Src/Timers.cyclo ./Src/Timers.d ./Src/Timers.o ./Src/Timers.su ./Src/XModem.cyclo ./Src/XModem.d ./Src/XModem.o ./Src/XModem.su ./Src/aes.cyclo ./Src/aes.d ./Src/aes.o ./Src/aes.su ./Src/assember.d ./Src/assember.o ./Src/cJSON.cyclo ./Src/cJSON.d ./Src/cJSON.o ./Src/cJSON.su ./Src/debug.cyclo ./Src/debug.d ./Src/debug.o ./Src/debug.su ./Src/decode_polling.cyclo ./Src/decode_polling.d ./Src/decode_polling.o ./Src/decode_polling.su ./Src/dma2d.cyclo ./Src/dma2d.d ./Src/dma2d.o ./Src/dma2d.su ./Src/ff.cyclo ./Src/ff.d ./Src/ff.o ./Src/ff.su ./Src/ffsystem.cyclo ./Src/ffsystem.d ./Src/ffsystem.o ./Src/ffsystem.su ./Src/ffunicode.cyclo ./Src/ffunicode.d ./Src/ffunicode.o ./Src/ffunicode.su ./Src/flash.cyclo ./Src/flash.d ./Src/flash.o ./Src/flash.su ./Src/fm.cyclo ./Src/fm.d ./Src/fm.o ./Src/fm.su ./Src/gifdec.cyclo ./Src/gifdec.d ./Src/gifdec.o ./Src/gifdec.su ./Src/hxcmod.cyclo ./Src/hxcmod.d ./Src/hxcmod.o ./Src/hxcmod.su ./Src/keyboard.cyclo ./Src/keyboard.d ./Src/keyboard.o ./Src/keyboard.su ./Src/kilo.cyclo ./Src/kilo.d ./Src/kilo.o ./Src/kilo.su ./Src/ltdc.cyclo ./Src/ltdc.d ./Src/ltdc.o ./Src/ltdc.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/mmc_stm32.cyclo ./Src/mmc_stm32.d ./Src/mmc_stm32.o ./Src/mmc_stm32.su ./Src/mouse.cyclo ./Src/mouse.d ./Src/mouse.o ./Src/mouse.su ./Src/picojpeg.cyclo ./Src/picojpeg.d ./Src/picojpeg.o ./Src/picojpeg.su ./Src/re.cyclo ./Src/re.d ./Src/re.o ./Src/re.su ./Src/reciter.cyclo ./Src/reciter.d ./Src/reciter.o ./Src/reciter.su ./Src/render.cyclo ./Src/render.d ./Src/render.o ./Src/render.su ./Src/sam.cyclo ./Src/sam.d ./Src/sam.o ./Src/sam.su ./Src/sprites.cyclo ./Src/sprites.d ./Src/sprites.o ./Src/sprites.su ./Src/stm32h7xx_hal_msp.cyclo ./Src/stm32h7xx_hal_msp.d ./Src/stm32h7xx_hal_msp.o ./Src/stm32h7xx_hal_msp.su ./Src/stm32h7xx_it.cyclo ./Src/stm32h7xx_it.d ./Src/stm32h7xx_it.o ./Src/stm32h7xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/system_stm32h7xx.cyclo ./Src/system_stm32h7xx.d ./Src/system_stm32h7xx.o ./Src/system_stm32h7xx.su ./Src/turtle.cyclo ./Src/turtle.d ./Src/turtle.o ./Src/turtle.su ./Src/upng.cyclo ./Src/upng.d ./Src/upng.o ./Src/upng.su ./Src/usb_host.cyclo ./Src/usb_host.d ./Src/usb_host.o ./Src/usb_host.su ./Src/usbh_conf.cyclo ./Src/usbh_conf.d ./Src/usbh_conf.o ./Src/usbh_conf.su ./Src/usbh_platform.cyclo ./Src/usbh_platform.d ./Src/usbh_platform.o ./Src/usbh_platform.su

.PHONY: clean-Src

