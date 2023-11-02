//
// Created by A on 2023/8/3.
//

#include "concurrent/thread_pool.h"
#include <pthread.h>
#include <utility>
#include "common/status.h"
#include "net/channel/timer_channel.h"

lizlib::ThreadPool::ThreadPool(uint32_t core_thread, EventScheduler::Ptr time_worker)
    : core_threads_(core_thread),
      event_scheduler_{std::move(time_worker)},
      cv_(&global_mu_, &wait_cv_, false) {
  if (event_scheduler_ == nullptr) {
    event_scheduler_ = std::make_shared<EventScheduler>(core_thread);
    own_event_scheduler_ = true;
  }

  for (int i = 0; i < core_thread; i++) {
    workers_.emplace_back(i);
    auto* new_worker = &workers_.back();
    new_worker->thread =
      std::make_shared<std::thread>([new_worker, this]() { this->coreWorkerRoutine(new_worker); });
  }
}

void lizlib::ThreadPool::Join() {
  if (!cancel_signal_.load(std::memory_order_relaxed)) {
    cancel_signal_.store(true, std::memory_order_release);
    cv_.NotifyAll(false);

    // join the worker
    for (Worker& worker : workers_) {
      // once joined/detached ,the std::thread would not be joinable
      if (worker.thread->joinable()) {
        worker.thread->join();
      }
    }

    {
      std::lock_guard<std::mutex> guard{global_mu_};
      while (!active_queue_.empty()) {
        LOG_TRACE("popping discarded job");
        Job* abandoned_job = active_queue_.front();
        if (abandoned_job->bind_channel != nullptr) {
          event_scheduler_->epoll_selector_.Remove(abandoned_job->bind_channel);
        }
        delete abandoned_job;  // this will do the epoll remove
        active_queue_.pop_front();
      }
    }

    //  timer worker will join itself
    if (own_event_scheduler_) {
      event_scheduler_->Join();
    }
    LOG_TRACE("thread_pool({}) joined", core_threads_);
  }
}

void lizlib::ThreadPool::coreWorkerRoutine(lizlib::Worker* worker) {
  pthread_setname_np(pthread_self(), fmt::format("worker[{}]", worker->id).c_str());
  while (!cancel_signal_.load(std::memory_order_relaxed)) {
    Job* cur_job = nullptr;
    {
      std::lock_guard<std::mutex> guard{global_mu_};
      if (!active_queue_.empty()) {
        cur_job = active_queue_.front();
        active_queue_.pop_front();
      }
    }

    if (cur_job != nullptr) {
      cur_job->func();
      // handle the delayed task, remove it at once
      if (cur_job->once) {
        delete cur_job;
      }
    } else {
      cv_.Wait();
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

    Job* job = new Job{work, !interval.Valid(), &active_queue_, new TimerChannel};
    job->bind_channel->SetCallback([this, job]() {
      if (job->once) {
        event_scheduler_->epoll_selector_.Remove(job->bind_channel);
      }
      job->func();
    });
    event_scheduler_->epoll_selector_.Add(job->bind_channel,
                                          SelectEvents::kReadEvent.EdgeTrigger());
    job->bind_channel->SetTimer(delay, interval);

  } else {
    // emplace queue
    std::unique_lock<std::mutex> guard{global_mu_};
    active_queue_.emplace_back(new Job{work, true, &active_queue_, nullptr});
    if (active_queue_.size() == 1) {
      cv_.NotifyAll(true);
    }
  }
}

lizlib::Worker& lizlib::Worker::operator=(lizlib::Worker&& worker) noexcept {
  assert(id == worker.id);
  thread.swap(worker.thread);
  return *this;
}
lizlib::Job& lizlib::Job::operator=(lizlib::Job&& job) noexcept {
  func = std::move(job.func);
  std::swap(once, job.once);
  std::swap(bind_channel, job.bind_channel);
  std::swap(destination, job.destination);
  return *this;
}
void lizlib::EventScheduler::schedulerWorkerRoutine() {
  pthread_setname_np(pthread_self(), "scheduler");
  using namespace std::chrono_literals;
  while (!cancel_signal_.load(std::memory_order_relaxed)) {
    Status rst =
      epoll_selector_.Wait(Duration::FromMilliSecs(config::kSelectTimeoutMilliSecs), &results_);
    if (!rst.OK()) {
      if (rst.Code() == EINTR) {
        LOG_TRACE("timeout")
        std::cout.flush();
        continue;
      }
      LOG_FATAL("epoll wait failed : {}", rst);
    }
    if (results_.events.empty()) {
      continue;
    }
    results_.Process();
  }
}
void lizlib::EventScheduler::Join() {
  if (!cancel_signal_.load(std::memory_order_relaxed)) {
    cancel_signal_.store(true, std::memory_order_release);
    if (thread_.joinable()) {
      thread_.join();
    }
    LOG_TRACE("timer scheduler joined");
  }
}
lizlib::EventScheduler::EventScheduler(uint32_t event_buf_size)
    : thread_{[this]() {
        this->schedulerWorkerRoutine();
      }},
      epoll_selector_(event_buf_size) {}
