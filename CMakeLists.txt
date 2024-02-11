cmake_minimum_required(VERSION 3.27)
project(cofgifs C)

set(CMAKE_C_STANDARD 99)

add_library(cofgifs STATIC src/cofgifs.c src/cofgifs.h)

add_executable(cofgifs_test test/basic.c)
target_link_libraries(cofgifs_test PUBLIC cofgifs)
target_include_directories(cofgifs_test PUBLIC src)
