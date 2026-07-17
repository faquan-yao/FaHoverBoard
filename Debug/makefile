################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Drivers/BSP/Mpu6050/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := FaHoverBoard
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
FaHoverBoard.elf \

MAP_FILES += \
FaHoverBoard.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
FaHoverBoard.list \

OBJCOPY_HEX += \
FaHoverBoard.hex \

OBJCOPY_BIN += \
FaHoverBoard.bin \


# All Target
all: main-build

# Main-build Target
main-build: FaHoverBoard.elf secondary-outputs

# Tool invocations
FaHoverBoard.elf FaHoverBoard.map: $(OBJS) $(USER_OBJS) C:\3_work\github\FaHoverBoard\STM32F103ZETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "FaHoverBoard.elf" @"objects.list" $(USER_OBJS) -Wl,--start-group $(LIBS) -Wl,--end-group -mcpu=cortex-m3 -T"C:\3_work\github\FaHoverBoard\STM32F103ZETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="FaHoverBoard.map" -Wl,--gc-sections -static -L../Middlewares/ST/STM32_MotionAC_Library/Lib -L../Middlewares/ST/STM32_MotionAW_Library/Lib -L../Middlewares/ST/STM32_MotionEC_Library/Lib -L../Middlewares/ST/STM32_MotionFX_Library/Lib -L../Middlewares/ST/STM32_MotionGC_Library/Lib -L../Middlewares/ST/STM32_MotionID_Library/Lib -L../Middlewares/ST/STM32_MotionMC_Library/Lib -L../Middlewares/ST/STM32_MotionPM_Library/Lib -L../Middlewares/ST/STM32_MotionPW_Library/Lib -L../Middlewares/ST/STM32_MotionTL_Library/Lib -L../Middlewares/ST/ARM/DSP/Lib --specs=nano.specs -mfloat-abi=soft -mthumb -u _printf_float -u _scanf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

FaHoverBoard.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "FaHoverBoard.list"
	@echo 'Finished building: $@'
	@echo ' '

FaHoverBoard.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "FaHoverBoard.hex"
	@echo 'Finished building: $@'
	@echo ' '

FaHoverBoard.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "FaHoverBoard.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) FaHoverBoard.bin FaHoverBoard.elf FaHoverBoard.hex FaHoverBoard.list FaHoverBoard.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
