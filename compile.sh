#!/bin/bash
mkdir out
mkdir out/linux_release
g++ -O3 -fpermissive -pthread -DORCUS_DYNAMIC_LIBRARY_COMPILE -shared -fPIC src/orcus.cpp src/orcus_error.cpp src/orcus_bytecode.cpp src/orcus_compiler.cpp src/libs/orcus_lib_std.cpp src/libs/orcus_lib_math.cpp src/libs/orcus_lib_array.cpp src/libs/orcus_lib_string.cpp src/libs/orcus_lib_thread.cpp src/libs/orcus_lib_io.cpp src/libs/orcus_lib_os.cpp src/libs/orcus_lib_table.cpp src/libs/orcus_lib_debug.cpp -o orcus.so
g++ -O3 -fpermissive -pthread src/orcus_runtime.cpp orcus.so -Wl,-rpath,'$ORIGIN' -o orcus
mv -f orcus.so out/linux_release/orcus.so
mv -f orcus out/linux_release/orcus