#!/bin/bash

# Get the absolute path of the script
SCRIPT_PATH="$(realpath "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"

# Output directories
DEBUG_DIR="${SCRIPT_DIR}/bin/debug"

# Ensure output directories exist
mkdir -p "$DEBUG_DIR"

# Compiler and flags
CXX="clang++"
CXXFLAGS="-std=c++17 -Werror -Wunused-function -g -O0 -DWARPUNK_EXPORT -DWARPUNK_DEBUG -fPIC -fvisibility=hidden"
INCLUDES="-I${SCRIPT_DIR} -I${SCRIPT_DIR}/warpunk.core -I/usr/include"

# Compile files in warpunk.core
echo "Compiling files in warpunk.core:"
$CXX $CXXFLAGS $INCLUDES -c main.cpp -o bin/debug/main.o

WARPUNK_CPP_FILES=$(find "${SCRIPT_DIR}/warpunk.core" -name "*.cpp")
WARPUNK_O_FILES=
for file in $WARPUNK_CPP_FILES; do
    RELATIVE_PATH=${file#${SCRIPT_DIR}/}
    echo "$RELATIVE_PATH"
    OBJECT_FILE="${DEBUG_DIR}/${RELATIVE_PATH%.cpp}.o"
    mkdir -p "$(dirname "$OBJECT_FILE")"
    $CXX $CXXFLAGS $INCLUDES -c "$file" -o "$OBJECT_FILE"
    if [ $? -ne 0 ]; then
        echo "Compilation failed for $RELATIVE_PATH"
        exit 1
    fi
    WARPUNK_O_FILES="$WARPUNK_O_FILES $OBJECT_FILE"
done

# Compile the game into a shared library (DLL or .so)
echo "Compiling magicians_misfits into a shared library..."
clang++ -std=c++17 -g -O0 -I./ -shared -fPIC magicians_misfits/magicians_misfits.cpp -o bin/debug/magicians_misfits.so

# Link the engine object files into the final executable
echo "Linking object files into the final executable..."
clang++ -std=c++17 -g -rdynamic -o bin/debug/warpunk.out bin/debug/main.o $WARPUNK_O_FILES -lvulkan -ldl -lX11 -lX11-xcb -lxcb

# Check if the build was successful
if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable is located at: bin/debug/warpunk.out"
    echo "Shared library is located at: bin/debug/magicians_misfits.so"
else
    echo "Build failed."
    exit 1
fi

