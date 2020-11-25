cmake_minimum_required(VERSION 3.0.0)
project(zdobywanie_koncesji VERSION 1.0)

add_compile_options(-Wall)

add_executable(urzad-regulacji.out urzad.c)
add_executable(poszukiwacz.out poszukiwacz.c)