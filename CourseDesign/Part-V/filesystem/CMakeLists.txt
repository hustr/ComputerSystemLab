cmake_minimum_required(VERSION 3.13)
project(filesystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(filesystem main.cpp rammiahfs/RFS.cpp rammiahfs/RFS.hpp)
add_executable(format format.cpp rammiahfs/RFS.cpp rammiahfs/RFS.hpp)
