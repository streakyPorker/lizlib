//
// Created by A on 2023/8/3.
//

#include "common/thread_pool.h"

#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"
using namespace lizlib;

TEST(ThreadPoolTest, tpt1) {
  using namespace std::chrono_literals;
  ThreadPool pool{std::thread::hardware_concurrency(), false};
  for (int i = 0; i < 5; i++) {
    std::cout << "waiting " << i << std::endl;
    std::this_thread::sleep_for(1s);
  }
  pool.Start();
  for (int i = 0; i < 100; i++) {
    pool.Submit(
      [](void* val) {
        std::cout << "work is running..." << std::endl;
        std::this_thread::sleep_for(1ms);
      },
      nullptr);
  }
  std::this_thread::sleep_for(5s);
  pool.Stop();
}

void myThreadFunction() {
  // 在这里编写线程的任务代码
  std::cout << "Thread is running..." << std::endl;
  // 假设线程执行一些耗时任务
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::cout << "Thread has finished its task." << std::endl;
}

TEST(ThreadPoolTest, tpt2) {
  std::thread myThread(myThreadFunction);

  // 让主线程等待一段时间
  std::cout << "Main thread is waiting..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // 等待新线程结束
  myThread.join();
  std::cout << "Main thread has finished." << std::endl;
}

TEST(ThreadPoolTest, testDelay) {
  using namespace  std::chrono_literals;
  ThreadPool threadPool{4};
  threadPool.SubmitDelay([](){
    fmt::println("here");
  },1ms);
}