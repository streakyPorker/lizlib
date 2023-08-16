//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_EVENT_CHANNEL_H
#define LIZLIB_EVENT_CHANNEL_H
#include "channel.h"

#include <utility>
#include "common/basic.h"
namespace lizlib {
class EventChannel final : public Channel {
 public:
  EventChannel() : Channel(), file_(createEventFile()) {}
  ~EventChannel() override = default;

  const File& GetFile() override { return file_; }
  void HandleEvents(Events events, Timestamp now) override;
  [[nodiscard]] std::string String() const override;

  inline void SetCallback(Callback callback) { callback_ = std::move(callback); }

  /**
   * to wake up an event channel, just write a byte to it
   */
  void WakeUp();

 private:
  File file_;
  Callback callback_;

 private:
  static File createEventFile();
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_CHANNEL_H
