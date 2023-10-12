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
namespace lizlib::config_loader {

#define LIZ_SET_CONFIG(name,val) ::lizlib::config::name = val

void LoadConfig(const std::string& cfg_file_name) {
  boost::property_tree::ptree pt;
  boost::property_tree::ini_parser::read_ini(cfg_file_name, pt);
  for (const auto& pair : pt) {
    fmt::println("{} {}", pair.first, pair.second.data());
    // can't reset config...
  }
}
}  // namespace lizlib

#endif  //LIZLIB_CONFIG_PARSER_H
