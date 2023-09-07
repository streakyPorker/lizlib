//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_EVENT_LOOP_H
#define LIZLIB_EVENT_LOOP_H

#include "concurrent/thread_pool.h"
#include "net/selector/selector.h"
namespace lizlib {

class EventLoop : public Executor {

 public:




 private:
  std::unique_ptr<ThreadPool> pool_;

  static EventLoop* current() {
    thread_local EventLoop* current = nullptr;
    return current;
  }
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_LOOP_H
