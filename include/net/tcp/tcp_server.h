//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_SERVER_H
#define LIZLIB_TCP_SERVER_H
#include <unordered_set>
#include "acceptor.h"
#include "common/basic.h"
#include "net/eventloop/event_loop_group.h"
#include "net/tcp/tcp_connection.h"
namespace lizlib {
class TcpServerChannelHandler;

class TcpServer {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(TcpServer);
  friend class TcpServerChannelHandler;


  void SetGroup(EventLoopGroup::Ptr boss, EventLoopGroup::Ptr worker) {
    boss_group_ = std::move(boss);
    worker_group_ = std::move(worker);
  }

  void Bind(const InetAddress& address);

  void Start();
  void Close();

  void SetBuilder(ChannelBuilder builder) { builder_ = std::move(builder); }

 private:
  EventLoopGroup::Ptr boss_group_;
  EventLoopGroup::Ptr worker_group_;
//  Acceptor::Ptr acceptor_;

  ChannelBuilder builder_;

  std::mutex mu_;
  Acceptor::Ptr acceptor_;
  std::unordered_set<TcpConnection::Ptr> conns_;
};

}  // namespace lizlib

#endif  //LIZLIB_TCP_SERVER_H
