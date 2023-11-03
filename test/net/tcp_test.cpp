//
// Created by A on 2023/10/31.
//
#include "common/basic.h"
#include "common/file.h"
#include "common/logger.h"
#include "concurrent/thread_pool.h"
#include "gtest/gtest.h"
#include "net/eventloop/event_loop_group.h"
#include "net/tcp/tcp_client.h"
#include "net/tcp/tcp_server.h"

using namespace lizlib;
using namespace std::chrono_literals;

TEST(TcpTest, server_test_1) {

  InetAddress server_addr{"127.0.0.1", 6666, false};
  EventLoopGroup::Ptr boss = std::make_shared<EventLoopGroup>(1);
  EventLoopGroup::Ptr worker = std::make_shared<EventLoopGroup>(2);

  TcpServer server{server_addr, boss, worker, std::make_shared<ChannelHandlerAdaptor>()};
  server.Start();
  std::this_thread::sleep_for(3s);

  TcpClient client{server_addr, worker, std::make_shared<ChannelHandlerAdaptor>()};
  client.Start();
  client.Send("asdrrr");
  client.Send("asdrrr");
  std::this_thread::sleep_for(30s);
}