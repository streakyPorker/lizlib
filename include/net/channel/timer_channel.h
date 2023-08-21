

#ifndef LIZLIB_TIMER_CHANNEL_H
#define LIZLIB_TIMER_CHANNEL_H

#include "channel.h"

#include <utility>
namespace lizlib {
class TimerChannel final : public Channel {
 public:
  explicit TimerChannel(Callback callback = nullptr)
      : callback_(std::move(callback)),
        file_(std::forward<File>(createTimerFile())) {}
  const File& GetFile() override;
  void HandleEvents(ReceiveEvents events, Timestamp now) override;
  [[nodiscard]] std::string String() const override;

  void SetCallback(const Callback& callback) noexcept { callback_ = callback; }

 private:
  File file_;
  Callback callback_;
  static File createTimerFile();
};
}  // namespace lizlib

#endif  //LIZLIB_TIMER_CHANNEL_H
