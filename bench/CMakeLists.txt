
add_executable(sologs-benchmark
    sologs-benchmark.cpp
)

find_package(CURL REQUIRED)

target_link_libraries(sologs-benchmark
    PUBLIC
    httplib

    PRIVATE
    CURL::libcurl
)

