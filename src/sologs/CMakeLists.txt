
add_subdirectory(database)
add_subdirectory(server)
add_subdirectory(domain)
add_subdirectory(service)
add_subdirectory(crypto)
add_subdirectory(utils)

add_executable(sologs main.cpp)

target_link_libraries(sologs 
    PRIVATE
        sologs::database
        sologs::server
        sologs::service

        project_flags
)

