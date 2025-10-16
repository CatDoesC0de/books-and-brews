@echo off
call :check_command cmake "This project requires CMake to build."
call :check_command sqlite3 "This project requires SQLite3 at runtime."

if not exist "..\build" (
    mkdir "..\build"
)

set "DB_FILE=..\build\books_and_brews.db"

REM Remove old database file
if exist "%DB_FILE%" (
    del "%DB_FILE%"
)

REM Create a fresh database
sqlite3 "%DB_FILE%" < ..\database\setup.sql
if errorlevel 1 (
    exit /b 1
)

cmake -S .. -B ..\build
if errorlevel 1 exit /b 1

cmake --build ..\build --config Release
if errorlevel 1 exit /b 1

exit /b 0

:check_command
where %1 >nul 2>&1
if errorlevel 1 (
    exit /b 1
)
