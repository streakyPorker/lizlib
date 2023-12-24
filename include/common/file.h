

#ifndef LIZLIB_FILE_H
#define LIZLIB_FILE_H
#include <fcntl.h>
#include <sys/poll.h>
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
  inline ssize_t PollRead(void* buf, size_t size, const Duration duration) const noexcept {
    struct pollfd poll_fd;
    poll_fd.fd = fd_;
    poll_fd.events = POLLIN;
    int rst = ::poll(&poll_fd, 1, duration.MilliSec());
    if (rst < 0) {
      LOG_FATAL("file poll failed : {}", Status::FromErr());
    } else if (rst == 0) {
      return -1;
    }
    return ::read(fd_, buf, size);
  }

  ssize_t Readv(const Slice* buf, size_t n) const;

  ssize_t Pread(void* buf, size_t n, uint64_t offset) const;

  ssize_t Preadv(const Slice* bufs, size_t n, uint64_t offset, int flags = 0) const;

  ssize_t Write(const void* buf, size_t size) noexcept;

  ssize_t Writev(Slice* buf, size_t n);

  ssize_t Pwrite(const void* buf, size_t n, uint64_t offset);

  ssize_t Pwritev(Slice* bufs, size_t n, uint64_t offset, int flags = 0);

  /**
   * 三种sync方式的区别：
   *<br>● fsync 用于将指定文件描述符对应的文件的所有数据和元数据刷写到磁盘上。
   * 它会确保文件系统元数据和文件数据的一致性，包括文件内容和文件的元信息（如修改时间、权限等）。
   * 调用 fsync 时，可能会导致文件系统中与该文件相关的所有数据和元数据都被刷新到磁盘。
    <br> ● fdatasync 与 fsync 类似，但它只刷新文件的数据部分，
    而不刷新元数据。如果你只关心文件内容的持久性，而不需要同步元数据，
    可以使用 fdatasync。fdatasync 的性能通常比 fsync 更好，因为它不需要刷新文件的所有元数据。
     <br>● syncfs 用于将指定文件描述符对应的文件所在的文件系统中关于该文件的所有数据和元数据刷写到磁盘上。
     与 fsync 不同，syncfs 是针对整个文件系统的，而不是特定的文件。调用 syncfs 时，
     可能会导致文件系统中与该文件系统相关的所有数据和元数据都被刷新到磁盘。
   * @return
   */
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
