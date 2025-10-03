#!/usr/bin/env bash

set -e

#Check if CMake is installed
if ! command -v cmake >/dev/null 2>&1
then
    echo "This project requires CMake to build."
    exit 1
fi

mkdir -p build 

cd build

cmake -S .. -B .
cmake --build . --config Release

cd ..
