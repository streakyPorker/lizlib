//
// Created by A on 2023/8/3.
//

#include "common/thread_pool.h"
lizlib::ThreadPool::ThreadPool(uint32_t core_thread, bool start)
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
void lizlib::ThreadPool::Start() {
  if (!started_ && !workers_.empty()) {
    for (int i = 0; i < coreThreads_; i++) {
      auto* worker = &workers_[i];
      worker->thread = std::make_shared<std::thread>(
        [worker, this]() { this->coreWorkerRoutine(worker); });
    }
    started_ = true;
  }
}
void lizlib::ThreadPool::Stop() {
  if (started_.load(std::memory_order_relaxed)) {
    // ok even if executed multiple time
    started_.store(false, std::memory_order_release);
    cancel_signal_.store(true, std::memory_order_release);
    task_queue_cv_.notify_all();
    for (Worker& worker : workers_) {
      if (worker.thread->joinable()) {
        worker.thread->join();
      }
    }
    LOG_TRACE("thread_pool({}) stopped", coreThreads_);
  }
}
void lizlib::ThreadPool::coreWorkerRoutine(lizlib::ThreadPool::Worker* worker) {
  while (!cancel_signal_.load(std::memory_order_relaxed)) {
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
    } else {
      std::unique_lock<std::mutex> waiter{task_queue_mutex_};
      task_queue_cv_.wait(waiter);
    }
  }
}
void lizlib::ThreadPool::Submit(const std::function<void()>& runnable) {
  std::lock_guard<std::mutex> guard{global_lock_};
  using namespace std::chrono_literals;
  SubmitDelay(runnable,1ms);
  enqueueTask(runnable);
}
