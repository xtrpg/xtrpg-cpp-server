#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string_view>
#include <utility>

#include "xtrpg/network/TcpConnection.hpp"
#include "xtrpg/xml/node/DeclarationNode.hpp"
#include "xtrpg/xml/node/TagNode.hpp"
#include "xtrpg/xml/tokenizer/TokenizationError.hpp"
#include "xtrpg/xml/tokenizer/XmlStreamTokenizer.hpp"
#include "xtrpg/xml/tokenizer/XmlToken.hpp"
#include "xtrpg/xml/tokenizer/XmlTokenListener.hpp"

namespace xtrpg::xmpp::session {

/** Coordinates XML tokenization and transport I/O for one XMPP client. */
class ClientSession : public xml::tokenizer::XmlTokenListener {
public:
  /**
   * Creates a session for an already accepted TCP connection.
   *
   * @param tcpConnection connection transferred to the new session; must not
   * be null
   */
  explicit ClientSession(network::TcpConnection *tcpConnection)
      : _ptrTcpConnection(tcpConnection) {
    this->_tokenizer.setObserver(this);
  }

  /** Stops token processing and releases the owned connection and XML state. */
  ~ClientSession();

  /** Starts reading and processing data from the client. */
  void start();

  /** Prevents the session from scheduling another read. */
  void stop();

  /** Registers a callback invoked once the pending read has completed. */
  void setCompletionCallback(std::function<void(ClientSession *)> callback) {
    std::lock_guard lock(this->_completionCallbackMutex);
    this->_completionCallback = std::move(callback);
  }

  /** Stops processing and closes the underlying connection. */
  void shutdown();

  /** Schedules the next asynchronous read when the session is active. */
  void process();

  /** Returns whether the owned connection has reached the closed state. */
  bool isClosed() const { return this->_ptrTcpConnection->isClosed(); }

  /** Asynchronously writes raw XML or other protocol data to the client. */
  void sendRaw(std::string_view data);

  /** Handles one token emitted by the XML stream tokenizer. */
  void onXmlToken(const xml::tokenizer::XmlToken &xmlToken);

  /** Handles a tokenizer error reported for this client stream. */
  void onTokenizationError(const xml::tokenizer::TokenizationError &error);

private:
  /** TCP connection owned by this session. */
  network::TcpConnection *_ptrTcpConnection;

  /** Stateful tokenizer that retains XML data between network reads. */
  xml::tokenizer::XmlStreamTokenizer _tokenizer;

  /** Parsed XML declaration, when one is retained by the session. */
  xml::node::DeclarationNode *_ptrDeclarationNode = nullptr;

  /** Root XMPP stream node, when one is retained by the session. */
  xml::node::TagNode *_ptrRootStreamNode = nullptr;

  /** Indicates whether the session is actively processing data. */
  std::atomic<bool> _isStopped{true};

  /** Indicates whether the underlying connection has terminated. */
  std::atomic<bool> _isShutdown{false};

  /** Protects the completion callback while it is being registered/read. */
  mutable std::mutex _completionCallbackMutex;
  std::function<void(ClientSession *)> _completionCallback;

  /** Ensures completion is reported at most once. */
  std::atomic<bool> _completionNotified{false};

  /** Notifies the manager that the session has completed. */
  void notifyCompletion();
};
} // namespace xtrpg::xmpp::session
