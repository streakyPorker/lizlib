//
// Created by lzy on 2023/8/20.
//

#include "net/tcp/tcp_server.h"

#include <utility>
#include "common/basic.h"
namespace lizlib {
class TcpServerWrapper final : public ChannelHandler {
 public:
  explicit TcpServerWrapper(TcpServer* server, ChannelHandler::Ptr custom_handler)
      : server_(server), custom_handler_(std::move(custom_handler)) {}

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
    std::unique_lock lock(server_->mu_);
    server_->conns_.emplace(ctx->GetConnection()->shared_from_this());
  }

  void OnClose(ChannelContext::Ptr ctx, Timestamp now) override {
    custom_handler_->OnClose(ctx, now);
    std::unique_lock lock(server_->mu_);
    server_->conns_.erase(ctx->GetConnection()->shared_from_this());
  }

 private:
  TcpServer* server_{nullptr};
  ChannelHandler::Ptr custom_handler_;
};
}  // namespace lizlib

void lizlib::TcpServer::Start() {
  LOG_TRACE("TcpServer::Start() begin...");
  acceptor_.OnAccept([this](Socket socket) {
    LOG_TRACE("TcpServer::OnAccept({})", socket);
    socket.ApplySettingOption();
    auto conn = std::make_shared<TcpConnection>(worker_group_->Next(), std::move(socket));
    // one proxy handler for all connections
    conn->SetHandler(internal_handler_);
    conn->Start();
    ASSERT_FATAL(conn.use_count() == 2, "or the connection will be reclaimed");
  });

  acceptor_.Listen();
  LOG_TRACE("TcpServer::Start() end...");
}
lizlib::ChannelHandler::Ptr lizlib::TcpServer::generateInternalHandler(
  const ChannelHandler::Ptr& custom_handler) {
  return std::dynamic_pointer_cast<ChannelHandler>(
    std::make_shared<TcpServerWrapper>(this, custom_handler));
}
void lizlib::TcpServer::Close() {
  LOG_TRACE("TcpServer::Close() begin...");
  acceptor_.Close();
  std::unique_lock<std::mutex> unique_lock{mu_};
  for (const auto& conn : conns_) {
    conn->Close();
  }
  conns_.clear();
  LOG_TRACE("TcpServer::Close() end...");
}
