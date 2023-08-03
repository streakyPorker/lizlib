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
  ThreadPool() : ThreadPool(1) {}

  explicit ThreadPool(uint32_t core_thread, bool start = true)
      : coreThreads_(core_thread), started_(start) {

    for (int i = 0; i < core_thread; i++) {
      workers_.emplace_back(i);
      if (start) {
        auto* new_worker = &workers_.back();
        new_worker->thread = std::make_shared<std::thread>(
          [new_worker, this]() { this->coreWorkerRoutine(new_worker); });
      }
    }
  }
  ~ThreadPool() { Stop(); }

  template <typename Func, typename... Args>
  auto Submit(Func&& func, Args&&... args)
    -> std::future<decltype(func(args...))> {
    using ReturnType = decltype(func(args...));
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    {
      std::lock_guard<std::mutex> guard{global_lock_};
      tasks_.push_back([task]() { (*task)(); });
    }
    return result;
  }
  void Submit(const std::function<void()>& runnable) {
    tasks_.push_back(runnable);
  }

  void Start() {
    if (!started_ && !workers_.empty()) {
      for (int i = 0; i < coreThreads_; i++) {
        auto* worker = &workers_[i];
        worker->thread = std::make_shared<std::thread>(
          [worker, this]() { this->coreWorkerRoutine(worker); });
      }
      started_ = true;
    }
  }

  void Stop() {
    if (started_) {
      started_ = false;
      cancel_signal_ = true;
      for (Worker& worker : workers_) {
        worker.thread->join();
      }
      LOG_TRACE("thread_pool({}) stopped", coreThreads_);
    }
  }

 private:
  struct Worker {
    explicit Worker(uint32_t id) : id(id) {}
    std::shared_ptr<std::thread> thread{nullptr};
    uint32_t id;

    Worker& operator=(const Worker& worker) = delete;
    Worker& operator=(Worker&& worker) noexcept {
      assert(id == worker.id);
      thread.swap(worker.thread);
    }
  };

  void coreWorkerRoutine(Worker* worker) {
    //    std::cout << "here is t" << worker->id << std::endl;
    while (!cancel_signal_) {
      Runnable job = nullptr;

      {
        std::lock_guard<std::mutex> guard{global_lock_};
        if (!tasks_.empty()) {
          job = tasks_.front();
          tasks_.pop_front();
        }
      }
      if (job) {
        job();
      }
    }
  }

  uint32_t coreThreads_{1};
  std::deque<std::function<void()>> tasks_;
  std::mutex global_lock_;
  std::condition_variable core_cv_;
  std::deque<Worker> workers_;
  std::atomic_bool cancel_signal_{false};
  std::atomic_bool started_{false};
};

}  // namespace lizlib

#endif  //LIZLIB_THREAD_POOL_H
