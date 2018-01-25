cmake_minimum_required(VERSION 3.9)
project(host C)
set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c rs232.c rs232.h timer.c timer.h)

add_executable(host ${SOURCE_FILES})