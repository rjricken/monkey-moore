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
): wildcard(wildcard), search_mode(none) {
   assert(!keyword.empty());

   initialize(keyword, char_seq);
   preprocess();
}

template <class Ty>
MonkeyMoore<Ty>::MonkeyMoore(
   const std::vector <short> &reference_values
): wildcard(0), search_mode(value_scan) {
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
   
   if (custom_character_seq.empty()) {
      // if the keyword has both uppercase and lowercase characters,
      // we replace the least occurring ones with wildcards which are
      // determined later based on the search results.
      auto uppercase_count = std::count_if(
         keyword.begin(), 
         keyword.end(), 
         is_ascii_upper);

      auto lowercase_count = std::count_if(
         keyword.begin(), 
         keyword.end(), 
         is_ascii_lower);

      mostly_lowercase = lowercase_count > uppercase_count;

      if (uppercase_count > 0 && lowercase_count > 0)
      {
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

   // START: builds the wildcard position map

   // indices with false mark wildcards
   // TODO: either rename this variable or switch to marking wildcards with true
   wildcard_pos_map.resize(keyword_len);
   for (long i = 0; i < keyword_len; i++) {
      wildcard_pos_map[i] = keyword_wildcards[i] != wildcard;
   }
      
   // START: builds the relative difference table

   wildcards_count = static_cast <int> (
      std::count(
         keyword_wildcards.begin(), 
         keyword_wildcards.end(), 
         wildcard));

   std::vector<CharType> keyword_normalized;
   keyword_normalized.reserve(keyword_len - wildcards_count);

   // appends non-wildcard characters to keyword_normalized
   for (long i = 0; i < keyword_len; i++) {
      if (wildcard_pos_map[i]) {
         keyword_normalized.push_back(keyword_wildcards[i]);
      }
   }

   std::vector<int> normalized_kw_table = custom_character_seq.empty()
      ? compute_relative_values(keyword_normalized)
      : compute_relative_values_char_seq(keyword_normalized);
   
   keyword_table.resize(keyword_len); 

   // fills the keyword table with relative values for non-wildcard characters
   int source_index = static_cast<int>(keyword_normalized.size()) - 1;
   for (int i = keyword_len - 1; i >= 0; --i) {

      if (wildcard_pos_map[i]) {
         keyword_table[i] = normalized_kw_table[source_index];
         source_index -= 1;   
      }
      else {
         keyword_table[i] = 0;
      }
   }

   // START: builds the jump table
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

   // START: builds the wildcard jump table
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

/**
* Performs a boyer-moore based relative search supporting wildcards.
* @param data byte array to search on
* @param data_len data length
* @return The relative values found.
*/
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

   int normalized_keyword_len = keyword_len - wildcards_count;

   std::vector<int> current_rel_table(keyword_len);
   std::vector<Ty> current_normalized(normalized_keyword_len);
   std::vector<int> current_normalized_rel_table(normalized_keyword_len);

   // current_rel_table = tmp_tbl
   // current_normalized = tmp_pure
   // current_normalized_rel_table = tmp_tbl_tmp

   while (search_tail <= data + data_len) {
      // builds a normalized view of the current search position
      for (long i=0, j=0; i < keyword_len; i++) {
         if (wildcard_pos_map[i]) {
            current_normalized[j++] = search_head[i];
         }
      }

      // computes relative values for current position's normalized view
      calc_reltable(
         current_normalized.data(), 
         current_normalized_rel_table.data(), 
         normalized_keyword_len);

      // computes relative values for the current position,
      // assigning 0 to indices containing wildcards 
      long target_index = keyword_len - wildcards_count - 1;
      for (long i = keyword_len - 1; i >= 0; --i) {
         if (wildcard_pos_map[i]) {
            current_rel_table[i] = current_normalized_rel_table[target_index];
            target_index--;
         }
         else {
            current_rel_table[i] = 0;
         }
      }

      // compares the relative values
      int matches = 0;
      for (; matches < keyword_len; matches++) {
         int current_index = keyword_len - matches - 1;

         if (keyword_wildcards[current_index] == wildcard)
            continue;
         if (current_rel_table[current_index] != keyword_table[current_index])
            break;
      }

      // we got a match
      if (matches == keyword_len) {
         equivalency_map result;

         long first_non_wildcard_index = 0;
         while (!wildcard_pos_map[first_non_wildcard_index]) {
            first_non_wildcard_index++;
         }

         // handles ascii values
         if (custom_character_seq.empty()) {
            int distance = *(search_head + first_non_wildcard_index) 
               - keyword_wildcards[first_non_wildcard_index];

            // if the key contains the same capitalization, then we guess the value
            // of the opposite character (ie: if key is "world", we must guess the value of A)
            if (!has_case_change) {
               result['A'] = static_cast <Ty> ('A' + distance);
               result['a'] = static_cast <Ty> ('a' + distance);
            }
            else {
               // if the key contains any capitalization changes, we need to
               // find the correct value of the less frequent case.

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
                  *(search_head + first_oposing_case_index) 
                     - keyword[first_oposing_case_index];

               result['A'] = mostly_lowercase 
                  ? static_cast <Ty> ('A' + oposing_case_char_distance) 
                  : static_cast <Ty> ('A' + distance);

               result['a'] = mostly_lowercase 
                  ? static_cast <Ty> ('a' + distance) 
                  : static_cast <Ty> ('a' + oposing_case_char_distance);
            }
         }
         else {
            int distance = *(search_head + first_non_wildcard_index) 
               - custom_character_index[keyword[first_non_wildcard_index]];

            for (CharType c : custom_character_seq) {
               result[c] = static_cast <Ty> (custom_character_index[c] + distance);
            }     
         }

         long offset = static_cast <long> (std::distance(data, search_head));
         results.push_back(std::make_pair(offset, result));

         int leading_wildcards_count = count_prefix_length(
            keyword_wildcards.begin(), 
            keyword_wildcards.end(), 
            wildcard);
            
         search_head += keyword_len - 1 - leading_wildcards_count;
         search_tail += keyword_len - 1 - leading_wildcards_count;
      }
      else {
         // for partial matches we have to figure out how many units to jump over
         int &mismatched_rel_value = current_rel_table[keyword_len - matches - 1];

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
* Calculates the relative difference between the bytes
* on src and stores the results on tbl.
* @param src source bytes
* @param tbl output table
* @param size source length
*/
template <class Ty>
template <class T> 
void MonkeyMoore<Ty>::calc_reltable(
   const T *source, 
   int *target, 
   int size
) const {
   target[0] = source[0] - source[size - 1];

   for (int i = size - 1; i > 0; --i)
      target[i] = source[i] - source[i - 1];
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

/**
* Custom character pattern version of calc_reltable.
* @param src source bytes
* @param tbl output table
* @param size source length
*/
/*void calc_reltable_cp (const CharType *src, int *tbl, int size)
{
   tbl[0] = cp_pos[src[0]] - cp_pos[src[size - 1]];

   for (int i = size - 1; i > 0; --i)
      tbl[i] = cp_pos[src[i]] - cp_pos[src[i - 1]];
}*/

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