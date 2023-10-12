//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_EVENT_LOOP_H
#define LIZLIB_EVENT_LOOP_H

#include <utility>

#include "concurrent/thread_pool.h"
#include "net/channel/event_channel.h"
#include "net/selector/selector.h"
namespace lizlib {

class EventLoop : public Executor {

 public:
  static EventLoop* GetEventLoop() noexcept { return current(); }

  explicit EventLoop(const TimerScheduler::Ptr& scheduler) : pool_{1, scheduler} {
    LOG_TRACE("EventLoop Created");
    pool_.Submit([this]() { current() = this; });
  }

  EventLoop(EventLoop&& other)  noexcept : EventLoop(other.GetTimeScheduler()) {}

  template <typename Runnable>
  void Run(Runnable&& runnable) {
    if (current() == this) {
      runnable();
      return;
    }
    Schedule(std::forward<Runnable>(runnable));
  }

  Selector* GetSelector() noexcept { return &pool_.GetTimerScheduler()->epoll_selector_; }
  TimerScheduler::Ptr GetTimeScheduler() { return pool_.GetTimerScheduler(); };
  void Submit(const Runnable& runnable) override;
  void Join() override;
  [[nodiscard]] inline size_t Size() const noexcept override { return 1; }
  void SubmitDelay(const Runnable& runnable, Duration delay) override;
  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;

  ~EventLoop() { pool_.Join(); }

 private:
  static EventLoop*& current() {
    thread_local EventLoop* current = nullptr;
    return current;
  }
  ThreadPool pool_;
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_LOOP_H
