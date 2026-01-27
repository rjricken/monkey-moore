// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OBJECT_PRED_HPP
#define OBJECT_PRED_HPP

#include <cctype>
#include <iterator>

/**
* The find_last() function searches for the element denoted by v. If such
* an element is found between start and end, the position of the last found
* element is returned. If no such element is found, 0 is returned. 
* @param start iterator to the begin of the sequence
* @param end iterator to the end of the sequenve
* @param v desired element
* @return Last position of the desired element.
*/
template <class FwdIt, class T>
inline int find_last (FwdIt start, const FwdIt end, const T &v)
{
   int last_pos = 0;

   for (int i = 0; start != end; i++)
      if (*start++ == v) last_pos = i;

   return last_pos;
}

/**
* Counts how many times an element appears repeatedly
* on the beggining of the given sequence.
* @param start iterator to the begin of the sequence
* @param end iterator to the end of the sequence
* @param v desired element
* @return The results of the counting process.
*/
template <class FwdIt, class T>
inline int count_begin (FwdIt start, const FwdIt end, const T &v)
{
   int n = 0;
   for (; start != end && *start++ == v ; n++);

   return n;
}

/**
* Unary predicate used to say wheter a character is uppercase or not.
* @param c character to be tested
* @return True if it's a uppercase character.
*/
inline bool is_upper (const char32_t &c) { 
   return (c < 128) && std::isupper(static_cast<unsigned char>(c)); 
}

/**
* Unary predicate used to say wheter a character is lowercase or not.
* @param c character to be tested
* @return True if it's a lowercase character.
*/
inline bool is_lower (const char32_t &c) { 
   return (c < 128) && std::islower(static_cast<unsigned char>(c)); 
}

/**
* Unary predicate used to say whether a character is a digit or not.
* @param c character to be tested
* @return True if it's a digit.
*/
inline bool is_digit (const char32_t &c) { 
   return (c < 128) && isdigit(static_cast<unsigned char>(c)); 
}

#endif //~OBJECT_PRED_HPP
