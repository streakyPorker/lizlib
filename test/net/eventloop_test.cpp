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
    std::atomic_int v = 0;
    for (int i = 0; i < 10000000; i++) {
      loop.Submit([&v]() { fmt::println("cur v is {}", v.fetch_add(1)); });
    }

    std::this_thread::sleep_for(2s);
    loop.Join();
  }
}

TEST(EventLoopTest, el_delay_test) {
  EventLoop loop{};
  loop.SubmitAfter(
    []() {
      fmt::println("delayed task");
      std::cout.flush();
    },
    1ms);
  std::this_thread::sleep_for(1s);
  loop.Join();
}

TEST(EventLoopTest, el_delay_every_test) {
  EventLoop loop{};
  loop.SubmitAfter(
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
    1s, 400ms);
  std::this_thread::sleep_for(4s);
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
