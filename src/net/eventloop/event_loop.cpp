//
// Created by lzy on 2023/8/20.
//

#include "net/eventloop/event_loop.h"
void lizlib::EventLoop::Submit(const lizlib::Runnable& runnable) {
  if(current()==this){
    runnable();
  }else{
    pool_->Submit(runnable);
  }
}
void lizlib::EventLoop::Join() { pool_->Join(); }
void lizlib::EventLoop::SubmitDelay(const lizlib::Runnable& runnable, lizlib::Duration delay) {
  pool_->SubmitDelay(runnable, delay);
}
void lizlib::EventLoop::SubmitEvery(const lizlib::Runnable& runnable, lizlib::Duration delay,
                                    lizlib::Duration interval) {
  pool_->SubmitEvery(runnable, delay, interval);
}
