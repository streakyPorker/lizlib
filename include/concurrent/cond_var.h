

#ifndef LIZLIB_COND_VAR_H
#define LIZLIB_COND_VAR_H
#include "common/basic.h"
namespace lizlib {
class CondVar {
 public:
  CondVar() : mu_(new std::mutex), cv_(new std::condition_variable), own_mutex_(true){};
  CondVar(std::mutex* mutex, std::condition_variable* cv, bool own_mutex = false)
      : mu_(mutex), cv_(cv), own_mutex_(own_mutex) {}

  ~CondVar() {
    if (own_mutex_) {
      delete mu_;
      delete cv_;
    }
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(*mu_);
    cv_->wait(lock);
  }

  void Wait(const Duration& duration) {
    std::unique_lock<std::mutex> lock(*mu_);
    cv_->wait_until(lock, std::chrono::steady_clock::now() + duration.ChronoMicroSec());
  }
  void NotifyOne(bool lock_held = false) {
    if (lock_held) {
      cv_->notify_one();
    } else {
      std::unique_lock<std::mutex> lock(*mu_);
      cv_->notify_one();
    }
  }
  void NotifyAll(bool lock_held) {
    if (lock_held) {
      cv_->notify_all();
    } else {
      std::unique_lock<std::mutex> lock(*mu_);
      cv_->notify_all();
    }
  }
  std::mutex* GetMutex() { return mu_; }

 private:
  bool own_mutex_;
  std::mutex* mu_;
  std::condition_variable* cv_;
};
}  // namespace lizlib

#endif  //LIZLIB_COND_VAR_H
