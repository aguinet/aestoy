project(aestoy)
cmake_minimum_required(VERSION 3.2)

include(CTest)

include_directories(include)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)
