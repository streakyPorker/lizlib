//
// Created by lzy on 2023/8/17.
//

#ifndef LIZLIB_CALLBACKS_H
#define LIZLIB_CALLBACKS_H
#include "common/buffer.h"
namespace lizlib {
class TcpConnection;
class ReceiveEvents;
class SelectEvents;
class Channel;

using Callback = std::function<void()>;
using SelectorCallback = std::function<void(ReceiveEvents events, Timestamp)>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using MessageCallback =
  std::function<void(const TcpConnectionPtr&, Buffer&, Timestamp)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWatermarkCallback =
  std::function<void(const TcpConnectionPtr&, size_t)>;
using ErrorCallback = std::function<void(const TcpConnectionPtr&, Status)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;


}  // namespace lizlib
#endif  //LIZLIB_CALLBACKS_H
