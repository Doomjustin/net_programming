function(add_exe target)
    add_executable(${target} ${target}.cpp)

    target_link_libraries(${target}
        PRIVATE
            GSL
            fmt::fmt
            spdlog::spdlog
            wsock32
            ws2_32
    )
endfunction()


add_exe(hello_server)
add_exe(hello_client)