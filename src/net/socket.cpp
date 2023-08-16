//
// Created by A on 2023/8/16.
//

#include "net/socket.h"
#include <netinet/in.h>
#include <sys/socket.h>
lizlib::Socket& lizlib::Socket::operator=(lizlib::Socket&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  File::Close();
  std::swap(fd_, other.fd_);
  return *this;
}

lizlib::Socket lizlib::Socket::Create(int domain, bool nonblock, int protocol) {
  int socket_type = SOCK_CLOEXEC | SOCK_STREAM;
  if (nonblock) {
    // non-blocking attr should be set explicitly
    socket_type |= SOCK_NONBLOCK;
  }
  int fd = ::socket(domain, socket_type, IPPROTO_TCP);
  ifUnlikely(fd < 0) {
    LOG_ERROR("failed to call ::socket({}, {}, {}), reason[{}]", domain,
              socket_type, protocol, Status::FromErr());
  }
}
