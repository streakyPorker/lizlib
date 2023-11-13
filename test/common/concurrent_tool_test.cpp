//
// Created by A on 2023/11/13.
//

#include "common/file.h"
#include "common/logger.h"
#include "concurrent/countdown_latch.h"
#include "gtest/gtest.h"

using namespace lizlib;
using namespace std::chrono_literals;

TEST(ConcurrentTest, cdlTest) {
  CountdownLatch latch{10};

  std::thread thread{[&latch]() {
    for(int i=0;i<10;i++){
      LOG_TRACE("counting {}",i+1);
      std::this_thread::sleep_for(1s);
      latch.CountDown();
    }
  }};
  latch.Await();
  LOG_TRACE("wait ended");
  thread.join();
}
