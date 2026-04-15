
add_subdirectory(db)

add_executable(sologs main.cpp)

target_link_libraries(sologs 
    PRIVATE
    cq_db
)

