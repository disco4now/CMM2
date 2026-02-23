################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Image/array_utility.c \
../Src/Image/bilinear_interpolation.c \
../Src/Image/resize.c \
../Src/Image/rotate.c 

OBJS += \
./Src/Image/array_utility.o \
./Src/Image/bilinear_interpolation.o \
./Src/Image/resize.o \
./Src/Image/rotate.o 

C_DEPS += \
./Src/Image/array_utility.d \
./Src/Image/bilinear_interpolation.d \
./Src/Image/resize.d \
./Src/Image/rotate.d 


# Each subdirectory must supply rules for building sources it contributes
Src/Image/%.o Src/Image/%.su Src/Image/%.cyclo: ../Src/Image/%.c Src/Image/subdir.mk
	arm-none-eabi-gcc -c "$<" -mcpu=cortex-m7 -std=gnu99 -DUSE_HAL_DRIVER -DSTRUCTENABLED -DCMD16BIT -DSTM32H743xx -c -I"C:/workspacecmm2/CMM2V6.00.00/FATFS" -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/Image -O2 -ffunction-sections -fdata-sections -mslow-flash-data -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-Image

clean-Src-2f-Image:
	-$(RM) ./Src/Image/array_utility.cyclo ./Src/Image/array_utility.d ./Src/Image/array_utility.o ./Src/Image/array_utility.su ./Src/Image/bilinear_interpolation.cyclo ./Src/Image/bilinear_interpolation.d ./Src/Image/bilinear_interpolation.o ./Src/Image/bilinear_interpolation.su ./Src/Image/resize.cyclo ./Src/Image/resize.d ./Src/Image/resize.o ./Src/Image/resize.su ./Src/Image/rotate.cyclo ./Src/Image/rotate.d ./Src/Image/rotate.o ./Src/Image/rotate.su

.PHONY: clean-Src-2f-Image

