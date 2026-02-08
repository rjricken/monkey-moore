// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "mmoore/search_engine.hpp"
#include <filesystem>
#include <vector>
#include <fstream>
#include <cstdint>

std::vector<CharType> to_vector(const std::u32string &from);

namespace mmoore {
   template<typename DataType>
   bool operator==(const mmoore::SearchResult<DataType> &a, const mmoore::SearchResult<DataType> &b) {
      return a.offset == b.offset && a.preview == b.preview;
   }

   template <typename DataType>
   std::ostream & operator<<(std::ostream &os, const SearchResult<DataType> &sr) {
      os << "{\n"
         << "   offset: " << sr.offset << ", \n"
         << "   preview: " << sr.preview << " \n" 
         << "}\n";
      return os;
   }
}

template <typename DataType>
class TempFile {
public:
   std::filesystem::path path;

   TempFile(const std::string &text_data, int offset = 0) {
      std::vector<DataType> data(text_data.size());
      std::transform(
         text_data.begin(), 
         text_data.end(), 
         data.data(), 
         [offset](const char &c) { 
            return static_cast<DataType>(c + offset);
         }
      );

      init(data);
   }

   TempFile(const std::vector<DataType> &data) {
      init(data);
   }

   ~TempFile() {
      if (std::filesystem::exists(path)) {
         std::filesystem::remove(path);
      }
   }
private:
   void init(std::vector<DataType> data) {
      path = std::filesystem::temp_directory_path() / "mmoore_test_blob.bin";

      std::ofstream file(path, std::ios::binary);
      file.write(reinterpret_cast<const char *>(data.data()), data.size() * sizeof(DataType));
   }
};

TEST_CASE("Search engine: 8-bit relative search correctness", "[search-engine][8-bit][relative-search]") {
   std::vector<uint8_t> file_data = {
      // t     e     x     t     #     #     #     #  (offset 0)
      // #     t     e     x     t     #     #     #  (offset 9)
      // #     #     #     #     #     #     #     #
      // #     t     e     t     e     x     t     #  (offset 27)
      // #     #     #     #     #     #     #     #
      // #     #     #     #     #     #     #     #  
      // #     #     t     e     x     t     #     #  (offset 50)
      // #     #     #     #     t     e     x     t  (offset 60)
      0x94, 0x85, 0x98, 0x94, 0x10, 0x10, 0x11, 0x11,
      0x00, 0x94, 0x85, 0x98, 0x94, 0x00, 0xFF, 0xFF,
      0x00, 0x00, 0x01, 0x0A, 0xFF, 0xFF, 0x00, 0x00,
      0x00, 0x94, 0x85, 0x94, 0x85, 0x98, 0x94, 0x00,
      0xFF, 0x00, 0x0A, 0xFF, 0xFF, 0x01, 0x00, 0x00,
      0xFF, 0x00, 0x0A, 0xFF, 0xFF, 0x01, 0x00, 0x00,
      0x00, 0xFF, 0x94, 0x85, 0x98, 0x94, 0x00, 0xFF,
      0x00, 0x01, 0xA5, 0xA1, 0x94, 0x85, 0x98, 0x94,
   };

   std::vector<mmoore::SearchResult<uint8_t>> expected_results;
   expected_results.push_back({  0, {}, ""});
   expected_results.push_back({  9, {}, ""});
   expected_results.push_back({ 27, {}, ""});
   expected_results.push_back({ 50, {}, ""});
   expected_results.push_back({ 60, {}, ""});

   TempFile temp_file(file_data);
   std::atomic<bool> abort{false};

   mmoore::SearchConfig<uint8_t> config;
   config.file_path = temp_file.path;
   config.keyword = to_vector(U"text");
   config.preferred_preview_width = 4;

   SECTION("Finds all matches under various configurations") {
      int num_threads = GENERATE(1, 4);

      // Search block segmentation strategy
      // 128: larger than the file size
      //   8: perfectly divisible
      //  23: misaligned reads
      //  29: block boundary overlap (splits the keyword across blocks)
      int block_size = GENERATE(128, 8, 23, 29);

      config.preferred_num_threads = num_threads;
      config.preferred_search_block_size = block_size;

      INFO(" Threads: " << num_threads << ", Block size: " << block_size);

      mmoore::SearchEngine<uint8_t> engine(config);
      auto results = engine.run([](int, const std::string &) {}, abort);

      REQUIRE_THAT(results, Catch::Matchers::Equals(expected_results));
   }
}

