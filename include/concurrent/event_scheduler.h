//
// Created by bangsun on 2023/10/13.
//

#ifndef LIZLIB_EVENT_SCHEDULER_H
#define LIZLIB_EVENT_SCHEDULER_H
#include "common/basic.h"
#include "common/logger.h"
#include "common/slice.h"
#include "common/status.h"
#include "net/selector/epoll_selector.h"
#include "net/selector/selector.h"
namespace lizlib {
class EventScheduler {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(EventScheduler);
  LIZ_CLAIM_SHARED_PTR(EventScheduler);
  friend class ThreadPool;
  explicit EventScheduler(uint32_t event_buf_size)
      : thread_{[this]() {
          this->schedulerWorkerRoutine();
        }},
        epoll_selector_(event_buf_size) {}

  void Join() {
    if (!cancel_signal_.load(std::memory_order_relaxed)) {
      cancel_signal_.store(true, std::memory_order_release);
      if (thread_.joinable()) {
        thread_.join();
      }
      LOG_TRACE("timer scheduler joined");
    }
  }

  void schedulerWorkerRoutine() {
    using namespace std::chrono_literals;
    while (!cancel_signal_.load(std::memory_order_relaxed)) {
      Status rst =
        epoll_selector_.Wait(Duration::FromMilliSecs(config::kSelectTimeoutMilliSecs), &results_);
      if (!rst.OK()) {
        LOG_FATAL("epoll wait failed : {}", rst);
      }
      if (results_.events.empty()) {
        continue;
      }
      results_.Process();
    }
  };
  ~EventScheduler() { Join(); }

 public:
  EpollSelector epoll_selector_;

 private:
  std::atomic_bool cancel_signal_{false};
  SelectChannels results_{};
  std::thread thread_;
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_SCHEDULER_H
