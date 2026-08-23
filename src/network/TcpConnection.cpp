#include "xtrpg/network/TcpConnection.hpp"

namespace xtrpg::network {

void TcpConnection::upgradeToTls(asio::ssl::context &ssl_ctx) {
  if (this->_isClosed) {
    // the TCP connection is closed
    std::cerr << "Unable to upgrade a closed TCP connection to TLS."
              << std::endl;
    return;
  }

  // Wrap the existing raw socket into Asio SSL stream
  this->_sslStream.emplace(std::move(this->_tcpSocket), ssl_ctx);

  auto self = shared_from_this();
  this->_sslStream->async_handshake(
      asio::ssl::stream_base::server, [this, self](std::error_code ec) {
        if (ec) {
          std::cerr << "TLS Handshake Failed: " << ec.message() << "\n";
          return;
        }

        this->_isTlsActive = true;
      });
}

void TcpConnection::write(std::string_view data) {
  if (this->_isClosed) {
    // the TCP connection is closed
    std::cerr << "Unable to write to a closed TCP connection." << std::endl;
    return;
  }

  if (this->_isTlsActive && this->_sslStream) {
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
  if (this->_isClosed) {
    std::cerr << "TCP Connection is already closed" << std::endl;
    return;
  }

  this->_isClosed = true;

  if (this->_isTlsActive && this->_sslStream) {
    this->_sslStream->lowest_layer().cancel();

    auto self = shared_from_this();
    this->_sslStream->async_shutdown([this, self](const asio::error_code &ec) {
      // Shut down the underlying TCP transport layer
      this->_sslStream->lowest_layer().shutdown(
          asio::ip::tcp::socket::shutdown_both);

      // Close the socket to free the file descriptor
      this->_sslStream->lowest_layer().close();
    });
    return;
  }

  this->_tcpSocket.shutdown(asio::ip::tcp::socket::shutdown_both);
  this->_tcpSocket.close();
}

} // namespace xtrpg::network