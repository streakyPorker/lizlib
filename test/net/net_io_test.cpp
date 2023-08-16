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

TEST(NETIOTEST, epoll_test) {
  File file = File::Open("testfile.txt", O_CREAT | O_RDWR);

}
