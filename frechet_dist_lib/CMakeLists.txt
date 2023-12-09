cmake_minimum_required(VERSION 3.11)

project(frechet_dist_lib)

add_library(${PROJECT_NAME} STATIC frechet_dist.c geometry.c frechet_dist.h geometry.h)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()