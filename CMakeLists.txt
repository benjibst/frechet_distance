cmake_minimum_required(VERSION 3.16)

project(frechet_distance)

file(GLOB SRCS "src/*.c")
set(CMAKE_C_STANDARD 99)

add_subdirectory(raylib)


add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE raygui/src)

target_link_libraries(${PROJECT_NAME} raylib)
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()
