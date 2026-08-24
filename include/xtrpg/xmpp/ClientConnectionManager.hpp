#pragma once

#include <asio.hpp>

#include "xtrpg/config/ConfigManager.hpp"

namespace xtrpg::xmpp {
/**
 *
 */
class ClientConnectionManager : public config::IModuleConfigProvider {
public:
  ClientConnectionManager() = default;

  explicit ClientConnectionManager(asio::io_context &ioContext)
      : _ioContext(&ioContext) {}

  /**
   *
   */
  config::ModuleConfig getConfigSchema() const {
    return {.name = "c2s",
            .description = "",
            .options = {{.key = "port",
                         .defaultValue = 5222,
                         .description =
                             "<value> Port number that this server will listen "
                             "on for Client (or C2S) connections."}}};
  }

private:
  asio::io_context *_ioContext = nullptr;
};

REGISTER_MODULE_CONFIG(ClientConnectionManager);

} // namespace xtrpg::xmpp
