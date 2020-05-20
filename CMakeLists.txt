cmake_minimum_required(VERSION 3.16)
project(Dreamachine)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(submodules)
add_subdirectory(src)