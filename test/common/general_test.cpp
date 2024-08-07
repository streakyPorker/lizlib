//
// Created by A on 2024/8/7.
//

#include "concurrent/thread_pool.h"

#include "common/basic.h"
#include "common/logger.h"
#include "gtest/gtest.h"
using namespace lizlib;

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

TEST(GeneralTest, gt1) {
  using namespace std;
  static_assert(CheckP<1,2,3,2,1>::isP(),"msg 1");
  static_assert(CheckP<1,2,3>::isP(),"msg 2");
}