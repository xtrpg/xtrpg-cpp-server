#include <fstream>
#include <iostream>

#include "xtrpg/config/ConfigManager.hpp"

int main(int argc, char *argv[]) {

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