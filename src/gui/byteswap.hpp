// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BYTESWAP_HPP
#define BYTESWAP_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
   #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include <wx/wx.h>
#endif

#include <cstdint>

/*
   When dealing with endianness, we need to consider 2 things:
    1) the endianness of the system where Monkey-Moore runs,
       which is usually little endian (on x86 hardware)
    2) the endianness specified in the search options. The user
       will choose it based on the endianness of the system he
       is working with.

   Multi-byte searches require that we swap bytes around acording to
   the system endianness and the targeted endianness. The following
   table shows when swapping is needed and when it is not:
                     
   Search option | Big Endian System | Little Endian System
   Little Endian | -                 | swap
   Big Endian    | swap              | -

*/

template <typename _DataType> _DataType swap_always (_DataType val);
template <typename _DataType> _DataType swap_on_le (_DataType val);
template <typename _DataType> _DataType swap_on_be (_DataType val);


// template specializations for swap_always
template <> inline uint8_t swap_always<uint8_t> (uint8_t val)
{ return val; }

template <> inline uint16_t swap_always<uint16_t> (uint16_t val)
{ return wxUINT16_SWAP_ALWAYS(val); }

template <> inline uint32_t swap_always<uint32_t> (uint32_t val)
{ return wxUINT32_SWAP_ALWAYS(val); }


// template specializations for swap_on_le
template <> inline uint8_t swap_on_le<uint8_t> (uint8_t val)
{ return val; }

template <> inline uint16_t swap_on_le<uint16_t> (uint16_t val)
{ return wxUINT16_SWAP_ON_LE(val); }

template <> inline uint32_t swap_on_le<uint32_t> (uint32_t val)
{ return wxUINT32_SWAP_ON_LE(val); }


// template specializations for swap_on_be
template <> inline uint8_t swap_on_be<uint8_t> (uint8_t val)
{ return val; }

template <> inline uint16_t swap_on_be<uint16_t> (uint16_t val)
{ return wxUINT16_SWAP_ON_BE(val); }

template <> inline uint32_t swap_on_be<uint32_t> (uint32_t val)
{ return wxUINT32_SWAP_ON_BE(val); }


#endif //~BYTESWAP_HPP
