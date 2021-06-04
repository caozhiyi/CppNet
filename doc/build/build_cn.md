# 编译

`Cppnet`目前支持三种编译方式：
- Linux和Mac OS下可使用 `make`
- Windows下可使用 `VS 2019`
- 使用 `cmake`

## make
在Linux和Mac OS下可直接在cppnet 目录执行    
```
make
```
即可编译`cppnet`静态库。    
其他示例代码，则需要进入到对应子目录，分别执行 `make` 命令。    
所有编译产出都在当前执行目录下。   

## VS 2019
`cppnet`对WinSDK没有特殊要求，github上携带的工程文件为VS2019，其他版本的VS可重新向平台之后再编译。   

## cmake
三个平台下均可使用cmake进行编译。
```
mkdir build
cd build
cmake ..
```
之后执行
```
make
```
或使用对应VS编译。   
所有可执行产出物均在当前`build`目录的`bin`目录下，`cppnet`静态库在当前`build`目录的`lib`目录下。   