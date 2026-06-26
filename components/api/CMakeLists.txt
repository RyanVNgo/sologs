
add_library(sologs.server STATIC)
add_library(sologs::server ALIAS sologs.server)
target_sources(sologs.server
    PRIVATE
        server.cpp
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES
                include/server/server.h
)

target_link_libraries(sologs.server
    PUBLIC
        sologs::service
        sologs::utils
        drogon
    PRIVATE
        project_flags
)

target_compile_features(sologs.server PUBLIC cxx_std_20)

