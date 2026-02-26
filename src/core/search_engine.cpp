// SPDX-License-Identifier: GPL-3.0-or-later

#include "encoding.hpp"
#include "debug_logging.hpp"
#include "mmoore/byteswap.hpp"
#include "mmoore/memory_utils.hpp"
#include "mmoore/search_engine.hpp"

#include <vector>
#include <fstream>
#include <cmath>
#include <future>
#include <algorithm>
#include <filesystem>
#include <mutex>
#include <iterator>
#include <chrono>
#include <unordered_map>
#include <sstream>

#include <iostream>

template <typename DataType>
std::vector<mmoore::SearchResult<DataType>> 
mmoore::SearchEngine<DataType>::run(
   ProgressCallback on_progress, 
   std::atomic<bool> &abort_flag,
   bool generate_previews
) {
   std::vector<mmoore::SearchResult<DataType>> results;

   MMOORE_LOG("config: file_path = ", config.file_path);
   MMOORE_LOG("config: is_relative_search = ", config.is_relative_search);
   MMOORE_LOG("config: endianness = ", config.endianness == mmoore::Endianness::Little ? "Little" : "Big");
   MMOORE_LOG("config: keyword (len) = ", config.keyword.size());
   MMOORE_LOG("config: custom_char_seq (len) = ", config.custom_char_seq.size());
   MMOORE_LOG("config: wildcard = ", config.wildcard);
   MMOORE_LOG("config: reference_values (size) = ", config.reference_values.size());
   MMOORE_LOG("config: preferred_num_threads = ", config.preferred_num_threads);
   MMOORE_LOG("config: preferred_search_block_size = ", config.preferred_search_block_size);
   MMOORE_LOG("config: preferred_preview_width = ", config.preferred_preview_width);

   if (!std::filesystem::exists(config.file_path)) {
      throw std::runtime_error("File not found");
   }

   on_progress(0, mmoore::SearchStep::Initializing);

   uint64_t file_size = std::filesystem::file_size(config.file_path);

   std::unique_ptr<MonkeyMoore<DataType>> searcher;

   if (config.is_relative_search) {
      searcher = std::make_unique<MonkeyMoore<DataType>>(
         config.keyword,
         config.wildcard,
         config.custom_char_seq
      );
   }
   else {
      searcher = std::make_unique<MonkeyMoore<DataType>>(config.reference_values);
   }

   auto blocks = compute_search_blocks(file_size);

   using ResultVector = std::vector<mmoore::SearchResult<DataType>>;
   std::vector<std::future<ResultVector>> active_futures;

   std::mutex progress_mutex;

   int max_threads = (config.preferred_num_threads > 0) 
      ? config.preferred_num_threads 
      : std::thread::hardware_concurrency();

   float total_progress = 0.0f;
   const float progress_increment = 100.0f / blocks.size();

   auto next_block = blocks.begin();

   on_progress(0, mmoore::SearchStep::Searching);

   while (next_block != blocks.end() || !active_futures.empty()) {
      for (auto it = active_futures.begin(); it != active_futures.end(); ) {
         if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            ResultVector local_results = it->get();

            MMOORE_LOG("Worker finished - found ", local_results.size(), " matches");

            if (!local_results.empty()) {
               results.insert(
                  results.end(),
                  std::move_iterator(local_results.begin()),
                  std::move_iterator(local_results.end())
               );
            }
            
            it = active_futures.erase(it);
         }
         else {
            ++it;
         }
      }

      if (next_block != blocks.end() && active_futures.size() < max_threads) {
         SearchBlock current_block = *next_block;

         auto worker = [
            this, 
            current_block, 
            &progress_mutex, 
            &total_progress, 
            progress_increment,
            &on_progress,
            &searcher
         ]() -> ResultVector {   
            ResultVector local_results;

            MMOORE_LOG("Worker spawned for block [offset=", current_block.offset, ", size=", current_block.size, "]");

            std::ifstream file(config.file_path, std::ios::binary);
            if (!file.is_open()) {
               throw std::runtime_error("Worker thread failed to open file: " + config.file_path.string());
            }

            std::vector<uint8_t> raw_buffer(current_block.size);
            file.seekg(current_block.offset);
            file.read(reinterpret_cast<char *>(raw_buffer.data()), current_block.size);

            for (
               uint32_t alignment_padding = 0; 
               alignment_padding < sizeof(DataType); 
               ++alignment_padding
            ) {
               std::vector<uint8_t> work_buffer = raw_buffer;

               DataType *data_ptr = reinterpret_cast<DataType *>(work_buffer.data() + alignment_padding);
               size_t data_count = static_cast<size_t>(floor(double(current_block.size) / sizeof(DataType)));

               if (reinterpret_cast<uint8_t *>(data_ptr + data_count) > work_buffer.data() + current_block.size) {
                  data_count -= 1;
               }

               if (sizeof(DataType) > 1) {
                  mmoore::adjust_endianness(data_ptr, data_count, config.endianness);
               }

               auto matches = searcher->search(data_ptr, data_count);

               local_results.reserve(matches.size());
               for (const auto &[match_position, values_map] : matches) {
                  auto offset = 
                     current_block.offset 
                     + (match_position * sizeof(DataType)) 
                     + alignment_padding;

                  MMOORE_LOG("Match found at offset ", offset);
                  local_results.push_back({ offset, values_map });
               }
            }

            {
               std::lock_guard<std::mutex> lock(progress_mutex);
               total_progress += progress_increment;
               on_progress(static_cast<int>(total_progress), SearchStep::Searching);
            }
            
            return local_results;
         };

         active_futures.push_back(std::async(std::launch::async, worker));
         ++next_block;
      }
      else if (active_futures.size() >= max_threads) {
         std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }

      if (abort_flag) {
         MMOORE_LOG("Search aborted - waiting for ", active_futures.size(), " active threads");

         for (auto &f : active_futures) {
            if (f.valid()) {
               f.wait();
            }
         }

         return {};
      }
   }

   MMOORE_LOG("Search completed - ", results.size(), " results found");
   on_progress(100, GeneratingPreviews);

   std::sort(results.begin(), results.end(), 
      [](mmoore::SearchResult<DataType> &a, mmoore::SearchResult<DataType> &b) {
         return a.offset < b.offset;
      }
   );

   if (generate_previews && !results.empty()) {
      MMOORE_LOG("Starting preview generation for ", results.size(), " results");

      std::ifstream preview_file(config.file_path, std::ios::binary);
      if (!preview_file.is_open()) {
         throw std::runtime_error("Failed to open file to generate previews: " + config.file_path.string());
      }

      std::for_each(results.begin(), results.end(), 
         [this, &preview_file, file_size](mmoore::SearchResult<DataType> &result) {
            MMOORE_LOG("Generating preview for result at offset ", result.offset);
            result.preview = generate_preview(preview_file, file_size, result.offset, result.values_map);
         }
      );
   }

   return results;
}

