#pragma once

#include <string>
#include <vector>

#include "xtrpg/config/ConfigManager.hpp"

namespace xtrpg::network {

struct TlsSettings {
  std::string certPath = "./server.crt";
  std::string keyPath = "./server.key";

  static std::vector<xtrpg::config::ConfigOption> configOptions() {
    return {{.key = "cert_path",
             .defaultValue = std::string("./server.crt"),
             .description = "<path> Path to the TLS certificate file"},
            {.key = "key_path",
             .defaultValue = std::string("./server.key"),
             .description = "<path> Path to the TLS private key file"}};
  }
};

/**
 * Shared process-level TLS configuration used for all server connections.
 * Initialize it once during startup and treat it as immutable after that.
 */
extern TlsSettings g_tlsSettings;

void initializeTlsSettings(const TlsSettings &settings);
const TlsSettings &getTlsSettings();

} // namespace xtrpg::network
