//
// Created by lzy on 2023/8/20.
//

#include "net/eventloop/event_loop_group.h"
void lizlib::EventLoopGroup::Submit(const lizlib::Runnable& runnable) {}
void lizlib::EventLoopGroup::Join() {}
size_t lizlib::EventLoopGroup::Size() const noexcept {
  return 0;
}
void lizlib::EventLoopGroup::SubmitDelay(const lizlib::Runnable& runnable, lizlib::Duration delay) {

}
void lizlib::EventLoopGroup::SubmitEvery(const lizlib::Runnable& runnable, lizlib::Duration delay,
                                         lizlib::Duration interval) {}
