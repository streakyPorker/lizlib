

#include "common/buffer.h"
#include <cassert>
void lizlib::Buffer::Clear() {
  Slice::Clear();
  Reset();
}
void lizlib::Buffer::Reset() {
  r_idx_ = w_idx_ = 0;
}
lizlib::Buffer::~Buffer() {

  if (deleter_ && data_) {
    LOG_TRACE("buf data({}byte) freed", length_)
    deleter_(static_cast<char*>(data_));
  }
}
bool lizlib::Buffer::ensureWritable(size_t require, bool capped, bool drop_read_bytes) {
  size_t w = WritableBytes();
  if (require == 0 || require <= w) {
    return true;
  }
  size_t used = ReadableBytes();

  if (drop_read_bytes && (length_ - used >= require)) {
    Rearrange();
    return true;
  }

  auto new_length = capped ? used + require : nextPowOfTwo(length_, used + require);
  char* new_data;
  if (aligned_) {
    new_data = static_cast<char*>(::aligned_alloc(kPageSize, new_length));
    assert(deleter_);
    if (new_data && data_) {
      ::memmove(new_data, data_, length_);
      deleter_(data_);
    }
  } else {
    new_data = static_cast<char*>(::realloc(data_, new_length));
  }
  ifUnlikely(!new_data) {
    LOG_ERROR("{}", "expand buffer size failed (OOM)");
    return false;
  }
  length_ = new_length;
  data_ = new_data;
  return true;
}

ssize_t lizlib::Buffer::Append(const void* data, size_t size, bool capped, bool drop_read_bytes) {
  ifUnlikely(!ensureWritable(size, capped, drop_read_bytes)) {
    return -1;
  }
  std::memcpy(data_ + WIndex(), data, size);
  w_idx_ += size;
  return size;
}
ssize_t lizlib::Buffer::Append(const File* file, bool capped, bool drop_read_bytes) {
  ssize_t file_size = file->Size();
  ssize_t ret = -1;
  ifUnlikely(!ensureWritable(file_size, capped, drop_read_bytes)) {
    return -1;
  }
  char* next_page = ceil_page_align_addr(WPtr());
  ssize_t page_remain = next_page - WPtr();
  // fits the remain page
  if (file_size <= page_remain) {
    ret = file->Read(WPtr(), file_size);
    if (ret >= 0) {
      w_idx_ += ret;
    }
    return ret;
  }
  ssize_t left = file_size - page_remain;
  ssize_t splits = (left / kFileRWUnit) + 1;
  ssize_t mod = left % kFileRWUnit;
  Slice slices[splits + 1];
  slices[0].Data(WPtr());
  slices[0].Length(page_remain);
  for (int i = 0; i < splits; ++i) {
    slices[i + 1].Data(next_page + i * kFileRWUnit);
    if (i == splits - 1) {
      slices[i + 1].Length(mod);
    } else {
      slices[i + 1].Length(kFileRWUnit);
    }
  }
  ret = file->Readv(slices, splits + 1);
  ifLikely(ret > 0) {
    w_idx_ += ret;
  }
  return ret;
}
ssize_t lizlib::Buffer::Read(void* data, ssize_t size, bool rearrange) {
  ifUnlikely(!data_) {
    return -1;
  }

  ssize_t read_bytes = std::min(size, (ssize_t)ReadableBytes());
  memcpy(data, RPtr(), read_bytes);
  r_idx_ += read_bytes;
  if (r_idx_ == w_idx_ && rearrange) {
    Rearrange();
  }
  return read_bytes;
}
ssize_t lizlib::Buffer::Read(lizlib::File* file, ssize_t size) {
  ifUnlikely(!data_) {
    return -1;
  }
  ssize_t ret = 0;
  if (size == -1) {
    size = ReadableBytes();
  } else {
    size = std::min(size, (ssize_t)ReadableBytes());
  }

  char* next_page = ceil_page_align_addr(RPtr());
  ssize_t page_remain = next_page - WPtr();
  if (size <= page_remain) {
    ret = file->Write(RPtr(), size);
    ifLikely(ret > 0) {
      r_idx_ += ret;
    }
    return ret;
  }
  ssize_t left = size - page_remain;
  ssize_t splits = (left / kFileRWUnit) + 1;
  ssize_t mod = left % kFileRWUnit;
  Slice* slices;
  LIZ_ESCAPABLE_MEM(slices, sizeof(slices) * (splits + 1));
  for (int i = 0; i < splits; ++i) {
    slices[i + 1].Data(next_page + i * kFileRWUnit);
    if (i == splits - 1) {
      slices[i + 1].Length(mod);
    } else {
      slices[i + 1].Length(kFileRWUnit);
    }
  }
  ret = file->Writev(slices, splits + 1);
  ifLikely(ret > 0) {
    r_idx_ += ret;
  }
  return ret;
}

ssize_t lizlib::Buffer::Append(lizlib::Slice& slice, bool capped, bool drop_read_bytes) {
  ssize_t size = slice.Length();
  ifUnlikely(!ensureWritable(size, capped, drop_read_bytes)) {
    return -1;
  }
  ssize_t ret = Append(slice.Data(), size, capped, drop_read_bytes);
  ifLikely(ret > 0) {
    w_idx_ += ret;
  }
  return ret;
}
void lizlib::Buffer::Rearrange() {
  size_t used = ReadableBytes();
  if (used != 0) {
    std::memmove(data_, data_ + r_idx_, used);
  }
  r_idx_ = 0;
  w_idx_ = used;
}
ssize_t lizlib::Buffer::Transfer(lizlib::Buffer& from, bool capped, bool drop_read_bytes) {
  ssize_t size = from.ReadableBytes();
  ifUnlikely(!ensureWritable(size, capped, drop_read_bytes)) {
    return -1;
  }
  ssize_t ret = Append(from.RPtr(), size, capped, drop_read_bytes);
  ifLikely(ret > 0) {
    from.r_idx_ += ret;
  }
  return ret;
}
void lizlib::Buffer::Retrieve(size_t n, bool rearrange) {
  r_idx_ = std::min(r_idx_ + n, w_idx_);
  if (r_idx_ == w_idx_ && rearrange) {
    Rearrange();
  }
}
