
add_library(sologs_service
    log_service.cpp
)

target_include_directories(sologs_service
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(sologs_service
    PUBLIC
    sologs_db
    sologs_domain
    jsonlib
)

target_compile_features(sologs_service PUBLIC cxx_std_17)

