

#ifndef LIZLIB_LATCH_H
#define LIZLIB_LATCH_H
#include "common/basic.h"
namespace lizlib {
class Latch {
 public:
  void wait() {
    std::unique_lock<std::mutex> lock(mu_);
    cv_.wait(lock);
  }
  void notify_one() { cv_.notify_one(); }
  void notify_all() { cv_.notify_all(); }

 private:
  std::mutex mu_;
  std::condition_variable cv_;
};
}  // namespace lizlib

#endif  //LIZLIB_LATCH_H
