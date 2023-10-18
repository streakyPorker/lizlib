//
// Created by lzy on 2023/8/20.
//
#include "concurrent/thread_pool.h"

#include <liburing.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "common/basic.h"
#include "common/buffer.h"
#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"
#include "net/selector/epoll_selector.h"

using namespace lizlib;

#define QUEUE_DEPTH 32
#define BUF_SIZE 4096
#define NUM_REQUESTS 5

TEST(EPOLL_TEST, timefd_test) {
  int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

  struct itimerspec timer_spec;
  timer_spec.it_value.tv_sec = 1;
  timer_spec.it_value.tv_nsec = 0;
  timer_spec.it_interval.tv_sec = 0;
  timer_spec.it_interval.tv_nsec = 0;

  EpollSelector epollSelector{512};

  // 创建一个 epoll 实例
  int epoll_fd = epoll_create1(0);
  struct epoll_event event;
  event.data.fd = -1;
  event.data.ptr = &timer_fd;
  event.events = EPOLLIN | EPOLLET;

  // 将 timerfd 添加到 epoll 实例中
  ASSERT_EQ(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event), 0);
  timerfd_settime(timer_fd, 0, &timer_spec, nullptr);
  for (int i = 0; i < 5; i++) {
    // 等待 epoll 事件
    epoll_event events[10];
    int num_events = epoll_wait(epoll_fd, events, 10, -1);

    if (num_events > 0) {
      fmt::println("Timer expired! fd:{} events_:{}", events[0].data.fd, events[0].events);
      char buf[10];
      fmt::println("{} bytes read,{}", read(*(int*)events[0].data.ptr, buf, 10), *(long*)(buf));
      std::cout.flush();
    } else {
      fmt::println("no fd avail {}", num_events);
    }
  }

  close(epoll_fd);
  close(timer_fd);
}

TEST(EPOLL_TEST, time_channel_test) {
  using namespace std::chrono_literals;

  EventScheduler timer_scheduler{10};
  TimerChannel::Ptr timer_channel = std::make_shared<TimerChannel>([]() {
    thread_local int v = 0;
    fmt::println("trigger! {}", v++);
    std::cout.flush();
  });
  SelectedChannels select_channels;
  timer_scheduler.epoll_selector_.Add(timer_channel, SelectEvents::kReadEvent.EdgeTrigger());
  timer_channel->SetTimer(1s, 0.5s);
  std::this_thread::sleep_for(10s);

  fmt::println("here");
  std::cout.flush();
  timer_scheduler.Join();
}
