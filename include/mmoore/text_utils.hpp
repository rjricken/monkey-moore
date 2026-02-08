// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXT_UTILS_HPP
#define TEXT_UTILS_HPP

#include <iterator>
#include <cctype>
#include <cstdint>

/**   
 * Finds the index of the last occurrence of 'v'.
 * @return The index, or -1 if not found.
 */
template <class FwdIt, class T>
inline int find_last_index (FwdIt start, const FwdIt end, const T &v)
{
   int last_pos = -1;

   for (int i = 0; start != end; i++)
      if (*start++ == v) last_pos = i;

   return last_pos;
}

/**
 * Counts how many consecutive elements at the start match 'v'.
 */
template <class FwdIt, class T>
inline int count_prefix_length (FwdIt start, const FwdIt end, const T &v)
{
   int n = 0;
   for (; start != end && *start++ == v ; n++);

   return n;
}

/**
 * Checks if a character is an ASCII uppercase letter.
 */
inline bool is_ascii_upper (const char32_t &c) { 
   return (c < 128) && std::isupper(static_cast<unsigned char>(c)); 
}

/**
 * Checks if a character is an ASCII lowercase letter.
 */
inline bool is_ascii_lower (const char32_t &c) { 
   return (c < 128) && std::islower(static_cast<unsigned char>(c)); 
}

/**
 * Checks if a character is an ASCII digit.
 */
inline bool is_ascii_digit (const char32_t &c) { 
   return (c < 128) && std::isdigit(static_cast<unsigned char>(c)); 
}

#endif // TEXT_UTILS_HPP
