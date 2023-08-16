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

void lizlib::Socket::Bind(const lizlib::InetAddress &addr) {
    Slice impl = addr.Data();
    if (::bind(fd_, reinterpret_cast<const sockaddr *>(impl.Data()),
               impl.Length()) != 0) {
        LOG_ERROR("failed to bind {}", addr);
    }
}

void lizlib::Socket::Listen() {
    if (::listen(fd_, SOMAXCONN) < 0) {
        LOG_ERROR("{} failed to listen", fd_);
    }
}

lizlib::Status lizlib::Socket::Accept(lizlib::InetAddress *remote,
                                      lizlib::Socket *socket) {
    *remote = std::move(InetAddress());
    Slice data = remote->Data();
    socklen_t len;
    Socket gen = Socket{::accept4(fd_, reinterpret_cast<sockaddr *>(data.Data()),
                                  &len, SOCK_NONBLOCK | SOCK_CLOEXEC)};
    if (!gen.Valid()) {
        LOG_ERROR("{} failed to accept", fd_);
        return Status::FromErr();
    }
    *socket = std::move(gen);
    return Status::Success();
}
