#include "xtrpg/xmpp/session/ClientSession.hpp"

#include <iostream>

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
  this->_tokenizer.setListener(nullptr);
}

void ClientSession::start() {
  std::cout << "[ClientSession] Requesting to start." << std::endl;
  if (this->_isShutdown) {
    std::cout << "[ClientSession] Failed to start, already shutdown."
              << std::endl;
    return;
  }

  this->_isStopped.exchange(false);
  this->process();
}

void ClientSession::stop() { this->_isStopped.exchange(true); }

void ClientSession::shutdown() {
  this->stop();
  if (this->_isShutdown.exchange(true)) {
    return;
  }

  // shutdown the TCP connection
  this->_tcpConnection.close();
}

void ClientSession::sendRaw(std::string_view data) {
  if (this->_isShutdown) {
    return;
  }

  this->_tcpConnection << data;
}

void ClientSession::process() {

  std::cout << "[ClientSession] Requesting to process." << std::endl;
  if (this->_isStopped) {
    std::cout << "[ClientSession] Session is stopped." << std::endl;
    return;
  }

  // calls the _tcpConnect to request the next chunk of data
  // the lambda function
  this->_tcpConnection.read([this](std::istream &is) {
    std::cout << "[Client Session] Passing input stream to the tokenizer."
              << std::endl;
    this->_tokenizer.process(is);
    this->process();
  });
}

void ClientSession::openTag(std::string_view tagname) {
  std::cout << "[Client Session] Receive tag: " << tagname << std::endl;

  this->sendRaw(
      "<stream:stream xmlns='jabber:client' "
      "xmlns:stream='http://etherx.jabber.org/streams' id='err-1' "
      "from='example.com' version='1.0'><stream:error><policy-violation "
      "xmlns='urn:ietf:params:xml:ns:xmpp-streams'/><text "
      "xmlns='urn:ietf:params:xml:ns:xmpp-streams' xml:lang='en'>Stanza size "
      "limit of 64KB exceeded.</text></stream:error></stream:stream>");
}
void ClientSession::closeTag() {}
void ClientSession::openDeclaration(std::string_view tagname) {
  // ignore declaration tags
}
void ClientSession::closeDeclaration() {
  // ignore declaration tags
}
void ClientSession::setAttribute(std::string_view name,
                                 std::string_view value) {}
void ClientSession::appendText(std::string_view content) {}
void ClientSession::onError(xml::tokenizer::TokenizationError error) {}
} // namespace xtrpg::xmpp::session