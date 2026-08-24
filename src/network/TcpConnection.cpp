#include "xtrpg/network/TcpConnection.hpp"

namespace xtrpg::network {

void TcpConnection::upgrade(asio::ssl::context &ssl_ctx) {
  this->assertOpenConnection();

  // Wrap the existing raw socket into Asio SSL stream
  this->_sslStream.emplace(std::move(this->_tcpSocket), ssl_ctx);

  auto self = shared_from_this();
  this->_sslStream->async_handshake(
      asio::ssl::stream_base::server, [this, self](std::error_code ec) {
        if (ec) {
          std::cerr << "TLS Handshake Failed: " << ec.message() << "\n";
          return;
        }
        this->_state = ConnectionState::SECURE;
      });
}

void TcpConnection::write(std::string_view data) {
  this->assertOpenConnection();

  if (this->isSecure() && this->_sslStream) {
    // Writing to the secure stream;
    asio::async_write(*this->_sslStream, asio::buffer(data),
                      [](std::error_code, std::size_t) {});
    return;
  }

  // Fallback to the raw connection
  asio::async_write(this->_tcpSocket, asio::buffer(data),
                    [](std::error_code, std::size_t) {});
}

void TcpConnection::close() {
  this->assertOpenConnection();

  if (this->isSecure() && this->_sslStream) {
    this->_state = ConnectionState::CLOSING;
    this->_sslStream->lowest_layer().cancel();

    auto self = shared_from_this();
    this->_sslStream->async_shutdown([this, self](const asio::error_code &ec) {
      // Shut down the underlying TCP transport layer
      this->_sslStream->lowest_layer().shutdown(
          asio::ip::tcp::socket::shutdown_both);

      // Close the socket to free the file descriptor
      this->_sslStream->lowest_layer().close();

      this->_state = ConnectionState::CLOSED;
    });
    return;
  }

  this->_state = ConnectionState::CLOSING;
  this->_tcpSocket.shutdown(asio::ip::tcp::socket::shutdown_both);
  this->_tcpSocket.close();

  this->_state = ConnectionState::CLOSED;
}

} // namespace xtrpg::network