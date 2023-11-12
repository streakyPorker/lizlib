//
// Created by bangsun on 2023/11/12.
//

#ifndef LIZLIB_EVENT_QUEUE_H
#define LIZLIB_EVENT_QUEUE_H

#include <utility>

#include "concurrent/lock_free_queue.h"
#include "net/callbacks.h"
#include "net/channel/event_channel.h"
namespace lizlib {
class TaskEvent {};

class EventQueue {
 public:
  explicit EventQueue() = default;

  void AddTask(Callback callback, const Callback& on_empty = nullptr) {
    lfq_.Push(std::move(callback), on_empty);
  }

  std::shared_ptr<Callback> PollTask() { return lfq_.Pop(); }

 private:
  LockFreeQueue<Callback> lfq_;
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_QUEUE_H
