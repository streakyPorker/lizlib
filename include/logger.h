

#ifndef LIZLIB_LOGGER_H
#define LIZLIB_LOGGER_H

#include <fmt/format.h>

namespace lizlib {
enum class Level {
  kTrace,
  kInfo,
  kWarn,
  kError,
  kDisable,
};
extern Level log_level;

#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#else
#define LOG_TRACE(format, ...)                                                 \
  do {                                                                         \
    if (log_level <= Level::kTrace) {                                          \
      fmt::print("[TRACE] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), \
                 __LINE__);                                                    \
      fmt::println((format), __VA_ARGS__);                                     \
    }                                                                          \
  } while (0);

#define LOG_DEBUG(format, ...)                                                 \
  do {                                                                         \
    if (log_level <= Level::kDebug) {                                          \
      fmt::print("[DEBUG] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), \
                 __LINE__);                                                    \
      fmt::println((format), __VA_ARGS__);                                     \
    }                                                                          \
  } while (0);

#define LOG_INFO(format, ...)                                                 \
  do {                                                                        \
    if (log_level <= Level::kInfo) {                                          \
      fmt::print("[INFO] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), \
                 __LINE__);                                                   \
      fmt::println((format), __VA_ARGS__);                                    \
    }                                                                         \
  } while (0);

#define LOG_ERROR(format, ...)                                                 \
  do {                                                                         \
    if (log_level <= Level::kError) {                                          \
      fmt::print("[ERROR] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), \
                 __LINE__);                                                    \
      fmt::println((format), __VA_ARGS__);                                     \
    }                                                                          \
  } while (0);
#endif
}  // namespace lizlib

#endif  //LIZLIB_LOGGER_H
