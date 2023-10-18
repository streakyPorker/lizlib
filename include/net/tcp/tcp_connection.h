//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_CONNECTION_H
#define LIZLIB_TCP_CONNECTION_H
#include "common/basic.h"
#include "common/buffer.h"
#include "concurrent/thread_pool.h"
#include "net/channel/socket_channel.h"
#include "net/eventloop/event_loop.h"

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

enum class TcpState { kConnected, kDisconnected, kConnecting, kDisconnecting };

/**
 * use shared_from_this() to retain the TcpConnection object in the EventLoop when itself goes
 * out of life cycle
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  LIZ_DISABLE_COPY_AND_MOVE(TcpConnection);
  friend class ChannelContext;

  LIZ_CLAIM_SHARED_PTR(TcpConnection);

  TcpConnection(EventLoop* loop, Socket socket)
      : channel_(std::make_shared<SocketChannel>(std::move(socket))),
        local_addr_(channel_->GetLocalAddress()),
        loop_{loop},
        context_(std::make_shared<ChannelContext>()) {
    ASSERT_FATAL(loop != nullptr, "invalid eventloop detected");
  }

  ~TcpConnection() = default;

  auto GetChannelContext() noexcept { return context_; }

  TcpState GetState() const noexcept { return state_; }

  void SetHandler(ChannelHandler::Ptr handler) { handler_ = std::move(handler); }

  const InetAddress& GetLocalAddress() const noexcept { return local_addr_; }

  InetAddress GetPeerAddress() const noexcept { return channel_->GetPeerAddress(); }

  void Start();

  void Send(Buffer* buf);

  void Send(const std::string& buffer);

  void Send(std::string_view buffer);

  /**
 * should check state to see if Close() succeeded
 */
  void Close();
  void Shutdown();
  void ForceShutdown();
  void ForceClose();

  std::string String() const;

 private:
  void handleRead(Timestamp now);
  void handleError(Status);
  void handleWrite();
  void handleClose();
  void handleRemove();
  void handleSend(std::string_view buffer);

  std::atomic<TcpState> state_{TcpState::kConnecting};

  Buffer output_;
  Buffer input_;

  SocketChannel::Ptr channel_;
  ChannelHandler::Ptr handler_;
  ChannelContext::Ptr context_;

  InetAddress local_addr_;
  EventLoop* loop_;
};
}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::TcpConnection);

#endif  //LIZLIB_TCP_CONNECTION_H
