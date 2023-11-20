//
// Created by bangsun on 2023/10/13.
//

#include "net/tcp/acceptor.h"
#include "concurrent/countdown_latch.h"
#include "net/eventloop/event_loop.h"
lizlib::Acceptor::Acceptor(lizlib::EventLoop* eventloop, const lizlib::InetAddress& address)
    : eventloop_(eventloop),
      server_address_(address),
      socket_channel_(createServerChannel(address)) {
  LOG_TRACE("Initializing Acceptor...");
  socket_channel_->SetExecutor(eventloop_);
  socket_channel_->SetSelector(eventloop_->GetSelector());
  socket_channel_->SetReadCallback([this](const ReceiveEvents& events, Timestamp now) {
    while (true) {
      Socket socket;
      InetAddress remote;
      auto err = socket_channel_->Accept(&remote, &socket);
      if (!err.OK()) {
        if (err.Code() == EAGAIN || err.Code() == EWOULDBLOCK) {
          LOG_INFO("need to try again : {}", err);
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
  return fmt::format("Acceptor[socket={}]", socket_channel_->GetFile());
}
void lizlib::Acceptor::Listen() {
  // clang-format off
  eventloop_->AddChannel(socket_channel_,
    [this] { socket_channel_->Listen(); },
    SelectEvents::kReadEvent.EdgeTrigger());
  // clang-format on
}
void lizlib::Acceptor::Close() {
  LOG_TRACE("Acceptor::Close() enter");
  CountdownLatch latch{1};
  eventloop_->RemoveChannel(socket_channel_, [&] { latch.CountDown(); });
  latch.Await();
  LOG_TRACE("Acceptor::Close() exit");
}
