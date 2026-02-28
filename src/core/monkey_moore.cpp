// SPDX-License-Identifier: GPL-3.0-or-later

#include "mmoore/monkey_moore.hpp"
#include "mmoore/text_utils.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <cassert>
#include <iostream>

template <class Ty> 
MonkeyMoore<Ty>::MonkeyMoore(
   const std::vector<CharType> &keyword, 
   CharType wildcard, 
   const std::vector<CharType> &char_seq
): wildcard(wildcard), search_mode(none), has_case_change(false) {
   assert(!keyword.empty());

   initialize(keyword, char_seq);
   preprocess();
}

template <class Ty>
MonkeyMoore<Ty>::MonkeyMoore(
   const std::vector <short> &reference_values
): wildcard(0), search_mode(value_scan), has_case_change(false) {
   assert(!reference_values.empty());

   std::vector<CharType> derived_keyword;
   derived_keyword.reserve(reference_values.size());

   for (short v: reference_values) {
      derived_keyword.push_back(static_cast<CharType>(v));
   }

   initialize(derived_keyword, {});
   preprocess();
}

template <class Ty>
std::vector <typename MonkeyMoore<Ty>::result_type> MonkeyMoore<Ty>::search(
   const Ty *data, 
   uint64_t data_len
) {
   return search_mode == simple_relative || search_mode == value_scan ?
      monkey_moore(data, data_len) :
      monkey_moore_wc(data, data_len);
}

/**
* Common internal state initialization logic 
*/
template <class Ty>
void MonkeyMoore<Ty>::initialize(
   const std::vector<CharType> &search_keyword, 
   const std::vector<CharType> &char_seq
) {
   keyword = search_keyword;
   custom_character_seq = char_seq;

   // Multiplied by 2 to account for negative relative differences
   long skip_table_size = static_cast<long>(std::numeric_limits<Ty>::max() + 1) * 2;
   skip_table.assign(skip_table_size, 0);

   bool has_wildcards = std::count(keyword.begin(), keyword.end(), wildcard) > 0;

   if (custom_character_seq.empty() && search_mode != value_scan) {
      auto uppercase_chars = std::count_if(keyword.begin(), keyword.end(), is_ascii_upper);
      auto lowercase_chars = std::count_if(keyword.begin(), keyword.end(), is_ascii_lower);

      has_case_change = uppercase_chars > 0 && lowercase_chars > 0;
   }

   if (search_mode != value_scan) {
      search_mode = has_wildcards || has_case_change ? wildcard_relative : simple_relative;
   }
}

/**
* Preprocess the search key and build the search tables.
*/
template <class Ty>
void MonkeyMoore<Ty>::preprocess () {
   // maps the character pattern positions for easy access
   if (!custom_character_seq.empty()) {
      for (std::size_t i = 0; i < custom_character_seq.size(); i++)
         custom_character_index[custom_character_seq[i]] = static_cast<int>(i);
   }

   if (search_mode == simple_relative || search_mode == value_scan) {
      preprocess_no_wildcards();
   }
   else if (search_mode == wildcard_relative) {
      preprocess_with_wildcards();  
   }
   else {
      throw std::runtime_error("Invalid search mode flag: none");
   }
}

/**
 * Preprocess step for searches that DO NOT use wildcards, which
 * relies on a simpler algorithm with fewer steps
 */
template <class Ty>
void MonkeyMoore<Ty>::preprocess_no_wildcards() {
   CharType *keyword_ptr = keyword.data();
   long keyword_len = static_cast<long>(keyword.size());

   if (custom_character_seq.empty()) {
      keyword_table = compute_relative_values(keyword);
   }
   else {
      keyword_table = compute_relative_values_char_seq(keyword);
   }

   // prefills the jump table with the largest jump size
   std::fill(
      skip_table.begin(), 
      skip_table.end(), 
      static_cast <int> (keyword_len - 1));

   long skip_table_len = static_cast<long>(skip_table.size());

   // negative indices are mapped on positions 0-255, and positive ones on 256-511
   for (int i = keyword_len - 1; i >= 0; i--) {
      int index = keyword_table[i] > 0 
         ? (skip_table_len / 2) + keyword_table[i] 
         : -keyword_table[i];

      if (index >= 0 && index < skip_table_len) {
         // if the skip value for the current index hasn't been set, 
         // we set it to the current distance to the last character in the keyword
         if (skip_table[index] == keyword_len - 1) {
            skip_table[index] = static_cast <int> (keyword_len - i - 1);
         }
      }
      else {
         //TODO: add more informaton to error
         throw std::runtime_error("Skip table index out of bounds");
      }
   }
}

