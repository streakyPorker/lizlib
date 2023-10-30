//
// Created by bangsun on 2023/10/21.
//

#ifndef LIZLIB_CHANNEL_HANDLER_H
#define LIZLIB_CHANNEL_HANDLER_H
#include "channel_context.h"
#include "common/basic.h"
#include "common/buffer.h"
#include "common/status.h"
namespace lizlib {

class ChannelHandler {
 public:
  LIZ_CLAIM_SHARED_PTR(ChannelHandler);
  LIZ_CLAIM_UNIQUE_PTR(ChannelHandler);

  virtual ~ChannelHandler() = default;
  virtual void OnRead(ChannelContext::Ptr ctx, Timestamp now, Buffer& buffer) = 0;
  virtual void OnWriteComplete(ChannelContext::Ptr ctx, Timestamp now) = 0;
  virtual void OnError(ChannelContext::Ptr ctx, Timestamp now, Status err) = 0;
  virtual void OnConnect(ChannelContext::Ptr ctx, Timestamp now) = 0;
  virtual void OnClose(ChannelContext::Ptr ctx, Timestamp now) = 0;
};

class ChannelHandlerAdaptor : public ChannelHandler {
 public:
  using Ptr = std::shared_ptr<ChannelHandlerAdaptor>;
  ChannelHandlerAdaptor() = default;
  void OnRead(ChannelContext::Ptr ctx, Timestamp now, Buffer& buffer) override {}
  void OnWriteComplete(ChannelContext::Ptr ctx, Timestamp now) override {}
  void OnError(ChannelContext::Ptr ctx, Timestamp now, Status err) override {}
  void OnConnect(ChannelContext::Ptr ctx, Timestamp now) override {}
  void OnClose(ChannelContext::Ptr ctx, Timestamp now) override {}
};


}  // namespace lizlib
#endif  //LIZLIB_CHANNEL_HANDLER_H
