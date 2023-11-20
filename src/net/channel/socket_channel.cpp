//
// Created by lzy on 2023/8/17.
//

#include "net/channel/socket_channel.h"
#include <sys/epoll.h>
lizlib::File& lizlib::SocketChannel::GetFile() {
  return *this;
}
void lizlib::SocketChannel::HandleEvents(lizlib::ReceiveEvents events, lizlib::Timestamp now) {

  if (events.ContainsAny(ReceiveEvents::kReadable, ReceiveEvents::kPriorReadable,
                         ReceiveEvents::kReadHangUp) &&
      read_callback_ != nullptr) {
    LOG_TRACE("{} handling read event", *this);
    read_callback_(events, now);
  }

  if (events.Contains(ReceiveEvents::kWritable) && write_callback_ != nullptr) {
    LOG_TRACE("{} handling write event", *this);
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
    events_.Add(SelectEvents::kReadEvent.EdgeTrigger());
  } else {
    events_.Remove(SelectEvents::kReadEvent.EdgeTrigger());
  }
  if (prev != events_) {
    selector_->Update(shared_from_this(), events_);
  }
}
void lizlib::SocketChannel::SetWritable(bool on) {
  if (selector_ == nullptr) {
    LOG_FATAL("update {} while its selector is not set", *this);
  }
  auto prev = events_;
  if (on) {
    events_.Add(SelectEvents::kWriteEvent.EdgeTrigger());
  } else {
    events_.Remove(SelectEvents::kWriteEvent.EdgeTrigger());
  }
  if (prev != events_) {
    selector_->Update(shared_from_this(), events_);
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