template <class Ty>
void MonkeyMoore<Ty>::preprocess_with_wildcards() {
   long keyword_len = static_cast<long>(keyword.size());
   keyword_wildcards = keyword;
   
   // Step 1: initializes case related variables
   if (custom_character_seq.empty()) {
      // if the keyword has both uppercase and lowercase characters, we replace the least 
      // occurring ones with wildcards which are determined later based on the search results.
      auto uppercase_count = std::count_if(
         keyword.begin(), 
         keyword.end(), 
         is_ascii_upper);

      auto lowercase_count = std::count_if(
         keyword.begin(), 
         keyword.end(), 
         is_ascii_lower);

      mostly_lowercase = lowercase_count > uppercase_count;

      if (uppercase_count > 0 && lowercase_count > 0) {
         if (uppercase_count > lowercase_count) {
            std::replace_if(
               keyword_wildcards.begin(), 
               keyword_wildcards.end(), 
               is_ascii_lower, 
               wildcard);
         }
         else {
            std::replace_if(
               keyword_wildcards.begin(), 
               keyword_wildcards.end(), 
               is_ascii_upper, 
               wildcard);
         }
      }
   }

   // Step 2: Build the wildcard map and collect valid indices

   //TODO: this is a bad nane - true means it's not a wildcard lol
   wildcard_pos_map.resize(keyword_len);

   std::vector<long> valid_indices;
   valid_indices.reserve(keyword_len);

   for (long i = 0; i < keyword_len; i++) {
      bool is_valid = (keyword_wildcards[i] != wildcard);
      wildcard_pos_map[i] = is_valid;

      if (is_valid) {{
         valid_indices.push_back(i);
      }}
   }

   wildcards_count = keyword_len - static_cast<int>(valid_indices.size());

   // Step 3: 1-pass branchless bridging & relative difference calculation

   keyword_table.assign(keyword_len, 0);
   wc_match_stride.assign(keyword_len, 0);
   wc_expected_pattern.assign(keyword_len, 0);
   wc_wildcard_mask.assign(keyword_len, 0);

   /*
      The following table illustrates the computed values for the various lookup structures used
      to speed up relative searches with support to wildcards. e.g. keyword = *ounter**easure 

                     |     0    0    0    0    0    0    0    0    0    0    1    1    1    1    1
                     |     0    1    2    3    4    5    6    7    8    9    0    1    2    3    4
      ---------------+-----------------------------------------------------------------------------
      valid_indices  |     -    x    x    x    x    x    x    -    -    x    x    x    x    x    x
      keyword        |     *    o    u    n    t    e    r    *    *    e    a    s    u    r    e
      stride         |     0  +13   -1   -1   -1   -1   -1    0    0   -3   -1   -1   -1   -1   -1
      mask           |  0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x00 0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      expected_diff  |     0  +10   +6   -7   +6  -15  +13    0    0  -14   -4  +18   +2   -3  -13
   */

   // todo: should always be true?
   if (!valid_indices.empty()) {
      for (size_t k = 0; k < valid_indices.size(); ++k) {
         long current_index = valid_indices[k];

         // find the previous non-wildcard character to bridge the gap
         // if we are at the first character, wrap around to the last one
         long previous_index = (k == 0) ? valid_indices.back() : valid_indices[k - 1];
         
         // computes the stride (an integer number representing the distance to the previous non-wildcard character)
         wc_match_stride[current_index] = static_cast<int>(previous_index - current_index);

         // computes the relative difference table for the keyword directly
         int relative_diff;
         if (custom_character_seq.empty()) {
            relative_diff = keyword_wildcards[current_index] - keyword_wildcards[previous_index];
         }
         else {
            relative_diff = custom_character_index[keyword_wildcards[current_index]]
               - custom_character_index[keyword_wildcards[previous_index]];
         }

         keyword_table[current_index] = relative_diff; // same as expected pattern?
         wc_expected_pattern[current_index] = static_cast<Ty>(relative_diff);
         wc_wildcard_mask[current_index] = static_cast<Ty>(~0ULL);
      }
   }


   // Step 4: builds the skip table (Boyer-Moore bad-character rule)
   std::fill(
      skip_table.begin(), 
      skip_table.end(), 
      static_cast<char> (keyword_len - 1));

   long skip_table_len = static_cast<long>(skip_table.size());

   for (long i = keyword_len - 1; i > 0; --i) {
      // negative indices are mapped on positions 0-255, and positive ones on 256-511
      int index = keyword_table[i] > 0
         ? (skip_table_len / 2) + keyword_table[i]
         : -keyword_table[i];

      if (index >= 0 && index < skip_table_len) {
         // count the number of wildcards present in the range that starts in i + 1
         auto remaining_wildcards_count = std::count(
            keyword_wildcards.begin() + i + 1, 
            keyword_wildcards.end(), 
            wildcard);

         // subtracts the number of wildcards from the regular jump value
         skip_table[index] = static_cast <char> (
            keyword_len - remaining_wildcards_count - i - 1);
      }
      else {
         //TODO: add more informaton to error
         throw std::runtime_error("Skip table index out of bounds");
      }
   }

   // Step 5: builds the wildcard skip table 
   // (similar to the bad-character rule above, but with semantics for wildcards)
   wildcard_skip_table.resize(keyword_len);

   for (long i = keyword_len - 1; i >= 0; --i) {
      if (keyword_wildcards[i] == wildcard) {
         wildcard_skip_table[i] = 1;
      }
      else {
         // finds the last wildcard in the range 
         // that ends at i in keyword_wildcards
         int last_wildcard_index = find_last_index(
            keyword_wildcards.begin(), 
            keyword_wildcards.begin() + i, 
            wildcard);

         if (last_wildcard_index == -1) {
            last_wildcard_index = 0;
         }

         // normalizes the jump value to 1 if the 
         // adjusted value for last_wildcard_index is <= 1
         int normalized_value = std::max<int>(i - last_wildcard_index - 1, 1);
         wildcard_skip_table[i] = static_cast<unsigned char>(normalized_value);
      }
   }
}

