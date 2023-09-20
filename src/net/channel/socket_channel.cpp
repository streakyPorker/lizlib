//
// Created by lzy on 2023/8/17.
//

#include "net/channel/socket_channel.h"
#include <sys/epoll.h>
const lizlib::File& lizlib::SocketChannel::GetFile() {
  return *this;
}
void lizlib::SocketChannel::HandleEvents(lizlib::ReceiveEvents events, lizlib::Timestamp now) {
  LOG_TRACE("{} handling events_ {}", *this, events.String());

  if (events.ContainsAny(ReceiveEvents::kReadable, ReceiveEvents::kPriorReadable,
                         ReceiveEvents::kReadHangUp) &&
      read_callback_ != nullptr) {
    read_callback_(events, now);
  }

  if (events.Contains(ReceiveEvents::kWritable) && write_callback_ != nullptr) {
    write_callback_(events, now);
  }

  if (events.Contains(ReceiveEvents::kError) && error_callback_ != nullptr) {
    error_callback_(events, now);
  }

  if (events.Contains(ReceiveEvents::kHangUp) && !events.Contains(ReceiveEvents::kReadable) &&
      close_callback_ != nullptr) {
    close_callback_(events, now);
  }
}
std::string lizlib::SocketChannel::String() const {
  return fmt::format("SocketChannel[fd={}]", fd_);
}
void lizlib::SocketChannel::SetReadable(bool on) {
  if (selector_ == nullptr) {
    LOG_FATAL("update {} while its selector is not set", *this);
  }
  auto prev = events_;
  if (on) {
    events_.Add(SelectEvents::kReadEvent);
  } else {
    events_.Remove(SelectEvents::kReadEvent);
  }
  if (prev != events_) {
    selector_->Update(this, events_);
  }
}
void lizlib::SocketChannel::SetWritable(bool on) {
  if (selector_ == nullptr) {
    LOG_FATAL("update {} while its selector is not set", *this);
  }
  auto prev = events_;
  if (on) {
    events_.Add(SelectEvents::kWriteEvent);
  } else {
    events_.Remove(SelectEvents::kWriteEvent);
  }
  if (prev != events_) {
    selector_->Update(this, events_);
  }
}
lizlib::Status lizlib::SocketChannel::GetError() const noexcept {
  int val;
  auto len = static_cast<socklen_t>(sizeof val);
  if (::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &val, &len) < 0) {
    return Status::FromErr();
  } else {
    return Status{val};
  }
}
