

#include "net/channel/timer_channel.h"
#include <sys/timerfd.h>

const lizlib::File& lizlib::TimerChannel::GetFile() {
  return file_;
}
void lizlib::TimerChannel::HandleEvents(lizlib::ReceiveEvents events, lizlib::Timestamp now) {
  uint64_t val;
  if (file_.Read(&val, sizeof(val)) != sizeof(val)) {
    LOG_WARN("failed to read timer event : {}", Status::FromErr());
  }
  if (callback_ != nullptr) {
    callback_();
  }
}

std::string lizlib::TimerChannel::String() const {
  return fmt::format("TimerChannel[fd={}]", file_.Fd());
}
lizlib::File lizlib::TimerChannel::createTimerFile() {
  int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
  if (timer_fd < 0) {
    LOG_FATAL("timer fd creation failed : {}", Status::FromErr());
  }
  return File{timer_fd};
}
