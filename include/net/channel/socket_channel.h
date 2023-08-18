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
  explicit SocketChannel(Socket socket) : Socket(std::move(socket)) {}
  SocketChannel(Socket socket, Selector* selector)
      : Socket(std::move(socket)), selector_(selector) {}
  ~SocketChannel() override = default;

  void SetSelector(Selector* aSelector) { selector_ = aSelector; }

  /**
   * will perform updates on selector_ if needed
   * @param on
   */
  void SetReadable(bool on);

  bool Readable() { return events_.Contains(SelectEvents::kReadEvent); }

  bool Writable() { return events_.Contains(SelectEvents::kWriteEvent); }

  /**
   * will perform updates on selector_ if needed
   * @param on
   */
  void SetWritable(bool on);

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
  /**
   * event process rules of socket channel:
   * <li>on one of readable/pri-readable/read-hangup, trigger read callback</li>
   * <li>on writable, trigger write callback</li>
   * <li>on kError, trigger error callback</li>
   * <li>on hangup and not readable, this means the other side has closed
   * the connection FOR GOOD, trigger close callback</li>
   */
  void HandleEvents(ReceiveEvents events, Timestamp now) final;
  [[nodiscard]] std::string String() const override;

 private:
  SelectEvents events_{SelectEvents::kNoneEvent};
  SelectorCallback close_callback_{nullptr};
  SelectorCallback read_callback_{nullptr};
  SelectorCallback error_callback_{nullptr};
  SelectorCallback write_callback_{nullptr};
  Selector* selector_{nullptr};
};
}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::SocketChannel);

#endif  //LIZLIB_SOCKET_CHANNEL_H
