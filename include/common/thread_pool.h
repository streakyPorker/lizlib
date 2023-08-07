//
// Created by A on 2023/8/3.
//

#ifndef LIZLIB_THREAD_POOL_H
#define LIZLIB_THREAD_POOL_H

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
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    enqueueTask([task]() { (*task)(); });
    return result;
  }
  void Submit(const std::function<void()>& runnable);

  void Start();

  void Stop();

 private:
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

  void enqueueTask(const std::function<void()>& runnable) {
    std::lock_guard<std::mutex> guard{global_lock_};
    tasks_.push_back(runnable);
    if (tasks_.size() == 1) {
      task_queue_cv_.notify_one();
    }
  }

  void coreWorkerRoutine(Worker* worker);

  uint32_t coreThreads_{1};
  std::deque<std::function<void()>> tasks_;
  std::mutex global_lock_;
  std::condition_variable task_queue_cv_;
  std::mutex task_queue_mutex_;
  std::deque<Worker> workers_;
  std::atomic_bool cancel_signal_{false};
  std::atomic_bool started_{false};
};

}  // namespace lizlib

#endif  //LIZLIB_THREAD_POOL_H
