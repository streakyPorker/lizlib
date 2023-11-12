//
// Created by A on 2023/8/3.
//

#ifndef LIZLIB_THREAD_POOL_H
#define LIZLIB_THREAD_POOL_H

#include <sys/epoll.h>
#include "common/basic.h"
#include "common/logger.h"
#include "concurrent/executor.h"
#include "cond_var.h"
#include "net/channel/channel.h"
#include "net/channel/timer_channel.h"
#include "net/config.h"
#include "net/selector/epoll_selector.h"

namespace lizlib {
using Runnable = std::function<void()>;
struct Job {
  Runnable func{nullptr};
  bool once{true};
  TimerChannel::Ptr bind_channel{nullptr};
  std::deque<Job*>* destination{nullptr};
  Job() = default;
  Job(Runnable func, bool once, std::deque<Job*>* destination, TimerChannel* channel)
      : func(std::move(func)), once(once), destination(destination) {
    bind_channel.reset(channel);
  }
  LIZ_DISABLE_COPY(Job);
  Job(Job&& job) = default;
  Job& operator=(Job&& job) noexcept;
  ~Job() {
    if (bind_channel != nullptr && ValidFd(bind_channel->GetFile().Fd())) {
      ::close(bind_channel->GetFile().Fd());
    }
  };
};

struct Worker {
  std::shared_ptr<std::thread> thread{nullptr};
  uint32_t id;

  explicit Worker(uint32_t id) : id(id) {}

  LIZ_DISABLE_COPY(Worker);
  Worker& operator=(Worker&& worker) noexcept;
};
class EventScheduler {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(EventScheduler);
  LIZ_CLAIM_SHARED_PTR(EventScheduler);
  friend class ThreadPool;
  explicit EventScheduler(uint32_t event_buf_size);
  void Join();
  void schedulerWorkerRoutine();

  ~EventScheduler() { Join(); }

 public:
  EpollSelector epoll_selector_;

 private:
  std::atomic_bool cancel_signal_{false};
  SelectedChannels results_{};
  std::thread thread_;
};

class ThreadPool final : public Executor {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(ThreadPool);
  LIZ_CLAIM_SHARED_PTR(ThreadPool);
  friend struct EventScheduler;

  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

  explicit ThreadPool(uint32_t core_thread, EventScheduler::Ptr time_worker = nullptr);

  ~ThreadPool() { Join(); }

  template <typename Func, typename... Args>
  auto SubmitWithFuture(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
    using ReturnType = decltype(func(args...));
    using namespace std::chrono_literals;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    enqueueTask([task]() { (*task)(); }, Duration::Invalid(), Duration::Invalid());
    return result;
  };
  void Submit(const Runnable& runnable) override;

  void SubmitAfter(const Runnable& runnable, Duration delay) override;

  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;

  void Join() override;

  [[nodiscard]] size_t Size() const noexcept override { return core_threads_; }

  [[nodiscard]] EventScheduler::Ptr GetEventScheduler() const noexcept { return event_scheduler_; }

 private:
  void enqueueTask(const Runnable& work, Duration delay, Duration interval);

  void coreWorkerRoutine(Worker* worker);

  uint32_t core_threads_{1};
  std::deque<Job*> active_queue_;
  std::mutex global_mu_;
  std::condition_variable wait_cv_;
  CondVar cv_;
  std::deque<Worker> workers_;
  std::atomic_bool cancel_signal_{false};

  EventScheduler::Ptr event_scheduler_;
  bool own_event_scheduler_{false};
};

}  // namespace lizlib

#endif  //LIZLIB_THREAD_POOL_H
