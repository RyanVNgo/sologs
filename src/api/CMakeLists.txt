
add_library(sologs_server
    server.cpp
)

target_include_directories(sologs_server
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(sologs_server
    PUBLIC
    sologs_service
    sologs_utils
    drogon

    PRIVATE
    project_flags
)

target_compile_features(sologs_server PUBLIC cxx_std_20)

