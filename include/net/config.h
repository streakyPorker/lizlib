//
// Created by lzy on 2023/8/18.
//

#ifndef LIZLIB_NET_CONFIG_H
#define LIZLIB_NET_CONFIG_H
namespace lizlib::config {

struct TcpOption {
  bool reuse_addr = true;
  bool reuse_port = true;
  bool keep_alive = true;
  bool tcp_no_delay = true;
};
static TcpOption kTcpOption{};

static uint64_t kTcpRetryConnectDelayMs = 1000L;

static ssize_t kTcpRwUnit = 16384L;

}  // namespace lizlib::config
#endif  //LIZLIB_NET_CONFIG_H
