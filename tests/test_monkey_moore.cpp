// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>
#include "mmoore/monkey_moore.hpp"
#include "mmoore/text_utils.hpp"

#include <numeric>
#include <codecvt>

std::vector<uint8_t> to_vector(const std::string &from);
std::vector<CharType> to_vector(const std::u32string &from);

template <class DataType>
void assert_matching_ascii_result(
   const typename MonkeyMoore<DataType>::result_type &result, 
   const uint64_t expected_offset,
   const DataType expected_lower_a_value,
   const DataType expected_upper_a_value
);

template <class DataType>
void assert_char_seq_result(
   const std::vector<CharType> &char_seq,
   const std::map<CharType, DataType> &result, 
   const std::vector<DataType> &expected_values
);

template <typename DataType>
void shift_alpha_values(
   std::vector<DataType> &sequence, 
   int lower_shift, 
   int upper_shift
);

const auto hiragana_seq = 
   U"あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわをゃっゅょ";

TEST_CASE("Search algorithm: no wildcard support", "[core][relative]") {
   SECTION("8-bit data type") {
      SECTION("ASCII keyword") {
         std::vector<uint8_t> data = {'d', 'd', 'd', 'c', 'c', 'a', 'c', 'a', 't', 'c', 'h', 'a', 'a', 't'};
         shift_alpha_values(data, 3, 3);

         SECTION("Returns correct offset and equivalency map for matching result") {
            std::vector<CharType> keyword = to_vector(U"catch");
            MonkeyMoore<uint8_t> searcher(keyword);

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);

            assert_matching_ascii_result<uint8_t>(results[0], 6, 'a' + 3, 'A' + 3);
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
            CHECK(results[0].first == 8);

            assert_char_seq_result(custom_seq, results[0].second, to_vector("abcdefghijklmnopqrstuvwxyz"));
         }
      }
   }

   SECTION("16-bit data type") {
      SECTION("ASCII keyword") {
         std::vector<uint16_t> data = {
            'q', 'u', 'e', 's', 't', 'i', 'o', 'n', ' ', 'o', 'f', ' ', 'p', 'r', 'i', 'c', 
            'e',  0 , 't', 'h', 'e', ' ', 'l', 'a', 's', 't', ' ', 'w', 'i', 's', 'h',  0
         };
         shift_alpha_values(data, -16, -16);

         SECTION("Returns correct offset and equivalency map for matching result") {
            std::vector<CharType> keyword = to_vector(U"price");
            MonkeyMoore<uint16_t> searcher(keyword);

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            assert_matching_ascii_result<uint16_t>(results[0], 12, 'a' - 16, 'A' - 16);
         }

         SECTION("Returns no results when no match is found") {
            std::vector<CharType> keyword = to_vector(U"station");
            MonkeyMoore<uint16_t> searcher(keyword);

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 0);
         }
      }

      SECTION("Custom character sequence keyword") {
         std::vector<CharType> custom_seq = to_vector(hiragana_seq);

         // data is just あした、わたしたちは、にわに、はなを、まきます 
         // converted to 1-based indices corresponding to the character sequence 
         std::vector<uint16_t> data = {
            1, 12, 16, 110, 44, 16, 12, 16, 17, 26, 110, 22, 44, 22, 110, 26,
            21, 45, 110, 31, 7, 31, 13 
         };

         SECTION("Returns correct values for matching result") {
            std::vector<CharType> keyword = to_vector(U"わたしたちは");
            MonkeyMoore<uint16_t> searcher(keyword, 0, custom_seq);
            
            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            CHECK(results[0].first == 4);


            std::vector<uint16_t> expected_values(49);
            std::iota(expected_values.begin(), expected_values.end(), 1);
            assert_char_seq_result(custom_seq, results[0].second, expected_values);
         }
      }
   }
}

