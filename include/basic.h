

#ifndef LIZLIB_BASIC_H
#define LIZLIB_BASIC_H
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <sys/time.h>
#include <cstdint>
#include <limits>
#include <string>

#include "const.h"

#define FORMATTER_REGISTRY(T)                                \
  template <>                                                \
  struct fmt::formatter<T> {                                 \
    constexpr auto parse(format_parse_context& ctx)          \
      -> format_parse_context::iterator {                    \
      return ctx.end();                                      \
    }                                                        \
    auto format(const T& item, format_context& ctx) const {  \
      return fmt::format_to(ctx.out(), "{}", item.String()); \
    };                                                       \
  }

namespace lizlib {

struct MallocDeleter {
  void operator()(void* ptr) { free(ptr); }
};

template <typename T>
struct Comparable {
  inline friend bool operator<(const T& p, const T& q) noexcept {
    return T::Compare(p, q) < 0;
  }
  friend bool operator<=(const T& p, const T& q) noexcept {
    return T::Compare(p, q) <= 0;
  }
  friend bool operator==(const T& p, const T& q) noexcept {
    return T::Compare(p, q) == 0;
  }
  friend bool operator>=(const T& p, const T& q) noexcept {
    return T::Compare(p, q) >= 0;
  }
  friend bool operator>(const T& p, const T& q) noexcept {
    return T::Compare(p, q) > 0;
  }
  friend bool operator!=(const T& p, const T& q) noexcept {
    return T::Compare(p, q) != 0;
  }
};

struct Duration : Comparable<Duration> {
  int64_t usecs{};
  Duration() = delete;
  explicit Duration(int64_t usec_diff) : usecs(usec_diff) {}
};

struct Timestamp : public Comparable<Timestamp> {
  int64_t usecs{};

  Timestamp() = default;
  explicit Timestamp(int64_t usecs) : usecs(usecs) {}

  static Timestamp Now() {
    struct timeval tv {};

    ::gettimeofday(&tv, nullptr);
    return Timestamp{tv.tv_usec + tv.tv_sec * kUsecPerSec};
  };

  static Timestamp Max() {
    return Timestamp{std::numeric_limits<int64_t>::max()};
  }

  static Timestamp Min() { return Timestamp{0}; }

  static int Compare(const Timestamp& p, const Timestamp& q) noexcept {
    return p.usecs == q.usecs ? 0 : p.usecs < q.usecs ? -1 : 1;
  }

  Duration operator-(const Timestamp& other) const noexcept {
    return Duration{usecs - other.usecs};
  }

  Timestamp operator+(const Duration& d) const noexcept {
    return Timestamp{usecs + d.usecs};
  }

  Timestamp operator-(const Duration& d) const noexcept {
    return Timestamp{usecs - d.usecs};
  }

  std::string String() const noexcept {
    time_t msecs = usecs / kUsecPerMsec;
    time_t tmp_usec = usecs % kUsecPerMsec;
    return fmt::format("{:%Y-%m-%d %H:%M:%S}.{:06}", fmt::localtime(msecs),
                       tmp_usec);
  };
};

}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::Timestamp);

#endif  //LIZLIB_BASIC_H