TEST_CASE("Search engine: 16-bit relative search correctness", "[search-engine][16-bit][relative-search]") {
   std::vector<uint16_t> file_data = {
      //   t       e       x       t       #       #       #       #  (offset 0)
      //   #       t       e       x       t       #       #       #  (offset 18)
      //   #       #       #       #       #       #       #       #
      //   #       t       e       t       e       x       t       #  (offset 54)
      //   #       #       #       #       #       #       #       #
      //   #       #       #       #       #       #       #       #  
      //   #       #       t       e       x       t       #       #  (offset 100)
      //   #       #       #       #       t       e       x       t  (offset 120)
      0x1094, 0x1085, 0x1098, 0x1094, 0x0010, 0x0010, 0x0011, 0x0011,
      0x0000, 0x1094, 0x1085, 0x1098, 0x1094, 0x0000, 0xFFFF, 0xFFFF,
      0x0000, 0x0000, 0x0001, 0x000A, 0xFFFF, 0xFFFF, 0x0000, 0x0000,
      0x0000, 0x1094, 0x1085, 0x1094, 0x1085, 0x1098, 0x1094, 0x0000,
      0xFFFF, 0x0000, 0x000A, 0xFFFF, 0xFFFF, 0x0001, 0x0000, 0x0000,
      0xFFFF, 0x0000, 0x000A, 0xFFFF, 0xFFFF, 0x0001, 0x0000, 0x0000,
      0x0000, 0xFFFF, 0x1094, 0x1085, 0x1098, 0x1094, 0x0000, 0x00FF,
      0x0000, 0x0110, 0xA510, 0x01A1, 0x1094, 0x1085, 0x1098, 0x1094,
   };

   std::vector<mmoore::SearchResult<uint16_t>> expected_results;
   expected_results.push_back({   0, {}, ""});
   expected_results.push_back({  18, {}, ""});
   expected_results.push_back({  54, {}, ""});
   expected_results.push_back({ 100, {}, ""});
   expected_results.push_back({ 120, {}, ""});

   TempFile temp_file(file_data);
   std::atomic<bool> abort{false};

   mmoore::SearchConfig<uint16_t> config;
   config.file_path = temp_file.path;
   config.keyword = to_vector(U"text");
   config.preferred_preview_width = 4;

   SECTION("Finds all matches under various configurations") {
      int num_threads = GENERATE(1, 4);

      // Search block segmentation strategy
      // 256: larger than the file size
      //  16: perfectly divisible
      //  47: misaligned reads
      //  58: block boundary overlap (splits the keyword across blocks)
      int block_size = GENERATE(256, 16, 47, 58);

      config.preferred_num_threads = num_threads;
      config.preferred_search_block_size = block_size;

      INFO(" Threads: " << num_threads << ", Block size: " << block_size);

      mmoore::SearchEngine<uint16_t> engine(config);
      auto results = engine.run([](int, const std::string &) {}, abort);

      REQUIRE_THAT(results, Catch::Matchers::Equals(expected_results));
   }
}

TEST_CASE("Search engine: 8-bit relative search preview generation", "[search-engine][8-bit][preview]") {
   mmoore::SearchConfig<uint8_t> config;
   config.preferred_search_block_size = 16;
   config.preferred_num_threads = 1;

   std::atomic<bool> abort{false};

   SECTION("Finds all matches with correct preview text") {
      TempFile<uint8_t> temp_file("#####the theater's theatrical theatergoer thanked the theatrical theater's theatrics####", 0x10);
      
      std::vector<mmoore::SearchResult<uint8_t>> expected_results;
      expected_results.push_back({  9, {}, "#####the#theater#s#theatr"});
      expected_results.push_back({ 30, {}, "eatrical#theatergoer#than"});
      expected_results.push_back({ 65, {}, "eatrical#theater#s#theatr"});
      
      config.file_path = temp_file.path;
      config.keyword = to_vector(U"theater");
      config.preferred_preview_width = 25;
      
      mmoore::SearchEngine<uint8_t> engine(config);
      auto results = engine.run([](int, const std::string &){}, abort, true);

      REQUIRE_THAT(results, Catch::Matchers::Equals(expected_results));
   }

   SECTION("Handles a match at the start of file") {
      TempFile<uint8_t> temp_file("match me please# \0", 0x0A);

      config.file_path = temp_file.path;
      config.keyword = to_vector(U"match");
      config.preferred_preview_width = 8;

      mmoore::SearchEngine<uint8_t> engine(config);
      auto results = engine.run([](int, const std::string &){}, abort, true);

      REQUIRE(results.size() == 1);

      auto &[offset, values_map, preview] = results[0];
      CHECK(offset == 0);
      CHECK(preview == "match#me");
   }

   SECTION("Handles a match at the end of file") {
      TempFile<uint8_t> temp_file("###reach the final", 0x2A);
      
      config.file_path = temp_file.path;
      config.keyword = to_vector(U"final");
      config.preferred_preview_width = 9;

      mmoore::SearchEngine<uint8_t> engine(config);
      auto results = engine.run([](int, const std::string &){}, abort, true);

      REQUIRE(results.size() == 1);

      auto &[offset, values_map, preview] = results[0];
      CHECK(offset == 13);
      CHECK(preview == "the#final");
   }

   SECTION("Handles a match larger than preview window") {
      TempFile<uint8_t> temp_file("community#understanding#information", -0x1F);

      config.file_path = temp_file.path;
      config.keyword = to_vector(U"understanding");
      config.preferred_preview_width = 11;

      mmoore::SearchEngine<uint8_t> engine(config);
      auto results = engine.run([](int, const std::string &){}, abort, true);

      REQUIRE(results.size() == 1);

      auto &[offset, values_map, preview] = results[0];
      CHECK(offset == 10);
      CHECK(preview == "nderstandin");
   }
}

