cmake_minimum_required(VERSION 3.5)

project(ipea)

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER_WORKS true)
set(CMAKE_CXX_COMPILER_WORKS true)

add_subdirectory(core)
add_subdirectory(AFL)
add_subdirectory(unittest)
add_subdirectory(compiler-plugins)

