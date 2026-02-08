// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_HPP
#define COMMON_HPP

#include "mmoore/search_engine.hpp"
#include "mmoore/text_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <fstream>

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

inline std::vector<CharType> to_vector(const std::u32string &from) {
   return std::vector<CharType>(from.begin(), from.end());
}

inline std::vector<uint8_t> to_vector(const std::string &from) {
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
      [lower_shift, upper_shift](DataType &i) { 
         if (is_ascii_lower(i)) {
            return static_cast<DataType>(i + lower_shift);
         }
         else if (is_ascii_upper(i)) {
            return static_cast<DataType>(i + upper_shift);
         }

         return i;
       });
}

#endif // COMMON_HPP