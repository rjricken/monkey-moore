// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_CORE_MEMORY_UTILS_HPP
#define MONKEY_CORE_MEMORY_UTILS_HPP

#include <type_traits>
#include <cstddef>

/**
 * Aligns 'num' up to the next multiple of 'Alignment'.
 * The Alignment must be a power of 2 and is checked at compile-time.
 */
template<size_t Alignment, typename T>
constexpr T align_up(T num) {
   static_assert(std::is_integral_v<T>, "'num' must be an integral type");

   static_assert((Alignment & (Alignment - 1)) == 0, "'Alignment' must be a power of 2");
   static_assert(Alignment != 0, "'Alignment' cannot be zero");
   
   constexpr T mask = static_cast<T>(Alignment - 1);

   return (num + mask) & ~mask;
}

#endif // MONKEY_CORE_MEMORY_UTILS_HPP
