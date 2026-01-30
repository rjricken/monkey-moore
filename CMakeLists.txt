cmake_minimum_required(VERSION 3.19)
project(MonkeyMoore)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/gui)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()