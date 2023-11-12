//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_EVENT_LOOP_H
#define LIZLIB_EVENT_LOOP_H

#include <utility>
#include "concurrent/executor.h"
#include "concurrent/thread_pool.h"
#include "event_queue.h"
#include "net/callbacks.h"
#include "net/channel/event_channel.h"
#include "net/selector/selector.h"

namespace lizlib {

class EventLoop : public Executor {

 public:
  static EventLoop* Current() noexcept { return current(); }

  static bool CheckUnderLoop(EventLoop* loop) { return current() == loop; }

  EventLoop() : EventLoop(nullptr) {}

  explicit EventLoop(const EventScheduler::Ptr& scheduler);

  EventLoop(EventLoop&& other) noexcept : EventLoop(other.getScheduler()) {}

  //  template <typename Runnable>
  //  void Run(Runnable&& runnable) {
  //    if (current() == this) {
  //      runnable();
  //      return;
  //    }
  //    Schedule(std::forward<Runnable>(runnable));
  //  }

  Selector* GetSelector() noexcept;

  void Submit(const Runnable& runnable) override;
  void Join() override { join(); };
  [[nodiscard]] inline size_t Size() const noexcept override { return 1; }
  void SubmitAfter(const Runnable& runnable, Duration delay) override;
  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;

  /**
   * Add channel to the eventloop, run the callback after it it done.
   * <br>
   * by default, the channel won't activate a epoll event, need to set it as Readable/Writable events
   * @param channel
   * @param cb
   * @param mode
   */
  void AddChannel(Channel::Ptr channel, const lizlib::Callback& cb,
                  const SelectEvents& mode = SelectEvents::kNoneEvent);

  void RemoveChannel(const Channel::Ptr& channel, const Callback& cb, bool unbind_executor = true);

  ~EventLoop() { join(); }

 private:
  static EventLoop*& current() {
    thread_local EventLoop* current = nullptr;
    return current;
  }

  EventScheduler::Ptr getScheduler();

  void loop() {
    current() = this;
    while (!cancel_signal_.load(std::memory_order_relaxed)) {
      std::shared_ptr<Callback> work;
      if (lfq_.Empty() || (work = lfq_.Pop()) == nullptr) {
        loop_cv_.Wait();
        continue;
      }
      ASSERT_FATAL(work != nullptr, "invalid work value");
      (*work)();
    }
  }

  void join() {
    if (!cancel_signal_.load(std::memory_order_relaxed)) {
      cancel_signal_.store(true, std::memory_order_release);
      loop_cv_.NotifyAll(false);
      if (thread_.joinable()) {
        thread_.join();
      }

      // remove time channel

      // scheduler_ and lfq_ will ~ after dtor
    }
  };

  EventScheduler::Ptr scheduler_;
  LockFreeQueue<Callback> lfq_;
  CondVar loop_cv_;
  std::atomic_bool cancel_signal_{false};
  std::thread thread_;
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_LOOP_H
