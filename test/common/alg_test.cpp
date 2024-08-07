//
// Created by A on 2024/8/9.
//
#include <charconv>
#include "concurrent/thread_pool.h"

#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"

using namespace lizlib;
using namespace std;
TEST(AlgTest, printChar) {
  int n = 5, rep = 5;
  atomic<char> charVar = 'A';
  vector<thread> threads;

  for (int i = 0; i < n; i++) {
    const int ii = i;
    threads.emplace_back([&, ii]() {
      char c;
      for (int j = 0; j < rep; j++) {
        while ((c = charVar.load(std::memory_order_relaxed)) - 'A' != ii) {}
        cout << c<< " ";
        charVar.store('A' + (ii + 1) % n, memory_order_release);
      }
    });
  }
  for (auto& t : threads) {
    t.join();
  }
}