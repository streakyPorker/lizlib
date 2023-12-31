//
// Created by A on 2023/8/16.
//

#include "net/tcp/socket.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

lizlib::Socket& lizlib::Socket::operator=(lizlib::Socket&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  std::swap(fd_, other.fd_);
  return *this;
}

lizlib::Socket lizlib::Socket::Create(int domain, bool nonblock, int protocol) {
  int socket_type = SOCK_CLOEXEC | SOCK_STREAM;
  if (nonblock) {
    // non-blocking attr should be set explicitly
    socket_type |= SOCK_NONBLOCK;
  }
  int fd = ::socket(domain, socket_type, protocol);
  ifUnlikely(fd < 0) {
    LOG_ERROR("failed to call ::socket({}, {}, {}), reason[{}]", domain, socket_type, protocol,
              Status::FromErr());
  }
  return Socket{fd};
}

lizlib::Status lizlib::Socket::Bind(const InetAddress& addr) {
  if (::bind(fd_, addr.Data(), addr.Length()) != 0) {
    LOG_ERROR("failed to bind {}", addr);
    return Status::FromErr();
  }
  return Status::Success();
}

lizlib::Status lizlib::Socket::Listen() {
  if (::listen(fd_, SOMAXCONN) < 0) {
    LOG_ERROR("{} failed to listen", fd_);
    return Status::FromErr();
  }
  return Status::Success();
}

lizlib::Status lizlib::Socket::Accept(lizlib::InetAddress* remote, lizlib::Socket* socket) {

  socklen_t len = sizeof(sockaddr);
  Socket gen = Socket{::accept4(fd_, remote->Data(), &len, 0)};
  if (!gen.Valid()) {
    return Status::FromErr();
  }
  if (len == sizeof(sockaddr_in)) {
    remote->ipv6_enabled_ = false;
    remote->host_.resize(INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &remote->impl_.in4.sin_addr, remote->host_.data(), INET_ADDRSTRLEN);
  } else {
    remote->ipv6_enabled_ = true;
    remote->host_.resize(INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &remote->impl_.in6.sin6_addr, remote->host_.data(), INET6_ADDRSTRLEN);
  }
  *socket = std::move(gen);
  return Status::Success();
}
lizlib::Status lizlib::Socket::Connect(const lizlib::InetAddress& address) {
  if (::connect(fd_, address.Data(), address.Length()) < 0) {
    return Status::FromErr();
  }
  return Status::Success();
}
lizlib::InetAddress lizlib::Socket::GetLocalAddress() const {
  InetAddress ret{};
  ret.ipv6_enabled_ = IsIpv6();
  socklen_t len = ret.Length();
  if (::getsockname(fd_, ret.Data(), &len) < 0) {
    LOG_FATAL("{} can't get local address, reason : {}", *this, Status::FromErr());
  }
  return ret;
}
lizlib::InetAddress lizlib::Socket::GetPeerAddress() const {
  InetAddress ret{};
  ret.ipv6_enabled_ = IsIpv6();
  socklen_t len = ret.Length();
  if (::getpeername(fd_, ret.Data(), &len) < 0) {
    LOG_FATAL("{} can't get peer address, reason : {}", *this, Status::FromErr());
  }
  return ret;
}
void lizlib::Socket::Shutdown(bool close_read) {
  int shutdown_flag = close_read ? SHUT_RDWR : SHUT_WR;
  if (::shutdown(fd_, shutdown_flag) < 0) {
    LOG_FATAL("{} shutdown failed", fd_);
  }
}
void lizlib::Socket::SetTcpNoDelay(bool on) {
  int val = on ? 1 : 0;
  ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}
void lizlib::Socket::SetReuseAddr(bool on) {
  int val = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
}
void lizlib::Socket::SetReusePort(bool on) {
  int val = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
}
void lizlib::Socket::SetKeepAlive(bool on) {
  int val = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
}
