//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_CONNECTION_H
#define LIZLIB_TCP_CONNECTION_H
#include "net/channel/socket_channel.h"
#include "net/eventloop/event_loop.h"
#include "net/tcp/channel_context.h"
#include "net/tcp/channel_handler.h"

namespace lizlib {

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
        // conn_ will be set on Start()
        context_(std::make_shared<ChannelContext>()) {
    ASSERT_FATAL(loop != nullptr, "invalid eventloop detected");
  }

  ~TcpConnection() { ForceClose(); };

  auto GetChannelContext() noexcept { return context_; }

  TcpState GetState() const noexcept { return state_; }

  void SetHandler(ChannelHandler::Ptr handler) { handler_ = std::move(handler); }

  const InetAddress& GetLocalAddress() const noexcept { return local_addr_; }

  InetAddress GetPeerAddress() const noexcept { return channel_->GetPeerAddress(); }

  void Start();

  void Send(Buffer* buf, bool flush = true);

  void Send(const std::string& buffer, bool flush = true);

  void Send(std::string_view buffer, bool flush = true);

  void Flush() {}

  SocketChannel::Ptr GetSocketChannel() { return channel_; };

  /**
 * should check state to see if Close() succeeded
 */
  void Close();
  /**
   * Shutdown first, then Close
   */
  void Shutdown(bool close_read);
  void ForceShutdown();
  void ForceClose();

  std::string String() const;

 private:
  void handleRead(Timestamp now);
  void handleError(Status);
  void handleWrite();
  void handleClose();
  void handleRemove();
  void handleSend(std::string_view buffer,bool flush);

  std::atomic<TcpState> state_{TcpState::kConnecting};

  Buffer output_{1 << 14, true};
  Buffer input_{1 << 14, true};

  SocketChannel::Ptr channel_;
  ChannelHandler::Ptr handler_;
  ChannelContext::Ptr context_;

  InetAddress local_addr_;
  EventLoop* loop_;
};
}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::TcpConnection);

#endif  //LIZLIB_TCP_CONNECTION_H
