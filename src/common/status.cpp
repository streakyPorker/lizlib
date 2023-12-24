//
// Created by A on 2023/8/16.
//

#include "common/status.h"
std::string lizlib::Status::getReason(int code) noexcept {
  thread_local char buf[1024];
  return strerror_r(code, buf, sizeof(buf));
}
std::string lizlib::Status::String() const {
  return fmt::format("Error[code:{}, reason:{}]", code_, reason_);
}
