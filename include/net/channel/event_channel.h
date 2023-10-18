//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_EVENT_CHANNEL_H
#define LIZLIB_EVENT_CHANNEL_H
#include "channel.h"

#include <utility>
#include "common/basic.h"
namespace lizlib {
/**
 * <p>
 * eventfd tranfers a uint64 counter to represent that the event has happened
 * </p>
 *
 */
class EventChannel final : public Channel {
 public:
  LIZ_CLAIM_SHARED_PTR(EventChannel);
  EventChannel(bool non_block, bool sync, Executor* executor = nullptr)
      : executor_(executor), Channel(), file_(createEventFile(non_block, sync)) {}
  explicit EventChannel(Executor* executor = nullptr)
      : Channel(), file_(createEventFile()), executor_(executor) {}
  ~EventChannel() override = default;

  File& GetFile() override { return file_; }
  void HandleEvents(ReceiveEvents events, Timestamp now) override;
  [[nodiscard]] std::string String() const override;

  Executor* GetExecutor() override { return executor_; }

  void SetExecutor(Executor* executor) noexcept override { executor_ = executor; }

  inline void SetCallback(Callback callback) { callback_ = std::move(callback); }

  /**
   * to wake up an event channel, just write a byte to it
   */
  void WakeUp();

 private:
  File file_;
  Callback callback_;
  Executor* executor_{nullptr};

 private:
  static File createEventFile(bool non_block = true, bool sync = false);
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_CHANNEL_H
