

#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"

using namespace lizlib;

TEST(BasicTest, fileOpenTest) {
  File file = File::Open("testfile.txt", O_CREAT | O_RDWR);
  file.Close();
}
