cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

add_library(sqlite3 STATIC
    vendor/sqlite3/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC
    ${CMAKE_SOURCE_DIR}/vendor/sqlite3
)

project(BooksAndBrews)
add_executable(${PROJECT_NAME}
    src/books_and_brews.cpp
    src/logger.cpp
    src/input.cpp
    src/database.cpp
)

target_link_libraries(${PROJECT_NAME} sqlite3)

if(CMAKE_CONFIGURATION_TYPES) # Multi-config (VS, Xcode)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:BB_DEBUG_BUILD>
    )
else() # Single-config (Makefiles, Ninja)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${PROJECT_NAME} PRIVATE BB_DEBUG_BUILD)
    endif()
endif()
