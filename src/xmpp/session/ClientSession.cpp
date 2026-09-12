#include "xtrpg/xmpp/session/ClientSession.hpp"

#include "xtrpg/utils/String.hpp"
#include "xtrpg/xmpp/stream/NegotiationStreamHandler.hpp"
#include "xtrpg/xmpp/stream/UnimplementedStreamHandler.hpp"

namespace xtrpg::xmpp::session {

ClientSession::ClientSession(network::TcpConnection *tcpConnection)
    : _ptrTcpConnection(tcpConnection) {
  this->_tokenizer.setObserver(this);
}

ClientSession::~ClientSession() {
  delete this->_ptrRootStreamNode;
  delete this->_ptrDeclarationNode;
  this->_tokenizer.setObserver(nullptr);
  delete this->_ptrTcpConnection;
  this->_ptrTcpConnection = nullptr;

  if (nullptr != this->_ptrCurrentXmlNode) {
    std::lock_guard lock(this->_currentXmlNodeMutex);
    delete this->_ptrCurrentXmlNode;
    this->_ptrCurrentXmlNode = nullptr;
  }
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

void ClientSession::send(const xml::node::INode &xmlNode) {
  std::ostringstream oss;
  oss << xmlNode;
  this->sendRaw(oss.str());
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

  std::cout << "Incoming XML Token" << std::endl;
  std::cout << "          Content: " << xmlToken.content << std::endl;

  // Ignore any comment tokens
  if (xml::tokenizer::TokenType::COMMENT == xmlToken.type) {
    return;
  }

  // Are we waiting for the client to start a new stream?
  if (nullptr == this->_ptrActiveStreamHandler) {
    // ignore declaration tokens
    if (xml::tokenizer::TokenType::DECLARATION == xmlToken.type) {
      return;
    }

    // if the incoming token is text content and blank (only contains whitespace
    // and newlines) or empty then ignore and return immediately.
    if (xml::tokenizer::TokenType::TEXT_CONTENT == xmlToken.type &&
        xtrpg::utils::string::isBlank(xmlToken.content)) {
      return;
    }

    // if it's not an opening tag, then it's not the start of a stream
    if (xml::tokenizer::TokenType::OPEN_TAG != xmlToken.type ||
        "stream:stream" != xmlToken.content) {
      // return a malformed xml stream error
      this->sendRaw(
          "<stream:stream "
          "xmlns:stream='http://etherx.jabber.org/"
          "streams'><stream:error><bad-format "
          "xmlns='urn:ietf:params:xml:ns:xmpp-streams'/><text "
          "xmlns='urn:ietf:params:xml:ns:xmpp-streams'>First element must be "
          "an opening stream header.</text></stream:error></stream:stream>");
      this->shutdown();
      return;
    }

    // determine which stream handler to activate
    if (!this->_ptrTcpConnection->isSecure()) {
      // start the negotiation phase
      std::lock_guard lock(this->_activeStreamHandlerMutex);
      this->_ptrActiveStreamHandler =
          &stream::NegotiationStreamHandler::instance();
      this->_ptrActiveStreamHandler->onStart(*this);
      return;
    }

    // if not authenticated
    // start the authentication phase

    // start the binded phase
    std::lock_guard lock(this->_activeStreamHandlerMutex);
    this->_ptrActiveStreamHandler =
        &stream::UnimplementedStreamHandler::instance();
    this->_ptrActiveStreamHandler->onStart(*this);
    return;
  }

  // Are we parsing the root stream:stream node?
  if (nullptr == this->_ptrCurrentXmlNode) {

    // Are we ending the current stream
    if (xml::tokenizer::TokenType::CLOSE_TAG == xmlToken.type &&
        "stream:stream" == xmlToken.content) {
      this->sendRaw("</stream:stream>");
      this->setActiveStreamHandler(nullptr);
      return;
    }

    // if (xml::tokenizer::TokenType::EMPTY_TAG != xmlToken.type) {
    // process the node and dispatch to handler.
    // return;
    // }

    // if (xml::tokenizer::TokenType::OPEN_TAG != xmlToken.type) { malformed
    // stream error close the stream.
    // return;
    // }

    // create the new xml node.
    // return;
  }

  // From here on down we are parsing a node

  if (xml::tokenizer::TokenType::CLOSE_TAG == xmlToken.type) {
    // if the current node != this close tag:
    // - then return a malformed error and close stream.
    // - return

    // if the current node does not have a parent node (ie parent == nullptr)
    // - then dispatch the current node to the handler
    // - set current node to nullptr
    // - return

    // set the parent of the current node to be the new current node.
    // return
  }

  if (xml::tokenizer::TokenType::EMPTY_TAG == xmlToken.type) {
    // append an empty node to the current node.
    // return
  }

  if (xml::tokenizer::TokenType::TEXT_CONTENT == xmlToken.type) {
    // append text content to the current node
    // return
  }

  if (xml::tokenizer::TokenType::OPEN_TAG == xmlToken.type) {
    // create a new node
    // append the new node to the current node
    // set the current node to be the new node
    // return
  }

  // return a malformed XML stream.

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

  std::cout << "UNABLE TO PROCESS INCOMING XML TOKEN" << std::endl;
}

void ClientSession::onTokenizationError(
    const xml::tokenizer::TokenizationError &error) {}

void ClientSession::setActiveStreamHandler(
    const stream::StreamHandler *streamHandler) {
  std::lock_guard lock(this->_activeStreamHandlerMutex);
  this->_ptrActiveStreamHandler = streamHandler;
}

const stream::StreamHandler *ClientSession::getActiveStreamHandler() const {
  std::lock_guard lock(this->_activeStreamHandlerMutex);
  return this->_ptrActiveStreamHandler;
}

} // namespace xtrpg::xmpp::session
