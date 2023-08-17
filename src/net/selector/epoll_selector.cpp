//
// Created by lzy on 2023/8/17.
//

#include "net/selector/epoll_selector.h"
void lizlib::EpollSelector::Add(lizlib::Channel* channel,
                                lizlib::SelectEvents events) {}
void lizlib::EpollSelector::Remove(lizlib::Channel* channel) {}
void lizlib::EpollSelector::Update(lizlib::Channel* channel,
                                   lizlib::SelectEvents events) {
}
lizlib::Status lizlib::EpollSelector::Wait(lizlib::Duration timeout,
                                           lizlib::SelectChannels* selected) {
  return Status::Success();
}
