//
// Created by bangsun on 2023/10/13.
//

#ifndef LIZLIB_EXECUTOR_H
#define LIZLIB_EXECUTOR_H
#include "common/basic.h"

namespace lizlib {
using Runnable = std::function<void()>;
class Executor {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(Executor);
  LIZ_CLAIM_SHARED_PTR(Executor);
  virtual void Submit(const Runnable& runnable) = 0;
  virtual void Join() = 0;
  [[nodiscard]] virtual size_t Size() const noexcept = 0;
  virtual void SubmitDelay(const Runnable& runnable, Duration delay) = 0;
  virtual void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) = 0;

 protected:
  Executor() = default;
};

class SelfExecutor final : Executor {
  LIZ_CLAIM_SHARED_PTR(SelfExecutor);
  void Submit(const Runnable& runnable) override { runnable(); }
  void Join() override {}
  [[nodiscard]] size_t Size() const noexcept override { return 1; }
  void SubmitDelay(const Runnable& runnable, Duration delay) override {
    std::this_thread::sleep_for(delay.ChronoMicroSec());
    runnable();
  }
  [[noreturn]] void SubmitEvery(const Runnable& runnable, Duration delay,
                                Duration interval) override {
    std::this_thread::sleep_for(delay.ChronoMicroSec());
    while (true) {
      runnable();
      std::this_thread::sleep_for(interval.ChronoMicroSec());
    }
  }
};

}  // namespace lizlib
#endif  //LIZLIB_EXECUTOR_H
