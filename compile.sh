#!/bin/bash
mkdir out
mkdir out/linux_release
g++ -O3 -fpermissive -pthread -DMERCURY_DYNAMIC_LIBRARY_COMPILE -shared -fPIC -o out/linux_release/mercury.so src/mercury.cpp src/mercury_error.cpp src/mercury_bytecode.cpp src/mercury_compiler.cpp src/libs/mercury_lib_std.cpp src/libs/mercury_lib_math.cpp src/libs/mercury_lib_array.cpp src/libs/mercury_lib_string.cpp src/libs/mercury_lib_thread.cpp src/libs/mercury_lib_io.cpp src/libs/mercury_lib_os.cpp src/libs/mercury_lib_table.cpp src/libs/mercury_lib_debug.cpp
g++ -O3 -fpermissive -pthread -o out/linux_release/mercury src/mercury_runtime.cpp -ldl out/linux_release/mercury.so -Wl,-rpath=.
