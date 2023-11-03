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

  EventLoop(EventLoop&& other) noexcept : EventLoop(other.getTimeScheduler()) {}

    template <typename Runnable>
    void Run(Runnable&& runnable) {
      if (current() == this) {
        runnable();
        return;
      }
      Schedule(std::forward<Runnable>(runnable));
    }

  Selector* GetSelector() noexcept;

  void Submit(const Runnable& runnable) override;
  void Join() override;
  [[nodiscard]] inline size_t Size() const noexcept override { return 1; }
  void SubmitDelay(const Runnable& runnable, Duration delay) override;
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

  ~EventLoop() { pool_.Join(); }

 private:
  static EventLoop*& current() {
    thread_local EventLoop* current = nullptr;
    return current;
  }
  ThreadPool pool_;
  EventScheduler::Ptr getTimeScheduler();
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_LOOP_H
