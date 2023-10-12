

#ifndef LIZLIB_BASIC_H
#define LIZLIB_BASIC_H
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <sys/time.h>
#include <any>
#include <cassert>
#include <cstdint>
#include <cstdlib>
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
#include "config.h"

#define LIZ_FORMATTER_REGISTRY(T)                                                       \
  template <>                                                                           \
  struct fmt::formatter<T> {                                                            \
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator { \
      return ctx.end();                                                                 \
    }                                                                                   \
    auto format(const T& item, format_context& ctx) const {                             \
      return fmt::format_to(ctx.out(), "{}", item.String());                            \
    };                                                                                  \
  }

#define LIZ_DISABLE_MOVE(T) \
  T(T&&) noexcept = delete; \
  T& operator=(T&&) noexcept = delete

#define LIZ_DISABLE_COPY(T)           \
  T(const T&) noexcept = delete;      \
  T(T&) noexcept = delete;            \
  T& operator=(T&) noexcept = delete; \
  T& operator=(const T&) noexcept = delete

#define LIZ_DISABLE_COPY_AND_MOVE(T) \
  LIZ_DISABLE_COPY(T);               \
  LIZ_DISABLE_MOVE(T)

#define ValidFd(fd) ((fd) >= 0)

#if defined(__cplusplus)
#define ifUnlikely(expr) if (__builtin_expect((expr), false))
#define ifLikely(expr) if (__builtin_expect((expr), true))
#else
#define ifUnlikely(expr) if (__builtin_expect((expr), 0))
#define ifLikely(expr) if (__builtin_expect((expr), 1))
#endif

#define NON_EXPLICIT

#define LIZ_PAUSE() asm volatile("pause")
#define LIZ_BARRIER() asm volatile("" ::: "memory")
#define LIZ_LFENSE() asm volatile("lfence" ::: "memory")
#define LIZ_SFENSE() asm volatile("sfence" ::: "memory")
#define LIZ_MFENSE() asm volatile("mfence" ::: "memory")

#define LIZ_ESCAPABLE_MEM(ptr, size)                   \
  std::unique_ptr<char, MallocDeleter> __cleaner##ptr; \
  if (size <= kStackAllocMaximum) { /*on stack*/       \
    ptr = static_cast<decltype(ptr)>(::alloca(size));  \
  } else { /*on heap*/                                 \
    ptr = static_cast<decltype(ptr)>(::malloc(size));  \
    __cleaner##ptr.reset((char*)ptr);                  \
  }

#define LIZ_CLAIM_SHARED_PTR(type) using Ptr = std::shared_ptr<type>
#define LIZ_CLAIM_UNIQUE_PTR(type) using UniPtr = std::unique_ptr<type>

namespace lizlib {
inline char* ceil_page_align_addr(void* ptr) {
  return reinterpret_cast<char*>(((uint64_t)ptr + kPageSize) & (~(kPageSize - 1)));
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
  inline friend bool operator<(const T& p, const T& q) noexcept { return T::Compare(p, q) < 0; }
  friend bool operator<=(const T& p, const T& q) noexcept { return T::Compare(p, q) <= 0; }
  friend bool operator==(const T& p, const T& q) noexcept { return T::Compare(p, q) == 0; }
  friend bool operator>=(const T& p, const T& q) noexcept { return T::Compare(p, q) >= 0; }
  friend bool operator>(const T& p, const T& q) noexcept { return T::Compare(p, q) > 0; }
  friend bool operator!=(const T& p, const T& q) noexcept { return T::Compare(p, q) != 0; }
};

struct Duration : Comparable<Duration> {
  int64_t usec_{-1};
  static Duration Invalid() { return Duration{}; }

  template <typename Rep, typename Period>
  Duration(std::chrono::duration<Rep, Period> other)
      : usec_(std::chrono::duration_cast<std::chrono::microseconds>(other).count()) {}

  explicit Duration(int64_t usecs_diff) : usec_(usecs_diff) {}

  static int Compare(const Duration& p, const Duration& q) noexcept {
    return p.usec_ == q.usec_ ? 0 : p.usec_ < q.usec_ ? -1 : 1;
  }

  Duration(const Duration& other) = default;
  Duration& operator=(const Duration& other) noexcept {
    usec_ = other.usec_;
    return *this;
  }
  Duration(Duration&& other) noexcept { std::swap(usec_, other.usec_); };
  Duration& operator=(Duration&& other) noexcept {
    std::swap(usec_, other.usec_);
    return *this;
  }

