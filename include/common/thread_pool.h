//
// Created by A on 2023/8/3.
//

#ifndef LIZLIB_THREAD_POOL_H
#define LIZLIB_THREAD_POOL_H

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
  virtual void SubmitEvery(const Runnable& runnable, Duration delay,
                           Duration interval) = 0;
  virtual void Start() = 0;
  virtual void Join(bool discard_tasks) = 0;
  virtual void Stop() = 0;

 protected:
  virtual ~Executor() = 0;
  Executor() = default;
};

class ThreadPool : public Executor {
 public:
  DISABLE_COPY_AND_MOVE(ThreadPool);

  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

  explicit ThreadPool(uint32_t core_thread, bool start = true);
  ~ThreadPool() override { Stop(); }

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
  void Submit(const Runnable& runnable) override;

  void SubmitDelay(const Runnable& runnable, Duration delay) override;

  void SubmitEvery(const Runnable& runnable, Duration delay,
                   Duration interval) override;

  void Start() override;

  void Join(bool discard_tasks) override;

  void Stop() override;

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

  void enqueueTask(const Runnable& work, Duration delay, Duration interval);

  void coreWorkerRoutine(Worker* worker);

  uint32_t coreThreads_{1};
  std::deque<Job> active_queue_;
  std::mutex global_mu_;
  std::condition_variable wait_cv_;
  BlockableBarrier barrier_;
  std::deque<Worker> workers_;
  std::atomic_bool cancel_signal_{false};
  std::atomic_bool started_{false};
};

}  // namespace lizlib

#endif  //LIZLIB_THREAD_POOL_H
