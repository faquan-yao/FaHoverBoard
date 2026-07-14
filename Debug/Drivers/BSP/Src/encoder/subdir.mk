################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Src/encoder/encoder.c 

OBJS += \
./Drivers/BSP/Src/encoder/encoder.o 

C_DEPS += \
./Drivers/BSP/Src/encoder/encoder.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Src/encoder/%.o Drivers/BSP/Src/encoder/%.su Drivers/BSP/Src/encoder/%.cyclo: ../Drivers/BSP/Src/encoder/%.c Drivers/BSP/Src/encoder/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../App/Inc -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I../Drivers/BSP/Inc -I../Drivers/BSP/Inc/encoder -I../Drivers/BSP/Inc/mpu6050 -I../Core/Inc/algorithm -O0 -ffunction-sections -fdata-sections -Wall -mfloat-abi=soft -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Src-2f-encoder

clean-Drivers-2f-BSP-2f-Src-2f-encoder:
	-$(RM) ./Drivers/BSP/Src/encoder/encoder.cyclo ./Drivers/BSP/Src/encoder/encoder.d ./Drivers/BSP/Src/encoder/encoder.o ./Drivers/BSP/Src/encoder/encoder.su

.PHONY: clean-Drivers-2f-BSP-2f-Src-2f-encoder

