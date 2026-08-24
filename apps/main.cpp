#include <fstream>
#include <iostream>

#include "version.hpp"
#include "xtrpg/config/ConfigManager.hpp"

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
  // Set console codepages to UTF-8 (65001)
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  try {

    // Output preamble information about the application and its configuration
    // system.
    std::cout << "XTRPG: A XMPP Server" << std::endl
              << "Version: " << xtrpg::VERSION << std::endl
              << "Copyright (C) 2026 XTRPG Contributors" << std::endl
              << "License: MIT" << std::endl
              << "         "
                 "<https://github.com/xtrpg/xtrpg-cpp-server/blob/main/LICENSE>"
              << std::endl
              << std::endl
              << "This is free and open-source software." << std::endl
              << "You are free to use, modify, and redistribute it under the "
                 "terms of the MIT License."
              << std::endl
              << "There is NO WARRANTY for this software." << std::endl
              << std::endl;

    // Discover and register all modules' configuration schemas, then load the
    // configuration file and parse command-line arguments.
    xtrpg::config::ConfigManager configManager;
    configManager.registerAllDiscoveredModules();
    std::ifstream configFile("./config.toml");
    if (configFile) {
      configManager.loadTomlFile(configFile);
    }
    configManager.parseCLI(argc, argv);

  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION OCCURRED" << std::endl
              << "Application closing die to \"" << e.what() << "\"."
              << std ::endl;
    return 1;
  }

  return 0;
}