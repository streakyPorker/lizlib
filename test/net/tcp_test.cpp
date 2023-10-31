//
// Created by A on 2023/10/31.
//
#include "common/basic.h"
#include "common/file.h"
#include "common/logger.h"
#include "concurrent/thread_pool.h"
#include "gtest/gtest.h"
#include "net/eventloop/event_loop_group.h"
#include "net/tcp/tcp_server.h"

using namespace lizlib;
using namespace std::chrono_literals;

TEST(TcpTest, server_test_1) {

  InetAddress server_addr{"127.0.0.1", 6666, false};
  EventLoopGroup::Ptr boss = std::make_shared<EventLoopGroup>(1);
  EventLoopGroup::Ptr worker = std::make_shared<EventLoopGroup>(1);

  TcpServer server{server_addr, boss, worker, std::make_shared<ChannelHandlerAdaptor>()};
  server.Start();
}