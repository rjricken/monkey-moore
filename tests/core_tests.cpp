#include <catch2/catch_test_macros.hpp>
#include "mmoore/monkey_moore.hpp"
#include "mmoore/object_pred.hpp"

#include <array>
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

TEST_CASE("Search algorithm wih no wildcard support", "[core]") {
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
            REQUIRE(results[0].first == 8);

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
            REQUIRE(results[0].first == 4);


            std::vector<uint16_t> expected_values(49);
            std::iota(expected_values.begin(), expected_values.end(), 1);
            assert_char_seq_result(custom_seq, results[0].second, expected_values);
         }
      }
   }
}

TEST_CASE("Search algorithm with wildcard support", "[core]") {
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
            REQUIRE(results[0].first == 8);

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
            REQUIRE(results[0].first == 31);
            REQUIRE(results[0].second['a'] == static_cast<uint16_t>('a' + 15));
            REQUIRE(results[0].second['A'] == static_cast<uint16_t>('A' - 9));
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
            REQUIRE(results[0].first == 5);

            std::vector<uint16_t> expected_values(52);
            std::iota(expected_values.begin(), expected_values.end(), 1);
            assert_char_seq_result(custom_seq, results[0].second, expected_values);
         }
      }
   }
}

TEST_CASE("Helpers functions", "[core]"){
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
   REQUIRE(result.first == expected_offset);

   auto &equivalency_map = result.second;

   REQUIRE(equivalency_map.at('a') == expected_lower_a_value);
   REQUIRE(equivalency_map.at('A') == expected_upper_a_value);
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
         if (is_lower(i)) {
            return static_cast<Ty>(i + lower_shift);
         }
         else if (is_upper(i)) {
            return static_cast<Ty>(i + upper_shift);
         }

         return i;
       });
}