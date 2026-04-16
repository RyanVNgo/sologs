
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

function(add_gtest TARGET)
    add_executable(${TARGET} ${ARGN})
    target_link_libraries(${TARGET}
        PRIVATE
        sologs_db
        GTest::gtest_main
    )
    target_include_directories(${TARGET}
        PRIVATE
        "${CMAKE_SOURCE_DIR}/src/"
    )
    gtest_discover_tests(${TARGET})
endfunction()

include(GoogleTest)

add_gtest(database_testing
    database_tests.cpp
)

