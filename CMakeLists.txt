cmake_minimum_required(VERSION 3.19)
project(MonkeyMoore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

option(BUILD_TESTING "Build unit tests" OFF)
option(BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(MMOORE_FORCE_LOGGING "Force enable internal logging in all builds")

if(MMOORE_FORCE_LOGGING)
    add_compile_definitions(MMOORE_ENABLE_LOGGING)
    message(STATUS "Monkey-Moore: Forced internal logging is ENABLED")
endif()

add_subdirectory(src/core)
add_subdirectory(src/gui)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()