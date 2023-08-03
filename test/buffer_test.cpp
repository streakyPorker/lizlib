
#include "buffer.h"
#include "basic.h"
#include "gtest/gtest.h"
#include "logger.h"
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
}