TEST_CASE("Search algorithm: with wildcard support", "[core][relative][wildcard]") {
   SECTION("8-bit data type") {
      SECTION("ASCII keyword") {
         SECTION("All lowercase") {
            std::vector<uint8_t> data = {
               't', 'h', 'e', 'b', 'i', 't', 't', 'e', 'r', 't', 'a', 's', 't', 'e', 'o', 'f', 
               'l', 'e' , 'm', 'o', 'n', 'w', 'i', 't', 'h', 'b', 'u', 't', 't', 'e', 'r', ','
            };
            shift_alpha_values(data, 8, 8);

            SECTION("Returns correct values for matching results") {
               std::vector<CharType> keyword = to_vector(U"b*tter");
               MonkeyMoore<uint8_t> searcher(keyword, '*');

               auto results = searcher.search(data.data(), data.size());
               REQUIRE(results.size() == 2);

               assert_matching_ascii_result<uint8_t>(results[0], 3, 'a' + 8, 'A' + 8);
               assert_matching_ascii_result<uint8_t>(results[1], 25, 'a' + 8, 'A' + 8);
            }

            SECTION("Returns correct values when a different wildcard character is used") {
               std::vector<CharType> keyword = to_vector(U"t?ste");
               MonkeyMoore<uint8_t> searcher(keyword, '?');

               auto results = searcher.search(data.data(), data.size());
               REQUIRE(results.size() == 1);

               assert_matching_ascii_result<uint8_t>(results[0], 9, 'a' + 8, 'A' + 8);               
            }

            SECTION("Returns no results when no match is found") {
               std::vector<CharType> keyword = to_vector(U"past*");
               MonkeyMoore<uint8_t> searcher(keyword);

               auto results = searcher.search(data.data(), data.size());
               REQUIRE(results.size() == 0);
            }
         }

         SECTION("Mixed casing") {
            std::vector<uint8_t> data = {
               'T', 'h', 'e', 'B', 'i', 't', 't', 'e', 'r', 'T', 'r', 'u', 't', 'h', 'A', 'b', 
               'o', 'u' , 't', 'B', 'e', 't', 't', 'e', 'r', 'B', 'u', 't', 't', 'e', 'r', '.'
            };
            shift_alpha_values(data, -32, 24);

            SECTION("Returns correct values for matching results") {
               std::vector<CharType> keyword = to_vector(U"B*tter");
               MonkeyMoore<uint8_t> searcher(keyword, '*');

               auto results = searcher.search(data.data(), data.size());
               REQUIRE(results.size() == 3);

               assert_matching_ascii_result<uint8_t>(results[0], 3, 'a' -32, 'A' + 24);
               assert_matching_ascii_result<uint8_t>(results[1], 19, 'a' -32, 'A' + 24);
               assert_matching_ascii_result<uint8_t>(results[2], 25, 'a' - 32, 'A' + 24);
            }

            SECTION("Returns no results when no match is found") {
               std::vector<CharType> keyword = to_vector(U"Matter");
               MonkeyMoore<uint8_t> searcher(keyword);

               auto results = searcher.search(data.data(), data.size());
               REQUIRE(results.size() == 0);
            }
         }
      }

      SECTION("Custom character sequence keyword") {
         std::vector<CharType> custom_seq = to_vector(U"aiueobcdfghjklmnpqrstvwxyz");
         std::vector<uint8_t> data = {'a', 'u', 'q', 'q', 't', 'k', 'c', 'a', 'o', 'a', 'u', 'g', 'k', 'a'};

         SECTION("Returns correct values for matching result") {
            std::vector<CharType> keyword = to_vector(U"*at*h");
            MonkeyMoore<uint8_t> searcher(keyword, '*', custom_seq);
            
            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            CHECK(results[0].first == 8);

            assert_char_seq_result(custom_seq, results[0].second, to_vector("abcdefghijklmnopqrstuvwxyz"));
         }
      }
   }

   SECTION("16-bit data type") {
      SECTION("ASCII keyword") {
         std::vector<uint16_t> data = {
            'T', 'h', 'e', 'y', ' ', 'm', 'u', 't', 't', 'e', 'r', 'e', 'd', ':', ' ', 'B', 
            'u', 't', 't', 'e', 'r', ',', ' ', 'B', 'E', 'T', 'T', 'E', 'R', ',', ' ', 'B',
            'u', 't', 'c', 'h', 'e', 'r', ',', ' ', 'm', 'a', 't', 't', 'e', 'r'
         };
         shift_alpha_values(data, 15, -9);

         SECTION("Returns correct offset and equivalency map for matching result") {
            std::vector<CharType> keyword = to_vector(U"But**er");
            MonkeyMoore<uint16_t> searcher(keyword, '*');

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            CHECK(results[0].first == 31);
            CHECK(results[0].second['a'] == static_cast<uint16_t>('a' + 15));
            CHECK(results[0].second['A'] == static_cast<uint16_t>('A' - 9));
         }

         SECTION("Returns no results when no match is found") {
            std::vector<CharType> keyword = to_vector(U"*ITTER");
            MonkeyMoore<uint16_t> searcher(keyword, '*');

            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 0);
         }
      }

      SECTION("Custom character sequence keyword") {
         std::u32string additional_kanji = U"学校行";
         std::vector<CharType> custom_seq = to_vector(hiragana_seq + additional_kanji);

         // datais just あしたは 学校に 行きますか？ わたしも 行きたいです。 
         // converted to 1-based indices corresponding to the character sequence 
         std::vector<uint16_t> data = {
            1, 12, 16, 26, 111, 50, 51, 22, 111, 52, 7, 31, 13, 6, 112, 111, 
            44, 16, 12, 35, 111, 52, 7, 16, 2, 113
         };

         SECTION("Returns correct values for matching result") {
            std::vector<CharType> keyword = to_vector(U"**に*行きますか");
            MonkeyMoore<uint16_t> searcher(keyword, '*', custom_seq);
            
            auto results = searcher.search(data.data(), data.size());
            REQUIRE(results.size() == 1);
            CHECK(results[0].first == 5);

            std::vector<uint16_t> expected_values(52);
            std::iota(expected_values.begin(), expected_values.end(), 1);
            assert_char_seq_result(custom_seq, results[0].second, expected_values);
         }
      }
   }
}

