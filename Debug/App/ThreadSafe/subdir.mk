################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/ThreadSafe/newlib_lock_glue.c 

OBJS += \
./App/ThreadSafe/newlib_lock_glue.o 

C_DEPS += \
./App/ThreadSafe/newlib_lock_glue.d 


# Each subdirectory must supply rules for building sources it contributes
App/ThreadSafe/%.o App/ThreadSafe/%.su App/ThreadSafe/%.cyclo: ../App/ThreadSafe/%.c App/ThreadSafe/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I../App/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-App-2f-ThreadSafe

clean-App-2f-ThreadSafe:
	-$(RM) ./App/ThreadSafe/newlib_lock_glue.cyclo ./App/ThreadSafe/newlib_lock_glue.d ./App/ThreadSafe/newlib_lock_glue.o ./App/ThreadSafe/newlib_lock_glue.su

.PHONY: clean-App-2f-ThreadSafe

