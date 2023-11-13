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

  [[nodiscard]] EventLoop* GetEventLoop() const noexcept { return worker_group_->CurrentPosLoop(); }

  void Start();
  void Close();
  void Shutdown(bool close_read = false);
  void ForceClose();
  void ForceShutdown();

  void Send(const std::string& message) {
    ASSERT_FATAL(conn_ != nullptr,"null conn");
    conn_->Send(message);
  };
  void Send(Buffer* message) { conn_->Send(message); };

 private:
  Status tryConnect(Socket client_socket);

  ChannelHandler::Ptr generateInternalHandler(const ChannelHandler::Ptr& custom_handler);

  EventLoopGroup::Ptr worker_group_;
  TcpConnection::Ptr conn_{nullptr};
  InetAddress dest_address_;

  ChannelHandler::Ptr internal_handler_{nullptr};

  std::atomic<TcpClientState> state_{TcpClientState::kOffline};
};
}  // namespace lizlib

#endif  //LIZLIB_TCP_CLIENT_H
