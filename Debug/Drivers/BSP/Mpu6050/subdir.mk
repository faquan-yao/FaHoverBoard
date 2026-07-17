################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Mpu6050/inv_mpu.c \
../Drivers/BSP/Mpu6050/inv_mpu_dmp_motion_driver.c 

OBJS += \
./Drivers/BSP/Mpu6050/inv_mpu.o \
./Drivers/BSP/Mpu6050/inv_mpu_dmp_motion_driver.o 

C_DEPS += \
./Drivers/BSP/Mpu6050/inv_mpu.d \
./Drivers/BSP/Mpu6050/inv_mpu_dmp_motion_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Mpu6050/%.o Drivers/BSP/Mpu6050/%.su Drivers/BSP/Mpu6050/%.cyclo: ../Drivers/BSP/Mpu6050/%.c Drivers/BSP/Mpu6050/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/3_work/github/FaHoverBoard/Application/Config" -I../Middlewares/ST/ARM/DSP/Inc -I../Middlewares/ST/STM32_MotionAC_Library/Inc -I../Middlewares/ST/STM32_MotionAW_Library/Inc -I../Middlewares/ST/STM32_MotionEC_Library/Inc -I../Middlewares/ST/STM32_MotionFX_Library/Inc -I../Middlewares/ST/STM32_MotionGC_Library/Inc -I../Middlewares/ST/STM32_MotionID_Library/Inc -I../Middlewares/ST/STM32_MotionMC_Library/Inc -I../Middlewares/ST/STM32_MotionPM_Library/Inc -I../Middlewares/ST/STM32_MotionPW_Library/Inc -I../Middlewares/ST/STM32_MotionTL_Library/Inc -I"C:/3_work/github/FaHoverBoard/Drivers/BSP/Mpu6050" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Mpu6050

clean-Drivers-2f-BSP-2f-Mpu6050:
	-$(RM) ./Drivers/BSP/Mpu6050/inv_mpu.cyclo ./Drivers/BSP/Mpu6050/inv_mpu.d ./Drivers/BSP/Mpu6050/inv_mpu.o ./Drivers/BSP/Mpu6050/inv_mpu.su ./Drivers/BSP/Mpu6050/inv_mpu_dmp_motion_driver.cyclo ./Drivers/BSP/Mpu6050/inv_mpu_dmp_motion_driver.d ./Drivers/BSP/Mpu6050/inv_mpu_dmp_motion_driver.o ./Drivers/BSP/Mpu6050/inv_mpu_dmp_motion_driver.su

.PHONY: clean-Drivers-2f-BSP-2f-Mpu6050

