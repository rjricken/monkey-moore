include(FetchContent)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googlebenchmark)

add_executable(benchmarks bench_search.cpp)

target_link_libraries(
    benchmarks
    PRIVATE
    monkey-core
    benchmark::benchmark
    benchmark::benchmark_main
)

if (MSVC)
    target_compile_options(benchmarks PRIVATE /O2)
else()
    target_compile_options(benchmarks PRIVATE -O3)
endif()

