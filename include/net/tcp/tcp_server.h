//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_SERVER_H
#define LIZLIB_TCP_SERVER_H
#include <unordered_set>
#include "acceptor.h"
#include "net/eventloop/event_loop_group.h"
#include "net/tcp/tcp_connection.h"
namespace lizlib {
class TcpServerChannelHandler;

/**
 * 3 step to create a TcpServer:
 * <br>1. create the object
 * <br>2. call Bind() to bind it to certain server address or the ctor with address
 * <br>3. call Start
 */
class TcpServer {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(TcpServer);
  friend class TcpServerChannelHandler;

  TcpServer() = default;
  explicit TcpServer(const InetAddress& address) : TcpServer() { Bind(address); }
  ~TcpServer() { Close(); }

  void SetGroup(EventLoopGroup::Ptr boss, EventLoopGroup::Ptr worker) {
    boss_group_ = std::move(boss);
    worker_group_ = std::move(worker);
  }

  /**
   * 1. check eventloopgroup setting
   * 2. init acceptor
   * 3. bind the acceptor to server address
   * @param address server address
   */
  void Bind(const InetAddress& address);

  void Start();
  void Close();

  void SetBuilder(ChannelBuilder handler) { builder_ = std::move(handler); }

 private:
  EventLoopGroup::Ptr boss_group_;
  EventLoopGroup::Ptr worker_group_;

  ChannelBuilder builder_;

  std::mutex mu_;
  Acceptor::UniPtr acceptor_;
  std::unordered_set<TcpConnection::Ptr> conns_;
};

}  // namespace lizlib

#endif  //LIZLIB_TCP_SERVER_H
