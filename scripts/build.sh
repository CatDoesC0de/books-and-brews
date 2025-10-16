#!/usr/bin/env bash

set -e

#Check if CMake is installed
if !command -v cmake >/dev/null 2>&1
then
    echo "This project requires CMake to build."
fi

mkdir -p ../build 

#Check if sqlite3 is in the PATH
if ! command -v sqlite3 >/dev/null >&1;
then
    echo "This project requires SQLite3 at runtime."
fi

DB_FILE="../build/books_and_brews.db"

#Remove old database file if it exists
rm -f "$DB_FILE"

#Create a fresh database
sqlite3 "$DB_FILE" < ../database/setup.sql

cmake -S .. -B ../build 
cmake --build ../build --config Release
