cmake_minimum_required(VERSION 3.11)

project(frechet_distance)

set(CMAKE_C_STANDARD 99)

add_subdirectory(raylib)
add_subdirectory(frechet_dist_lib)

add_executable(visualizer visualizer/main.c visualizer/visualizer.c)

target_link_libraries(visualizer PRIVATE raylib)
target_include_directories(visualizer PRIVATE raygui/src)
target_link_libraries(visualizer PRIVATE frechet_dist_lib)
target_include_directories(visualizer PRIVATE frechet_dist_lib)

if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()
