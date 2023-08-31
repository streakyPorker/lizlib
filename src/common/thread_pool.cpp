//
// Created by A on 2023/8/3.
//

#include "common/thread_pool.h"
#include <sys/timerfd.h>
#include "common/status.h"
lizlib::ThreadPool::ThreadPool(uint32_t core_thread, bool start)
    : coreThreads_(core_thread), started_(start), timer_worker_{core_thread, core_thread} {

  timer_worker_.thread =
    std::make_shared<std::thread>([this]() { this->timerWorkerRoutine(&this->timer_worker_); });

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
      worker->thread =
        std::make_shared<std::thread>([worker, this]() { this->coreWorkerRoutine(worker); });
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
  timer_worker_.thread->join();
}

void lizlib::ThreadPool::Stop() {
  started_.store(false, std::memory_order_release);
  Join(true);
  workers_.clear();
  LOG_TRACE("thread_pool({}) stopped", coreThreads_);
}

void lizlib::ThreadPool::coreWorkerRoutine(lizlib::ThreadPool::Worker* worker) {
  while (!cancel_signal_.load(std::memory_order_relaxed)) {
    Job* cur_job = nullptr;
    {
      std::lock_guard<std::mutex> guard{global_mu_};
      if (!active_queue_.empty()) {
        cur_job = active_queue_.front();
        active_queue_.pop_front();
      }
    }
    if (cur_job != nullptr && cur_job->func != nullptr) {
      cur_job->func();
      if (cur_job->once) {
        if (ValidFd(cur_job->bind_timer)) {
          ::close(cur_job->bind_timer);
        }
        delete cur_job;
      }
    } else {
      std::unique_lock<std::mutex> waiter{global_mu_};
      wait_cv_.wait(waiter);
    }
  }
}
void lizlib::ThreadPool::Submit(const Runnable& runnable) {
  using namespace std::chrono_literals;
  enqueueTask(runnable, Duration::Invalid(), Duration::Invalid());
}

void lizlib::ThreadPool::SubmitDelay(const lizlib::Runnable& runnable, lizlib::Duration delay) {
  enqueueTask(runnable, delay, Duration::Invalid());
}
void lizlib::ThreadPool::SubmitEvery(const lizlib::Runnable& runnable, lizlib::Duration delay,
                                     lizlib::Duration interval) {
  enqueueTask(runnable, delay, interval);
}

void lizlib::ThreadPool::enqueueTask(const lizlib::Runnable& work, lizlib::Duration delay,
                                     lizlib::Duration interval) {

  if (interval.Valid() || delay.Valid()) {
    int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    itimerspec timer_spec{};
    if (delay.Valid()) {
      timer_spec.it_value.tv_sec = delay.Sec();
      timer_spec.it_value.tv_nsec = delay.MicrosBelowSec() * 1000;
    }
    if (interval.Valid()) {
      timer_spec.it_interval.tv_sec = interval.Sec();
      timer_spec.it_interval.tv_nsec = interval.MicrosBelowSec() * 1000;
    }
    epoll_event event{};
    event.data.ptr = new Job(work, !interval.Valid(), timer_fd);
    event.events = EPOLLIN | EPOLLET;
    ASSERT_FATAL(::epoll_ctl(timer_worker_.timer_epoll_fd, EPOLL_CTL_ADD, timer_fd, &event) == 0,
                 "adding timer to epoll failed");
    ASSERT_FATAL(timerfd_settime(timer_fd, 0, &timer_spec, nullptr) == 0,
                 "initializing timer failed");
  } else {
    // emplace queue
    std::lock_guard<std::mutex> guard{global_mu_};
    active_queue_.emplace_back(new Job{work, true});
    if (active_queue_.size() == 1) {
      wait_cv_.notify_all();
    }
  }
}

void lizlib::ThreadPool::timerWorkerRoutine(lizlib::ThreadPool::TimerWorker* worker) {
  uint64_t expirations;
  if (!ValidFd(worker->timer_epoll_fd)) {
    worker->timer_epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    ASSERT_FATAL(ValidFd(worker->timer_epoll_fd), "initializing timer worker failed");
  }
  while (!cancel_signal_.load(std::memory_order_release)) {
    int num_events =
      ::epoll_wait(worker->timer_epoll_fd, worker->events.data(), worker->events.size(), 1000);
    LOG_TRACE("epoll wait returned, size {}", num_events);
    for (int i = 0; i < num_events; i++) {
      Job* job = static_cast<Job*>(worker->events[i].data.ptr);
      ASSERT_FATAL(job != nullptr, "timer job fetch failed");

      {
        std::lock_guard<std::mutex> guard{global_mu_};
        // emplace front to achieve relative fairness
        active_queue_.emplace_front(job);
        if (active_queue_.size() == 1) {
          wait_cv_.notify_all();
        }
      }

      if (job->once && ValidFd(job->bind_timer)) {
        epoll_ctl(timer_worker_.timer_epoll_fd, EPOLL_CTL_DEL, job->bind_timer, nullptr);
      } else {
        ::read(job->bind_timer, &expirations, sizeof(expirations));
      }
    }
  }
}
