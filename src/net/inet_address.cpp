//
// Created by A on 2023/8/16.
//

#include "net/inet_address.h"

lizlib::InetAddress::InetAddress(const std::string& host, uint16_t port, bool ipv6)
    : host_(host), ipv6_enabled_(ipv6) {
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
  return htons(ipv6_enabled_ ? impl_.in6.sin6_port : impl_.in4.sin_port);
}

lizlib::InetAddress& lizlib::InetAddress::operator=(lizlib::InetAddress&& addr) noexcept {
  host_.swap(addr.host_);
  std::swap(ipv6_enabled_, addr.ipv6_enabled_);
  std::swap(impl_, addr.impl_);
  return *this;
}
std::string lizlib::InetAddress::String() const noexcept {
  return fmt::format("InetAddress[{}:{}]", host_, Port());
}
