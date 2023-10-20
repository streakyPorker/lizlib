//
// Created by bangsun on 2023/10/13.
//

#include "net/tcp/acceptor.h"
#include "concurrent/countdown_latch.h"
#include "net/eventloop/event_loop.h"
lizlib::Acceptor::Acceptor(lizlib::EventLoop* eventloop, const lizlib::InetAddress& address)
    : eventloop_(eventloop),
      address_(address),
      channel_(std::make_shared<SocketChannel>(Socket::Create(address.Family(), true))) {
  LOG_TRACE("Initializing Acceptor...");
  channel_->SetExecutor(eventloop_);
  channel_->SetSelector(eventloop_->GetSelector());
  channel_->OnRead([this](const ReceiveEvents& events, Timestamp now) {
    while (true) {
      LOG_TRACE("{}::handleRead()", *this);
      Socket socket;
      InetAddress remote;
      auto err = channel_->Accept(&remote, &socket);
      if (!err.OK()) {
        if (err.Code() == EAGAIN || err.Code() == EWOULDBLOCK) {
          break;
        }
        LOG_ERROR("failed to accept [{}]", err);
        continue;
      }
      if (acceptor_callback_) {
        acceptor_callback_(std::move(socket));
      }
    }
  });
}
std::string lizlib::Acceptor::String() const {
  return fmt::format("Acceptor[socket={}]", channel_->GetFile());
}
void lizlib::Acceptor::Listen() {
  eventloop_->AddChannel(
    channel_,
    [this] {
      channel_->SetReadable(true);
      channel_->Listen();
    },
    false);
}
void lizlib::Acceptor::Close() {
  LOG_TRACE("Acceptor::Close() enter");
  CountdownLatch latch{1};
  eventloop_->RemoveChannel(channel_, [&] { latch.CountDown(); });
  latch.Await();
  LOG_TRACE("Acceptor::Close() exit");
}
