#include "xtrpg/config/ConfigManager.hpp"

namespace xtrpg::config {

/**
 * Registers a module's configuration schema with the ConfigManager, allowing
 * it to be included in the final resolved configuration values.
 */
void ConfigManager::registerModule(const IModuleConfigProvider &provider) {
  ModuleConfig schema = provider.getConfigSchema();
  m_schemas[schema.name] = schema.options;
  for (const auto &opt : schema.options) {
    m_values[schema.name][opt.key] = {.systemDefault = opt.defaultValue};
  }
}

/**
 * Prints a formatted help menu to the console, including usage instructions
 * and descriptions of all registered configuration options.
 *
 * @param os The output stream to which the help menu will be printed.
 */
void ConfigManager::printHelp(std::ostream &os) const {
  os << "Usage: app [options]" << std::endl << std::endl;
  os << "Options:" << std::endl;
  os << "  --help, -h          Show this help message and exit" << std::endl;
  for (const auto &[section, options] : m_schemas) {
    os << "[" << section << "]" << std::endl;
    for (const auto &opt : options) {
      os << "--" << section << "." << opt.key << std::endl
         << "      " << opt.description
         << " (Default: " << formatValue(opt.defaultValue) << ")\n";
    }
    os << std::endl;
  }
}

/**
 * Loads configuration values from a TOML file, parsing the content and
 * applying the values to the appropriate sections and keys.
 */
bool ConfigManager::loadTomlFile(std::istream &input) {
  std::string line;
  std::string currentSection = "global";

  while (std::getline(input, line)) {
    // Trim simple whitespace
    line.erase(0, line.find_first_not_of(" \t\r\n"));
    line.erase(line.find_last_not_of(" \t\r\n") + 1);

    if (line.empty() || line[0] == '#')
      continue;

    // Section parsing [section]
    if (line.front() == '[' && line.back() == ']') {
      currentSection = line.substr(1, line.size() - 2);
      continue;
    }

    // Key-value parsing (key = value)
    auto eqPos = line.find('=');
    if (eqPos != std::string::npos) {
      std::string key = line.substr(0, eqPos);
      std::string valStr = line.substr(eqPos + 1);

      // Basic string clean up
      key.erase(key.find_last_not_of(" \t") + 1);
      valStr.erase(0, valStr.find_first_not_of(" \t"));

      // Deduce type & assign (Overwrites module/compile defaults)
      if (valStr == "true")
        // Boolean type set to true
        m_values[currentSection][key].fileOverride = true;
      else if (valStr == "false")
        // Boolean type set to false
        m_values[currentSection][key].fileOverride = false;
      else if (valStr.front() == '"' && valStr.back() == '"') {
        // String type
        m_values[currentSection][key].fileOverride =
            valStr.substr(1, valStr.size() - 2);
      } else if (valStr.find('.') != std::string::npos) {
        // Double type
        m_values[currentSection][key].fileOverride = std::stod(valStr);
      } else {
        // Integer type
        m_values[currentSection][key].fileOverride = std::stoll(valStr);
      }
    }
  }
  return true;
}

/**
 * Parses command-line arguments, updating configuration values based on the
 * provided flags and their associated values.
 */
void ConfigManager::parseCLI(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      printHelp(std::cout);
      std::exit(0);
    }

    // Check registered short/custom flags or dot-notation
    // (--section.key=value)
    std::string keyPath;
    std::string valueStr;

    auto eqPos = arg.find('=');
    if (eqPos != std::string::npos) {
      keyPath = arg.substr(0, eqPos);
      valueStr = arg.substr(eqPos + 1);
    } else {
      keyPath = arg;
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        valueStr = argv[++i];
      }
    }

    if (keyPath.rfind("--", 0) == 0) {
      keyPath = keyPath.substr(2); // Strip leading --
    }

    auto dotPos = keyPath.find('.');
    if (dotPos != std::string::npos) {
      std::string section = keyPath.substr(0, dotPos);
      std::string key = keyPath.substr(dotPos + 1);

      if (!valueStr.empty()) {
        if (valueStr == "true")
          m_values[section][key].cliOverride = true;
        else if (valueStr == "false")
          m_values[section][key].cliOverride = false;
        else if (std::all_of(valueStr.begin(), valueStr.end(), ::isdigit)) {
          m_values[section][key].cliOverride = std::stoll(valueStr);
        } else {
          m_values[section][key].cliOverride = valueStr;
        }
      }
    }
  }
}

/**
 * Dumps the resolved configuration values to the specified output stream,
 * formatting them in a human-readable way for inspection or debugging.
 *
 * @param os The output stream to which the resolved configuration will be
 * dumped. Defaults to std::cout.
 */
void ConfigManager::dumpResolvedConfig(std::ostream &os) const {

  for (const auto &[section, options] : m_values) {
    os << "[" << section << "]" << std::endl;
    for (const auto &[key, value] : options) {

      // output the key's documentation/description if available
      auto schemaIt = m_schemas.find(section);
      if (schemaIt != m_schemas.end()) {
        const auto &schemaOptions = schemaIt->second;
        auto optIt = std::find_if(
            schemaOptions.begin(), schemaOptions.end(),
            [&key](const ConfigOption &opt) { return opt.key == key; });
        if (optIt != schemaOptions.end()) {
          os << "# " << optIt->description << std::endl;
        }
      }

      // output the key and its corresponding value in a readable format
      os << key << " = " << formatValue(value.getEffectiveValue()) << std::endl;
    }
    os << std::endl;
  }
}

} // namespace xtrpg::config