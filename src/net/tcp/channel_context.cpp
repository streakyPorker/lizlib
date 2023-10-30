//
// Created by A on 2023/10/30.
//
#include "net/tcp/channel_context.h"
#include "net/tcp/tcp_connection.h"
lizlib::Buffer& lizlib::ChannelContext::GetOutputBuffer() {
  return conn_->output_;
}
lizlib::Buffer& lizlib::ChannelContext::GetInputBuffer() {
  return conn_->input_;
}
