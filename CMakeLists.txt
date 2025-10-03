cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

set(CMAKE_BUILD_TYPE Debug)
project(BooksAndBrews)

add_executable(${PROJECT_NAME}
    src/books_and_brews.cpp
    src/logger.cpp
    src/input.cpp
    src/menu.cpp
)

