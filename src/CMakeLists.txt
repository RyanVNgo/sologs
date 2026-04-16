
add_subdirectory(db)
add_subdirectory(api)
add_subdirectory(domain)
add_subdirectory(service)

add_executable(sologs main.cpp)

target_link_libraries(sologs 
    PRIVATE
    sologs_db
    sologs_server
    sologs_service
)

