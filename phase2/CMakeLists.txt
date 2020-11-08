cmake_minimum_required(VERSION 3.10)

# set the project name
project(PHASE2 VERSION 1.0)

#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(server phase2server.c portable_socket.c)
target_include_directories(server PUBLIC "${PROJECT_BINARY_DIR}" )

add_executable(client phase2client.c portable_socket.c)
target_include_directories(client PUBLIC "${PROJECT_BINARY_DIR}" )


