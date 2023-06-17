cmake_minimum_required(VERSION 3.24)
project(nyanc)

set(CMAKE_C_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -Wno-unused-parameter -Wno-unused-variable")
set(CMAKE_BUILD_TYPE Debug)

add_executable(nyanc main.c nyan.c nyan.h mouse.h mouse.c util.c util.h argparse.c argparse.h asm.c asm.h)

# installer
install(TARGETS nyanc DESTINATION /usr/local/bin)
