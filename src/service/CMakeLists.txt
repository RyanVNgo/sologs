
add_library(sologs_service
    log_service.cpp
    user_service.cpp
    bootstrap_service.cpp
)

target_include_directories(sologs_service
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(sologs_service
    PUBLIC
    sologs_db
    sologs_domain
    sologs_crypto
    nlohmann_json::nlohmann_json

    PRIVATE
    project_flags
)

target_compile_features(sologs_service PUBLIC cxx_std_20)

