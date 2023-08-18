//
// Created by A on 2023/8/3.
//

#ifndef LIZLIB_THREAD_POOL_H
#define LIZLIB_THREAD_POOL_H

#include <utility>

#include "basic.h"
#include "logger.h"
namespace lizlib {
using Runnable = std::function<void()>;

class ThreadPool {
 public:
  DISABLE_COPY_AND_MOVE(ThreadPool);

  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

  explicit ThreadPool(uint32_t core_thread, bool start = true);
  ~ThreadPool() { Stop(); }

  template <typename Func, typename... Args>
  auto Submit(Func&& func, Args&&... args)
    -> std::future<decltype(func(args...))> {
    using ReturnType = decltype(func(args...));
    using namespace std::chrono_literals;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    enqueueTask([task]() { (*task)(); }, Duration::Invalid(),
                Duration::Invalid());
    return result;
  }
  void Submit(const Runnable& runnable);

  void SubmitDelay(const Runnable& runnable, Duration delay);

  void SubmitEvery(const Runnable& runnable, Duration delay, Duration interval);

  void Start();

  void Join();

  void Stop();

 private:
  struct Job {
    Runnable func{nullptr};
    Duration delay{-1};
    Duration interval{-1};
    Job() = default;
    Job(Runnable func, Duration delay, Duration interval)
        : func(std::move(func)),
          delay(std::move(delay)),
          interval(std::move(interval)) {}
    DISABLE_COPY(Job);
    Job(Job&& job) = default;
    Job& operator=(Job&& job) noexcept {
      func = std::move(job.func);
      delay = std::move(job.delay);
      interval = std::move(job.interval);
      return *this;
    }
  };

  struct Worker {
    explicit Worker(uint32_t id) : id(id) {}
    std::shared_ptr<std::thread> thread{nullptr};
    uint32_t id;

    Worker& operator=(const Worker& worker) = delete;
    Worker& operator=(Worker&& worker) noexcept {
      assert(id == worker.id);
      thread.swap(worker.thread);
      return *this;
    }
  };

  void enqueueTask(const Runnable& work, Duration delay, Duration interval) {
    std::lock_guard<std::mutex> guard{global_mutex_};
    active_queue_.emplace_back(work, delay, interval);
    if (active_queue_.size() == 1) {
      wait_cv_.notify_one();
    }
  }

  void coreWorkerRoutine(Worker* worker);

  uint32_t coreThreads_{1};
  std::deque<Job> active_queue_;
  std::mutex global_mutex_;

  std::mutex wait_mutex_;
  std::condition_variable wait_cv_;

  std::deque<Worker> workers_;

  std::atomic_bool cancel_signal_{false};
  std::atomic_bool started_{false};
};

}  // namespace lizlib

#endif  //LIZLIB_THREAD_POOL_H