template<typename DataType>
std::vector<typename mmoore::SearchEngine<DataType>::SearchBlock> 
mmoore::SearchEngine<DataType>::compute_search_blocks(uint64_t file_size) {
   std::vector<SearchBlock> blocks;

   size_t pattern_len =  config.is_relative_search
      ? config.keyword.size()
      : config.reference_values.size();

   const uint32_t overlap_size = (pattern_len - 1) * sizeof(DataType);

   const uint32_t block_base_size = config.preferred_search_block_size;
   const uint32_t full_block_size = block_base_size + overlap_size;

   uint32_t num_blocks = static_cast<uint32_t>(
      std::ceil(static_cast<double>(file_size) / block_base_size)
   );

   MMOORE_LOG("compute_search_blocks: overlap_size = ", overlap_size);
   MMOORE_LOG("compute_search_blocks: block_base_size = " , block_base_size);
   MMOORE_LOG("compute_search_blocks: full_block_size = ", full_block_size);
   MMOORE_LOG("compute_search_blocks: num_blocks: ", num_blocks);

   for (uint32_t i = 0; i < num_blocks; ++i) {
      uint64_t offset = i * block_base_size;

      uint64_t remaining = file_size - offset;
      uint32_t size = static_cast<uint32_t>(
         std::min(static_cast<uint64_t>(full_block_size), remaining)
      );

      blocks.push_back({ offset, size });
   }

   return blocks;
}