TEST_CASE("Search engine: 16-bit relative search preview generation", "[search-engine][16-bit][preview]") {
   mmoore::SearchConfig<uint16_t> config;
   config.preferred_search_block_size = 32;
   config.preferred_num_threads = 1;

   std::atomic<bool> abort{false};

   SECTION("Finds all matches with correct preview text") {
      TempFile<uint16_t> temp_file("#####the theater's theatrical theatergoer thanked the theatrical theater's theatrics####", 0x20);
      
      std::vector<mmoore::SearchResult<uint16_t>> expected_results;
      expected_results.push_back({  18, {}, "#####the#theater#s#theatr"});
      expected_results.push_back({  60, {}, "eatrical#theatergoer#than"});
      expected_results.push_back({ 130, {}, "eatrical#theater#s#theatr"});
      
      config.file_path = temp_file.path;
      config.keyword = to_vector(U"theater");
      config.preferred_preview_width = 25;
      
      mmoore::SearchEngine<uint16_t> engine(config);
      auto results = engine.run([](int, const std::string &){}, abort, true);

      REQUIRE_THAT(results, Catch::Matchers::Equals(expected_results));
   }

   SECTION("Handles a match at the start of file") {
      TempFile<uint16_t> temp_file("catch me please# \0");

      config.file_path = temp_file.path;
      config.keyword = to_vector(U"catch");
      config.preferred_preview_width = 8;

      mmoore::SearchEngine<uint16_t> engine(config);
      auto results = engine.run([](int, const std::string &){}, abort, true);

      REQUIRE(results.size() == 1);

      auto &[offset, values_map, preview] = results[0];
      CHECK(offset == 0);
      CHECK(preview == "catch#me");
   }

   SECTION("Handles a match at the end of file") {
      TempFile<uint16_t> temp_file("###the final step");
      
      config.file_path = temp_file.path;
      config.keyword = to_vector(U"step");
      config.preferred_preview_width = 9;

      mmoore::SearchEngine<uint16_t> engine(config);
      auto results = engine.run([](int, const std::string &){}, abort, true);

      REQUIRE(results.size() == 1);

      auto &[offset, values_map, preview] = results[0];
      CHECK(offset == 26);
      CHECK(preview == "inal#step");
   }
}

TEST_CASE("Search engine: error handling", "[search-engine][error]") {
   std::atomic<bool> abort{false};

   mmoore::SearchConfig<uint8_t> config;
   config.file_path = "path/to/inexistent/file";

   SECTION("Throws runtime error if file is not found") {
      mmoore::SearchEngine<uint8_t> engine(config);
      REQUIRE_THROWS_AS(engine.run([](int, const std::string &){}, abort), std::runtime_error);
   }
}

TEST_CASE("Search engine: progress reporting", "[search-engine][progress]") {
   std::vector<uint8_t> file_data(128, 0x00);
   TempFile temp_file(file_data);

   mmoore::SearchConfig<uint8_t> config;
   config.file_path = temp_file.path;
   config.keyword = to_vector(U"text");

   std::atomic<bool> abort{false};
   std::vector<int> progress_history;
   auto progress_callback = [&](int percent, const std::string &) {
      progress_history.push_back(percent);
   };

   SECTION("Progress increases monotonically (single threaded)") {
      config.preferred_num_threads = 1;
      config.preferred_search_block_size = 16;

      mmoore::SearchEngine<uint8_t> engine(config);
      engine.run(progress_callback, abort);

      REQUIRE(progress_history.size() == 9);
      CHECK(progress_history.back() == 100);

      bool is_monotonic = true;
      for (size_t i = 1; i < progress_history.size(); ++i) {
         if (progress_history[i] < progress_history[i - 1]) {
            is_monotonic = false;
            break;
         }
      }

      CHECK(is_monotonic);
   }
}
