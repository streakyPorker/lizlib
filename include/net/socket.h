//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_SOCKET_H
#define LIZLIB_SOCKET_H

#include <netinet/in.h>
#include "common/basic.h"
#include "common/file.h"
#include "inet_address.h"

namespace lizlib {

class Socket : public File {
 public:
  Socket() : Socket(-1) {}
  Socket(Socket&& other) noexcept
      : File(std::forward<File>(other)) {}
  Socket& operator=(Socket&& other) noexcept;

  static Socket Create(int domain, bool nonblock, int protocol = IPPROTO_TCP);

  void Bind(const InetAddress& addr);

  void Listen();

  Status Accept(InetAddress* remote, Socket* socket);

  Status Connect(const InetAddress& address) {
    Slice impl = address.Data();
    Socket gen = Socket {
      ::connect(fd_, reinterpret_cast<sockaddr*>(impl.Data()), impl.Length());
    };
  };

  [[nodiscard]] std::string String() const override {
    return fmt::format("[Socket{}]", fd_);
  }

 private:
  explicit Socket(int fd) : File(fd) {}
};

}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::Socket);

#endif  //LIZLIB_SOCKET_H
