cmake_minimum_required(VERSION 3.12)
project(RISC-V-Emulator)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the include directory
include_directories(include)

# Add the source files
file(GLOB SOURCES src/*.cpp)

# Create the executable
add_executable(RISCV_Emulator ${SOURCES})
