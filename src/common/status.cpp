//
// Created by A on 2023/8/16.
//

#include "common/status.h"
const char* lizlib::Status::getReason() const noexcept {
  thread_local char buf[1024];
  return strerror_r(code_, buf, sizeof(buf));
}
