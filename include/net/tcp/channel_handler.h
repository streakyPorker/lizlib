//
// Created by bangsun on 2023/10/21.
//

#ifndef LIZLIB_CHANNEL_HANDLER_H
#define LIZLIB_CHANNEL_HANDLER_H
#include "common/basic.h"
#include "common/buffer.h"
#include "common/status.h"
#include "channel_context.h"
namespace lizlib {

class ChannelHandler {
 public:
  LIZ_CLAIM_SHARED_PTR(ChannelHandler);
  LIZ_CLAIM_UNIQUE_PTR(ChannelHandler);

  virtual ~ChannelHandler() = default;
  virtual void OnRead(ChannelContext::Ptr ctx,Timestamp now, Buffer& buffer) = 0;
  virtual void OnWriteComplete(ChannelContext::Ptr ctx,Timestamp now) = 0;
  virtual void OnError(ChannelContext::Ptr ctx,Timestamp now, Status err) = 0;
  virtual void OnConnect(ChannelContext::Ptr ctx,Timestamp now) = 0;
  virtual void OnClose(ChannelContext::Ptr ctx,Timestamp now) = 0;
};
}  // namespace lizlib
#endif  //LIZLIB_CHANNEL_HANDLER_H
