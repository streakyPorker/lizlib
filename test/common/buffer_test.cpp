
#include "common/buffer.h"
#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"
using namespace lizlib;
static int cnter = 0;

struct Tc {
  int i{0};
  ~Tc() {
    fmt::println("deleted,prev is {}", cnter);
    cnter++;
  }
};

TEST(BufferTest, buffertest1) {
  Tc t1{};
  {
    void* t2 = malloc(100);
    Buffer buf{t2, 100, free};
    Buffer b2{std::move(buf)};
    fmt::println("{} {}", buf, b2);
  }

  {
    std::string str;
    str += "asdrrr";
    Buffer b1 = str;
    Buffer b2{b1};
    fmt::println("{} {}", b1, b2);
    Buffer b3{std::move(b1)};
    fmt::println("b1:{}    b2:{}     b3:{}", b1, b2, b3);
  }

  { auto buf_ptr = std::make_unique<Buffer>(1024); }
}

TEST(BufferTest, buffertest2) {
  char asd[100];
  fmt::println("{}", asd + 99 - asd);
  fmt::println("{} {} {}", (void*)CeilPageAlignAddr(asd), (void*)FloorPageAlignAddr(asd),
               (void*)asd);
}

TEST(BufferTest, bufferfiletwtest) {

  std::string adder = "this is added content\n";
  fmt::println("adder length {}", adder.length());
  File file = File::Open("bufferfiletwtest.txt", O_CREAT | O_RDWR);

  {
    file.Seek(0, SEEK_SET);
    file.Truncate(0);
    file.Write(adder.c_str(), adder.length());
    Buffer bufFile{}, bufBytes{};
    file.Seek(0, SEEK_SET);
    bufFile.Append(file, config::kFileRWUnit);
    bufBytes.Append(adder.data(), adder.length());
    fmt::println("{}", bufFile);
    fmt::println("{}", bufBytes);
    EXPECT_TRUE(!memcmp(adder.data(), bufFile.Data(), adder.size()));
    EXPECT_TRUE(!memcmp(adder.data(), bufBytes.Data(), adder.size()));
    EXPECT_EQ(adder.length(), bufBytes.ReadableBytes());
    EXPECT_EQ(adder.length(), bufFile.ReadableBytes());
  }

  {
    file.Seek(0, SEEK_SET);
    file.Truncate(0);
    for (int i = 0; i < 1e5; ++i) {
      file.Write(adder.data(), adder.length());
    }
    file.Seek(0, SEEK_SET);
    Buffer bufFile{};
    bufFile.Append(file,4096);
    fmt::println("{}", bufFile);
    EXPECT_EQ(file.Size(), bufFile.ReadableBytes());
  }
}

TEST(BufferTest, bufferRWtest) {
  Buffer buf1{}, buf2{}, buf3{};

  buf1.AppendBasic<int64_t>(123);
  buf1.AppendBasic<int64_t>(456);
  buf1.AppendBasic<int64_t>(789);
  fmt::println("{} {}", buf1, buf2);
  fmt::println("{}", buf2.Append(buf1));
  fmt::println("{} {}", buf1, buf2);
  fmt::println("{}", buf3.Transfer(buf1));
  fmt::println("{} {}", buf1, buf3);
  buf1.Rearrange();
  fmt::println("{} {}", buf3.ReadBasic<int64_t>(), buf3.ReadBasic<int64_t>());
  fmt::println("{} {}", buf1, buf3);
}