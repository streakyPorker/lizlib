//
// Created by lzy on 2023/8/20.
//

#include "net/tcp/tcp_server.h"
void lizlib::TcpServer::Bind(const lizlib::InetAddress& address) {
  LOG_TRACE("TcpServer::Bind");
  ASSERT_FATAL(boss_group_ != nullptr, "Boss group unset");
  acceptor_ = std::make_shared<Acceptor>(boss_group_->Next(), address);
  acceptor_->Bind();
}
