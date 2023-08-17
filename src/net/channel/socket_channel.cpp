//
// Created by lzy on 2023/8/17.
//

#include "net/channel/socket_channel.h"
const lizlib::File& lizlib::SocketChannel::GetFile() {
  return *this;
}
void lizlib::SocketChannel::HandleEvents(lizlib::Events events,
                                         lizlib::Timestamp now) {
  LOG_TRACE("{} handling events {}", *this, events);
}
std::string lizlib::SocketChannel::String() const {
  return fmt::format("SocketChannel[fd={}]", fd_);
}
