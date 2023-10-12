//
// Created by A on 2023/10/12.
//

#ifndef LIZLIB_SPIN_LOCK_H
#define LIZLIB_SPIN_LOCK_H
#include <pthread.h>
namespace lizlib {
class SpinLock {
  pthread_spinlock_t actual_{};

 public:
  SpinLock() { pthread_spin_init(&actual_, PTHREAD_PROCESS_PRIVATE); }

  ~SpinLock() { pthread_spin_destroy(&actual_); }

  void lock() noexcept { pthread_spin_lock(&actual_); }

  bool try_lock() noexcept { return pthread_spin_trylock(&actual_); }

  void unlock() noexcept { pthread_spin_unlock(&actual_); }
};

}  // namespace lizlib
#endif  //LIZLIB_SPIN_LOCK_H