  [[nodiscard]] inline bool Valid() const noexcept { return usec_ >= 0; }

  [[nodiscard]] inline std::chrono::microseconds MicroSec() const noexcept {
    return std::chrono::microseconds(usec_);
  };
  [[nodiscard]] inline int64_t MilliSec() const noexcept { return Valid() ? usec_ / 1000L : -1; };
  [[nodiscard]] inline int64_t Sec() const noexcept { return Valid() ? usec_ / 1000000 : -1L; };

  [[nodiscard]] inline int64_t MicrosBelowMilli() const noexcept {
    return Valid() ? usec_ % 1000L : -1;
  };
  [[nodiscard]] inline int64_t MicrosBelowSec() const noexcept {
    return Valid() ? usec_ % 1000000L : -1;
  };

 private:
  Duration() = default;
};

struct Timestamp : public Comparable<Timestamp> {
  int64_t usecs_{};

  Timestamp() = default;
  explicit Timestamp(int64_t usecs) : usecs_(usecs) {}

  static Timestamp Now() {
    struct timeval tv {};

    ::gettimeofday(&tv, nullptr);
    return Timestamp{tv.tv_usec + tv.tv_sec * kUsecPerSec};
  };

  static Timestamp Max() { return Timestamp{std::numeric_limits<int64_t>::max()}; }

  static Timestamp Min() { return Timestamp{0}; }

  static int Compare(const Timestamp& p, const Timestamp& q) noexcept {
    return p.usecs_ == q.usecs_ ? 0 : p.usecs_ < q.usecs_ ? -1 : 1;
  }

  Duration operator-(const Timestamp& other) const noexcept {

    return Duration{usecs_ - other.usecs_};
  }

  Timestamp operator+(const Duration& d) const noexcept { return Timestamp{usecs_ + d.usec_}; }

  Timestamp operator-(const Duration& d) const noexcept { return Timestamp{usecs_ - d.usec_}; }

  [[nodiscard]] std::string String() const noexcept {
    time_t msecs = usecs_ / kUsecPerMsec;
    time_t tmp_usec = usecs_ % kUsecPerMsec;
    return fmt::format("{:%Y-%m-%d %H:%M:%S}.{:06}", fmt::localtime(msecs), tmp_usec);
  };
};

struct ConcurrentTimestamp : public Comparable<ConcurrentTimestamp> {
  volatile int64_t usecs_{};

  ConcurrentTimestamp() = default;
  explicit ConcurrentTimestamp(int64_t usecs) : usecs_(usecs) {}
  static int Compare(const ConcurrentTimestamp& p, const ConcurrentTimestamp& q) noexcept {
    return p.usecs_ == q.usecs_ ? 0 : p.usecs_ < q.usecs_ ? -1 : 1;
  }

  [[nodiscard]] Timestamp Get() const {
    Timestamp read{usecs_};
    return read;
  }

  Duration operator-(const Timestamp& other) const noexcept {
    return Duration{usecs_ - other.usecs_};
  }
  Duration operator-(const ConcurrentTimestamp& other) const noexcept {
    return Duration{usecs_ - other.usecs_};
  }

  void Incr(const Duration& d) {
    uint64_t prev = usecs_;
    uint64_t modified = prev + d.usec_;
    uint64_t rst;
    while ((rst = __sync_val_compare_and_swap(&usecs_, prev, modified)) != prev) {
      LIZ_PAUSE();
      prev = rst;
      modified = prev + d.usec_;
    }
  }

  void Decr(const Duration& d) {
    uint64_t prev = usecs_;
    uint64_t modified = prev - d.usec_;
    uint64_t rst;
    while ((rst = __sync_val_compare_and_swap(&usecs_, prev, modified)) != prev) {
      LIZ_PAUSE();
      prev = rst;
      modified = prev - d.usec_;
    }
  }

  Timestamp operator+(const Duration& d) const noexcept { return Timestamp{usecs_ + d.usec_}; }

  Timestamp operator-(const Duration& d) const noexcept { return Timestamp{usecs_ - d.usec_}; }

  [[nodiscard]] std::string String() const noexcept {
    time_t msecs = usecs_ / kUsecPerMsec;
    time_t tmp_usec = usecs_ % kUsecPerMsec;
    return fmt::format("{:%Y-%m-%d %H:%M:%S}.{:06}", fmt::localtime(msecs), tmp_usec);
  };
};

}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::Timestamp);
LIZ_FORMATTER_REGISTRY(lizlib::ConcurrentTimestamp);

#endif  //LIZLIB_BASIC_H
