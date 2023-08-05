

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

  static Status Success() { return Status{0}; }
  /**
   * give errno (if there is any) to code
   * @param ret
   * @return
   */
  static Status FromRet(int ret) {
    if (ret == 0) {
      return Status::Success();
    } else {
      return Status{errno};
    }
  }
  static Status Invalid() { return Status{INT32_MIN}; }

  inline bool OK() const { return code_ == 0; }
  inline int32_t Code() const { return code_; }

 private:
  int32_t code_{INT32_MIN};
};
}  // namespace lizlib

#endif  //LIZLIB_STATUS_H
