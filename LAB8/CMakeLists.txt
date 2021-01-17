cmake_minimum_required(VERSION 3.16)
project(LAB8 C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall)
add_library(utils utils.h utils.c)
target_include_directories(utils PUBLIC .)
target_link_libraries(utils PUBLIC rt)

add_executable(LAB8 main.c)
target_link_libraries(LAB8 PRIVATE utils)

