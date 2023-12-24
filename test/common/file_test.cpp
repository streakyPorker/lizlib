

#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"

using namespace lizlib;

TEST(File, fileOpenTest) {
  File file = File::Open("testfile.txt", O_CREAT | O_RDWR);
  file.Close();
}

TEST(File, fileWriteTest) {
  File fileWithEveryWriteSync = File::Open("fileWithEveryWriteSync.txt", O_CREAT | O_RDWR);
  File fileWithEverySecondSync = File::Open("fileWithEverySecondSync.txt", O_CREAT | O_RDWR);
  File fileWithLastSync = File::Open("fileWithLastSync.txt", O_CREAT | O_RDWR);

  size_t writeSize = 10 * 1024;      // 10kB per write
  char* data = new char[writeSize];  // 10B data
  std::string temp = "hello world from test!\n";
  for (int i = 0; i < writeSize / temp.size(); i++) {
    memcpy(data + i * temp.size(), temp.data(), temp.size());
  }

  size_t writeTotal = 100L * 1024 * 1024;  // 100MB write

  {
    Timestamp now = Timestamp::Now();
    for (int i = 0; i < writeTotal / writeSize; i++) {
      fileWithEveryWriteSync.Write(data, writeSize);
      fileWithEveryWriteSync.Sync(kSync);
      LOG_INFO("{}KB written", (i + 1) * writeSize / 1024);
    }
    fmt::println("fileWithEveryWriteSync cost {}ms, throughput:{}kB/ns", Timestamp::Now() - now,
                 writeTotal * 1e6 / 1024 / (Timestamp::Now() - now).usec_);
  }
}