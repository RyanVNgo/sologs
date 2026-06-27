
add_library(sologs.service STATIC)
add_library(sologs::service ALIAS sologs.service)
target_sources(sologs.service
    PRIVATE
        log_service.cpp
        user_service.cpp
        bootstrap_service.cpp
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs
            FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/service/user_service.h
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/service/log_service.h
                ${CMAKE_CURRENT_SOURCE_DIR}/../../sologs/service/bootstrap_service.h
)

target_link_libraries(sologs.service
    PUBLIC
        sologs::database
        sologs::domain
        sologs::crypto
        nlohmann_json::nlohmann_json
    PRIVATE
        project_flags
)

target_compile_features(sologs.service PUBLIC cxx_std_20)

