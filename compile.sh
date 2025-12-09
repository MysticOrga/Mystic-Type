#!/bin/bash

set -e

BUILD_DIR="build"

build_project() {
    TARGET=$1
    echo "=========================================="
    echo "Building Target: $TARGET"
    echo "=========================================="
    
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir "$BUILD_DIR"
    fi

    cd "$BUILD_DIR"

    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    if [ "$TARGET" = "all" ]; then
        cmake --build . --config Release
    else
        cmake --build . --target "$TARGET" --config Release
    fi

    cd ..
    
    echo "Build finished for $TARGET."
    echo "Executables are located in $BUILD_DIR/src/graphical-client/ or $BUILD_DIR/src/server/"
}

if [ "$1" = "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    echo "Done."
    exit 0
fi

case "$1" in
    client)
        build_project "rtype-client"
        ;;
    server)
        build_project "rtype-server"
        ;;
    all|"")
        build_project "all"
        ;;
    *)
        echo "Usage: ./compile.sh [client|server|all|clean]"
        exit 1
        ;;
esac