
#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"
#include "net/config.h"
using namespace lizlib;

TEST(BasicTest, basicTest1) {
  Timestamp ts = Timestamp::Now();
  std::cout << fmt::format("{}", ts) << std::endl;
  std::cout << fmt::format("{:d}", 123) << std::endl;
}

TEST(BasicTest, logTest) {
  LOG_INFO("{} {} {} {} {}", "testing", 2, 3, 4, 5);
  ASSERT_TRACE(false, "{}", 1123);
  ASSERT_TRACE(false, "{} {}", kTcpOption.reuse_port, kTcpOption.reuse_addr);
}

TEST(BasicTest, stackAllocTest) {
  int val = 1;
  fmt::println("{}", (void*)&val);
  int *intptr1 = nullptr, *intptr2 = nullptr;
  LIZ_ESCAPABLE_MEM(intptr1, 100);
  LIZ_ESCAPABLE_MEM(intptr2, 100000);
}


TEST(BasicTest, ctsTest) {
  ConcurrentTimestamp cts{Timestamp::Now()};
  using namespace std::chrono_literals;
  fmt::println("{}",cts);
  cts.Incr(1min);
  fmt::println("{} {}",cts,Timestamp::Now());
  fmt::println("{}",Rdtsc());
  fmt::println("{}",Rdtsc());
  fmt::println("{}",Rdtsc());
}