/**
 * @brief Performs a Relative Boyer-Moore search on the data buffer.
 * This implementation uses a "lazy" difference calculation strategy to compare 
 * byte relationships on-the-fly, avoiding temporary memory allocations.
 * The search loop is structurally split into two parts (Loop Peeling) to ensure
 * a consistent execution path for the primary comparison logic.
 * @param data Pointer to the start of the data buffer.
 * @param data_len The length of the data buffer.
 * @return std::vector<result_type> A vector of matches containing offset and map.
 */
template<class Ty>
std::vector <typename MonkeyMoore<Ty>::result_type> MonkeyMoore<Ty>::monkey_moore(
   const Ty *data, 
   uint64_t data_len
) {
   using result_type = typename MonkeyMoore<Ty>::result_type;
   using equivalency_map = typename MonkeyMoore<Ty>::equivalency_map;

   std::vector <result_type> results;

   const long keyword_len = static_cast<long>(keyword.size());
   const long skip_table_len = static_cast<long>(skip_table.size());
   const long skip_table_positives_index = skip_table_len / 2;

   const Ty *search_head = data;
   const Ty *data_end = data + data_len;

   int mismatched_rel_value = 0;

   // Helper lambda to calculate the relative difference between two positions.
   // Returns false immediately if the difference doesn't match the precomputed keyword table. 
   auto check_match = [&](long current_index, long previous_index) -> bool {
      int diff = search_head[current_index] - search_head[previous_index];

      if (diff != keyword_table[current_index]) {
         mismatched_rel_value = diff;
         return false;
      }

      return true;
   };

   while (search_head + keyword_len <= data_end) {
      bool match_failed = false;

      // Optimized keyword matching (loop peeling)
      
      // Part 1 - standard contiguous comparison
      // Iterates backwards from the last character down to index 1.
      long k = keyword_len - 1;
      for (; k > 0; --k) {
         // Verifies that every character (except the first) has the
         // correct relative differences from its predecessor.
         if (!check_match(k, k - 1)) {
            match_failed = true;
            break;
         }
      }

      // Part 2 - wrap-around comparison
      // If all characters matched except the first, we verify the relative
      // difference between the first character (index 0) and the last character.
      if (!match_failed) {
         if (!check_match(0, keyword_len - 1)) {
            match_failed = true;
         }
      }

      
      if (!match_failed) {
         equivalency_map result;

         if (search_mode != value_scan) {
            // for value scan we're only interested in the offset

            if (custom_character_seq.empty()) {
               int distance = *search_head - keyword[0];

               result['A'] = static_cast <Ty> ('A' + distance);
               result['a'] = static_cast <Ty> ('a' + distance);
            }
            else {
               int distance = *search_head - custom_character_index[keyword[0]];

               for (CharType c : custom_character_seq) {
                  result[c] = static_cast<Ty>(custom_character_index[c] + distance);
               }
            }
         }

         long match_position = static_cast <long> (std::distance(data, search_head));
         results.push_back({match_position, result});

         search_head += keyword_len - 1;
      }
      else {
         // Calculate jump distance based on the mismatched relative value
         int skip_table_index = mismatched_rel_value > 0 
            ? skip_table_positives_index + mismatched_rel_value 
            : -mismatched_rel_value;

         int jump_value = std::max<int>(skip_table[skip_table_index], 1);

         search_head += jump_value;
      }
   }

   return results;
}


