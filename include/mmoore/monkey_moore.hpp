// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_CORE_MONKEY_MOORE_HPP
#define MONKEY_CORE_MONKEY_MOORE_HPP

#include <memory>
#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
#include <limits>
#include <cassert>
#include <cstdint>

using CharType = char32_t;

template <class Ty> class MonkeyMoore {
public:
   using equivalency_map = std::map<CharType, Ty>;
   using result_type = std::pair<uint64_t, equivalency_map>;

   /**
    * Standard relative search constructor.
    * @param keyword search keyword
    * @param wildcard character representing a wildcard
    * @param char_seq user defined character sequence
    */
   MonkeyMoore(
      const std::vector<CharType> &keyword, 
      CharType wildcard = 0, 
      const std::vector<CharType> &char_seq = {}
   );


   /**
   * Value scan relative constructor.
   * @param reference_values Values representing the desired relative pattern
   */
   MonkeyMoore(
      const std::vector <short> &reference_values
   );

   /**
   * Performs a relative search/value scan relative based on the
   * constructor called during instantiation
   * @param data pointer to binary data to be searched
   * @param data_len length of data
   * @return Search results
   */
   std::vector <result_type> search(const Ty *data, uint64_t data_len);

private:
   enum { none, simple_relative, wildcard_relative, value_scan } search_mode;
   
   std::vector<CharType> keyword;
   std::vector<int> keyword_table; // rename to expected_diff
   std::vector<int> skip_table;

   const CharType wildcard;
   std::vector<CharType> keyword_wildcards;
   std::vector<unsigned char> wildcard_skip_table;
   std::vector<bool> wildcard_pos_map; // rename to non_wc_pos_map?

   // new wildcard impl
   std::vector<int> wc_match_stride; // rename to wc_prev_stride
   std::vector<Ty> wc_expected_pattern; // rename to wc_expected_diff
   std::vector<Ty> wc_wildcard_mask;

   bool has_case_change;
   bool mostly_lowercase;
   int wildcards_count;

   std::vector<CharType> custom_character_seq;
   std::map<CharType, int> custom_character_index;

   void initialize(
      const std::vector<CharType> &search_keyword, 
      const std::vector<CharType> &char_seq
   );

   void preprocess();
   void preprocess_no_wildcards();
   void preprocess_with_wildcards();

   std::vector <result_type> monkey_moore(const Ty *data, uint64_t data_len);
   std::vector <result_type> monkey_moore_wc(const Ty *data, uint64_t data_len);

   std::vector<int> compute_relative_values(
      const std::vector<CharType> &source
   );

   std::vector<int> compute_relative_values_char_seq(
      const std::vector<CharType> &source
   );
};

#endif // MONKEY_CORE_MONKEY_MOORE_HPP
