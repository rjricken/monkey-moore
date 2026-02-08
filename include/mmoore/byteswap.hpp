// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_CORE_BYTESWAP_HPP
#define MONKEY_CORE_BYTESWAP_HPP

#include <cstdint>
#include <algorithm>

namespace mmoore {

   enum class Endianness {
      Little,
      Big
   };

   inline Endianness get_system_endianness() {
      const int value = 1;
      return reinterpret_cast<const char *>(&value)[0] ? Endianness::Little : Endianness::Big;
   }

   template <typename T>
   constexpr T swap_always(T value) {
      return value;
   }

   template <>
   constexpr uint16_t swap_always<uint16_t>(uint16_t value) {
      return (value << 8) | (value >> 8);
   }

   template <>
   constexpr uint32_t swap_always<uint32_t>(uint32_t value) {
      return 
         ((value << 24) & 0xFF000000) |
         ((value << 8)  & 0x00FF0000) |
         ((value >> 8)  & 0x0000FF00) |
         ((value >> 24) & 0x000000FF);
   }

   /**
    * Swaps bytes only when the current system is little-endian.
    */
   template <typename T>
   T swap_on_little_endian(T value) {
      if (get_system_endianness() == Endianness::Little) {
         return swap_always(value);
      }

      return value;
   }

   /**
    * Swaps bytes only when the current system is big-endian.
    */
   template <typename T>
   T swap_on_big_endian(T value) {
      if (get_system_endianness() == Endianness::Big) {
         return swap_always(value);
      }

      return value;
   }

   /**
    * Adjust the endianness of a sequence in place.
    * @param dataPtr Pointer to the start of the sequence
    * @param count Number of elements in the sequence (not number of bytes)
    * @param desired_endianness The desired endianness of the data
    */
   template<typename T>
   void adjust_endianness(T *dataPtr, size_t count, Endianness desired_endianness) {
      Endianness system_endianness = get_system_endianness();

      if (system_endianness != desired_endianness) {
         std::transform(dataPtr, dataPtr + count, dataPtr, [](T value) {
            return swap_always<T>(value);
         });
      }
   }
} // namespace mmoore

#endif // MONKEY_CORE_BYTESWAP_HPP