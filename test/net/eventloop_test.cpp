//
// Created by A on 2023/10/12.
//

#include "concurrent/thread_pool.h"

#include <liburing.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "common/basic.h"
#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"
#include "net/eventloop/event_loop_group.h"

using namespace lizlib;
using namespace std::chrono_literals;
#include "common/basic.h"
#include "net/eventloop/event_loop.h"
TEST(EventLoopTest, basic_dummy_test) {
  using namespace std::chrono_literals;

  {
    EventLoop loop{};
    //  TimerScheduler::Ptr ts = loop.GetTimeScheduler();
    loop.Submit([&loop]() { ASSERT_FATAL(EventLoop::Current() == &loop, "failed to bind"); });
    loop.Submit([]() {
      fmt::println("here 1");
      std::cout.flush();
      std::this_thread::sleep_for(5s);
      fmt::println("here 2");
      std::cout.flush();
    });
    std::this_thread::sleep_for(6s);
    loop.Join();
  }

  for (int i = 0; i < 10; i++) {
    fmt::println("cnt");
    std::cout.flush();
    std::this_thread::sleep_for(1s);
  }
}
TEST(EventLoopTest, el_delay_test) {
  EventLoop loop{};
  loop.SubmitDelay(
    []() {
      fmt::println("delayed task");
      std::cout.flush();
    },
    1s);
  loop.SubmitEvery(
    []() {
      fmt::println("every task");
      std::cout.flush();
    },
    2s, 800ms);
  std::this_thread::sleep_for(6s);
}

TEST(EventLoopTest, regression_test) {
  ThreadPool pool{1};
  pool.Submit([]() {
    fmt::println("here");
    std::cout.flush();
  });
  std::this_thread::sleep_for(1s);
}

TEST(EventLoopTest, elg_test) {
  EventLoopGroup elg{3};
  elg.Next()->Submit([]() {
    fmt::println("here");
    std::cout.flush();
  });
  elg.Next()->Submit([]() {
    fmt::println("here");
    std::cout.flush();
  });
  elg.Next()->Submit([]() {
    fmt::println("here");
    std::cout.flush();
  });
  std::this_thread::sleep_for(2s);
  elg.Join();
}
