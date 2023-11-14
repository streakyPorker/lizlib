

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

  LIZ_DISABLE_COPY(File);

  // only move ctor allowed
  File(File&& file) noexcept : fd_(file.fd_), flags_(file.flags_) {
    file.fd_ = -1;
    file.flags_ = 0;
  }
  explicit File(int fd) : fd_(fd){};
  virtual ~File() { Close(); }

  static File Open(const char* name, int32_t flags, int32_t createOp = 0);

  static Status Remove(const char* name);

  static File From(int fd) { return File(fd); }

  [[nodiscard]] inline int Fd() const { return fd_; }
  [[nodiscard]] inline bool Valid() const { return fd_ != -1; }
  [[nodiscard]] inline bool Direct() const { return (flags_ & O_DIRECT) == 0; }

  Status Close() noexcept;

  inline ssize_t Read(void* buf, size_t size) const noexcept { return ::read(fd_, buf, size); }

  ssize_t Readv(const Slice* buf, size_t n) const;

  ssize_t Pread(void* buf, size_t n, uint64_t offset) const;

  ssize_t Preadv(const Slice* bufs, size_t n, uint64_t offset, int flags = 0) const;

  ssize_t Write(const void* buf, size_t size) noexcept;

  ssize_t Writev(Slice* buf, size_t n);

  ssize_t Pwrite(const void* buf, size_t n, uint64_t offset);

  ssize_t Pwritev(Slice* bufs, size_t n, uint64_t offset, int flags = 0);

  Status Sync() const noexcept;

  [[nodiscard]] ssize_t Cur() const { return Seek(0, SEEK_CUR); }

  ssize_t Seek(ssize_t offset, SeekMode mode) const;

  ssize_t Truncate(ssize_t new_size);

  virtual Status GetError() const noexcept { return Status::FromErr(); };

  [[nodiscard]] ssize_t Size() const;

  [[nodiscard]] virtual std::string String() const { return fmt::format("{}", fd_); }

 protected:
  int fd_{-1};
  int flags_{0};
  File() = default;
};
}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::File);

#endif  //LIZLIB_FILE_H
