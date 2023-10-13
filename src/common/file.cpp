

#include "common/file.h"
lizlib::Status lizlib::File::Close() noexcept {
  if (Valid()) {
    LOG_TRACE("File(fd:{})::Close()", fd_);
    auto rst = Status::FromRet(::close(fd_));
    if (rst.OK()) {
      fd_ = -1;
    }
    return rst;
  }
  return Status::Invalid();
}
ssize_t lizlib::File::Readv(const lizlib::Slice* bufs, size_t n) const {
  struct iovec* blocks;
  LIZ_ESCAPABLE_MEM(blocks, sizeof(struct iovec) * n);
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = bufs[i].Data();
    blocks[i].iov_len = bufs[i].Length();
  }
  return ::readv(fd_, blocks, n);
}
ssize_t lizlib::File::Preadv(const lizlib::Slice* bufs, size_t n, uint64_t offset,
                             int flags) const {
  struct iovec* blocks;
  LIZ_ESCAPABLE_MEM(blocks, sizeof(struct iovec) * n);
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = bufs[i].Data();
    blocks[i].iov_len = bufs[i].Length();
  }
  return ::preadv64v2(fd_, blocks, n, offset, flags);
}

ssize_t lizlib::File::Writev(lizlib::Slice* buf, size_t n) {
  struct iovec* blocks;
  LIZ_ESCAPABLE_MEM(blocks, sizeof(struct iovec) * n);
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = buf[i].Data();
    blocks[i].iov_len = buf[i].Length();
  }
  return ::writev(fd_, blocks, n);
}
ssize_t lizlib::File::Seek(uint64_t offset, lizlib::File::SeekMode mode) const {
  return static_cast<ssize_t>(::lseek64(fd_, offset, mode));
}
ssize_t lizlib::File::Size() const {
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
ssize_t lizlib::File::Pread(void* buf, size_t n, uint64_t offset) const {
  return ::pread64(fd_, buf, n, offset);
}
ssize_t lizlib::File::Pwrite(const void* buf, size_t n, uint64_t offset) {
  return ::pwrite64(fd_, buf, n, offset);
}
ssize_t lizlib::File::Pwritev(lizlib::Slice* bufs, size_t n, uint64_t offset, int flags) {
  struct iovec* blocks;
  LIZ_ESCAPABLE_MEM(blocks, sizeof(struct iovec) * n);
  for (int i = 0; i < n; ++i) {
    // const cast removes qualifier
    blocks[i].iov_base = bufs[i].Data();
    blocks[i].iov_len = bufs[i].Length();
  }
  return ::pwritev64v2(fd_, blocks, n, offset, flags);
}
ssize_t lizlib::File::Write(const void* buf, size_t size) noexcept {
  return ::write(fd_, buf, size);
}
lizlib::Status lizlib::File::Remove(const char* name) {
  return Status::FromRet(::remove(name));
}
ssize_t lizlib::File::Truncate(ssize_t new_size) {
  return ::ftruncate64(fd_, new_size);
}
lizlib::File lizlib::File::Open(const char* name, int32_t flags, int32_t createOp) {
  auto file = File();
  file.flags_ = flags;
  if ((flags & O_CREAT) != 0) {
    file.fd_ = open(name, file.flags_, createOp);
  } else {
    file.fd_ = ::open(name, file.flags_);
  }
  return file;
}
