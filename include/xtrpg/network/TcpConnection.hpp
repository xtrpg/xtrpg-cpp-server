#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <iostream>
#include <memory>
#include <optional>

#include "xtrpg/network/exception/ConnectionClosed.hpp"

namespace xtrpg::network {

/**
 * Represents the state of a TCP connection.
 */
enum class ConnectionState {
  /**
   * The connection is closed.
   */
  CLOSED,
  /**
   * The connection is closing.
   */
  CLOSING,
  /**
   * The connection is secure (SSL/TLS).
   */
  SECURE,
  /**
   * The connection is insecure (TCP).
   */
  INSECURE
};

/**
 * Represents a TCP connection that can be upgraded to TLS. It provides methods
 * to write data to the connection, read data from the connection and close it.
 */
class TcpConnection {

public:
  /**
   * Constructs a TcpConnection with the given TCP socket.
   */
  explicit TcpConnection(asio::ip::tcp::socket tcpSocket)
      : _tcpSocket(std::move(tcpSocket)) {
    this->_strand.emplace(asio::make_strand(this->_tcpSocket.get_executor()));
  }

  /**
   * Upgrades the TCP connection to a TLS connection using the provided SSL
   * context.
   */
  void upgrade(asio::ssl::context &ssl_ctx);

  /**
   * Async read from the underlying tcp connection, calling the provided lambda
   * function with a new istream of the incoming stream data.
   */
  void read(std::function<void(std::istream &)> callback);

  /**
   * Writes data to the connection. If the connection is closed, it will throw
   * an exception.
   */
  void write(std::string_view data);

  /**
   * Closes the connection. If the connection is already closed, it will do
   * nothing.
   */
  void close();

  /**
   * Checks if the connection is secure (SSL/TLS).
   */
  bool isSecure() const { return ConnectionState::SECURE == this->_state; }

  /**
   * Checks if the connection is closing.
   */
  bool isClosing() const { return ConnectionState::CLOSING == this->_state; }

  /**
   * Checks if the connection is open.
   */
  bool isOpen() const {
    return ConnectionState::CLOSED != this->_state &&
           ConnectionState::CLOSING != this->_state;
  }

  /**
   * Checks if the connection is closed.
   */
  bool isClosed() const { return ConnectionState::CLOSED == this->_state; }

  /**
   * Asserts that the connection is open.If the connection is closed,
   * it will throw an exception.
   */
  void assertOpenConnection() const {
    if (!this->isOpen()) {
      throw exception::ConnectionClosed();
    }
  }

  /**
   * Stream writer to write data to the socket.
   */
  TcpConnection &operator<<(std::string_view str) {
    this->write(str);
    return *this;
  }

private:
  /**
   * The current state of the connection.
   */
  ConnectionState _state{ConnectionState::INSECURE};

  /**
   * The underlying TCP socket used for the connection.
   */
  asio::ip::tcp::socket _tcpSocket;

  /**
   * Serializes access to the socket and connection state so writes, upgrades,
   * and closes cannot race against each other.
   */
  std::optional<asio::strand<asio::any_io_executor>> _strand;

  /**
   * The optional SSL stream used for secure communication. It is only
   * initialized when the connection is upgraded to TLS. If the connection is
   * not secure, this will be std::nullopt.
   */
  std::optional<asio::ssl::stream<asio::ip::tcp::socket>> _sslStream;
};
} // namespace xtrpg::network