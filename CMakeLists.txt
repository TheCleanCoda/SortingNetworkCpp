cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(SortingNetworkCpp)

set(BUILD_MAJOR "0")
set(BUILD_MINOR "0")
set(BUILD_VERSION "1")
set(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_VERSION})

option(BUILD_BENCHMARK "Build the benchmark" ON)
option(BUILD_TESTS "Build the tests" OFF)

if(BUILD_TESTS)
  if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  add_subdirectory("extern/googletest/googletest")
  enable_testing()
  include(GoogleTest)
endif()

set(SN_BENCHMARK_EXECUTABLE_NAME "benchmark")
set(SN_TESTS_EXECUTABLE_NAME "tests")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "/W3 /O2 /fp:fast")
  add_compile_options("-bigobj")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -ffast-math")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

include_directories("include")

if (BUILD_BENCHMARK)
  add_executable(${SN_BENCHMARK_EXECUTABLE_NAME} "src/benchmark.cpp")
endif()

if(BUILD_TESTS)

  if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
  endif()
  include_directories("extern/googletest/googletest/include")
  add_executable(${SN_TESTS_EXECUTABLE_NAME} "test/sorting_network_tests.cpp")
  target_link_libraries(${SN_TESTS_EXECUTABLE_NAME} gtest gtest_main)
  gtest_discover_tests(${SN_TESTS_EXECUTABLE_NAME})
endif()
