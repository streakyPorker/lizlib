//
// Created by bangsun on 2023/9/18.
//

#ifndef LIZLIB_TRANSACTION_UTIL_H
#define LIZLIB_TRANSACTION_UTIL_H
#include "common/basic.h"
#include "common/logger.h"
namespace lizlib {
class TransactionUtil {
 public:
  static bool CAS64() {
#if defined(__GNUC__)
    long* ptr = new long(-1);  // 指针指向的内存区域
    long oldValue = -1;        // 旧值
    long newValue = -2;        // 新值
    bool success = __atomic_compare_exchange(ptr, &oldValue, &newValue, false, __ATOMIC_SEQ_CST,
                                             __ATOMIC_SEQ_CST);
    LOG_INFO("success: {} , after val : {}", success, *((unsigned long*)ptr));
    return true;
#else
    assert(0);
#endif
  }
};
}  // namespace lizlib

#endif  //LIZLIB_TRANSACTION_UTIL_H
