#ifndef _SLICE_H_
#define _SLICE_H_
#include "basic.h"

namespace lizlib {
class Slice {
 public:
  Slice() = default;

  Slice(void* data, size_t length)
      : data_(static_cast<char*>(data)), length_(length) {}

  Slice(const void* data, size_t length)
      : data_((char*)data), length_(length) {}

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

  [[nodiscard]] inline char* Data() const { return data_; }

  inline void Data(void* data) { data_ = static_cast<char*>(data); }

  [[nodiscard]] size_t Length() const { return length_; }

  [[nodiscard]] size_t* LengthPtr() { return &length_; }

  inline void Length(ssize_t length) { length_ = length; }

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