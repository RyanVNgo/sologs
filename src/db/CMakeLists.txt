
add_library(cq_db
    database.cpp
)

target_include_directories(cq_db
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(cq_db

    PRIVATE
    sqlite3
)

target_compile_features(cq_db PUBLIC cxx_std_17)

