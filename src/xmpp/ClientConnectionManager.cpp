#include "xtrpg/xmpp/ClientConnectionManager.hpp"

namespace xtrpg::xmpp {

ClientConnectionManager::~ClientConnectionManager() {
  // Loop over the `this->_clientSessionPtrs` vector, shut them down and delete
  // the instances
  for (auto ptrSession : this->_clientSessionPtrs) {
    if (ptrSession) {
      ptrSession->shutdown();
      delete ptrSession;
    }
  }
  this->_clientSessionPtrs.clear();
}

/**
 * A new Tcp Connection is created.
 */
void ClientConnectionManager::onObservation(
    std::shared_ptr<network::TcpConnection> &ctx) {
  std::cout << "[ClientConnectionManager] New client connection received"
            << std::endl;

  // Create a ClientSession for this connection
  auto clientSession = new session::ClientSession(std::move(*ctx));
  this->_clientSessionPtrs.push_back(clientSession);

  // Start the session
  clientSession->start();

  std::cout << "[ClientConnectionManager] ClientSession created and started"
            << std::endl;
}

} // namespace xtrpg::xmpp