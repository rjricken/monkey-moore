/*
 * Monkey-Moore - A simple and powerful relative search tool
 * Copyright (C) 2007 Ricardo J. Ricken (Darkl0rd)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef OBJECT_PRED_HPP
#define OBJECT_PRED_HPP

#include <string>
#include <functional>

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
inline bool is_upper (const wxChar &c)
{ return isupper(c) ? true : false; }

/**
* Unary predicate used to say wheter a character is lowercase or not.
* @param c character to be tested
* @return True if it's a lowercase character.
*/
inline bool is_lower (const wxChar &c)
{ return islower(c) ? true : false; }

/**
* Unary predicate used to say whether a character is a digit or not.
* @param c character to be tested
* @return True if it's a digit.
*/
inline bool is_digit (const wxChar &c)
{ return isdigit(c) ? true : false; }

/**
* Functor used to say whether a character is punctuation or not.
*/
struct is_punct : public std::unary_function <wxChar, bool>
{
   /**
   * Functor operator.
   * @param c character to be tested
   * @return True if it's a punctuation character.
   */
   bool operator() (const wxChar &c) const {
      return ispunct(c) ? true : false;
   }
};

/**
* Functor used to say wheter a character is a letter or not.
*/
struct is_alpha : public std::unary_function <wxChar, bool>
{
   /**
   * Functor operator.
   * @param c character to be tested
   * @return True if it's a letter.
   */
   bool operator() (const wxChar &c) const {
      return isalpha(c) ? true : false;
   }
};

#endif //~OBJECT_PRED_HPP
