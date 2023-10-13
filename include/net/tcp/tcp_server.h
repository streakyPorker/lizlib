//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_SERVER_H
#define LIZLIB_TCP_SERVER_H
#include "common/basic.h"
#include "net/eventloop/event_loop_group.h"
#include "net/tcp/tcp_connection.h"
namespace lizlib {
class TcpServerChannelHandler;

class TcpServer {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(TcpServer);
  friend class TcpServerChannelHandler;

 private:
  EventLoopGroup::Ptr boss_group_;
  EventLoopGroup::Ptr worker_group_;
  Acceptor::Ptr acceptor_;

  ChannelBuilder builder_;

  std::mutex mu_;
  std::unordered_set<TcpConnection::Ptr> conns_;
};

}  // namespace lizlib

#endif  //LIZLIB_TCP_SERVER_H
