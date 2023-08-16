//
// Created by A on 2023/8/16.
//

#include "net/channel/event_channel.h"
#include <sys/eventfd.h>

using Inner = uint64_t;

lizlib::File lizlib::EventChannel::createEventFile(bool non_block, bool sync) {
  int eventfd_flags = EFD_CLOEXEC;
  if (non_block) {
    eventfd_flags |= EFD_NONBLOCK;
  }
  if (sync) {
    eventfd_flags |= EFD_SEMAPHORE;
  }
  int fd = ::eventfd(0, eventfd_flags);
  if (fd <= 0) {
    LOG_ERROR("failed to create event fd, reason[{}]", errno);
  }
  return File{fd};
}

void lizlib::EventChannel::HandleEvents(lizlib::Events events,
                                        lizlib::Timestamp now) {
  Inner val;

  // validate file first
  if (file_.Read(&val, sizeof(val)) != sizeof(val)) {
    LOG_ERROR("failed to read event fd: ", Status::FromErr());
  }
  LOG_TRACE("got an event happened {} time(s)", val);

  if (callback_) {
    callback_();
  }
}

std::string lizlib::EventChannel::String() const {
  return fmt::format("EventChannel[fd={}]", file_.Fd());
}

void lizlib::EventChannel::WakeUp() {
  Inner val = 1;
  file_.Write(&val, sizeof(val));
}
