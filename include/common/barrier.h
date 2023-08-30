

#ifndef LIZLIB_BARRIER_H
#define LIZLIB_BARRIER_H
#include <atomic>
#include "common/basic.h"
#include "latch.h"
namespace lizlib {
class BlockableBarrier {
 public:
  explicit BlockableBarrier(size_t waits, Latch* blocker = nullptr)
      : total_(waits), waiting_(0), blocker_(blocker) {}

  void await() {
    std::unique_lock<std::mutex> lock(mu_);
    waiting_++;
    if (waiting_ < total_) {
      cv_.wait(lock);
    } else {
      if (blocker_ != nullptr) {
        blocker_->wait();
      }
      waiting_ = 0;
      cv_.notify_all();
    }
  }

 private:
  size_t total_;
  size_t waiting_;
  std::mutex mu_;
  std::condition_variable cv_;
  Latch* blocker_{nullptr};
};
}  // namespace lizlib

#endif  //LIZLIB_BARRIER_H
