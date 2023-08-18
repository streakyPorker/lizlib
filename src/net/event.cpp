//
// Created by A on 2023/8/16.
//

#include "net/event.h"
#include <sys/epoll.h>
#include <sys/poll.h>

namespace lizlib {
const ReceiveEvents ReceiveEvents::kHangUp{EPOLLHUP};
const ReceiveEvents ReceiveEvents::kError{EPOLLERR};
const ReceiveEvents ReceiveEvents::kReadable{EPOLLIN};
const ReceiveEvents ReceiveEvents::kPriorReadable{EPOLLPRI};
const ReceiveEvents ReceiveEvents::kReadHangUp{EPOLLRDHUP};
const ReceiveEvents ReceiveEvents::kWritable{EPOLLOUT};

const SelectEvents SelectEvents::kNoneEvent{0};
const SelectEvents SelectEvents::kReadEvent{EPOLLIN | EPOLLPRI};
const SelectEvents SelectEvents::kWriteEvent{EPOLLOUT};

SelectEvents SelectEvents::Trigger(SelectTrigger trigger) const noexcept {
  SelectEvents events = *this;
  if (trigger == SelectTrigger::kEdge) {
    events.Add(SelectEvents{EPOLLET});
  }
  return events;
}

};  // namespace lizlib
