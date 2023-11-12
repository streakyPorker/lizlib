

#ifndef LIZLIB_TIMER_CHANNEL_H
#define LIZLIB_TIMER_CHANNEL_H

#include <sys/timerfd.h>
#include <utility>
#include "channel.h"
namespace lizlib {
class TimerChannel final : public Channel {
 public:
  LIZ_CLAIM_SHARED_PTR(TimerChannel);
  explicit TimerChannel(Callback callback = nullptr, Executor* executor = nullptr)
      : callback_(std::move(callback)),
        file_(std::forward<File>(createTimerFile())),
        executor_{executor} {}

  ~TimerChannel() override { LOG_TRACE("~{}", String().c_str()); };
  File& GetFile() override;
  void HandleEvents(ReceiveEvents events, Timestamp now) override;
  [[nodiscard]] std::string String() const override;

  Executor* GetExecutor() override { return executor_; }

  void SetExecutor(Executor* executor) noexcept override { executor_ = executor; }

  void SetCallback(const Callback& callback) noexcept { callback_ = callback; }

  void SetTimer(const Duration& delay, const Duration& interval) noexcept;

 private:
  inline static const Duration kMinWakeUpDuration{100};

  File file_;
  Executor* executor_{nullptr};
  Callback callback_;
  static File createTimerFile();
};
}  // namespace lizlib
LIZ_FORMATTER_REGISTRY(lizlib::TimerChannel);

#endif  //LIZLIB_TIMER_CHANNEL_H
