cmake_minimum_required(VERSION 3.5)

project(frechet_distance)

add_executable(frechet_distance 
	"./src/frechet_dist.c"
	"./src/geometry.c"
	"./src/main.c"
	"./src/visualizer.c"
)
if(MSVC)
	target_include_directories(frechet_distance PUBLIC "./include")
	target_link_directories(frechet_distance PUBLIC "C:\\dev\\raylib\\build\\raylib\\Release")
	target_link_libraries(frechet_distance PUBLIC "raylib" "winmm.lib")
	target_compile_definitions(frechet_distance PUBLIC "SINGLE=1")
	target_compile_options(frechet_distance PUBLIC "/W1" "/Md")
else()
	target_include_directories(frechet_distance PUBLIC "./include")
	target_link_libraries(frechet_distance PUBLIC "raylib" "m")
	target_link_directories(frechet_distance PUBLIC "/home/benni/dev/raylib/build/raylib")
	target_compile_definitions(frechet_distance PUBLIC "SINGLE=1")
	target_compile_options(frechet_distance PUBLIC "-Wall" "-ggdb")
endif()