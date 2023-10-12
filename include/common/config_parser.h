//
// Created by A on 2023/10/12.
//

#ifndef LIZLIB_CONFIG_PARSER_H
#define LIZLIB_CONFIG_PARSER_H
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>
#include "common/basic.h"
namespace lizlib {
void LoadConfig(const std::string& cfg_file_name) {
  boost::property_tree::ptree pt;
  boost::property_tree::ini_parser::read_ini(cfg_file_name, pt);
  for (const auto& pair : pt) {
    fmt::println("{} {}", pair.first, pair.second.data());
  }
}
}  // namespace lizlib

#endif  //LIZLIB_CONFIG_PARSER_H
