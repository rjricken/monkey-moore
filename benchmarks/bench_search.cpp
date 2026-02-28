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
   const size_t buffer_size_bytes = state.range(0);
   auto data = generate_data<DataType>(buffer_size_bytes);

   std::vector<CharType> keyword = { 'a', 'b', 'c', 'd', 'e' };
   MonkeyMoore<DataType> searcher(keyword, 0, {});

   for (auto _ : state) {
      auto results = searcher.search(data.data(), data.size());
      benchmark::DoNotOptimize(results);
   }

   state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(data.size()) * sizeof(DataType));
}

template<typename DataType, int WildcardPos>
static void BM_MonkeyMoore_WildcardRelative(benchmark::State &state) {
   const size_t buffer_size_bytes = state.range(0);
   auto data = generate_data<DataType>(buffer_size_bytes);

   std::vector<CharType> keyword;

   if constexpr (WildcardPos == 0) {
      keyword = { '*', 'b', 'c', 'd', 'e' };
   }
   else if constexpr (WildcardPos == 1) {
      keyword = { 'a', 'b', '*', 'd', 'e' };
   }
   else if constexpr (WildcardPos == 2) {
      keyword = { 'a', 'b', 'c', 'd', '*' };
   }

   MonkeyMoore<DataType> searcher(keyword, '*', {});

   for (auto _ : state) {
      auto results = searcher.search(data.data(), data.size());
      benchmark::DoNotOptimize(results);
   }

   state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(data.size()) * sizeof(DataType));
}

BENCHMARK_TEMPLATE(BM_MonkeyMoore_Relative, uint8_t)
   ->Name("BM_Search/Relative/8-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE(BM_MonkeyMoore_Relative, uint16_t)
   ->Name("BM_Search/Relative/16-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE(BM_MonkeyMoore_WildcardRelative, uint8_t, 0)
   ->Name("BM_Search/Relative/Wildcard/Front/8-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE(BM_MonkeyMoore_WildcardRelative, uint8_t, 1)
   ->Name("BM_Search/Relative/Wildcard/Middle/8-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE(BM_MonkeyMoore_WildcardRelative, uint8_t, 2)
   ->Name("BM_Search/Relative/Wildcard/Back/8-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE(BM_MonkeyMoore_WildcardRelative, uint16_t, 0)
   ->Name("BM_Search/Relative/Wildcard/Front/16-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE(BM_MonkeyMoore_WildcardRelative, uint16_t, 1)
   ->Name("BM_Search/Relative/Wildcard/Middle/16-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_TEMPLATE(BM_MonkeyMoore_WildcardRelative, uint16_t, 2)
   ->Name("BM_Search/Relative/Wildcard/Back/16-Bit")
   ->RangeMultiplier(4)
   ->Range(128<<10, 16<<20);

BENCHMARK_MAIN();