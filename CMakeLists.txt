cmake_minimum_required(VERSION 3.27)
project(quaternion)

set(CMAKE_CXX_STANDARD 17)

add_library(quaternion STATIC library.cpp)
