//
// Created by A on 2023/10/12.
//

#include <filesystem>
#include "common/config_parser.h"
#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"

using namespace lizlib;

TEST(EngTest, t1) {
  fmt::println("{}", __FILE__);
  std::shared_ptr<std::string> p1 = nullptr;
  std::shared_ptr<std::string> p2 = std::make_shared<std::string>("2");
  std::shared_ptr<std::string> p3 = std::make_shared<std::string>("asdrrr");
  p2 = std::move(p3);

  fmt::println("{} {}", p2->c_str(), p3 == nullptr);
  //  fmt::println("{}", std::filesystem::current_path().c_str());
  //  config_loader::LoadConfig("/root/codebase/cxx/lizlib/config/config.ini");
}
