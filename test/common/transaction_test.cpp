//
// Created by bangsun on 2023/9/18.
//
#include "common/file.h"
#include "common/logger.h"
#include "concurrent/transaction_util.h"
#include "gtest/gtest.h"

using namespace lizlib;

TEST(TransactionTest, cas_test) {
  TransactionUtil::CAS64();
}
