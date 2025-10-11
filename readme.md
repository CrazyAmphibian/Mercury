# Mercury
**current version: Alpha 5**

is an interpreted programming language

Some features include:
* integers and floats as distinct types
* multithreading
* self-modifying code
* logical XOR
* strong dynamic typing
* fine variable scope control
* lambdas / anonymous functions



## how to build

### windows
1. Use visual studio to open /Mercury/Mercury.sln
2. run batch build. select the proper configuration for your platform, and ensure that 1 build of Mercury and 1 build of Mercury_Runtime is selected.
3. Build. The output will be in ./out/windows[configuration]

### unix-like systems
1. ensure your system has the g++ package installed
2. run compile.sh (or compile_debug.sh)
3. The output will be in ./out/linux[_debug/_release]


### not compiling with libraries
to not compile with a library, modify mercury.cpp so that it does not include the library's header. next, remove the library's files from the build. On windows, this is done by removing them from the solution. On unix-like systems, this is done by removing them from the file list in the compile script.


## how to contribute
* expanding documentation
* enhancing stability
* adding new library functions
* increasing runtime speed
* reporting bugs
