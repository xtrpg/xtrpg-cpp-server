#include "xtrpg/xmpp/session/ClientSession.hpp"

namespace xtrpg::xmpp::session {

ClientSession::~ClientSession() {
  delete this->_ptrRootStreamNode;
  delete this->_ptrDeclarationNode;
  this->_tokenizer.setObserver(nullptr);
  delete this->_ptrTcpConnection;
  this->_ptrTcpConnection = nullptr;
}

void ClientSession::start() {
  if (this->_isShutdown) {
    return;
  }
  this->_isStopped = false;
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
  this->_ptrTcpConnection->close([this]() { this->notifyCompletion(); });
}

void ClientSession::sendRaw(std::string_view data) {
  if (!this->_isShutdown) {
    *this->_ptrTcpConnection << data;
  }
}

void ClientSession::process() {
  if (this->_isStopped || this->_isShutdown) {
    return;
  }
  this->_ptrTcpConnection->read(
      [this](const std::error_code &error, std::istream &stream) {
        if (error || this->_isStopped || this->_isShutdown) {
          this->notifyCompletion();
          return;
        }
        this->_tokenizer.process(stream);
        this->process();
      });
}

void ClientSession::notifyCompletion() {
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
