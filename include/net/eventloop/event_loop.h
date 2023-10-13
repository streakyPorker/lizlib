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
  static EventLoop* Current() noexcept { return current(); }

  static bool CheckUnderLoop(EventLoop* loop) { return current() == loop; }

  EventLoop() : EventLoop(nullptr) {}

  explicit EventLoop(const EventScheduler::Ptr& scheduler) : pool_{1, scheduler} {
    LOG_TRACE("EventLoop Created");
    pool_.Submit([this]() { current() = this; });
  }

  EventLoop(EventLoop&& other) noexcept : EventLoop(other.GetTimeScheduler()) {}

  template <typename Runnable>
  void Run(Runnable&& runnable) {
    if (current() == this) {
      runnable();
      return;
    }
    Schedule(std::forward<Runnable>(runnable));
  }

  Selector* GetSelector() noexcept { return &pool_.GetEventScheduler()->epoll_selector_; }
  EventScheduler::Ptr GetTimeScheduler() { return pool_.GetEventScheduler(); };
  void Submit(const Runnable& runnable) override;
  void Join() override;
  [[nodiscard]] inline size_t Size() const noexcept override { return 1; }
  void SubmitDelay(const Runnable& runnable, Duration delay) override;
  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;

  void AddChannel(const Channel::Ptr& channel, const Callback& cb) {
    LOG_TRACE("EventLoop::AddChannel({})", *channel);
    if (current() != this) {
      Submit([&]() mutable { AddChannel(channel, cb); });
    } else {
      GetSelector()->Add(channel, SelectEvents::kNoneEvent);
      if (cb) {
        cb();
      }
    }
  }

  void RemoveChannel(const Channel::Ptr& channel, const Callback& cb) {
    LOG_TRACE("EventLoop::RemoveChannel({})", *channel);
    if (current() != this) {
      Submit([&]() mutable { RemoveChannel(channel, cb); });
    } else {
      GetSelector()->Remove(channel);
      if (cb) {
        cb();
      }
    }
  }

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
