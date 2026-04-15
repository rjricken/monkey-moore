find_package(benchmark CONFIG REQUIRED)

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

