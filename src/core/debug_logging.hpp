// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_CORE_DEBUG_LOGGING_HPP
#define MONKEY_CORE_DEBUG_LOGGING_HPP

#ifndef NDEBUG
   #include <iostream>
   #include <mutex>

   inline std::mutex log_mutex;

   constexpr const char *file_basename(const char *path) {
      const char *file = path;

      while (*path) {
         if (*path == '/' || *path == '\\') {
            file = path + 1;
         }
         path++;
      }

      return file;
   }

   template<typename... Args>
   void log_debug_internal(const char *file, int line, Args&&... args) {
      std::lock_guard<std::mutex> lock(log_mutex);
      std::cerr << "[DEBUG] [" << file_basename(file) << ":" << line << "] ";
      
      ((std::cerr << args), ...);
      std::cerr << "\n";
   }

   #define MMOORE_LOG(...) \
      log_debug_internal(__FILE__, __LINE__, __VA_ARGS__)


#else
  #define MMOORE_LOG(...) do {} while(0)

#endif // NDEBUG
#endif // MONKEY_CORE_DEBUG_LOGGING_HPP