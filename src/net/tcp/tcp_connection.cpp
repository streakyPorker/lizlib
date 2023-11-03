//
// Created by lzy on 2023/8/20.
//

#include "net/tcp/tcp_connection.h"
void lizlib::TcpConnection::Start() {
  context_->SetConnection(this);
  channel_->SetReadCallback([this](auto events, auto now) { handleRead(now); });
  channel_->SetWriteCallback([this](auto events, auto now) { handleWrite(); });
  channel_->SetCloseCallback([this](auto events, auto now) { handleClose(); });
  channel_->SetErrorCallback([this](auto events, auto now) { handleError(channel_->GetError()); });
  channel_->SetSelector(loop_->GetSelector());

  // set to redirect the callback tasks to the executor, rather than run it in the epoll thread
  channel_->SetExecutor(loop_);

  /*
   * run OnConnect first, then enable handling requests
   */
  loop_->AddChannel(channel_, [self = shared_from_this()] {
    LOG_INFO("handleConnection {}", *self);
    TcpState desired = TcpState::kConnecting;

    if (self->state_.compare_exchange_strong(desired, TcpState::kConnected,
                                             std::memory_order_acq_rel)) {
      // already connected
      return;
    }
    self->handler_->OnConnect(self->GetChannelContext(), Timestamp::Now());
    self->channel_->SetReadable(true);
  });
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
  handler_->OnRead(GetChannelContext(), now, input_);
}
void lizlib::TcpConnection::handleError(lizlib::Status err) {
  handler_->OnError(GetChannelContext(), Timestamp::Now(), err);
}
void lizlib::TcpConnection::handleWrite() {
  if (!channel_->Writable()) {
    LOG_FATAL("Not allowed to write to channel[{}]", channel_->GetFile());
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
    handler_->OnWriteComplete(GetChannelContext(), Timestamp::Now());
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
  // at this time, the TcpConnection Object is not dead yet, so use this
  loop_->Submit([this]() { handleRemove(); });
}

void lizlib::TcpConnection::handleRemove() {
  ASSERT_FATAL(state_ == TcpState::kDisconnected, "remove while not disconnected");
  handler_->OnClose(GetChannelContext(), Timestamp::Now());
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
  loop_->Submit([this]() {
    if (output_.ReadableBytes() == 0) {
      handleRemove();
    }
  });
}
void lizlib::TcpConnection::Shutdown(bool close_read) {
  auto desired = TcpState::kConnected;
  if (!state_.compare_exchange_strong(desired, TcpState::kDisconnecting)) {
    LOG_WARN("Shutting down a connection while it's not connected won't help")
    return;
  }
  loop_->Submit([this, close_read]() {
    if (output_.ReadableBytes() == 0) {
      LOG_TRACE("{}::Shutdown()", *this);
      channel_->SetWritable(false);
      channel_->Shutdown(close_read);
    }
  });
}
void lizlib::TcpConnection::ForceShutdown() {
  if (state_ == TcpState::kDisconnected) {
    return;
  }
  state_ = TcpState::kDisconnecting;
  loop_->Submit([this]() {
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
  loop_->Submit([self = shared_from_this()]() { self->handleRemove(); });
}
void lizlib::TcpConnection::Send(lizlib::Buffer* buf) {
  if (EventLoop::CheckUnderLoop(loop_)) {
    if (buf == &output_) {
      if (output_.ReadableBytes()) {
        channel_->SetWritable(true);
        handleWrite();
        //        buf->Reset();
      }
      return;
    }
    handleSend({buf->RPtr(), static_cast<size_t>(buf->ReadableBytes())});
    buf->Reset();
    return;
  }
  loop_->Submit([self = shared_from_this(),
                 clone = std::string_view{buf->RPtr(), static_cast<size_t>(buf->ReadableBytes())}] {
    self->handleSend(clone);
  });
}
void lizlib::TcpConnection::Send(const std::string& buffer) {
  if (EventLoop::CheckUnderLoop(loop_)) {
    handleSend(buffer);
    return;
  }
  loop_->Submit(
    [self = shared_from_this(), clone = std::string_view(buffer)] { self->handleSend(clone); });
}

void lizlib::TcpConnection::Send(std::string_view buffer) {
  if (EventLoop::CheckUnderLoop(loop_)) {
    handleSend(buffer);
    return;
  }
  // shallow copy here sine string_view itself is a non-owning object
  loop_->Submit([self = shared_from_this(), &buffer] { self->handleSend(buffer); });
}
void lizlib::TcpConnection::handleSend(std::string_view buffer) {
  if (state_.load(std::memory_order_relaxed) != TcpState::kConnected) {
    LOG_TRACE("{}::{}: give up sending buffer", *this, __func__)
    return;
  }
  if (buffer.empty()) {
    return;
  }

  if (output_.ReadableBytes() == 0) {
    ssize_t writen_bytes = channel_->Write(buffer.data(), buffer.size());
    if (writen_bytes < 0) {
      auto err = Status::FromErr();
      if (err.Code() != EWOULDBLOCK && err.Code() != EAGAIN) {
        handleError(err);
      }
    }
    buffer = buffer.substr(std::max(0L, writen_bytes));
  }

  if (!buffer.empty()) {
    ssize_t left_bytes = output_.Append(buffer.data(), buffer.size(), false, false);
    ASSERT_FATAL(left_bytes == buffer.size(), "tcp output buffer full!");
    channel_->SetWritable(true);
  }
}
