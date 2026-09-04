#include "xtrpg/xmpp/ClientConnectionManager.hpp"

namespace xtrpg::xmpp {

ClientConnectionManager::ClientConnectionManager(asio::io_context &ioContext,
                                                 uint16_t port)
    : _ioContext(&ioContext) {
  this->_ptrSocketConnectionListener =
      new network::SocketConnectionListener(this->_ioContext, port);
  this->_ptrSocketConnectionListener->setObserver(this);
  this->_ptrSocketConnectionListener->start();
}

ClientConnectionManager::~ClientConnectionManager() {
  if (this->_ptrSocketConnectionListener != nullptr) {
    this->_ptrSocketConnectionListener->stop();
    this->_ptrSocketConnectionListener->setObserver(nullptr);
  }

  {
    std::lock_guard lock(this->_callbackMutex);
    this->_isShuttingDown = true;
  }

  std::vector<session::ClientSession *> sessions;
  {
    std::unique_lock lock(this->_clientSessionsVectorMutex);
    sessions.swap(this->_clientSessionPtrs);
  }

  for (auto *ptrSession : sessions) {
    if (ptrSession != nullptr) {
      ptrSession->setCompletionCallback(nullptr);
      ptrSession->shutdown();
    }
  }

  if (this->_ioContext != nullptr) {
    this->_ioContext->restart();
    this->_ioContext->run();
  }

  for (auto *ptrSession : sessions) {
    delete ptrSession;
  }

  delete this->_ptrSocketConnectionListener;
  this->_ptrSocketConnectionListener = nullptr;
}

void ClientConnectionManager::onObservation(network::TcpConnection *ctx) {
  std::lock_guard callbackLock(this->_callbackMutex);
  if (this->_isShuttingDown) {
    delete ctx;
    return;
  }

  auto *clientSession = new session::ClientSession(ctx);
  {
    std::unique_lock lock(this->_clientSessionsVectorMutex);
    this->_clientSessionPtrs.push_back(clientSession);
  }

  ctx->appendStateChangeCallback(
      [this, clientSession](network::ConnectionState state) {
        std::lock_guard lock(this->_callbackMutex);
        if (!this->_isShuttingDown &&
            state == network::ConnectionState::CLOSED) {
          clientSession->stop();
        }
      });

  clientSession->setCompletionCallback([this](session::ClientSession *session) {
    std::lock_guard callbackLock(this->_callbackMutex);
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
    asio::post(*this->_ioContext, [session]() { delete session; });
  });

  clientSession->start();
}

} // namespace xtrpg::xmpp
