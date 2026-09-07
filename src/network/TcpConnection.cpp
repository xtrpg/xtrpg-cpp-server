#include "xtrpg/network/TcpConnection.hpp"

#include <sstream>

namespace xtrpg::network {

void TcpConnection::dispatchCloseCallbacks() {
  auto callbacks = std::move(this->_closeCallbacks);
  for (auto &callback : callbacks) {
    if (callback) {
      callback();
    }
  }
}

void TcpConnection::upgrade(asio::ssl::context &ssl_ctx) {
  if (!this->isOpen()) {
    return;
  }

  asio::post(*this->_strand, [this, &ssl_ctx]() {
    if (this->isClosed() || this->isClosing() || this->isSecure()) {
      return;
    }

    this->_sslStream.emplace(std::move(this->_tcpSocket), ssl_ctx);

    this->_sslStream->async_handshake(
        asio::ssl::stream_base::server, [this](std::error_code ec) {
          if (ec) {
            this->close();
            return;
          }

          this->dispatchStateChange(ConnectionState::SECURE);
        });
  });
}

void TcpConnection::read(
    std::function<void(const std::error_code &, std::istream &)> callback) {
  if (!this->isOpen()) {
    // Preserve read's callback contract even when the socket closed before
    // the asynchronous operation could be posted.
    std::istringstream stream;
    callback(asio::error::operation_aborted, stream);
    return;
  }

  auto buffer = std::make_shared<std::vector<char>>(4096);
  asio::post(*this->_strand, [this, buffer, callback]() {
    if (!this->isOpen()) {
      // The connection may close after the caller's initial state check but
      // before this strand handler begins executing.
      std::istringstream stream;
      callback(asio::error::operation_aborted, stream);
      return;
    }

    if (this->isSecure() && this->_sslStream) {
      this->_sslStream->async_read_some(
          asio::buffer(*buffer),
          [this, buffer, callback](std::error_code ec,
                                   std::size_t bytes_transferred) {
            std::istringstream stream;
            if (ec) {
              this->close([callback, ec]() {
                std::istringstream errorStream;
                callback(ec, errorStream);
              });
              return;
            }
            std::string data(buffer->data(), bytes_transferred);
            stream.str(data);
            callback(ec, stream);
          });

      return;
    }

    this->_tcpSocket.async_read_some(
        asio::buffer(*buffer),
        [this, buffer, callback](std::error_code ec,
                                 std::size_t bytes_transferred) {
          std::istringstream stream;
          if (ec) {
            this->close([callback, ec]() {
              std::istringstream errorStream;
              callback(ec, errorStream);
            });
            return;
          }
          std::string data(buffer->data(), bytes_transferred);
          stream.str(data);
          callback(ec, stream);
        });
  });
}

void TcpConnection::cancelRead() {
  asio::post(*this->_strand, [this]() {
    if (this->isOpen()) {
      if (this->isSecure() && this->_sslStream) {
        this->_sslStream->lowest_layer().cancel();
      } else {
        this->_tcpSocket.cancel();
      }
    }
  });
}

void TcpConnection::write(std::string_view data) {
  if (!this->isOpen()) {
    return;
  }
  auto payload = std::make_shared<std::string>(data);

  asio::post(*this->_strand, [this, payload]() {
    if (!this->isOpen()) {
      return;
    }

    if (this->isSecure() && this->_sslStream) {
      asio::async_write(*this->_sslStream, asio::buffer(*payload),
                        [this, payload](std::error_code ec, std::size_t) {
                          if (ec) {
                            this->close();
                          }
                        });
      return;
    }

    asio::async_write(this->_tcpSocket, asio::buffer(*payload),
                      [this, payload](std::error_code ec, std::size_t) {
                        if (ec) {
                          this->close();
                        }
                      });
  });
}

void TcpConnection::close(std::function<void()> callback) {
  if (this->is(ConnectionState::CLOSED) || this->is(ConnectionState::CLOSING)) {
    if (this->isClosed() && callback) {
      callback();
    } else if (this->isClosing() && callback) {
      this->_closeCallbacks.push_back(std::move(callback));
    }
    return;
  }

  if (callback) {
    this->_closeCallbacks.push_back(std::move(callback));
  }

  // Set the state to closing.
  this->dispatchStateChange(ConnectionState::CLOSING);

  // Serialize transport shutdown with reads and writes on the strand.
  asio::post(*this->_strand, [this]() {
    if (this->isSecure() && this->_sslStream) {
      this->_sslStream->lowest_layer().cancel();

      this->_sslStream->async_shutdown([this](const asio::error_code &ec) {
        if (this->_sslStream) {
          this->_sslStream->lowest_layer().shutdown(
              asio::ip::tcp::socket::shutdown_both);
          this->_sslStream->lowest_layer().close();
        }

        this->dispatchStateChange(ConnectionState::CLOSED);
        this->dispatchCloseCallbacks();
      });
      return;
    }

    std::error_code ec;
    this->_tcpSocket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    this->_tcpSocket.close();
    this->dispatchStateChange(ConnectionState::CLOSED);
    this->dispatchCloseCallbacks();
  });
}

} // namespace xtrpg::network