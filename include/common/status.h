

#ifndef LIZLIB_STATUS_H
#define LIZLIB_STATUS_H
#include "basic.h"
namespace lizlib {
/**
 *
 */
class Status {
 public:
  Status() = delete;
  Status(const Status&) = default;
  explicit Status(int code) : code_(code) {}

  inline static Status Success() { return Status{0}; }
  /**
   * give errno (if there is any) to code
   * @param ret
   * @return
   */
  static Status FromRet(int ret) {
    if (ret == 0) {
      return Status::Success();
    } else {
      return FromErr();
    }
  }

  inline static Status FromErr() { return Status{errno}; }

  inline static Status Invalid() { return Status{INT32_MIN}; }

  inline bool OK() const { return code_ == 0; }
  inline int32_t Code() const { return code_; }
  [[nodiscard]] std::string String() const {
    return fmt::format("Error[code:{}, reason:{}]", code_, getReason());
  }

 private:
  [[nodiscard]] const char* getReason() const noexcept;
  int32_t code_{INT32_MIN};
};
}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::Status);

#endif  //LIZLIB_STATUS_H
