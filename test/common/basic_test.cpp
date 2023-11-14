
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
  ASSERT_TRACE(false, "{} {}", config::kTcpOption.reuse_port, config::kTcpOption.reuse_addr);
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
  fmt::println("{}", cts);
  cts.Incr(1min);
  fmt::println("{} {}", cts, Timestamp::Now());
  fmt::println("{}", Rdtsc());
  fmt::println("{}", Rdtsc());
  fmt::println("{}", Rdtsc());
}
using namespace std::chrono_literals;
struct A {
  A() { fmt::println("A::A();"); }
};

struct B {
  B() : b1(generateB1()) {}

  static int generateB1() {
    fmt::println("before or after A::A()???");
    return 1;
  }
  A a;
  int b1;
};
TEST(BasicTest, ctorCallTest) {
  B b;
}

TEST(BasicTest, lcTest) {
  using namespace std;
  auto cmp = [](const pair<int, int>& a, const pair<int, int>& b) {
    return a.second > b.second;  // 最大堆排序
  };
  priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(cmp)> queue(cmp);
  queue.top()
}