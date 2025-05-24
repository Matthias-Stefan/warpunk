#!/bin/bash
set -e

# ================================
# Paths
# ================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEBUG_DIR="$SCRIPT_DIR/bin/debug"
CORE_DIR="$SCRIPT_DIR/warpunk.core"
RUNTIME_DIR="$SCRIPT_DIR/warpunk.runtime"
GAME_SRC="$SCRIPT_DIR/magicians_misfits/magicians_misfits.cpp"

mkdir -p "$DEBUG_DIR"
mkdir -p "$DEBUG_DIR/warpunk.core"
mkdir -p "$DEBUG_DIR/warpunk.runtime"

# ================================
# Compiler and flags
# ================================
CXX=clang++
CXXFLAGS="-std=c++17 -g -O0 -Wall -Wextra -fPIC -DWARPUNK_DEBUG=1"
INCLUDES="-I$SCRIPT_DIR -I$CORE_DIR -I$RUNTIME_DIR"
LIBS="-lvulkan -ldl -lX11 -lX11-xcb -lxcb"

# ================================
# Build warpunk.core (shared lib)
# ================================
echo
echo "========================================="
echo "Building warpunk.core (Shared Library)"
echo "========================================="

CORE_OBJS=""
for src in $(find "$CORE_DIR" -name "*.cpp"); do
    obj="$DEBUG_DIR/warpunk.core/$(basename "$src" .cpp).o"
    echo "Compiling $src"
    $CXX $CXXFLAGS -DWARPUNK_EXPORT=1 $INCLUDES -c "$src" -o "$obj"
    CORE_OBJS="$CORE_OBJS $obj"
done

$CXX -shared -o "$DEBUG_DIR/libwarpunk.core.so" $CORE_OBJS $LIBS

# ================================
# Build warpunk.runtime (shared lib)
# ================================
echo
echo "========================================="
echo "Building warpunk.runtime (Shared Library)"
echo "========================================="

RUNTIME_OBJS=""
for src in $(find "$RUNTIME_DIR" -name "*.cpp"); do
    obj="$DEBUG_DIR/warpunk.runtime/$(basename "$src" .cpp).o"
    echo "Compiling $src"
    $CXX $CXXFLAGS -DWARPUNK_EXPORT=1 $INCLUDES -c "$src" -o "$obj"
    RUNTIME_OBJS="$RUNTIME_OBJS $obj"
done

$CXX -shared -o "$DEBUG_DIR/libwarpunk.runtime.so" $RUNTIME_OBJS \
    -L"$DEBUG_DIR" -lwarpunk.core $LIBS

# ================================
# Build magicians_misfits (executable)
# ================================
echo
echo "========================================="
echo "Building magicians_misfits (Executable)"
echo "========================================="

GAME_OBJ="$DEBUG_DIR/magicians_misfits.o"
$CXX $CXXFLAGS $INCLUDES -DWARPUNK_IMPORT=1 -c "$GAME_SRC" -o "$GAME_OBJ"

$CXX -o "$DEBUG_DIR/magicians_misfits" "$GAME_OBJ" \
    -L"$DEBUG_DIR" -Wl,-rpath,'$ORIGIN' \
    -lwarpunk.core -lwarpunk.runtime $LIBS

# ================================
# Done
# ================================
echo
echo "========================================="
echo "Build complete!"
echo "  → $DEBUG_DIR/libwarpunk.core.so"
echo "  → $DEBUG_DIR/libwarpunk.runtime.so"
echo "  → $DEBUG_DIR/magicians_misfits"
echo "========================================="
