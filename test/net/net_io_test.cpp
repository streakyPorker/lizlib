//
// Created by A on 2023/8/5.
//

#include "common/thread_pool.h"

#include <liburing.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "common/basic.h"
#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"

using namespace lizlib;

#define QUEUE_DEPTH 32
#define BUF_SIZE 4096
#define NUM_REQUESTS 5

TEST(NETIOTEST, basic_dummy_test) {
  fmt::println("{} {}",sizeof (sockaddr_in),sizeof (sockaddr_in6));
}



TEST(NETIOTEST, epoll_test) {
  File file = File::Open("testfile.txt", O_CREAT | O_RDWR);
}


TEST(NETIOTEST, byte_order_test) {
  uint16_t port = 1234;
  fmt::println("{} {}",htons(port), htons(htons(port)));
}
