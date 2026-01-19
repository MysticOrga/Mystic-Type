#!/bin/bash

# Script de compilation pour Linux

set -e  # Arrête le script en cas d'erreur

echo "=== R-Type Build Script (Linux) ==="

# Créer le répertoire build s'il n'existe pas
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Se déplacer dans le répertoire build
cd build

# Générer les fichiers de build avec CMake
echo "Running CMake..."
cmake ..

# Compiler tous les projets
echo "Building all projects..."
make -j$(nproc)

echo ""
echo "=== Build completed successfully! ==="
echo "Binaries are located in: build/bin/"
echo ""
echo "Available executables:"
ls -lh bin/

echo ""
echo "To run:"
echo "  ./build/bin/rtype-client"
echo "  ./build/bin/rtype-tcp-server"
echo "  ./build/bin/rtype-udp-server"