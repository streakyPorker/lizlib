
#include "basic.h"
#include "gtest/gtest.h"
#include "logger.h"
using namespace lizlib;

TEST(BasicTest, basicTest1) {
  Timestamp ts = Timestamp::Now();
  std::cout << fmt::format("{}", ts) << std::endl;
  std::cout << fmt::format("{:d}", 123) << std::endl;
}

TEST(BasicTest, logTest) {
  LOG_INFO("{} {} {} {} {}", "testing", 2, 3, 4, 5);
}