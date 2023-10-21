//
// Created by lzy on 2023/8/17.
//

#include "net/selector/epoll_selector.h"
#include <sys/epoll.h>
void lizlib::EpollSelector::Add(const Channel::Ptr& channel, SelectEvents events) {
  internalUpdate(channel.get(), EPOLL_CTL_ADD, events);
}

void lizlib::EpollSelector::Remove(const Channel::Ptr& channel) {
  internalUpdate(channel.get(), EPOLL_CTL_DEL, SelectEvents::kNoneEvent);
}
void lizlib::EpollSelector::Update(const Channel::Ptr& channel, lizlib::SelectEvents events) {
  internalUpdate(channel.get(), EPOLL_CTL_MOD, events);
}
lizlib::Status lizlib::EpollSelector::Wait(lizlib::Duration timeout,
                                           lizlib::SelectedChannels* selected) {
  int count =
    ::epoll_wait(fd_, epoll_events_.data(), (int)epoll_events_.size(), (int)timeout.MilliSec());
  if (count < 0) {
    LOG_WARN("{}'s epoll_wait turns out abnormal", *this);
    epoll_events_.clear();
    return Status::FromErr();
  }
  selected->occur_ts = Timestamp::Now();
  selected->events.clear();
  selected->channels.clear();
  for (int i = 0; i < count; i++) {
    selected->channels.emplace_back(static_cast<Channel*>(epoll_events_[i].data.ptr));
    selected->events.emplace_back(epoll_events_[i].events);
  }
  return Status::Success();
}
void lizlib::EpollSelector::internalUpdate(lizlib::Channel* channel, int epoll_op,
                                           const lizlib::SelectEvents& select_events) {
  struct epoll_event update_event {};
  update_event.events = select_events.Value();
  update_event.data.ptr = channel;
  int update_fd = channel->GetFile().Fd();
  ASSERT_FATAL(
    ::epoll_ctl(fd_, epoll_op, update_fd, epoll_op == EPOLL_CTL_DEL ? nullptr : &update_event) >= 0,
    "{} updating epoll event failed : {}", *this, Status::FromErr());
}
size_t lizlib::EpollSelector::Size() {
  return epoll_events_.size();
}
