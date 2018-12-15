cmake_minimum_required(VERSION 3.10)
project(microsha)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")
add_executable(microsha main.cpp)