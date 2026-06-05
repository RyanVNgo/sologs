
find_package(OpenSSL REQUIRED)

add_library(sologs_crypto
    crypto.cpp
)

target_include_directories(sologs_crypto
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(sologs_crypto
    PUBLIC
    OpenSSL::Crypto
    stduuid
)

target_compile_features(sologs_crypto PUBLIC cxx_std_20)

