//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_CLIENT_H
#define LIZLIB_TCP_CLIENT_H
#include "common/basic.h"
#include "net/eventloop/event_loop_group.h"
#include "tcp_connection.h"

namespace lizlib {
class TcpClientWrapper;
enum class TcpClientState { kOffline, kConnecting, kConnected, kDisconnecting, kDisconnected };
/**
 * one connection per client
 */
class TcpClient {
 public:
  friend class TcpClientWrapper;
  LIZ_DISABLE_COPY_AND_MOVE(TcpClient);

  explicit TcpClient(InetAddress dest_address, EventLoopGroup::Ptr worker,
                     const ChannelHandler::Ptr& custom_handler)
      : dest_address_(std::move(dest_address)),
        worker_group_(std::move(worker)),
        internal_handler_(generateInternalHandler(custom_handler)) {}

  void SetGroup(EventLoopGroup::Ptr worker_group) { worker_group_ = std::move(worker_group); }

  [[nodiscard]] TcpConnection::Ptr GetConnection() const noexcept { return conn_; }

  [[nodiscard]] EventLoop* GetEventLoop() const noexcept { return eventloop_; }

  void Start();
  void Close();
  void Shutdown();
  void ForceClose();
  void ForceShutdown();

  void Send(std::string message);
  void Send(std::string_view message);
  void Send(Buffer& message);

 private:
  void tryConnect(){

  }



  ChannelHandler::Ptr generateInternalHandler(const ChannelHandler::Ptr& custom_handler);

  EventLoopGroup::Ptr worker_group_;
  TcpConnection::Ptr conn_;
  InetAddress dest_address_;

  ChannelHandler::Ptr internal_handler_{nullptr};

  std::atomic<TcpClientState> state_{TcpClientState::kOffline};

  EventLoop* eventloop_{};
};
}  // namespace lizlib

#endif  //LIZLIB_TCP_CLIENT_H
