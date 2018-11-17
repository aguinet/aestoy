project(aestoy)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BENCHMARK_LIB benchmark CACHE STRING "Benchmark library to use")
set(BENCHMARK_INCLUDE "" CACHE STRING "Benchmark include directory")
set(BENCHMARK_USE_PTHREAD ON CACHE STRING "Benchmark include directory")

if (BENCHMARK_USE_PTHREAD)
  set(BENCHMARK_LIB ${BENCHMARK_LIB} pthread)
endif()
include(CTest)

include_directories(include)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)
