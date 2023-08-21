

#include "net/channel/timer_channel.h"
#include <sys/timerfd.h>

const lizlib::File& lizlib::TimerChannel::GetFile() {
  return file_;
}
void lizlib::TimerChannel::HandleEvents(lizlib::ReceiveEvents events,
                                        lizlib::Timestamp now) {
  if (events.ContainsAny(ReceiveEvents::kReadable,
                         ReceiveEvents::kPriorReadable,
                         ReceiveEvents::kReadHangUp) &&
      callback_ != nullptr) {
    callback_();
  }
}
std::string lizlib::TimerChannel::String() const {
  return fmt::format("TimerChannel[fd={}]", file_.Fd());
}
lizlib::File lizlib::TimerChannel::createTimerFile() {
  int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, 0);
  if (timer_fd < 0) {
    LOG_FATAL("timer fd creation failed : {}", Status::FromErr());
  }
  return File{timer_fd};
}
