//
// Created by A on 2023/10/20.
//

#include "concurrent/countdown_latch.h"
#include "common/logger.h"
void lizlib::CountdownLatch::CountDown() {
  size_t read_count;
  while (true) {
    read_count = count_.load(std::memory_order_acquire);
    if (read_count == 0) {
      break;
    }
    if (count_.compare_exchange_strong(read_count, read_count - 1, std::memory_order_release)) {
      read_count = read_count - 1;
      break;
    }
  }
  if (read_count == 0) {
    cv_.NotifyAll(false);
  }
}
bool lizlib::CountdownLatch::Await(const lizlib::Duration& timeout) {
  cv_.Wait(timeout);
  return count_.load(std::memory_order_relaxed) == 0;
}
void lizlib::CountdownLatch::Await() {
  cv_.Wait();
  ASSERT_FATAL(count_.load(std::memory_order_relaxed) == 0, "inconsistent latch");
}
