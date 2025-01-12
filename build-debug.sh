#!/bin/bash

# Check if the "bin" directory exists; if not, create it
if [ ! -d "bin" ]; then
    echo "Creating bin/ directory..."
    mkdir bin
fi

# Check if the "bin/debug" directory exists; if not, create it
if [ ! -d "bin/debug" ]; then
    echo "Creating bin/debug/ directory..."
    mkdir -p bin/debug
fi

# Compile the engine source files into object files with debug symbols
echo "Compiling engine files..."
clang++ -std=c++17 -g -O0 -I./warpunk.core -DWARPUNK_EXPORT -fPIC -fvisibility=hidden -c main.cpp -o bin/debug/main.o
clang++ -std=c++17 -g -O0 -I./warpunk.core -DWARPUNK_EXPORT -fPIC -fvisibility=hidden -c warpunk.core/platform_linux.cpp -o bin/debug/platform_linux.o
clang++ -std=c++17 -g -O0 -I./warpunk.core -DWARPUNK_EXPORT -fPIC -fvisibility=hidden -c warpunk.core/input_system.cpp -o bin/debug/input_system.o

# Compile the game into a shared library (DLL or .so)
echo "Compiling magicians_misfits into a shared library..."
clang++ -std=c++17 -g -O0 -I./ -shared -fPIC magicians_misfits/magicians_misfits.cpp -o bin/debug/magicians_misfits.so

# Link the engine object files into the final executable and link X11 libraries
echo "Linking object files into the final executable..."
#clang++ -std=c++17 -g -o bin/debug/warpunk.out bin/debug/main.o bin/debug/platform_linux.o -ldl -lX11 -lX11-xcb -lxcb
clang++ -std=c++17 -g -rdynamic -o bin/debug/warpunk.out bin/debug/main.o bin/debug/platform_linux.o bin/debug/input_system.o -ldl -lX11 -lX11-xcb -lxcb

# Check if the build was successful
if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable is located at: bin/debug/warpunk.out"
    echo "Shared library is located at: bin/debug/magicians_misfits.so"
else
    echo "Build failed."
    exit 1
fi

