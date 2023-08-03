#ifndef _SLICE_H_
#define _SLICE_H_
#include "basic.h"

namespace lizlib {
class Slice {
 public:
  Slice() = default;
  Slice(void* data, size_t length)
      : data_(static_cast<char*>(data)), length_(length) {}

  Slice(const Slice& cpy) = default;

  Slice(const std::string& str)
      : data_(const_cast<char*>(str.data())), length_(str.size()) {}
  Slice(std::string&& str)
      : data_(const_cast<char*>(str.data())), length_(str.size()) {
    str.assign(nullptr);
    str.resize(0);
  }

  Slice(Slice&& mv) noexcept : data_(mv.data_), length_(mv.length_) {
    mv.Clear();
  }

  [[nodiscard]] inline void* Data() const { return data_; }
  [[nodiscard]] inline size_t Length() const { return length_; }

  virtual ~Slice() = default;

  virtual void Clear() {
    data_ = nullptr;
    length_ = 0;
  }

 protected:
  size_t length_{0};
  char* data_{nullptr};
};

}  // namespace lizlib

#endif  // _SLICE_H_