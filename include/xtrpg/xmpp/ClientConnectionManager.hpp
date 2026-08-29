#pragma once

#include <asio.hpp>
#include <memory>
#include <vector>

#include "xtrpg/config/ConfigManager.hpp"
#include "xtrpg/interface/Observer.hpp"
#include "xtrpg/network/TcpConnection.hpp"
#include "xtrpg/xmpp/session/ClientSession.hpp"

namespace xtrpg::xmpp {
/**
 *
 */
class ClientConnectionManager
    : public config::IModuleConfigProvider,
      public interface::Observer<std::shared_ptr<network::TcpConnection>> {
public:
  ClientConnectionManager() = default;

  ~ClientConnectionManager();

  explicit ClientConnectionManager(asio::io_context &ioContext)
      : _ioContext(&ioContext) {}

  /**
   * A new Tcp Connection is created.
   */
  void onObservation(std::shared_ptr<network::TcpConnection> &ctx);

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

  std::vector<session::ClientSession *> _clientSessionPtrs;
};

REGISTER_MODULE_CONFIG(ClientConnectionManager);

} // namespace xtrpg::xmpp
