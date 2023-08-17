//
// Created by lzy on 2023/8/17.
//

#ifndef LIZLIB_EPOLL_SELECTOR_H
#define LIZLIB_EPOLL_SELECTOR_H

#include "common/file.h"
#include "selector.h"
#include <sys/epoll.h>
namespace lizlib{
class EpollSelector final :public File,public Selector{
 public:


  void Add(Channel* channel, SelectEvents events) override;
  void Remove(Channel* channel) override;
  void Update(Channel* channel, SelectEvents events) override;
  Status Wait(Duration timeout, SelectChannels* selected) override;

 private:
  std::vector<struct epoll_event> epoll_events_;

};
}

#endif  //LIZLIB_EPOLL_SELECTOR_H
