#include <catch2/catch_test_macros.hpp>
#include "mmoore/monkey_moore.hpp"
#include "mmoore/object_pred.hpp"

#include <array>
#include <codecvt>

std::vector<CharType> to_vector(const std::u32string &from);

void assert_char_seq_result(
   const std::map<CharType, uint8_t> &result, 
   const std::string &expected_values
);

TEST_CASE("Monkey-Moore core algorithm wih no wildcard support", "[core]") {
   SECTION("8-bit data type") {
      SECTION("ASCII keyword") {
         std::vector<uint8_t> data = {'d', 'd', 'd', 'c', 'c', 'a', 'c', 'a', 't', 'c', 'h', 'a', 'a', 't'};
         std::transform(
            data.begin(), 
            data.end(), 
            data.begin(), 
            [](uint8_t &i) { return i + 3; });
            
         SECTION("Returns correct offset and equivalency map for matching result") {
            std::vector<CharType> keyword = to_vector(U"catch");
            MonkeyMoore<uint8_t> searcher(keyword);

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            REQUIRE(results[0].first == 6);
            REQUIRE(results[0].second['a'] == static_cast<uint8_t>('a' + 3));
            REQUIRE(results[0].second['A'] == static_cast<uint8_t>('A' + 3));
         }

         SECTION("Returns no results when no match is found") {
            std::vector<CharType> keyword = to_vector(U"maca");
            MonkeyMoore<uint8_t> searcher(keyword);

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 0);
         }
      }

      SECTION("Custom character sequence keyword") {
         std::vector<CharType> custom_seq = to_vector(U"aiueobcdfghjklmnpqrstvwxyz");
         std::vector<uint8_t> data = {'a', 'u', 'q', 'q', 't', 'k', 'c', 'a', 'o', 'a', 'u', 'g', 'k', 'a'};

         SECTION("Returns correct values for matching result") {
            std::vector<CharType> keyword = to_vector(U"match");
            MonkeyMoore<uint8_t> searcher(keyword, 0, custom_seq);
            
            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            REQUIRE(results[0].first == 8);
            assert_char_seq_result(results[0].second, "afghdijkblmnopeqrstucvwxyz");
         }
      }
   }

   SECTION("16-bit data type") {
      SECTION("ASCII keyword") {
         std::vector<uint16_t> data = {
            'q', 'u', 'e', 's', 't', 'i', 'o', 'n', ' ', 'o', 'f', ' ', 'p', 'r', 'i', 'c', 
            'e',  0 , 't', 'h', 'e', ' ', 'l', 'a', 's', 't', ' ', 'w', 'i', 's', 'h',  0
         };

         std::transform(
            data.begin(), 
            data.end(), 
            data.begin(), 
            [](uint16_t &i) { return i - 16; });
            
         SECTION("Returns correct offset and equivalency map for matching result") {
            std::vector<CharType> keyword = to_vector(U"price");
            MonkeyMoore<uint16_t> searcher(keyword);

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            REQUIRE(results[0].first == 12);
            REQUIRE(results[0].second['a'] == static_cast<uint16_t>('a' - 16));
            REQUIRE(results[0].second['A'] == static_cast<uint16_t>('A' - 16));
         }

         SECTION("Returns no results when no match is found") {
            std::vector<CharType> keyword = to_vector(U"station");
            MonkeyMoore<uint16_t> searcher(keyword);

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 0);
         }
      }
   }
}

TEST_CASE("Monkey-Moore helpers functions", "[core]"){
   SECTION("find_last") {
      std::array<int, 10> data = {3, 3, 5, 7, 6, 3, 8, 9, 3, 10};

      SECTION("returns index of last occurrence of value repeated multiple times") {
         int last_pos = find_last(data.begin(), data.end(), 3);
         REQUIRE(last_pos == 8);
      }

      SECTION("returns 0 when the target value is not in the sequence") {
         int not_found = find_last(data.begin(), data.end(), 2);
         REQUIRE(not_found == 0);
      }   
   }

   SECTION("count_begin") {
      std::array<int, 10> data = {3, 3, 3, 3, 6, 3, 8, 9, 3, 10};

      SECTION("returns count for element repeated multiple times at the start of the sequence") {
         int count = count_begin(data.begin(), data.end(), 3);
         REQUIRE(count == 4);
      }

      SECTION("returns 0 for element that is not repeated at the start of the sequence") {
         int count = count_begin(data.begin(), data.end(), 6);
         REQUIRE(count == 0);
      }

      SECTION("returns 0 when the target element is not in the sequence") {
         int count = count_begin(data.begin(), data.end(), 2);
         REQUIRE(count == 0);
      }
   }

   std::string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
   std::string lowercase = "abcdefghijklmnopqrstuvwxyz";
   std::string non_alpha = "=+_-.,;()[]{}";

   SECTION("is_upper") {
      auto count_all_upper = std::count_if(uppercase.begin(), uppercase.end(), is_upper);
      REQUIRE(count_all_upper == 26);

      auto count_non_alpha = std::count_if(non_alpha.begin(), non_alpha.end(), is_upper);
      REQUIRE(count_non_alpha == 0);
   }

   SECTION("is_lower") {
      auto count_all_lower = std::count_if(lowercase.begin(), lowercase.end(), is_lower);
      REQUIRE(count_all_lower == 26);

      auto count_non_alpha = std::count_if(non_alpha.begin(), non_alpha.end(), is_lower);
      REQUIRE(count_non_alpha == 0);
   }
}

std::vector<CharType> to_vector(const std::u32string &from) {
   return std::vector<CharType>(from.begin(), from.end());
}

void assert_char_seq_result(
   const std::map<CharType, uint8_t> &result, 
   const std::string &expected_values
) {
   size_t index = 0;

   for (const auto &entry : result) {
      if (index >= expected_values.size()) {
         FAIL("Map size exceeds expected length");
      }

      uint8_t actual_char = entry.second;
      uint8_t expected_char = static_cast<uint8_t>(expected_values[index]);

      CAPTURE(entry.first);
      CAPTURE(index);

      REQUIRE(actual_char == expected_char);
      index++;
   }
}