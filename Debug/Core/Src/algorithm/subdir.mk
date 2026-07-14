################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/algorithm/kalman.c \
../Core/Src/algorithm/pid.c \
../Core/Src/algorithm/fixed_math.c 

OBJS += \
./Core/Src/algorithm/kalman.o \
./Core/Src/algorithm/pid.o \
./Core/Src/algorithm/fixed_math.o 

C_DEPS += \
./Core/Src/algorithm/kalman.d \
./Core/Src/algorithm/pid.d \
./Core/Src/algorithm/fixed_math.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/algorithm/%.o Core/Src/algorithm/%.su Core/Src/algorithm/%.cyclo: ../Core/Src/algorithm/%.c Core/Src/algorithm/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../App/Inc -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I../Drivers/BSP/Inc -I../Drivers/BSP/Inc/encoder -I../Drivers/BSP/Inc/mpu6050 -I../Core/Inc/algorithm -O0 -ffunction-sections -fdata-sections -Wall -mfloat-abi=soft -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-algorithm

clean-Core-2f-Src-2f-algorithm:
	-$(RM) ./Core/Src/algorithm/kalman.cyclo ./Core/Src/algorithm/kalman.d ./Core/Src/algorithm/kalman.o ./Core/Src/algorithm/kalman.su ./Core/Src/algorithm/pid.cyclo ./Core/Src/algorithm/pid.d ./Core/Src/algorithm/pid.o ./Core/Src/algorithm/pid.su ./Core/Src/algorithm/fixed_math.cyclo ./Core/Src/algorithm/fixed_math.d ./Core/Src/algorithm/fixed_math.o ./Core/Src/algorithm/fixed_math.su

.PHONY: clean-Core-2f-Src-2f-algorithm

