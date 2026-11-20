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
set(CMAKE_PROJECT_NAME smartcar)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Core/Inc/proto.h
        Core/Src/proto.c
        Core/Inc/app.h
        Core/Src/app.c
        Core/Inc/pca9685.h
        Core/Src/pca9685.c
        Core/Inc/servo_profile.h
        Core/Src/servo_profile.c
        Core/Inc/servo.h
        Core/Src/servo.c
        Core/Inc/dbg.h
        Core/Src/dbg.c
        Core/Inc/console.h
        Core/Src/console.c)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/motor.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/chassis.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/proto.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/app.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/pca9685.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/servo_profile.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/servo.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/dbg.c
        ${CMAKE_CURRENT_LIST_DIR}/Core/Src/console.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined include paths
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