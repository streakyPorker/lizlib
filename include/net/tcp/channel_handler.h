//
// Created by bangsun on 2023/10/21.
//

#ifndef LIZLIB_CHANNEL_HANDLER_H
#define LIZLIB_CHANNEL_HANDLER_H
#include "common/basic.h"
#include "common/buffer.h"
#include "common/status.h"
namespace lizlib {
struct ChannelHandler {
  LIZ_CLAIM_SHARED_PTR(ChannelHandler);
  LIZ_CLAIM_UNIQUE_PTR(ChannelHandler);

  virtual ~ChannelHandler() = default;
  virtual void OnRead(Timestamp now, Buffer& buffer) = 0;
  virtual void OnWriteComplete(Timestamp now) = 0;
  virtual void OnError(Timestamp now, Status err) = 0;
  virtual void OnConnect(Timestamp now) = 0;
  virtual void OnClose(Timestamp now) = 0;
};
}  // namespace lizlib
#endif  //LIZLIB_CHANNEL_HANDLER_H
