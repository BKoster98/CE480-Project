cmake_minimum_required(VERSION 3.10)

# set the project name
project(PHASE3 VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(server server.cpp portable_socket.cpp)
target_include_directories(server PUBLIC "${PROJECT_BINARY_DIR}" )

add_executable(client client.cpp portable_socket.cpp)
target_include_directories(client PUBLIC "${PROJECT_BINARY_DIR}" )

if(MSVC)
    target_compile_options(client PRIVATE /W4 /WX)
else()
    target_compile_options(client PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

if(MSVC)
    target_compile_options(server PRIVATE /W4 /WX)
else()
    target_compile_options(server PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

