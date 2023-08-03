

#ifndef LIZLIB_FILE_H
#define LIZLIB_FILE_H
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fstream>
#include "basic.h"
#include "logger.h"
#include "slice.h"
#include "status.h"

namespace lizlib {
class File {
 public:
  using SeekMode = int;

  File(File&& file) noexcept : fd_(file.fd_), flags_(file.flags_) {
    file.fd_ = -1;
    file.flags_ = 0;
  }

  static File Open(const char* name, int32_t flags, int32_t createOp = 0) {
    auto file = File();
    file.flags_ = flags;
    if ((flags & O_CREAT) != 0) {
      file.fd_ = open(name, file.flags_, createOp);
    } else {
      file.fd_ = ::open(name, file.flags_);
    }
    return file;
  }

  static Status Remove(const char* name);

  static File From(int fd) { return File(fd); }

  [[nodiscard]] inline bool Valid() const { return fd_ == -1; }
  [[nodiscard]] inline bool Direct() const { return (flags_ & O_DIRECT) == 0; }

  Status Close() noexcept;

  inline size_t Read(void* buf, size_t size) const noexcept {
    return ::read(fd_, buf, size);
  }

  int64_t Readv(const Slice* buf, size_t n) const;

  int64_t Pread(void* buf, size_t n, uint64_t offset) const;

  int64_t Preadv(const Slice* bufs, size_t n, uint64_t offset,
                 int flags = 0) const;

  inline size_t Write(const void* buf, size_t size) noexcept;

  int64_t Writev(Slice* buf, size_t n);

  int64_t Pwrite(const void* buf, size_t n, uint64_t offset);

  int64_t Pwritev(Slice* bufs, size_t n, uint64_t offset, int flags = 0);

  Status Sync() const noexcept;

  int64_t Seek(uint64_t offset, SeekMode mode) const;

  [[nodiscard]] int64_t Size() const;

  inline std::string String() const { return fmt::format("{}", fd_); }

 protected:
  int fd_{-1};
  int flags_{0};

 private:
  File() = default;
  explicit File(int fd) : fd_(fd){};
};
}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::File);

#endif  //LIZLIB_FILE_H
