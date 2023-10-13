//
// Created by lzy on 2023/8/17.
//

#ifndef LIZLIB_EPOLL_SELECTOR_H
#define LIZLIB_EPOLL_SELECTOR_H

#include <sys/epoll.h>
#include "common/file.h"
#include "net/channel/channel.h"
#include "selector.h"
namespace lizlib {
class EpollSelector final : public Selector {
 public:
  LIZ_CLAIM_SHARED_PTR(EpollSelector);
  explicit EpollSelector(size_t buf_size) : fd_{createEpollFd()}, epoll_events_{buf_size} {}
  void Add(const Channel::Ptr& channel, SelectEvents events) override;
  void Remove(const Channel::Ptr& channel) override;
  void Update(const Channel::Ptr& channel, SelectEvents events) override;
  Status Wait(Duration timeout, SelectChannels* selected) override;
  size_t Size() override;

  [[nodiscard]] std::string String() const noexcept {
    return fmt::format("EpollSelector[fd={},size={}]", fd_, epoll_events_.size());
  }

  ~EpollSelector() override {
    ASSERT_FATAL(::close(fd_) == 0, "epoll selector failed to close : {}", Status::FromErr());
    LOG_TRACE("EpollSelector closed");
  }

 private:
  int fd_{-1};
  std::vector<struct epoll_event> epoll_events_;
  void internalUpdate(Channel*, int epoll_op, const SelectEvents& select_events);

  static int createEpollFd() {
    int epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
      LOG_FATAL("create epoll fd failed : {}", Status::FromErr());
    }
    return epoll_fd;
  }
};
}  // namespace lizlib

LIZ_FORMATTER_REGISTRY(lizlib::EpollSelector);

#endif  //LIZLIB_EPOLL_SELECTOR_H
