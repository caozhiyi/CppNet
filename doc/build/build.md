# Build

`Cppnet`it supports three compilation methods:
- Linux and Mac OS use `make`
- Windows use `VS 2019`
- use `cmake`

## make
On Linux and Mac OS, can executed directly in `cppnet` directory
```
make
```
The `cppnet` static library can be compiled.  
Other example code,  
For other sample codes, you need to enter the corresponding subdirectories and execute the `make` command respectively.   
All compilation outputs are in the current execution directory.     

## VS 2019
`cppnet` has no special requirements for WinSDK. The project file carried on GitHub is vs2019. Other versions of vs can be recompiled to the platform.

## cmake
`cmake` can be used to compile on all three platforms.  
```
mkdir build
cd build
cmake ..
```
Then execute
```
make
```
Or compile with the corresponding vs.  
All executable outputs are in the `bin` directory of the current `build` directory, and the `cppnet` static library is in the `lib` directory of the current build directory.