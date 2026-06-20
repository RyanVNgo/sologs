
add_library(sologs_utils
    utils.cpp
)

target_include_directories(sologs_utils
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(sologs_utils
    PUBLIC
    sologs_domain
    
    PRIVATE
    project_flags
)

target_compile_features(sologs_utils INTERFACE cxx_std_20)

