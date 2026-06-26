
find_package(OpenSSL REQUIRED)

add_library(sologs.crypto STATIC)
add_library(sologs::crypto ALIAS sologs.crypto)
target_sources(sologs.crypto
    PRIVATE
        crypto.cpp
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES
                include/crypto/crypto.h
)

target_link_libraries(sologs.crypto
    PUBLIC
        OpenSSL::Crypto
        stduuid
    PRIVATE
        project_flags
)

target_compile_features(sologs.crypto PUBLIC cxx_std_20)

