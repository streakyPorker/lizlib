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
class TcpServerWrapper;


class TcpServer {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(TcpServer);
  friend class TcpServerWrapper;

  /**
   * Ctor of a TcpServer
   * @param address server addr
   * @param boss eventloops that distributes connections to workers;
   * @param worker eventloops that process the connection events
   * @param custom_handler reactive style of webserver
   */
  TcpServer(const InetAddress& address, EventLoopGroup::Ptr boss, EventLoopGroup::Ptr worker,
            const ChannelHandler::Ptr& custom_handler)
      : boss_group_{std::move(boss)},
        worker_group_{std::move(worker)},
        acceptor_{std::move(std::make_unique<Acceptor>(boss_group_->Next(), address))},
        internal_handler_{generateInternalHandler(custom_handler)} {
    LOG_TRACE("TcpServer init...");
    acceptor_->Bind();
  }

  ~TcpServer() { Close(); }

  /**
   * 1. check eventloopgroup setting
   * 2. init acceptor
   * 3. bind the acceptor to server address
   * @param address server address
   */

  void Start();
  void Close();



 private:
  ChannelHandler::Ptr generateInternalHandler(const ChannelHandler::Ptr& custom_handler);

  const EventLoopGroup::Ptr boss_group_;
  const EventLoopGroup::Ptr worker_group_;

  ChannelHandler::Ptr internal_handler_{nullptr};

  std::mutex mu_;
  const Acceptor::Ptr acceptor_;
  std::unordered_set<TcpConnection::Ptr> conns_;
};

}  // namespace lizlib

#endif  //LIZLIB_TCP_SERVER_H
