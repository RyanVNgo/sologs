
add_executable(sologs main.cpp)

target_link_libraries(sologs 
    PRIVATE
        sologs::database
        sologs::server
        sologs::service

        project_flags
)

