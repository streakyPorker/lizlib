//
// Created by lzy on 2023/8/20.
//

#include "net/eventloop/event_loop.h"
void lizlib::EventLoop::Submit(const lizlib::Runnable& runnable) {
  if (!cancel_signal_.load(std::memory_order_relaxed)) {
    lfq_.Push(runnable, [this]() { loop_cv_.NotifyOne(false); });
  }
}

void lizlib::EventLoop::SubmitAfter(const Runnable& runnable, Duration delay) {
  TimerChannel::Ptr tc = std::make_shared<TimerChannel>(nullptr, this);
  tc->SetCallback([runnable, this, tc]() mutable {
    if (!cancel_signal_.load(std::memory_order_relaxed) && runnable) {
      runnable();
      fmt::println("tc ref count:{}", tc.use_count());
      // remove immediately for After tasks
      RemoveChannel(std::move(tc), nullptr, true);
    }
  });

  AddChannel(
    tc, [tc, delay]() { tc->SetTimer(delay, Duration::Invalid()); },
    SelectEvents::kReadEvent.EdgeTrigger());
}
void lizlib::EventLoop::SubmitEvery(const lizlib::Runnable& runnable, lizlib::Duration delay,
                                    lizlib::Duration interval) {
  TimerChannel::Ptr tc = std::make_shared<TimerChannel>(nullptr, this);
  tc->SetCallback([runnable, this, tc]() mutable {
    if (!cancel_signal_.load(std::memory_order_relaxed) && runnable) {
      runnable();
    } else {
      RemoveChannel(std::move(tc), nullptr, true);
    }
  });
  AddChannel(
    tc, [tc, delay, interval]() { tc->SetTimer(delay, interval); },
    SelectEvents::kReadEvent.EdgeTrigger());
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
    : scheduler_(scheduler == nullptr
                   ? std::make_shared<EventScheduler>(config::kEpollEventPoolSize)
                   : scheduler),
      thread_([this]() {
        current() = this;
        loop();
      }) {}
lizlib::Selector* lizlib::EventLoop::GetSelector() noexcept {
  return &scheduler_->epoll_selector_;
}
lizlib::EventScheduler::Ptr lizlib::EventLoop::getScheduler() {
  return scheduler_;
}
