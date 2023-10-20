

#include "net/channel/timer_channel.h"
#include <sys/timerfd.h>

lizlib::File& lizlib::TimerChannel::GetFile() {
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
void lizlib::TimerChannel::SetTimer(const lizlib::Duration& delay,
                                    const lizlib::Duration& interval) noexcept {
  if (!delay.Valid() && !interval.Valid()) {
    LOG_WARN("invalid timer setting");
    return;
  }
  itimerspec timer_spec{};
  if (delay.Valid()) {
    Duration actual_delay{std::max(delay, kMinWakeUpDuration)};
    timer_spec.it_value.tv_sec = actual_delay.Sec();
    timer_spec.it_value.tv_nsec = actual_delay.MicrosBelowSec() * 1000;
  }
  if (interval.Valid()) {
    Duration actual_interval{std::max(interval, kMinWakeUpDuration)};
    timer_spec.it_interval.tv_sec = actual_interval.Sec();
    timer_spec.it_interval.tv_nsec = actual_interval.MicrosBelowSec() * 1000;
  }
  ASSERT_FATAL(::timerfd_settime(file_.Fd(), 0, &timer_spec, nullptr) == 0,
               "initializing timer failed");
}
