//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_SOCKET_H
#define LIZLIB_SOCKET_H

#include "common/basic.h"
#include "common/file.h"
#include "net/config.h"
#include "net/inet_address.h"

namespace lizlib {

/**
 * a <code>Socket</code> represents a handle of connection-based network
 * protocol(namely TCP), it controls the data flow of the connection
 */
class Socket : public File {
 public:
  Socket() : Socket(-1) {}
  LIZ_DISABLE_COPY(Socket);
  Socket(Socket&& other) noexcept : File(std::move(other)) {}
  Socket& operator=(Socket&& other) noexcept;

  static Socket Create(int domain, bool nonblock, int protocol = IPPROTO_TCP);

  Status Bind(const InetAddress& addr);

  Status Listen();

  Status Accept(InetAddress* remote, Socket* socket);

  Status Connect(const InetAddress& address);

  void SetReuseAddr(bool on);

  void SetReusePort(bool on);

  void SetKeepAlive(bool on);

  void SetTcpNoDelay(bool on);

  void ApplySettingOption() {
    SetReuseAddr(config::kTcpOption.reuse_addr);
    SetReusePort(config::kTcpOption.reuse_port);
    SetKeepAlive(config::kTcpOption.keep_alive);
    SetTcpNoDelay(config::kTcpOption.tcp_no_delay);
  }

  [[nodiscard]] bool IsIpv6() const {
    int v6only;
    socklen_t option_len = sizeof(v6only);
    if (getsockopt(fd_, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, &option_len) == 0) {
      return v6only != 0;
    }
#ifdef IPV6_BINDV6ONLY
    if (getsockopt(fd_, IPPROTO_IPV6, IPV6_BINDV6ONLY, &v6only, &option_len) == 0) {
      return v6only != 0;
    }
#endif
    return false;
  }

  [[nodiscard]] InetAddress GetLocalAddress() const;
  [[nodiscard]] InetAddress GetPeerAddress() const;

  void Shutdown(bool close_read);

  [[nodiscard]] std::string String() const override { return fmt::format("[Socket-{}]", fd_); }

 private:
  explicit Socket(int fd) : File(fd) {}
};

}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::Socket);

#endif  //LIZLIB_SOCKET_H
