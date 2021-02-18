cmake_minimum_required(VERSION 3.16)
project(zal C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall)

add_executable(monochord monochord/main.c monochord/utils.h monochord/utils.c)
target_link_libraries(monochord rt m)

add_executable(rejestrator rejestrator/rejestrator.c rejestrator/utils.c rejestrator/utils.h)
target_link_libraries(rejestrator rt)

add_executable(info_rejestrator info_rejestrator/main.c)
add_executable(debug debug/main.c)