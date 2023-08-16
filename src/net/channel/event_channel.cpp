//
// Created by A on 2023/8/16.
//

#include "net/channel/event_channel.h"
#include <sys/eventfd.h>

lizlib::File lizlib::EventChannel::createEventFile() {
  int fd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (fd <= 0) {
    LOG_ERROR("failed to create event fd, reason[{}]", errno);
  }
  return File{fd};
}
void lizlib::EventChannel::HandleEvents(lizlib::Events events,
                                        lizlib::Timestamp now) {
  uint64_t val;

  // validate file first
  if (file_.Read(&val, sizeof(val)) != sizeof(val)) {
    LOG_ERROR("failed to read event fd: ", Status::FromErr());
  }

  if (callback_) {
    callback_();
  }
}
std::string lizlib::EventChannel::String() const {
  return fmt::format("EventChannel[fd={}]", file_.Fd());
}