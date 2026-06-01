
cmake_minimum_required(VERSION 3.20)
project(sologs
    VERSION   0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD              17)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_EXTENSIONS           OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options 
option(BUILD_TESTS "Build the test suite" OFF)

add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE external/httplib)

add_library(jsonlib INTERFACE)
target_include_directories(jsonlib INTERFACE external/json)

add_library(sqlite3 STATIC external/sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC external/sqlite)

add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

