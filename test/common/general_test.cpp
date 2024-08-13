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
void funcCall() {}
namespace general_test {
template <int... Nums>
struct CheckP {
  static constexpr int value = sizeof...(Nums);
  static conditional<value % 2 == 0, int, double> var;
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

  cout << typeid(CheckP<1, 2>::var).name() << " " << typeid(CheckP<1, 2, 3>::var).name() << endl;
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

void func(int a, int b) {
  cout << &a << " " << &b << endl;
};
TEST(GeneralTest, streamView) {
  //指定 default_delete 作为释放规则
  func(1, 2);
}

struct A {
  virtual void a1() {}
  virtual void a2() {}
  virtual ~A() {}
};

struct B {
  virtual void b1() {}
  virtual void b2() {}
  virtual ~B() {}
};

class C : public A, public B {
 public:
  C() {}
  virtual void a1() override {}
  virtual void a2() override {}
  virtual void b1() override {}
  virtual void b2() override {}
  virtual ~C() {}
};

// 用于获取虚函数表的辅助函数
template<typename T>
void* get_vtable(T*) {
  static_assert(sizeof(T) == 0, "This is an invalid type");
  return nullptr;
}

template<typename T>
void* get_vtable(T& obj) {
  return &obj + 1;
}


TEST(GeneralTest, testVirtual) {
  C c;

  void* vtable_c = get_vtable(c);

  // 输出虚函数表的位置
  std::cout << "Virtual Table of C: " << vtable_c << std::endl;

  // 获取虚函数地址
  void (*a1)(C*) = *reinterpret_cast<void (C::**)()>(vtable_c);
  void (*a2)(C*) = *reinterpret_cast<void (C::**)()>(reinterpret_cast<char*>(vtable_c) + sizeof(void*));
  void (*b1)(C*) = *reinterpret_cast<void (C::**)()>(reinterpret_cast<char*>(vtable_c) + 2 * sizeof(void*));
  void (*b2)(C*) = *reinterpret_cast<void (C::**)()>(reinterpret_cast<char*>(vtable_c) + 3 * sizeof(void*));

  // 输出虚函数地址
  std::cout << "Address of a1 in C: " << a1 << std::endl;
  std::cout << "Address of a2 in C: " << a2 << std::endl;
  std::cout << "Address of b1 in C: " << b1 << std::endl;
  std::cout << "Address of b2 in C: " << b2 << std::endl;

}

}  // namespace general_test
