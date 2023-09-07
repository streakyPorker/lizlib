//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_CONNECTION_H
#define LIZLIB_TCP_CONNECTION_H
#include "common/basic.h"
#include "common/buffer.h"
#include "net/channel/socket_channel.h"
namespace lizlib {
class TcpConnection;

class ChannelContext {
  friend class TcpConnection;

 public:
  LIZ_CLAIM_SHARED_PTR(ChannelContext);

  TcpConnection* GetConnection() { return conn_; }
  Buffer* GetOutputBuffer();
  Buffer* GetInputBuffer();

 private:
  TcpConnection* conn_{};
};

struct ChannelHandler {
  LIZ_CLAIM_SHARED_PTR(ChannelHandler);

  virtual ~ChannelHandler() = default;
  virtual void OnRead(Timestamp now, Buffer& buffer) = 0;
  virtual void OnWriteComplete(Timestamp now) = 0;
  virtual void OnError(Timestamp now, Status err) = 0;
  virtual void OnConnect(Timestamp now) = 0;
  virtual void OnClose(Timestamp now) = 0;
};

using ChannelBuilder = std::function<ChannelHandler::Ptr(ChannelContext::Ptr)>;

class ChannelHandlerAdaptor : public ChannelHandler {
 public:
  using Ptr = std::shared_ptr<ChannelHandlerAdaptor>;

  explicit ChannelHandlerAdaptor(ChannelContext::Ptr ctx) : ctx_(std::move(ctx)) {}

  void OnRead(Timestamp now, Buffer& buffer) override {}
  void OnWriteComplete(Timestamp now) override {}
  void OnError(Timestamp now, Status err) override {}
  void OnConnect(Timestamp now) override {}
  void OnClose(Timestamp now) override {}
  auto GetConnection() { return ctx_->GetConnection(); }
  ChannelContext& GetContext() { return *ctx_; }

 private:
  ChannelContext::Ptr ctx_;
};

class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(TcpConnection);
  friend class ChannelContext;
  enum class ConnState { kConnected, kDisconnected, kConnecting, kDisconnecting };

  LIZ_CLAIM_SHARED_PTR(TcpConnection);

  TcpConnection(EventLoop& event_loop, Socket socket)
      : channel_(std::make_shared<SocketChannel>(std::move(socket))),
        local_addr_(channel_->GetLocalAddress()),
        event_loop_(event_loop),
        context_(std::make_shared<ChannelContext>()) {}

 private:
  std::atomic<ConnState> state_{ConnState::kConnecting};

  Buffer output_;
  Buffer input_;

  SocketChannel::Ptr channel_;
  ChannelHandler::Ptr handler_;
  ChannelContext::Ptr context_;

  InetAddress local_addr_;
  EventLoop& event_loop_;
};
}  // namespace lizlib

#endif  //LIZLIB_TCP_CONNECTION_H
