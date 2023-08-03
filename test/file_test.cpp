

#include "file.h"
#include "gtest/gtest.h"
#include "logger.h"

using namespace lizlib;

TEST(BasicTest, fileOpenTest) {
  File file = File::Open("testfile.txt", O_CREAT | O_RDWR);
  file.Close();
}
