#include <fmt/core.h>
#include <functional>
#include <iostream>
int main() {
  std::cout << fmt::format("%s", "asd") << std::endl;

  fmt::println("{}", typeid(decltype(std::bind((void*){}, nullptr))).name());
  return 0;
}
