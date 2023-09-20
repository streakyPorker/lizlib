

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
  explicit Buffer(size_t length, bool aligned = true)
      : Buffer(aligned ? ::aligned_alloc(kPageSize, length) : malloc(length), length, free) {
    aligned_ = aligned;
  }
  Buffer(const Buffer& cpy) = default;
  Buffer(Buffer&& cpy) noexcept : Buffer(cpy) {
    cpy.deleter_ = nullptr;
    cpy.Clear();
  };

  Buffer(void* data, size_t length, Deleter deleter = nullptr)
      : Slice(data, length), deleter_(deleter) {}
  Buffer(const std::string& str, Deleter deleter = nullptr) : Slice(str), deleter_(deleter) {}
  Buffer(std::string&& str, Deleter deleter = nullptr) : Slice(std::move(str)), deleter_(deleter) {}

  [[nodiscard]] inline uint64_t RIndex() const { return r_idx_; }
  [[nodiscard]] inline ssize_t ReadableBytes() const { return w_idx_ - r_idx_; }
  [[nodiscard]] inline ssize_t WritableBytes() const { return length_ - w_idx_; }
  [[nodiscard]] inline uint64_t WIndex() const { return w_idx_; }
  [[nodiscard]] inline char* RPtr() const { return data_ + r_idx_; }
  [[nodiscard]] inline char* WPtr() const { return data_ + w_idx_; }

  ssize_t Append(const void* data, size_t size, bool capped, bool drop_read_bytes = false);

  ssize_t Append(const void* data, size_t size) { return Append(data, size, false); }

  template <typename Basic>
  ssize_t AppendBasic(const Basic& v, bool capped = false, bool drop_read_bytes = false) {
    ssize_t size = sizeof(Basic);
    ifUnlikely(!ensureWritable(size, capped, drop_read_bytes)) {
      return -1;
    }
    *(Basic*)WPtr() = v;
    w_idx_ += size;
    return size;
  }

  ssize_t Append(const File* file, bool capped = false, bool drop_read_bytes = false);

  ssize_t Append(Slice& slice, bool capped = false, bool drop_read_bytes = false);

  ssize_t Append(Buffer& buffer, bool capped = false, bool drop_read_bytes = false) {
    ssize_t size = buffer.ReadableBytes();
    ifUnlikely(!ensureWritable(size, capped, drop_read_bytes)) {
      return -1;
    }
    ssize_t ret = Append(buffer.RPtr(), size, capped, drop_read_bytes);
    return ret;
  }

  ssize_t Transfer(Buffer& from, bool capped = false, bool drop_read_bytes = false);

  template <typename Basic>
  [[maybe_unused]] Basic ReadBasic() {
    ifUnlikely(!data_) {
      return -1;
    }
    ssize_t size = sizeof(Basic);
    if (ReadableBytes() < size) {
      return -1;
    }
    Basic rst = *(Basic*)RPtr();
    r_idx_ += size;
    return rst;
  }

  ssize_t Read(void* data, ssize_t size, bool rearrange = true);

  ssize_t Read(File* file, ssize_t size = -1);

  /**
   * tell the buffer n bytes have been read
   * @param n
   */
  void Retrieve(size_t n, bool rearrange = true);

  void Reset();

  void Rearrange();

  /**
   * delete would only be called at dtor,not by Clear()
   */
  void Clear() override;

  ~Buffer() override;

  [[nodiscard]] inline std::string String() const {
    return fmt::format("{:x}({})-[r:{},w:{}],deleter={:x}", (uint64_t)data_, length_, r_idx_,
                       w_idx_, (uint64_t)deleter_);
  }

 private:
  uint64_t r_idx_{0}, w_idx_{0};
  Deleter deleter_{nullptr};
  bool aligned_{false};

  /**
   * ensure that the next write of `require` bytes could success
   * <br>
   * the buffer could be empty (null,0)
   * @param require the required bytes
   * @param capped this buffer won't grow anymore
   * @param drop_read_bytes should we drop the bytes before RIndex()
   */
  bool ensureWritable(size_t require, bool capped, bool drop_read_bytes);

  template <typename T>
  T nextPowOfTwo(T base, const T& target) {
    if (base == 0) {
      base = 2;
    }
    while (base < target) {
      base <<= 1;
    }
    return base;
  }
};
}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::Buffer);

#endif  //LIZLIB_BUFFER_H
