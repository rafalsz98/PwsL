cmake_minimum_required(VERSION 3.16)
project(LAB7 C)

set(CMAKE_C_STANDARD 99)

add_executable(cw1_p1_v1 cw1_p1_v1.c)
add_executable(kill kill.c)
add_executable(cw1_p2_v1 cw1_p2_v1.c)
add_executable(cw2_p1_v2 cw2_p1_v2.c)
add_executable(cw2_p2_v2 cw2_p2_v2.c)
target_link_libraries(cw2_p2_v2 PRIVATE rt)