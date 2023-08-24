cmake_minimum_required(VERSION 3.5)

project(frechet_distance)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g")

add_executable(frechet_distance 
	"./src/frechet_dist.c"
	"./src/geometry.c"
	"./src/main.c"
	"./src/visualizer.c"
)
target_include_directories(frechet_distance PUBLIC "./include")
target_link_directories(frechet_distance PUBLIC "./lib")
target_link_libraries(frechet_distance PUBLIC "raylib" "m" "raygui")
target_compile_definitions(frechet_distance PUBLIC "SINGLE=1")
