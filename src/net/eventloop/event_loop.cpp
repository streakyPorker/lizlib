//
// Created by lzy on 2023/8/20.
//

#include "net/eventloop/event_loop.h"
void lizlib::EventLoop::Submit(const lizlib::Runnable& runnable) {
  if (current() == this) {
    runnable();
  } else {
    pool_.Submit(runnable);
  }
}
void lizlib::EventLoop::Join() {
  pool_.Join();
}
void lizlib::EventLoop::SubmitDelay(const lizlib::Runnable& runnable, lizlib::Duration delay) {
  pool_.SubmitDelay(runnable, delay);
}
void lizlib::EventLoop::SubmitEvery(const lizlib::Runnable& runnable, lizlib::Duration delay,
                                    lizlib::Duration interval) {
  pool_.SubmitEvery(runnable, delay, interval);
}
void lizlib::EventLoop::RemoveChannel(const lizlib::Channel::Ptr& channel,
                                      const lizlib::Callback& cb, bool unbind_executor) {
  LOG_TRACE("EventLoop::RemoveChannel({})", *channel);
  if (current() != this) {
    Submit([&]() mutable { RemoveChannel(channel, cb); });
  } else {
    GetSelector()->Remove(channel);
    if (unbind_executor) {
      channel->SetExecutor(nullptr);
    }
    if (cb) {
      cb();
    }
  }
}
void lizlib::EventLoop::AddChannel(const lizlib::Channel::Ptr& channel, const lizlib::Callback& cb,
                                   const SelectEvents& mode) {
  LOG_TRACE("EventLoop::AddChannel({})", *channel);
  if (current() != this) {
    Submit([&]() mutable { AddChannel(channel, cb, mode); });
  } else {
    GetSelector()->Add(channel, mode);
    channel->SetExecutor(this);
    if (cb) {
      cb();
    }
  }
}
lizlib::EventLoop::EventLoop(const lizlib::EventScheduler::Ptr& scheduler) : pool_{1, scheduler} {
  LOG_TRACE("EventLoop Created");
  pool_.Submit([this]() { current() = this; });
}
lizlib::Selector* lizlib::EventLoop::GetSelector() noexcept {
  return &pool_.GetEventScheduler()->epoll_selector_;
}
lizlib::EventScheduler::Ptr lizlib::EventLoop::getTimeScheduler() {
  return pool_.GetEventScheduler();
}
