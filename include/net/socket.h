//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_SOCKET_H
#define LIZLIB_SOCKET_H

#include <netinet/in.h>
#include <netinet/tcp.h>
#include "common/basic.h"
#include "common/file.h"
#include "inet_address.h"

namespace lizlib {

class Socket : public File {
 public:
  Socket() : Socket(-1) {}
  Socket(Socket&& other) noexcept : File(std::forward<File>(other)) {}
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

  void ApplyDefaultOption() {
    SetReuseAddr(NetOption::reuse_addr);
    SetReuseAddr(NetOption::reuse_addr);
    SetKeepAlive(NetOption::keep_alive);
    SetTcpNoDelay(NetOption::tcp_no_delay);
  }

  [[nodiscard]] InetAddress GetLocalAddress() const;
  [[nodiscard]] InetAddress GetPeerAddress() const;

  void Shutdown(bool close_read = true);

  [[nodiscard]] std::string String() const override {
    return fmt::format("[Socket{}]", fd_);
  }

 private:
  explicit Socket(int fd) : File(fd) {}
};

}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::Socket);

#endif  //LIZLIB_SOCKET_H
