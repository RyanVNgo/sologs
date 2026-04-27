
enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_library(test_utils
    utils/db_setup.cpp
    utils/db_validate.cpp
)
target_include_directories(test_utils
    PUBLIC
    utils/
)
target_link_libraries(test_utils
    sologs_db
)
target_compile_features(test_utils PUBLIC cxx_std_20)


# ------------------------------------------------------------
# Database Tests
# ------------------------------------------------------------
add_executable(database_tests
    db/db_init_tests.cpp
    db/db_execute_tests.cpp
    db/db_prepared_tests.cpp
    db/db_batch_tests.cpp
    db/db_query_tests.cpp
)
target_link_libraries(database_tests
    PRIVATE
    sologs_db
    test_utils
    sqlite3
    GTest::gtest_main
)
target_include_directories(database_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
)
gtest_discover_tests(database_tests
    WORKING_DIRECTORY $<TARGET_FILE_DIR:database_tests>
)

# ------------------------------------------------------------
# Repository Tests
# ------------------------------------------------------------
add_executable(repo_tests
    db/repo_init_tests.cpp
    db/repo_insert_tests.cpp
)
target_link_libraries(repo_tests
    PRIVATE
    sologs_db
    test_utils
    sqlite3
    GTest::gtest_main
)
target_include_directories(repo_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
)
gtest_discover_tests(repo_tests
    WORKING_DIRECTORY $<TARGET_FILE_DIR:repo_tests>
)

