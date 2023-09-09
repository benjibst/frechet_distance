cmake_minimum_required(VERSION 3.5)

project(frechet_distance)



add_executable(frechet_distance 
	"./src/frechet_dist.c"
	"./src/geometry.c"
	"./src/main.c"
	"./src/visualizer.c"
)