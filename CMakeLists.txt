project(aestoy)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-march=native) 
include(CTest)

include_directories(include)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)
