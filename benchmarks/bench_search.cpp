// SPDX-License-Identifier: GPL-3.0-or-later

#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <type_traits>

#include "mmoore/monkey_moore.hpp"

template<typename DataType>
static std::vector<DataType> generate_data(size_t size_in_bytes) {
   std::vector<DataType> data(size_in_bytes / sizeof(DataType));
   std::mt19937 rng(42);
   std::uniform_int_distribution<unsigned int> dist(0, std::numeric_limits<DataType>::max());

   for (auto &v : data) {
      v = static_cast<DataType>(dist(rng));
   }

   return data;
}

template<typename DataType>
static void BM_MonkeyMoore_Relative(benchmark::State &state) {
   const size_t buffer_size = state.range(0);
   auto data = generate_data<DataType>(buffer_size);

   std::vector<CharType> keyword = { 'a', 'b', 'c', 'd', 'e' };
   MonkeyMoore<DataType> searcher(keyword, 0, {});

   for (auto _ : state) {
      auto results = searcher.search(data.data(), data.size());
      benchmark::DoNotOptimize(results);
   }

   state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(buffer_size) * sizeof(DataType));
}

BENCHMARK_TEMPLATE(BM_MonkeyMoore_Relative, uint8_t)
   ->Name("BM_Search/Relative/8-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE1(BM_MonkeyMoore_Relative, uint16_t)
   ->Name("BM_Search/Relative/16-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_MAIN();