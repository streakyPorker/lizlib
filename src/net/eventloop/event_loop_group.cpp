//
// Created by lzy on 2023/8/20.
//

#include "net/eventloop/event_loop_group.h"
void lizlib::EventLoopGroup::Submit(const lizlib::Runnable& runnable) {
  Next()->Submit(runnable);
}

size_t lizlib::EventLoopGroup::Size() const noexcept {
  return 0;
}
void lizlib::EventLoopGroup::SubmitAfter(const Runnable& runnable, Duration delay) {
  Next()->SubmitAfter(runnable, delay);
}
void lizlib::EventLoopGroup::SubmitEvery(const lizlib::Runnable& runnable, lizlib::Duration delay,
                                         lizlib::Duration interval) {
  Next()->SubmitEvery(runnable, delay, interval);
}
size_t lizlib::EventLoopGroup::next() noexcept {
  return next_.fetch_add(1, std::memory_order_acq_rel) % size_;
}
void lizlib::EventLoopGroup::handleJoin() {
  for (auto& el : loops_) {
    el.Join();
  }
}
lizlib::EventLoopGroup::EventLoopGroup(size_t loop_size)
    : timer_scheduler_{std::move(std::make_shared<EventScheduler>(config::kEpollEventPoolSize))},
      size_(loop_size),
      next_(0) {
  for (int i = 0; i < size_; i++) {
    loops_.emplace_back(timer_scheduler_);
  }
}
