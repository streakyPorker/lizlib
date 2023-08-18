//
// Created by A on 2023/8/16.
//

#include "net/inet_address.h"

lizlib::InetAddress::InetAddress(const std::string& host, uint16_t port,
                                 bool ipv6)
    : host_(host), ipv6_(ipv6) {
  if (ipv6) {
    impl_.in6.sin6_family = AF_INET6;
    impl_.in6.sin6_port = htons(port);
    if (::inet_pton(AF_INET6, host.c_str(), &impl_.in4.sin_addr) <= 0) {
      LOG_ERROR("wrong host format : {}", host.c_str());
    }
  } else {
    impl_.in4.sin_family = AF_INET;
    impl_.in4.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &impl_.in4.sin_addr) <= 0) {
      LOG_ERROR("wrong host format : {}", host.c_str());
    }
  }
}

int lizlib::InetAddress::Port() const noexcept {
  return htons(ipv6_ ? impl_.in6.sin6_port : impl_.in4.sin_port);
}

lizlib::InetAddress& lizlib::InetAddress::operator=(
  lizlib::InetAddress&& addr) {
  host_.swap(addr.host_);
  std::swap(ipv6_, addr.ipv6_);
  std::swap(impl_, addr.impl_);
  return *this;
}

lizlib::InetAddress& lizlib::InetAddress::operator=(
  const lizlib::InetAddress& addr) {
  host_ = addr.host_;
  ipv6_ = addr.ipv6_;
  impl_ = addr.impl_;
  return *this;
}
