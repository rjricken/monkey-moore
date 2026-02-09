// SPDX-License-Identifier: GPL-3.0-or-later

#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>

#include "mmoore/monkey_moore.hpp"

static std::vector<uint8_t> generate_data(size_t size) {
   std::vector<uint8_t> data(size);
   std::mt19937 rng(42);
   std::uniform_int_distribution<int> dist(0, 255);

   for (auto &v : data) {
      v = static_cast<uint8_t>(dist(rng));
   }

   return data;
}

static void BM_MonkeyMoore_Relative_8Bit(benchmark::State &state) {
   const size_t buffer_size = state.range(0);
   auto data = generate_data(buffer_size);

   std::vector<CharType> keyword = { 'a', 'b', 'c', 'd', 'e' };
   MonkeyMoore<uint8_t> searcher(keyword, 0, {});

   for (auto _ : state) {
      auto results = searcher.search(data.data(), data.size());
      benchmark::DoNotOptimize(results);
   }

   state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(buffer_size));
}

BENCHMARK(BM_MonkeyMoore_Relative_8Bit)
   ->RangeMultiplier(2)
   ->Range(128<<10, 32<<20);

BENCHMARK_MAIN();