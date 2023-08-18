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
  EventChannel(bool non_block, bool sync)
      : Channel(), file_(createEventFile(non_block, sync)) {}
  EventChannel() : Channel(), file_(createEventFile()) {}
  ~EventChannel() override = default;

  const File& GetFile() override { return file_; }
  void HandleEvents(ReceiveEvents events, Timestamp now) override;
  [[nodiscard]] std::string String() const override;

  inline void SetCallback(Callback callback) {
    callback_ = std::move(callback);
  }

  /**
   * to wake up an event channel, just write a byte to it
   */
  void WakeUp();

 private:
  File file_;
  Callback callback_;

 private:
  static File createEventFile(bool non_block = true, bool sync = false);
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_CHANNEL_H
