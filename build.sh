#! /bin/bash
mkdir -p build/linux
cd build/linux
cmake ../..
make -j$(nproc)
if [ $? -eq 0 ]; then
    echo -e "Linux build successful!"
else
    echo -e "Linux build failed!\n"
    exit 1
fi
cd ../..

# Build for Windows
mkdir -p build/windows
cd build/windows
cmake -DCMAKE_TOOLCHAIN_FILE=../../mingw-w64-toolchain.cmake ../..
make -j$(nproc)
if [ $? -eq 0 ]; then
    echo -e "Windows build successful!"
else
    echo -e "Windows build failed!\n"
    exit 1
fi