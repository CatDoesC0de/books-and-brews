@echo off

::Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo This project requires CMake to build.
    exit /b 1
)

if not exist build (
    mkdir build
)

cd build

cmake -S .. -B .
cmake --build . --config Release

cd ..
