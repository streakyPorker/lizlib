

#include "buffer.h"
void lizlib::Buffer::Clear() {
  Slice::Clear();
  Reset();
}
void lizlib::Buffer::Reset() {
  r_idx_ = w_idx_ = 0;
}
lizlib::Buffer::~Buffer() {
  if (deleter_ && data_) {
    deleter_(static_cast<char*>(data_));
  }
}
void lizlib::Buffer::ensureWritable(size_t require, bool capped,
                                    bool drop_read_bytes) {
  size_t w = WritableBytes();
  if (require <= w) {
    return;
  }
  size_t used = ReadableBytes();
  if (drop_read_bytes && (length_ - used >= require)) {
    std::memmove(data_, data_ + RIndex(), used);
    RIndex(0);
    WIndex(used);
    return;
  }
  length_ = capped ? used + require : nextPowOfTwo(used + require);
  data_ = static_cast<char*>(::realloc(data_, length_));
}
