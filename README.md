# frechet_distance
This project includes a library to compute the Fréchet distance between polygonal curves and an interactive application to create curves and visualize the free space diagram. 
Try it out at https://benjibst.github.io/frechet_distance/

## Using the app
On the left panel you can left or right click to add points to either the first or the second curve.
On the right panel you see the free space diagram for the curves and a given epsilon.
You can change the epsilon by entering a number in the top right corner.
Press X to delete the curves and start again.

https://github.com/benjibst/frechet_distance/assets/76067254/114ed263-398e-4841-9b58-c6290c61d095

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