TEST_CASE("Search algorithm: value scan mode", "[core][value-scan]") {
   SECTION("8-bit data type") {
      std::vector<uint8_t> data = {
         0x00, 0x00, 0x25, 0x26, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x20, 0x20, 0x00, 0x00, 0x01, 0x00,
         0x01, 0x00, 0x00, 0x89, 0x00, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x81, 0x00, 0x00, 0x01, 0x00, 0x00
      };

      SECTION("Returns correct offset for matching results") {
         std::vector<short> values = { 60, 61, 62, 63, 64, 71};
         MonkeyMoore<uint8_t> searcher(values);

         auto results = searcher.search(data.data(), data.size());
         REQUIRE(results.size() == 2);
         CHECK(results[0].first == 4);
         CHECK(results[1].first == 21);
      }

      SECTION("Returns no results when no match is found") {
         std::vector<short> values = { 80, 81, 82, 83, 84, 85, 86 };
         MonkeyMoore<uint8_t> searcher(values);

         auto results = searcher.search(data.data(), data.size());
         REQUIRE(results.size() == 0);
      }
   }

   SECTION("8-bit data type") {
      std::vector<uint16_t> data = {
         0x0000, 0x0100, 0x0135, 0x0136, 0x0135, 0x0136, 0x0137, 0x0138, 
         0x0139, 0x0140, 0x0120, 0x0120, 0x0000, 0x0100, 0x0101, 0x0000,
         0x0101, 0x0089, 0x0000, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 
         0x0050, 0x0000, 0x0100, 0x0000, 0x0100, 0x0001, 0x0100, 0x0000
      };

      SECTION("Returns correct offset on match") {
         std::vector<short> values = { 105, 106, 107, 108, 109, 116};
         MonkeyMoore<uint16_t> searcher(values);

         auto results = searcher.search(data.data(), data.size());
         REQUIRE(results.size() == 2);
         CHECK(results[0].first == 4);
         CHECK(results[1].first == 19);
      }

      SECTION("Returns no results when no match is found") {
         std::vector<short> values = { 200, 201, 205, 208, 209 };
         MonkeyMoore<uint16_t> searcher(values);

         auto results = searcher.search(data.data(), data.size());
         REQUIRE(results.size() == 0);
      }
   }
}

