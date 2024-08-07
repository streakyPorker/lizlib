//
// Created by A on 2024/8/7.
//

#include <charconv>
#include "concurrent/thread_pool.h"

#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"
using namespace lizlib;
using namespace std;

namespace general_test {
template <int... Nums>
struct CheckP {
  static constexpr bool value = sizeof...(Nums);
  constexpr static bool isP() {
    std::array<int, sizeof...(Nums)> arr{Nums...};
    for (int i = 0; i <= arr.size() / 2; i++) {
      if (arr[i] != arr[arr.size() - 1 - i]) {
        return false;
      }
    }
    return true;
  }
};
TEST(GeneralTest, useOfConstexpr) {
  using namespace std;
  static_assert(CheckP<1, 2, 3, 2, 1>::isP(), "msg 1");
  //  static_assert(CheckP<1,2,3>::isP(),"msg 2");
}

template <typename T>
int print(T t) {
  static int i = 0;
  cout << t << " ";
  return i++;
}
template <typename... Args>
void prints(Args... args) {
  vector<int> rst = {print(args)...};
  vector<int> rst2 = {(print(args), 0)...};
  for (int v : rst) {
    cout << v << " ";
  }
}

TEST(GeneralTest, varArgTest) {
  prints('a', 'b', 'c', 'd');
}

enum Enum : long long { A = 1, B, C, D };
TEST(GeneralTest, enumTest) {
  cout << sizeof(D) << endl;
}

TEST(GeneralTest, smartPtrTest) {
  //指定 default_delete 作为释放规则
  std::shared_ptr<int> p6(new int[10], std::default_delete<int[]>());
  //自定义释放规则
  auto deleteInt = [](const int* p) {
    delete[] p;
  };
  //初始化智能指针，并自定义释放规则
  std::shared_ptr<int> p7(new int[10], deleteInt);
}

struct Obj {
  int v;
  explicit Obj(int val = 0) : v(val) {}
  ~Obj() { cout << "dtor of " << v << endl; }
};

TEST(GeneralTest, smartPtrTest2) {
  //指定 default_delete 作为释放规则
  Obj* obj1 = new Obj(1);
  Obj* obj2 = new Obj(2);
  shared_ptr<Obj> ptr(obj1);
  optional<int> opInt;
  opInt = 1;

}

template<typename T> concept Incrementable = requires(T x) {x++; ++x;};
TEST(GeneralTest, streamView) {
  //指定 default_delete 作为释放规则
  vector<int> data {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  const std::string str{"123456098"};
  int val = 10;
  auto cpy = from_chars(str.data(), str.data()+4,val);
  cout << cpy.ptr << endl;
}

}  // namespace general_test
