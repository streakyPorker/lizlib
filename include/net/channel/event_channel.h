//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_EVENT_CHANNEL_H
#define LIZLIB_EVENT_CHANNEL_H
#include "channel.h"
#include "common/basic.h"
namespace lizlib {
class EventChannel final : public Channel {
 public:
  EventChannel() : Channel(), file_(createEventFile()) {}
  ~EventChannel() override = default;

  const File& GetFile() override { return file_; }
  void HandleEvents(Events events, Timestamp now) override;
  [[nodiscard]] std::string String() const override;

 private:
  File file_;
  Callback callback_;

 private:
  static File createEventFile();
};
}  // namespace lizlib

#endif  //LIZLIB_EVENT_CHANNEL_H
