//
// Created by lzy on 2023/8/20.
//

#include "net/tcp/tcp_server.h"

#include <utility>
#include "common/basic.h"
namespace lizlib {
class TcpServerChannelHandler final : public ChannelHandler {
 public:
  explicit TcpServerChannelHandler(ChannelContext::Ptr ctx, TcpServer* server)
      : ctx_(std::move(ctx)), server_(server) {
    custom_handler_ = server_->builder_(ctx_);
  }

  void OnRead(Timestamp now, Buffer& buffer) override { custom_handler_->OnRead(now, buffer); }

  void OnWriteComplete(Timestamp now) override { custom_handler_->OnWriteComplete(now); }

  void OnError(Timestamp now, Status err) override { custom_handler_->OnError(now, err); }

  void OnConnect(Timestamp now) override {
    custom_handler_->OnConnect(now);
    std::unique_lock lock(server_->mu_);
    server_->conns_.emplace(ctx_->GetConnection()->shared_from_this());
  }

  void OnClose(Timestamp now) override {
    custom_handler_->OnClose(now);
    std::unique_lock lock(server_->mu_);
    server_->conns_.erase(ctx_->GetConnection()->shared_from_this());
  }

 private:
  TcpServer* server_;
  ChannelContext::Ptr ctx_;
  std::shared_ptr<ChannelHandler> custom_handler_;
};
}  // namespace lizlib

void lizlib::TcpServer::Bind(const lizlib::InetAddress& address) {
  LOG_TRACE("TcpServer::Bind");
  ASSERT_FATAL(boss_group_ != nullptr, "Boss group unset");
  acceptor_ = std::make_unique<Acceptor>(boss_group_->Next(), address);
  acceptor_->Bind();
}
void lizlib::TcpServer::Start() {
  LOG_TRACE("TcpServer::Start() begin...");
  acceptor_->OnAccept([this](Socket socket) {
    LOG_TRACE("TcpServer::OnAccept({})", socket);
    socket.ApplySettingOption();
    auto conn = std::make_shared<TcpConnection>(worker_group_->Next(), std::move(socket));
    conn->SetHandler(std::make_shared<TcpServerChannelHandler>(conn->GetChannelContext(), this));
    conn->Start();
  });
}
