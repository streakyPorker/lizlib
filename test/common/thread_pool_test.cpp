//
// Created by A on 2023/8/3.
//

#include "concurrent/thread_pool.h"

#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"
using namespace lizlib;

TEST(ThreadPoolTest, tpt1) {
  using namespace std::chrono_literals;
  ThreadPool pool{1};

  for (int i = 0; i < 20; i++) {
    pool.Submit(
      [&i](void* val) {
        std::cout << "work is running..." << std::endl;
        std::this_thread::sleep_for(200ms);
      },
      nullptr);
  }
  std::this_thread::sleep_for(5s);
  pool.Join();
}

TEST(ThreadPoolTest, test_multi_pool_single_scheduler) {
  using namespace std::chrono_literals;
  std::shared_ptr<EventScheduler> scheduler = std::make_shared<EventScheduler>(20);
  ThreadPool pool1{1, scheduler};
  ThreadPool pool2{1, scheduler};

  for (int i = 0; i < 20; i++) {
    pool1.Submit(
      [&i](void* val) {
        std::cout << "work is running..." << std::endl;
        std::this_thread::sleep_for(200ms);
      },
      nullptr);
    pool2.Submit(
      [&i](void* val) {
        std::cout << "work is running..." << std::endl;
        std::this_thread::sleep_for(200ms);
      },
      nullptr);
  }
  std::this_thread::sleep_for(5s);
  pool1.Join();
  fmt::println("between p1 and p2");
  std::cout.flush();
  pool2.Join();
  fmt::println("after p1 and p2");
}
TEST(ThreadPoolTest, tpt2) {
  using namespace std::chrono_literals;
  ThreadPool pool{std::thread::hardware_concurrency()};

  for (int i = 0; i < 20 * 20; i++) {
    pool.Submit(
      [&i](void* val) {
        std::cout << "work is running..." << std::endl;
        std::this_thread::sleep_for(200ms);
      },
      nullptr);
  }
  std::this_thread::sleep_for(5s);
  pool.Join();
}

TEST(ThreadPoolTest, test_every) {
  lizlib::log_level = lizlib::Level::kTrace;
  using namespace std::chrono_literals;
  std::atomic_int64_t val = 0;
  EventScheduler::Ptr timer_scheduler = std::make_shared<EventScheduler>(10);
  ThreadPool pool{1, timer_scheduler};
  for (int i = 0; i < 5; i++) {
    pool.SubmitEvery(
      [&val]() {
        std::cout << "work is running..." << val.fetch_add(1) << std::endl;
        std::this_thread::sleep_for(200ms);
      },
      1s, 1s);
  }
  for (int i = 0; i < 5; i++) {
    LOG_TRACE("wait {}s", i);
    std::this_thread::sleep_for(1s);
  }
  LOG_TRACE("{}", val.load());

  ThreadPool pool2{1, timer_scheduler};
  for (int i = 0; i < 5; i++) {
    pool2.SubmitEvery(
      [&val]() {
        std::cout << "work is running..." << val.fetch_add(1) << std::endl;
        std::this_thread::sleep_for(200ms);
      },
      1s, 1s);
  }
  for (int i = 0; i < 5; i++) {
    LOG_TRACE("wait {}s", i);
    std::this_thread::sleep_for(1s);
  }
  LOG_TRACE("{}", val.load());
  pool.Join();
  pool2.Join();
}

TEST(ThreadPoolTest, testDelay) {
  using namespace std::chrono_literals;
  ThreadPool threadPool{4};
  threadPool.SubmitDelay([]() { fmt::println("here"); }, 1ms);
}