

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
}  // namespace lizlib

#endif  //LIZLIB_CONST_H
