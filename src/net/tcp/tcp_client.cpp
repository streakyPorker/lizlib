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
  eventloop_ = worker_group_->Next();

  Socket client_socket = Socket::Create(dest_address_.Family(), true);
  client_socket.ApplySettingOption();
  Status status = client_socket.Connect(dest_address_);
  //  if (status.OK()) {
  //    auto conn = std::make_shared<TcpConnection>(worker_group_->Next(), std::move(client_socket));
  //    conn->Start();
  //    conn_ = conn;
  //    internal_handler_->OnConnect(conn_->GetChannelContext(), Timestamp::Now());
  //  }

  switch (status.Code()) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN: {
      conn_ = std::make_shared<TcpConnection>(worker_group_->Next(), std::move(client_socket));
      conn_->Start();
      internal_handler_->OnConnect(conn_->GetChannelContext(), Timestamp::Now());
      return;
    }

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      //      retry(std::move(socket), err);
      return;

    case EACCES:
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
}
