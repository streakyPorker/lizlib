

#ifndef LIZLIB_LOGGER_H
#define LIZLIB_LOGGER_H

#include <fmt/format.h>
#include <iostream>
#include "common/basic.h"

namespace lizlib {
enum class Level { kTrace, kDebug, kInfo, kWarn, kError, kDisable };
extern Level log_level;
class Timestamp;

#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#else
#define LOG_TRACE(...)                                                                    \
  do {                                                                                    \
    if (log_level <= Level::kTrace) {                                                     \
      fmt::print("[TRACE] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), __LINE__); \
      fmt::println(__VA_ARGS__);                                                          \
      std::cout.flush();                                                                  \
    }                                                                                     \
  } while (0);

#define ASSERT_TRACE(expr, ...) \
  if (!(expr)) {                \
    LOG_TRACE(__VA_ARGS__);     \
  }

#define LOG_DEBUG(...)                                                                    \
  do {                                                                                    \
    if (log_level <= Level::kDebug) {                                                     \
      fmt::print("[DEBUG] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), __LINE__); \
      fmt::println(__VA_ARGS__);                                                          \
    }                                                                                     \
  } while (0);

#define ASSERT_DEBUG(expr, ...) \
  if (!(expr)) {                \
    LOG_DEBUG(__VA_ARGS__);     \
  }

#define LOG_INFO(...)                                                                    \
  do {                                                                                   \
    if (log_level <= Level::kInfo) {                                                     \
      fmt::print("[INFO] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), __LINE__); \
      fmt::println(__VA_ARGS__);                                                         \
    }                                                                                    \
  } while (0);

#define ASSERT_INFO(expr, ...) \
  if (!(expr)) {               \
    LOG_INFO(__VA_ARGS__);     \
  }

#define LOG_WARN(...)                                                                    \
  do {                                                                                   \
    if (log_level <= Level::kWarn) {                                                     \
      fmt::print("[WARN] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), __LINE__); \
      fmt::println(__VA_ARGS__);                                                         \
    }                                                                                    \
  } while (0);

#define ASSERT_WARN(expr, ...) \
  if (!(expr)) {               \
    LOG_WARN(__VA_ARGS__);     \
  }

#define LOG_ERROR(...)                                                                    \
  do {                                                                                    \
    if (log_level <= Level::kError) {                                                     \
      fmt::print("[ERROR] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), __LINE__); \
      fmt::println(__VA_ARGS__);                                                          \
    }                                                                                     \
  } while (0);
#endif

#define ASSERT_ERROR(expr, ...) \
  if (!(expr)) {                \
    LOG_ERROR(__VA_ARGS__);     \
  }

#define LOG_FATAL(...)                                                                  \
  do {                                                                                  \
    fmt::print("[FATAL] {} {}:{} ", Timestamp::Now(), std::string(__FILE__), __LINE__); \
    fmt::println(__VA_ARGS__);                                                          \
    std::terminate();                                                                   \
  } while (0);

#define ASSERT_FATAL(expr, ...) \
  if (!(expr)) {                \
    LOG_FATAL(__VA_ARGS__);     \
  }

}  // namespace lizlib

#endif  //LIZLIB_LOGGER_H