TEST_CASE("Search algorithm: Boyer-Moore skip table allocation", "[core][internal][regression]") {
   /*
   * Regression Test: Fix for off-by-one error in Boyer-Moore skip table allocation.
   *
   * The skip table size was previously determined by std::numeric_limits<T>::max()
   * (e.g., 255 for uint8_t). However, since values are used as 0-based indices, 
   * a size of 255 is insufficient to store the entry for the value 0xFF.
   *
   * This fix ensures the table size corresponds to the type's cardinality (max() + 1),
   * preventing out-of-bounds access when processing the highest possible byte value.
   */
   SECTION("Skip table correctly handles maximum representable 8-bit value (0xFF)") {
      std::vector<uint8_t> data = {
         0x98, 0x94, 0x00, 0xFF, 0xFF, 0x00, 0x01, 0xA5, 
         0xA1, 0x94, 0x85, 0x98, 0x94
      };

      std::vector<CharType> keyword = to_vector(U"text");
      MonkeyMoore<uint8_t> searcher(keyword);

      auto results = searcher.search(data.data(), data.size());
      REQUIRE(results.size() == 1);

      CHECK(results[0].first == 9);
   }

   SECTION("Skip table correctly handles maximum representable 16-bit value (0xFFFF)") {
      std::vector<uint16_t> data = {
         0x1098, 0x1094, 0x0000, 0xFFFF, 0xFFFF, 0x1000, 0x1001, 0x10A5, 
         0x10A1, 0x1094, 0x1085, 0x1098, 0x1094
      };

      std::vector<CharType> keyword = to_vector(U"text");
      MonkeyMoore<uint16_t> searcher(keyword);

      auto results = searcher.search(data.data(), data.size());
      REQUIRE(results.size() == 1);

      CHECK(results[0].first == 9);
   }
}

std::vector<CharType> to_vector(const std::u32string &from) {
   return std::vector<CharType>(from.begin(), from.end());
}

std::vector<uint8_t> to_vector(const std::string &from) {
   return std::vector<uint8_t>(from.begin(), from.end());
}

template <class DataType>
void assert_matching_ascii_result(
   const typename MonkeyMoore<DataType>::result_type &result, 
   const uint64_t expected_offset,
   const DataType expected_lower_a_value,
   const DataType expected_upper_a_value
) {
   CHECK(result.first == expected_offset);

   auto &equivalency_map = result.second;

   CHECK(equivalency_map.at('a') == expected_lower_a_value);
   CHECK(equivalency_map.at('A') == expected_upper_a_value);
}

template <class DataType>
void assert_char_seq_result(
   const std::vector<CharType> &char_seq,
   const std::map<CharType, DataType> &result, 
   const std::vector<DataType> &expected_values
) {
   size_t index = 0;

   for (const auto &seq_element : char_seq) {
      if (index >= expected_values.size()) {
         FAIL("Character sequence size exceeds expected length");
      }

      DataType actual_char = result.at(seq_element);
      DataType expected_char = expected_values[index];

      CAPTURE(seq_element);
      CAPTURE(index);

      REQUIRE(actual_char == expected_char);
      index++;
   }
}

template <typename DataType>
void shift_alpha_values(std::vector<DataType> &sequence, int lower_shift, int upper_shift) {
   std::transform(
      sequence.begin(), 
      sequence.end(), 
      sequence.begin(), 
      [&](DataType &i) { 
         if (is_ascii_lower(i)) {
            return static_cast<DataType>(i + lower_shift);
         }
         else if (is_ascii_upper(i)) {
            return static_cast<DataType>(i + upper_shift);
         }

         return i;
       });
}