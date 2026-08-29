#include "xtrpg/network/TcpConnection.hpp"

#include <sstream>

namespace xtrpg::network {

void TcpConnection::upgrade(asio::ssl::context &ssl_ctx) {

  asio::post(*this->_strand, [this, &ssl_ctx]() {
    if (this->isClosed() || this->isClosing() || this->isSecure()) {
      return;
    }

    this->_sslStream.emplace(std::move(this->_tcpSocket), ssl_ctx);

    this->_sslStream->async_handshake(
        asio::ssl::stream_base::server, [this](std::error_code ec) {
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

void TcpConnection::read(std::function<void(std::istream &)> callback) {
  std::cout << "[TcpConnection] Requesting to read." << std::endl;
  auto buffer = std::make_shared<std::vector<char>>(4096);

  asio::post(*this->_strand, [this, buffer, callback]() {
    std::cout << "[TcpConnection] asio::post." << std::endl;
    if (!this->isOpen()) {
      std::cout << "[TcpConnection] Stream not open." << std::endl;
      throw exception::ConnectionClosed();
    }

    if (this->isSecure() && this->_sslStream) {
      this->_sslStream->async_read_some(
          asio::buffer(*buffer),
          [this, buffer, callback](std::error_code ec,
                                   std::size_t bytes_transferred) {
            if (ec) {
              this->_state = ConnectionState::CLOSED;
              if (this->_sslStream) {
                this->_sslStream->lowest_layer().close();
              }
              return;
            }
            std::string data(buffer->data(), bytes_transferred);
            std::istringstream stream(data);
            callback(stream);
          });

      return;
    }

    this->_tcpSocket.async_read_some(
        asio::buffer(*buffer),
        [this, buffer, callback](std::error_code ec,
                                 std::size_t bytes_transferred) {
          if (ec) {
            this->_state = ConnectionState::CLOSED;
            this->_tcpSocket.close();
            return;
          }
          std::string data(buffer->data(), bytes_transferred);
          std::istringstream stream(data);
          callback(stream);
        });
  });
}

void TcpConnection::write(std::string_view data) {
  auto payload = std::make_shared<std::string>(data);

  asio::post(*this->_strand, [this, payload]() {
    if (!this->isOpen()) {
      throw exception::ConnectionClosed();
    }

    if (this->isSecure() && this->_sslStream) {
      asio::async_write(*this->_sslStream, asio::buffer(*payload),
                        [this, payload](std::error_code ec, std::size_t) {
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
                      [this, payload](std::error_code ec, std::size_t) {
                        if (ec) {
                          this->_state = ConnectionState::CLOSED;
                          this->_tcpSocket.close();
                        }
                      });
  });
}

void TcpConnection::close() {

  asio::post(*this->_strand, [this]() {
    if (this->isClosed() || this->isClosing()) {
      return;
    }

    this->_state = ConnectionState::CLOSING;

    if (this->isSecure() && this->_sslStream) {
      this->_sslStream->lowest_layer().cancel();

      this->_sslStream->async_shutdown([this](const asio::error_code &ec) {
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