template<class Ty>
std::vector <typename MonkeyMoore<Ty>::result_type> MonkeyMoore<Ty>::monkey_moore_wc(
   const Ty *data, 
   uint64_t data_len
) {
   std::vector<result_type> results;

   long keyword_len = static_cast<long>(this->keyword.size());
   long skip_table_len = static_cast<long>(skip_table.size());

   const Ty *search_head = data;
   const Ty *search_tail = data + keyword_len;

   int leading_wildcards_count = count_prefix_length(
      keyword_wildcards.begin(),
      keyword_wildcards.end(),
      wildcard);

   // replace with.... = leading_wildcards_count??
   long first_non_wildcard_index = 0;
   while (first_non_wildcard_index < keyword_len && !wildcard_pos_map[first_non_wildcard_index]) {
      first_non_wildcard_index++;
   }

   while (search_tail <= data + data_len) {
      int matches = 0;
      int mismatched_rel_value = 0;

      for (; matches < keyword_len; matches++) {
         int i = keyword_len - matches - 1;

         // fetch the previous value by bridging it using the precomputed stride
         Ty current_value = search_head[i];
         Ty previous_value = search_head[i + wc_match_stride[i]];

         // computes the unsigned difference between the current and bridged previous value
         Ty current_diff = static_cast<Ty>(current_value - previous_value);

         // skip over wildcards while matching the current difference with the expected one
         if ((current_diff & wc_wildcard_mask[i]) != wc_expected_pattern[i]) {
            // only compute signed relative difference on a mismatch 
            // so we can use it as index for the skip_table
            mismatched_rel_value = static_cast<int>(current_value) - static_cast<int>(previous_value);
            break;
         }
      }

      if (matches == keyword_len) {
         equivalency_map result;

         // handles ascii values
         if (custom_character_seq.empty()) {
            int distance = static_cast<int>(*(search_head + first_non_wildcard_index))
               - static_cast<int>(keyword_wildcards[first_non_wildcard_index]);

            // if the keyword does not contain case changes, then we need to guess the value
            // of the corresponding character in the opposite case 
            // (e.g. if key is "world", we must guess the value of A)
            if (!has_case_change) {
               result['A'] = static_cast <Ty> ('A' + distance);
               result['a'] = static_cast <Ty> ('a' + distance);
            }
            else {
               // if the keyword contains case changes, then we need to find the first occurrence of
               // a character in the opposite case in order to compute its value
               int first_oposing_case_index = 0;

               while (first_oposing_case_index < keyword_len) {
                  CharType &c = keyword[first_oposing_case_index];
                  bool match = mostly_lowercase ? is_ascii_upper(c) : is_ascii_lower(c);

                  if (match) {
                     break;
                  }

                  first_oposing_case_index++;
               }

               /* Modern C++ STL algorithms version
               auto is_target = [mostly_lowercase](CharType c) {
                  return mostly_lowercase ? is_upper(c) : is_lower(c);
               };

               auto it = std::find_if(keyword.begin(), keyword.end(); is_target);
               int idx = std::distance(keyword.begin(), it);

               if (it == keyword.end()) {
                  throw std::runtime_error("Unexpect end of keyword when finding characters of opposing case");
               }

               int oposing_case_char_distance = *(search_head + idx) - *it;
               */

               int oposing_case_char_distance = 
                  static_cast<int>(*(search_head + first_oposing_case_index))
                     - static_cast<int>(keyword[first_oposing_case_index]);

               result['A'] = mostly_lowercase 
                  ? static_cast <Ty> ('A' + oposing_case_char_distance) 
                  : static_cast <Ty> ('A' + distance);

               result['a'] = mostly_lowercase 
                  ? static_cast <Ty> ('a' + distance) 
                  : static_cast <Ty> ('a' + oposing_case_char_distance);
            }
         }
         else {
            int distance = static_cast<int>(*(search_head + first_non_wildcard_index))
               - custom_character_index[keyword[first_non_wildcard_index]];

            for (CharType c : custom_character_seq) {
               result[c] = static_cast <Ty> (custom_character_index[c] + distance);
            }     
         }

         long offset = static_cast <long> (std::distance(data, search_head));
         results.push_back(std::make_pair(offset, result));
            
         search_head += keyword_len - 1 - leading_wildcards_count;
         search_tail += keyword_len - 1 - leading_wildcards_count;
      }
      else {
         // Calculate jump distance based on the mismatched relative value and wildcard closest wildcard position
         int skip_table_index = mismatched_rel_value > 0 
            ? (skip_table_len / 2) + mismatched_rel_value 
            : -mismatched_rel_value;

         unsigned char wildcard_jump_value = 
            wildcard_skip_table[keyword_len - matches - 1];

         int jump_size = std::min<int>(
            wildcard_jump_value, 
            std::max<int>(skip_table[skip_table_index], 1));

         search_head += jump_size;
         search_tail += jump_size;
      }
   }

   return results;
}

/**
* Computes the relative difference between the elements of the provided array.
*/
template <class Ty>
std::vector<int> MonkeyMoore<Ty>::compute_relative_values(
   const std::vector<CharType> &source
) {
   if (source.empty()) {
      return std::vector<int>();
   }

   std::vector<int> target(source.size());
   target[0] = source[0] - source.back();
   
   for (size_t i = source.size() - 1; i > 0; --i) {
      target[i] = source[i] - source[i - 1];
   }

   return target;
}

template <class Ty>
std::vector<int> MonkeyMoore<Ty>::compute_relative_values_char_seq(
   const std::vector<CharType> &source
) {
   if (source.empty()) {
      return std::vector<int>();
   }

   std::vector<int> target(source.size());
   target[0] = custom_character_index[source[0]] - custom_character_index[source.back()];
   
   for (size_t i = source.size() - 1; i > 0; --i) {
      target[i] = custom_character_index[source[i]] - custom_character_index[source[i - 1]];
   }

   return target;
}

template class MonkeyMoore<uint8_t>;
template class MonkeyMoore<uint16_t>;