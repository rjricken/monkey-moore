#ifndef MONKEY_CORE_ENCODING_HPP
#define MONKEY_CORE_ENCODING_HPP

#include <string>

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4996)
#endif

#include <locale>
#include <codecvt>

namespace mmoore {
   namespace encoding {

      /**
       * @brief Converts a 32-bit Unicode code point into a UTF-8 string.
       */
      inline std::string to_utf8(char32_t codepoint) {
         std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf8_conv;
         return utf8_conv.to_bytes(codepoint);
      }
   }
}

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
    #pragma warning(pop)
#endif

#endif // MONKEY_CORE_ENCODING_HPP