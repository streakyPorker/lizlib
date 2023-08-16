//
// Created by A on 2023/8/16.
//

#ifndef LIZLIB_SOCKET_H
#define LIZLIB_SOCKET_H

#include <netinet/in.h>
#include "common/basic.h"
#include "common/file.h"

namespace lizlib {

class Socket : public File {
 public:
  Socket() : Socket(-1) {}
  Socket(Socket&& other) noexcept : File(std::forward<File>(other)) {}
  Socket& operator=(Socket&& other) noexcept;

  static Socket Create(int domain, bool nonblock, int protocol = IPPROTO_TCP);



 private:
  explicit Socket(int fd) : File(fd) {}
};

}  // namespace lizlib

#endif  //LIZLIB_SOCKET_H
