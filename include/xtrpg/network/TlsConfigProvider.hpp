#pragma once

#include <string>

#include "xtrpg/config/ConfigManager.hpp"
#include "xtrpg/network/TlsConfig.hpp"

namespace xtrpg::network {

/**
 * Configuration provider for XMPP TLS settings, including certificate and key
 * file paths.
 */
class TlsConfigProvider : public xtrpg::config::IModuleConfigProvider {
public:
  /** Returns the TLS configuration schema. */
  xtrpg::config::ModuleConfig getConfigSchema() const override {
    return {.name = "tls",
            .description = "TLS/SSL configuration for XMPP server",
            .options = TlsSettings::configOptions()};
  }
};

REGISTER_MODULE_CONFIG(TlsConfigProvider);

} // namespace xtrpg::network
