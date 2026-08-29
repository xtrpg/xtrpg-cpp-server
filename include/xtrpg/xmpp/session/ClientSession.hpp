#pragma once

#include <atomic>
#include <memory>

#include "xtrpg/network/TcpConnection.hpp"
#include "xtrpg/xml/node/DeclarationNode.hpp"
#include "xtrpg/xml/node/TagNode.hpp"
#include "xtrpg/xml/tokenizer/TokenizationError.hpp"
#include "xtrpg/xml/tokenizer/XmlStreamTokenizer.hpp"
#include "xtrpg/xml/tokenizer/XmlToken.hpp"
#include "xtrpg/xml/tokenizer/XmlTokenListener.hpp"

namespace xtrpg::xmpp::session {

class ClientSession : public xml::tokenizer::XmlTokenListener,
                      public std::enable_shared_from_this<ClientSession> {

public:
  ClientSession(network::TcpConnection tcpConnection)
      : _tcpConnection(std::move(tcpConnection)) {
    std::cout << "[ClientSession] New Instance created." << std::endl;
    this->_tokenizer.setObserver(this);
  }
  ~ClientSession();

  // Session Control
  void start();
  void stop();
  void shutdown();
  void process();

  // Transport Control
  void sendRaw(std::string_view data);

  // Tokenizer Calls
  void onObservation(const xml::tokenizer::XmlToken &xmlToken);
  void onObservation(const xml::tokenizer::TokenizationError &error);

  void onError(xml::tokenizer::TokenizationError error);

private:
  network::TcpConnection _tcpConnection;
  xml::tokenizer::XmlStreamTokenizer _tokenizer;

  xml::node::DeclarationNode *_ptrDeclarationNode = nullptr;
  xml::node::TagNode *_ptrRootStreamNode = nullptr;

  /**
   * Boolean flag that indicates whether the session is actively processing
   * data to/from the underling connection.
   */
  std::atomic<bool> _isStopped{true};

  /**
   * Boolean flag that indicates whether the underlying connection has been
   * terminated.
   */
  std::atomic<bool> _isShutdown{false};
};
} // namespace xtrpg::xmpp::session