
cmake_minimum_required(VERSION 3.20)
project(sologs
    VERSION   0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD              20)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_EXTENSIONS           OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options 
option(BUILD_TESTS "Build the test suite" OFF)

include(FetchContent)
include(cmake/CPM.cmake)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_CTL OFF CACHE BOOL "" FORCE)
set(BUILD_DOC OFF CACHE BOOL "" FORCE)
set(BUILD_ORM OFF CACHE BOOL "" FORCE)
set(BUILD_REDIS OFF CACHE BOOL "" FORCE)
set(BUILD_MYSQL OFF CACHE BOOL "" FORCE)
set(BUILD_POSTGRESQL OFF CACHE BOOL "" FORCE)
set(BUILD_SQLITE OFF CACHE BOOL "" FORCE)
CPMAddPackage(
    NAME drogon
    VERSION 1.9.13
    GITHUB_REPOSITORY drogonframework/drogon
    GIT_TAG v1.9.13
)

set(HTTPLIB_COMPILE ON)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY  https://github.com/yhirose/cpp-httplib.git
    GIT_TAG         v0.46.0
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
    GIT_TAG         v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(boost_uuid INTERFACE)
target_include_directories(boost_uuid INTERFACE external/boost)

add_library(sqlite3 STATIC external/sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC external/sqlite)

add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

