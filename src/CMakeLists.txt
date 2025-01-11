list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

#catch_discover_tests(test_target)


if (WIN32)
    add_subdirectory(windows)
elseif (UNIX)
    add_subdirectory(posix)
endif()
