
add_library(sologs.database STATIC)
add_library(sologs::database ALIAS sologs.database)
target_sources(sologs.database
    PRIVATE
        database.cpp
        log_repository.cpp
        auth_repository.cpp
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs
            FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/database/database.h
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/database/auth_repository.h
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/database/log_repository.h
)

target_link_libraries(sologs.database
    PUBLIC
        sologs::domain
        sologs::utils
    PRIVATE
        sqlite3
        project_flags
)

target_compile_features(sologs.database PUBLIC cxx_std_20)

