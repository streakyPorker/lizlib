//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_EVENT_LOOP_GROUP_H
#define LIZLIB_EVENT_LOOP_GROUP_H

#include "concurrent/thread_pool.h"
namespace lizlib {
class EventLoopGroup final : public Executor {
 public:
  void Submit(const Runnable& runnable) override;
  void Join() override;
  size_t Size() const noexcept override;
  void SubmitDelay(const Runnable& runnable, Duration delay) override;
  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_LOOP_GROUP_H
