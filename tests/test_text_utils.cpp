// SPDX-License-Identifier: GPL-3.0-or-later

#include "mmoore/text_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>

TEST_CASE("Text utilities: sequence algorithms", "[core][utils]"){
   SECTION("find_last_index") {
      std::array<int, 10> data = {3, 3, 5, 7, 6, 3, 8, 9, 3, 10};

      SECTION("returns index of last occurrence of value repeated multiple times") {
         int last_pos = find_last_index(data.begin(), data.end(), 3);
         CHECK(last_pos == 8);
      }

      SECTION("returns 0 when the target value is not in the sequence") {
         int not_found = find_last_index(data.begin(), data.end(), 2);
         CHECK(not_found == 0);
      }   
   }

   SECTION("count_prefix_length") {
      std::array<int, 10> data = {3, 3, 3, 3, 6, 3, 8, 9, 3, 10};

      SECTION("returns count for element repeated multiple times at the start of the sequence") {
         int count = count_prefix_length(data.begin(), data.end(), 3);
         CHECK(count == 4);
      }

      SECTION("returns 0 for element that is not repeated at the start of the sequence") {
         int count = count_prefix_length(data.begin(), data.end(), 6);
         CHECK(count == 0);
      }

      SECTION("returns 0 when the target element is not in the sequence") {
         int count = count_prefix_length(data.begin(), data.end(), 2);
         CHECK(count == 0);
      }
   }

   std::string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
   std::string lowercase = "abcdefghijklmnopqrstuvwxyz";
   std::string non_alpha = "=+_-.,;()[]{}";

   SECTION("is_ascii_upper") {
      auto count_all_upper = std::count_if(uppercase.begin(), uppercase.end(), is_ascii_upper);
      CHECK(count_all_upper == 26);

      auto count_non_alpha = std::count_if(non_alpha.begin(), non_alpha.end(), is_ascii_upper);
      CHECK(count_non_alpha == 0);
   }

   SECTION("is_ascii_lower") {
      auto count_all_lower = std::count_if(lowercase.begin(), lowercase.end(), is_ascii_lower);
      CHECK(count_all_lower == 26);

      auto count_non_alpha = std::count_if(non_alpha.begin(), non_alpha.end(), is_ascii_lower);
      CHECK(count_non_alpha == 0);
   }
}