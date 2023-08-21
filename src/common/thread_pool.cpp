//
// Created by A on 2023/8/3.
//

#include "common/thread_pool.h"
lizlib::ThreadPool::ThreadPool(uint32_t core_thread, bool start)
    : coreThreads_(core_thread),
      started_(start),
      barrier_{core_thread, nullptr} {
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
  if (!started_.load(std::memory_order_relaxed) && !workers_.empty()) {
    for (int i = 0; i < coreThreads_; i++) {
      auto* worker = &workers_[i];
      worker->thread = std::make_shared<std::thread>(
        [worker, this]() { this->coreWorkerRoutine(worker); });
    }
    started_.store(true, std::memory_order_release);
  }
}

void lizlib::ThreadPool::Join(bool discard_tasks) {
  if (discard_tasks) {
    cancel_signal_.store(true, std::memory_order_release);
    wait_cv_.notify_all();
  }
  for (Worker& worker : workers_) {
    // once joined/detached ,the std::thread would not be joinable
    if (worker.thread->joinable()) {
      worker.thread->join();
    }
  }
}

void lizlib::ThreadPool::Stop() {
  started_.store(false, std::memory_order_release);
  Join(true);
  workers_.clear();
  LOG_TRACE("thread_pool({}) stopped", coreThreads_);
}
void lizlib::ThreadPool::enqueueTask(const lizlib::Runnable& work,
                                     lizlib::Duration delay,
                                     lizlib::Duration interval) {
  std::lock_guard<std::mutex> guard{global_mu_};
  active_queue_.emplace_back(work, delay, interval);
  if (active_queue_.size() == 1) {
    wait_cv_.notify_all();
  }
}

void lizlib::ThreadPool::coreWorkerRoutine(lizlib::ThreadPool::Worker* worker) {
  while (!cancel_signal_.load(std::memory_order_relaxed)) {
    Job cur_job;
    {
      std::lock_guard<std::mutex> guard{global_mu_};
      if (!active_queue_.empty()) {
        cur_job = std::move(active_queue_.front());
        active_queue_.pop_front();
      }
    }
    if (cur_job.func != nullptr) {

      if (cur_job.interval.Valid()) {  // every
        if (cur_job.delay.Valid()) {
          std::this_thread::sleep_for(cur_job.delay.MicroSec());
        }
        cur_job.func();
        std::this_thread::sleep_for(cur_job.interval.MicroSec());
        std::lock_guard<std::mutex> guard{global_mu_};
        active_queue_.emplace_back(std::move(cur_job));
      } else if (cur_job.delay.Valid()) {  // delay
        std::this_thread::sleep_for(cur_job.delay.MicroSec());
        cur_job.func();
      } else {  // normal
        cur_job.func();
      }
    } else {

      std::unique_lock<std::mutex> waiter{global_mu_};
      wait_cv_.wait(waiter);
    }
  }
}
void lizlib::ThreadPool::Submit(const std::function<void()>& runnable) {
  using namespace std::chrono_literals;
  enqueueTask(runnable, Duration::Invalid(), Duration::Invalid());
}

void lizlib::ThreadPool::SubmitDelay(const lizlib::Runnable& runnable,
                                     lizlib::Duration delay) {
  enqueueTask(runnable, delay, Duration::Invalid());
}
void lizlib::ThreadPool::SubmitEvery(const lizlib::Runnable& runnable,
                                     lizlib::Duration delay,
                                     lizlib::Duration interval) {
  enqueueTask(runnable, delay, interval);
}
