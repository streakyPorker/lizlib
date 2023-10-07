

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

  void wait() {
    std::unique_lock<std::mutex> lock(*mu_);
    cv_->wait(lock);
  }
  void notify_one() { cv_->notify_one(); }
  void notify_all() { cv_->notify_all(); }

 private:
  bool own_mutex_;
  std::mutex* mu_;
  std::condition_variable* cv_;
};
}  // namespace lizlib

#endif  //LIZLIB_COND_VAR_H