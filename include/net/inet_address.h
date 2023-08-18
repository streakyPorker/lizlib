//
// Created by lzy on 2023/8/16.
//

#ifndef LIZLIB_INET_ADDRESS_H
#define LIZLIB_INET_ADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include "common/basic.h"
#include "common/logger.h"
#include "common/slice.h"
namespace lizlib {

class InetAddress {
 public:
  InetAddress() = default;
  InetAddress(const std::string& host, uint16_t port, bool ipv6 = false);
  InetAddress(const InetAddress& addr) = default;

  InetAddress(InetAddress&& addr) noexcept
      : host_(std::move(addr.host_)), ipv6_(addr.ipv6_) {
    std::swap(impl_, addr.impl_);
  }

  ~InetAddress() = default;

  InetAddress& operator=(InetAddress&& addr);
  InetAddress& operator=(const InetAddress& addr);

  [[nodiscard]] bool Ipv6() const noexcept { return ipv6_; }
  [[nodiscard]] int Family() const noexcept {
    return ipv6_ ? AF_INET6 : AF_INET;
  }

  [[nodiscard]] const std::string& Host() const noexcept { return host_; }
  /**
   * the returned value will meet the platform's endian
   * @return
   */
  [[nodiscard]] int Port() const noexcept;

  [[nodiscard]] std::string String() const noexcept {
    return fmt::format("InetAddress[{}:{}]", host_, Port());
  }

  [[nodiscard]] sockaddr* Data() { return reinterpret_cast<sockaddr*>(&impl_); }
  [[nodiscard]] const sockaddr* Data() const {
    return reinterpret_cast<const sockaddr*>(&impl_);
  }
  [[nodiscard]] socklen_t Length() const { return sizeof(impl_); }

 private:
  /**
 * what need to be noted that net processing in linux is set to use big
 * endian format, so the stuffed data need to be transformed first, namely
 * htobexx
 */
  union InetImpl {
    struct sockaddr_in6 in6;
    struct sockaddr_in in4;
  };

  friend class Socket;
  std::string host_;
  bool ipv6_;
  InetImpl impl_{};
};

}  // namespace lizlib

FORMATTER_REGISTRY(lizlib::InetAddress);

#endif  //LIZLIB_INET_ADDRESS_H
