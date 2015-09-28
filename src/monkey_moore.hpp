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

#ifndef MONKEY_MOORE_HPP
#define MONKEY_MOORE_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "object_pred.hpp"

#include <wx/wxchar.h>
#include <memory>
#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
#include <limits>

typedef unsigned char u8;
typedef unsigned short u16;

/**
* Manages and performs relative searchs.
*/
template <class Ty> class MonkeyMoore
{
public:
   /**
   * Relative search constructor. Initializes attributes and allocates memory.
   * @param keyword search key
   * @param wildcard user defined wildcard
   * @param pattern custom character set
   */
   MonkeyMoore (const wxString &keyword, const wxChar &wildcard = 0, const wxString &pattern = wxT(""))
   : card(wildcard), type(none)
   {
      wxASSERT(keyword.length() != 0);

      //auto patternStr = pattern.c_str();
      //const wxChar *patternPtr = pattern.length() ? patternStr.AsInternal() : 0;

      init(keyword.c_str(), keyword.length(), pattern.c_str());
   }

   /**
   * Value scan relative constructor. Initializes attributes and allocates memory.
   * @param keyword search key
   * @param wildcard user defined wildcard
   */
   MonkeyMoore (const std::vector <short> &vals)
   : card(0)
   {
      wxASSERT(vals.size() != 0);
      type = value_scan;

      const int vsize = static_cast <int> (vals.size());
      std::unique_ptr<wxChar[]> temp(new wxChar[vsize]);

      for (int i = 0; i < vsize; i++)
         temp[i] = static_cast <wxChar> (vals[i]);

      init(temp.get(), vsize, 0); 
   }

   /**
   * Destructor. Frees allocated memory.
   */
   ~MonkeyMoore ()
   {
      delete [] key;
      delete [] key_tbl;
      delete [] skip;

      if (type == wildcard_relative)
      {
         delete [] mdkey;
         delete [] wc_pos;
         delete [] cards;
      }

      if (cplen)
         delete [] char_pattern;
   }

   typedef std::map <wxChar, Ty> equivalency_type;
   typedef std::pair <long, equivalency_type> relative_type;

   /**
   * Performs a relative search/value scan relative based on the
   * constructor called during instantiation.
   * @param data byte array to search on
   * @param len data length
   * @return Search results.
   */
   std::vector <relative_type> search (const Ty *data, long len)
   {
      return type == simple_relative || type == value_scan ?
         monkey_moore(data, len) :
         monkey_moore_wc(data, len);
   }

private:
   /**
   * Preprocess the search key and build the search tables.
   */
   void preprocess ()
   {
      bool wildcards = std::count(key, key + klen, card) > 0;

      if (!cplen && type != value_scan)
         case_change = std::count_if(key, key + klen, is_upper) && std::count_if(key, key + klen, is_lower);

      if (type != value_scan)
         type = wildcards || case_change ? wildcard_relative : simple_relative;

      // maps the character pattern positions for easy access
      if (cplen)
      {
         for (int i = 0; i < cplen; i++)
            cp_pos[char_pattern[i]] = i;
      }

      if (type == simple_relative || type == value_scan)
      {
         key_tbl = new int[klen];
         !cplen ? calc_reltable(key, key_tbl, klen) : calc_reltable_cp(key, key_tbl, klen);

         // prepares the jump table
         std::fill(skip, skip + sklen, static_cast <char> (klen - 1));

         // negative indices are mapped on positions 0-255, and positive ones on 256-511
         for (int i = klen - 1; i > 0; i--)
         {
            int index = key_tbl[i] > 0 ? (sklen / 2) + key_tbl[i] : -key_tbl[i];

            if (skip[index] == klen - 1)
               skip[index] = static_cast <char> (klen - i - 1);
         }
      }
      else // type == wildcard_relative
      {
         mdkey = new wxChar[klen];
         std::copy(key, key + klen, mdkey);

         if (!cplen)
         {
            // determines if the majority of characters is upper or lower.
            // therefore, if the majority is lower, the uppers are replaced with
            // wildcards and found later by comparison.
            int n_upper = static_cast <int> (std::count_if(key, key + klen, is_upper));
            int n_lower = static_cast <int> (std::count_if(key, key + klen, is_lower));
            lower = n_lower > n_upper;

            if (n_upper && n_lower)
            {
               n_upper > n_lower ?
                  std::replace_if(mdkey, mdkey + klen, is_lower, card) :
                  std::replace_if(mdkey, mdkey + klen, is_upper, card);
            }
         }

         // --- builds the wildcard position table
         // indices with false mark wildcards
         wc_pos = new bool[klen];

         for (int i = 0; i < klen; i++)
            wc_pos[i] = mdkey[i] != card;


         // --- builds the relative difference table
         n_wildcards = static_cast <int> (std::count(mdkey, mdkey + klen, card));
         wxChar *mdkey_pure = new wxChar[klen - n_wildcards];

         for (int i = 0, j = 0; i < klen; i++)
            if (wc_pos[i]) mdkey_pure[j++] = mdkey[i];

         int *key_tbl_tmp = new int[klen - n_wildcards];
         
         !cplen ?
            calc_reltable(mdkey_pure, key_tbl_tmp, klen - n_wildcards) :
            calc_reltable_cp(mdkey_pure, key_tbl_tmp, klen - n_wildcards);

         key_tbl = new int[klen];

         for (int i = klen - 1, j = klen - n_wildcards - 1; i >= 0; i--)
            key_tbl[i] = wc_pos[i] ? key_tbl_tmp[j--] : 0;

         // --- builds the jump table
         std::fill(skip, skip + sklen, static_cast<char> (klen - 1));

         // negative indices are mapped on positions 0-255, and positive ones on 256-511
         for (int i = klen - 1; i > 0; i--)
         {
            int index = key_tbl[i] > 0 ? (sklen / 2) + key_tbl[i] : -key_tbl[i];

            if (skip[index] == klen - 1)
               skip[index] = static_cast <char> (klen - std::count(mdkey + i + 1, mdkey + klen, card) - i - 1);
         }

         // --- builds the wildcard jump table
         cards = new u8[klen];

         for (int i = klen - 1; i >= 0; i--)
            cards[i] = mdkey[i] == card ? 1 : static_cast <u8> (std::max<int>(i - find_last(mdkey, mdkey + i, card) - 1, 1));

         // --- clean up
         delete [] mdkey_pure;
         delete [] key_tbl_tmp;
      }
   }

   /**
   * Performs some initialization stuff.
   * @param kw pointer to keyword
   * @param ksz keyword length
   * @param cp pointer to character pattern
   */
   void init (const wxChar *kw, const int ksz, const wxChar *cp)
   {
      key = mdkey = char_pattern = 0;
      klen = cplen = 0;
      key_tbl = 0;
      wc_pos = 0;
      cards = 0;
      skip = 0;
      case_change = false;

      klen = ksz;
      key = new wxChar[klen];
      std::copy(kw, kw + klen, key);

      if (wxStrlen(cp))
      {
         cplen = wxStrlen(cp);
         char_pattern = new wxChar[cplen];
         std::copy(cp, cp + cplen, char_pattern);
      }

      sklen = std::numeric_limits<Ty>::max() * 2;
      skip = new char[sklen];

      preprocess();
   }

   /**
   * Performs a boyer-moore based relative search.
   * @param data byte array to search on
   * @param hlen data length
   * @return The relative values found.
   */
   std::vector <relative_type> monkey_moore (const Ty *data, long hlen)
   {
      std::vector <relative_type> results;

      const Ty *hpos_end = data + klen;
      const Ty *hpos_start = data;

      int *tmp_tbl = new int[klen];

      while (hpos_end <= data + hlen)
      {
         // builds the current block table
         calc_reltable(hpos_start, tmp_tbl, klen);

         // compares the relative tables
         int hits = 0;
         for (; hits < klen && tmp_tbl[klen - hits - 1] == key_tbl[klen - hits - 1]; hits++);

         // we got a match
         if (hits == klen)
         {
            equivalency_type eq;

            if (type != value_scan)
            {
               // for value scan, we're only interested in the offset, not the values

               if (!cplen)
               {
                  int dist = *hpos_start - key[0];

                  eq[wxT('A')] = static_cast <Ty> (wxT('A') + dist);
                  eq[wxT('a')] = static_cast <Ty> (wxT('a') + dist);
               }
               else
               {
                  int base_diff = *hpos_start - cp_pos[key[0]];

                  for (int i = 0; i < cplen; i++)
                     eq[char_pattern[i]] = static_cast <Ty> (cp_pos[char_pattern[i]] + base_diff);
               }
            }

            results.push_back(make_pair(static_cast <long> (std::distance(data, hpos_start)), eq));

            hpos_end += klen - 1;
            hpos_start += klen - 1;
         }
         else
         {
            // key didn't fully match, so we must figure out how many bytes to jump over

            int &elem = tmp_tbl[klen - hits - 1];
            int jump = std::max<char>(skip[elem > 0 ? (sklen / 2) + elem : -elem], 1);

            hpos_end += jump;
            hpos_start += jump;
         }
      }

      delete [] tmp_tbl;
      return results;
   }



   /**
   * Performs a boyer-moore based relative search (supporting wildcards).
   * @param data byte array to search on
   * @param hlen data length
   * @return The relative values found.
   */
   std::vector <relative_type> monkey_moore_wc (const Ty *data, long hlen)
   {
      std::vector <relative_type> results;

      const Ty *hpos_end = data + klen;
      const Ty *hpos_start = data;

      int plen = klen - n_wildcards;

      int *tmp_tbl = new int[klen];
      int *tmp_tbl_tmp = new int[plen];
      Ty *tmp_pure = new Ty[plen];

      while (hpos_end <= data + hlen)
      {
         // calculates the current block's table
         for (int i=0, j=0; i<klen; i++)
            if (wc_pos[i]) tmp_pure[j++] = hpos_start[i];

         calc_reltable(tmp_pure, tmp_tbl_tmp, plen);

         for (int i = klen - 1, j = klen - n_wildcards - 1; i >= 0; i--)
            tmp_tbl[i] = wc_pos[i] ? tmp_tbl_tmp[j--] : 0;

         // compares the relative tables
         int hits = 0;
         for (; hits < klen; hits++)
         {
            if (mdkey[klen - hits - 1] == card)
               continue;
            if (tmp_tbl[klen - hits - 1] != key_tbl[klen - hits - 1])
               break;
         }

         // we got a match
         if (hits == klen)
         {
            equivalency_type eq;

            int index = 0;
            for (; !wc_pos[index]; index++);

            // handles ascii values
            if (!cplen)
            {
               int diff = *(hpos_start + index) - mdkey[index];

               // if the key contains the same capitalization, then we guess the value
               // of the opposite character (ie: if key is "world", we must guess the value of A)
               if (!case_change)
               {
                  eq[wxT('A')] = static_cast <Ty> (wxT('A') + diff);
                  eq[wxT('a')] = static_cast <Ty> (wxT('a') + diff);
               }
               else
               {
                  // if the key contains any capitalization changes, we need to
                  // find the correct value of the less frequent case.

                  int pos = 0;
                  for (; lower ? !is_upper(key[pos]) : !is_lower(key[pos]); pos++);
                  int diff2 = *(hpos_start + pos) - key[pos];

                  eq[wxT('A')] = lower ? static_cast <Ty> (wxT('A') + diff2) : static_cast <Ty> (wxT('A') + diff);
                  eq[wxT('a')] = lower ? static_cast <Ty> (wxT('a') + diff) : static_cast <Ty> (wxT('a') + diff2);
               }
            }
            else
            {
               int base_diff = *(hpos_start + index) - cp_pos[key[index]];

               for (int i = 0; i < cplen; i++)
                  eq[char_pattern[i]] = static_cast <Ty> (cp_pos[char_pattern[i]] + base_diff);
            }

            results.push_back(std::make_pair(static_cast <long> (std::distance(data, hpos_start)), eq));

            int nw = count_begin(mdkey, mdkey + klen, card);
            hpos_end += klen - 1 - nw;
            hpos_start += klen - 1 - nw;
         }
         else
         {
            // key didn't fully match, so we must figure out how many bytes to jump over

            int &elem = tmp_tbl[klen - hits - 1];
            int jump = std::min<u8>(cards[klen - hits - 1], std::max<char>(skip[elem > 0 ? (sklen / 2) + elem : -elem], 1));

            hpos_end += jump;
            hpos_start += jump;
         }
      }

      delete [] tmp_tbl_tmp;
      delete [] tmp_tbl;
      delete [] tmp_pure;

      return results;
   }

   /**
   * Calculates the relative difference between the bytes
   * on src and stores the results on tbl.
   * @param src source bytes
   * @param tbl output table
   * @param size source length
   */
   template <class T> inline void calc_reltable (const T *src, int *tbl, int size) const
   {
      tbl[0] = src[0] - src[size - 1];

      for (int i = size - 1; i > 0; --i)
         tbl[i] = src[i] - src[i - 1];
   }

   /**
   * Custom character pattern version of calc_reltable.
   * @param src source bytes
   * @param tbl output table
   * @param size source length
   */
   void calc_reltable_cp (const wxChar *src, int *tbl, int size)
   {
      tbl[0] = cp_pos[src[0]] - cp_pos[src[size - 1]];

      for (int i = size - 1; i > 0; --i)
         tbl[i] = cp_pos[src[i]] - cp_pos[src[i - 1]];
   }

   // general attributes

   wxChar *key;        /**< search key            */
   long klen;          /**< key length            */
   int *key_tbl;       /**< key's relative table  */
   char *skip;         /**< jump table            */
   long sklen;         /**< skip table lenght     */

   enum { none, simple_relative, wildcard_relative, value_scan } type;

   // wildcard search attributes

   wxChar *mdkey;      /**< modified key (ie: MonkeyMoore -> *onkey*oore) */
   u8 *cards;          /**< wildcards jump table */ // <-- check this!
   bool *wc_pos;       /**< wildcard map */

   bool case_change;   /**< indicates change in key's capitalization */
   bool lower;         /**< there are more lower characters then upper? */
   int n_wildcards;    /**< how many wildcards in key */

   const wxChar card;  /**< wildcard character (0 means no wildcard) */

   // special attributes

   bool custom_charpattern;
   wxChar *char_pattern;
   int cplen;

   std::map <wxChar, int> cp_pos;
};

#endif //~MONKEY_MOORE_HPP
