//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_EVENT_LOOP_GROUP_H
#define LIZLIB_EVENT_LOOP_GROUP_H

#include "common/config.h"
#include "concurrent/thread_pool.h"
#include "event_loop.h"
namespace lizlib {
class EventLoopGroup final : public Executor {
 public:
  LIZ_CLAIM_SHARED_PTR(EventLoopGroup);

  explicit EventLoopGroup(size_t loop_size);

  template <typename... Executor>
  static void JoinAll(Executor&&... executor) {
    (executor->Join(), ...);
  }

  EventLoop* Next() { return &loops_[next()]; }

  void Submit(const Runnable& runnable) override;
  void Join() override { handleJoin(); };
  [[nodiscard]] size_t Size() const noexcept override;
  void SubmitDelay(const Runnable& runnable, Duration delay) override;
  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;

  ~EventLoopGroup() { handleJoin(); }

 private:
  size_t next() noexcept;
  void handleJoin();

  EventLoop timer_scheduler_;
  std::atomic_size_t next_;
  const size_t size_;
  std::vector<EventLoop> loops_;
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_LOOP_GROUP_H
