
add_library(sologs_domain INTERFACE)

target_include_directories(sologs_domain
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_features(sologs_domain INTERFACE cxx_std_20)

