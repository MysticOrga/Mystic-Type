@echo off
REM Script de compilation pour Windows

echo === R-Type Build Script (Windows) ===

REM Créer le répertoire build s'il n'existe pas
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Se déplacer dans le répertoire build
cd build

REM Générer les fichiers de build avec CMake
echo Running CMake...
cmake .. -G "Visual Studio 17 2026" || cmake .. -G "Visual Studio 16 2019" || cmake .. -G "MinGW Makefiles"

if errorlevel 1 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

REM Compiler tous les projets
echo Building all projects...
cmake --build . --config Release -j

if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo === Build completed successfully! ===
echo Binaries are located in: build\bin\
echo.
echo Available executables:
dir /b bin\*.exe

echo.
echo To run:
echo   build\bin\rtype-client.exe
echo   build\bin\rtype-tcp-server.exe
echo   build\bin\rtype-udp-server.exe
echo.
pause