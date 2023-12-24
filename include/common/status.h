

#ifndef LIZLIB_STATUS_H
#define LIZLIB_STATUS_H
#include <utility>

#include "basic.h"
namespace lizlib {
/**
 *
 */
class Status {
 public:
  Status() = delete;
  Status(const Status&) = default;
  explicit Status(int code, std::string reason = "") : code_(code), reason_(std::move(reason)) {}

  inline static Status Success() { return Status{0}; }
  /**
   * give errno (if there is any) to code
   * @param ret
   * @return
   */
  inline static Status FromRet(int ret) {
    if (ret == 0) {
      return Status::Success();
    } else {
      return FromErr();
    }
  }

  inline static Status FromErr() { return Status{errno, getReason(errno)}; }

  inline static Status Invalid(const std::string& desc = "") { return Status{INT32_MIN, desc}; }

  [[nodiscard]] inline bool OK() const { return code_ == 0; }
  [[nodiscard]] inline int32_t Code() const { return code_; }
  [[nodiscard]] std::string String() const;

 private:
  [[nodiscard]] static std::string getReason(int code) noexcept;
  int32_t code_{INT32_MIN};
  std::string reason_{};
};
}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::Status);

#endif  //LIZLIB_STATUS_H
