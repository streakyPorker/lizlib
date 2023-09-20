//
// Created by A on 2023/8/3.
//

#include "common/concurrent/thread_pool.h"
#include <sys/timerfd.h>

#include <utility>
#include "common/status.h"
#include "net/channel/timer_channel.h"
lizlib::ThreadPool::ThreadPool(uint32_t core_thread, std::shared_ptr<TimerScheduler> time_worker)
    : core_threads_(core_thread),
      timer_scheduler_{time_worker},
      cv_(&global_mu_, &wait_cv_, false) {
  if (timer_scheduler_ == nullptr) {
    timer_scheduler_ = std::make_shared<TimerScheduler>(core_thread);
    own_timer_scheduler_ = true;
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
    cv_.notify_all();
    for (Worker& worker : workers_) {
      // once joined/detached ,the std::thread would not be joinable
      if (worker.thread->joinable()) {
        worker.thread->join();
      }
    }
    //  timer worker will join itself
    LOG_TRACE("thread_pool({}) joined", core_threads_);
  }
}

void lizlib::ThreadPool::coreWorkerRoutine(lizlib::Worker* worker) {
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
      if (cur_job->once && cur_job->bind_channel != nullptr &&
          ValidFd(cur_job->bind_channel->GetFile().Fd())) {
        ::close(cur_job->bind_channel->GetFile().Fd());
        delete cur_job;
      }
    } else {
      cv_.wait();
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

    Job* job = new Job{work, !interval.Valid(), &active_queue_, new TimerChannel};
    job->bind_channel->SetCallback([this, job]() {
      {
        std::lock_guard<std::mutex> guard{global_mu_};
        // emplace front to achieve relative fairness
        active_queue_.emplace_front(job);
        if (active_queue_.size() == 1) {
          wait_cv_.notify_all();
        }
      }
      if (job->once) {
        timer_scheduler_->epoll_selector_.Remove(job->bind_channel);
      }
    });

    timer_scheduler_->epoll_selector_.Add(job->bind_channel,
                                          SelectEvents::kReadEvent.EdgeTrigger());
    job->bind_channel->SetTimer(delay, interval);

    //    epoll_event event{};
    //    event.data.ptr = new Job(work, !interval.Valid(), &active_queue_, timer_fd);
    //    event.events = EPOLLIN | EPOLLET;
    //    ASSERT_FATAL(
    //      ::epoll_ctl(timer_scheduler_->timer_epoll_fd, EPOLL_CTL_ADD, timer_fd, &event) == 0,
    //      "adding timer to epoll failed");
    //    ASSERT_FATAL(timerfd_settime(timer_fd, 0, &timer_spec, nullptr) == 0,
    //                 "initializing timer failed");
  } else {
    // emplace queue
    std::lock_guard<std::mutex> guard{global_mu_};
    active_queue_.emplace_back(new Job{work, true, &active_queue_, nullptr});
    if (active_queue_.size() == 1) {
      wait_cv_.notify_all();
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
