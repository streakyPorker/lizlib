

#ifndef LIZLIB_TIMER_CHANNEL_H
#define LIZLIB_TIMER_CHANNEL_H

#include <sys/timerfd.h>
#include <utility>
#include "channel.h"
namespace lizlib {
class TimerChannel final : public Channel {
 public:
  LIZ_CLAIM_SHARED_PTR(TimerChannel);
  explicit TimerChannel(Callback callback = nullptr)
      : callback_(std::move(callback)), file_(std::forward<File>(createTimerFile())) {}
  const File& GetFile() override;
  void HandleEvents(ReceiveEvents events, Timestamp now) override;
  [[nodiscard]] std::string String() const override;

  void SetCallback(const Callback& callback) noexcept { callback_ = callback; }
  void SetTimer(const Duration& delay, const Duration& interval) noexcept {
    if (!delay.Valid() && !interval.Valid()) {
      LOG_WARN("invalid timer setting");
      return;
    }
    itimerspec timer_spec{};
    if (delay.Valid()) {
      timer_spec.it_value.tv_sec = delay.Sec();
      timer_spec.it_value.tv_nsec = delay.MicrosBelowSec() * 1000;
    }
    if (interval.Valid()) {
      timer_spec.it_interval.tv_sec = interval.Sec();
      timer_spec.it_interval.tv_nsec = interval.MicrosBelowSec() * 1000;
    }
    ASSERT_FATAL(::timerfd_settime(file_.Fd(), 0, &timer_spec, nullptr) == 0,
                 "initializing timer failed");
  }

 private:
  File file_;
  Callback callback_;
  static File createTimerFile();
};
}  // namespace lizlib

#endif  //LIZLIB_TIMER_CHANNEL_H
