//
// Created by A on 2023/8/16.
//

#include "net/event.h"
#include <sys/epoll.h>
#include <sys/poll.h>

namespace lizlib {
const ReceiveEvents ReceiveEvents::kHangUp{POLLHUP};
const ReceiveEvents ReceiveEvents::kInvalid{POLLNVAL};
const ReceiveEvents ReceiveEvents::kError{POLLERR};
const ReceiveEvents ReceiveEvents::kReadable{POLLIN};
const ReceiveEvents ReceiveEvents::kPriorReadable{POLLPRI};
const ReceiveEvents ReceiveEvents::kReadHangUp{POLLRDHUP};
const ReceiveEvents ReceiveEvents::kWritable{POLLOUT};

const SelectEvents SelectEvents::kNoneEvent{0};
const SelectEvents SelectEvents::kReadEvent{POLLIN | POLLPRI};
const SelectEvents SelectEvents::kWriteEvent{POLLOUT};

SelectEvents SelectEvents::Trigger(SelectTrigger trigger) const noexcept {
  SelectEvents events = *this;
  if (trigger == SelectTrigger::kEdge) {
    events.Add(SelectEvents{EPOLLET});
  }
  return events;
}

};  // namespace lizlib
