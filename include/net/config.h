//
// Created by lzy on 2023/8/18.
//

#ifndef LIZLIB_NET_CONFIG_H
#define LIZLIB_NET_CONFIG_H
namespace lizlib {

struct TcpOption {
  bool reuse_addr = true;
  bool reuse_port = false;
  bool keep_alive = true;
  bool tcp_no_delay = true;
};
static TcpOption kTcpOption{};

static uint64_t kTcpRetryConnectDelayMs = 1000L;

}  // namespace lizlib
#endif  //LIZLIB_NET_CONFIG_H
