# frechet_distance
This project includes a library to compute the Fréchet distance between polygonal curves and an interactive application to create curves and visualize the free space diagram. 
Try it out at https://benjibst.github.io/frechet_distance/ <br>
Binaries are available for Windows and Linux at https://github.com/benjibst/frechet_distance/releases 

## Using the app
On the left panel you can left or right click to add points to either the first or the second curve.
On the right panel you see the free space diagram for the curves and a given epsilon.
You can change the epsilon by entering a number in the top right corner.
By pressing the compute button you can calculate the frechet distance of the curves.
Press X to delete the curves and start again.


https://github.com/benjibst/frechet_distance/assets/76067254/e488777e-a760-4c8c-88e8-207bfe6c289b



## How to build
First clone the repo and the dependencies
```
git clone https://github.com/benjibst/frechet_distance.git
git submodule update --init
```
Create a build directory
```
mkdir build
cd build
```
Generate the platform build files using cmake
```
cmake ..
```
Compile on Linux:
```
make
```
Compile on Windows: 
Open a developer command prompt window, change to the build directory and run:
```
MSBuild.exe .\frechet_distance.vcxproj
```



