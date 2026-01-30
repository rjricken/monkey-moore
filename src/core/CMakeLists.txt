add_library(monkey-core STATIC monkey_moore.cpp)

target_include_directories(monkey-core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(monkey-core PUBLIC ${CMAKE_SOURCE_DIR}/include)