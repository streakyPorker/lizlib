//
// Created by lzy on 2023/8/20.
//

#include "net/tcp/tcp_connection.h"
void lizlib::TcpConnection::Start() {
  context_->conn_ = this;
  channel_->OnRead([this](auto events, auto now) { handleRead(now); });
  channel_->OnWrite([this](auto events, auto now) { handleWrite(); });
  channel_->OnClose([this](auto events, auto now) { handleClose(); });
  channel_->OnError([this](auto events, auto now) { handleError(channel_->GetError()); });

  // where to get selector
  channel_->SetSelector();

}
void lizlib::TcpConnection::handleRead(lizlib::Timestamp now) {
  ssize_t n = input_.Append(&channel_->GetFile());
  if (n < 0) {
    auto err = Status::FromErr();
    if (err.Code() != EWOULDBLOCK && err.Code() != EAGAIN) {
      handleError(err);
    }
    return;
  }
  if (n == 0) {
    LOG_INFO("TcpConnection closed");
    handleClose();
    return;
  }
  handler_->OnRead(now, input_);
}
void lizlib::TcpConnection::handleError(lizlib::Status err) {
  handler_->OnError(Timestamp::Now(), err);
}
void lizlib::TcpConnection::handleWrite() {
  if (!channel_->Writable()) {
    LOG_WARN("Not allowed to write to channel[{}]", channel_->GetFile());
    return;
  }
  if (output_.ReadableBytes() > 0) {
    ssize_t n = channel_->Write(output_.RPtr(), output_.ReadableBytes());
    if (n < 0) {
      handleError(Status::FromErr());
      return;
    }
    output_.Retrieve(n);
  }

  if (output_.ReadableBytes() == 0) {
    channel_->SetWritable(false);
    handler_->OnWriteComplete(Timestamp::Now());
    if (state_ == TcpState::kDisconnecting) {
      channel_->Shutdown(false);
    }
  }
}
void lizlib::TcpConnection::handleClose() {
  if (state_ == TcpState::kDisconnected) {
    return;
  }
  LOG_INFO("{}::handleClose()", *this);
  state_.store(TcpState::kDisconnected, std::memory_order_release);
  // is it meaningful to do register&remove?
  executor_->Submit([this]() { handleRemove(); });
}

void lizlib::TcpConnection::handleRemove() {
  if (state_ != TcpState::kDisconnected) {
    LOG_FATAL("only remove when disconnected");
    return;
  }
  handler_->OnClose(Timestamp::Now());
  context_->conn_ = nullptr;
}

std::string lizlib::TcpConnection::String() const {
  return fmt::format("TcpConnection[local={}, peer={}, channel={}]", local_addr_, GetPeerAddress(),
                     *channel_);
}
void lizlib::TcpConnection::Close() {
  auto desired = TcpState::kConnected;
  if (!state_.compare_exchange_strong(desired, TcpState::kDisconnected)) {
    return;
  }
  executor_->Submit([this]() {
    if (output_.ReadableBytes() == 0) {
      handleRemove();
    }
  });
}
void lizlib::TcpConnection::Shutdown() {
  auto desired = TcpState::kConnected;
  if (!state_.compare_exchange_strong(desired, TcpState::kDisconnecting)) {
    return;
  }
  executor_->Submit([this]() {
    if (output_.ReadableBytes() == 0) {
      LOG_TRACE("{}::Shutdown()", *this);
      channel_->SetWritable(false);
      channel_->Shutdown(false);
    }
  });
}
void lizlib::TcpConnection::ForceShutdown() {
  if (state_ == TcpState::kDisconnected) {
    return;
  }
  state_ = TcpState::kDisconnecting;
  executor_->Submit([this]() {
    LOG_TRACE("{}::ForceShutdown()", *this);
    channel_->SetWritable(false);
    channel_->Shutdown(false);
  });
}
void lizlib::TcpConnection::ForceClose() {
  if (state_ == TcpState::kDisconnected) {
    return;
  }
  state_ = TcpState::kDisconnected;
  executor_->Submit([this]() { handleRemove(); });
}
