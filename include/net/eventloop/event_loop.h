//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_EVENT_LOOP_H
#define LIZLIB_EVENT_LOOP_H

#include <utility>
#include "concurrent/executor.h"
#include "concurrent/thread_pool.h"
#include "net/callbacks.h"
#include "net/selector/selector.h"

namespace lizlib {

class EventLoop : public Executor {

 public:
  static EventLoop* Current() noexcept { return current(); }

  static bool CheckUnderLoop(EventLoop* loop) { return current() == loop; }

  EventLoop() : EventLoop(nullptr) {}

  explicit EventLoop(const EventScheduler::Ptr& scheduler);

  EventLoop(EventLoop&& other) noexcept : EventLoop(other.GetTimeScheduler()) {}

  template <typename Runnable>
  void Run(Runnable&& runnable) {
    if (current() == this) {
      runnable();
      return;
    }
    Schedule(std::forward<Runnable>(runnable));
  }

  Selector* GetSelector() noexcept;
  EventScheduler::Ptr GetTimeScheduler();;

  void Submit(const Runnable& runnable) override;
  void Join() override;
  [[nodiscard]] inline size_t Size() const noexcept override { return 1; }
  void SubmitDelay(const Runnable& runnable, Duration delay) override;
  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;

  void AddChannel(const Channel::Ptr& channel, const Callback& cb);

  void RemoveChannel(const Channel::Ptr& channel, const Callback& cb);

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
