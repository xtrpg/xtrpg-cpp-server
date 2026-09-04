#include "xtrpg/xmpp/session/ClientSession.hpp"

#include <iostream>

#include "xtrpg/xml/tokenizer/XmlToken.hpp"

namespace xtrpg::xmpp::session {

ClientSession::~ClientSession() {
  // destroy the root stream node
  if (this->_ptrRootStreamNode != nullptr) {
    delete this->_ptrRootStreamNode;
    this->_ptrRootStreamNode = nullptr;
  }

  // destroy the xml declaration node
  if (this->_ptrDeclarationNode != nullptr) {
    delete this->_ptrDeclarationNode;
    this->_ptrDeclarationNode = nullptr;
  }

  // remove myself from the tokenizer
  this->_tokenizer.setObserver(nullptr);

  // destroy the TCP connection
  delete this->_ptrTcpConnection;
  this->_ptrTcpConnection = nullptr;
}

void ClientSession::start() {
  if (this->_isShutdown) {
    return;
  }

  this->_isStopped.exchange(false);
  this->process();
}

void ClientSession::stop() {
  if (!this->_isStopped.exchange(true)) {
    this->_ptrTcpConnection->cancelRead();
  }
}

void ClientSession::shutdown() {
  this->stop();
  if (this->_isShutdown.exchange(true)) {
    return;
  }

  // Notify the manager after the connection's close work has completed. This
  // also covers shutdown initiated from a successful read callback.
  this->_ptrTcpConnection->close([this]() { this->notifyCompletion(); });
}

void ClientSession::sendRaw(std::string_view data) {
  if (this->_isShutdown) {
    return;
  }

  *this->_ptrTcpConnection << data;
}

void ClientSession::process() {
  if (this->_isStopped || this->_isShutdown) {
    return;
  }

  // Each read either schedules the next read or reports completion, so a
  // closed connection cannot leave the session registered indefinitely.
  this->_ptrTcpConnection->read(
      [this](const std::error_code &error, std::istream &stream) {
        if (error || this->_isStopped || this->_isShutdown) {
          // Errors, cancellation, and explicit shutdown all end the session.
          this->notifyCompletion();
          return;
        }
        this->_tokenizer.process(stream);
        this->process();
      });
}

void ClientSession::notifyCompletion() {
  // Multiple terminal events can race; only the first one may notify the
  // manager and trigger deletion.
  if (this->_completionNotified.exchange(true)) {
    return;
  }

  std::function<void(ClientSession *)> completionCallback;
  {
    std::lock_guard lock(this->_completionCallbackMutex);
    completionCallback = this->_completionCallback;
  }

  if (completionCallback) {
    completionCallback(this);
  }
}

void ClientSession::onXmlToken(const xml::tokenizer::XmlToken &xmlToken) {
  if (xml::tokenizer::TokenType::OPEN_TAG == xmlToken.type &&
      "stream:stream" == xmlToken.content) {
    this->sendRaw(
        "<stream:stream xmlns='jabber:client' "
        "xmlns:stream='http://etherx.jabber.org/streams' id='err-1' "
        "from='example.com' version='1.0'><stream:error><policy-violation "
        "xmlns='urn:ietf:params:xml:ns:xmpp-streams'/><text "
        "xmlns='urn:ietf:params:xml:ns:xmpp-streams' xml:lang='en'>Stanza size "
        "limit of 64KB exceeded.</text></stream:error></stream:stream>");

    this->shutdown();
  }
}
void ClientSession::onTokenizationError(
    const xml::tokenizer::TokenizationError &error) {}

} // namespace xtrpg::xmpp::session