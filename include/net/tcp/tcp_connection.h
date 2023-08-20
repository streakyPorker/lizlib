//
// Created by lzy on 2023/8/20.
//

#ifndef LIZLIB_TCP_CONNECTION_H
#define LIZLIB_TCP_CONNECTION_H
#include "common/basic.h"

namespace lizlib {
class TcpConnection {
 public:
  DISABLE_COPY_AND_MOVE(TcpConnection);
  enum class ConnState { kConnected, kDisconnected, kConnecting, kDisconnecting };



};
}  // namespace lizlib

#endif  //LIZLIB_TCP_CONNECTION_H
