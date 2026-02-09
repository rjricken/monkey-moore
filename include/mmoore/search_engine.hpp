// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_CORE_SEARCH_ENGINE_HPP
#define MONKEY_CORE_SEARCH_ENGINE_HPP

#include <filesystem>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include "mmoore/byteswap.hpp"
#include "mmoore/monkey_moore.hpp"

namespace mmoore {

   template<typename DataType> 
   struct SearchResult {
      uint64_t offset;
      typename MonkeyMoore<DataType>::equivalency_map values_map;
      std::string preview;
   };

   struct SearchConfig {
      std::filesystem::path file_path;

      bool is_relative_search = true;
      mmoore::Endianness endianness = Endianness::Little;

      std::vector<CharType> keyword;
      std::vector<CharType> custom_char_seq = {};
      CharType wildcard = '*';

      std::vector<short> reference_values = {};
      
      int preferred_num_threads = std::thread::hardware_concurrency();
      int preferred_search_block_size = 524288;
      int preferred_preview_width = 50;
   };

   template<typename DataType> 
   class SearchEngine {
   public:
      //TODO: introduce enum to replace the string argument
      using ProgressCallback = std::function<void(int, const std::string &)>;

      explicit SearchEngine(const SearchConfig &cfg) : config(cfg) {}

      std::vector<SearchResult<DataType>> run(
         ProgressCallback on_progress, 
         std::atomic<bool> &abort_flag, 
         bool generate_previews = false
      );

   private:
      SearchConfig config;

      struct SearchBlock {
         uint64_t offset;
         uint32_t size;
      };

      std::vector<SearchBlock> compute_search_blocks(uint64_t file_size);

      std::string generate_preview(
         std::ifstream &file,
         uint64_t file_size,
         uint64_t match_offset, 
         std::map<CharType, DataType> &values_map
      );

      std::string decode_raw_data(
         std::map<CharType, DataType> &values_map,
         std::vector<DataType> &raw_data
      );
   };

}

#endif // MONKEY_CORE_SEARCH_ENGINE_HPP