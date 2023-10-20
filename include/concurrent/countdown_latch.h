//
// Created by A on 2023/10/20.
//

#ifndef LIZLIB_COUNTDOWN_LATCH_H
#define LIZLIB_COUNTDOWN_LATCH_H

#include "common/basic.h"
#include "cond_var.h"
namespace lizlib {
class CountdownLatch final {
 public:
  explicit CountdownLatch(int counts) : count_(counts), cv_() {}

  [[nodiscard]] size_t Count() const noexcept { return count_.load(std::memory_order_acquire); }

  void CountDown();

  bool Await(const lizlib::Duration& timeout);
  void Await();

 private:
  std::atomic_size_t count_;
  CondVar cv_;
};
}  // namespace lizlib

#endif  //LIZLIB_COUNTDOWN_LATCH_H
