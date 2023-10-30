//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_CLIENT_H
#define LIZLIB_TCP_CLIENT_H
#include "common/basic.h"
#include "net/eventloop/event_loop_group.h"
#include "tcp_connection.h"

class TcpClientInternalHandler;

namespace lizlib {
class TcpClient {
 public:
  friend class TcpClientInternalHandler;
  LIZ_DISABLE_COPY_AND_MOVE(TcpClient);
  enum class State { kOffline, kConnecting, kConnected, kDisconnecting, kDisconnected };

 private:
  EventLoopGroup::Ptr worker_group_;
  TcpConnection::Ptr conn_;
  InetAddress address_;
  std::atomic<State> state_{State::kOffline};

  EventLoop* eventloop_{};
};
}  // namespace lizlib

#endif  //LIZLIB_TCP_CLIENT_H
