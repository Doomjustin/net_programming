add_library(unp 
    STATIC
        include/unp.h
        ip.cpp
        socket.cpp
        unp.cpp
)

target_include_directories(unp PUBLIC include)

target_link_libraries(unp
    PUBLIC
        GSL
        fmt::fmt
        spdlog::spdlog
)

add_executable(test_unp test_unp.cpp)
target_link_libraries(test_unp PRIVATE unp)


function(unp_add_executable target)
    add_executable(${target} ${target}.cpp)
    
    target_link_libraries(${target} 
        PRIVATE 
            GSL
            fmt::fmt
            spdlog::spdlog
            unp
    )
endfunction()


add_subdirectory(client)
add_subdirectory(server)

add_subdirectory(chapter1)
add_subdirectory(chapter4)
add_subdirectory(chapter6)
