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
  //  fmt::println("{}", std::filesystem::current_path().c_str());
  //  config_loader::LoadConfig("/root/codebase/cxx/lizlib/config/config.ini");
}
