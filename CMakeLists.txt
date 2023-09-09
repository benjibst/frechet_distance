cmake_minimum_required(VERSION 3.16)

project(frechet_distance)

file(GLOB SRCS "src/*.c")
set(CMAKE_C_STANDARD 99)

add_subdirectory(raylib)

add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE raylib/src)

target_include_directories(${PROJECT_NAME} PRIVATE raygui/src)

target_link_libraries(${PROJECT_NAME} raylib)
