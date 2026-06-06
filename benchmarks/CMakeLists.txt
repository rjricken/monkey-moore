find_package(benchmark CONFIG REQUIRED)

add_executable(mmoore-benchmarks bench_search.cpp)

target_link_libraries(
    mmoore-benchmarks
    PRIVATE
    monkey-core
    benchmark::benchmark
    benchmark::benchmark_main
)

if (MSVC)
    target_compile_options(mmoore-benchmarks PRIVATE /O2)
else()
    target_compile_options(mmoore-benchmarks PRIVATE -O3)
endif()

