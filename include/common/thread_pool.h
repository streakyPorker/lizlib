//
// Created by A on 2023/8/3.
//

#ifndef LIZLIB_THREAD_POOL_H
#define LIZLIB_THREAD_POOL_H

#include <sys/epoll.h>
#include <utility>
#include "barrier.h"
#include "basic.h"
#include "logger.h"
namespace lizlib {
using Runnable = std::function<void()>;

class Executor {
  DISABLE_COPY_AND_MOVE(Executor);

  virtual void Submit(const Runnable& runnable) = 0;
  virtual void SubmitDelay(const Runnable& runnable, Duration delay) = 0;
  virtual void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval) = 0;
  virtual void Start() = 0;
  virtual void Join(bool discard_tasks) = 0;
  virtual void Stop() = 0;

 protected:
  Executor() = default;
};

class ThreadPool final : public Executor {
 public:
  DISABLE_COPY_AND_MOVE(ThreadPool);

  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

  explicit ThreadPool(uint32_t core_thread, bool start = true);

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

  void Start() override;

  void Join(bool discard_tasks) override;

  void Stop() override;

 private:
  struct Job {
    Runnable func{nullptr};
    bool once{true};
    int bind_timer{-1};
    Job() = default;
    Job(Runnable func, bool once, int bind_timer = -1)
        : func(std::move(func)), once(once), bind_timer(bind_timer) {}
    DISABLE_COPY(Job);
    Job(Job&& job) = default;
    Job& operator=(Job&& job) noexcept {
      func = std::move(job.func);
      std::swap(once, job.once);
      std::swap(bind_timer, job.bind_timer);
      return *this;
    }
  };

  struct Worker {
    std::shared_ptr<std::thread> thread{nullptr};
    uint32_t id;

    explicit Worker(uint32_t id) : id(id) {}

    DISABLE_COPY(Worker);
    Worker& operator=(Worker&& worker) noexcept {
      assert(id == worker.id);
      thread.swap(worker.thread);
      return *this;
    }
  };

  struct TimerWorker : public Worker {
    int timer_epoll_fd{-1};
    std::vector<epoll_event> events;

    explicit TimerWorker(uint32_t id, uint32_t event_buf_size) : Worker(id) {
      events.resize(event_buf_size);
    }
  };

  void enqueueTask(const Runnable& work, Duration delay, Duration interval);

  void coreWorkerRoutine(Worker* worker);

  void timerWorkerRoutine(TimerWorker* worker);

  uint32_t coreThreads_{1};
  std::deque<Job*> active_queue_;
  std::mutex global_mu_;
  std::condition_variable wait_cv_;
  std::deque<Worker> workers_;
  std::atomic_bool cancel_signal_{false};
  std::atomic_bool started_{false};

  TimerWorker timer_worker_;
};

}  // namespace lizlib

#endif  //LIZLIB_THREAD_POOL_H
