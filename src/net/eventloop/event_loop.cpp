//
// Created by lzy on 2023/8/20.
//

#include "net/eventloop/event_loop.h"
void lizlib::EventLoop::Submit(const lizlib::Runnable& runnable) {
  lfq_.Push(runnable, [this]() { loop_cv_.NotifyOne(false); });
}
void lizlib::EventLoop::Join() {
  //  pool_.Join();
}
void lizlib::EventLoop::SubmitDelay(const lizlib::Runnable& runnable, lizlib::Duration delay) {
  //  pool_.SubmitDelay(runnable, delay);
}
void lizlib::EventLoop::SubmitEvery(const lizlib::Runnable& runnable, lizlib::Duration delay,
                                    lizlib::Duration interval) {
  //  pool_.SubmitEvery(runnable, delay, interval);
}
void lizlib::EventLoop::RemoveChannel(const lizlib::Channel::Ptr& channel,
                                      const lizlib::Callback& cb, bool unbind_executor) {

  if (current() != this) {
    Submit([=]() mutable { RemoveChannel(channel, cb); });
  } else {
    LOG_TRACE("EventLoop::RemoveChannel({})", *channel);
    GetSelector()->Remove(channel);
    if (unbind_executor) {
      channel->SetExecutor(nullptr);
    }
    if (cb) {
      cb();
    }
  }
}
void lizlib::EventLoop::AddChannel(lizlib::Channel::Ptr channel, const lizlib::Callback& cb,
                                   const SelectEvents& mode) {

  if (current() != this) {
    Submit([=]() mutable { AddChannel(std::move(channel), cb, mode); });
  } else {
    LOG_TRACE("EventLoop::AddChannel({})", *channel);
    GetSelector()->Add(channel, mode);
    channel->SetExecutor(this);
    if (cb) {
      cb();
    }
  }
}

//
lizlib::EventLoop::EventLoop(const lizlib::EventScheduler::Ptr& scheduler)
    : loop_cv_(), thread_([this]() { loop(); }) {
  if (scheduler == nullptr) {
    scheduler_ = std::move(std::make_shared<EventScheduler>(config::kEpollEventPoolSize));
  } else {
    scheduler_ = scheduler;
  }
  loop_cv_.NotifyAll(false);
}
lizlib::Selector* lizlib::EventLoop::GetSelector() noexcept {
  return &scheduler_->epoll_selector_;
}
lizlib::EventScheduler::Ptr lizlib::EventLoop::getScheduler() {
  return scheduler_;
}
lizlib::EventChannel::Ptr lizlib::EventLoop::generateEventChannel() {
  std::make_shared<EventChannel>(true, false, nullptr, []() {});
}
