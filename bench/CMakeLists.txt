
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.5
)
FetchContent_MakeAvailable(benchmark)

add_executable(all_benchmarks
    bench_main.cpp

    e2e/post_logs.cpp
    e2e/get_logs.cpp
    e2e/post_auth.cpp
    e2e/get_auth.cpp

    database/database_ph.cpp
    server/server_ph.cpp
    service/service_ph.cpp
    utils/utils_ph.cpp
)
target_link_libraries(all_benchmarks
    PRIVATE
        sologs::server
        sologs::service
        sologs::database
        sologs::crypto
        sologs::utils

        GTest::gtest_main
        GTest::gmock
        benchmark::benchmark_main
)

