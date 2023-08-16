

#ifndef LIZLIB_CONST_H
#define LIZLIB_CONST_H

#include <unistd.h>
#include <cstdint>
namespace lizlib {
const uint64_t kFileRWUnit = 65536L;
const uint64_t kBufInitSize = ::sysconf(_SC_PAGESIZE);
const uint64_t kPageSize = ::sysconf(_SC_PAGESIZE);
const int64_t kUsecPerSec = 1000000L;
const int64_t kUsecPerMsec = 1000000L;
const size_t kStackAllocMaximum = 65536L;

struct NetOption {
  const static bool reuse_addr = true;
  const static bool reuse_port = true;
  const static bool keep_alive = true;
  const static bool tcp_no_delay = true;
};

}  // namespace lizlib

#endif  //LIZLIB_CONST_H
