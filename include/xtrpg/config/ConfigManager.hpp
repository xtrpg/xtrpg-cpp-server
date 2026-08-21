#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#define REGISTER_MODULE_CONFIG(ModuleClass)                                    \
  static struct ModuleClass##ConfigRegistrar {                                 \
    ModuleClass##ConfigRegistrar() {                                           \
      xtrpg::config::ConfigManager::registerStaticModule(                      \
          []() { return std::make_unique<ModuleClass>(); });                   \
    }                                                                          \
  } global_##ModuleClass##_config_registrar;

namespace xtrpg::config {

/**
 * Supporting basic primitive types.
 */
using ConfigValue = std::variant<bool, int64_t, double, std::string>;

/**
 * Represents a single configuration option with its metadata.
 */
struct ConfigOption {
  /**
   * Key name for the option, used in TOML and CLI (e.g., "threads" or
   * "engine.render_quality").
   */
  std::string key;

  /**
   * Default value for the option, used if not overridden by TOML or CLI.
   */
  ConfigValue defaultValue;

  /**
   * Description of the option, used for help output and documentation.
   */
  std::string description;
};

/**
 * Represents a module's configuration schema, including its name and the
 * options it provides.
 */
struct ModuleConfig {
  /**
   * Name of the module, used as a section header in TOML and CLI (e.g.,
   * "engine" or "network").
   */
  std::string name;

  /**
   * Optional description of the module, used for help output and documentation.
   */
  std::string description;

  /**
   * List of configuration options provided by the module, including their
   * keys, default values, descriptions, and optional CLI flags.
   */
  std::vector<ConfigOption> options;
};

/**
 * Interface for modules to provide their configuration schema to the
 * ConfigManager.
 */
class IModuleConfigProvider {
public:
  /**
   * Virtual destructor to ensure proper cleanup of derived classes.
   */
  virtual ~IModuleConfigProvider() = default;

  /**
   * Returns the configuration schema for the module, including its name and
   * the options it provides.
   *
   * @returns A ModuleConfig object containing the module's name and its
   * configuration options.
   */
  virtual ModuleConfig getConfigSchema() const = 0;
};

/**
 * Manages the configuration system, including loading from TOML files,
 * parsing command-line arguments, and providing access to resolved values.
 */
class ConfigManager {
private:
  /**
   * Stores the resolved configuration values, organized by section and key.
   */
  std::unordered_map<std::string, std::unordered_map<std::string, ConfigValue>>
      m_values;

  /**
   * Stores the configuration schemas for registered modules, organized by
   * section name.
   */
  std::unordered_map<std::string, std::vector<ConfigOption>> m_schemas;

  /**
   * Maps CLI flags to their corresponding section.key for easy lookup during
   * command-line parsing.  --flag -> "section.key"
   */
  std::unordered_map<std::string, std::string> m_cliFlagMap;

  /**
   * Formats a ConfigValue into a string representation for display in help
   * output and documentation.
   *
   * @param val The ConfigValue to format.
   * @returns A string representation of the value, suitable for display.
   */
  std::string formatValue(const ConfigValue &val) const {
    return std::visit(
        [](auto &&arg) -> std::string {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, bool>)
            return arg ? "true" : "false";
          else if constexpr (std::is_same_v<T, std::string>)
            return "\"" + arg + "\"";
          else
            return std::to_string(arg);
        },
        val);
  }

public:
  /**
   * Type alias for a factory function that creates an instance of
   * IModuleConfigProvider.
   */
  using ProviderFactory =
      std::function<std::unique_ptr<IModuleConfigProvider>()>;

  /**
   * Returns a reference to the static registry of module configuration provider
   * factories, allowing modules to register their configuration schemas at
   * compile time.
   */
  static std::vector<ProviderFactory> &getRegistry() {
    static std::vector<ProviderFactory> registry;
    return registry;
  }

  /**
   * Registers a module's configuration provider factory in the static registry,
   * allowing it to be discovered and included in the final resolved
   * configuration values.
   *
   * @param factory A factory function that returns a unique_ptr to an
   * IModuleConfigProvider instance for the module.
   */
  static void registerStaticModule(ProviderFactory factory) {
    ConfigManager::getRegistry().push_back(factory);
  }

  /**
   * Registers all discovered modules from the static registry, allowing their
   * configuration schemas to be included in the final resolved configuration
   * values. This should be called after all modules have been registered and
   * before loading any configuration files or parsing command-line arguments.
   */
  void registerAllDiscoveredModules() {
    for (const auto &factory : ConfigManager::getRegistry()) {
      auto provider = factory();
      registerModule(*provider);
    }
  }

  /**
   * Registers a module's configuration schema with the ConfigManager, allowing
   * it to be included in the final resolved configuration values.
   */
  void registerModule(const IModuleConfigProvider &provider);

  /**
   * Prints a formatted help menu to the console, including usage instructions
   * and descriptions of all registered configuration options.
   *
   * @param os The output stream to which the help menu will be printed.
   * Defaults to std::cout.
   */
  void printHelp(std::ostream &os) const;

  /**
   * Loads configuration values from a TOML file, parsing the content and
   * applying the values to the appropriate sections and keys.
   */
  bool loadTomlFile(const std::string &fileContent);

  /**
   * Parses command-line arguments, updating configuration values based on the
   * provided flags and their associated values.
   */
  void parseCLI(int argc, char *argv[]);

  /**
   * Retrieves the resolved configuration value for a given section and key,
   * returning it as the specified type T. Throws an exception if the section or
   * key does not exist or if the type does not match.
   */
  template <typename T>
  T get(const std::string &section, const std::string &key) const {
    return std::get<T>(m_values.at(section).at(key));
  }

  /**
   * Dumps the resolved configuration values to the specified output stream,
   * formatting them in a human-readable way for inspection or debugging.
   *
   * @param os The output stream to which the resolved configuration will be
   * dumped. Defaults to std::cout.
   */
  void dumpResolvedConfig(std::ostream &os) const;
};

} // namespace xtrpg::config