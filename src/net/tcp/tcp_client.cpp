//
// Created by lzy on 2023/8/20.
//

#include "net/tcp/tcp_client.h"

namespace lizlib {
class TcpClientWrapper final : public ChannelHandler {
 public:
  explicit TcpClientWrapper(TcpClient* client, ChannelHandler::Ptr custom_handler)
      : client_(client), custom_handler_(std::move(custom_handler)) {}

  void OnRead(ChannelContext::Ptr ctx, Timestamp now, Buffer& buffer) override {
    custom_handler_->OnRead(ctx, now, buffer);
  }

  void OnWriteComplete(ChannelContext::Ptr ctx, Timestamp now) override {
    custom_handler_->OnWriteComplete(ctx, now);
  }

  void OnError(ChannelContext::Ptr ctx, Timestamp now, Status err) override {
    custom_handler_->OnError(ctx, now, err);
  }

  void OnConnect(ChannelContext::Ptr ctx, Timestamp now) override {
    custom_handler_->OnConnect(ctx, now);
  }

  void OnClose(ChannelContext::Ptr ctx, Timestamp now) override {
    custom_handler_->OnClose(ctx, now);
    client_->state_ = TcpClientState::kDisconnected;
    client_->conn_.reset();
  }

 private:
  TcpClient* client_{nullptr};
  ChannelHandler::Ptr custom_handler_;
};
}  // namespace lizlib

lizlib::ChannelHandler::Ptr lizlib::TcpClient::generateInternalHandler(
  const lizlib::ChannelHandler::Ptr& custom_handler) {
  return std::dynamic_pointer_cast<ChannelHandler>(
    std::make_shared<TcpClientWrapper>(this, custom_handler));
}
void lizlib::TcpClient::Start() {
  LOG_TRACE("TcpClient::Start() begin...");
  TcpClientState desired = TcpClientState::kOffline;
  if (!state_.compare_exchange_strong(desired, TcpClientState::kConnecting)) {
    return;
  }

  Socket client_socket = Socket::Create(dest_address_.Family(), false);
  client_socket.ApplySettingOption();
  tryConnect(std::move(client_socket));
  LOG_TRACE("TcpClient::Start() end...");
}

lizlib::Status lizlib::TcpClient::tryConnect(Socket client_socket) {
  LOG_TRACE("try connecting to {}", client_socket);
  Status status = client_socket.Connect(dest_address_);
  switch (status.Code()) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN: {
      auto desired = TcpClientState::kConnecting;
      if (!state_.compare_exchange_strong(desired, TcpClientState::kConnected,
                                          std::memory_order_acq_rel)) {
        return status;
      }
      conn_ = std::make_shared<TcpConnection>(worker_group_->Next(), std::move(client_socket));
      conn_->SetHandler(internal_handler_);
      conn_->Start();
      return status;
    }
    case EAGAIN:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      LOG_TRACE("need to retry connection to {}, reason:{}", client_socket, status);
      //      worker_group_->SubmitAfter(
      //        [this, &client_socket]() mutable {
      //          LOG_TRACE("connect op retrying");
      //          Socket own_sock = std::move(client_socket);
      //          tryConnect(std::move(own_sock));
      //        },
      //        Duration::FromMilliSecs(config::kTcpRetryConnectDelayMs));
      return tryConnect(std::move(client_socket));

    case EACCES:
    case EADDRINUSE:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_ERROR("known error: {}", status);
      break;
    default:
      LOG_ERROR("unexpected connection error: {}", status);
      break;
  }
  return status;
}
void lizlib::TcpClient::Close() {
  auto desired = TcpClientState::kConnected;
  if (state_.compare_exchange_strong(desired, TcpClientState::kDisconnecting)) {
    return;
  }
  if (conn_) {
    conn_->Close();  // once connection closed, stata_ will be set to disconnected in OnClose
  }
}
void lizlib::TcpClient::Shutdown(bool close_read) {
  auto desired = TcpClientState::kConnected;
  if (state_.compare_exchange_strong(desired, TcpClientState::kDisconnecting)) {
    return;
  }
  if (conn_) {
    conn_->Shutdown(close_read);
  }
}
