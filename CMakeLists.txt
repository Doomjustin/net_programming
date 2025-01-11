cmake_minimum_required(VERSION 3.29 FATAL_ERROR)

project (NET_PROGRAMMING VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(AddDependencies)
include(NoInSourceBuild)

add_subdirectory(src)
add_subdirectory(unp)
