

#include "file.h"
lizlib::Status lizlib::File::Close() noexcept {

  if (Valid()) {
    LOG_TRACE("{})(fd:{})::Close()", *this, fd_);
    auto rst = Status::FromRet(::close(fd_));
    if (rst.OK()) {
      fd_ = -1;
    }
    return rst;
  }
  return Status::Invalid();
}
int64_t lizlib::File::Readv(const lizlib::Slice* bufs, size_t n) const {
  struct iovec* blocks;
  std::unique_ptr<struct iovec, MallocDeleter> cleaner;

  if (n * sizeof(struct iovec) <= kStackAllocMaximum) {
    blocks = static_cast<iovec*>(alloca(sizeof(struct iovec) * n));
  } else {
    // use malloc to enable customized allocator like jemalloc
    blocks = static_cast<iovec*>(malloc(sizeof(struct iovec) * n));
    cleaner.reset(blocks);
  }
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = bufs[i].data_;
    blocks[i].iov_len = bufs[i].length_;
  }
  return ::readv(fd_, blocks, n);
}
int64_t lizlib::File::Preadv(const lizlib::Slice* bufs, size_t n,
                             uint64_t offset, int flags) const {
  struct iovec* blocks;
  std::unique_ptr<struct iovec, MallocDeleter> cleaner;

  if (n * sizeof(struct iovec) <= kStackAllocMaximum) {
    blocks = static_cast<iovec*>(alloca(sizeof(struct iovec) * n));
  } else {
    // use malloc to enable customized allocator like jemalloc
    blocks = static_cast<iovec*>(malloc(sizeof(struct iovec) * n));
    cleaner.reset(blocks);
  }
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = bufs[i].data_;
    blocks[i].iov_len = bufs[i].length_;
  }
  ::preadv64v2(fd_, blocks, n, offset, flags);
}

int64_t lizlib::File::Writev(lizlib::Slice* buf, size_t n) {
  struct iovec* blocks;
  std::unique_ptr<struct iovec, MallocDeleter> cleaner;

  if (n * sizeof(struct iovec) <= kStackAllocMaximum) {
    blocks = static_cast<iovec*>(alloca(sizeof(struct iovec) * n));
  } else {
    // use malloc to enable customized allocator like jemalloc
    blocks = static_cast<iovec*>(malloc(sizeof(struct iovec) * n));
    cleaner.reset(blocks);
  }
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = buf[i].data_;
    blocks[i].iov_len = buf[i].length_;
  }
  return ::writev(fd_, blocks, n);
}
int64_t lizlib::File::Seek(uint64_t offset, lizlib::File::SeekMode mode) const {
  return static_cast<int64_t>(::lseek64(fd_, offset, mode));
}
int64_t lizlib::File::Size() const {
  auto cur = Seek(0, SEEK_CUR);  //  check
  if (cur < 0) {
    return cur;
  }
  auto size = Seek(0, SEEK_END);
  // need to seek back to origin pos
  if (size < 0 || Seek(cur, SEEK_SET) < 0) {
    return -1;
  }
  return size;
}
lizlib::Status lizlib::File::Sync() const noexcept {
  return Status::FromRet(::syncfs(fd_));
}
int64_t lizlib::File::Pread(void* buf, size_t n, uint64_t offset) const {
  return ::pread64(fd_, buf, n, offset);
}
int64_t lizlib::File::Pwrite(const void* buf, size_t n, uint64_t offset) {
  return ::pwrite64(fd_, buf, n, offset);
}
int64_t lizlib::File::Pwritev(lizlib::Slice* bufs, size_t n, uint64_t offset,
                              int flags) {
  struct iovec* blocks;
  std::unique_ptr<struct iovec, MallocDeleter> cleaner;

  if (n * sizeof(struct iovec) <= kStackAllocMaximum) {
    blocks = static_cast<iovec*>(alloca(sizeof(struct iovec) * n));
  } else {
    // use malloc to enable customized allocator like jemalloc
    blocks = static_cast<iovec*>(malloc(sizeof(struct iovec) * n));
    cleaner.reset(blocks);
  }
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = bufs[i].data_;
    blocks[i].iov_len = bufs[i].length_;
  }
  ::pwritev64v2(fd_, blocks, n, offset, flags);
}
size_t lizlib::File::Write(const void* buf, size_t size) noexcept {
  return ::write(fd_, buf, size);
}
lizlib::Status lizlib::File::Remove(const char* name) {
  return Status::FromRet(::remove(name));
}
