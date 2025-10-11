mkdir out
mkdir out/linux
mkdir out/linux_debug
g++ -O0 -fpermissive -pthread -DMERCURY_DYNAMIC_LIBRARY_COMPILE -DDebug -shared -fPIC -o out/linux_debug/mercury.so src/mercury.cpp src/mercury_error.cpp src/mercury_bytecode.cpp src/mercury_compiler.cpp src/libs/mercury_lib_std.cpp src/libs/mercury_lib_math.cpp src/libs/mercury_lib_array.cpp src/libs/mercury_lib_string.cpp src/libs/mercury_lib_thread.cpp src/libs/mercury_lib_io.cpp src/libs/mercury_lib_os.cpp src/libs/mercury_lib_table.cpp src/libs/mercury_lib_debug.cpp
g++ -O0 -fpermissive -pthread -o out/linux_debug/mercury src/mercury_runtime.cpp -ldl out/linux_debug/mercury.so -Wl,-rpath=.
