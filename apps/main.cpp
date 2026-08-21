#include <iostream>

#include "xtrpg/config/ConfigManager.hpp"

int main(int argc, char *argv[]) {

  // Discover and register all modules' configuration schemas
  xtrpg::config::ConfigManager configManager;
  configManager.registerAllDiscoveredModules();
  configManager.parseCLI(argc, argv);

  return 0;
}