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
    wait_cv_.notify_all();
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
    Job cur_job;
    {
      std::lock_guard<std::mutex> guard{global_mutex_};
      if (!active_queue_.empty()) {
        cur_job = active_queue_.front();
        active_queue_.pop_front();
      }
    }
    if (cur_job.func != nullptr) {
      switch (cur_job.type) {
        case NORMAL:
          cur_job.func();
          break;
        case DELAY:
          std::this_thread::sleep_for(cur_job.time.MicroSec());
          cur_job.func();
        case EVERY:
          std::this_thread::sleep_for(cur_job.time.MicroSec());
          cur_job.func();
          std::lock_guard<std::mutex> guard{global_mutex_};
          active_queue_.push_back(cur_job);
          break;
      }
    } else {
      std::unique_lock<std::mutex> waiter{wait_mutex_};
      wait_cv_.wait(waiter);
    }
  }
}
void lizlib::ThreadPool::Submit(const std::function<void()>& runnable) {
  using namespace std::chrono_literals;
  enqueueTask(runnable, NORMAL, 0ms);
}
