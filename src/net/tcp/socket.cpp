//
// Created by A on 2023/8/16.
//

#include "net/tcp/socket.h"
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

lizlib::Status lizlib::Socket::Accept(lizlib::InetAddress* remote,
                                      lizlib::Socket* socket) {
  *remote = std::move(InetAddress());
  socklen_t len;
  Socket gen =
    Socket{::accept4(fd_, remote->Data(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC)};
  if (!gen.Valid()) {
    LOG_ERROR("{} failed to accept", fd_);
    return Status::FromErr();
  }
  if (len == sizeof(sockaddr_in)) {
    remote->ipv6_ = false;
    remote->host_.resize(INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &remote->impl_.in4.sin_addr, remote->host_.data(),
              INET_ADDRSTRLEN);
  } else {
    remote->ipv6_ = true;
    remote->host_.resize(INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &remote->impl_.in6.sin6_addr, remote->host_.data(),
              INET6_ADDRSTRLEN);
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
  socklen_t len;
  if (::getsockname(fd_, ret.Data(), &len) < 0) {
    LOG_ERROR("{} can't get local address", fd_);
  }
  return ret;
}
lizlib::InetAddress lizlib::Socket::GetPeerAddress() const {
  InetAddress ret{};
  socklen_t len;
  if (::getpeername(fd_, ret.Data(), &len) < 0) {
    LOG_ERROR("{} can't get peer address", fd_);
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
