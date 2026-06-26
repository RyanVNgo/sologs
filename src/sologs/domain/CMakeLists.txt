
add_library(sologs.domain INTERFACE)
add_library(sologs::domain ALIAS sologs.domain)
target_sources(sologs.domain
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs
            FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/domain/auth.h
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/domain/log_entry.h
)

target_compile_features(sologs.domain INTERFACE cxx_std_20)

