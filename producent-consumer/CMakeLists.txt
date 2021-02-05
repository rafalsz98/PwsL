cmake_minimum_required(VERSION 3.16)
project(producent_consumer C)

set(CMAKE_C_STANDARD 99)

add_executable(producent main.c utils.h utils.c)
target_link_libraries(producent rt)