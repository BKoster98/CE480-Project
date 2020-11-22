cmake_minimum_required(VERSION 3.10)

# set the project name
project(phase3 VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# add the executable
add_executable(phase3 phase3.cpp)
target_include_directories(phase3 PUBLIC "${PROJECT_BINARY_DIR}" )

