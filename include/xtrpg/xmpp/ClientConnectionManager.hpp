#pragma once

#include <algorithm>
#include <asio.hpp>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <vector>

#include "xtrpg/config/ConfigManager.hpp"
#include "xtrpg/interface/Observer.hpp"
#include "xtrpg/network/SocketConnectionListener.hpp"
#include "xtrpg/network/TcpConnection.hpp"
#include "xtrpg/xmpp/session/ClientSession.hpp"

namespace xtrpg::xmpp {
/**
 * Owns active XMPP client sessions and accepts new client connections.
 *
 * The manager owns each session created from an accepted TCP connection and
 * removes it when the session completes. Listener and session callbacks are
 * synchronized with destruction so the manager remains valid while callbacks
 * are in flight.
 */
class ClientConnectionManager
    : public config::IModuleConfigProvider,
      public interface::Observer<network::TcpConnection *> {
public:
  /** Creates an inactive manager without a connection listener. */
  ClientConnectionManager() = default;

  /**
   * Stops accepting connections and releases all owned client sessions.
   *
   * The owner must stop and join every thread calling run() on the supplied
   * I/O context before destroying this manager. The destructor drains pending
   * handlers itself and cannot synchronize with handlers running concurrently
   * on an I/O-context thread it does not own.
   */
  ~ClientConnectionManager();

  /**
   * Starts listening for client connections on the supplied I/O context.
   *
   * @param ioContext I/O context used for asynchronous network operations
   * @param port TCP port on which to accept client connections
   */
  explicit ClientConnectionManager(asio::io_context &ioContext,
                                   uint16_t port = 5222);

  /**
   * Takes ownership of a newly accepted TCP connection and starts its session.
   *
   * @param ctx newly accepted connection; ownership is transferred to the
   * session manager
   */
  void onObservation(network::TcpConnection *ctx) override;

  /**
   * Returns the schema for the client-to-server listener configuration.
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

  /** Returns the number of currently registered client sessions. */
  int countConnections() const {
    std::shared_lock lock(this->_clientSessionsVectorMutex);
    return static_cast<int>(this->_clientSessionPtrs.size());
  }

private:
  /** I/O context used by the listener and client sessions. */
  asio::io_context *_ioContext = nullptr;

  /** Listener that accepts client TCP connections. */
  network::SocketConnectionListener *_ptrSocketConnectionListener = nullptr;

  /** Registry of sessions currently owned by this manager. */
  mutable std::shared_mutex _clientSessionsVectorMutex;
  std::vector<session::ClientSession *> _clientSessionPtrs;

  /** Serializes manager callbacks with shutdown and destruction. */
  mutable std::recursive_mutex _callbackMutex;

  /** Prevents callbacks from entering the manager during destruction. */
  bool _isShuttingDown = false;
};

REGISTER_MODULE_CONFIG(ClientConnectionManager);

} // namespace xtrpg::xmpp
