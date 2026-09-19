#!/bin/bash
mkdir out
mkdir out/linux_debug
g++ -O0 -fpermissive -pthread -DMERCURY_DYNAMIC_LIBRARY_COMPILE -DDEBUG -g -shared -fPIC src/mercury.cpp src/mercury_error.cpp src/mercury_bytecode.cpp src/mercury_compiler.cpp src/libs/mercury_lib_std.cpp src/libs/mercury_lib_math.cpp src/libs/mercury_lib_array.cpp src/libs/mercury_lib_string.cpp src/libs/mercury_lib_thread.cpp src/libs/mercury_lib_io.cpp src/libs/mercury_lib_os.cpp src/libs/mercury_lib_table.cpp src/libs/mercury_lib_debug.cpp -o mercury.so
g++ -O0 -fpermissive -pthread -DDEBUG -g src/mercury_runtime.cpp mercury.so -Wl,-rpath,'$ORIGIN' -o mercury
mv -f mercury.so out/linux_debug/mercury.so
mv -f mercury out/linux_debug/mercury