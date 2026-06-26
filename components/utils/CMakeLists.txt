
add_library(sologs.utils STATIC)
add_library(sologs::utils ALIAS sologs.utils)
target_sources(sologs.utils
    PRIVATE
        utils.cpp
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES
                include/utils/utils.h
)

target_link_libraries(sologs.utils
    PUBLIC
        sologs::domain
    PRIVATE
        project_flags
)

target_compile_features(sologs.utils INTERFACE cxx_std_20)

