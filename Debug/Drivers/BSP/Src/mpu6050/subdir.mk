################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Src/mpu6050/inv_mpu.c \
../Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.c \
../Drivers/BSP/Src/mpu6050/mpu6050.c 

OBJS += \
./Drivers/BSP/Src/mpu6050/inv_mpu.o \
./Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.o \
./Drivers/BSP/Src/mpu6050/mpu6050.o 

C_DEPS += \
./Drivers/BSP/Src/mpu6050/inv_mpu.d \
./Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.d \
./Drivers/BSP/Src/mpu6050/mpu6050.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Src/mpu6050/%.o Drivers/BSP/Src/mpu6050/%.su Drivers/BSP/Src/mpu6050/%.cyclo: ../Drivers/BSP/Src/mpu6050/%.c Drivers/BSP/Src/mpu6050/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../App/Inc -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I../Drivers/BSP/Inc -I../Drivers/BSP/Inc/encoder -I../Drivers/BSP/Inc/mpu6050 -I../Core/Inc/algorithm -O0 -ffunction-sections -fdata-sections -Wall -mfloat-abi=soft -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Src-2f-mpu6050

clean-Drivers-2f-BSP-2f-Src-2f-mpu6050:
	-$(RM) ./Drivers/BSP/Src/mpu6050/inv_mpu.cyclo ./Drivers/BSP/Src/mpu6050/inv_mpu.d ./Drivers/BSP/Src/mpu6050/inv_mpu.o ./Drivers/BSP/Src/mpu6050/inv_mpu.su ./Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.cyclo ./Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.d ./Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.o ./Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.su ./Drivers/BSP/Src/mpu6050/mpu6050.cyclo ./Drivers/BSP/Src/mpu6050/mpu6050.d ./Drivers/BSP/Src/mpu6050/mpu6050.o ./Drivers/BSP/Src/mpu6050/mpu6050.su

.PHONY: clean-Drivers-2f-BSP-2f-Src-2f-mpu6050

