//
// Created by lzy on 2023/8/20.
//
#include "common/thread_pool.h"

#include <liburing.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include "common/basic.h"
#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"

using namespace lizlib;

#define QUEUE_DEPTH 32
#define BUF_SIZE 4096
#define NUM_REQUESTS 5

TEST(EPOLL_TEST, timefd_test) {
  int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, 0);

  // 设置定时器，每3秒触发一次
  struct itimerspec timer_spec;
  timer_spec.it_value.tv_sec = 1;
  timer_spec.it_value.tv_nsec = 0;
  timer_spec.it_interval.tv_sec = 3;
  timer_spec.it_interval.tv_nsec = 0;

  // 创建一个 epoll 实例
  int epoll_fd = epoll_create1(0);
  struct epoll_event event;
  event.data.fd = timer_fd;
  event.data.ptr = (void*)"asdrrr";
  event.events = EPOLLIN;

  // 将 timerfd 添加到 epoll 实例中
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event);
  timerfd_settime(timer_fd, 0, &timer_spec, nullptr);
  for (;;) {
    // 等待 epoll 事件
    epoll_event events[10];
    int num_events = epoll_wait(epoll_fd, events, 10, -1);

    if (num_events > 0) {
      fmt::println("{}", num_events);
      fmt::println("Timer expired! fd:{} events:{} ,data :{}",
                   events[0].data.fd, events[0].events,
                   (char*)events[0].data.ptr);
      epoll_ctl(epoll_fd, EPOLL_CTL_DEL, timer_fd, nullptr);

//      epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event);
    }
  }

  close(epoll_fd);
  close(timer_fd);
}
