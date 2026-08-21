#include <fstream>
#include <iostream>

#include "xtrpg/config/ConfigManager.hpp"

int main(int argc, char *argv[]) {

  // Output preamble information about the application and its configuration
  // system.
  std::cout << "XTRPG: A XMPP Server" << std::endl
            << "Version: 0.1.0" << std::endl
            << "Copyright (C) 2026 XTRPG Contributors" << std::endl
            << "License: MIT" << std::endl
            << "         "
               "<https://github.com/xtrpg/xtrpg-cpp-server/blob/main/LICENSE>"
            << std::endl
            << std::endl
            << "This is free and open-source software. You are free to use, "
               "modify, and redistribute it under the terms of the MIT License."
            << std::endl
            << "There is NO WARRANTY for this software." << std::endl;

  // Discover and register all modules' configuration schemas, then load the
  // configuration file and parse command-line arguments.
  xtrpg::config::ConfigManager configManager;
  configManager.registerAllDiscoveredModules();
  std::ifstream configFile("./config.toml");
  if (configFile) {
    configManager.loadTomlFile(configFile);
  }
  configManager.parseCLI(argc, argv);

  return 0;
}