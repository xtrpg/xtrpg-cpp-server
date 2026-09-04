#include "xtrpg/xmpp/ClientConnectionManager.hpp"

namespace xtrpg::xmpp {

ClientConnectionManager::ClientConnectionManager(asio::io_context &ioContext,
                                                 uint16_t port)
    : _ioContext(&ioContext) {

  // Create the listener that accepts incoming client TCP connections.
  this->_ptrSocketConnectionListener =
      new network::SocketConnectionListener(this->_ioContext, port);

  // Route newly accepted connections to this manager.
  this->_ptrSocketConnectionListener->setObserver(this);

  // Begin accepting client connections on the configured port.
  this->_ptrSocketConnectionListener->start();
}

ClientConnectionManager::~ClientConnectionManager() {

  // Stop accepting new connections and detach the observer before destruction.
  if (nullptr != this->_ptrSocketConnectionListener) {
    // Prevent new accept completions from entering the manager while it is
    // dismantling the sessions it already owns.
    this->_ptrSocketConnectionListener->stop();
    this->_ptrSocketConnectionListener->setObserver(nullptr);
  }

  // Move the sessions out while holding the mutex so no concurrent operation
  // can access the manager's vector during shutdown.
  {
    std::lock_guard lock(this->_callbackMutex);
    this->_isShuttingDown = true;
  }
  std::vector<session::ClientSession *> sessions;
  {
    std::unique_lock lock(this->_clientSessionsVectorMutex);
    sessions.swap(this->_clientSessionPtrs);
  }

  // Shut down and delete sessions after releasing the mutex because shutdown
  // may trigger callbacks that access the manager.
  for (auto *ptrSession : sessions) {
    if (ptrSession) {
      // Completion callbacks must not mutate the registry while the manager
      // is being destroyed; the destructor owns cleanup from this point on.
      ptrSession->setCompletionCallback(nullptr);
      ptrSession->shutdown();
    }
  }
  if (this->_ioContext != nullptr) {
    // Run cancellation and close handlers while the manager and sessions are
    // still alive. This drains callbacks that capture their raw addresses.
    this->_ioContext->restart();
    this->_ioContext->run();
  }
  for (auto *ptrSession : sessions) {
    // All handlers have completed, so deleting the sessions is now safe.
    delete ptrSession;
  }

  delete this->_ptrSocketConnectionListener;
  this->_ptrSocketConnectionListener = nullptr;
}

/**
 * A new Tcp Connection is created.
 */
void ClientConnectionManager::onObservation(network::TcpConnection *ctx) {
  std::lock_guard callbackLock(this->_callbackMutex);
  if (this->_isShuttingDown) {
    delete ctx;
    return;
  }

  std::cout << "[ClientConnectionManager] New client connection received"
            << std::endl;

  // Wrap the new TCP connection in a session owned by this manager.
  auto clientSession = new session::ClientSession(ctx);
  {
    // Protect the session registry from concurrent connection callbacks.
    std::unique_lock lock(this->_clientSessionsVectorMutex);
    this->_clientSessionPtrs.push_back(clientSession);
  }

  // Stop the session when its underlying connection closes. Destruction is
  // deferred until the pending read handler has completed.
  ctx->appendStateChangeCallback(
      [this, clientSession](network::ConnectionState state) {
        std::lock_guard lock(this->_callbackMutex);
        if (this->_isShuttingDown) {
          return;
        }
        std::cout << "[ClientConnectionManager] State Change: "
                  << (network::ConnectionState::CLOSED == state    ? "CLOSED"
                      : state == network::ConnectionState::CLOSING ? "CLOSING"
                      : state == network::ConnectionState::SECURE  ? "SECURE"
                                                                   : "INSECURE")
                  << std::endl;
        if (network::ConnectionState::CLOSED == state) {
          // Stop scheduling reads; completion still comes from the pending
          // read or close callback and removes the session from the registry.
          clientSession->stop();
        }
      });

  clientSession->setCompletionCallback([this](session::ClientSession *session) {
    std::lock_guard callbackLock(this->_callbackMutex);
    // Shutdown suppresses registry mutation because the destructor already
    // removed every session from the registry before draining the context.
    if (this->_isShuttingDown) {
      return;
    }

    std::unique_lock lock(this->_clientSessionsVectorMutex);
    const auto sessionIt = std::find(this->_clientSessionPtrs.begin(),
                                     this->_clientSessionPtrs.end(), session);
    if (sessionIt == this->_clientSessionPtrs.end()) {
      return;
    }

    this->_clientSessionPtrs.erase(sessionIt);
    lock.unlock();
    // Let the transport callback return before destroying the session and its
    // owned connection; TcpConnection may still be dispatching close callbacks.
    asio::post(*this->_ioContext, [session]() { delete session; });
  });

  // Start reading and processing data for the new client.
  clientSession->start();

  std::cout << "[ClientConnectionManager] ClientSession created and started"
            << std::endl;
}

} // namespace xtrpg::xmpp