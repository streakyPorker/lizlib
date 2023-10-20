

#ifndef LIZLIB_BLOCKABLE_BARRIER_H
#define LIZLIB_BLOCKABLE_BARRIER_H
#include <atomic>
#include "common/basic.h"
#include "cond_var.h"
namespace lizlib {
class BlockableBarrier {
 public:
  explicit BlockableBarrier(size_t waits, CondVar* blocker = nullptr)
      : total_(waits), waiting_(0), own_mutex_(true), blocker_(blocker) {
    mu_ = new std::mutex;
    cv_ = new std::condition_variable;
  }

  BlockableBarrier(size_t waits, std::mutex* mutex, std::condition_variable* cv,
                   bool own_mutex = false, CondVar* blocker = nullptr)
      : total_(waits), waiting_(0), own_mutex_(own_mutex), blocker_(blocker), mu_(mutex), cv_(cv) {}

  ~BlockableBarrier() {
    if (own_mutex_) {
      delete mu_;
      delete cv_;
    }
  }

  void await() {
    std::unique_lock<std::mutex> lock(*mu_);
    waiting_++;
    if (waiting_ < total_) {
      cv_->wait(lock);
    } else {
      if (blocker_ != nullptr) {
        blocker_->Wait();
      }
      waiting_ = 0;
      cv_->notify_all();
    }
  }

 private:
  size_t total_;
  size_t waiting_;
  std::mutex* mu_;
  std::condition_variable* cv_;
  bool own_mutex_;
  CondVar* blocker_{nullptr};
};
}  // namespace lizlib

#endif  //LIZLIB_BLOCKABLE_BARRIER_H
