

#ifndef LIZLIB_CONFIG_H
#define LIZLIB_CONFIG_H

#include <unistd.h>
#include <cstdint>
namespace lizlib {
const static int64_t kUsecPerSec = 1000000L;
const static int64_t kUsecPerMsec = 1000000L;

static uint64_t kFileRWUnit = 65536L;
static uint64_t kBufInitSize = ::sysconf(_SC_PAGESIZE);
static uint64_t kPageSize = ::sysconf(_SC_PAGESIZE);
static size_t kStackAllocMaximum = 65536L;

}  // namespace lizlib

#endif  //LIZLIB_CONFIG_H
