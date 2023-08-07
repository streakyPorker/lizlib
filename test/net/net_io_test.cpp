//
// Created by A on 2023/8/5.
//

#include "common/thread_pool.h"

#include <liburing.h>
#include <sys/epoll.h>
#include "common/basic.h"
#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"
using namespace lizlib;

#define QUEUE_DEPTH 32
#define BUF_SIZE 4096
#define NUM_REQUESTS 5

TEST(NETIOTEST, t1) {
  File file = File::Open("testfile.txt", O_CREAT | O_RDWR);

  struct io_uring ring {};
  ASSERT_FALSE(io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0);

  struct io_uring_sqe* sqe;
  char write_buffer[] = "Hello, IO_uring!";
  char read_buffer[BUF_SIZE];
  for (int i = 0; i < NUM_REQUESTS; i++) {
    sqe = io_uring_get_sqe(&ring);
    sqe->user_data = (__u64) "this is write op";
    io_uring_prep_write(sqe, file.Fd(), write_buffer, strlen(write_buffer),
                        i * strlen(write_buffer));
  }
  io_uring_submit(&ring);
  struct io_uring_cqe* cqe;
  for (int i = 0; i < NUM_REQUESTS; i++) {
    ASSERT_FALSE(io_uring_wait_cqe(&ring, &cqe) < 0);
    // 处理写请求的结果
    if (cqe->res < 0) {
      perror("write");
    } else {
      printf("Write: %d bytes\n", cqe->res);
      printf("userdata %s\n", (char*)cqe->user_data);
    }
    // 释放完成队列元素
    io_uring_cqe_seen(&ring, cqe);
  }
}
