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

class TestServerHandler : public ChannelHandlerAdaptor {
 public:
  ~TestServerHandler() override = default;
  void OnRead(ChannelContext::Ptr ctx, Timestamp now, Buffer& buffer) override {
    std::string data(buffer.RPtr(), buffer.ReadableBytes());
    fmt::println("got data:{}", data.c_str());
  }
  void OnWriteComplete(ChannelContext::Ptr ctx, Timestamp now) override {
    LOG_TRACE("{}:{}->{} write complete", ctx->GetConnection()->GetSocketChannel()->GetFile(),
              ctx->GetConnection()->GetLocalAddress(), ctx->GetConnection()->GetPeerAddress());
  }
  void OnError(ChannelContext::Ptr ctx, Timestamp now, Status err) override {
    ChannelHandlerAdaptor::OnError(ctx, now, err);
  }
  void OnConnect(ChannelContext::Ptr ctx, Timestamp now) override {
    LOG_TRACE("custom handle connection");
  }
  void OnClose(ChannelContext::Ptr ctx, Timestamp now) override {
    ChannelHandlerAdaptor::OnClose(ctx, now);
  }
};

TEST(TcpTest, testSockSize) {
  struct sockaddr a1;
  struct sockaddr_in a2;
  struct sockaddr_in6 a3;
  fmt::println("{} {} {}", sizeof(a1), sizeof(a2), sizeof(a3));
}

TEST(TcpTest, server_test_1) {

  InetAddress server_addr{"127.0.0.1", 6666, false};
  EventLoopGroup::Ptr boss = std::make_shared<EventLoopGroup>(1);
  EventLoopGroup::Ptr worker = std::make_shared<EventLoopGroup>(2);

  TcpServer server{server_addr, boss, worker, std::make_shared<TestServerHandler>()};
  server.Start();
  std::this_thread::sleep_for(2500ms);

  TcpClient client{server_addr, worker, std::make_shared<TestServerHandler>()};
  client.Start();

  client.Send("asdrrr");
  client.Send("asdrrr");
  client.Send("asdrrr");
  for (int i = 0; i < 100; i++) {
    client.Send(fmt::format("asdrrr[{}]", i));
    std::this_thread::sleep_for(1s);
  }
  std::this_thread::sleep_for(300h);
}