template<typename DataType>
std::string mmoore::SearchEngine<DataType>::generate_preview(
   std::ifstream &file, 
   uint64_t file_size,
   uint64_t match_offset, 
   std::map<CharType, DataType> &values_map
) {
   const size_t keyword_len = config.keyword.size();
   const int preview_window_width = config.preferred_preview_width;
   
   // places current match in the center of the preview
   const int kw_half_width = static_cast<int>(std::floor(keyword_len / 2.0));
   const int window_half_width = preview_window_width / 2;

   // calculate ideal start position
   int64_t positions_to_backup = window_half_width - kw_half_width;
   int64_t bytes_to_backup = positions_to_backup * sizeof(DataType);

   // align starting position correctly for multi-byte searches
   bytes_to_backup = align_up<sizeof(DataType)>(bytes_to_backup);

   int64_t start_offset = static_cast<int64_t>(match_offset) - bytes_to_backup;
   int64_t end_offset = start_offset + (preview_window_width * sizeof(DataType));

   if (end_offset > file_size) {
      start_offset -= end_offset - file_size;
   }

   file.seekg(std::max(static_cast<int64_t>(0), start_offset), std::ios::beg);

   std::vector<DataType> buffer(preview_window_width);
   file.read(reinterpret_cast<char *>(buffer.data()), preview_window_width * sizeof(DataType));

   //TODO: remove?
   // handle end of file
   size_t bytes_read = file.gcount();
   size_t items_read = bytes_read / sizeof(DataType);
   buffer.resize(items_read);

   if (sizeof(DataType) > 1) {
      mmoore::adjust_endianness(buffer.data(), buffer.size(), config.endianness);
   }

   return decode_raw_data(values_map, buffer);
}

template<typename DataType>
std::string mmoore::SearchEngine<DataType>::decode_raw_data(
   std::map<CharType, DataType> &values_map, 
   std::vector<DataType> &raw_data
) {
   const bool is_ascii_search = config.custom_char_seq.empty();

   std::unordered_map<DataType, std::string> decoding_map(values_map.size());

   for (const auto &[character, value] : values_map) {
      if (is_ascii_search && (character == 'a'  || character == 'A')) {
         for (auto letter_offset = 0; letter_offset < 26; ++letter_offset) {
            const CharType codepoint = static_cast<CharType>(character) + letter_offset;
            decoding_map[value + static_cast<DataType>(letter_offset)] = mmoore::encoding::to_utf8(codepoint);
         }
      }
      else {
         const CharType codepoint = static_cast<CharType>(character);
         decoding_map[value] = mmoore::encoding::to_utf8(codepoint);
      }
   }

   std::stringstream result_stream;

   if (config.is_relative_search) {
      for (const auto &val : raw_data) {
         if (decoding_map.count(val)) {
            result_stream << decoding_map.at(val);
         }
         else {
            result_stream << "#";
         }
      }
   }
   else {
      result_stream << std::hex << std::uppercase << std::setfill('0');

      for (size_t i = 0; i < raw_data.size(); ++i) {
         result_stream << std::setw(sizeof(DataType) * 2) << static_cast<uint64_t>(raw_data[i]);
         if (i < raw_data.size() - 1) {
            result_stream << " ";
         }
      }
   }

   return result_stream.str();
}

template class mmoore::SearchEngine<uint8_t>;
template class mmoore::SearchEngine<uint16_t>;