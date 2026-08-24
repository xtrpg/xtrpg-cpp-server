#include "xtrpg/network/TcpConnection.hpp"

namespace xtrpg::network {

void TcpConnection::upgrade(asio::ssl::context &ssl_ctx) {
  auto self = shared_from_this();
  asio::post(*this->_strand, [this, self, &ssl_ctx]() {
    if (this->isClosed() || this->isClosing() || this->isSecure()) {
      return;
    }

    this->_sslStream.emplace(std::move(this->_tcpSocket), ssl_ctx);

    this->_sslStream->async_handshake(
        asio::ssl::stream_base::server, [this, self](std::error_code ec) {
          if (ec) {
            this->_state = ConnectionState::CLOSED;
            if (this->_sslStream) {
              this->_sslStream->lowest_layer().close();
            }
            return;
          }
          this->_state = ConnectionState::SECURE;
        });
  });
}

void TcpConnection::write(std::string_view data) {
  auto payload = std::make_shared<std::string>(data);
  auto self = shared_from_this();

  asio::post(*this->_strand, [this, self, payload]() {
    if (!this->isOpen()) {
      throw exception::ConnectionClosed();
    }

    if (this->isSecure() && this->_sslStream) {
      asio::async_write(*this->_sslStream, asio::buffer(*payload),
                        [this, self, payload](std::error_code ec, std::size_t) {
                          if (ec) {
                            this->_state = ConnectionState::CLOSED;
                            if (this->_sslStream) {
                              this->_sslStream->lowest_layer().close();
                            }
                          }
                        });
      return;
    }

    asio::async_write(this->_tcpSocket, asio::buffer(*payload),
                      [this, self, payload](std::error_code ec, std::size_t) {
                        if (ec) {
                          this->_state = ConnectionState::CLOSED;
                          this->_tcpSocket.close();
                        }
                      });
  });
}

void TcpConnection::close() {
  auto self = shared_from_this();
  asio::post(*this->_strand, [this, self]() {
    if (this->isClosed() || this->isClosing()) {
      return;
    }

    this->_state = ConnectionState::CLOSING;

    if (this->isSecure() && this->_sslStream) {
      this->_sslStream->lowest_layer().cancel();

      this->_sslStream->async_shutdown(
          [this, self](const asio::error_code &ec) {
            if (this->_sslStream) {
              this->_sslStream->lowest_layer().shutdown(
                  asio::ip::tcp::socket::shutdown_both);
              this->_sslStream->lowest_layer().close();
            }

            this->_state = ConnectionState::CLOSED;
          });
      return;
    }

    std::error_code ec;
    this->_tcpSocket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    this->_tcpSocket.close();
    this->_state = ConnectionState::CLOSED;
  });
}

} // namespace xtrpg::network