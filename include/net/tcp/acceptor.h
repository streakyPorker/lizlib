//
// Created by bangsun on 2023/10/13.
//

#ifndef LIZLIB_ACCEPTOR_H
#define LIZLIB_ACCEPTOR_H

#include "common/basic.h"
#include "net/channel/channel.h"
#include "net/channel/socket_channel.h"
#include "net/tcp/socket.h"

namespace lizlib {
class EventLoop;
using AcceptorCallback = std::function<void(Socket)>;

/**
 * Help TcpServer accepting tcp connections from a fixed server address
 */
class Acceptor {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(Acceptor);
  LIZ_CLAIM_SHARED_PTR(Acceptor);

  Acceptor(EventLoop* eventloop, const InetAddress& address);

  ~Acceptor() { Close(); }

  void Bind() { socket_channel_->Bind(server_address_); }

  void OnAccept(AcceptorCallback acceptor_callback) {
    acceptor_callback_ = std::move(acceptor_callback);
  }

  void Listen();

  void Close();

  [[nodiscard]] std::string String() const;

 private:
  AcceptorCallback acceptor_callback_;
  InetAddress server_address_;
  SocketChannel::Ptr socket_channel_;
  EventLoop* eventloop_{nullptr};
};
}  // namespace lizlib
LIZ_FORMATTER_REGISTRY(lizlib::Acceptor);
#endif  //LIZLIB_ACCEPTOR_H
