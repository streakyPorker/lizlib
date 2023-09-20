//
// Created by A on 2023/8/3.
//

#ifndef LIZLIB_THREAD_POOL_H
#define LIZLIB_THREAD_POOL_H

#include <sys/epoll.h>
#include <utility>
#include "blockable_barrier.h"
#include "common/basic.h"
#include "common/logger.h"
#include "net/channel/timer_channel.h"
#include "net/selector/epoll_selector.h"

namespace lizlib {
using Runnable = std::function<void()>;

class Executor {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(Executor);
  virtual void Submit(const Runnable& runnable) = 0;
  virtual void Join() = 0;
  virtual size_t Size() const noexcept = 0;
  virtual void SubmitDelay(const Runnable& runnable, Duration delay) = 0;
  virtual void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) = 0;

 protected:
  Executor() = default;
};

struct Job {
  Runnable func{nullptr};
  bool once{true};
  TimerChannel* bind_channel{nullptr};
  std::deque<Job*>* destination{nullptr};
  Job() = default;
  Job(Runnable func, bool once, std::deque<Job*>* destination, TimerChannel* binder)
      : func(std::move(func)), once(once), destination(destination), bind_channel(binder) {}
  LIZ_DISABLE_COPY(Job);
  Job(Job&& job) = default;
  Job& operator=(Job&& job) noexcept;
  ~Job() { delete bind_channel; }
};

struct Worker {
  std::shared_ptr<std::thread> thread{nullptr};
  uint32_t id;

  explicit Worker(uint32_t id) : id(id) {}

  LIZ_DISABLE_COPY(Worker);
  Worker& operator=(Worker&& worker) noexcept;
};

class TimerScheduler {

 public:
  LIZ_DISABLE_COPY_AND_MOVE(TimerScheduler);
  LIZ_CLAIM_SHARED_PTR(TimerScheduler);
  friend class ThreadPool;
  explicit TimerScheduler(uint32_t event_buf_size)
      : thread_{[this]() {
          this->timerWorkerRoutine();
        }},
        epoll_selector_(event_buf_size) {}

  void Join() {
    if (!cancel_signal_.load(std::memory_order_relaxed)) {
      cancel_signal_.store(true, std::memory_order_release);
      if (thread_.joinable()) {
        thread_.join();
      }
      LOG_TRACE("timer scheduler joined");
    }
  }

  void timerWorkerRoutine() {
    using namespace std::chrono_literals;
    while (!cancel_signal_.load(std::memory_order_relaxed)) {
      Status rst = epoll_selector_.Wait(1s, &results_);
      if (!rst.OK()) {
        LOG_FATAL("epoll wait failed : {}", rst);
      }
      if (results_.events.empty()) {
        continue;
      }
      results_.Process();
    }
  };
  ~TimerScheduler() { Join(); }

 public:
  EpollSelector epoll_selector_;
  SelectChannels results_{};
  std::thread thread_;
  std::atomic_bool cancel_signal_{false};
};

class ThreadPool final : public Executor {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(ThreadPool);
  LIZ_CLAIM_SHARED_PTR(ThreadPool);
  friend struct TimerScheduler;

  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

  explicit ThreadPool(uint32_t core_thread, std::shared_ptr<TimerScheduler> time_worker = nullptr);

  ~ThreadPool() { Join(); }

  template <typename Func, typename... Args>
  auto Submit(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
    using ReturnType = decltype(func(args...));
    using namespace std::chrono_literals;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    enqueueTask([task]() { (*task)(); }, Duration::Invalid(), Duration::Invalid());
    return result;
  };
  void Submit(const Runnable& runnable) override;

  void SubmitDelay(const Runnable& runnable, Duration delay) override;

  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) override;

  void Join() override;

  [[nodiscard]] size_t Size() const noexcept override { return core_threads_; }

  [[nodiscard]] TimerScheduler::Ptr GetTimerScheduler() const noexcept { return timer_scheduler_; }

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

  std::shared_ptr<TimerScheduler> timer_scheduler_;
  bool own_timer_scheduler_{false};
};

}  // namespace lizlib

#endif  //LIZLIB_THREAD_POOL_H
