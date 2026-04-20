
add_library(sologs_db
    database.cpp
    log_repository.cpp
)

target_include_directories(sologs_db
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(sologs_db
    PUBLIC
    sologs_domain

    PRIVATE
    sqlite3
)

target_compile_features(sologs_db PUBLIC cxx_std_20)

