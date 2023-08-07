

#ifndef LIZLIB_BASIC_H
#define LIZLIB_BASIC_H
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <sys/time.h>
#include <any>
#include <cassert>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <utility>
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

#define DISABLE_MOVE(T)     \
  T(T&&) noexcept = delete; \
  T& operator=(T&&) noexcept = delete

#define DISABLE_COPY(T)               \
  T(const T&) noexcept = delete;      \
  T(T&) noexcept = delete;            \
  T& operator=(T&) noexcept = delete; \
  T& operator=(const T&) noexcept = delete

#define DISABLE_COPY_AND_MOVE(T) \
  DISABLE_COPY(T);                \
  DISABLE_MOVE(T)

namespace lizlib {
// likely/unlikely are likely to clash with other symbols,so do not #define
#if defined(__cplusplus)
constexpr bool likely(bool expr) {
  return __builtin_expect(expr, true);
}
constexpr bool unlikely(bool expr) {
  return __builtin_expect(expr, false);
}
#else
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#endif

#define ifUnlikely(expr) if (unlikely((expr)))
#define ifLikely(expr) if (likely((expr)))

#define EscapableMem(ptr, size)                        \
  std::unique_ptr<char, MallocDeleter> __cleaner##ptr; \
  if (size <= kStackAllocMaximum) { /*on stack*/       \
    ptr = static_cast<decltype(ptr)>(::alloca(size));  \
  } else { /*on heap*/                                 \
    ptr = static_cast<decltype(ptr)>(::malloc(size));  \
    __cleaner##ptr.reset((char*)ptr);                  \
  }

inline char* ceil_page_align_addr(void* ptr) {
  return reinterpret_cast<char*>(((uint64_t)ptr + kPageSize) &
                                 (~(kPageSize - 1)));
}

inline char* floor_page_align_addr(void* ptr) {
  return reinterpret_cast<char*>((uint64_t)ptr & (~(kPageSize - 1)));
}

struct MallocDeleter {
  void operator()(void* ptr) { free(ptr); }
};
struct DummyDeleter {
  void operator()(void* ptr) {}
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
