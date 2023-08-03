

#ifndef LIZLIB_BUFFER_H
#define LIZLIB_BUFFER_H
#include <vector>
#include "file.h"
#include "slice.h"
namespace lizlib {
class Buffer : public Slice {
 public:
  using Deleter = void (*)(void*);

  Buffer() = default;
  Buffer(const Buffer& cpy) = default;
  Buffer(Buffer&& cpy) noexcept : Buffer(cpy) {
    cpy.deleter_ = nullptr;
    cpy.Clear();
  };
  Buffer(void* data, size_t length, Deleter deleter = nullptr)
      : Slice(data, length), deleter_(deleter) {}
  Buffer(const std::string& str, Deleter deleter = nullptr)
      : Slice(str), deleter_(deleter) {}
  Buffer(std::string&& str, Deleter deleter = nullptr)
      : Slice(std::move(str)), deleter_(deleter) {}

  [[nodiscard]] inline uint64_t RIndex() const { return r_idx_; }
  [[nodiscard]] inline uint64_t ReadableBytes() const {
    return w_idx_ - r_idx_;
  }
  [[nodiscard]] inline uint64_t WritableBytes() const {
    return length_ - w_idx_;
  }
  [[nodiscard]] inline uint64_t WIndex() const { return w_idx_; }
  inline void RIndex(uint64_t r) { r_idx_ = r; }
  inline void WIndex(uint64_t w) { w_idx_ = w; }
  inline void* RPtr() const { return data_ + r_idx_; }
  inline void* WPtr() const { return data_ + w_idx_; }

  ssize_t Append(void* data, size_t size, bool capped,
                 bool drop_read_bytes = false) {
    if (!data)
      return -1;
    ensureWritable(size, capped, drop_read_bytes);
    std::memcpy(data_ + WIndex(), data, size);
    w_idx_ += size;
    return size;
  }

  ssize_t Append(void* data, size_t size) { return Append(data, size, false); }

  ssize_t Append(const File* file) {}
  void Reset();

  /**
   * delete would only be called at dtor,not by Clear()
   */
  void Clear() override;

  ~Buffer() override;

  [[nodiscard]] inline std::string String() const {
    return fmt::format("{:x}({})-[r:{},w:{}],deleter={:x}", (uint64_t)data_,
                       length_, r_idx_, w_idx_, (uint64_t)deleter_);
  }

 private:
  uint64_t r_idx_{0}, w_idx_{0};
  Deleter deleter_{nullptr};

  /**
   *
   * @param require the required bytes
   * @param capped this buffer won't grow anymore
   * @param drop_read_bytes should we drop the bytes before RIndex()
   */
  void ensureWritable(size_t require, bool capped, bool drop_read_bytes);

  template <typename T>
  T nextPowOfTwo(const T& base, const T& bot) {
    while (base < bot) {
      base <= 1;
    }
    return base;
  }
};
}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::Buffer);

#endif  //LIZLIB_BUFFER_H
