cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME FaHoverBoard)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

set(TARGET_CHIP STM32F10x)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F103XX_FLASH.ld)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/algorithm/kalman.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/algorithm/pid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/algorithm/fixed_math.c
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Src/interrupt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Src/encoder/encoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Src/mpu6050/mpu6050.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Src/mpu6050/inv_mpu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Src/mpu6050/inv_mpu_dmp_motion_driver.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/algorithm
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Inc/encoder
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Inc/mpu6050
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
