//
// Created by lzy on 2023/8/17.
//

#ifndef LIZLIB_SOCKET_CHANNEL_H
#define LIZLIB_SOCKET_CHANNEL_H

#include "channel.h"
#include "net/selector/selector.h"
#include "net/socket.h"
namespace lizlib {
class SocketChannel final : public Socket, Channel {
 public:
  // TODO: imple SocketChannel
  explicit SocketChannel(Socket socket) : Socket(std::move(socket)) {}
  SocketChannel(Socket socket, Selector* selector)
      : Socket(std::move(socket)), selector_(selector) {}
  ~SocketChannel() override = default;

  void OnRead(SelectorCallback read_callback) {
    read_callback_ = std::move(read_callback);
  }

  void OnClose(SelectorCallback close_callback) {
    close_callback_ = std::move(close_callback);
  }

  void OnWrite(SelectorCallback write_callback) {
    write_callback_ = std::move(write_callback);
  }

  void OnError(SelectorCallback error_callback) {
    error_callback_ = std::move(error_callback);
  }

  const File& GetFile() override;
  void HandleEvents(Events events, Timestamp now) final;
  [[nodiscard]] std::string String() const override;

 private:
  SelectEvents events_{SelectEvents::kNoneEvent};
  SelectorCallback close_callback_;
  SelectorCallback read_callback_;
  SelectorCallback error_callback_;
  SelectorCallback write_callback_;
  Selector* selector_{nullptr};
};
}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::SocketChannel);

#endif  //LIZLIB_SOCKET_CHANNEL_H
