//
// Created by A on 2023/10/30.
//

#ifndef LIZLIB_CHANNEL_CONTEXT_H
#define LIZLIB_CHANNEL_CONTEXT_H
#include "common/basic.h"
#include "common/buffer.h"

namespace lizlib {
class TcpConnection;

class ChannelContext {
  friend class TcpConnection;

 public:
  LIZ_CLAIM_SHARED_PTR(ChannelContext);
  explicit ChannelContext(TcpConnection* conn = nullptr) : conn_{conn} {}
  TcpConnection* GetConnection() { return conn_; }
  void SetConnection(TcpConnection* conn) { conn_ = conn; }
  Buffer& GetOutputBuffer();
  Buffer& GetInputBuffer();

 private:
  TcpConnection* conn_{};
};
}  // namespace lizlib

#endif  //LIZLIB_CHANNEL_CONTEXT_H
