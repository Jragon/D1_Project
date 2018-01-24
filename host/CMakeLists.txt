cmake_minimum_required(VERSION 3.9)
project(host C)

set(CURSES_NEED_NCURSES TRUE)
FIND_PACKAGE(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c rs232.c rs232.h timer.c timer.h)

add_executable(host ${SOURCE_FILES})
target_link_libraries(host ncurses)