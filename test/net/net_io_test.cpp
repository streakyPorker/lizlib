//
// Created by A on 2023/8/5.
//

#include "common/thread_pool.h"

#include <liburing.h>
#include <sys/epoll.h>
#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"
using namespace lizlib;

TEST(NETIOTEST, t1) {
  int epoll_fd = ::epoll_create(5);
}
