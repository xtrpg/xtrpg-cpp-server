#include <stdexcept>

#include "xtrpg/network/TlsConfig.hpp"

namespace xtrpg::network {

TlsSettings g_tlsSettings{};

void initializeTlsSettings(const TlsSettings &settings) {
  if (settings.certPath.empty() || settings.keyPath.empty()) {
    throw std::invalid_argument(
        "TLS certificate and key paths must not be empty");
  }

  g_tlsSettings = settings;
}

const TlsSettings &getTlsSettings() { return g_tlsSettings; }

} // namespace xtrpg::network
